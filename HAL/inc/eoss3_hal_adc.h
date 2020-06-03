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

#include "test_types.h"
#include "eoss3_hal_def.h"
typedef enum
{
  ADC_CHANNEL_0,
  ADC_CHANNEL_1
}ADC_channel;
HAL_StatusTypeDef HAL_ADC_Init(ADC_channel channelNum,uint8_t enableBattMeasure);
void HAL_ADC_StartConversion(void );
HAL_StatusTypeDef HAL_ADC_GetData( uint16_t *pData);