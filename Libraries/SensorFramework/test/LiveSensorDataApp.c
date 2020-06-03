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
 *    File   : LiveSensorDataApp.c
 *    Purpose: 
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#include "common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "QL_Trace.h"
#include "QL_SAL.h"
#include "QL_SensorIoctl.h"
#include "QL_SDF.h"


#include <ffe_ipc_lib.h>
#include <ql_SensorHub_MemoryMap.h>

#include "Fw_global_config.h"
#include "LiveSensorDataApp.h"

#ifdef HOST_SENSOR

/* future use - if needed to consolidate all Queues and Tasks at one place */
extern TaskHandle_t xHandleTaskLiveSensorDataHandler;
extern QueueHandle_t xHandleQueueLiveSensorDataHandler;
void vLiveSensorDataHandlerTaskHandler( void *pvParameters );

#define LIVE_SENSOR_DATA_START			1
#define LIVE_SENSOR_DATA_STOP			2

struct xLiveSensorDataPacket
{
	uint8_t command;
};

struct sensorData_Accel
{
	unsigned int timestamp;
	unsigned short len;
	signed short int data[3];
};

struct sensorData_Mag
{
	unsigned int timestamp;
	unsigned short len;
	signed short int data[3];
};

struct sensorData_Gyro
{
	unsigned int timestamp;
	unsigned short len;
	signed short int data[3];
};




/* Task and Queue Handles for this task */
TaskHandle_t xHandleTaskLiveSensorDataHandler;
QueueHandle_t xHandleQueueLiveSensorDataHandler;


QL_SAL_SensorHandle_t sensorHandle_Accel;
QL_SAL_SensorHandle_t sensorHandle_Accel2;
QL_SAL_SensorHandle_t sensorHandle_Mag;
QL_SAL_SensorHandle_t sensorHandle_Gyro;



/* temporary variables for GET IOCTLs */
static unsigned int temp = 0;
static struct QL_SF_Ioctl_Set_Data_Buf tempDataBufInfo;
static QL_SensorEventCallback_t tempCallback;

static struct QL_SF_Ioctl_Set_Data_Buf databufinfo;

static unsigned int enable;
#if ( !defined(USE_INTERRUPTS_FOR_LIVE_DATA) || defined(PURE_M4_DRIVERS) )
static unsigned int highest_odr;
static unsigned int poll_time_ms;
static uint8_t* sensorDataBuffer;
#endif /*  !defined(USE_INTERRUPTS_FOR_LIVE_DATA) || defined(PURE_M4_DRIVERS) )*/

unsigned int LiveSensorApp_sensorDynamicRange_Accel;
unsigned int LiveSensorApp_sensorODR_Accel;
unsigned int LiveSensorApp_enable_Accel;
float LiveSensorApp_scaling_factor_Accel;

//unsigned int LiveSensorApp_sensorDynamicRange_Accel2; (range is shared with Accel, cannot be different)
unsigned int LiveSensorApp_sensorODR_Accel2;
unsigned int LiveSensorApp_enable_Accel2;
float LiveSensorApp_scaling_factor_Accel2;

//unsigned int LiveSensorApp_sensorDynamicRange_Mag; (range is N.A. for mag)
unsigned int LiveSensorApp_sensorODR_Mag;
unsigned int LiveSensorApp_enable_Mag;
float LiveSensorApp_scaling_factor_Mag;

unsigned int LiveSensorApp_sensorDynamicRange_Gyro;
unsigned int LiveSensorApp_sensorODR_Gyro;
unsigned int LiveSensorApp_enable_Gyro;
float LiveSensorApp_scaling_factor_Gyro;

/* public function */
/* add a message work packet to this task's queue from an ISR context */
uint32_t ulAddPacketToLiveSensorDataHandlerQueueFromISR(struct xLiveSensorDataPacket *pxMsg)
{
	uint32_t ulErrCode = 0;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if( xQueueSendFromISR(xHandleQueueLiveSensorDataHandler, ( void * )pxMsg, &xHigherPriorityTaskWoken ) != pdPASS )
	{
		ulErrCode = 0xFFFF;
	}

	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

	return ulErrCode;
}


/* public function */
/* add a message work packet to this task's queue from a task context (not used currently) */
uint32_t ulAddPacketToLiveSensorDataHandlerQueue(struct xLiveSensorDataPacket *pxMsg)
{
	uint32_t ulErrCode = 0;

	if( xQueueSend( xHandleQueueLiveSensorDataHandler, ( void * )pxMsg, ( TickType_t )10 ) != pdPASS )
		ulErrCode = 0xFFFF;

	return ulErrCode;
}


void StartLiveSensorApp()
{
	struct xLiveSensorDataPacket msg;

	msg.command = LIVE_SENSOR_DATA_START;

	ulAddPacketToLiveSensorDataHandlerQueue(&msg);

}


/* public: this creates and inits the AP Command Handler Task and all dependencies*/
portBASE_TYPE xInitLiveSensorDataHandlerTask()
{
	static uint8_t ucParameterToPass;


	/* Create queue for AP Task */
	xHandleQueueLiveSensorDataHandler = xQueueCreate( 1024, sizeof(struct xLiveSensorDataPacket) );
	if(xHandleQueueLiveSensorDataHandler == 0)
	{
		//printf("[%s] : Error while creating queue\r\n", __func__);
		return pdFAIL;
	}

	/* Create AP Task */
	xTaskCreate (vLiveSensorDataHandlerTaskHandler, "LiveSensorDataHandlerTaskHandler", configMINIMAL_STACK_SIZE, &ucParameterToPass, tskIDLE_PRIORITY+17, &xHandleTaskLiveSensorDataHandler);
	configASSERT( xHandleTaskLiveSensorDataHandler );

	return pdPASS;
}


static QL_Status SensorEventCallback_Accel(void *cookie, struct QL_SensorEventInfo *event_info)
{
	struct sensorData_Accel sensorData;
//	QL_Status ret;
//	static int i;
	QL_TRACE_TEST_DEBUG("+\n");
	QL_ASSERT(event_info);

//	static int counter = 0;

	if (QL_SENSOR_EVENT_DATA == event_info->event)
	{
		float accel[3];


		sensorData = *((struct sensorData_Accel *)event_info->data);
		accel[0] = sensorData.data[0] * LiveSensorApp_scaling_factor_Accel;
		accel[1] = sensorData.data[1] * LiveSensorApp_scaling_factor_Accel;
		accel[2] = sensorData.data[2] * LiveSensorApp_scaling_factor_Accel;


		//printf("TS:[%u], [A] %d %d %d\n",sensorData.timestamp,sensorData.data[0],sensorData.data[1],sensorData.data[2]);
		printf("TS:[%u], [A] %f %f %f\n",sensorData.timestamp >> 8,
				accel[0],
				accel[1],
				accel[2]);




//		memcpy((void*)(sensorDataBuffer + (counter*sizeof(struct sensorData_Accel))), event_info->data, sizeof(struct sensorData_Accel));
//		counter++;
//
//		if(counter > 100)
//		{
//			LiveSensorApp_enable_Accel = 0;
//			ret = QL_SAL_SensorOpen(&sensorHandle_Accel, QL_SAL_SENSOR_ID_ACCEL);
//			QL_ASSERT(ret == QL_STATUS_OK);
//			printf("QL_SAL_SensorOpen OK.\n");
//
//			ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_SET_CALLBACK, (void *)0);
//			QL_ASSERT(ret == QL_STATUS_OK);
//			ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_GET_CALLBACK, (void *)&tempCallback);
//			QL_ASSERT(ret == QL_STATUS_OK);
//			printf("QL_SAL_IOCTL_GET_CALLBACK: 0x%x\n",tempCallback);
//
//			/* disable the sensor */
//			ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_ENABLE, (void *)&LiveSensorApp_enable_Accel);
//			QL_ASSERT(ret == QL_STATUS_OK);
//			ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_GET_ENABLE, &temp);
//			QL_ASSERT(ret == QL_STATUS_OK);
//			printf("QL_SAL_IOCTL_GET_ENABLE 0x%x\n",temp);
//			printf("\n\n");
//
//			/* close the sensor handle, we are done. */
//			ret = QL_SAL_SensorClose(sensorHandle_Accel);
//			QL_ASSERT(ret == QL_STATUS_OK);
//			printf("QL_SAL_SensorClose OK.\n");
//			printf("\n\n");
//
//			for (i = 0; i < counter; i++)
//			{
//				float accel[3];
//
//
//						sensorData = *((struct sensorData_Accel *)(sensorDataBuffer + (i*sizeof(struct sensorData_Accel)) ));
//						accel[0] = sensorData.data[0] * LiveSensorApp_scaling_factor_Accel;
//						accel[1] = sensorData.data[1] * LiveSensorApp_scaling_factor_Accel;
//						accel[2] = sensorData.data[2] * LiveSensorApp_scaling_factor_Accel;
//
//
//						//printf("TS:[%u], [A] %d %d %d\n",sensorData.timestamp,sensorData.data[0],sensorData.data[1],sensorData.data[2]);
//						printf("TS:[%u], [A] %f %f %f\n",sensorData.timestamp >> 8,
//								accel[0],
//								accel[1],
//								accel[2]);
//
//
//
//			}
//			vSensorHubStopTimer();
//			while(1);
//
//		}

	}

	QL_TRACE_TEST_DEBUG("-\n");
	return QL_STATUS_OK;                            // to remove warnings ,  added return statement
}

#if (defined (FFE_DRIVERS) || defined(HYBRID_FFE_M4_DRIVERS))
static QL_Status SensorEventCallback_Accel2(void *cookie, struct QL_SensorEventInfo *event_info)
{
	struct sensorData_Accel sensorData;
	QL_TRACE_TEST_DEBUG("+\n");
	QL_ASSERT(event_info);

	if (QL_SENSOR_EVENT_DATA == event_info->event)
	{
		float accel[3];


		sensorData = *((struct sensorData_Accel *)event_info->data);
		accel[0] = sensorData.data[0] * LiveSensorApp_scaling_factor_Accel2;
		accel[1] = sensorData.data[1] * LiveSensorApp_scaling_factor_Accel2;
		accel[2] = sensorData.data[2] * LiveSensorApp_scaling_factor_Accel2;


		//printf("TS:[%u], [A] %d %d %d\n",sensorData.timestamp,sensorData.data[0],sensorData.data[1],sensorData.data[2]);
		printf("TS:[%u], [A2] %f %f %f\n",sensorData.timestamp >> 8,
				accel[0],
				accel[1],
				accel[2]);

	}

	QL_TRACE_TEST_DEBUG("-\n");
	return QL_STATUS_OK;                            // to remove warnings ,  added return statement
}


static QL_Status SensorEventCallback_Mag(void *cookie, struct QL_SensorEventInfo *event_info)
{
	struct sensorData_Mag sensorData;

	float mag[3];


	sensorData = *((struct sensorData_Mag *)event_info->data);
	mag[0] = sensorData.data[0] * LiveSensorApp_scaling_factor_Mag;
	mag[1] = sensorData.data[1] * LiveSensorApp_scaling_factor_Mag;
	mag[2] = sensorData.data[2] * LiveSensorApp_scaling_factor_Mag;


	QL_TRACE_TEST_DEBUG("+\n");
	QL_ASSERT(event_info);

	if (QL_SENSOR_EVENT_DATA == event_info->event)
	{
		sensorData = *((struct sensorData_Mag *)event_info->data);
		printf("TS:[%u], [M] %f %f %f\n",sensorData.timestamp >> 8,mag[0],mag[1],mag[2]);

	}

	QL_TRACE_TEST_DEBUG("-\n");
	return QL_STATUS_OK;                            // to remove warnings ,  added return statement
}


static QL_Status SensorEventCallback_Gyro(void *cookie, struct QL_SensorEventInfo *event_info)
{
	struct sensorData_Gyro sensorData;

	float gyro[3];

	sensorData = *((struct sensorData_Gyro *)event_info->data);
	gyro[0] = sensorData.data[0] * LiveSensorApp_scaling_factor_Gyro;
	gyro[1] = sensorData.data[1] * LiveSensorApp_scaling_factor_Gyro;
	gyro[2] = sensorData.data[2] * LiveSensorApp_scaling_factor_Gyro;


	QL_TRACE_TEST_DEBUG("+\n");
	QL_ASSERT(event_info);

	if (QL_SENSOR_EVENT_DATA == event_info->event)
	{
		sensorData = *((struct sensorData_Gyro *)event_info->data);
		printf("TS:[%u], [G] %f %f %f\n",sensorData.timestamp >> 8,gyro[0],gyro[1],gyro[2]);

	}

	QL_TRACE_TEST_DEBUG("-\n");
	return QL_STATUS_OK;                            // to remove warnings ,  added return statement
}
#endif /*#if (defined (FFE_DRIVERS) || defined(HYBRID_FFE_M4_DRIVERS))*/

/* internal function: main handler for the AP Command Handler Task */
void vLiveSensorDataHandlerTaskHandler( void *pvParameters )
{

	BaseType_t 						xResult;
	struct xLiveSensorDataPacket       	xReceivedMsg;



	/* TODO do we add a task control variable here ? maybe useful to control the lifetime externally */
	for( ;; )
	{

		/*
		 * block until a command from AP is available.
		 * commands are pushed into this queue from ISR context
		 * when the AP raises the interrupt.
		 */
		xResult = xQueueReceive( xHandleQueueLiveSensorDataHandler, &( xReceivedMsg ), portMAX_DELAY);

		if( xResult == pdPASS )
		{
			switch(xReceivedMsg.command)
			{
			case LIVE_SENSOR_DATA_START:
			{
				QL_Status ret;
				printf("\n\nStarting Live Sensor Data Application\n\n");

				/* FFE Side, Accel Ranges:
				 *  	0 -> +-2G
				 *  	1 -> +-4G
				 *  	2 -> +-8G
				 *  	3 -> +-16G
				 *  	scaling factor = 32767 /x * 9.81 (x == 2/4/8/16)
				 */

				// ACCEL
				if(LiveSensorApp_sensorDynamicRange_Accel == 0)
				{
					LiveSensorApp_scaling_factor_Accel = 0.000598163;
					LiveSensorApp_scaling_factor_Accel2 = 0.000598163;
				}
				else if(LiveSensorApp_sensorDynamicRange_Accel == 1)
				{
					LiveSensorApp_scaling_factor_Accel = 0.001196326;
					LiveSensorApp_scaling_factor_Accel2 = 0.001196326;
				}
				else if(LiveSensorApp_sensorDynamicRange_Accel == 2)
				{
					LiveSensorApp_scaling_factor_Accel = 0.002392651;
					LiveSensorApp_scaling_factor_Accel2 = 0.002392651;
				}
				else if(LiveSensorApp_sensorDynamicRange_Accel == 3)
				{
					LiveSensorApp_scaling_factor_Accel = 0.004785302;
					LiveSensorApp_scaling_factor_Accel2 = 0.004785302;
				}

				// ACCEL2
				// actually has range set for ACCEL ! no separate range for Accel2.

				// MAG
				/* Mag : AK09915, range is fixed
				 * 32752 (max reading) == 4912 (uT)
				 * 16 bit signed number, so 32767 == 4912
				 * scaling factor = 4912/32767 = 0.1499				 *
				 */
				LiveSensorApp_scaling_factor_Mag = 0.1499;


				// GYRO
				/*
				 * FFE Side, Ranges:
				 * 0 -> 2000 dps
				 * 1 -> 1000 dps
				 * 2 -> 500 dps
				 * 3 -> 250 dps
				 * 4 -> 125 dps
				 *
				 * scaling factor = 32767/ x * dps (x == 2000/1000/500/250/125)
				 */
				if(LiveSensorApp_sensorDynamicRange_Gyro == 0)
				{
					LiveSensorApp_scaling_factor_Gyro = 0.0610;
				}
				else if(LiveSensorApp_sensorDynamicRange_Gyro == 1)
				{
					LiveSensorApp_scaling_factor_Gyro = 0.0305;
				}
				else if(LiveSensorApp_sensorDynamicRange_Gyro == 2)
				{
					LiveSensorApp_scaling_factor_Gyro = 0.0153;
				}
				else if(LiveSensorApp_sensorDynamicRange_Gyro == 3)
				{
					LiveSensorApp_scaling_factor_Gyro = 0.0076;
				}
				else if(LiveSensorApp_sensorDynamicRange_Gyro == 4)
				{
					LiveSensorApp_scaling_factor_Gyro = 0.0038;
				}

				//////////////////////////////////////////////////////////////////////////////
				if(LiveSensorApp_enable_Accel)
				{
					printf("Sensor ID: %d\n",QL_SAL_SENSOR_ID_ACCEL);
					ret = QL_SAL_SensorOpen(&sensorHandle_Accel, QL_SAL_SENSOR_ID_ACCEL);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_SensorOpen OK.\n");

					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_SET_CALLBACK, (void *)SensorEventCallback_Accel);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_GET_CALLBACK, (void *)&tempCallback);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_CALLBACK: 0x%x\n",tempCallback);
					printf("CallBack Function Address: 0x%x\n",SensorEventCallback_Accel);
					printf("\n\n");

					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_SET_DYNAMIC_RANGE, (void*)&LiveSensorApp_sensorDynamicRange_Accel);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_GET_DYNAMIC_RANGE, &temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_DYNAMIC_RANGE 0x%x\n",temp);
					printf("\n\n");

					//printf("accel_scaling_factor: %f\n",Get_Accel_Scale_Factor());

					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_SET_ODR, (void*)&LiveSensorApp_sensorODR_Accel);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_GET_ODR, &temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_ODR 0x%x\n",temp);
					printf("\n\n");

					/* in batch mode, we have a pre-fixed batch buffer */
					/* in non batch mode, we have a pre-fixed SRAM address to read data */
					/* so, deprecating this for now, look at it if this is good to handle large buffer allocation ? */

					//				ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_ALLOC_DATA_BUF, (void*)&databufinfo);
					//				QL_ASSERT(ret == QL_STATUS_OK);
					//				printf("QL_SAL_IOCTL_ALLOC_DATA_BUF 0x%x, 0x%x\n",databufinfo.buf,databufinfo.size);
					//				printf("\n\n");

					databufinfo.buf = (void *)LIVE_SENSOR_DATA_ACCEL_ADDR;							/* LIVE DATA ADDR */
					databufinfo.size = LIVE_SENSOR_DATA_PACKET_SIZE;						/* LIVE DATA PACKET SIZE */

					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_SET_DATA_BUF, (void*)&databufinfo );
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_SET_DATA_BUF 0x%x\n",databufinfo.buf);
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_GET_DATA_BUF, (void*)&tempDataBufInfo);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_DATA_BUF 0x%x\n",tempDataBufInfo.buf);
					printf("\n\n");

					enable = 1;
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_LIVE_DATA_ENABLE, (void*)&enable);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_GET_LIVE_DATA_ENABLE, (void*)&temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_LIVE_DATA_ENABLE 0x%x\n",temp);
					printf("\n\n");

				}
				//////////////////////////////////////////////////////////////////////////////


#if (defined (FFE_DRIVERS) || defined(HYBRID_FFE_M4_DRIVERS))
				//////////////////////////////////////////////////////////////////////////////
				if(LiveSensorApp_enable_Accel2)
				{
					printf("Sensor ID: %d\n",QL_SAL_SENSOR_ID_ACCEL2);
					ret = QL_SAL_SensorOpen(&sensorHandle_Accel2, QL_SAL_SENSOR_ID_ACCEL2);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_SensorOpen OK.\n");

					ret = QL_SAL_SensorIoctl(sensorHandle_Accel2, QL_SAL_IOCTL_SET_CALLBACK, (void *)SensorEventCallback_Accel2);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel2, QL_SAL_IOCTL_GET_CALLBACK, (void *)&tempCallback);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_CALLBACK: 0x%x\n",tempCallback);
					printf("CallBack Function Address: 0x%x\n",SensorEventCallback_Accel2);
					printf("\n\n");

					ret = QL_SAL_SensorIoctl(sensorHandle_Accel2, QL_SAL_IOCTL_SET_DYNAMIC_RANGE, (void*)&LiveSensorApp_sensorDynamicRange_Accel);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel2, QL_SAL_IOCTL_GET_DYNAMIC_RANGE, &temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_DYNAMIC_RANGE 0x%x\n",temp);
					printf("\n\n");

					//printf("accel_scaling_factor: %f\n",Get_Accel_Scale_Factor());

					ret = QL_SAL_SensorIoctl(sensorHandle_Accel2, QL_SAL_IOCTL_SET_ODR, (void*)&LiveSensorApp_sensorODR_Accel2);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel2, QL_SAL_IOCTL_GET_ODR, &temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_ODR 0x%x\n",temp);
					printf("\n\n");

					/* in batch mode, we have a pre-fixed batch buffer */
					/* in non batch mode, we have a pre-fixed SRAM address to read data */
					/* so, deprecating this for now, look at it if this is good to handle large buffer allocation ? */

					//				ret = QL_SAL_SensorIoctl(sensorHandle_Accel2, QL_SAL_IOCTL_ALLOC_DATA_BUF, (void*)&databufinfo);
					//				QL_ASSERT(ret == QL_STATUS_OK);
					//				printf("QL_SAL_IOCTL_ALLOC_DATA_BUF 0x%x, 0x%x\n",databufinfo.buf,databufinfo.size);
					//				printf("\n\n");

					databufinfo.buf = (void *)LIVE_SENSOR_DATA_ACCEL2_ADDR;							/* LIVE DATA ADDR */
					databufinfo.size = LIVE_SENSOR_DATA_PACKET_SIZE;						/* LIVE DATA PACKET SIZE */

					ret = QL_SAL_SensorIoctl(sensorHandle_Accel2, QL_SAL_IOCTL_SET_DATA_BUF, (void*)&databufinfo );
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_SET_DATA_BUF 0x%x\n",databufinfo.buf);
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel2, QL_SAL_IOCTL_GET_DATA_BUF, (void*)&tempDataBufInfo);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_DATA_BUF 0x%x\n",tempDataBufInfo.buf);
					printf("\n\n");

					enable = 1;
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel2, QL_SAL_IOCTL_LIVE_DATA_ENABLE, (void*)&enable);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(sensorHandle_Accel2, QL_SAL_IOCTL_GET_LIVE_DATA_ENABLE, (void*)&temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_LIVE_DATA_ENABLE 0x%x\n",temp);
					printf("\n\n");

				}
				//////////////////////////////////////////////////////////////////////////////


				//////////////////////////////////////////////////////////////////////////////
				if(LiveSensorApp_enable_Mag)
				{
					printf("Sensor ID: %d\n",QL_SAL_SENSOR_ID_MAG);
					ret = QL_SAL_SensorOpen(&sensorHandle_Mag, QL_SAL_SENSOR_ID_MAG);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_SensorOpen OK.\n");

					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_SET_CALLBACK, (void *)SensorEventCallback_Mag);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_GET_CALLBACK, (void *)&tempCallback);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_CALLBACK: 0x%x\n",tempCallback);
					printf("CallBack Function Address: 0x%x\n",SensorEventCallback_Mag);
					printf("\n\n");

					//				ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_SET_DYNAMIC_RANGE, (void*)&sensorDynamicRange_Mag);
					//				QL_ASSERT(ret == QL_STATUS_OK);
					//				ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_GET_DYNAMIC_RANGE, &temp);
					//				QL_ASSERT(ret == QL_STATUS_OK);
					//				printf("QL_SAL_IOCTL_GET_DYNAMIC_RANGE 0x%x\n",temp);
					//				printf("\n\n");

					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_SET_ODR, (void*)&LiveSensorApp_sensorODR_Mag);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_GET_ODR, &temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_ODR 0x%x\n",temp);
					printf("\n\n");

					/* in batch mode, we have a pre-fixed batch buffer */
					/* in non batch mode, we have a pre-fixed SRAM address to read data */
					/* so, deprecating this for now, look at it if this is good to handle large buffer allocation ? */

					//				ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_ALLOC_DATA_BUF, (void*)&databufinfo);
					//				QL_ASSERT(ret == QL_STATUS_OK);
					//				printf("QL_SAL_IOCTL_ALLOC_DATA_BUF 0x%x, 0x%x\n",databufinfo.buf,databufinfo.size);
					//				printf("\n\n");

					databufinfo.buf = (void *)LIVE_SENSOR_DATA_MAG_ADDR;							/* LIVE DATA ADDR */
					databufinfo.size = LIVE_SENSOR_DATA_PACKET_SIZE;						/* LIVE DATA PACKET SIZE */

					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_SET_DATA_BUF, (void*)&databufinfo );
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_SET_DATA_BUF 0x%x\n",databufinfo.buf);
					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_GET_DATA_BUF, (void*)&tempDataBufInfo);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_DATA_BUF 0x%x\n",tempDataBufInfo.buf);
					printf("\n\n");

					enable = 1;
					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_LIVE_DATA_ENABLE, (void*)&enable);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_GET_LIVE_DATA_ENABLE, (void*)&temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_LIVE_DATA_ENABLE 0x%x\n",temp);
					printf("\n\n");

				}
				//////////////////////////////////////////////////////////////////////////////


				//////////////////////////////////////////////////////////////////////////////
				if(LiveSensorApp_enable_Gyro)
				{
					printf("Sensor ID: %d\n",QL_SAL_SENSOR_ID_GYRO);
					ret = QL_SAL_SensorOpen(&sensorHandle_Gyro, QL_SAL_SENSOR_ID_GYRO);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_SensorOpen OK.\n");

					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_SET_CALLBACK, (void *)SensorEventCallback_Gyro);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_GET_CALLBACK, (void *)&tempCallback);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_CALLBACK: 0x%x\n",tempCallback);
					printf("CallBack Function Address: 0x%x\n",SensorEventCallback_Gyro);
					printf("\n\n");

					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_SET_DYNAMIC_RANGE, (void*)&LiveSensorApp_sensorDynamicRange_Gyro);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_GET_DYNAMIC_RANGE, &temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_DYNAMIC_RANGE 0x%x\n",temp);
					printf("\n\n");

					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_SET_ODR, (void*)&LiveSensorApp_sensorODR_Gyro);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_GET_ODR, &temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_ODR 0x%x\n",temp);
					printf("\n\n");

					/* in batch mode, we have a pre-fixed batch buffer */
					/* in non batch mode, we have a pre-fixed SRAM address to read data */
					/* so, deprecating this for now, look at it if this is good to handle large buffer allocation ? */

					//				ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_ALLOC_DATA_BUF, (void*)&databufinfo);
					//				QL_ASSERT(ret == QL_STATUS_OK);
					//				printf("QL_SAL_IOCTL_ALLOC_DATA_BUF 0x%x, 0x%x\n",databufinfo.buf,databufinfo.size);
					//				printf("\n\n");

					databufinfo.buf = (void *)LIVE_SENSOR_DATA_GYRO_ADDR;							/* LIVE DATA ADDR */
					databufinfo.size = LIVE_SENSOR_DATA_PACKET_SIZE;						/* LIVE DATA PACKET SIZE */

					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_SET_DATA_BUF, (void*)&databufinfo );
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_SET_DATA_BUF 0x%x\n",databufinfo.buf);
					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_GET_DATA_BUF, (void*)&tempDataBufInfo);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_DATA_BUF 0x%x\n",tempDataBufInfo.buf);
					printf("\n\n");

					enable = 1;
					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_LIVE_DATA_ENABLE, (void*)&enable);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_GET_LIVE_DATA_ENABLE, (void*)&temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_LIVE_DATA_ENABLE 0x%x\n",temp);
					printf("\n\n");

				}
				//////////////////////////////////////////////////////////////////////////////
#endif // #if (defined (FFE_DRIVERS) || defined(HYBRID_FFE_M4_DRIVERS))


				if(LiveSensorApp_enable_Accel)
				{
					/* enable the sensor */
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_ENABLE, (void *)&LiveSensorApp_enable_Accel);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_GET_ENABLE, &temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_ENABLE 0x%x\n",temp);
					printf("\n\n");

					/* close the sensor handle, we are done. */
					ret = QL_SAL_SensorClose(sensorHandle_Accel);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_SensorClose OK.\n");
					printf("\n\n");

				}

#if (defined (FFE_DRIVERS) || defined(HYBRID_FFE_M4_DRIVERS))
				if(LiveSensorApp_enable_Accel2)
				{
					/* enable the sensor */
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel2, QL_SAL_IOCTL_ENABLE, (void *)&LiveSensorApp_enable_Accel2);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel2, QL_SAL_IOCTL_GET_ENABLE, &temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_ENABLE 0x%x\n",temp);
					printf("\n\n");

					/* close the sensor handle, we are done. */
					ret = QL_SAL_SensorClose(sensorHandle_Accel2);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_SensorClose OK.\n");
					printf("\n\n");
				}

				if(LiveSensorApp_enable_Mag)
				{
					/* enable the sensor */
					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_ENABLE, (void *)&LiveSensorApp_enable_Mag);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_GET_ENABLE, &temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_ENABLE 0x%x\n",temp);
					printf("\n\n");

					/* close the sensor handle, we are done. */
					ret = QL_SAL_SensorClose(sensorHandle_Mag);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_SensorClose OK.\n");
					printf("\n\n");
				}


				if(LiveSensorApp_enable_Gyro)
				{
					/* enable the sensor */
					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_ENABLE, (void *)&LiveSensorApp_enable_Gyro);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_GET_ENABLE, &temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_ENABLE 0x%x\n",temp);
					printf("\n\n");

					/* close the sensor handle, we are done. */
					ret = QL_SAL_SensorClose(sensorHandle_Gyro);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_SensorClose OK.\n");
					printf("\n\n");
				}
#endif // (defined (FFE_DRIVERS) || defined(HYBRID_FFE_M4_DRIVERS))

#if( !defined(USE_INTERRUPTS_FOR_LIVE_DATA) || defined(PURE_M4_DRIVERS) )

				if(LiveSensorApp_sensorODR_Accel > highest_odr)
				{
					highest_odr = LiveSensorApp_sensorODR_Accel;
				}
				if(LiveSensorApp_sensorODR_Accel2 > highest_odr)
				{
					highest_odr = LiveSensorApp_sensorODR_Accel2;
				}
				if(LiveSensorApp_sensorODR_Gyro > highest_odr)
				{
					highest_odr = LiveSensorApp_sensorODR_Gyro;
				}
				if(LiveSensorApp_sensorODR_Mag > highest_odr)
				{
					highest_odr = LiveSensorApp_sensorODR_Mag;
				}

				poll_time_ms = 1000/highest_odr;
				if(poll_time_ms < 1) poll_time_ms = 1;

				if(QL_SDF_SensorAllocMem(QL_SAL_SENSOR_ID_ACCEL, (void **)&sensorDataBuffer, highest_odr*30) != QL_STATUS_OK)
				{
					printf("could not alloc.\n");
					QL_ASSERT(0);
				}

				printf("poll_time_ms: %u\n",poll_time_ms);

				vSensorHubCallbackInit(poll_time_ms);
				//vSensorHubChangeTimerPeriod(poll_time_ms); // this is the fastest we could possibly go, practically more like 10ms+
				vSensorHubStartTimer();
#endif // ( !defined(USE_INTERRUPTS_FOR_LIVE_DATA) || defined(PURE_M4_DRIVERS) )

			}
			break;

			case LIVE_SENSOR_DATA_STOP:
			{

			}
			break;

			default:
			{

			}
			break;
			}
		}
	}
}

#endif /* HOST_SENSOR */
