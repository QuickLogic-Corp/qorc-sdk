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

#define SENSORS_AD7476_ACQUISITION_READ_FROM_FILE (0)

#if (SENSORS_AD7476_ACQUISITION_READ_FROM_FILE)

#include "sensor_ad7476_config.h"
#include "sensor_ad7476_acquisition.h"
#include "sensor_ad7476_process.h"

#include "datablk_mgr.h"
#include "process_ids.h"
#include "timers.h"
#include <string.h>

#define AD7476_TIMER_PERIOD     (10)
TimerHandle_t ad7476TimId;

int ad7476_batch_size_get(void)
{
  int batch_size;
  batch_size  = 10*(ad7476_config.rate_hz) + 999;
  batch_size /= 1000;
  batch_size += 1;
  batch_size &= ~1;
  return batch_size;
}

int ad7476_sensordata_buffer_ready(void)
{
  int16_t *p_dest = (int16_t *)pad7476_data_block_prev->p_data;
  int nSamples;
  int batch_size;

  nSamples  = (AD7476_TIMER_PERIOD * ad7476_config.rate_hz);
  nSamples /= 1000;
  nSamples *= SENSOR_AD7476_CHANNELS;
  memset( p_dest, 0x11, nSamples*2 );
  ad7476_samples_collected += nSamples;
  
  batch_size = ad7476_batch_size_get() * SENSOR_AD7476_CHANNELS ;
  if (ad7476_samples_collected >= batch_size)
  {
    pad7476_data_block_prev->dbHeader.numDataElements = ad7476_samples_collected;
    pad7476_data_block_prev->dbHeader.numDataChannels = SENSOR_AD7476_CHANNELS;
    ad7476_samples_collected = 0;
    return 1;
  }
  else
  {
    return 0;
  }
}

void ad7476_dataTimer_Callback(TimerHandle_t timHandle)
{ 
  // Warning: must not call vTaskDelay(), vTaskDelayUntil(), or specify a non zero 
  // block time when accessing a queue or a semaphore. 
  if (ad7476_isr_outq_processor.process_func)
    ad7476_isr_outq_processor.process_func();
} 

void ad7476_dataTimerStart(void)
{
  BaseType_t  status;
 
  // Create periodic timer
  if (!ad7476TimId) {
    ad7476TimId = xTimerCreate
                (
                   "ad7476Timer",
                   AD7476_TIMER_PERIOD, // 10 ticks = ~10ms
                   pdTRUE,            // auto-reload when the timer expires
                   (void *)0,
                   ad7476_dataTimer_Callback
                );
  }
  
  if (ad7476TimId)  {
    status = xTimerStart (ad7476TimId, 0);                // start timer
    if (status != pdPASS)  {
      // Timer could not be started
    } 
  }
  ad7476_set_first_data_block();
}

void ad7476_dataTimerStop(void)
{
  if (ad7476TimId) {
    xTimerStop(ad7476TimId, 0);
  }
}
#endif

