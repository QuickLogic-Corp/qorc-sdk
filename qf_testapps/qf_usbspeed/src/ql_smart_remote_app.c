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
 *    File   : ql_smart_remote_app.c
 *    Purpose: application code which sets the drivers and associated processing 
 *    for smart remote application. 
 *                                                          
 *=========================================================*/

/** @file ql_smart_remote_app.c */

#include "Fw_global_config.h"

#include <string.h>
#include <stdbool.h>
#include "datablk_mgr.h"
#include "datablk_processor.h"
#include "process_ids.h"
#include "ql_util.h"

/** @addtogroup QL_SMART_REMOTE_APP QuickAI SDK Audio smart remote example
 *
 *  @brief Smart remote application code
 * This example demonstrates VR engine which works with QL QuickAI framework.
 * (\ref THREAD_QL_VR_PROCESS) is provided in this example.
 * 
 * @{
 */

#define THREAD_1_Q_SIZE   (60)
#define THREAD_1_PRIORITY (10)

#define NUM_AUDIO_BRICKS (60)
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

#include "ql_vr.h"

uint8_t nwaHeapBuffer[30*1024]; ///< Heap memory needed for the Nuance VR Engine
void audio_ql_vr_event_handler(int pid, int event_type, void *p_event_data, int num_data_bytes);



/* ql vr processing element functions */
datablk_pe_funcs_t ql_vr_funcs = {datablk_pe_config_ql_vr, datablk_pe_process_ql_vr, NULL, NULL, NULL } ;

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

outQ_processor_t audio_ql_vr_outq_processor =
{
  .process_func = NULL,
  .p_dbm = &audioBuffDataBlkMgr,
  .in_pid = AUDIO_QL_VR_PID,
  .outQ_num = 0,
  .outQ = NULL,
  .p_event_notifier = audio_ql_vr_event_handler
};

datablk_pe_descriptor_t  datablk_pe_descr_audio[] =
{ /* processing element descriptor for Nuance VR */
    { AUDIO_ISR_PID, 
    AUDIO_QL_VR_PID, 
    true, 
    false, 
    true, 
    &audio_ql_vr_outq_processor, 
    &ql_vr_funcs, 
    NULL, 
    &ql_vr_sem 
    },
    
};

datablk_processor_params_t datablk_processor_params[] = { 
    { THREAD_1_PRIORITY, &thread_1_q, sizeof(datablk_pe_descr_audio)/sizeof(datablk_pe_descr_audio[0]), datablk_pe_descr_audio, 256, "THREAD_QL_VR_PROCESS", NULL}
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
void ql_smart_remote_example(void)
{
  /** Create Audio Queues */
  thread_1_q = xQueueCreate(THREAD_1_Q_SIZE, sizeof(QAI_DataBlock_t *));
  QL_LOG_INFO( "Starting QL Smart Remote Test Application \n");

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

void audio_ql_vr_event_handler(int pid, int event_type, void *p_event_data, int num_data_bytes)
{
  static int kp_dtection_count = 1; //chalil, todo - void static, shall be read from structure. 
  ql_vr_event_data_t *p_ql_vr_evt = (ql_vr_event_data_t *)p_event_data;
  // Wait on AudioSysQ handle
  if (pid != AUDIO_QL_VR_PID)
    return;
#if AUDIO_LED_TEST
    LedOrangeOn_AutoOff();
#endif
   uint32_t timestamp=xTaskGetTickCount();

    QL_LOG_RESULT("VR EVT-KP %2d : ts =%6d, idx = %2d, score = %4d, offset = %6d\n",
                  kp_dtection_count++, 
                  timestamp, 
                  0, /*voicespot_class_string,*/ 
                  p_ql_vr_evt->score, 
                  p_ql_vr_evt->startFramesBack);


}

/** @} */
