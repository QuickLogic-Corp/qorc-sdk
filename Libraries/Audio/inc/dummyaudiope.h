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

/*
 *
 */
 
#include "Fw_global_config.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "datablk_mgr.h"

#include "vs_process.h"  //TIM
#include "ql_vr_common.h"

extern outQ_processor_t dummyaudio_outq_processor;
extern SemaphoreHandle_t  dummyaudio_sem;
  
void datablk_pe_config_dummyaudio(void *p_pe_object);
void datablk_pe_process_dummyaudio(QAI_DataBlock_t *pIn, QAI_DataBlock_t *pOut, QAI_DataBlock_t **pRet,
                                 void (*p_event_notifier)(int pid, int event_type, void *p_event_data, int num_data_bytes));


//TIM?????????????????
typedef struct _ql_vr_info_rdsp_t
{
    struct vsProcessVar* p_vs;
    uint32_t n_frame_sz;
    uint32_t n_samp_rate;
    //ql_vr_callback pf_callback;
    void *ql_vr_callback_context;
    uint32_t grammar;
    int32_t timerFlag;   
    int32_t foundTrigger;    
    int32_t detection_frame;
    int32_t isSpeech;
    int32_t wakeUpSeen;
    uint16_t *phrase;
    int32_t phraseLen;
    int32_t startFramesBack;
    int32_t phase;

    int16_t  kp_word_mapping[MAX_NO_OF_KPDS];
#ifdef KPDCMD
   int16_t  cmd_word_mapping[MAX_CMD];
#endif
   int32_t frame_count;
   int32_t last_event;
   int32_t event_duration_frames;
   int32_t num_frames_per_second;
   int32_t num_scores;
   int32_t *scores;
   //int32_t *p_max_scores;        //vsQ->num_outputs];
   //int32_t *p_max_frame_count;   //vsQ->num_outputs];     // Max scores and frame count

   int32_t frame_size_32;
   float *in_buffer_32;
   uint32_t processing_period;
   int32_t processing_level;
   int32_t event_threshold;
}ql_vr_info_rdsp_t;
