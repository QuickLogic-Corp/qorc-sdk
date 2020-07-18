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

/*==========================================================
 *                                                          
 *    File   : audio_pipeline_example.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

/** @file audio_pipeline_example.c */

#include "Fw_global_config.h"

#include <string.h>
#include <stdbool.h>
#include "datablk_mgr.h"
#include "datablk_processor.h"
#include "process_ids.h"

/** @addtogroup QAI_AUDIO_PIPELINE_EXAMPLE QuickAI SDK Audio pipeline example
 *
 *  @brief Audio pipeline example code
 * This example code demonstrates setting up Audio Queues,
 * setting up the audio brick buffer manager (\ref DATABLK_MGR)
 * and setting up the thread processing elements (\ref THREAD_PE).
 * A specific audio processing element for voice recognition using the Nuance VR
 * (\ref THREAD_PE_NUANCEVR) is provided in this example.
 * 
 * @{
 */

#define THREAD_1_Q_SIZE   (10)
#define THREAD_1_PRIORITY (10)

#define NUM_AUDIO_BRICKS (30)
#define AUDIO_BRICK_SIZE_SAMPLES (240)

typedef struct {
  QAI_DataBlockHeader_t dbHeader;
  int16_t pcm_data[AUDIO_BRICK_SIZE_SAMPLES];
} QAI_AudioBrick_t ;

#ifdef GCC_MAKE
QAI_AudioBrick_t dma_buffer[NUM_AUDIO_BRICKS] _PLACE_("HWA");
#else
QAI_AudioBrick_t dma_buffer[NUM_AUDIO_BRICKS] @ "HWA";
#endif
QAI_DataBlockMgr_t audioBuffDataBlkMgr;
QueueHandle_t  thread_1_q;

#include "nuance_vr.h"

/* Add the Nuance VR blob file containing phrases to be recognized */
//#include "OK_QuickLogic_enu_wuw.blob.h" // Ok Quicklogic - Key Phrase needs pause in between
#include "quicklogic_Cosmos_enu_wuw_SI.blob.h" //OK Cosmos - Key Phrase works better

uint8_t nwaHeapBuffer[30*1024]; ///< Heap memory needed for the Nuance VR Engine
void audio_nuance_vr_event_handler(int pid, int event_type, void *p_event_data, int num_data_bytes);

datablk_pe_config_nuance_vr_t nwa_cfg = {
  /** Setup Nuance VR Engine configuration, Use English language model */
  .planguage_model = (void *)gsData.szData,
  .modelSize = sizeof(gsData.szData),
  .pHeapMem = nwaHeapBuffer,
  .heapSize = sizeof(nwaHeapBuffer),
  .frameSamples = AUDIO_BRICK_SIZE_SAMPLES,
  .bUsesMAD = 0 // false
};

/* Nuance vr processing element functions */
datablk_pe_funcs_t nuance_vr_funcs = {datablk_pe_config_nuance_vr, datablk_pe_process_nuance_vr, NULL, NULL, (void *)&nwa_cfg } ;

struct st_dbm_init {
  QAI_DataBlockMgr_t *pdatablk_mgr_handle;
  void  *pmem;
  int mem_size;
  int item_count;
  int item_size_bytes;
} ;

struct st_dbm_init dbm_init_table[] =
{ 
  {&audioBuffDataBlkMgr, (void *)dma_buffer, sizeof(dma_buffer), AUDIO_BRICK_SIZE_SAMPLES, sizeof(int16_t)},
};

outQ_processor_t audio_nuance_vr_outq_processor =
{
  .process_func = NULL,
  .p_dbm = &audioBuffDataBlkMgr,
  .in_pid = AUDIO_NUANCE_VR_PID,
  .outQ_num = 0,
  .outQ = NULL,
  .p_event_notifier = audio_nuance_vr_event_handler
};

datablk_pe_descriptor_t  datablk_pe_descr_audio[] =
{ // { IN_ID, OUT_ID, ACTIVE, fSupplyOut, fReleaseIn, OUTQ_NUM, outQ, evtHandler, dbmHandle, &pe_function_pointers, bypass_function, pe_semaphore  }
  
    /* processing element descriptor for Nuance VR */
    { AUDIO_ISR_PID, AUDIO_NUANCE_VR_PID, true, false, true, &audio_nuance_vr_outq_processor, &nuance_vr_funcs, NULL, &nuance_vr_sem },
    
};

datablk_processor_params_t datablk_processor_params[] = { 
    { THREAD_1_PRIORITY, &thread_1_q, sizeof(datablk_pe_descr_audio)/sizeof(datablk_pe_descr_audio[0]), datablk_pe_descr_audio, 256, "THREAD_1", NULL}
};

/* Audio PDM capture ISR */
#define AUDIO_ISR_OUTQS_NUM (1)
QueueHandle_t *audio_isr_outQs[AUDIO_ISR_OUTQS_NUM] = { &thread_1_q };
extern void audio_isr_onDmac0BufferDone(void);
outQ_processor_t audio_isr_outq_processor =
{
  .process_func = audio_isr_onDmac0BufferDone,
  .p_dbm = &audioBuffDataBlkMgr,
  .in_pid = AUDIO_ISR_PID,
  .outQ_num = 1,
  .outQ = audio_isr_outQs,
  .p_event_notifier = NULL
};

#include <stdio.h>
void audio_pipeline_example(void)
{
  /** Create Audio Queues */
  thread_1_q = xQueueCreate(THREAD_1_Q_SIZE, sizeof(QAI_DataBlock_t *));

  /** Setup the data block manager */
  for (int k = 0; k < sizeof(dbm_init_table)/sizeof(struct st_dbm_init); k++) {
      datablk_mgr_init( dbm_init_table[k].pdatablk_mgr_handle, 
                        dbm_init_table[k].pmem, 
                        dbm_init_table[k].mem_size, 
                        dbm_init_table[k].item_count, 
                        dbm_init_table[k].item_size_bytes
                      );
  }

  /** Setup Audio Thread Handler Processing Elements */
  datablk_processor_task_setup(&datablk_processor_params[0]);
}

static int convert_unicode_to_utf8_one(unsigned short unic, unsigned char *pOutput)
{
    if (unic <= 0x0000007F)
    {
        // * U-00000000 - U-0000007F:  0xxxxxxx
        *pOutput     = (unic & 0x7F);
        return 1;
    }
    else if (unic >= 0x00000080 && unic <= 0x000007FF)
    {
        // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
        *(pOutput+1) = (unic & 0x3F) | 0x80;
        *pOutput     = ((unic >> 6) & 0x1F) | 0xC0;
        return 2;
    }
    else
    {
        // * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
        *(pOutput+2) = (unic & 0x3F) | 0x80;
        *(pOutput+1) = ((unic >>  6) & 0x3F) | 0x80;
        *pOutput     = ((unic >> 12) & 0x0F) | 0xE0;
        return 3;
    }
}

void audio_nuance_vr_event_handler(int pid, int event_type, void *p_event_data, int num_data_bytes)
{
  nuance_vr_event_data_t *p_nwa_evt = (nuance_vr_event_data_t *)p_event_data;
  // Wait on AudioSysQ handle
  if (pid != AUDIO_NUANCE_VR_PID)
    return;
  printf("[Nuance VR Event] Detected: ");
  for (int k = 0; k < (num_data_bytes); k++) {
      unsigned char uc;
      convert_unicode_to_utf8_one(p_nwa_evt->p_phrase_text[k],  &uc);
      printf("%c", uc);
  }
  printf(" @ %d of length %d frames (score: %d)\n", -p_nwa_evt->startFramesBack, p_nwa_evt->lengthFrames, p_nwa_evt->score);
}

/** @} */
