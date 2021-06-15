/******************************************************************************/
/* Copyright (C) 2019 QuickLogic Inc. All rights reserved                     */
/* FQL Wrapper for Consilient AEC                                             */
/* FileName : ql_consilient_aec_wrapper.c                                     */
/* Provided APIs to perform following audio processing    :                   */
/*      - init aec with required parameters and mem / buffer                  */
/*      - process aec for every frame                                         */
/******************************************************************************/

#include <stdbool.h>
#include <string.h>
#include "common.h"
#include "aec_api.h"
#include "ql_audio_stream.h"
#include "ql_audio_preproc.h"

#define AEC_SAMPLING_RATE      (16000)
#define AEC_DELAY_COMP_FACT    (90*(AEC_SAMPLING_RATE/1000))
#define AEC_ECHO_TAIL_LEN      (32)




/* should be taken from pool */
int32_t AECStateMemory[AEC_STATE_MEM_SIZE];
/* can be re-used with other libs ? */
int32_t AEC_Scratchmemory[AEC_SCRATCH_MEM_SIZE];

struct t_ql_consilient_aec_info
{
    int32_t SamplingRate;
    int32_t *pAECStateMemory;
    int32_t *pAEC_Scratchmemory;
    bool IsNSRequired;
    bool ECEnable;
    bool PPEnable;
    //    long AecMemSize;
    int32_t frame_size;
    int32_t delay_compasation;
    AEC_CTRL_PARAMS AECCtrlParams;
    AEC_RTCTRL_PARAMS AECRTCtrlParams ;
    NS_RTCTRL_PARAMS  NSRTCtrlParams ;


} o_ql_consilient_aec_info= {
  .pAECStateMemory = &AECStateMemory[0],
  .pAEC_Scratchmemory = &AEC_Scratchmemory[0],
  .IsNSRequired = true,
  .ECEnable = false,
  .PPEnable = false,
  .delay_compasation = AEC_DELAY_COMP_FACT,

  .AECCtrlParams = {
    .AECEnable = true,
    .HPFEnable = true,
    .SamplingRate = AEC_SAMPLING_RATE,
    .CNGEnable = 0, //1;
    .PTime = 10 , // 20mS , 15mS
    .BulkDelay = 0, //atoi(argv [7]) ;
    .CNGInitialLvl = 0 ,
    .CNGAdjustLvl = 1 ,
    .DTControl = 1 ,
  },


};

#define FRAME_SIZE_5MS_IN_SAMPLES 80  //80 samples per 5 ms @16000Hz
#define TOTAL_PHASE 2  //2 5ms frames = 10ms consilient processing frame.
static INT16 internal_output_buffer[FRAME_SIZE_5MS_IN_SAMPLES * TOTAL_PHASE];
static INT16 internal_input_buffer_mic[FRAME_SIZE_5MS_IN_SAMPLES * TOTAL_PHASE];
static INT16 internal_input_buffer_ref[FRAME_SIZE_5MS_IN_SAMPLES * TOTAL_PHASE];

e_ql_audio_preproc_status consilient_aec_process(t_ql_audio_stream * ptr_audio, t_ql_audio_stream * ptr_reference, t_ql_audio_stream *ptr_outBuf)
{
  static int phase = 0;
  e_ql_audio_preproc_status status = e_ql_audio_preproc_status_ok;
  int16_t *pRin = (int16_t*)ptr_reference->p_buffer;
  int16_t *pSin = (int16_t*)ptr_audio->p_buffer;
  int16_t *pSout = (int16_t*)ptr_outBuf->p_buffer;

  //Accumulate input into internal buffer.
  memcpy((internal_input_buffer_mic + (phase * FRAME_SIZE_5MS_IN_SAMPLES)), pSin, FRAME_SIZE_5MS_IN_SAMPLES * sizeof(INT16));
  memcpy((internal_input_buffer_ref + (phase * FRAME_SIZE_5MS_IN_SAMPLES)), pRin, FRAME_SIZE_5MS_IN_SAMPLES * sizeof(INT16));
  phase++;

  if (TOTAL_PHASE == phase) //if 10ms data is accumulated then call ns processing.
  {
    AEC_Process  (o_ql_consilient_aec_info.pAECStateMemory,
                  internal_input_buffer_ref,
                  internal_input_buffer_mic,
                  internal_output_buffer,
                  &o_ql_consilient_aec_info.AECRTCtrlParams,
                  &o_ql_consilient_aec_info.NSRTCtrlParams);
    phase = 0;
  }

  //Copy only 5ms data to output buffer from internal output buffer.
  memcpy(pSout, (internal_output_buffer + (phase * FRAME_SIZE_5MS_IN_SAMPLES)), FRAME_SIZE_5MS_IN_SAMPLES * sizeof(INT16));

  return status;
}

e_ql_audio_preproc_status consilient_aec_init(void *p)
{
  e_ql_audio_preproc_status status = e_ql_audio_preproc_status_ok;
  int mem_sz = 0;
    memset (&o_ql_consilient_aec_info.AECCtrlParams, 0, sizeof (AEC_CTRL_PARAMS)) ;
    memset (&o_ql_consilient_aec_info.AECRTCtrlParams, 0, sizeof (AEC_RTCTRL_PARAMS)) ;
    memset (&o_ql_consilient_aec_info.NSRTCtrlParams, 0, sizeof (NS_RTCTRL_PARAMS)) ;
    
    
    o_ql_consilient_aec_info.pAECStateMemory = &AECStateMemory[0];
    o_ql_consilient_aec_info.pAEC_Scratchmemory = &AEC_Scratchmemory[0];
    o_ql_consilient_aec_info.IsNSRequired = false; //true;
    o_ql_consilient_aec_info.ECEnable = false;
    o_ql_consilient_aec_info.PPEnable = false;
    o_ql_consilient_aec_info.delay_compasation = AEC_DELAY_COMP_FACT;

    o_ql_consilient_aec_info.AECCtrlParams.PPEnable = false;
    
    o_ql_consilient_aec_info.AECCtrlParams.AECEnable = true;
    o_ql_consilient_aec_info.AECCtrlParams.HPFEnable = false;
    o_ql_consilient_aec_info.AECCtrlParams.SamplingRate = AEC_SAMPLING_RATE;
    o_ql_consilient_aec_info.AECCtrlParams.CNGEnable = 0; //1;
    o_ql_consilient_aec_info.AECCtrlParams.PTime = 10; // 20mS , 15mS
    o_ql_consilient_aec_info.AECCtrlParams.BulkDelay = 0; //atoi(argv [7]) ;
    o_ql_consilient_aec_info.AECCtrlParams.CNGInitialLvl = 0;
    o_ql_consilient_aec_info.AECCtrlParams.CNGAdjustLvl = 1;
    o_ql_consilient_aec_info.AECCtrlParams.DTControl = 1;
    
    o_ql_consilient_aec_info.AECCtrlParams.EchoTaillen = AEC_ECHO_TAIL_LEN;
    
    o_ql_consilient_aec_info.AECCtrlParams.PPCtrlParams.ResidualSuppressionFactor = 10;
    /*
   This function returns the required AEC state memory for given configurations
   and application should allocate the same memory and pass the pointer as an argument
   to AEC_Init() and AEC_Process () functions
   */
    mem_sz = AEC_GetChannelMemory (o_ql_consilient_aec_info.AECCtrlParams.EchoTaillen,
                          o_ql_consilient_aec_info.AECCtrlParams.SamplingRate,
                          o_ql_consilient_aec_info.AECCtrlParams.PTime,
                          o_ql_consilient_aec_info.ECEnable,
                          o_ql_consilient_aec_info.AECCtrlParams.PPEnable,
                          o_ql_consilient_aec_info.IsNSRequired,
                          o_ql_consilient_aec_info.AECCtrlParams.BulkDelay,
    0) ;  // mono

    if(mem_sz <= AEC_STATE_MEM_SIZE)
    {

    AEC_Init (AECStateMemory,
              &o_ql_consilient_aec_info.AECCtrlParams,
              (INT16 *)NULL,
              (INT16 *)NULL,
              o_ql_consilient_aec_info.pAEC_Scratchmemory);

    o_ql_consilient_aec_info.AECRTCtrlParams.AdaptationControl = 1 ;
    //    AECRTCtrlParams.PPLevel = 4 ;
    o_ql_consilient_aec_info.NSRTCtrlParams.NSEnable = o_ql_consilient_aec_info.IsNSRequired ;
    o_ql_consilient_aec_info.NSRTCtrlParams.NSLevel = 10 ;
    }
    else{
          status = e_ql_audio_preproc_status_out_of_data_mem;
    }

  return status;
}

