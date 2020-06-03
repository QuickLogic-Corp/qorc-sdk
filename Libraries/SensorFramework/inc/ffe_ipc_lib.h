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
 *    File   : ffe_ipc_lib.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __FFE_IPC_LIB_H__
#define __FFE_IPC_LIB_H__
#include "QL_SensorCommon.h"



/*
 * FFE Lib Interface for "pseudo" drivers from M4 need:
 *
 * open		-> no implementation needed on FFE side?
 * close	-> no implementation needed on FFE side?
 * read		-> no implementation needed on FFE side?
 * probe	-> FFE Probe will be called, can also have a M4 version
 * suspend	-> no implementation needed on FFE side?
 * resume	-> no implementation needed on FFE side?
 * ioctl	-> FFE specific implementation for commands, so mimic the SAL's IOCTLs itself, and re-use the same structure.
 *
 */

/*
 * FFE Lib Interface also provides a higher level of control:
 * From an AP's perspective, all the sensors/algorithms can be disabled or enabled at once
 * as well as the period can be set.
 * sensor task control -> enable/disable, set/get period (all sensors)
 * application task control -> enable/disable, set/get period (all algorithms)
 *
 */


/* Operation Codes and Argument Format Structures for specific Operations are needed here */
/* FFE Global Ops and FFE Sensor Ops have separate API and separate Ops Codes and Argument Format Structures */

/* sensor ops */

/* sentinel, arg = NULL */
#define QL_FFE_SENSOR_OPS_INVALID						(0)

/* arg = unsigned int* (1 = enable, 0 = disable) */
#define QL_FFE_SENSOR_OPS_ENABLE						(1)
/* arg = unsigned int* */
#define QL_FFE_SENSOR_OPS_GET_ENABLE					(2)


/* arg = unsigned int* (1 = enable, 0 = disable) */
#define QL_FFE_SENSOR_OPS_LIVE_DATA_ENABLE				(3)
/* arg = unsigned int* */
#define QL_FFE_SENSOR_OPS_GET_LIVE_DATA_ENABLE			(4)

/* arg = QL_SF_Ioctl_Set_Data_Buf struct* (NULL = release mem buffer) */
#define QL_FFE_SENSOR_OPS_SET_MEM_BUFFER				(5)
/* arg = QL_SF_Ioctl_Set_Data_Buf struct * */
#define QL_FFE_SENSOR_OPS_GET_MEM_BUFFER				(6)


/* arg = QL_SF_Ioctl_Set_Batch_Data_Buf* */
#define QL_FFE_SENSOR_OPS_SET_BATCH						(7)
/* arg = QL_SF_Ioctl_Set_Batch_Data_Buf* */
#define QL_FFE_SENSOR_OPS_GET_BATCH						(8)

/* arg = unsigned int*, (NULL = batch off) */
#define QL_FFE_SENSOR_OPS_BATCH_ENABLE					(9)
/* arg = unsigned int*, (NULL = batch off) */
#define QL_FFE_SENSOR_OPS_BATCH_GET_ENABLE				(10)

/* arg = NULL always */
#define QL_FFE_SENSOR_OPS_BATCH_FLUSH					(11)


/* arg = unsigned int* */
#define QL_FFE_SENSOR_OPS_SET_DYNAMIC_RANGE				(14)
/* arg = unsigned int* */
#define QL_FFE_SENSOR_OPS_GET_DYNAMIC_RANGE				(15)

/* arg = unsigned int* */
#define QL_FFE_SENSOR_OPS_SET_ODR						(16)
/* arg = unsigned int* */
#define QL_FFE_SENSOR_OPS_GET_ODR						(17)

/* arg = unsigned int* */
#define QL_FFE_SENSOR_OPS_SET_BATCH_PACKET_IDS      		(18)
/* arg = unsigned int* */
#define QL_FFE_SENSOR_OPS_GET_BATCH_PACKET_IDS	          	(19)

/* sensor ops data structures */

/* sensor ops API */
QL_Status QL_FFE_OperationForSensor(unsigned int sensorID, unsigned int operation, void* arg);
QL_Status QL_FFE_Sensor_Probe(unsigned int sensorid);


/* global ops */

/* arg = unsigned int* */
#define QL_FFE_OPS_FFE_ENABLE							(1)
/* arg = unsigned int* */
#define QL_FFE_OPS_FFE_GET_ENABLE						(2)
/* arg = unsigned int* */
#define QL_FFE_OPS_FFE_SET_PERIOD						(3)
/* arg = unsigned int* */
#define QL_FFE_OPS_FFE_GET_PERIOD						(4)
/* arg = unsigned int*, 1 == HALT, 0 == RELEASE */
#define QL_FFE_OPS_FFE_HALT								(5)


/* global ops API */
QL_Status QL_FFE_OperationGlobal(unsigned int operation, void* arg);
void FFE_Halt(void);
void FFE_Release(void);
int FFE_IsBusy(void);
unsigned int FFE_GetEnableSPT();

void QL_FFE_GiveControlOfI2CBus();
void QL_FFE_TakeControlOfI2CBus();

QL_Status ffe_ipc_library_init(void);
void QL_FFE0MSG_ISR(void);

#endif /*__FFE_IPC_LIB_H__*/
