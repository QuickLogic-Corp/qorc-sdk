/*==========================================================
 * Copyright 2020 QuickLogic Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *==========================================================*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "datablk_processor.h"
#include "ql_vr_common.h"
#include "ql_util.h"
#include "process_ids.h"
#include "ql_controlTask.h"
#include "ql_audio.h"
#include "string.h"
#include "vr_engine_api.h"

int ww_detected = 0;
int ww_size = 0;
int ww_confidence = 0;
char ww_keyword[20] = { 0 };
static int vr_engine_samples_per_frame = 240;
//********************************
// QuickLogic wrapper code

static bool fVRenabled = false;     // Tells VR whether to process or ignore data
SemaphoreHandle_t ql_vr_sem;       ///< Semaphore specific to QL VR functions
/** QL VR event data */
static ql_vr_event_data_t ql_vr_event_data ;

#define SAVE_DEBUG_BUF_ENABLE 1 // 1=enable if want to save first 1sec of VR buffer
#if (SAVE_DEBUG_BUF_ENABLE == 1)
#define AUDSIZE 8*4096
uint8_t    acAudio[AUDSIZE];
uint8_t*   pcAudio = acAudio;
#endif

/*********************************************************************************
 *
 *  VR-FSM interface
 *
 ********************************************************************************/
enum process_state  VR_pstate = PSTATE_UNCONFIG;
int                 VR_FSMConfigData = 50;

enum process_state VR_FSMAction(enum process_action pa, void* parg) {
    
    switch(pa) {
    case PACTION_CONFIG:
        VR_pstate = PSTATE_STOPPED;
        break;
        
    case PACTION_STOP:
        fVRenabled = false; // Tells vr to ignore incomming datablks
        VR_pstate = PSTATE_STOPPED;
        break;
        
    case PACTION_START:
#if (SAVE_DEBUG_BUF_ENABLE == 1)
        pcAudio = acAudio;  //TIM Debug: start over on WOS
#endif
        fVRenabled = true; // Tells vr to process incomming datablks
        VR_pstate = PSTATE_STARTED;
        break;
    default:
        configASSERT(0);
    }        
    return(VR_pstate);
}
//after ~2secs of stream data, restart VR
void enable_stream_VR(void)
{
  fVRenabled = true; // Tells vr to process incomming datablks
}
//during first ~2secs of stream data, disable VR
void disable_stream_VR(void)
{
  fVRenabled = false; // Tells vr to ignore incomming datablks
}

void datablk_pe_config_ql_vr(void *p_pe_object)
{
  ql_vr_sem = xSemaphoreCreateBinary(  );

  /* Initialize specific voice recognition engine used */
  vr_engine_init();
  /* set the samples-per-frame for the specific voice recognition used */
  vr_engine_samples_per_frame = vr_engine_get_samples_per_frame();

  xSemaphoreGive(ql_vr_sem);
  
  return ;
}

static int16_t a_audio_brick[240];
static int16_t a_last_buff_tail[240];
static uint32_t p_last_buff_tail_sz = 0;
//void vr_rdsp_clear_static_mem()
void vr_clear_static_mem()
{
  p_last_buff_tail_sz = 0;
}

static int vr_frame_sz_adapter(uint16_t *p_buffer, uint32_t num_samples, int16_t *p_brick, uint32_t n_size)
{
  uint16_t *paudio_brick = p_buffer;
  uint32_t samples_idx = 0;
  uint32_t new_tail = 0;
  
  for(int i = 0; i < p_last_buff_tail_sz; i++)
  {
    p_brick[samples_idx++] = a_last_buff_tail[i];
  }
  
  for(int i = 0; i < n_size - p_last_buff_tail_sz; i++)
  {
    p_brick[samples_idx++] = paudio_brick[i];
  }
  
  for(int i = n_size - p_last_buff_tail_sz; i < num_samples; i++)
  {
    a_last_buff_tail[new_tail++] = paudio_brick[i];
  }
  
  p_last_buff_tail_sz = new_tail;
  
  return p_last_buff_tail_sz; 
}

void datablk_pe_process_ql_vr(QAI_DataBlock_t *pIn, QAI_DataBlock_t *pOut, QAI_DataBlock_t **pRet,
                      void (*p_event_notifier)(int pid, int event_type, void *p_event_data, int num_data_bytes))
{
     e_ql_vr_status      ret;
     
     int k = pIn->dbHeader.numDataElements * pIn->dbHeader.dataElementSize;
#if (SAVE_DEBUG_BUF_ENABLE == 1)
     uint8_t* p2 = pIn->p_data;
     for (int i = 0; i != k; i++) {
         *pcAudio++ = *p2++;
         if (pcAudio == &acAudio[AUDSIZE])
             pcAudio = acAudio+AUDSIZE-1;   // Overwrite last sample
     }
#endif
     if(!fVRenabled) // If VR is disabled, bypass VR process and return.
       return;

     uint32_t ksamplesNew = pIn->dbHeader.numDataElements/pIn->dbHeader.numDataChannels;
     while(1)
     {
         uint32_t balance_samples = vr_frame_sz_adapter((uint16_t*)pIn->p_data, ksamplesNew, &a_audio_brick[0], vr_engine_samples_per_frame);
         
         ret = vr_engine_process((short*)&a_audio_brick[0]);
        
         if ( ret ==  e_ql_vr_status_detection_ok) {
           vr_clear_static_mem();
           disable_stream_VR(); //disable VR automatically
           ql_vr_event_data.startFramesBack = ww_size; // a frame is a sample ?
           ql_vr_event_data.score = ww_confidence;
           ql_vr_event_data.p_phrase_text = ww_keyword;   // phrase text
           ql_vr_event_data.len_phrase_text = strlen(ww_keyword);
           if (p_event_notifier) {
              (*p_event_notifier)(AUDIO_QL_VR_PID, 0, &ql_vr_event_data, sizeof(ql_vr_event_data));
           }
           break; //do not process any more
         } else if (ret != 0) {
           dbg_str_int("ql_vr_process ret ", ret);
         }

         if(balance_samples < vr_engine_samples_per_frame)
         {
           break;
         }
         else
         {
           ksamplesNew = 0;
         }
     }
       
     return ;
}



