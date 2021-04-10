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
 *    File   : batt_level_task.h
 *    Purpose: Monitor battery level, indicate if below some threshold
 *
 *=========================================================*/

#include "Fw_global_config.h"

#define BATTERY_LEVEL_MONITOR_INTERVAL         (1000) ///< monitor battery level at 1000ms intervals
#define BATTERY_LEVEL_WARNING_THRESHOLD        (2900) ///< turn on LED for a while if below this threshold
#define BATTERY_LEVEL_LED_BLINK_PERIOD         (50)   ///< turn on LED for 50ms to warn about battery level

#define BATTERY_LEVEL_MONITOR_TASK_STACKSIZE   (256)  ///< stack size (in words) for this task

extern int mCurrentBatteryLevel;
extern xTaskHandle xBatteryLevelMonitorTask;

/* Return the battery voltage level in mV units */
extern int getCurrentBatteryLevel(void);

/* Start a new ADC conversion and update the current battery level */
extern void updateCurrentBatteryLevel(void);

extern void battery_level_monitor_task(void *pParameter);

/* start battery level monitor task */
extern void start_battery_level_monitor_task(void);
