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
/** @file sensor_ad7476_acquisition.h */

#ifndef __SENSOR_AD7476_ACQUISITION_H__
#define __SENSOR_AD7476_ACQUISITION_H__

#include "Fw_global_config.h"

#include "datablk_mgr.h"

/* Sensor AD7476 capture ISR */
#define AD7476_ISR_EVENT_NO_BUFFER  (1)   ///< error getting a new datablock buffer

extern QAI_DataBlock_t *pad7476_data_block_prev;
extern int              ad7476_samples_collected;
extern outQ_processor_t ad7476_isr_outq_processor;

extern void ad7476_set_first_data_block();
extern void ad7476_acquisition_read_callback(void);
extern int ad7476_sensordata_buffer_ready(void);

#define SENSORS_AD7476_ACQUISITION_READ_FROM_FILE (0)
#if (SENSORS_AD7476_ACQUISITION_READ_FROM_FILE)
extern void ad7476_dataTimerStart(void);
extern void ad7476_dataTimerStop(void);
#endif

#endif /* __SENSOR_FOO_ACQUISITION_H__ */