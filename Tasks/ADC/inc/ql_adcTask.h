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

#ifndef __QL_ADCTASK_H_
#define __QL_ADCTASK_H_

#if !defined( _EnD_Of_Fw_global_config_h )
#error "Include Fw_global_config.h first"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include <queue.h>
#include "common.h"
#include "test_types.h"
#include "eoss3_hal_fpga_adc_api.h"


/* sensor control functions */

/* return TRUE if the task is running and collecting data */
int  sensor_adc_fpga_is_running( void );

/* clear sensor configuration */
void sensor_adc_fpga_clear(void);

// verify the fpga is present for this fpga
bool sensor_is_adc_fpga_present( uint32_t sensorid );


/* configure the sensor based content of 'ltc1859_task_config' */
void sensor_adc_fpga_configure(void);

/* start or stop the LTC1859 sensor */
void sensor_adc_fpga_startstop( int is_start );

/* Last known configuration of the sensor */
extern HAL_ADC_FPGA_cfg_t adc_fpga_task_config;

//Analog channel type
#define LTC1859_SINGLE_ENDED 1    //Single Ended configuration
#define LTC1859_DIFFERENTIAL 0    //Differential configuration

#define LTC1859_SINGLE_DIFF_CHANNEL_CONFIG     LTC1859_SINGLE_ENDED  //Current configuration

#define LTC1859_CH0_ADDR ((0x0) | (LTC1859_SINGLE_DIFF_CHANNEL_CONFIG << 3))
#define LTC1859_CH1_ADDR ((0x1) | (LTC1859_SINGLE_DIFF_CHANNEL_CONFIG << 3))
#define LTC1859_CH2_ADDR ((0x2) | (LTC1859_SINGLE_DIFF_CHANNEL_CONFIG << 3))
#define LTC1859_CH3_ADDR ((0x3) | (LTC1859_SINGLE_DIFF_CHANNEL_CONFIG << 3))

/*
 * If the sensor is "running" this points to current data.
 * meaning: An array of uint16_t values representing each enabled channel.
 *
 * The purpose is so that debug code can reasonably display live sensor data on a terminal screen.
 */
extern const uint16_t *ltc1859_watch_data;
extern const uint16_t *ad7476_watch_data;

#define LTC1859_UNIPOLAR 1
#define LTC1859_BIPOLAR 0

#define LTC1859_POLARITY        LTC1859_BIPOLAR    //Current configuration

#define LTC1859_RANGE5V 0
#define LTC1859_RANGE10V 1

#define LTC1859_GAIN            LTC1859_RANGE5V    //Current configuration for gain.

#define LTC1859_NAP             0
#define LTC1859_SLEEP           0

//Current configuration for power mode. Always set it to 0. We dont support low power configuration.
#define LTC1859_POWER_DOWN_MODE ((LTC1859_NAP << 1) | (LTC1859_SLEEP << 0))


#define LTC1859_CHANNEL_NUM_BITS_POS 4
#define LTC1859_POLARITY_BIT_POS     3
#define LTC1859_GAIN_BIT_POS         2
#define LTC1859_POWER_MODE_BIT_POS   0


//Refer LTC185x Datasheet.pdf page no. 15 for descrption of various fields.
#define LTC1859_CHANNEL_0_COMMAND   ((LTC1859_CH0_ADDR << LTC1859_CHANNEL_NUM_BITS_POS) |\
                             (LTC1859_POLARITY << LTC1859_POLARITY_BIT_POS) |\
                             (LTC1859_GAIN << LTC1859_GAIN_BIT_POS) |\
                             (LTC1859_POWER_DOWN_MODE << LTC1859_POWER_MODE_BIT_POS))

#define LTC1859_CHANNEL_1_COMMAND   ((LTC1859_CH1_ADDR << LTC1859_CHANNEL_NUM_BITS_POS) |\
                             (LTC1859_POLARITY << LTC1859_POLARITY_BIT_POS) |\
                             (LTC1859_GAIN << LTC1859_GAIN_BIT_POS) |\
                             (LTC1859_POWER_DOWN_MODE << LTC1859_POWER_MODE_BIT_POS))

#define LTC1859_CHANNEL_2_COMMAND   ((LTC1859_CH2_ADDR << LTC1859_CHANNEL_NUM_BITS_POS) |\
                             (LTC1859_POLARITY << LTC1859_POLARITY_BIT_POS) |\
                             (LTC1859_GAIN << LTC1859_GAIN_BIT_POS) |\
                             (LTC1859_POWER_DOWN_MODE << LTC1859_POWER_MODE_BIT_POS))

#define LTC1859_CHANNEL_3_COMMAND   ((LTC1859_CH3_ADDR << LTC1859_CHANNEL_NUM_BITS_POS) |\
                             (LTC1859_POLARITY << LTC1859_POLARITY_BIT_POS) |\
                             (LTC1859_GAIN << LTC1859_GAIN_BIT_POS) |\
                             (LTC1859_POWER_DOWN_MODE << LTC1859_POWER_MODE_BIT_POS))

#define LTC1859_ADC_SAMPLE_FREQ_16K   16000   // ADC sampling frequency  16000Hz
#define LTC1859_ADC_SAMPLE_FREQ_100K  100000  // ADC sampling frequency 100000Hz

#define LTC1859_EN_ALL_ADC_CHNLS      0xFF    // If all ADC channels need to be enabled


signed portBASE_TYPE StartRtosTaskADC(void);

#endif //__QL_AUDIOTASK_H_
