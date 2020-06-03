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

/*
 * eoss3_hal_leds.h
 *
 */

 #ifndef _leds_h
#define _leds_h

#include "Fw_global_config.h"
#include <stdio.h>
#include "eoss3_dev.h"
#include "FreeRTOS.h"
#include "timers.h"

void LedGreenOn(void);
void LedGreenOff(void);
void LedBlueOn(void);
void LedBlueOff(void);
void LedOrangeOn(void);
void LedOrangeOff(void);
void LedOrangeOn_AutoOff(void);
void LedYellowOn(void);
void LedYellowOff(void);
void LedYellowBlink(void);
void LedGreenBlink(void);
void turnOnLeds(uint8_t on);

#endif /* !_leds_h */
