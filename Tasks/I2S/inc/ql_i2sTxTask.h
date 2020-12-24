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
/*==========================================================
 *
 *    File   : ql_i2sTask.h
 *    Purpose: 
 *
 *=========================================================*/

#ifndef QL_I2STXTASK_H_
#define QL_I2STXTASK_H_
#include "datablk_mgr.h"
extern QueueHandle_t I2SDataQ;

extern signed portBASE_TYPE StartRtosTaskI2S( void);
extern int addDatablkToQueue_I2S(QAI_DataBlock_t *pIn);

extern void stop_i2sTx(void);
extern void start_i2sTx(int start_blks);
extern int check_i2s_space_available(void);

#endif //QL_I2STXTASK_H_