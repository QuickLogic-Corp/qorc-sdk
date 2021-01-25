/** @file sensor_audio_acquisition.c */

/*==========================================================
 *
 *-  Copyright Notice  -------------------------------------
 *                                                          
 *    Licensed Materials - Property of QuickLogic Corp.     
 *    Copyright (C) 2019 QuickLogic Corporation             
 *    All rights reserved                                   
 *    Use, duplication, or disclosure restricted            
 *                                                          
 *    File   : sensor_audio_acquisition.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

#include "sensor_audio_config.h"
#include "sensor_audio_acquisition.h"
#include "sensor_audio_process.h"
#include "eoss3_hal_audio.h"
#include "datablk_mgr.h"
#include "process_ids.h"

/* Sensor AUDIO capture ISR */
#define AUDIO_ISR_EVENT_NO_BUFFER  (1)   ///< error getting a new datablock buffer

#define AUDIO_ISR_OUTQS_NUM        (1)
QueueHandle_t   *audio_isr_outQs[AUDIO_ISR_OUTQS_NUM] = { &audio_dbp_thread_q };
QAI_DataBlock_t *paudio_data_block_prev = NULL;
int              audio_samples_collected = 0;

extern void audio_isr_onDmac0BufferDone(void);
outQ_processor_t audio_isr_outq_processor =
{
  .process_func = audio_isr_onDmac0BufferDone, // audio_acquisition_read_callback,
  .p_dbm = &audioBuffDataBlkMgr,
  .in_pid = AUDIO_ISR_PID,
  .outQ_num = 1,
  .outQ = audio_isr_outQs,
  .p_event_notifier = NULL
};

void audio_set_first_data_block()
{
  /* Acquire a datablock buffer */
  if (NULL == paudio_data_block_prev) 
  {
    datablk_mgr_acquire(audio_isr_outq_processor.p_dbm, &paudio_data_block_prev, 0);
  }
  configASSERT(paudio_data_block_prev); // probably indicates uninitialized datablock manager handle
  audio_samples_collected = 0;
  paudio_data_block_prev->dbHeader.Tstart = xTaskGetTickCount();
}

void audio_acquisition_read_callback(void)
{
    int gotNewBlock = 0;
    QAI_DataBlock_t  *pdata_block = NULL;
  
    if (!audio_sensordata_buffer_ready())
    {
      return;
    }
    /* Acquire a new data block buffer */
    datablk_mgr_acquire(audio_isr_outq_processor.p_dbm, &pdata_block, 0);
    if (pdata_block)
    {
        gotNewBlock = 1;
    }
    else
    {
        // send error message 
        // xQueueSendFromISR( error_queue, ... )
        if (audio_isr_outq_processor.p_event_notifier)
          (*audio_isr_outq_processor.p_event_notifier)(audio_isr_outq_processor.in_pid, AUDIO_ISR_EVENT_NO_BUFFER, NULL, 0);
        pdata_block = paudio_data_block_prev;
        pdata_block->dbHeader.Tstart = xTaskGetTickCount();
        pdata_block->dbHeader.numDropCount++;
    }

    if (gotNewBlock)
    {
        /* send the previously filled audio data to specified output Queues */     
        paudio_data_block_prev->dbHeader.Tend = pdata_block->dbHeader.Tstart;
        datablk_mgr_WriteDataBufferToQueues(&audio_isr_outq_processor, NULL, paudio_data_block_prev);
        paudio_data_block_prev = pdata_block;
    }
}

