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
 *    File   : eoss3_hal_wdt.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __EOSS3_HAL_WDT_H__
#define __EOSS3_HAL_WDT_H__
#include "eoss3_hal_def.h"

HAL_StatusTypeDef HAL_WDT_Init(unsigned int secs_timeout);
HAL_StatusTypeDef HAL_WDT_DeInit(void);
HAL_StatusTypeDef HAL_WDT_Start(void);
HAL_StatusTypeDef HAL_WDT_Stop(void);
HAL_StatusTypeDef HAL_WDT_Reload(void);
HAL_StatusTypeDef HAL_WDT_WdtIsStartStatus(void);

#endif
