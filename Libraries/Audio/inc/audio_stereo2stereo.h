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

#ifndef AUDIO_STEREO2STEREO_H
#define AUDIO_STEREO2STEREO_H
 
#include "Fw_global_config.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "datablk_mgr.h"

extern outQ_processor_t audio_stereo2stereo_outq_processor;
  
void datablk_pe_config_stereo2stereo(void *p_pe_object);
void datablk_pe_process_stereo2stereo(QAI_DataBlock_t *pIn, QAI_DataBlock_t *pOut, QAI_DataBlock_t **pRet,
                                 void (*p_event_notifier)(int pid, int event_type, void *p_event_data, int num_data_bytes));

#endif /* AUDIO_STEREO2STEREO_H */