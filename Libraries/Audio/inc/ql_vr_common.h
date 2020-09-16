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

#ifndef QL_VR_COMMON_H
#define QL_VR_COMMON_H

#include <stdint.h>
#include <assert.h>
#include "Fw_global_config.h"
#include "datablk_mgr.h"
#include "semphr.h"

#define MAX_NO_OF_KPDS  (2)

typedef enum _e_ql_kp_type_t
{
  e_ql_kp_type_kpt = 1,
  e_ql_kp_type_cmd = 2

}e_ql_kp_type_t;


typedef int16_t t_pcm;

typedef enum
{
  e_ql_vr_param_net,
  e_ql_vr_param_grammar,
  e_ql_vr_param_sdet,
  e_ql_vr_param_min_snr,
  e_ql_vr_param_all,
  e_ql_vr_param_reset,
  e_ql_vr_param_restart,
  e_ql_vr_param_terminate,
  e_ql_vr_param_net_gram
}e_ql_vr_parameters_t;




typedef enum
{
  ql_vr_callback_event_none             = 0,
  ql_vr_callback_event_det_ok           = 10,
  ql_vr_callback_event_det_ok_sv        = 20,
  ql_vr_callback_event_det_incomplete   = 30,
  ql_vr_callback_event_warning          = 40,
  ql_vr_callback_event_err              = -1, // critial error

}e_ql_vr_callback_events_t;


typedef enum
{
  e_ql_vr_status_ok = 0,
  e_ql_vr_status_detection_ok = 1,
  e_ql_vr_status_detection_not_finished = 2,
  e_ql_vr_status_out_of_token_mem = 10, // non critial error
  e_ql_vr_status_sv_failure = 20, // non critial error
  e_ql_vr_status_err = -1,
  e_ql_vr_status_invalid_param = -2,
  e_ql_vr_status_out_of_heap = -3,
  e_ql_vr_status_out_of_data_mem = -4

}e_ql_vr_status;



typedef unsigned short ql_vr_errors_t;
typedef short ql_vr_pcm_sample_t;
typedef void* h_ql_vr_handle_t;
typedef struct _ql_vr_results_t
{
  e_ql_vr_status e_status;
  h_ql_vr_handle_t handle;
  e_ql_vr_callback_events_t event_id;
  uint32_t time_stamp;
  int16_t start_idx;
  int16_t end_idx;


  int16_t word_id;           // word recognized (0 = SIL for T2SI)
  int16_t word_id_org;           // word recognized (0 = SIL for T2SI)
  uint16_t in_vocab;          // 1 if not NOTA or SIL
  uint16_t is_nota;
  int16_t  svcore;            // segmental score, may be negative for WS
  uint16_t gscore;           // garbage score, can be used to compute duration
  uint16_t final_score;
  uint16_t duration;

}ql_vr_results_t;

extern SemaphoreHandle_t  ql_vr_sem; ///< Semaphore specific to Nuance VR functions

typedef struct st_ql_vr_event_data {
  int startFramesBack;
  int lengthFrames;
  int score;
#if 0 //1=rdsp, 0=AWWE
  unsigned short *p_phrase_text;
  //int len_phase_text;
  int len_phrase_text;
#else  
  char *p_phrase_text; //in AWWE phrase is char string
  int len_phrase_text;//type corrected
#endif    
} ql_vr_event_data_t;

typedef e_ql_vr_status (*ql_vr_callback) (void *context, ql_vr_results_t* p_ql_vr_results);

typedef struct _ql_vr_parameters_t
{
  uint32_t net;
  uint32_t grammar;
  uint16_t sdet;
  uint16_t epq_min_snr;
  ql_vr_callback pf_callback;
  uint16_t frame_duration_ms;
  void *ql_vr_callback_context;

  int16_t  kp_word_mapping[MAX_NO_OF_KPDS];

#ifdef KPDCMD
  int16_t  cmd_word_mapping[MAX_CMD];
#endif
}ql_vr_parameters_t;


extern void datablk_pe_config_ql_vr(void *p_pe_object);
extern void datablk_pe_process_ql_vr(QAI_DataBlock_t *pIn, QAI_DataBlock_t *pOut, QAI_DataBlock_t **pRet,
                                 void (*p_event_notifier)(int pid, int event_type, void *p_event_data, int num_data_bytes));

#ifdef __cplusplus
extern "C" {
#endif

  


#ifdef __cplusplus
}
#endif

#endif /* QL_VR_COMMON_H */

