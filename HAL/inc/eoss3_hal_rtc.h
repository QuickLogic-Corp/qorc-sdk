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

#ifndef __EOSS3_HAL_RTC_H__
#define __EOSS3_HAL_RTC_H__
#include <stdio.h>
#include "eoss3_hal_def.h"

/*
 * HAL_RTC_Init : Initializes the rtc with the value uiInitialRTCTimeValueSecs.
 * This call can happen only once & same is taken care inside this api. Second
 * time call will not have any effect.
 * RTC will start counting from this value. This value can be changed in runtime
 * using API HAL_RTC_SetTime
 */
HAL_StatusTypeDef HAL_RTC_Init(uint32_t uiInitialRTCTimeValueSecs);

/*
 * HAL_RTC_SetupAlarm : Setup RTC to interrupt every "rtc_alarm_interval" seconds & call
 * rtc_cb
 */
HAL_StatusTypeDef HAL_RTC_SetupAlarm(uint32_t rtc_alarm_interval, void (*rtc_cb)(void));

/*
 * HAL_RTC_SetTime : RTC will start counting from the value uiRTCValueSecs
 */
HAL_StatusTypeDef HAL_RTC_SetTime(uint32_t uiRTCValueSecs);

/*
 * HAL_RTC_GetTime : RTC will return current seconds counted in uiRTCValueSecs
 */
HAL_StatusTypeDef HAL_RTC_GetTime(uint32_t *uiRTCValueSecs);

void HAL_RTC_ISR(void);

#endif