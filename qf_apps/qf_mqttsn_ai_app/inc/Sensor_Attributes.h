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

#ifndef __QL_SENSOR_ATTRIBUTES_H_
#define __QL_SENSOR_ATTRIBUTES_H_

#if !defined( _EnD_Of_Fw_global_config_h )
#error "Include Fw_global_config.h first"
#endif

#include "QL_Trace.h" /* for QL_STATUS */
#include "QL_SensorCommon.h" /* for QL_SAL_SENSOR_ID s */

#define ENABLE_SENSOR 1
#define DISABLE_SENSOR 0


/* Sensor range enum to set/get to Accel Sensor using Set_Sensor_Range/Get_Sensor_Range API*/
typedef enum {
    Accel_Sensor_Range_2G = 0,
    Accel_Sensor_Range_4G = 1,
    Accel_Sensor_Range_8G = 2,
    Accel_Sensor_Range_16G = 3,
    Accel_Sensor_Range_Max
}Accel_Sensor_Range_Value;

/* Sensor range enum to set/get to Gyro Sensor using Set_Sensor_Range/Get_Sensor_Range API*/
typedef enum {
    Gyro_Sensor_Range_2000DPS = 0,
    Gyro_Sensor_Range_1000DPS = 1,
    Gyro_Sensor_Range_500DPS = 2,
    Gyro_Sensor_Range_245DPS = 3,
    Gyro_Sensor_Range_125DPS = 4,
}Gyro_Sensor_Range_Value;  

/* enable/disable both Accel & Gyro */
QL_Status Set_AccelGyro_SensorEnable( int enable );

/* API can be used to set ODR value to both Accel and Gyro sensors. */
QL_Status Set_AccelGyro_SensorODR( unsigned int odr_val);

/* API is used to set Sensor Range.
 * Use range in Accel_Sensor_Range_Value enum to set range to Accel Sensor.
 * Use range in Gyro_Sensor_Range_Value enum to set range to Gyro Sensor.
 */
QL_Status Set_Sensor_Range(unsigned int sensor_id, unsigned int range);

/* API can be used to get ODR value to sensors.
 * Accel[id - QL_SAL_SENSOR_ID_ACCEL] Gyro[id - QL_SAL_SENSOR_ID_GYRO]
 */
QL_Status Get_Sensor_ODR(unsigned int sensor_id, unsigned int* odr_val);

/* API is used to get Sensor Range.
 * Consider output range as Accel_Sensor_Range_Value for Accel sensor.
 * Consider output range as Gyro_Sensor_Range_Value for Gyro Sensor.
 */
QL_Status Get_Sensor_Range(unsigned int sensor_id, unsigned int* range);

/*
 * Enable/Disable Sensor. 
 * This supports QL_SAL_SENSOR_ID_ACCEL and QL_SAL_SENSOR_ID_GYRO
 * To Enable, flag enable = ENABLE_SENSOR, To disble enable = DISABLE_SENSOR
 * Gyro can be enabled, only if Accel is already enabled.
 * And if Accel is disabling, Gyro will be disabled.
 */
QL_Status Sensor_Enable(unsigned int sensor_id, int enable);

#endif  /* __QL_SENSOR_ATTRIBUTES_H_ */
