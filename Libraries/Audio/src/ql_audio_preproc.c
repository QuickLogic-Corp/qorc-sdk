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

/******************************************************************************/
/* Free RTOS Task specific to Audio processing.                               */
/* File Name : ql_audio_preproc.c                                             */
/* This file contains audio pre-processing high-level function implementation */
/*      -                                                                     */
/*      -                                                                     */
/*      -                                                                     */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ql_audio_preproc.h"
#include "ql_util.h"

#ifndef UNIT_TEST
SemaphoreHandle_t  ql_pre_proc_sem;
#endif
e_ql_audio_preproc_status ql_preproc_bypass1_process(t_ql_audio_stream * p_mic_input, t_ql_audio_stream * p_ref_input, t_ql_audio_stream *p_audio_output)
{
    t_pcm_sample *p_mic_left = p_mic_input->p_buffer;
    t_pcm_sample *p_output = p_audio_output->p_buffer;

    for(int i = 0; i < p_mic_input->n_frame_sz; i++){
      p_output[i] = p_mic_left[i];
  }

  p_audio_output->n_channels = 1;
  p_audio_output->n_frame_sz = p_mic_input->n_frame_sz;
  p_audio_output->n_stride = p_mic_input->n_frame_sz;

   return e_ql_audio_preproc_status_ok;
}
e_ql_audio_preproc_status ql_preproc_bypass2_process(t_ql_audio_stream * p_mic_input, t_ql_audio_stream * p_ref_input, t_ql_audio_stream *p_audio_output)
{
    t_pcm_sample *p_mic_right = p_mic_input->p_buffer;
    t_pcm_sample *p_output = p_audio_output->p_buffer;
    assert(p_mic_input->n_channels ==2 );

    for(int i = 0; i < p_mic_input->n_frame_sz; i += p_mic_input->n_channels){
      p_output[i] = p_mic_right[i + p_mic_input->n_stride];
   }

  p_audio_output->n_channels = 1;
  p_audio_output->n_frame_sz = p_mic_input->n_frame_sz;
  p_audio_output->n_stride = p_mic_input->n_frame_sz;

   return e_ql_audio_preproc_status_ok;
}
e_ql_audio_preproc_status ql_preproc_bypass3_process(t_ql_audio_stream * p_mic_input, t_ql_audio_stream * p_ref_input, t_ql_audio_stream *p_audio_output)
{
    t_pcm_sample *p_reference = p_ref_input->p_buffer;
    t_pcm_sample *p_output = p_audio_output->p_buffer;

    for(int i = 0; i < p_ref_input->n_frame_sz; i++){
      p_output[i] = p_reference[i];
  }

  p_audio_output->n_channels = 1;
  p_audio_output->n_frame_sz = p_ref_input->n_frame_sz;
  p_audio_output->n_stride = p_ref_input->n_frame_sz;

  return e_ql_audio_preproc_status_ok;
}

e_ql_audio_preproc_status ql_preproc_bypass_mix_left_ref_process(t_ql_audio_stream * p_mic_input, t_ql_audio_stream * p_ref_input, t_ql_audio_stream *p_audio_output)
{
    t_pcm_sample *p_mic_left = p_mic_input->p_buffer;
    t_pcm_sample *p_reference = p_ref_input->p_buffer;
    t_pcm_sample *p_output = p_audio_output->p_buffer;

    for(int i = 0; i < p_ref_input->n_frame_sz; i++){
      p_output[i] = p_reference[i] + p_mic_left[i];
  }

  p_audio_output->n_channels = 1;
  p_audio_output->n_frame_sz = p_ref_input->n_frame_sz;
  p_audio_output->n_stride = p_ref_input->n_frame_sz;

  return e_ql_audio_preproc_status_ok;
}

e_ql_audio_preproc_status ql_preproc_bypass_mix_left_right_process(int16_t * p_mic_left, int16_t * p_mic_right, int16_t *p_output, int n_frame_sz)
{
    for(int i = 0; i < n_frame_sz; i++){
      p_output[i] = p_mic_left[i] + p_mic_right[i];
    }

  return e_ql_audio_preproc_status_ok;
}

t_audio_preproc_info o_audio_preproc_info = {.bypass = 0};
t_audio_preproc_info *ql_audio_preproc_init(t_ql_audio_preproc_option_e e_option)
{

  e_ql_audio_preproc_status status = e_ql_audio_preproc_status_ok;

   o_audio_preproc_info.fs       = 16000;
   o_audio_preproc_info.frame_sz = 120;
   o_audio_preproc_info.channels = 2;
  o_audio_preproc_info.e_status = status;
  return &o_audio_preproc_info;
}

#ifdef DEBUG_PRE_PROC
/**************************************************************************************************/
/* FOR DEBUGGING AW : if this macros is enabled, the input to AW schematics will be igonred and   */
/* a sine tone of 1000Hz will be taken as input. The amplitude is 50% or -3dBFS                   */
/* to create the table in excel use the following formula                                         */
/* =ROUND(2^14*SIN(2*PI()*1000*(ROW()-1)/16000),0)                                                */
/**************************************************************************************************/
int16_t SineTable_test[240] = {
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271
};

#endif
//#define MEASURE_PRE_PROC_MIPS 

#ifdef MEASURE_PRE_PROC_MIPS
#define MIPS_AVE_COUNT (200)
int a_mips_pre_proc[MIPS_AVE_COUNT];
int a_mips_pre_proc_count = 0;

#endif






e_ql_audio_preproc_status ql_audio_preproc_close(void)
{
  e_ql_audio_preproc_status status = e_ql_audio_preproc_status_ok;


  return status;
}

static void reset_stereo_blk_count(void);

void datablk_pe_config_ql_pre_process(void *p_pe_object)
{
  ql_pre_proc_sem = xSemaphoreCreateBinary(  );

  ql_audio_preproc_init(e_ql_audio_preproc_option_default);

  reset_stereo_blk_count();
 
  xSemaphoreGive(ql_pre_proc_sem);
  
  return ;
}

///DataBLK is 240 samples = 120 stereo samples
// => 2 stero DataBlocks = 1 Mono DSP output Block
//So we produce output for alternative buffers based on input_stereo_blk_count
// if it is 0 no output, 1 there is 1 datablock output
uint16_t stereo_stride_buffer[2*240]; // 2*240 keep room to collect 1 Stereo blocks
int input_stereo_blk_count = 0;
 //Note: blk_size is assumed to be 240 samples for this function to work
uint16_t *fill_n_get_stride_buffer(uint16_t *p_interleaved_buf, int blk_size, int blk_count)
{
  int i = 0; //left channel start from 0
  int j = blk_size;//right channel starts from midway = 240
  blk_size = blk_size >> 1; //get stereo sample count = 120
  if(blk_count)
  {
    //if one stereo DataBlock is filled increment the start points
    i += blk_size;//+= 120
    j += blk_size;//+= 120
  }

#if (EN_STEREO_DUAL_BUF == 1) //first half left channel and the next half right channel

  memcpy(&stereo_stride_buffer[i], p_interleaved_buf, sizeof(uint16_t)*blk_size);
  p_interleaved_buf += blk_size;
  memcpy(&stereo_stride_buffer[j], p_interleaved_buf, sizeof(uint16_t)*blk_size);

#else // interleaved samples
  for(int k = 0; k < blk_size; k++)
  {
    stereo_stride_buffer[i++] = *p_interleaved_buf++;
    stereo_stride_buffer[j++] = *p_interleaved_buf++;
  }
#endif

  //we wait for 2 input blocks
  if(blk_count & 0x1) {
    return stereo_stride_buffer;
  }  else {
    return NULL;
  }
}
static void reset_stereo_blk_count(void)
{
  input_stereo_blk_count = 0;
}

void datablk_pe_process_ql_pre_process(QAI_DataBlock_t *pIn, QAI_DataBlock_t *pOut, QAI_DataBlock_t **pRet,
                                 void (*p_event_notifier)(int pid, int event_type, void *p_event_data, int num_data_bytes))
{
  e_ql_audio_preproc_status ret = e_ql_audio_preproc_status_ok;

     uint16_t *pInbuf;
     pInbuf = fill_n_get_stride_buffer((uint16_t*)&pIn->p_data, pIn->dbHeader.numDataElements, input_stereo_blk_count);
     input_stereo_blk_count++;
     if(pInbuf == NULL) {
       if(pOut) {
         //have to make sure usecount is at least 1
         if (pOut->dbHeader.numUseCount == 0)
           pOut->dbHeader.numUseCount = 1;
         datablk_mgr_release_generic(pOut);
       }
       *pRet = NULL;
       return;
     }
     //We can produce 1 DataBlock of Mono size 240 samples from BF
     input_stereo_blk_count = 0; //reset stereo block count

     int16_t *p_left = (int16_t *)pInbuf;
     int16_t *p_right = p_left + pOut->dbHeader.numDataElements;
     ql_preproc_bypass_mix_left_right_process(p_left, p_right, (int16_t*)pOut->p_data, pOut->dbHeader.numDataElements);

     /* return the output */
     *pRet =  pOut;
     return ;
}

