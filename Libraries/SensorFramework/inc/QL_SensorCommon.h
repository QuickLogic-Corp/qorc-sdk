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
 *    File   : QL_SensorCommon.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __QL_SENSOR_COMMON_H__
#define __QL_SENSOR_COMMON_H__

#include "QL_Trace.h"

/*
 * Header File to declare the interfaces that both Clients and Drivers would use.
 * The Sensor IDs used by the Sensor Framework are defined here.
 * The Sensor Framework Initialization API is placed here.
 */

enum
{
	QL_SAL_SENSOR_ID_INVALID,
	QL_SAL_SENSOR_ID_ACCEL,
	QL_SAL_SENSOR_ID_ACCEL2,
	QL_SAL_SENSOR_ID_DOUBLE_TAP,
	QL_SAL_SENSOR_ID_PCG,
	QL_SAL_SENSOR_ID_GYRO,
    QL_SAL_SENSOR_ID_MAG,
    QL_SAL_SENSOR_ID_SENSIML_APP1,
    QL_SAL_SENSOR_ID_SENSIML_APP2
};

#define __PLATFORM_INIT__							

/* Sensor Framework API Init : This *must* be called after all drivers register themselves to the framework */
QL_Status QL_SensorFramework_Init(void);                                                

/*ql_SensorHubTimerCallback declarations*/
QL_Status vSensorHubCallbackInit(unsigned int periodInms);
void vSensorHubChangeTimerPeriod(unsigned int timeInms);
void vSensorHubStartTimer();
void vSensorHubStopTimer();

#endif /*__QL_SENSOR_COMMON_H__*/
