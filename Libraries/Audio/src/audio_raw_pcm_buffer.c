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

#include "Fw_global_config.h"

#include "FreeRTOS.h"
#include "timers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ql_util.h"


#include "datablk_mgr.h"
#include "audio_raw_pcm_buffer.h"

/* Audio Raw Stream -
   This module holds the next Data Blocks to be streamed in a Queue.
   The Data blocks held in the Queue are expected to be from a Cricular Buffer.
   Then the size of the Queue must be substracted from the Circular buffer hold count.
   Else Data Block manager will run out of Free data blocks.

   Usage -
   1. Initialize Queue and other variables using - init_audio_raw_stream()
   2. Register with Circular buffer callback function with QueueHandle_t
   3. Call one of the function to get the Audio samples
      From ISR context - get_audio_stream_dataFromISR()
      From non-ISR context - get_audio_stream_data()
  4. Optionally pass non zero period for a Timer and register with callback function.
     The callback function may implement to send a msg to a task when required 
     amount of data is available.

   Note: 
      This is not a replacement for a Circular Buffer, but expects to get data from
      the Circular Buffer.
      The reason for having an array/queue to hold the buffers is to be able to 
      immmediately give the required amount of Linear Buffer from the Data Blocks
      held by the Circular Buffer.
*/


#define RAW_DATA_CONTEXT_NON_ISR  0
#define RAW_DATA_CONTEXT_ISR      1


/* this is pointer to the Queue that holds the Data blocks to be streamed. 
  Effectively the Queue works as FIFO/Cricular buffer */
static QueueHandle_t p_audio_stream_Queue;
/* holds the pointer to the block that is not emptied last call */
static QAI_DataBlock_t *p_raw_stream_block;
/* read index into the current data block start emptying */
//static int g_raw_stream_rd_index;

/* this is needed since there is no way to compute if the Queue is empty */
static int g_samples_per_block;

/* provide an option through timer for a callback function */
static TimerHandle_t raw_stream_timer;
static void (*raw_data_ready_callbackfn)(int required_buf_size) = NULL;
static int raw_data_callback_count;
//static int raw_data_callback_arg;

/* Periodically check the amount of data available and inform using callback function */
static void raw_stream_timer_callback(TimerHandle_t xTimer)
{
  /* only if callback is set */
  if(raw_data_ready_callbackfn != NULL)
  {
    /* only if non zero count */
    if(raw_data_callback_count > 0)
    {
      int avail_count = get_raw_audio_sample_count();
       
      /* only if required amount in the queue blocks */
      if(avail_count >= raw_data_callback_count)
      {
         uint32_t timestamp=xTaskGetTickCount();
         QL_LOG_DBG(" timer tc = %d n = %d ", timestamp, avail_count);
         (*raw_data_ready_callbackfn)(raw_data_callback_count);
    }
    }else
    {
      configASSERT(0);
    }
  }
  
  return;
}

void set_audio_raw_block_sample_count(int samples_per_data_block);

/* Creates a Queue to hold DataBlocks. get the Queuehandle using the API.
 * Also can set a period for call back timer which can be started when need
 * Note: calling with a period 0 or less will not create a timer.
 */
void init_audio_raw_stream(int samples_per_data_block, int callback_timer_period_ms)
{
  /* first create a Queue  to hold the Datablocks */
  p_audio_stream_Queue = xQueueCreate(MAX_AUDIO_RAW_STREAM_QUEUE_SIZE, sizeof(QAI_DataBlock_t *));
  vQueueAddToRegistry(p_audio_stream_Queue,"p_audio_stream_Queue");
  //configASSERT(p_audio_stream_Queue);
  
  /* set the current block pointer and read index  */
  p_raw_stream_block = NULL;
  set_audio_raw_block_sample_count( samples_per_data_block);
  
  /* create a 15ms or more timer that can be used for callback function */
  if(callback_timer_period_ms > 0)
  {
    /* the datablocks are 15ms */
    if(callback_timer_period_ms < 15)
      callback_timer_period_ms = 15;
    raw_stream_timer = xTimerCreate("raw_stream_timer", pdMS_TO_TICKS(callback_timer_period_ms), pdTRUE, (void*)0, raw_stream_timer_callback);
  }
  else
    raw_stream_timer = NULL;
  raw_data_ready_callbackfn = NULL;
  raw_data_callback_count = 0;
  
  return;
}

/* allow change of number of samples per block at a later time */
void set_audio_raw_block_sample_count(int samples_per_data_block)
{
  g_samples_per_block = samples_per_data_block;
 
}

/*
* This module can be used to set the callback function when at least the required
* amount of data is available in the Raw stream queue.
* Note: If the data is not taken fast enough, there will be 1 callback for each Timer period.
*/
void set_raw_data_ready_callback(void (*data_ready_callback)(int required_buf_size), int callback_sample_count)
{
  raw_data_ready_callbackfn = data_ready_callback;
  raw_data_callback_count = callback_sample_count;
  /* if timer is not created, it can not be started */
  if(raw_stream_timer != NULL)
    xTimerStart(raw_stream_timer, pdMS_TO_TICKS(0));
  
}

/* get the Queue handle through API only */
QueueHandle_t get_audio_stream_queue(void)
{
    return p_audio_stream_Queue;
}



/* calculate the available blocks in the held queue */
static int get_avail_audio_block_count(int context)
{
    /* each block in the Queue has full buffer of samples */
    int n_count;
    if(context == RAW_DATA_CONTEXT_NON_ISR)
      n_count = uxQueueMessagesWaiting(p_audio_stream_Queue);
    else
      n_count = uxQueueMessagesWaitingFromISR(p_audio_stream_Queue);
    
    return n_count;
}

/* calculate the available samples in the held buffer */
//static int get_avail_audio_sample_count(int context)
//{
//    /* each block in the Queue has full buffer of samples */
//    int queue_count;
//    if(context == RAW_DATA_CONTEXT_NON_ISR)
//      queue_count = uxQueueMessagesWaiting(p_audio_stream_Queue);
//    else
//      queue_count = uxQueueMessagesWaitingFromISR(p_audio_stream_Queue);
//    int sample_count = queue_count * g_samples_per_block;
//    
//    /* add the partial block */
//    if(p_raw_stream_block != NULL)
//    {
//      sample_count += (g_samples_per_block - g_raw_stream_rd_index);
//    }
//    
//    return sample_count;
//}


/* Fill the given number of audio raw sample in the given buffer. 
   If the required number is not available returns 0.
*/
//static int get_audio_stream_data_generic(int16_t *buf, int sample_count, int context)
//{
//  /* if not enough samples available for filling return 0 */
//  /* Note: This should not happen when called from ISR. So, proper Queue size
//     should be set */
//  if(get_avail_audio_sample_count(context) < sample_count)
//    return 0;
//  
//  BaseType_t  xResult;
//  int copy_count;
//  int total_count = 0;
//  int queue_count;
//  
//  /* get the number of blocks available  */
//  if(context == RAW_DATA_CONTEXT_NON_ISR)
//   queue_count = uxQueueMessagesWaiting(p_audio_stream_Queue);
//  else
//   queue_count = uxQueueMessagesWaitingFromISR(p_audio_stream_Queue); 
//  
//  /* first block is the last un-emptied block or a new block */
//  if(p_raw_stream_block == NULL)
//  {
//    xResult = xQueueReceive( p_audio_stream_Queue, &p_raw_stream_block, 0);
//    g_raw_stream_rd_index = 0;
//    copy_count = g_samples_per_block;
//  }
//  else
//  {
//    copy_count = (g_samples_per_block - g_raw_stream_rd_index);
//  }
//  
//  /* loop until buffer is filled */
//  for(int i=0; i< queue_count; i++)
//  {
//    /* must cast the pointer into correct data type for pointer arithemetic */
//    int16_t *p_data = (int16_t *)p_raw_stream_block->p_data;
//    
//    /* do not exceed the required number for copy */
//    if((total_count + copy_count) > sample_count)
//      copy_count = sample_count - total_count;
//    
//    memcpy((void *)&buf[total_count], (void *)&p_data[g_raw_stream_rd_index], sizeof(int16_t)*copy_count);
//    total_count += copy_count;
//    g_raw_stream_rd_index += copy_count;
//    
//    /* release the current block only if all the samples are taken */
//    if(g_raw_stream_rd_index >= g_samples_per_block)
//    {
//      if(p_raw_stream_block->dbHeader.numUseCount <= 0)
//      {
//        //printf("error");
//        //configASSERT(0);
//      }
//      if(p_raw_stream_block->dbHeader.numUseCount > 0)
//      {
//        if(context == RAW_DATA_CONTEXT_NON_ISR)
//          datablk_mgr_release_generic(p_raw_stream_block);
//        else
//          datablk_mgr_releaseFromISR_generic(p_raw_stream_block);
//      }
//      p_raw_stream_block = NULL;
//      g_raw_stream_rd_index = 0;
//    }
//    /* if the required samples are filled, stop */
//    if(total_count >= sample_count)
//      break;
//    
//    /* if need more samples, get the next block */
//    if(context == RAW_DATA_CONTEXT_NON_ISR)
//      xResult = xQueueReceive( p_audio_stream_Queue, &p_raw_stream_block, 0);
//    else
//      xResult = xQueueReceiveFromISR( p_audio_stream_Queue, &p_raw_stream_block, 0);
//    if(xResult == pdFAIL)
//    {
//      //configASSERT();
//      break;
//    }
//    /* full block available for fill */
//    copy_count = g_samples_per_block;
//  }
//  
//  return total_count;
//}

/* get the block from p_audio_stream_Queue, copy data from block to the buffer and release */
static int get_cur_block_data(int16_t *buf, int context)
{
    QAI_DataBlock_t *p_block;
  BaseType_t  xResult;
    int packet_size = 0;  // size in int16
      
    /* get block from queue */
    xResult = xQueueReceive( p_audio_stream_Queue, &p_block, 0);    
    configASSERT(xResult != pdFAIL);
    
    /* copy data from block, check usecount and release */
    if(p_block->dbHeader.numUseCount > 0)
    {
#if QL_XPORT_INCLUDE_HEADER==0
     //configASSERT(p_block->dbHeader.numDataElements == g_samples_per_block);
     packet_size = p_block->dbHeader.numDataElements;
     memcpy((void *)&buf[0], p_block->p_data, p_block->dbHeader.dataElementSize*packet_size);
#else
     packet_size = (sizeof(QAI_DataBlockHeader_t)/p_block->dbHeader.dataElementSize) + p_block->dbHeader.numDataElements;
     memcpy((void *)&buf[0], p_block, p_block->dbHeader.dataElementSize*packet_size);     
#endif
     
  if(context == RAW_DATA_CONTEXT_NON_ISR)
        datablk_mgr_release_generic(p_block);
  else
        datablk_mgr_releaseFromISR_generic(p_block);
  
  }
  else
  {
    //  configASSERT(0);
      packet_size = -1;    
  }
    return packet_size;
}
/* Fill the given number of audio raw sample in the given buffer. 
   If the required number is not available returns 0.
*/
static   int call_count = 0;
static int get_audio_stream_block_data_generic(int16_t *buf, int sample_count, int context)
  {
  int ret = 0;
  int block_count;
  int copy_count = 0;
  int total_count = 0;

  call_count++;
  /* get blocks in queue */
  block_count = get_avail_audio_block_count(context);
    
  /* if sufficient blocks are available in the queue */
  if(block_count * g_samples_per_block > sample_count)
    {
    /* loop for every blocks and break when buffer is filled with samples = sample_count */
    for(int i=0; i< block_count; i++)
      {
      /* copy data from the current clock and release the block */
      configASSERT(total_count + g_samples_per_block <= sample_count);
      
      copy_count = get_cur_block_data(&buf[total_count], context);
      
      if(copy_count == g_samples_per_block)
      {
           total_count += copy_count;
          /* break loop when required samples are read */
          if(total_count >= sample_count) {
            /* success */
            ret = total_count;
            break;
      }
    }
    else
    {
        /* error case */
      //  configASSERT(0);
        ret = -1;
      break;
    }
  }
  //  configASSERT(total_count == sample_count);
  }
  else
  { 
    /* unexpected case, but not really error case */
    /* if not enough blocks are available for filling return 0 */
    /* Note: This should not happen when called from ISR. So, proper Queue size
     should be set */ 
    /* it's ok to have this case, stll checking for cases */
    //configASSERT(0);
    ret = 0;
  }

  /* expected that tx buffer is a multiple of block size */
  QL_LOG_DBG("  %d  %d  %d\n", call_count, block_count, total_count);
  
  return ret;
}
/*
* This module releases all the currently held datablocks. The circular buffer
* output to queuse must be disabled before calling this.
* This module also stops the timer
*/
void stop_audio_raw_stream(void)
{
  int queue_count;

  /* first stop the timer if created and reset callback function */
  if(raw_stream_timer != NULL)
      xTimerStop(raw_stream_timer, pdMS_TO_TICKS(0));
  
  raw_data_ready_callbackfn = NULL;
  raw_data_callback_count = 0;
  
  /* release if holding a partial block */
  if(p_raw_stream_block != NULL)
    datablk_mgr_release_generic(p_raw_stream_block);
  
  /* release all the block held in the queue */
  queue_count = uxQueueMessagesWaiting(p_audio_stream_Queue);
  for(int i = 0; i < queue_count; i++)
  {
    xQueueReceive( p_audio_stream_Queue, &p_raw_stream_block, 0);
    datablk_mgr_release_generic(p_raw_stream_block);
  }

  p_raw_stream_block = NULL;

  return;
}


                           
/* From non-ISR: Fill the required number of audio raw sample in the given buffer. 
   If the required number is not available returns 0.
*/
int get_audio_stream_data(int16_t *buf, int sample_count)
{
  /* non ISR context */
  return  get_audio_stream_block_data_generic(buf, sample_count, RAW_DATA_CONTEXT_NON_ISR);
}
                           
/* From ISR: Fill the required number of audio raw sample in the given buffer. 
   If the required number is not available returns 0.
*/
int get_audio_stream_dataFromISR(int16_t *buf, int sample_count)
{
  /* ISR context */
  return  get_audio_stream_block_data_generic(buf, sample_count, RAW_DATA_CONTEXT_ISR);
}

/* From non-ISR: Fill the required number of audio raw sample in the given buffer. 
   If the required number is not available returns 0.
   sample_count is required to be a multiple of blockSize
*/
int get_audio_stream_block_data(int16_t *buf, int sample_count)
{
  /* non ISR context */
  return  get_audio_stream_block_data_generic(buf, sample_count, RAW_DATA_CONTEXT_NON_ISR);
}
                           
/* From ISR: Fill the required number of audio raw sample in the given buffer. 
   If the required number is not available returns 0.
*/
int get_audio_stream_block_dataFromISR(int16_t *buf, int sample_count)
{
  /* ISR context */
  return  get_audio_stream_block_data_generic(buf, sample_count, RAW_DATA_CONTEXT_ISR);
}

/* return available raw samples in the queue */
int get_raw_audio_sample_count(void)
{
  return get_avail_audio_block_count(RAW_DATA_CONTEXT_NON_ISR)*g_samples_per_block;
}
/* return available raw samples in the queue from ISR */
int get_raw_audio_sample_countFromISR(void)
{
  return get_avail_audio_block_count(RAW_DATA_CONTEXT_ISR)*g_samples_per_block;  
}

