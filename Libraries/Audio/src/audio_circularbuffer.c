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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "process_ids.h"
#include "datablk_mgr.h"
#include "audio_circularbuffer.h"
#include "ql_controlTask.h"

#if MAX_AUDIO_SAVE_BLOCKS_ARRAY_SIZE 
   //must be defined in Fw_global_config.h
#else
  //assume all the blocks are 15ms and a total of ~3-4 sec of source blocks  
  #define MAX_AUDIO_SAVE_BLOCKS_ARRAY_SIZE  250  
#endif
#if MIN_AUDIO_SAVE_BLOCKS_SPARE_SIZE
   //must be defined in Fw_global_config.h
#else
  //by default this many blocks will be available in the free pool  
  #define MIN_AUDIO_SAVE_BLOCKS_SPARE_SIZE  30  
#endif

/*
* The Audio Circular Buffer can be in one of the 4 modes.
* Mode 0 : Disabled. The input blocks will be released immediately
* Mode 1 : Enabled. The input blocks are buffered until full. When full
*          the oldest stored block will be released
* Mode 2: For every input block, a callback function will be called with
*         oldest stored blocks. Example: Opus Encoder, I2S buffers
* Mode 3: For every input block, oldest stored blocks will be output to a
*         registered Queue. The block can be put at the front or back of the Queue 
*/
#define AUDIO_BUFFER_STATUS_DISABLE    0
#define AUDIO_BUFFER_STATUS_BUFFERING  1
#define AUDIO_BUFFER_STATUS_TRANSMIT   2
#define AUDIO_BUFFER_STATUS_OUTPUT2Q   3
typedef struct audio_blocks_info {
  
  /* these are pointers to audio data blocks that are being held in the circular buffer */
  QAI_DataBlock_t *audio_data_blocks[MAX_AUDIO_SAVE_BLOCKS_ARRAY_SIZE];
  
  /* max number of data blocks that can be held in the array, should be less than the array size */
  int array_hold_max;
  
  /* array index where to put the next received data block pointer, should be less than or equal to max */
  int array_wr_index;
  
  /* array index where to get the next transmit data block pointer, should be less than or equal to max */
  int array_rd_index;
  
  /* number of valid blocks pointers in array */
  int array_block_count;

  /* status the circular buffer, holding or releasing */
  int buffer_hold_status; /* 0 = not enabled, = nothing is held
                             1 = enabled and buffering only, no transmitting, oldest held block is released
                             2 = enabled and transmitting to a callback function 
                             3 = enabled and sendinng to a Queue */

  
  /* the register function to which the block to be released, when transmitting */
  void (*transmit_callback)(QAI_DataBlock_t *pBlock);
  
  /* the callback function can be called with a the given number of block in a loop */
  int transmit_count; 

  /* the callback function will release the block after the call back retuns if mode is 1, else it doesn't release. */
  int transmit_mode; 

  /* when a new datablock is received, the oldest can datablock can be output to a Queue */
  QueueHandle_t audiobuffer_outputQ_handle;

  /* the number blocks to be output to a Queue can be more than 1 */
  int outputQ_count;
  
  /* the output block can be sent to the front or back of the Queue */
  Circularbuffer_outQ_mode_t outputQ_mode;

} audio_blocks_info_t;

/* audio blocks are stored in a circular buffer */
static audio_blocks_info_t audio_buffer_info; 

/* protect setup changes using a semaphore */
SemaphoreHandle_t audio_circbuffer_sem = NULL; 

/*********************************************************************************
 *
 *  CircularBuffer-FSM interface
 *
 ********************************************************************************/
static struct CircularBuffer_ConfigData CircularBuffer_ConfigDataLocal;
enum process_state CircularBuffer_State = PSTATE_UNCONFIG;

enum process_state CircularBuffer_FSMAction(enum process_action pa, void* pv) {
       
    switch(pa) {   
    case PACTION_CONFIG:
        configASSERT(CircularBuffer_State == PSTATE_UNCONFIG);
        CircularBuffer_ConfigDataLocal = *((struct CircularBuffer_ConfigData*)pv);
        set_audiobuffer_hold_count(CircularBuffer_ConfigDataLocal.hold_count, 0);
        CircularBuffer_State = PSTATE_STOPPED;
        break;
        
    case PACTION_STOP:
        configASSERT(CircularBuffer_State != PSTATE_UNCONFIG);
        disable_audiobuffer_hold();
        CircularBuffer_State = PSTATE_STOPPED;
        break;
        
    case PACTION_START_SAVING:
        configASSERT(CircularBuffer_State != PSTATE_UNCONFIG);
        enable_audiobuffer_hold();
        CircularBuffer_State = PSTATE_SAVING;
        break;

    case PACTION_START_OUTQ:
        configASSERT(CircularBuffer_State != PSTATE_UNCONFIG);
        enable_output2Q_audiobuffer(*(CircularBuffer_ConfigDataLocal.outQ_handle), CircularBuffer_ConfigDataLocal.out_count, CircularBuffer_ConfigDataLocal.queue_mode );
        CircularBuffer_State = PSTATE_OUTQING;
        break;
    default:
        CircularBuffer_State = PSTATE_UNKNOWN;
        configASSERT(0);
    }
return(CircularBuffer_State);
}


/* 
 * Release the data blocks that are already tranmitted or when full .
 */
static void release_used_blocks(audio_blocks_info_t *blocks_info, int release_count)
{
    int k;
    QAI_DataBlock_t *pBlock;
    int index = blocks_info->array_rd_index;
      
    /* first release the requested count */
    for(k = 0; k < release_count; k++)
    {
      pBlock = blocks_info->audio_data_blocks[index];

      /* release only if valid block pointer */
      if(pBlock) { 
        /* only if nonzero usecount, else it is an error */
        /* Should we assert here since this should never happen ? */
        if(pBlock->dbHeader.numUseCount > 0)
          datablk_mgr_release_generic(pBlock);
      }
      blocks_info->audio_data_blocks[index++] = NULL;
      if(index >= blocks_info->array_hold_max)
        index = 0;
    }

    /* update the read index */
    blocks_info->array_rd_index = index;
    blocks_info->array_block_count -= release_count;

    /*update valid block count */
    if(blocks_info->array_block_count < 0)
      blocks_info->array_block_count = 0;
    return;
}
static void release_used_blocks_no_db_release(audio_blocks_info_t *blocks_info, int release_count)
{
    int k;

    int index = blocks_info->array_rd_index;
      
    /* first release the requested count */
    for(k = 0; k < release_count; k++)
    {
      blocks_info->audio_data_blocks[index++] = NULL;
      if(index >= blocks_info->array_hold_max)
        index = 0;
    }

    /* update the read index */
    blocks_info->array_rd_index = index;
    blocks_info->array_block_count -= release_count;

    /*update valid block count */
    if(blocks_info->array_block_count < 0)
      blocks_info->array_block_count = 0;
    return;
}
/*
 * Reset the circular buffer save array struct elements
 */
static void reset_audiobuffer_blocks(audio_blocks_info_t *blocks_info, int array_hold_max)
{
    /* can hold only max array size */
    if(array_hold_max > MAX_AUDIO_SAVE_BLOCKS_ARRAY_SIZE)
      array_hold_max = MAX_AUDIO_SAVE_BLOCKS_ARRAY_SIZE;
    
    /* reset the data block array and storage indices */
    blocks_info->array_wr_index = 0;
    blocks_info->array_rd_index = 0;
    blocks_info->transmit_count = 1;
 //   blocks_info->transmit_mode = CIRCULARBUFFER_TRANSMIT_MODE_NO_RELEASE;
    blocks_info->array_block_count = 0;

    /* release all the blocks if non-null and reset to NULL */
    release_used_blocks(blocks_info, array_hold_max);
    blocks_info->array_rd_index = 0;
    blocks_info->array_block_count = 0;

    return;
}

/*
 * Call the callback function in a loop if there are enough blocks
 */
static void transmit_to_callback(int count)
{
  audio_blocks_info_t *audio = &audio_buffer_info;

  /* if not enough wait for the blocks to fill */
  if(count < audio->transmit_count)
   return;

  /* can not call null function, so set the state to default release */
  if(!audio->transmit_callback)
  {
    audio->buffer_hold_status = AUDIO_BUFFER_STATUS_BUFFERING;
    return;
  }
  /* give the blocks for processing and release */
  for(int i= 0; i< audio->transmit_count;i++)
  {
    QAI_DataBlock_t *pBlock = audio->audio_data_blocks[audio->array_rd_index];

    /* call the processing function */
    audio->transmit_callback(pBlock);
    
    if(audio->transmit_mode == CIRCULARBUFFER_TRANSMIT_MODE_RELEASE)
    {
     release_used_blocks(audio, 1);
    }else
    {
      release_used_blocks_no_db_release(audio, 1);
    }

    /* should not happen: do not exceed available blocks */
    if(audio->array_rd_index == audio->array_wr_index)
    {
      configASSERT(0);
     break;
    }
  }

  return;
}

/*
 * Send the output to a Queue in a loop if there are enough blocks.
 * Stop if the Queue is full.
 */
static int transmit_to_outputQ(int count)
{
  int count_actual = 0;
  audio_blocks_info_t *audio = &audio_buffer_info;
  BaseType_t ret;

  /* if not enough wait for the blocks to fill */
  if(count < audio->outputQ_count)
  {
   return 0;
  }
  /* can not send to null Queue, so set the state to default release */
  if(audio->audiobuffer_outputQ_handle == NULL)
  {
    audio->buffer_hold_status = AUDIO_BUFFER_STATUS_BUFFERING;
   return 0;
  }
  /* send datablocks to the back or front of Queue */
  for(int i= 0; i< audio->outputQ_count;i++)
  {
     QAI_DataBlock_t *pBlock = audio->audio_data_blocks[audio->array_rd_index];
     
     /* change the process ID and add to the Queue. Should not release the block */
     pBlock->dbHeader.pid = AUDIO_CIRCULAR_BUFFER_PID;
     
     //since the DataBlock will be released by the recipient, do not keep the
     //address else it will try to release when emptying
     audio->audio_data_blocks[audio->array_rd_index] = NULL;
     
     /* ignore the return value when adding to the queue with 0 delay */
     if(audio->outputQ_mode == CIRCULARBUFFER_OUTQ_MODE_FRONT)
       ret = xQueueSendToFront(audio->audiobuffer_outputQ_handle, &pBlock, 0);
     else
       ret = xQueueSendToBack(audio->audiobuffer_outputQ_handle, &pBlock, 0);
     
     /* if the queue is full then stop */
     if(ret != pdTRUE)
     {
      configASSERT(0);
       break;
     }
     /* Decrease count of held blocks, and increase the count of sent blocks */
     audio->array_block_count--;
     count_actual++;
     /* increment and wrap around the index */
     audio->array_rd_index++;
     if(audio->array_rd_index >= audio->array_hold_max)
       audio->array_rd_index = 0;
     
     /* should not happen: do not exceed available blocks */
     /* Chalil - looks like this case can come. isn't ok as rd_idx never cross the wr_index ? */
     if(audio->array_rd_index == audio->array_wr_index)
     {
       break;
     }
  }
  return count_actual;
}
/*
 * The current filled block count can be queried any time.
*/
int get_audiobuffer_filled_block_count(void)
{
  audio_blocks_info_t *audio = &audio_buffer_info;
  return audio->array_block_count;
}
/* 
 * Save new block in the Array.
 * Check if the buffer is full. Then Release the last block.
 * If transmiting mode, call the registered function.
 */
static void audiobuffer_save_block(QAI_DataBlock_t *pIn)
{
  int count_actual = 0;
  audio_blocks_info_t *audio = &audio_buffer_info;

  /* first put the block in the array */
  if ((audio->array_block_count == 0) || (audio->array_wr_index != audio->array_rd_index))
  {
      audio->audio_data_blocks[audio->array_wr_index++] = pIn;
      if(audio->array_wr_index >= audio->array_hold_max)
        audio->array_wr_index = 0;
      audio->array_block_count++;
  }

  /* get available block count */
  int count = get_audiobuffer_filled_block_count();

  switch(audio->buffer_hold_status)  {
    case AUDIO_BUFFER_STATUS_DISABLE:
      /* no point holding blocks if not saving */
      release_used_blocks(audio, count);
      audio->array_wr_index = 0;
      audio->array_rd_index = 0;
      break;
      
    case AUDIO_BUFFER_STATUS_TRANSMIT:
      /* release the requested number of blocks when available to a callback function */
      transmit_to_callback(count);
      break;

    case AUDIO_BUFFER_STATUS_OUTPUT2Q:
      /* release the requested number of blocks when available to a output Queue */
      count_actual = transmit_to_outputQ(count);
      configASSERT((count_actual == 0) || (count_actual == audio_buffer_info.outputQ_count));
      break;
     
    case AUDIO_BUFFER_STATUS_BUFFERING:
    default:
      /* Fall through Case: Will release the oldest always */
      /* if the read and write pointers match release the last buffer */
      if(audio->array_wr_index == audio->array_rd_index)
      {
        release_used_blocks(audio, 1);
      } 
     break;
  }

  return;
}
/* 
 * Only data block instantiating function in main knows the maximum blocks.
 *
 * Tim: I think this is overly complex.  Just let the desginer say how many blocks
 *      the CB should hold.
 *
 * //The total number of Audio Blocks are input. 
 * //The circular buffer holds 10 less blocks than the max available.
 */
void set_audiobuffer_hold_count(int hold_count, int enable)
{
    audio_blocks_info_t *audio = &audio_buffer_info;
    //total_count = total_count - MIN_AUDIO_SAVE_BLOCKS_SPARE_SIZE;
    if(hold_count > MAX_AUDIO_SAVE_BLOCKS_ARRAY_SIZE)
      hold_count = MAX_AUDIO_SAVE_BLOCKS_ARRAY_SIZE;
    
    /* if first time need aquire the semaphore */
    if(audio_circbuffer_sem == NULL)
    {
      audio_circbuffer_sem = xSemaphoreCreateBinary();
      /* if cannot get semphore must fix it */
      configASSERT(audio_circbuffer_sem);
      
      /* or disable it */
      if(audio_circbuffer_sem == NULL)
      {
        audio->buffer_hold_status = AUDIO_BUFFER_STATUS_DISABLE;
        return;
      }
      xSemaphoreGive(audio_circbuffer_sem);
    }
    
    /* take semaphore to prevent access */
    xSemaphoreTake(audio_circbuffer_sem, portMAX_DELAY);
    
    audio->array_hold_max = hold_count;
    reset_audiobuffer_blocks(&audio_buffer_info, MAX_AUDIO_SAVE_BLOCKS_ARRAY_SIZE);
    /* prevent misuse of circular buffer function */
    if(hold_count < MIN_AUDIO_SAVE_BLOCKS_SPARE_SIZE)
    {
      audio->buffer_hold_status = AUDIO_BUFFER_STATUS_DISABLE;
      
      /* done with changes so release*/
      xSemaphoreGive(audio_circbuffer_sem);
      return;
    }
    /* set the state */
    if(enable)
      audio->buffer_hold_status = AUDIO_BUFFER_STATUS_BUFFERING;
    else
      audio->buffer_hold_status = AUDIO_BUFFER_STATUS_DISABLE;
         
    /* done with changes so release*/
    xSemaphoreGive(audio_circbuffer_sem);
    
    return;
}
/* 
 * Circular buffer can be enabled or disabled any time.
 * When disabled must release if any blocks are held.
*/
void disable_audiobuffer_hold(void)
{
  audio_blocks_info_t *audio = &audio_buffer_info;

  /* take semaphore to prevent access */
  xSemaphoreTake(audio_circbuffer_sem, portMAX_DELAY);
  audio->buffer_hold_status = AUDIO_BUFFER_STATUS_DISABLE;
  
  /* release all the blocks*/
  reset_audiobuffer_blocks(&audio_buffer_info, MAX_AUDIO_SAVE_BLOCKS_ARRAY_SIZE);

  /* done with changes so release*/
  xSemaphoreGive(audio_circbuffer_sem);

  return;
}
/* 
 * Circular buffer can be enabled or disabled any time.
 */
void enable_audiobuffer_hold(void)
{
  audio_blocks_info_t *audio = &audio_buffer_info;
  /* take semaphore to prevent access */
  xSemaphoreTake(audio_circbuffer_sem, portMAX_DELAY);

  audio->buffer_hold_status = AUDIO_BUFFER_STATUS_BUFFERING;
  /* done with changes so release*/
  xSemaphoreGive(audio_circbuffer_sem);

  return;
}
/* 
 * Circular buffer blocks can be released to a call back function.
 * Else, by default they will be released to the datablock pool when full.
 * The Transmit function can request to be called with multiple blocks
 * per transmission. (Useful for OPUS encoder which requires multiple
 * Data blocks to produce one encoded Block)
*/
void enable_transmit_audiobuffer(void (*transmit_callback)(QAI_DataBlock_t *pBlock), int transmit_count, Circularbuffer_TRANSMIT_mode_t e_mode )
{
  audio_blocks_info_t *audio = &audio_buffer_info;
  
  /* valid only for non-zero callback function handle */
  if(transmit_callback)
  {
    /* take semaphore to prevent access */
    xSemaphoreTake(audio_circbuffer_sem, portMAX_DELAY);

    audio->buffer_hold_status = AUDIO_BUFFER_STATUS_TRANSMIT;
    audio->transmit_callback = transmit_callback;
    audio->transmit_count = transmit_count;
    audio->transmit_mode = e_mode;

    /* done with changes so release*/
    xSemaphoreGive(audio_circbuffer_sem);
  }
  return;
}
/* 
 * Circular buffer blocks can be released to a Queue.
 * Else, by default they will be released to the datablock pool when full.
 * The output to a Queue request has to setup the Queue to be sent.
 * The same input Audio Block received by the circular buffer will be sent to 
 * Queue with only the ID changed. 
 *
 * So, it is the responsibility of the receiver to release the datablock.
 *
 * Note: The output Q must not be Input Q when the out_count is greater than 1.
 * For more than 1, the Blocks must be sent to the Back of the Queue.
 * For output count =1, the datablock can be sent to the Front or Back of the Queue
*/
void enable_output2Q_audiobuffer(QueueHandle_t outQ_handle, int out_count, Circularbuffer_outQ_mode_t queue_mode )
{
  audio_blocks_info_t *audio = &audio_buffer_info;
  

  /* valid only for non-zero handle */
  if(outQ_handle)
  {
    /* take semaphore to prevent access */
    xSemaphoreTake(audio_circbuffer_sem, portMAX_DELAY);
    
    audio->buffer_hold_status = AUDIO_BUFFER_STATUS_OUTPUT2Q;
    audio->audiobuffer_outputQ_handle = outQ_handle;
    
    /* if the mode is to send the block to the front of the Q, then only 1 block can be released */
    if(queue_mode == CIRCULARBUFFER_OUTQ_MODE_FRONT)
    {
      audio->outputQ_count = 1;
      audio->outputQ_mode = CIRCULARBUFFER_OUTQ_MODE_FRONT;
    }
    else
    {
      audio->outputQ_count = out_count;
      audio->outputQ_mode = CIRCULARBUFFER_OUTQ_MODE_BACK;
    }
    
    /* done with changes so release*/
    xSemaphoreGive(audio_circbuffer_sem);

  }
  return;
}
/* 
 * Truncates the Circular buffer blocks beyond the a requested count.
 * The requested count will be the max remaining blocks after the truncation. 
 * If there are less than the requested, nothing is removed.
 * Always returns the number of remaining datablocks in the buffer.
 */
int truncate_audio_circularbuffer(int num_blocks)
{
  audio_blocks_info_t *audio = &audio_buffer_info;
  
  /* take semaphore to prevent access */
  xSemaphoreTake(audio_circbuffer_sem, portMAX_DELAY);

  /* get available block count */
  int count = get_audiobuffer_filled_block_count();

  /* release the rest only if more than the requested are in the buffer */
  int release_count = count - num_blocks;
  if(release_count > 0)
  {
    release_used_blocks(audio, release_count);
  }
  count = get_audiobuffer_filled_block_count();

  /* done with changes so release*/
  xSemaphoreGive(audio_circbuffer_sem);

  return count;
}
/* 
 * Datablock processor for Audio Circular buffer.
 */
void audio_circularbuffer_processor(

       QAI_DataBlock_t *pIn,
       QAI_DataBlock_t *pOut,
       QAI_DataBlock_t **pRet,
       datablk_pe_event_notifier_t *pevent_notifier
     )
{
#if 0 //only for debug 
  static int init_hold_count = 0;
  if(!init_hold_count)
  {
    set_audiobuffer_hold_count(MAX_AUDIO_SAVE_BLOCKS_ARRAY_SIZE, 1);
    init_hold_count++;
  }
  //for debug: 
   pIn->dbHeader.numUseCount++;

#endif  
    /* take semaphore to prevent access */
    xSemaphoreTake(audio_circbuffer_sem, portMAX_DELAY);
    
    *pRet = NULL;  
    /* the input may be from different sources, ISR, NS, I2S etc. */
    audiobuffer_save_block(pIn);

    /* done with changes so release*/
    xSemaphoreGive(audio_circbuffer_sem);
    
    return;
}
