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
 *    File   : eoss3_hal_timer.h
 *    Purpose: This file contains macros and APIs for
 *             M4 peripheral timer 
 *                                                          
 *=========================================================*/

#ifndef HAL_INC_EOSS3_HAL_TIMER_H_
#define HAL_INC_EOSS3_HAL_TIMER_H_

#include <common.h>
#include <stdint.h>
#include <stddef.h>

#include "test_types.h"
#include "eoss3_hal_def.h"

typedef struct __Timer_HandleTypeDef
{
     UINT32_t	ulClkFreq;
     UINT32_t   ulReload_val;
     
}Timer_HandleTypeDef;
   
/* Ctrl register bit definition */
#define TIMER_INT_ENABLE	(UINT32_t)(1 << BYTE_IDX_3)
#define SEL_EXTINT_AS_CLK	(UINT32_t)(1 << BYTE_IDX_2)
#define SEL_EXTINT_AS_EN	(UINT32_t)(1 << BYTE_IDX_1)
#define TIMER_ENABLE		(UINT32_t)(1 << BYTE_IDX_0)

#define HAL_TIMER_ONESHOT	(1 << 1)
#define HAL_TIMER_PERIODIC	(1 << 0)

typedef void (*HAL_TimerCallback_t)(void *);
struct HAL_Timer_t;
typedef struct HAL_Timer_t* HAL_Timer_t;

HAL_StatusTypeDef HAL_Delay_Init(void);
void HAL_DelayUSec(uint32_t usecs);

HAL_StatusTypeDef HAL_TimerCreate(HAL_Timer_t *Timer, uint32_t USecs,
				  uint32_t Flags, HAL_TimerCallback_t CallBack,
				  void *Cookie);
HAL_StatusTypeDef HAL_TimerStart(HAL_Timer_t Timer);
HAL_StatusTypeDef HAL_TimerStop(HAL_Timer_t Timer);
HAL_StatusTypeDef HAL_TimerDelete(HAL_Timer_t Timer);

HAL_StatusTypeDef HAL_Timer_Enable( );                          // to remove warnings 
HAL_StatusTypeDef HAL_Timer_Disable( );							// function declarations


#endif /* HAL_INC_EOSS3_HAL_TIMER_H_ */
