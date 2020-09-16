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

/** @file ql_i2stx_pe.h */

#ifndef QL_I2STX_PE_H
#define QL_I2STX_PE_H

#include <stdint.h>
#include "Fw_global_config.h"
#include "datablk_mgr.h"

void datablk_pe_config_ql_i2stx(void *p_pe_object);

void datablk_pe_process_ql_i2stx(QAI_DataBlock_t *pIn, QAI_DataBlock_t *pOut, QAI_DataBlock_t **pRet,
                                 void (*p_event_notifier)(int pid, int event_type, void *p_event_data, int num_data_bytes));

#endif /* QL_I2STX_PE_H */
