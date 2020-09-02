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

/** @file sensor_ad7476_acquisition.c */

#include "sensor_ad7476_config.h"
#include "sensor_ad7476_acquisition.h"
#include "sensor_ad7476_process.h"

#include "datablk_mgr.h"
#include "process_ids.h"

#include "eoss3_hal_fpga_adc_api.h"  /* ADC FPGA Driver API */

#define AD7476_ISR_OUTQS_NUM        (1)
QueueHandle_t   *ad7476_isr_outQs[AD7476_ISR_OUTQS_NUM] = { &ad7476_dbp_thread_q };
QAI_DataBlock_t *pad7476_data_block_prev = NULL;
int              ad7476_samples_collected = 0;

extern void ad7476_isr_DmacDone(void);
outQ_processor_t ad7476_isr_outq_processor =
{
  .process_func = ad7476_isr_DmacDone, // ad7476_acquisition_read_callback,
  .p_dbm = &ad7476BuffDataBlkMgr,
  .in_pid = AD7476_ISR_PID,
  .outQ_num = 1,
  .outQ = ad7476_isr_outQs,
  .p_event_notifier = NULL
};

void ad7476_set_first_data_block()
{
  /* Acquire a datablock buffer */
  if (NULL == pad7476_data_block_prev) 
  {
    datablk_mgr_acquire(ad7476_isr_outq_processor.p_dbm, &pad7476_data_block_prev, 0);
  }
  configASSERT(pad7476_data_block_prev); // probably indicates uninitialized datablock manager handle
  ad7476_samples_collected = 0;
  pad7476_data_block_prev->dbHeader.Tstart = xTaskGetTickCount();
}

void ad7476_acquisition_read_callback(void)
{
    int gotNewBlock = 0;
    QAI_DataBlock_t  *pdata_block = NULL;
  
    if (!ad7476_sensordata_buffer_ready())
    {
      return;
    }
    /* Acquire a new data block buffer */
    datablk_mgr_acquire(ad7476_isr_outq_processor.p_dbm, &pdata_block, 0);
    if (pdata_block)
    {
        gotNewBlock = 1;
    }
    else
    {
        // send error message 
        // xQueueSendFromISR( error_queue, ... )
        if (ad7476_isr_outq_processor.p_event_notifier)
          (*ad7476_isr_outq_processor.p_event_notifier)(ad7476_isr_outq_processor.in_pid, AD7476_ISR_EVENT_NO_BUFFER, NULL, 0);
        pdata_block = pad7476_data_block_prev;
        pdata_block->dbHeader.Tstart = xTaskGetTickCount();
        pdata_block->dbHeader.numDropCount++;
    }

    if (gotNewBlock)
    {
        /* send the previously filled audio data to specified output Queues */     
        pad7476_data_block_prev->dbHeader.Tend = pdata_block->dbHeader.Tstart;
        datablk_mgr_WriteDataBufferToQueues(&ad7476_isr_outq_processor, NULL, pad7476_data_block_prev);
        pad7476_data_block_prev = pdata_block;
    }
}

