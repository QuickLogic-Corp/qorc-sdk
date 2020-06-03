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
 *    File   : ql_SensorHub_MemoryMap.h
 *    Purpose: Header File which provides the details of the IOCTLs 
 *             available for the clients to use and for the Drivers to implement.
 *             Data Structures for specific IOCTLs if required, are also to be declared here.
 *                                                          
 * TODO: Description of each IOCTL, and the argument type to be passed in for each.
 *                                                          
 *=========================================================*/

#ifndef __QL_SENSOR_IOCTL_H__
#define __QL_SENSOR_IOCTL_H__

#include "Fw_global_config.h"
#include "QL_SensorCommon.h"

/* sentinel, arg = NULL */
#define QL_SAL_IOCTL_INVALID					(0)

/* arg = unsigned int* (1 = enable, 0 = disable) */
#define QL_SAL_IOCTL_ENABLE						(1)
/* arg = unsigned int* (1 = enable, 0 = disable) */
#define QL_SAL_IOCTL_GET_ENABLE					(2)

/* arg = QL_SF_Ioctl_Req_Events struct * (NULL = remove callback) */
/* QL_SensorEventCallback_t */
#define QL_SAL_IOCTL_SET_CALLBACK				(3)
/* arg = QL_SF_Ioctl_Req_Events struct * (NULL = no callback) */
/* QL_SensorEventCallback_t */
#define QL_SAL_IOCTL_GET_CALLBACK				(4)
typedef enum
{
	QL_SENSOR_EVENT_BATCH,
	QL_SENSOR_EVENT_DATA
} QL_SensorEvent_t;

struct QL_SensorEventInfo
{
	QL_SensorEvent_t event;
	unsigned int numpkt;
	void* data;
};
typedef QL_Status (*QL_SensorEventCallback_t) (void *cookie, struct QL_SensorEventInfo *event_info);


struct QL_SF_Ioctl_Req_Events
{
	void* cookie;
	QL_SensorEventCallback_t event_callback;
};


/* arg = unsigned int* (1 = enable, 0 = disable) */
#define QL_SAL_IOCTL_LIVE_DATA_ENABLE						(19)
/* arg = unsigned int* (1 = enable, 0 = disable) */
#define QL_SAL_IOCTL_GET_LIVE_DATA_ENABLE					(20)

/* arg = QL_SF_Ioctl_Set_Data_Buf struct* (NULL = release mem buffer) */
#define QL_SAL_IOCTL_SET_DATA_BUF				(5)
#define QL_SAL_IOCTL_ALLOC_DATA_BUF     		(6)
#define QL_SAL_IOCTL_GET_DATA_BUF     			(7)
struct QL_SF_Ioctl_Set_Data_Buf
{
	void* buf; // live sensor data
	unsigned int  size;	// size is now ignored, can change later.
};


/* arg = unsigned int* */
#define QL_SAL_IOCTL_BATCH_ENABLE				(8)
/* arg = unsigned int* */
#define QL_SAL_IOCTL_BATCH_GET_ENABLE			(9)

/* arg = QL_SF_Ioctl_Set_Batch_Data_Buf* */
#define QL_SAL_IOCTL_SET_BATCH					(10)
/* arg = QL_SF_Ioctl_Set_Batch_Data_Buf* */
#define QL_SAL_IOCTL_GET_BATCH					(11)
struct QL_SF_Ioctl_Set_Batch_Data_Buf
{
	void* batch_mem_start;				// 32 bit values (not Q15.16)
	void* batch_mem_end;				// 32 bit values (not Q15.16)
	unsigned int batchSize; 			// Batch size is number of packets
	void* batch_info_memPtr; 			// 32 bit values (not Q15.16)
};


/* arg = NULL always */
#define QL_SAL_IOCTL_BATCH_FLUSH				(12)

/* arg = unsigned int* */
#define QL_SAL_IOCTL_SET_DYNAMIC_RANGE			(15)
/* arg = unsigned int* */
#define QL_SAL_IOCTL_GET_DYNAMIC_RANGE			(16)

/* arg = unsigned int* */
#define QL_SAL_IOCTL_SET_ODR					(17)
/* arg = unsigned int* */
#define QL_SAL_IOCTL_GET_ODR					(18)

/* arg = unsigned int* */
#define QL_SAL_IOCTL_SET_BATCH_PACKET_IDS      	(21)

/* arg = unsigned int* */
#define QL_SAL_IOCTL_GET_BATCH_PACKET_IDS      	(22)

/* Supported ODR range to FFE sensors through IOCTL API */
#ifndef MIN_SUPPORTED_SENSOR_ODR
#define MIN_SUPPORTED_SENSOR_ODR 25
#endif
#ifndef MAX_SUPPORTED_SENSOR_ODR
#define MAX_SUPPORTED_SENSOR_ODR 1600
#endif


#endif	/*__QL_SENSOR_IOCTL_H__*/
