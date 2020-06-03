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
 *    File   : QL_SAL.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __QL_SAL_H__
#define __QL_SAL_H__
#include "QL_Trace.h"
#include "QL_SensorCommon.h"

/*
 * Header File to declare the interface for Clients who wish to interact with the Sensors.
 * Each Client should:
 * 1. Attempt to obtain a handle to the Sensor using QL_SAL_SensorOpen()
 *		The client *must* check the return value(QL_STATUS_OK) to see if it could obtain a valid handle (QL_SAL_SensorHandle_t)
 * 2. Use the  QL_SAL_SensorRead() and QL_SAL_SensorIoctl() passing in the obtained handle.
 * 3. Once done with a sensor, release the obtained handle back to the framework using QL_SAL_SensorClose()
 *
 * Description of the IOCTLs available to the client varies with the particular Sensor, and has to be documented
 * for each Sensor Driver. Refer to the individual Sensor Driver documentation.
 */


/* opaque handle for clients for interaction with Sensors */
struct QL_SAL_SensorHandle;
typedef struct QL_SAL_SensorHandle *QL_SAL_SensorHandle_t;


/* Sensor Framework Functions for client interaction with Sensors */
QL_Status QL_SAL_SensorOpen(QL_SAL_SensorHandle_t *handle, unsigned int id);
QL_Status QL_SAL_SensorClose(QL_SAL_SensorHandle_t handle);
QL_Status QL_SAL_SensorRead(QL_SAL_SensorHandle_t handle, void *buf, int size);
QL_Status QL_SAL_SensorIoctl(QL_SAL_SensorHandle_t handle, unsigned int cmd, void *arg);

#endif /* __QL_SAL_H__ */
