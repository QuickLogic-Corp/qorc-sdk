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

/** @file sensor_ad7476_config.c */

#include <stdbool.h>
#include <string.h>

#include "Fw_global_config.h"
#include "sensor_ad7476_config.h"
#include "sensor_ad7476_acquisition.h"

#include "dbg_uart.h"
#include "dcl_commands.h"           /* for sensor configuration structure definitions */
#include "eoss3_hal_fpga_adc_api.h" /* ADC FPGA Driver API */

ad7476_config_t ad7476_config;
HAL_ADC_FPGA_cfg_t adc_fpga_task_config =
{
  .sensor_id = SENSOR_AD7476_ID,
  .frequency = 1000000
};

void sensor_ad7476_startstop( int is_start )
{
  /** @todo Replace contents of this function */
#if  (SENSORS_AD7476_ACQUISITION_READ_FROM_FILE)
  if (is_start == 1) 
  {
    ad7476_dataTimerStart();
  }
  else
  {
    ad7476_dataTimerStop();
  }
#endif

  if ( (is_start == 1) && (ad7476_config.enabled) )
  {
    extern void ad7476_isr_DmacDone(void);
    extern void ad7476_start_dma(void);
    if (ad7476_config.is_running)
      return;
    int status = HAL_ADC_FPGA_Init( &adc_fpga_task_config, &ad7476_isr_DmacDone);
    configASSERT( status == HAL_OK );
    ad7476_start_dma();
    dbg_str_int("ADC DMA Start", xTaskGetTickCount());
    ad7476_config.is_running = 1;
  }
  else
  { /* stop ADC FPGA Driver */
    extern void ad7476_stop_dma(void);
    if (ad7476_config.is_running)
    {
      ad7476_stop_dma();
        HAL_ADC_FPGA_De_Init();
        ad7476_config.is_running = 0;
    }
  }
}

void sensor_ad7476_configure(void)
{
  /** @todo Replace contents of this function */
  ad7476_config.rate_hz = SENSOR_AD7476_RATE_HZ_MAX;
}

void sensor_ad7476_clear( void )
{
  ad7476_config.enabled = false;
  /** @todo Replace contents of this function */
  //adc_fpga_task_config.sensor_id = 0;
  adc_fpga_task_config.ltc1859.channel_enable_bits = 0;
  //ltc1859_watch_data = NULL;
  //ad7476_watch_data = NULL;
}

void sensor_ad7476_add(void)
{
  ad7476_config.enabled = true;
  ad7476_config.rate_hz = sensor_config_msg.sensor_common.rate_hz;
  /** @todo Replace contents of this function */
  memset( (void *)(&adc_fpga_task_config), 0,sizeof(adc_fpga_task_config));
  if( sensor_config_msg.sensor_common.sensor_id != SENSOR_AD7476_ID ){
      dbg_fatal_error("INVALID sensor id for adc");
  }
  adc_fpga_task_config.sensor_id = SENSOR_AD7476_ID; // HAL_SENSOR_ID_AD7476;
  dbg_str_hex32("add-adc-ad7476", adc_fpga_task_config.sensor_id );
  adc_fpga_task_config.frequency = 
        sensor_config_msg.sensor_common.rate_hz;
  adc_fpga_task_config.ad7476.param0 = 
        sensor_config_msg.unpacked.ad7476.param0;
  adc_fpga_task_config.ad7476.param1 =
        sensor_config_msg.unpacked.ad7476.param1;
}
