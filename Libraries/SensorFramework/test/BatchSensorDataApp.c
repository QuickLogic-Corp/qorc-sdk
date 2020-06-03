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
 *    File   : BatchSensorDataApp.c
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


#include <ffe_ipc_lib.h>
#include <ql_SensorHub_MemoryMap.h>

#include "Fw_global_config.h"
#include "BatchSensorDataApp.h"

#ifdef HOST_SENSOR

/* future use - if needed to consolidate all Queues and Tasks at one place */
extern TaskHandle_t xHandleTaskBatchSensorDataHandler;
extern QueueHandle_t xHandleQueueBatchSensorDataHandler;
void vBatchSensorDataHandlerTaskHandler( void *pvParameters );

#define BATCH_SENSOR_DATA_START			1
#define BATCH_SENSOR_DATA_STOP			2
#define	BATCH_SENSOR_DATA_FLUSH			3

struct xBatchSensorDataPacket
{
	uint8_t command;
};


static QL_SAL_SensorHandle_t sensorHandle_Accel;
static QL_SAL_SensorHandle_t sensorHandle_Mag;
static QL_SAL_SensorHandle_t sensorHandle_Gyro;


static QL_SensorEventCallback_t tempCallback;

static struct QL_SF_Ioctl_Set_Batch_Data_Buf batch_databuf_info, tempbatch_databuf_info;;

static unsigned int enable;

static unsigned int sensorDynamicRange_Accel = 0x2,sensorDynamicRange_Gyro = 0x2;
static unsigned int sensorODR_Accel = 1600,sensorODR_Mag=50, sensorODR_Gyro=50;



/* Task and Queue Handles for this task */
TaskHandle_t xHandleTaskBatchSensorDataHandler;
QueueHandle_t xHandleQueueBatchSensorDataHandler;


/* public function */
/* add a message work packet to this task's queue from an ISR context */
uint32_t ulAddPacketToBatchSensorDataHandlerQueueFromISR(struct xBatchSensorDataPacket *pxMsg)
{
	uint32_t ulErrCode = 0;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if( xQueueSendFromISR(xHandleQueueBatchSensorDataHandler, ( void * )pxMsg, &xHigherPriorityTaskWoken ) != pdPASS )
	{
		ulErrCode = 0xFFFF;
	}

	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

	return ulErrCode;
}


/* public function */
/* add a message work packet to this task's queue from a task context (not used currently) */
uint32_t ulAddPacketToBatchSensorDataHandlerQueue(struct xBatchSensorDataPacket *pxMsg)
{
	uint32_t ulErrCode = 0;

	if( xQueueSend( xHandleQueueBatchSensorDataHandler, ( void * )pxMsg, ( TickType_t )10 ) != pdPASS )
		ulErrCode = 0xFFFF;

	return ulErrCode;
}


/* public: this creates and inits the AP Command Handler Task and all dependencies*/
portBASE_TYPE xInitBatchSensorDataHandlerTask()
{
	static uint8_t ucParameterToPass;


	/* Create queue for AP Task */
	xHandleQueueBatchSensorDataHandler = xQueueCreate( 100, sizeof(struct xBatchSensorDataPacket) );
	if(xHandleQueueBatchSensorDataHandler == 0)
	{
		//printf("[%s] : Error while creating queue\r\n", __func__);
		return pdFAIL;
	}

	/* Create AP Task */
	xTaskCreate (vBatchSensorDataHandlerTaskHandler, "BatchSensorDataHandlerTaskHandler", configMINIMAL_STACK_SIZE, &ucParameterToPass, tskIDLE_PRIORITY+18, &xHandleTaskBatchSensorDataHandler);
	configASSERT( xHandleTaskBatchSensorDataHandler );

	return pdPASS;
}

struct SensorAccel_Data
{
	unsigned int timestamp;
	unsigned short len;
	signed short int data[3];
};

//#define ADATA_BATCH_PRINT
//#define MDATA_BATCH_PRINT
//#define GDATA_BATCH_PRINT

#ifdef ADATA_BATCH_PRINT
static const float Ascaling = 0.0023926; /* scaling factor derived for dynamic range = +-8G */
#endif

static QL_Status SensorEventCallback_Accel(void *cookie, struct QL_SensorEventInfo *event_info)
{
#ifdef ADATA_BATCH_PRINT
	struct SensorAccel_Data Adata;
	float accel[3] = {0};
#endif

	printf("Function - %s - event = %x \n",__func__,event_info->event);
	if (QL_SENSOR_EVENT_BATCH == event_info->event)
	{
		printf(" Accel - Batch Completed : Number of Packets -> %d @%x\n",event_info->numpkt, event_info->data);

#ifdef ADATA_BATCH_PRINT
		for(int i=0 ; i < event_info->numpkt ; i++) {

		Adata = *((struct SensorAccel_Data *) (event_info->data));

		accel[0] = Adata.data[0] * Ascaling;
		accel[1] = Adata.data[1] * Ascaling;
		accel[2] = Adata.data[2] * Ascaling;

		printf(" [ A ] X: %f, Y: %f, Z: %f\n",accel[0],accel[1],accel[2]);

		event_info->data =  (void *) ((unsigned int) (event_info->data) + sizeof(struct SensorAccel_Data));

		// Take care of wrap around in case of a circular batched buffer
		if ((unsigned int)(event_info->data) >=
				(unsigned int) ((BATCH_BUFFER_DATA_ADDR_ACCEL) + BATCH_BUFFER_DATA_SIZE_ACCEL ))
		{
			event_info->data = (void *)(BATCH_BUFFER_DATA_ADDR_ACCEL);
		}

		}
#endif
	}
	return QL_STATUS_OK;
}

static QL_Status SensorEventCallback_Mag(void *cookie, struct QL_SensorEventInfo *event_info)
{
#ifdef	MDATA_BATCH_PRINT
	struct SensorAccel_Data Mdata;
#endif

	printf("Function - %s - event = %x \n",__func__,event_info->event);
	if (QL_SENSOR_EVENT_BATCH == event_info->event)
	{
		printf(" Mag - Batch Completed : Number of Packets -> %d @%x\n",event_info->numpkt, event_info->data);

#ifdef MDATA_BATCH_PRINT
		for(int i=0 ; i < event_info->numpkt ; i++) {

		Mdata = *((struct SensorAccel_Data *) (event_info->data));

		printf(" [ M ] X: %d, Y: %d, Z: %d\n",Mdata.data[0],Mdata.data[1],Mdata.data[2]);

		event_info->data =  (void *) ((unsigned int) (event_info->data) + sizeof(struct SensorAccel_Data));

		// Take care of wrap around in case of a circular batched buffer
		if ((unsigned int)(event_info->data) >=
				(unsigned int) ((BATCH_BUFFER_DATA_ADDR_ACCEL) + BATCH_BUFFER_DATA_SIZE_ACCEL ))
		{
			event_info->data = (void *)(BATCH_BUFFER_DATA_ADDR_ACCEL);
		}

		}
#endif
	}
	return QL_STATUS_OK;
}

static QL_Status SensorEventCallback_Gyro(void *cookie, struct QL_SensorEventInfo *event_info)
{
#ifdef GDATA_BATCH_PRINT
	struct SensorAccel_Data Gdata;
	float gyro[3] = {0};
#endif

	printf("Function - %s - event = %x \n",__func__,event_info->event);
	if (QL_SENSOR_EVENT_BATCH == event_info->event)
	{
		printf(" Gyro - Batch Completed : Number of Packets -> %d @%x\n",event_info->numpkt, event_info->data);
#ifdef GDATA_BATCH_PRINT
		for(int i=0 ; i < event_info->numpkt ; i++) {

		Gdata = *((struct SensorAccel_Data *) (event_info->data));

		printf(" [ G] X: %d, Y: %d, Z: %d\n",Gdata.data[0],Gdata.data[1],Gdata.data[2]);

		event_info->data =  (void *) ((unsigned int) (event_info->data) + sizeof(struct SensorAccel_Data));

		// Take care of wrap around in case of a circular batched buffer
		if ((unsigned int)(event_info->data) >=
				(unsigned int) ((BATCH_BUFFER_DATA_ADDR_ACCEL) + BATCH_BUFFER_DATA_SIZE_ACCEL ))
		{
			event_info->data = (void *)(BATCH_BUFFER_DATA_ADDR_ACCEL);
		}

		}
#endif
	}
	return QL_STATUS_OK;
}

#define EN_MAG
#define EN_ACCEL
#define EN_GYRO

/* internal function: main handler for the AP Command Handler Task */
void vBatchSensorDataHandlerTaskHandler( void *pvParameters )
{

	BaseType_t 						xResult;
	struct xBatchSensorDataPacket       	xReceivedMsg;



	/* TODO do we add a task control variable here ? maybe useful to control the lifetime externally */
	for( ;; )
	{

		/*
		 * block until a command from AP is available.
		 * commands are pushed into this queue from ISR context
		 * when the AP raises the interrupt.
		 */
		xResult = xQueueReceive( xHandleQueueBatchSensorDataHandler, &( xReceivedMsg ), portMAX_DELAY);

		if( xResult == pdPASS )
		{
			switch(xReceivedMsg.command)
			{
				case BATCH_SENSOR_DATA_START:
				{
					QL_Status ret;
					uint32_t temp;

#ifdef EN_ACCEL
					/* Start the batching for Accel */
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

					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_SET_DYNAMIC_RANGE, (void*)&sensorDynamicRange_Accel);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_GET_DYNAMIC_RANGE, &temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_DYNAMIC_RANGE 0x%x\n",temp);
					printf("\n\n");

					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_SET_ODR, (void*)&sensorODR_Accel);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_GET_ODR, &temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_ODR 0x%x\n",temp);
					printf("\n\n");

					/* Set the Batching Buffer*/
					batch_databuf_info.batch_info_memPtr = (void *)BATCH_BUFFER_INFO_ADDR_ACCEL;									/* BATCH INFO */

					batch_databuf_info.batch_mem_start = (void *)BATCH_BUFFER_DATA_ADDR_ACCEL;										/* BATCH DATA START ADDR*/
					batch_databuf_info.batch_mem_end = (void *)(BATCH_BUFFER_DATA_ADDR_ACCEL + BATCH_BUFFER_DATA_SIZE_ACCEL);		/* BATCH DATA END ADDR, MAX */
					batch_databuf_info.batchSize = 200;//BATCH_BUFFER_DATA_SIZE_ACCEL - (BATCH_BUFFER_DATA_SIZE_ACCEL/2);			/* BATCH SIZE for expired interrupt */

					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_SET_BATCH, (void*)&batch_databuf_info);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_GET_BATCH, (void*)&tempbatch_databuf_info);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_BATCH 0x%x, [ 0x%x - 0x%x ], %u\n",
																	tempbatch_databuf_info.batch_info_memPtr,
																	tempbatch_databuf_info.batch_mem_start,
																	tempbatch_databuf_info.batch_mem_end,
																	tempbatch_databuf_info.batchSize
																	);
					printf("\n\n");

#endif
#ifdef EN_MAG
					/* Start the batching for Mag */
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

					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_SET_ODR, (void*)&sensorODR_Mag);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_GET_ODR, &temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_ODR 0x%x\n",temp);
					printf("\n\n");

					/* Set the Batching Buffer*/
					batch_databuf_info.batch_info_memPtr = (void *)BATCH_BUFFER_INFO_ADDR_MAG;									/* BATCH INFO */

					batch_databuf_info.batch_mem_start = (void *)BATCH_BUFFER_DATA_ADDR_MAG;										/* BATCH DATA START ADDR*/
					batch_databuf_info.batch_mem_end = (void *)(BATCH_BUFFER_DATA_ADDR_MAG + BATCH_BUFFER_DATA_SIZE_MAG);		/* BATCH DATA END ADDR, MAX */
					batch_databuf_info.batchSize = 250;//BATCH_BUFFER_DATA_SIZE_MAG - (BATCH_BUFFER_DATA_SIZE_MAG/2);			/* BATCH SIZE for expired interrupt */

					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_SET_BATCH, (void*)&batch_databuf_info);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_GET_BATCH, (void*)&tempbatch_databuf_info);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_BATCH 0x%x, [ 0x%x - 0x%x ], %u\n",
																	tempbatch_databuf_info.batch_info_memPtr,
																	tempbatch_databuf_info.batch_mem_start,
																	tempbatch_databuf_info.batch_mem_end,
																	tempbatch_databuf_info.batchSize
																	);
					printf("\n\n");

#endif
#ifdef EN_GYRO
					/* Start the batching for Gyro */
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

					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_SET_DYNAMIC_RANGE, (void*)&sensorDynamicRange_Gyro);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_GET_DYNAMIC_RANGE, &temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_DYNAMIC_RANGE 0x%x\n",temp);
					printf("\n\n");

					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_SET_ODR, (void*)&sensorODR_Gyro);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_GET_ODR, &temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_ODR 0x%x\n",temp);
					printf("\n\n");

					/* Set the Batching Buffer*/
					batch_databuf_info.batch_info_memPtr = (void *)BATCH_BUFFER_INFO_ADDR_GYRO;									/* BATCH INFO */

					batch_databuf_info.batch_mem_start = (void *)BATCH_BUFFER_DATA_ADDR_GYRO;										/* BATCH DATA START ADDR*/
					batch_databuf_info.batch_mem_end = (void *)(BATCH_BUFFER_DATA_ADDR_GYRO + BATCH_BUFFER_DATA_SIZE_GYRO);		/* BATCH DATA END ADDR, MAX */
					batch_databuf_info.batchSize = 300;//BATCH_BUFFER_DATA_SIZE_GYRO - (BATCH_BUFFER_DATA_SIZE_GYRO/2);			/* BATCH SIZE for expired interrupt */

					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_SET_BATCH, (void*)&batch_databuf_info);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_GET_BATCH, (void*)&tempbatch_databuf_info);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_BATCH 0x%x, [ 0x%x - 0x%x ], %u\n",
																	tempbatch_databuf_info.batch_info_memPtr,
																	tempbatch_databuf_info.batch_mem_start,
																	tempbatch_databuf_info.batch_mem_end,
																	tempbatch_databuf_info.batchSize
																	);
					printf("\n\n");

#endif
					/* ENABLE BATCHING */

#ifdef EN_ACCEL
					enable = 1;
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_BATCH_ENABLE, (void*)&enable);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_BATCH_GET_ENABLE, (void*)&temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_BATCH_GET_ENABLE 0x%x\n",temp);
					printf("\n\n");
#endif
#ifdef EN_MAG

					enable = 1;
					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_BATCH_ENABLE, (void*)&enable);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_BATCH_GET_ENABLE, (void*)&temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_BATCH_GET_ENABLE 0x%x\n",temp);
					printf("\n\n");
#endif
#ifdef EN_GYRO
					enable = 1;
					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_BATCH_ENABLE, (void*)&enable);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_BATCH_GET_ENABLE, (void*)&temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_BATCH_GET_ENABLE 0x%x\n",temp);
					printf("\n\n");
#endif


					/*Enable the sensors*/
#ifdef EN_ACCEL
					enable = 1;
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_ENABLE, (void *)&enable);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_GET_ENABLE, &temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_ENABLE 0x%x\n",temp);
					printf("\n\n");
#endif
#ifdef EN_MAG
					enable = 1;
					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_ENABLE, (void *)&enable);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_GET_ENABLE, &temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_ENABLE 0x%x\n",temp);
					printf("\n\n");
#endif
#ifdef EN_GYRO
					enable = 1;
					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_ENABLE, (void *)&enable);
					QL_ASSERT(ret == QL_STATUS_OK);
					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_GET_ENABLE, &temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_ENABLE 0x%x\n",temp);
					printf("\n\n");
#endif

				}
				break;

				case BATCH_SENSOR_DATA_STOP:
				{
					QL_Status ret;
					uint32_t temp;
					enable = 0;

#ifdef EN_ACCEL
					/*Stop Accel Batching*/
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_BATCH_ENABLE, (void*)&enable);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_BATCH_GET_ENABLE, (void*)&temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_BATCH_GET_ENABLE 0x%x\n",temp);
					printf("\n\n");

					ret = QL_SAL_SensorClose(sensorHandle_Accel);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_SensorClose OK.\n");
#endif

#ifdef EN_MAG
					/* Stop Mag Batching*/
					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_BATCH_ENABLE, (void*)&enable);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_BATCH_GET_ENABLE, (void*)&temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_BATCH_GET_ENABLE 0x%x\n",temp);
					printf("\n\n");

					ret = QL_SAL_SensorClose(sensorHandle_Mag);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_SensorClose OK.\n");
#endif

#ifdef EN_GYRO
					/*Stop Gyro Batching*/
					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_BATCH_ENABLE, (void*)&enable);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_BATCH_GET_ENABLE, (void*)&temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_BATCH_GET_ENABLE 0x%x\n",temp);
					printf("\n\n");

					ret = QL_SAL_SensorClose(sensorHandle_Gyro);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_SensorClose OK.\n");
#endif

				}
				break;
				case BATCH_SENSOR_DATA_FLUSH:
				{
					QL_Status ret;

#ifdef EN_ACCEL
					/* Accel */
					ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_BATCH_FLUSH,(void *)NULL);
					QL_ASSERT(ret == QL_STATUS_OK);
#endif

#ifdef EN_MAG
					/* Mag */
					ret = QL_SAL_SensorIoctl(sensorHandle_Mag, QL_SAL_IOCTL_BATCH_FLUSH,(void *)NULL);
					QL_ASSERT(ret == QL_STATUS_OK);
#endif

#ifdef EN_GYRO
					/* Gyro */
					ret = QL_SAL_SensorIoctl(sensorHandle_Gyro, QL_SAL_IOCTL_BATCH_FLUSH,(void *)NULL);
					QL_ASSERT(ret == QL_STATUS_OK);
#endif

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


QL_Status xStartBatchSensorData(void){
	struct xBatchSensorDataPacket msg2q;
	uint32_t ret;

	msg2q.command = BATCH_SENSOR_DATA_START;

	ret = ulAddPacketToBatchSensorDataHandlerQueue(&msg2q);
	QL_ASSERT(ret == 0);

	return QL_STATUS_OK;
}

QL_Status xStopBatchSensorData(void){
	struct xBatchSensorDataPacket msg2q;
	uint32_t ret;

	msg2q.command = BATCH_SENSOR_DATA_STOP;

	ret = ulAddPacketToBatchSensorDataHandlerQueue(&msg2q);
	QL_ASSERT(ret == 0);
	return QL_STATUS_OK;
}


QL_Status xFlushBatchSensorData(void){
	struct xBatchSensorDataPacket msg2q;
	uint32_t ret;

	msg2q.command = BATCH_SENSOR_DATA_FLUSH;

	ret = ulAddPacketToBatchSensorDataHandlerQueue(&msg2q);
	QL_ASSERT(ret == 0);
	return QL_STATUS_OK;
}
#endif /* HOST_SENSOR */
