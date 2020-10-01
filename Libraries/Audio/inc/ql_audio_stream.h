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

#ifndef QL_AUDIO_STREAM__H
#define QL_AUDIO_STREAM__H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t t_pcm_sample;

typedef struct t_ql_audio_stream_
{
  t_pcm_sample *p_buffer; /* PCM buffer pointer, inter-leaved or strided for multichannel case */
  int16_t n_channels;     /* number of channels                                                */
  int16_t n_frame_sz;     /* number of sampels in the buffer per channel                       */
  int16_t n_stride;       /* to be used for multi-channel case, = n_channels if interleaved    */
  int16_t reserved;       /* not used                                                          */
}t_ql_audio_stream;



/* for test purpose */
#define WAV_HDR_SZ       (44)

typedef int32_t t_stream_id;
typedef enum
{
  e_mode_none,
  e_mode_read,
  e_mode_write
}t_stream_mode_e;

t_ql_audio_stream* stream_new(int16_t *p_mem);
t_stream_id stream_create(char *file, int32_t channels, int32_t block_sz, t_stream_mode_e e_mode);
int32_t stream_delete(t_stream_id stream_id);
int32_t stream_get(t_stream_id stream_id, t_ql_audio_stream *p_stream);
int32_t stream_put(t_stream_id stream_id, t_ql_audio_stream *p_stream);

#ifdef __cplusplus
}
#endif

#endif /* QL_AUDIO_STREAM__H */

