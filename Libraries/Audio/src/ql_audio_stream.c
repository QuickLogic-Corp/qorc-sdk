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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <string.h>
#include <stdint.h>

#include "Fw_global_config.h"

#include "ql_util.h"
#include "ql_audio_preproc.h"
#include "ql_audio_stream.h"

typedef struct t_stream_info_
{
  FILE *fp;
  t_stream_mode_e e_mode;
  int32_t file_sz;
  int32_t channels;
  int32_t block_sz;
  int32_t stride;
}t_stream_info;

#define MAX_STREAMS    (3)
#define NUM_MIC_CHANNELS 1

#define BRICK_SIZE_SAMPLES_MAX 240
#define AW_BLOCK_SIZE_SAMPLES  80
int16_t  temp_in[BRICK_SIZE_SAMPLES_MAX*NUM_MIC_CHANNELS];

t_stream_info ao_stream_info[MAX_STREAMS];

t_stream_id stream_count_curr = 0;



t_stream_id stream_create(char *file, int32_t channels, int32_t block_sz, t_stream_mode_e e_mode);
int32_t stream_delete(t_stream_id stream_id);
int32_t stream_get(t_stream_id stream_id, t_ql_audio_stream *p_stream);
int32_t stream_put(t_stream_id stream_id, t_ql_audio_stream *p_stream);

/* create an instance of stream, allocate the mem for the struct and then the buffer from p_mem */
t_ql_audio_stream* stream_new(int16_t *p_mem)
{
  t_ql_audio_stream *p_stream = (t_ql_audio_stream*) p_mem;
  p_stream->p_buffer = (uint16_t*)(p_mem + sizeof(t_ql_audio_stream)/(sizeof(int16_t)));

  return p_stream;
}


t_stream_id stream_create(char *file, int32_t channels, int32_t block_sz, t_stream_mode_e e_mode)
{
   t_stream_id stream_id = -1;
   if(stream_count_curr < MAX_STREAMS)
   {
     stream_id = stream_count_curr;

      if(e_mode_read == e_mode)
      {
        ao_stream_info[stream_id].fp = fopen(file, "rb");
        if(ao_stream_info[stream_id].fp != NULL){
          fseek(ao_stream_info[stream_id].fp,0,SEEK_END);
          int32_t file_sz = ftell(ao_stream_info[stream_id].fp) / sizeof(int16_t);
          rewind(ao_stream_info[stream_id].fp);
          ao_stream_info[stream_id].file_sz = file_sz;
          printf("\n File open success for %s, total PCM samples = %d", file, file_sz-WAV_HDR_SZ/2);
          if(1){
            int hdr_sz = WAV_HDR_SZ;
            int n1 = fread(temp_in, 1, hdr_sz, ao_stream_info[stream_id].fp);
           if(n1 =! hdr_sz)
           {
             printf("ERROR : error in reading file %s", file);
             stream_id = -1; // return invalid file id
           }
          }
        }
        else{
          printf("ERROR : Open failed for file %s\n", file);
          stream_id = -1; // return invalid file id
        }
      }
      else
      {
        ao_stream_info[stream_id].fp = fopen(file, "wb");
        ao_stream_info[stream_id].file_sz = 0;
      }
      if(stream_id >= 0)
      {
        ao_stream_info[stream_id].e_mode = e_mode;
        ao_stream_info[stream_id].block_sz = block_sz;
        ao_stream_info[stream_id].channels = channels;
        ao_stream_info[stream_id].stride = block_sz; // stride is same as block size, here
      }
   }

   stream_count_curr++;

   return stream_id;
}

int32_t stream_delete(t_stream_id stream_id)
{
  if(ao_stream_info[stream_id].fp != NULL)
  {
    fclose(ao_stream_info[stream_id].fp);
    printf("\n Closing file id = %d, the current size is : %d\n", stream_id, ao_stream_info[stream_id].file_sz);

    ao_stream_info[stream_id].fp = NULL;
    ao_stream_info[stream_id].e_mode = e_mode_none;
    ao_stream_info[stream_id].block_sz = -1;

  }
  else
  {
    return -1;
  }
  return 0;
}
int32_t stream_get(t_stream_id stream_id, t_ql_audio_stream *p_stream)
{
  int count = ao_stream_info[stream_id].block_sz * ao_stream_info[stream_id].channels;
  if(ao_stream_info[stream_id].fp != NULL)
  {
    int i = 0;
        int n1 = fread(temp_in, sizeof(int16_t), count, ao_stream_info[stream_id].fp);
        if(n1 != count)
        {
          count = n1;
        }
        for(int s = 0; s < count; s++)
        { // pcm file is channel interleaved
          for(int ch = 0; ch < ao_stream_info[stream_id].channels; ch++){
             p_stream->p_buffer[s + ao_stream_info[stream_id].stride * ch] = temp_in[i++];
          }
        }
        p_stream->n_channels = ao_stream_info[stream_id].channels;
        p_stream->n_frame_sz = ao_stream_info[stream_id].block_sz;
        p_stream->n_stride = ao_stream_info[stream_id].stride;
        ao_stream_info[stream_id].file_sz -= count;
  }else{
        p_stream->n_channels = 0;
        p_stream->n_frame_sz = 0;
        p_stream->n_stride = 0;
  }
  return count;
}
int32_t stream_put(t_stream_id stream_id, t_ql_audio_stream *p_stream)
{
  int count = ao_stream_info[stream_id].block_sz * ao_stream_info[stream_id].channels;
  if((ao_stream_info[stream_id].fp != NULL) && (count > 0))
  {
        int n1 = fwrite(p_stream->p_buffer, sizeof(int16_t), count, ao_stream_info[stream_id].fp);
        if(n1 != count)
        {
          count = n1;
        }
        ao_stream_info[stream_id].file_sz += count;
  }
  else
  {
    count = -1;
  }
  return count;
}
