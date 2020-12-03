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

#if (FEATURE_OPUS_ENCODER == 1)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "FreeRTOS.h"
#include "task.h"
#include "ql_opus.h"

/* These should be defined in the libary headers */
/* All for 20ms Opus Buffer/frame size */
#define OPUS_SAMPLES_PER_FRAME       (320)
#define OPUS_ENCODED_BYTES_PER_FRAME (88)

#define SZ_QL_MEM_BLOCK1  (9*1024)
#define SZ_QL_MEM_BLOCK2  (27*1024)

#define E_QL_OPUS_ENC_OPTION E_QL_OPUS_ENCODER_OPTION4

/* opus encoder state and buffers */
static ql_opus_encoder_t *p_ql_opus_encoder;
static char opus_mem_buff_1[SZ_QL_MEM_BLOCK1];
static char opus_mem_buff_2[SZ_QL_MEM_BLOCK2];

/*
 * This should be called every time a new Audio Stream is available.
 * Note: Whenever there is a break in the Audio stream due to waking up
 * from sleep (due to LPSD) and detecting a KP, this should be initialized
*/
void init_opus_encoder(void)
{
  /* create opus encoder */
  ql_opus_set_mem(opus_mem_buff_1, opus_mem_buff_2, SZ_QL_MEM_BLOCK1, SZ_QL_MEM_BLOCK2);

  /* init the encoder */
  p_ql_opus_encoder = ql_opus_init(E_QL_OPUS_ENC_OPTION);

  return;
}
/* 
* This function Encodes the raw samples using Opus Encoder. 
* The number of input samples are expected to be exact multiple of Opus Frame Size
* For each 320 samples (= Opus Frame Size), 80 bytes of encoded data + 8 bytes of
* Header is produced.
* Each Data Block is 240 samples (15ms fixed). So 4 Data Blocks worth of samples 
* (=60ms = 960samples) are passed in. Then output should always be 
*               = (960/320)*(80+8) = 3*88 = 264 bytes
*
* Returns the number of encoded bytes copied into the output buffer
*/
int32_t get_opus_encoded_data(int16_t *p_pcm_data, int32_t n_samples_in, uint8_t *p_encoded_buffer)
{
  uint8_t *p_out = p_encoded_buffer;
  int16_t *p_in = p_pcm_data;
  int32_t encoded_byte_count = 0;
  int32_t samples_consumed = 0;
  int32_t n_in = OPUS_SAMPLES_PER_FRAME;
  int32_t n_out = 0;
  int32_t n_frames = n_samples_in/OPUS_SAMPLES_PER_FRAME; 

  //input must be exact multiple of opus frame size
  configASSERT(n_samples_in == OPUS_SAMPLES_PER_FRAME*n_frames);

  for(int frame = 0; frame < n_frames; frame++) {
    e_ql_opus_encoder_status_t  ret = ql_opus_encode(p_ql_opus_encoder, p_in, n_in, 1, p_out, &n_out, &samples_consumed);
    
    //should not return an error and must consume exact number of samples always
    configASSERT(ret == E_QL_OPUS_ENCODER_STATUS_OK);
    configASSERT(n_in == samples_consumed);

    encoded_byte_count += n_out;

    p_out += n_out;
    p_in  += n_in;
  }
  
  //the final number of encoded bytes must match the number frames processed
  configASSERT(encoded_byte_count == OPUS_ENCODED_BYTES_PER_FRAME*n_frames);
  
  return encoded_byte_count;  
}

#if 0 
//these should override the math.h functions
double floor(double X)
{
  return (float)floorf(X);
}
double sin(double v)
{
  return (double)sinf(v);
}
double cos(double v)
{
  return (double)cosf(v);
}
double fabs(double X)
{
  return (float)fabsf(X);
}
double sqrt(double X)
{
  return (float)sqrtf(X);
}
double log(double X)
{
  return (float)logf(X);
}
#endif


#endif // (FEATURE_OPUS_ENCODER == 1)