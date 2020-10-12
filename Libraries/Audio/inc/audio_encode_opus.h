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

#ifndef __AUDIO_ENCODE_OPUS__
#define __AUDIO_ENCODE_OPUS__

#include "datablk_mgr.h"

#define SAMPLES_PER_FRAME (320)
#define ENCODED_BYTES_PER_FRAME (88)
#define OPUS_FRAMES       (3)
#define OPUS_BUFFER_IN_SZ     (SAMPLES_PER_FRAME*OPUS_FRAMES)
#define OPUS_BUFFER_OUT_SZ    (ENCODED_BYTES_PER_FRAME*OPUS_FRAMES)
/*
* This module returns the opus output Block size in bytes and block time in msecs.
* It is expected to be used to estimate the number of interrupts and hence the
* buffer size to transmit to host at a time.
*/
extern void get_opus_out_block_size(int *block_size_bytes, int *block_time_msec);
/*
* This module gives the available Opus Encoded bytes.
* It can be used before calling get_opus_enc_data to get the buffer.
*/
extern int check_opus_enc_data(void);
/*
* Get the Opus encoded data in the passed in buffer. If required amount of data 
* is not available, only available dta will be returned.
*
* Note: This module can not detect if there is buffer overflow 
*/
extern int get_opus_enc_data(uint8_t *enc_output, int request_size);

/* 
 * Datablock config for Opus Audio Encoder.
 */
extern void audio_enc_config(void *p_pe_object);
//extern void audio_ql_cbuf_transmit_callback_opus(QAI_DataBlock_t *pdata_block_in);
extern void ql_cbuf_transmit_callback_audio_enc(QAI_DataBlock_t *pdata_block_in);
extern void ql_cbuf_transmit_callback_audio_raw(QAI_DataBlock_t *pdata_block_in);
extern outQ_processor_t audio_isr_outq_processor ;
void audio_encode_reset(void);

int32_t encode_audio_blocks_opus(QAI_DataBlock_t *pdata_block_in, QAI_DataBlock_t **pdata_block_out);

#endif /* __AUDIO_ENCODE_OPUS__ */
