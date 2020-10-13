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

#ifndef QL_AUDIO_PREPROC__H
#define QL_AUDIO_PREPROC__H

#include <stdint.h>
#include <assert.h>

#include <assert.h>
#include "ql_audio_stream.h"
#include "Fw_global_config.h"

#include "datablk_mgr.h"
#include "semphr.h"

#define     QL_AUDIO_PREPROC_NONE               (0)
#define     QL_AUDIO_PREPROC_BYPASS1            (1)
#define     QL_AUDIO_PREPROC_BYPASS2            (2)
#define     QL_AUDIO_PREPROC_BYPASS3            (3)
#define     QL_AUDIO_PREPROC_CONS_NS_1MIC      (11)
#define     QL_AUDIO_PREPROC_CONS_AEC          (15)
#define     QL_AUDIO_PREPROC_DSPC_NS_1MIC      (21)
#define     QL_AUDIO_PREPROC_DSPC_NS_2MIC      (22)
#define     QL_AUDIO_PREPROC_DSPC_NS_AEC       (25)
#define     QL_AUDIO_PREPROC_DSPC_AEC       (26)

typedef enum {
  e_ql_audio_preproc_option_default = 0,
  e_ql_audio_preproc_option_overlay = 1,
}t_ql_audio_preproc_option_e;

typedef enum
{
  e_ql_audio_preproc_status_ok = 0,
  e_ql_audio_preproc_status_overflow = 1, // non critial error
  e_ql_audio_preproc_status_underflow = 2, // non critial error
  e_ql_audio_preproc_status_err = -1,
  e_ql_audio_preproc_status_invalid_param = -2, // critical erro
  e_ql_audio_preproc_status_out_of_heap = -3,
  e_ql_audio_preproc_status_out_of_data_mem = -4
}e_ql_audio_preproc_status;

typedef struct _t_audio_preproc_info
{
  e_ql_audio_preproc_status e_status;
  int32_t frame_sz;
  int32_t process_frame_sz;
  int32_t fs;
  int32_t channels;
  uint32_t bypass;
}t_audio_preproc_info;
#ifdef __cplusplus
extern "C" {
#endif

t_audio_preproc_info *ql_audio_preproc_init(t_ql_audio_preproc_option_e e_option);
e_ql_audio_preproc_status ql_audio_preproc_process(t_ql_audio_stream *p_mic_input,
                                                   t_ql_audio_stream *p_ref_input,
                                                   t_ql_audio_stream *p_audio_output);
e_ql_audio_preproc_status ql_audio_preproc_close(void);
#ifndef UNIT_TEST
extern SemaphoreHandle_t  ql_pre_proc_sem;
void datablk_pe_config_ql_pre_process(void *p_pe_object);
void datablk_pe_process_ql_pre_process(QAI_DataBlock_t *pIn, QAI_DataBlock_t *pOut, QAI_DataBlock_t **pRet,
                                 void (*p_event_notifier)(int pid, int event_type, void *p_event_data, int num_data_bytes));
#endif
void pre_process_bypass(uint32_t bypass);

#ifdef __cplusplus
}
#endif

#endif /* QL_AUDIO_PREPROC__H */

