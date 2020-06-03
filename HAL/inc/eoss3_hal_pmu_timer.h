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
 *    File   : eoss3_hal_pmu_timer.h
 *    Purpose: This file contains macros, structures and APIs to
 *             for M4 peripheral timer
 *                                                          
 *=========================================================*/

#ifndef HAL_INC_EOSS3_HAL_PMU_TIMER_H_
#define HAL_INC_EOSS3_HAL_PMU_TIMER_H_

#include <common.h>
#include <stdint.h>
#include <stddef.h>

#include "test_types.h"
#include "eoss3_hal_def.h"

typedef struct __PMU_Timer_HandleTypeDef
{
     UINT32_t   timerReload_val;
     void (*timer_callback) (void *args);
}PMU_Timer_HandleTypeDef;
   
#define MAX_TIMER_COUNT  1
#define MSEC_PER_TICK   1.953125f

HAL_StatusTypeDef HAL_PMU_Set_Timer(UINT32_t timerLoadVal, void (*callbackFn) (void *args));
UINT32_t HAL_PMU_Disable_Timer( );
HAL_StatusTypeDef HAL_PMU_Timer_Interrupt_Callback();

#endif /* HAL_INC_EOSS3_HAL_PMU_TIMER_H_ */
