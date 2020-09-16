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

#ifndef __AUDIO_RAW_STREAM__
#define __AUDIO_RAW_STREAM__

#include "datablk_mgr.h"

/* Creates a Queue to hold DataBlocks. get the Queuehandle using the API.
 * Also can set a period for call back timer which can be started when need
 * Note: calling with a period 0 or less will not create a timer.
 */
extern void init_audio_raw_stream(int samples_per_data_block, int callback_timer_period_ms);
/* allow change of number of samples per block at a later time */
extern void set_audio_raw_block_samples(int samples_per_data_block);
/*
* This module can be used to set the callback function when at least the required
* amount of data is available in the Raw stream queue.
* Note: If the data is not taken fast enough, there will be 1 callback for each Timer period.
*/
extern void set_raw_data_ready_callback(void (*data_ready_callback)(int required_buf_size), int callback_sample_count);

/* get the Queue handle through API for external functions */
extern QueueHandle_t get_audio_stream_queue(void);

/* From non-ISR: Fill the required number of audio raw sample in the given buffer. 
   If the required number is not available returns 0.
*/
extern int get_audio_stream_data(int16_t *buf, int sample_count);
/* From ISR: Fill the required number of audio raw sample in the given buffer. 
   If the required number is not available returns 0.
*/
extern int get_audio_stream_dataFromISR(int16_t *buf, int sample_count);

/*
* This module releases all the currently held datablocks.
*/
extern void stop_audio_raw_stream(void);
/* return available raw samples in the queue */
extern int get_raw_audio_sample_count(void);
/* return available raw samples in the queue from ISR */
extern int get_raw_audio_sample_countFromISR(void);


#endif /* __AUDIO_RAW_STREAM__ */
