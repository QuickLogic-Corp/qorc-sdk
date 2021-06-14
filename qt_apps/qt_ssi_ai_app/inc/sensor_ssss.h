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

/** @file sensor_ssss.h */

#ifndef SENSOR_SSSS_H
#define SENSOR_SSSS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Fw_global_config.h"
#include "sensor_generic.h"
#include "datablk_mgr.h"

/*--- BEGIN User modifiable section ---*/
/* User settable MACROs and variables */

#define SENSOR_SSSS_SAMPLE_RATE_HZ       (100) // sensor sample rate per channel in Hz
#define SENSOR_SSSS_CHANNELS_PER_SAMPLE  ( 3)  // Number of channels
#define SENSOR_SSSS_LATENCY              (20)  // process samples every 20ms
#define SENSOR_SSSS_BIT_DEPTH            (16)  // bit-depth per sample, must be 16
#define SENSOR_SSSS_MAX_DATA_BLOCKS      (20)  // number of data blocks to allocate

#define SENSOR_SSSS_ID                   (0x53535353)
#define SENSOR_SSSS_NAME                 ssss
#define SENSOR_SSSS_NAME_UPPERCASE       SSSS

/* These should be add to process_ids.h, replace SSSS with the sensor name
   for example in case of Accelerometer sensor, replace SSSS_ISR_PID with
   ACCEL_ISR_PID
*/
#define SENSOR_SSSS_ISR_PID                ((process_id_t)21)
#define SENSOR_SSSS_AI_PID                 ((process_id_t)22)
#define SENSOR_SSSS_DATASAVE_PID           ((process_id_t)23)
#define SENSOR_SSSS_LIVESTREAM_PID         ((process_id_t)24)

/* END of User settable MACROs and variables */
/*--- END of User modifiable section ---*/

/* Do not modify the below MACRO definitions */

// compile error check for SENSOR_BIT_DEPTH
#if (SENSOR_SSSS_BIT_DEPTH == 16)
/* */
#else
#error "Sensor bit-depth should be 16-bits"
#endif

#define SENSOR_SSSS_SAMPLES_PER_CHANNEL  ((SENSOR_SSSS_LATENCY) * (SENSOR_SSSS_SAMPLE_RATE_HZ) / 1000)
#if (SENSOR_SSSS_SAMPLES_PER_CHANNEL == 0)
#undef  SENSOR_SSSS_SAMPLES_PER_CHANNEL
#define SENSOR_SSSS_SAMPLES_PER_CHANNEL    (1)
#endif
#define SENSOR_SSSS_SAMPLES_PER_BLOCK    ((SENSOR_SSSS_SAMPLES_PER_CHANNEL) * (SENSOR_SSSS_CHANNELS_PER_SAMPLE))
#define SENSOR_SSSS_BYTES_PER_BLOCK      ( (SENSOR_SSSS_SAMPLES_PER_BLOCK) * (SENSOR_SSSS_BIT_DEPTH) / 8 )
#define SENSOR_SSSS_MEMSIZE_MAX          ((SENSOR_SSSS_MAX_DATA_BLOCKS) * ((SENSOR_SSSS_BYTES_PER_BLOCK)+32))

extern const char json_string_sensor_config[];
extern sensor_generic_config_t sensor_ssss_config;

extern void sensor_ssss_startstop( int is_start );
extern void sensor_ssss_configure(void);
extern void sensor_ssss_clear( void );
extern void sensor_ssss_add(void);

/* sensor data acquisition */
extern QAI_DataBlock_t *psensor_ssss_data_block_prev;
extern int              sensor_ssss_samples_collected;
extern outQ_processor_t sensor_ssss_isr_outq_processor;

extern int sensor_ssss_batch_size_get(void);

extern void sensor_ssss_set_first_data_block();
extern void sensor_ssss_acquisition_read_callback(void);
extern int  sensor_ssss_acquisition_buffer_ready(void);

extern void sensor_ssss_dataTimerStart(void);
extern void sensor_ssss_dataTimerStop(void);

/* sensor processing datablocks, datablock manager and thread Q */
extern uint8_t               sensor_ssss_data_blocks[SENSOR_SSSS_MEMSIZE_MAX] ;
extern QAI_DataBlockMgr_t    sensor_ssssBuffDataBlkMgr;
extern QueueHandle_t         sensor_ssss_dbp_thread_q;

extern void sensor_ssss_block_processor(void);

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_SSSS_H */
