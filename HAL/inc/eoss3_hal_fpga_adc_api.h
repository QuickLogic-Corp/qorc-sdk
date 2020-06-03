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

/*! \file eoss3_hal_fpga_adc_api.h
 *
 *  \brief This file contains ADC wrapper APIs to use by FPGA ADC IP drivers
 *         for data transfer to and from FPGA ADC IP on S3.
 *         These APIs can be used by only FPGA IP applications.
 */

#ifndef HAL_EOSS3_HAL_FPGA_ADC_API_H_
#define HAL_EOSS3_HAL_FPGA_ADC_API_H_

#include "eoss3_dev.h"
#include "eoss3_hal_def.h"

/* ADC LTC1859 command configuration.
 * channelX_command 8 bit format as follows:
 * bit 7:4->Channel Mux Address ==> Channel selection and Single/Differential config.
 * bit 3:2->Input Range selection ==> Polarity and Gain of input signal.
 * bit 1:0->Power down modes. [NAP:SLEEP]. Currently these bits are not supported.
 * Each channel can be configured independently of others.
 * Refere data sheet "LTC185x Datasheet.pdf" for details about these configuration.
 */
typedef struct __LTC1859_cfg_t
{
  uint32_t frequency;
  /* today - we support 4 channels, future we might support 8 single ended */
#define LTC1859_MAX_CHANNELS 8
  uint8_t chnl_commands[LTC1859_MAX_CHANNELS];
  uint8_t channel_enable_bits;

} HAL_LTC1859_cfg_t;

/* 
 * This structure maps a "config" value
 * into two things:
 *   (1) A format char for the JSON data.
 *   (2) conversion factors that convert the ADC count into uVolts.
 */
struct LTC1859_ymx_plus_b {
     uint8_t masked_config_value;
     char    python_format_char;
     int8_t  is_signed;
     int8_t  dummy_pad;
     int32_t slopeM; /* multiplier */
     int32_t slopeD; /* divisor */
     int32_t intercept;
};

/* given a config value, return a valid conversion struct for the sensor value */
const struct LTC1859_ymx_plus_b *HAL_LTC1859_CfgToYmxB( int config );
int LTC1859_to_uVolts( uint8_t cfg, uint16_t value );

/* Initilazation of FPGA ADC HAL driver.
 * cfg -> pointer to filled configuration structure for adc channels.
 * pcb_read is call back function which gets called when sensor data read is
 * completed when HAL_FPGA_ADC_Read() is called.
 * Returns Success/Failure.
 */
HAL_StatusTypeDef HAL_LTC1859_ADC_Init(HAL_LTC1859_cfg_t *cfg, void (*pcb_read)(void));

/*
 * API to read block of sensor data.
 * Returns success/failure.
 * This is non-blocking API, when it fills buffer, read call back will be called.
 * Buffer length is expected to be multiple of 4 bytes. If buffer length is not
 * multiple of 4 bytes, it will fill buffer upto nearest multiple of 4 [= n_bytes/4*4]
 */
HAL_StatusTypeDef HAL_LTC1859_ADC_Read(void *buffer, size_t n_bytes);

/*
 * API to release acquired resources.
  */
void HAL_LTC1859_ADC_De_Init(void);


#endif //HAL_EOSS3_HAL_FPGA_ADC_API_H_
