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

#include "vr_engine_api.h"
// These 3 APIs need to be provided by every VR engine
__attribute__((weak)) void vr_engine_init(void)
{
  // initialize the VR engine
  dbg_str("vr_engine_init: Using a STUB VR engine API, replace with desired VR engine");
  return;
}

__attribute__((weak)) int vr_engine_get_samples_per_frame(void)
{
  // return samples per frame size used by the VR engine
  return 240; // default samples per frame size
}

__attribute__((weak)) int vr_engine_process(short *samples)
{
  // process input samples and return if wakeword is detected
  return 0; // 0 = no wakeword detected, 1 = wakeword detected
}

// The below APIs need to be provided by every AEC engine
#include "datablk_mgr.h"
#include "FreeRTOS.h"
#include "semphr.h"
__attribute__((weak)) SemaphoreHandle_t  ql_pre_proc_sem;

__attribute__((weak))  void set_signal_detect_state(int state)
{
	return;
}

__attribute__((weak)) void datablk_pe_process_ql_pre_process(QAI_DataBlock_t *pIn, QAI_DataBlock_t *pOut, QAI_DataBlock_t **pRet,
                                 void (*p_event_notifier)(int pid, int event_type, void *p_event_data, int num_data_bytes))
{
	// process input samples
	return;
}

__attribute__((weak)) void datablk_pe_config_ql_pre_process(void *p_pe_object)
{
	// configure the PE
	return;
}
