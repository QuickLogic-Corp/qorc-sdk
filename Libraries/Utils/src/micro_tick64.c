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

/** @file micro_tick64.c */
/*==========================================================
 *
 *    File   : micro_tick64.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

#include "FreeRTOS.h"
#include "task.h"

static volatile uint64_t   xRtcTickCount64_usec       = 0;
static volatile uint64_t   xRtcTickCount64_msec_start = 0;

void xTaskSet_uSecCount(uint64_t new_val)
{
    portENTER_CRITICAL();
    xRtcTickCount64_msec_start = xTaskGetTickCount();
    xRtcTickCount64_usec = new_val;
    portEXIT_CRITICAL();
}

uint64_t xTaskGet_uSecCount(void)
{
    uint64_t v;
    portENTER_CRITICAL();
    v = (xTaskGetTickCount() - xRtcTickCount64_msec_start ) * 1000;
    portEXIT_CRITICAL();
    return (v + xRtcTickCount64_usec);
}

uint64_t convert_to_uSecCount(uint32_t tickCount)
{
    uint64_t v;
    v = (tickCount - xRtcTickCount64_msec_start ) * 1000;
    return (v + xRtcTickCount64_usec);
}
