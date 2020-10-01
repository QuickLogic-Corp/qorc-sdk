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

#include "datablk_mgr.h"

/*
* The datablock can be sent to the Back or Front of the Queue
* When Front is chosen, only 1 datablock can be output at time
* The mode is set to Back of the Queue, the output count must
* be less than the Queue size
*/
typedef enum Circularbuffer_outQ_mode {
  CIRCULARBUFFER_OUTQ_MODE_BACK, // FIFO mode
  CIRCULARBUFFER_OUTQ_MODE_FRONT, // LIFO mode
} Circularbuffer_outQ_mode_t;

/*
* use CIRCULARBUFFER_TRANSMIT_MODE_RELEASE mode to enable automatic release.
* CIRCULARBUFFER_TRANSMIT_MODE_NO_RELEASE mode will not release the block, the callback
* shall ensure that the Block is released outside.
* 
*/
typedef enum Circularbuffer_transmit_mode {
  CIRCULARBUFFER_TRANSMIT_MODE_NO_RELEASE = 0,  
  CIRCULARBUFFER_TRANSMIT_MODE_RELEASE
} Circularbuffer_TRANSMIT_mode_t;

/*
 * The current filled block count can be queried any time.
*/
extern int get_audiobuffer_filled_block_count(void);
/* 
 * Only data block instantiating function in main knows the maximum blocks.
 * The total number of Audio Blocks are input. 
 * The circular buffer holds 10 less blocks than the max available.
 */
extern void set_audiobuffer_hold_count(int total_count, int enable);
/* 
 * Circular buffer can be enabled or disabled any time.
 * When disabled must release if any blocks are held.
*/
extern void disable_audiobuffer_hold(void);
/* 
 * Circular buffer can be enabled or disabled any time.
 */
extern void enable_audiobuffer_hold(void);
/* 
 * Circular buffer blocks can be released to a call back function.
 * Else, by default they will be released to the datablock pool when full.
 * The Transmit function can request to be called with multiple blocks
 * per transmission. (Useful for OPUS encoder which requires multiple
 * Data blocks to produce one encoded Block)
*/
extern void enable_transmit_audiobuffer(void (*transmit_callback)(QAI_DataBlock_t *pBlock), int transmit_count, Circularbuffer_TRANSMIT_mode_t e_mode );
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
extern void enable_output2Q_audiobuffer(QueueHandle_t outQ_handle, int out_count, Circularbuffer_outQ_mode_t queue_mode );
/* 
 * Truncates the Circular buffer blocks beyond the a requested count.
 * The requested count will be the max remaining blocks after the truncation. 
 * If there are less than the requested, nothing is removed.
 * Always returns the number of remaining datablocks in the buffer.
 */
extern int truncate_audio_circularbuffer(int num_blocks);
/* 
 * Datablock processor for Audio Circular buffer.
 */
extern void audio_circularbuffer_processor(
       QAI_DataBlock_t *pIn,
       QAI_DataBlock_t *pOut,
       QAI_DataBlock_t **pRet,
       datablk_pe_event_notifier_t *pevent_notifier
     );

//TIM
struct CircularBuffer_ConfigData {
    int                             hold_count;
    // Parameters for enable_transmit_audiobuffer
    void (*transmit_callback)(QAI_DataBlock_t *pBlock);
    int                             transmit_count;
    Circularbuffer_TRANSMIT_mode_t  e_mode;
    // Parameters for enable_output2Q_audiobuffer
    QueueHandle_t*                  outQ_handle;
    int                             out_count;
    Circularbuffer_outQ_mode_t      queue_mode;
};
