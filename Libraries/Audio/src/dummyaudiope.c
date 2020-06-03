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


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "ql_vr_common.h"
#include "ql_util.h"
#include "process_ids.h"
#include "ql_controlTask.h"
#include "ql_audio.h"
#include "datablk_processor.h"
#include "dummyaudiope.h"



/*
 *--------------------------------------------------
 * Defines
 *--------------------------------------------------
*/


//--------------------------------------------------
// Global variables
//--------------------------------------------------
SemaphoreHandle_t         dummyaudio_sem;          ///< Semaphore specific to dummyaudio function

//--------------------------------------------------
// Local variables
//--------------------------------------------------




/******************* Function Implementations ******************************/

void datablk_pe_config_dummyaudio(void *p_pe_object)
{
  // Create semaphohre that will be used to lock control
  dummyaudio_sem = xSemaphoreCreateBinary(  );
  xSemaphoreGive(dummyaudio_sem);
  // Put and other configuration code here
  return ;
}

void datablk_pe_process_dummyaudio(QAI_DataBlock_t *pIn, QAI_DataBlock_t *pOut, QAI_DataBlock_t **pRet,
                                 void (*p_event_notifier)(int pid, int event_type, void *p_event_data, int num_data_bytes))
{
    xSemaphoreTake(dummyaudio_sem, portMAX_DELAY);
    // Processing code goes here -- protected from any control code by the semaphore
    xSemaphoreGive(dummyaudio_sem);
    return ;
}
