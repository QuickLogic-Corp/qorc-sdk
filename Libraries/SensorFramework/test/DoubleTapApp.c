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

#include "QL_Trace.h"
#include "QL_SAL.h"
#include "QL_SensorIoctl.h"

#include <SenseMe_wrapper.h>

#include "eoss3_hal_pad_config.h"
#include "eoss3_hal_fpga_gpio.h"
#include <stdio.h>
#include <string.h>                                             // to remove warnings

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "Fw_global_config.h"

#include "DoubleTapLib.h"                                      // to remove warnings
#include "DoubleTapApp.h"
#include <ffe_ipc_lib.h>
#include <ql_SensorHub_MemoryMap.h>


/*
 * Double Tap Algorithm demo task.
 * The task will be idle, until we get a START command.
 * on START, we init the sensor (Accel) and register our callback with the sensor.
 *
 * if algo-on-m4:
 *
 * 		if non-batch mode:
 * 			we get immediate accel data in each callback.
 * 			we pass this data through the m4 algo
 * 			whenever algo reports a detect event, we report this (uart)
 *
 * 		if batch mode:
 * 			we get batch size expired, and batch info in each callback
 * 			we read all of the current batch data, and pass it through the m4 algo
 * 			whenever algo reports a detect event, we report this (uart)
 *
 *
 * 	if algo-on-ffe:
 *
 * 		both batch and non-batch mode:
 * 			we get double tap detect events in each callback.
 * 			we report this whenever we get the callback.
 *
 * 	we also now support a STOP command. The task will then de-init and close the sensor.
 * 	the task will then wait for the next START command.
 *
 *
 *
 * 	COMPANION_SENSOR:
 *
 * 	The functionality in this mode is exactly the same, except that we support communication
 * 	with a "master" HOST.
 * 	The START/STOP command comes from the HOST.
 * 	The Batch Event is reported to the HOST via interrupts.
 * 	The HOST then reads the batch data according to the batch information.
 * 	Note that, in this case, we always run in batch mode.
 *
 *
 */
#if (defined(HOST_SENSOR) || defined(COMPANION_SENSOR))

unsigned int double_tap_algo_in_m4;
unsigned int double_tap_in_batch_mode;

#ifdef DT_ALGO_ON_M4_LIVE
unsigned int double_tap_algo_in_m4 = 1;
unsigned int double_tap_in_batch_mode = 0;
#endif

#ifdef DT_ALGO_ON_M4_BATCH
unsigned int double_tap_algo_in_m4 = 1;
unsigned int double_tap_in_batch_mode = 1;
#endif

#ifdef DT_ALGO_ON_FFE
unsigned int double_tap_algo_in_m4 = 0;
unsigned int double_tap_in_batch_mode = 0;
#endif

#ifdef PCG_AND_DT_ON_FFE
unsigned int double_tap_algo_in_m4 = 0;
unsigned int double_tap_in_batch_mode = 0;
#endif

#ifdef PCG_AND_DT_ON_M4_BATCH
unsigned int double_tap_algo_in_m4 = 1;
unsigned int double_tap_in_batch_mode = 1;
#endif

unsigned int double_tap_batch_mode_test_type = 0; // simple 0(always batch). timed 1(flush at fixed time). random 2(flush at random times).


/*
 *
MemoryAddress									Data
				Byte3 [31:24]		Byte2 [23:16]		Byte1[15:8]			Byte0 [7:0]
buf_mem0		Time Stamp													0
buf_mem0 + 4	Accel Data  (X)							Packet Length
buf_mem0 + 8	Accel Data  (Z)							Accel Data  (Y)
 *
 */
struct sensorData_Accel
{
	unsigned int timestamp;
	unsigned short len;
	signed short int data[3];
};

float accel[3];
float accel_scaling = 1.0;



/*
 *
Memory Address				Data
					Byte3 [31:24]		yte2 [23:16]		Byte1[15:8]		Byte0 [7:0]
buf_mem0									SRAM Address [31:0]
buf_mem0 + 4								Number of Packets
 *
 */
struct sensorData_AccelBatchInfo
{
	void* batchdata_addr;
	unsigned int numpackets;
};


struct DoubleTapMsg
{
	unsigned int command;
	union
	{
		unsigned int sensorid;
		unsigned int timestamp;
		struct sensorData_Accel sensorData_A;
		struct sensorData_AccelBatchInfo batchdata;
	};
};



// Batch Configuration:
// BATCH_START_ADDR: (fixed)
// BATCH_MAX_SIZE: (fixed)
// BATCH_SIZE: (variable, interrupt triggered on size)

// Sensor Configuration:
// ODR, RANGE.

// BATCH TESTING CONFIGURATION:
// Batching configs 
//TBD: should go to a platform config file 
#define CFG_PING_PONG_FACTOR					3/2 // Prevents overflow @ the cost of memory. Producer writes to ping buffer while consume consumes pong buffer
#define CFG_DOUBLE_TAP_BATCHING_TIME_SEC		(1)
#define CFG_MAX_BATCH_DATA_SIZE_DOUBLETAP 		(CFG_PING_PONG_FACTOR * CFG_DOUBLE_TAP_SAMPLING_FREQ_HZ * CFG_DOUBLE_TAP_BATCHING_TIME_SEC * sizeof(struct sensorData_Accel))


// COMMANDS:
#define DOUBLE_TAP_CMD_START					1
#define DOUBLE_TAP_CMD_ACCEL_DATA				2
#define DOUBLE_TAP_CMD_TAP_DETECTED				3
#define DOUBLE_TAP_CMD_ACCEL_BATCH_DATA 		4
#define DOUBLE_TAP_CMD_STOP						9
#define DOUBLE_TAP_CMD_ENABLE					10

#define CFG_DOUBLE_TAP_SAMPLING_FREQ_HZ			400
#define CFG_DOUBLE_TAP_DYNAMIC_RANGE			2

static unsigned int sensorODR = 400;				// 400 Hz required for DT
static unsigned int enable = 0;

static TimerHandle_t  batchingTimer = NULL;
static volatile unsigned int  batchingON = 1,batchflush = 1;

/* temporary variables for GET IOCTLs */
static unsigned int temp = 0;
struct QL_SF_Ioctl_Set_Data_Buf tempDataBufInfo;
struct QL_SF_Ioctl_Set_Batch_Data_Buf tempbatch_databuf_info;
struct QL_SF_Ioctl_Req_Events tempEvent;
QL_SensorEventCallback_t tempCallback;



#ifdef COMPANION_SENSOR

/*
Structure defines the batching memory start address and size which is 
shared with AP before sharing batched data.  
*/
typedef struct {
	uint32_t ShmemStartAddr;
	uint32_t ShmemSize;
} ShmemAPM4Info_t;

/*
Structure defines the current batched data start address and number of batched
packets to shared with AP.
*/
typedef struct {
	uint32_t CurrAddr;
	uint32_t NumPackets;
} ShmemAPM4BatchInfo_t;

/*
Both batching memory start address,its size and current batched data start 
address, umber of batched packets are shared with AP using the same
shared memory. 
Note : This shared memory should reside in always on domain section of SRAM
*/
typedef union {
	ShmemAPM4Info_t ShmemLayout;
	ShmemAPM4BatchInfo_t ShmemBatchInfo;
} ShmemAPM4DataInfo_t; 


/*
Shared Memory between AP and SensorHub shall reside in always on domain
section of SRAM. Currently it is placed at address '0x20060000'
*/
static ShmemAPM4DataInfo_t ShareMemInfoAp @ 0x20060000;
#endif // #ifdef COMPANION_SENSOR

#define DOUBLE_TAP_MSGQ_ITEMS			10
#define DOUBLE_TAP_MGSQ_WAIT_TIME		portMAX_DELAY

QueueHandle_t 	DoubleTapMsgQ;
TaskHandle_t 	DoubleTapTaskHandle;

QL_SAL_SensorHandle_t doubleTapAppSensorHandle_DT;
QL_SAL_SensorHandle_t doubleTapAppSensorHandle_Accel;


#ifdef COMPANION_SENSOR
/*
This function generates an interrupt to AP. Software interrupt-2 is used to
generate interrupt to AP.
*/
void Generate_Interrupt_AP(void)
{
	/* Write 1 to generate interrupt to AP */
	INTR_CTRL->SOFTWARE_INTR_2 = 0x01UL;
}
#endif // #ifdef COMPANION_SENSOR


static void batchTimerCB( TimerHandle_t Timer )
{
	QL_Status ret;
	QL_LOG_DBG_150K("In timer call back\n");
	if (batchingON){
		if(batchflush)
		{
			vTaskDelay(100);
			ret = QL_SAL_SensorIoctl(doubleTapAppSensorHandle_DT, QL_SAL_IOCTL_BATCH_FLUSH,(void *)NULL);
			QL_ASSERT(ret == QL_STATUS_OK);
			batchflush = 0;
		}
		else
		{
			batchingON = 0;
			ret = QL_SAL_SensorIoctl(doubleTapAppSensorHandle_DT, QL_SAL_IOCTL_SET_BATCH,(void *)NULL);
			QL_ASSERT(ret == QL_STATUS_OK);
		}
	}
	else
	{
		batchingON = 1;
		ret = QL_SAL_SensorIoctl(doubleTapAppSensorHandle_DT, QL_SAL_IOCTL_SET_BATCH, (void*)(CFG_DOUBLE_TAP_SAMPLING_FREQ_HZ * CFG_DOUBLE_TAP_BATCHING_TIME_SEC));
		QL_ASSERT(ret == QL_STATUS_OK);
		batchflush = 1;

	}

}

void setupBatchTimer(void) {

	// create a timer of 5 X Batching time
	batchingTimer = xTimerCreate("batchingTimer", ((5 *CFG_DOUBLE_TAP_BATCHING_TIME_SEC *1000)/ portTICK_PERIOD_MS) , /*pdFALSE*/ pdTRUE  , ( void * ) 0, batchTimerCB);
	if(batchingTimer == NULL)
	{
		QL_LOG_ERR_150K("Fail to create batching timer!!! \r\n");
	}
	else
	{
		if(xTimerStart( batchingTimer, 0 ) != pdPASS)
		{
			QL_LOG_ERR_150K("Fail to start batching timer!!! \r\n");
		}
	}

}

#if (defined(DT_ALGO_ON_M4_BATCH) || defined(PCG_AND_DT_ON_M4_BATCH))
void EnableDoubleTapApp(void)
{
	struct DoubleTapMsg DoubleTapMsg;
#ifdef COMPANION_SENSOR
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
#endif

	DoubleTapMsg.command = DOUBLE_TAP_CMD_ENABLE;

#ifdef COMPANION_SENSOR
	if(pdTRUE != xQueueSendFromISR(DoubleTapMsgQ, &DoubleTapMsg, &xHigherPriorityTaskWoken) )
	{
		QL_LOG_ERR_150K("Msg Send Failed\n");
	}
	else
	{
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
#else 	/* COMPANION_SENSOR */

	if(xQueueSend(DoubleTapMsgQ, &DoubleTapMsg, 100) != pdPASS)
	{
		QL_LOG_ERR_150K("Msg Send Failed\n");
	}

#endif	/* COMPANION_SENSOR */
}
#endif /* (defined(DT_ALGO_ON_M4_BATCH) || defined(PCG_AND_DT_ON_M4_BATCH)) */

void StartDoubleTap(void)
{
	struct DoubleTapMsg DoubleTapMsg;

#ifdef COMPANION_SENSOR
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;                
#endif

	DoubleTapMsg.command = DOUBLE_TAP_CMD_START;

#ifdef COMPANION_SENSOR
	if(pdTRUE != xQueueSendFromISR(DoubleTapMsgQ, &DoubleTapMsg, &xHigherPriorityTaskWoken) )
	{
		QL_LOG_ERR_150K("Msg Send Failed\n");
	}
	else
	{
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);		
	}
#else 	/* COMPANION_SENSOR */

	if(xQueueSend(DoubleTapMsgQ, &DoubleTapMsg, 100) != pdPASS)
	{
		QL_LOG_ERR_150K("Msg Send Failed\n");
	}

#endif	/* COMPANION_SENSOR */
}



static QL_Status SensorEventCallback(void *cookie, struct QL_SensorEventInfo *event_info)
{

#ifndef COMPANION_SENSOR
	struct DoubleTapMsg DoubleTapMsg;
#endif

	//printf("CB\n");
      
	QL_TRACE_TEST_DEBUG("+\n");
	QL_ASSERT(event_info);
//	QL_ASSERT(event_info->numpkt);
//	QL_ASSERT(event_info->data);

//	QL_TRACE_TEST_DEBUG("cookie = %x, event_info->event = %x, "
//			"event_info->data = %x, event_info->numpkt = %x\n",
//			(int) cookie, event_info->event, event_info->data,
//			event_info->numpkt);

#ifndef COMPANION_SENSOR

        
	if (double_tap_algo_in_m4)
	{
		if (QL_SENSOR_EVENT_DATA == event_info->event)
		{
			DoubleTapMsg.command = DOUBLE_TAP_CMD_ACCEL_DATA;
			DoubleTapMsg.sensorData_A = *((struct sensorData_Accel *)event_info->data);

			/* numpkt == LSB SensorID, 2Bytes, MSB 2 Bytes is the number of samples counted (ignore this) */
		}
		else if (QL_SENSOR_EVENT_BATCH == event_info->event )
		{
			DoubleTapMsg.command = DOUBLE_TAP_CMD_ACCEL_BATCH_DATA;
			DoubleTapMsg.batchdata.batchdata_addr = event_info->data;
			DoubleTapMsg.batchdata.numpackets = event_info->numpkt;

			QL_TRACE_TEST_DEBUG("NumPkt In Batch mode = %d\n", event_info->numpkt);

			//printf("NumPkt In Batch mode = %d\n", event_info->numpkt);
			/* numpkt == LSB SensorID, 2Bytes, MSB 2 Bytes is the number of samples counted (ignore this) */

		}
		else
		{
			QL_TRACE_TEST_ERROR ("Invalid event type received event= %d \n", event_info->event);
		}

	}
	else
	{
		// if algo runs in FFE, then we get callback only for double tap detected event.
		DoubleTapMsg.command = DOUBLE_TAP_CMD_TAP_DETECTED;
		DoubleTapMsg.timestamp = (unsigned int)event_info->data;
	}

	if(xQueueSend(DoubleTapMsgQ, &DoubleTapMsg, portMAX_DELAY) != pdPASS)
	{
		QL_TRACE_TEST_ERROR("Msg Send Failed\n");
	}

#else  	/* COMPANION_SENSOR */
	/* 
	Inform AP for single packet as well as batch packet
	CurrAddr = event_info->data NumPackets = event_info->numpkt
	Update the structure, Update the address of the structure in the SW2 MB
	Trigger Interrupt to M4
	 */

	//printf("\nBatching info share with AP");	
	ShareMemInfoAp.ShmemBatchInfo.CurrAddr = (uint32_t) (event_info->data);
	ShareMemInfoAp.ShmemBatchInfo.NumPackets  = event_info->numpkt;

	QL_LOG_DBG_150K("Batch Event Address = 0x%x\n", (uint32_t)event_info->data);
	QL_LOG_DBG_150K("NumPkt In Batch mode = %d\n", event_info->numpkt);

	MISC_CTRL->SW_MB_2 = (uint32_t)&ShareMemInfoAp;
	Generate_Interrupt_AP();
#endif 	/* COMPANION_SENSOR */

	QL_TRACE_TEST_DEBUG("-\n");
	return QL_STATUS_OK;                            // to remove warnings ,  added return statement
}

/*                                                                                                                   // to remove warnings
static void toggleled(void)
{
	static unsigned int togglestate;
	togglestate = !togglestate;
	HAL_FB_GPIO_Write(FB_GPIO_0, togglestate);
}
 */


static void DoubleTapAppTask(void *a)
{
	QL_Status ret;
	BaseType_t qret;
	unsigned int DoubleTapTaskStop = 0;

	//struct QL_SF_Ioctl_Req_Events event;
	struct QL_SF_Ioctl_Set_Data_Buf databufinfo;

	struct DoubleTapMsg DoubleTapMsg;

	while(!DoubleTapTaskStop)
	{
		memset(&DoubleTapMsg, 0, sizeof(DoubleTapMsg));
		qret = xQueueReceive(DoubleTapMsgQ, &DoubleTapMsg, DOUBLE_TAP_MGSQ_WAIT_TIME);
		QL_ASSERT(qret == pdTRUE);

		switch(DoubleTapMsg.command)
		{
			case DOUBLE_TAP_CMD_START:
			{
				QL_LOG_INFO_150K("\n\nStarting Double Tap Application\n\n");

				if (double_tap_algo_in_m4)
				{
					/* sensor pseudo driver */
					DoubleTapMsg.sensorid = QL_SAL_SENSOR_ID_ACCEL;
				}
				else
				{
					/* algorithm pseudo driver */
					DoubleTapMsg.sensorid = QL_SAL_SENSOR_ID_DOUBLE_TAP;
				}

				//printf("Sensor ID: %d\n",DoubleTapMsg.sensorid);
				ret = QL_SAL_SensorOpen(&doubleTapAppSensorHandle_DT, DoubleTapMsg.sensorid);
				QL_ASSERT(ret == QL_STATUS_OK);
				//printf("QL_SAL_SensorOpen OK.\n");

				ret = QL_SAL_SensorIoctl(doubleTapAppSensorHandle_DT, QL_SAL_IOCTL_SET_CALLBACK, (void *)SensorEventCallback);
				QL_ASSERT(ret == QL_STATUS_OK);
				ret = QL_SAL_SensorIoctl(doubleTapAppSensorHandle_DT, QL_SAL_IOCTL_GET_CALLBACK, (void *)&tempCallback);
				QL_ASSERT(ret == QL_STATUS_OK);
				//printf("QL_SAL_IOCTL_GET_CALLBACK: 0x%x\n",tempCallback);
				//printf("CallBack Function Address: 0x%x\n",SensorEventCallback);
				//printf("\n\n");

#if 0
				/* attribute is always set to ACCEL */
				//printf("Sensor ID: %d\n",QL_SAL_SENSOR_ID_ACCEL);
				ret = QL_SAL_SensorOpen(&doubleTapAppSensorHandle_Accel, QL_SAL_SENSOR_ID_ACCEL);
				QL_ASSERT(ret == QL_STATUS_OK);
				//printf("QL_SAL_SensorOpen OK.\n");

				ret = QL_SAL_SensorIoctl(doubleTapAppSensorHandle_Accel, QL_SAL_IOCTL_SET_DYNAMIC_RANGE, (void*)&sensorDynamicRange);
				QL_ASSERT(ret == QL_STATUS_OK);
				ret = QL_SAL_SensorIoctl(doubleTapAppSensorHandle_Accel, QL_SAL_IOCTL_GET_DYNAMIC_RANGE, &temp);
				QL_ASSERT(ret == QL_STATUS_OK);
				//printf("QL_SAL_IOCTL_GET_DYNAMIC_RANGE 0x%x\n",temp);
				//printf("\n\n");

				/* close the sensor handle, we are done. */
				ret = QL_SAL_SensorClose(doubleTapAppSensorHandle_Accel);
				QL_ASSERT(ret == QL_STATUS_OK);
				//printf("QL_SAL_SensorClose OK.\n");
				//printf("\n\n");

#endif

				ret = QL_SAL_SensorIoctl(doubleTapAppSensorHandle_DT, QL_SAL_IOCTL_SET_ODR, (void*)&sensorODR);
				QL_ASSERT(ret == QL_STATUS_OK);
				ret = QL_SAL_SensorIoctl(doubleTapAppSensorHandle_DT, QL_SAL_IOCTL_GET_ODR, &temp);
				QL_ASSERT(ret == QL_STATUS_OK);
				//printf("QL_SAL_IOCTL_GET_ODR 0x%x\n",temp);
				//printf("\n\n");

				/* in batch mode, we have a pre-fixed batch buffer */
				/* in non batch mode, we have a pre-fixed SRAM address to read data */
				/* so, deprecating this for now, look at it if this is good to handle large buffer allocation ? */

//				ret = QL_SAL_SensorIoctl(doubleTapAppSensorHandle_DT, QL_SAL_IOCTL_ALLOC_DATA_BUF, (void*)&databufinfo);
//				QL_ASSERT(ret == QL_STATUS_OK);
//				printf("QL_SAL_IOCTL_ALLOC_DATA_BUF 0x%x, 0x%x\n",databufinfo.buf,databufinfo.size);
//				printf("\n\n");


#ifdef COMPANION_SENSOR
				/*
				Send the info of batching memory to AP
				ShmemStartAddr = ( unsigned int) (databufinfo.buf)
				ShmemSize = databufinfo.size
				Update the structure with the above info, Update the address of the
				structure into SW2 MB
				Trigger interrupt to M4
				 */
				//printf("\nShare Memory  info shared with AP");
				ShareMemInfoAp.ShmemLayout.ShmemStartAddr = (uint32_t) (BATCH_BUFFER_INFO_ADDR_ACCEL);
				ShareMemInfoAp.ShmemLayout.ShmemSize = BATCH_BUFFER_DATA_SIZE_ACCEL;
				MISC_CTRL->SW_MB_2 = (uint32_t)&ShareMemInfoAp;
				Generate_Interrupt_AP();
#endif		/* COMPANION_SENSOR */

				if(double_tap_in_batch_mode)
				{
#if 0                    
					/* batch mode */

					batch_databuf_info.batch_info_memPtr = (void *)BATCH_BUFFER_INFO_ADDR_ACCEL;									/* BATCH INFO */

					batch_databuf_info.batch_mem_start = (void *)BATCH_BUFFER_DATA_ADDR_ACCEL;										/* BATCH DATA START ADDR*/
					batch_databuf_info.batch_mem_end = (void *)(BATCH_BUFFER_DATA_ADDR_ACCEL + (BATCH_BUFFER_DATA_SIZE_ACCEL));		/* BATCH DATA END ADDR, MAX */
					batch_databuf_info.batchSize = 100;//BATCH_BUFFER_DATA_SIZE_ACCEL - (BATCH_BUFFER_DATA_SIZE_ACCEL/2);			/* BATCH SIZE for expired interrupt */

					ret = QL_SAL_SensorIoctl(doubleTapAppSensorHandle_DT, QL_SAL_IOCTL_SET_BATCH, (void*)&batch_databuf_info);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(doubleTapAppSensorHandle_DT, QL_SAL_IOCTL_GET_BATCH, (void*)&tempbatch_databuf_info);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_GET_BATCH 0x%x, [ 0x%x - 0x%x ], %u\n",
																	tempbatch_databuf_info.batch_info_memPtr,
																	tempbatch_databuf_info.batch_mem_start,
																	tempbatch_databuf_info.batch_mem_end,
																	tempbatch_databuf_info.batchSize
																	);
					printf("\n\n");

					enable = 1;
					ret = QL_SAL_SensorIoctl(doubleTapAppSensorHandle_DT, QL_SAL_IOCTL_BATCH_ENABLE, (void*)&enable);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(doubleTapAppSensorHandle_DT, QL_SAL_IOCTL_BATCH_GET_ENABLE, (void*)&temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					printf("QL_SAL_IOCTL_BATCH_GET_ENABLE 0x%x\n",temp);
					printf("\n\n");

					//setupBatchTimer(); // for different test mode.
#endif                    
				}
				else
				{
					/* non- batch mode */

					if(double_tap_algo_in_m4)
					{
						databufinfo.buf = (void *)LIVE_SENSOR_DATA_ACCEL_ADDR;					/* LIVE DATA ADDR */
						databufinfo.size = LIVE_SENSOR_DATA_PACKET_SIZE;						/* LIVE DATA PACKET SIZE */
					}
					else
					{
						databufinfo.buf = (void *)LIVE_SENSOR_DATA_DT_ADDR;						/* LIVE DATA ADDR */
						databufinfo.size = LIVE_SENSOR_DATA_PACKET_SIZE;						/* LIVE DATA PACKET SIZE */
					}


					ret = QL_SAL_SensorIoctl(doubleTapAppSensorHandle_DT, QL_SAL_IOCTL_SET_DATA_BUF, (void*)&databufinfo );
					QL_ASSERT(ret == QL_STATUS_OK);
					QL_LOG_DBG_150K("QL_SAL_IOCTL_SET_DATA_BUF 0x%x\n",databufinfo.buf);
					ret = QL_SAL_SensorIoctl(doubleTapAppSensorHandle_DT, QL_SAL_IOCTL_GET_DATA_BUF, (void*)&tempDataBufInfo);
					QL_ASSERT(ret == QL_STATUS_OK);
					QL_LOG_DBG_150K("QL_SAL_IOCTL_GET_DATA_BUF 0x%x\n",tempDataBufInfo.buf);
					QL_LOG_DBG_150K("\n\n");

					enable = 1;
					ret = QL_SAL_SensorIoctl(doubleTapAppSensorHandle_DT, QL_SAL_IOCTL_LIVE_DATA_ENABLE, (void*)&enable);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(doubleTapAppSensorHandle_DT, QL_SAL_IOCTL_GET_LIVE_DATA_ENABLE, (void*)&temp);
					QL_ASSERT(ret == QL_STATUS_OK);
					QL_LOG_DBG_150K("QL_SAL_IOCTL_GET_LIVE_DATA_ENABLE 0x%x\n",temp);
					QL_LOG_DBG_150K("\n\n");
				}


#if (!defined( DT_ALGO_ON_M4_BATCH) && !defined(PCG_AND_DT_ON_M4_BATCH))
				/* enable the sensor */
				enable = 1;
				ret = QL_SAL_SensorIoctl(doubleTapAppSensorHandle_DT, QL_SAL_IOCTL_ENABLE, (void *)&enable);
				QL_ASSERT(ret == QL_STATUS_OK);
				ret = QL_SAL_SensorIoctl(doubleTapAppSensorHandle_DT, QL_SAL_IOCTL_GET_ENABLE, &temp);
				QL_ASSERT(ret == QL_STATUS_OK);
				QL_LOG_DBG_150K("QL_SAL_IOCTL_GET_ENABLE 0x%x\n",temp);
				QL_LOG_DBG_150K("\n\n");
#endif	/* (!defined( DT_ALGO_ON_M4_BATCH) && !defined(PCG_AND_DT_ON_M4_BATCH)) */

				/* close the sensor handle, we are done. */
				ret = QL_SAL_SensorClose(doubleTapAppSensorHandle_DT);
				QL_ASSERT(ret == QL_STATUS_OK);
				QL_LOG_DBG_150K("QL_SAL_SensorClose OK.\n");
				QL_LOG_DBG_150K("\n\n");


				if(!double_tap_in_batch_mode && double_tap_algo_in_m4)
				{
#if( !defined(USE_INTERRUPTS_FOR_LIVE_DATA) || defined(PURE_M4_DRIVERS) )
					vSensorHubCallbackInit(2);
					//vSensorHubChangeTimerPeriod(2); // cannot run at 2ms properly!
					vSensorHubStartTimer();
#endif // ( !defined(USE_INTERRUPTS_FOR_LIVE_DATA) || defined(PURE_M4_DRIVERS) )
				}

			}
			break;
#if 1
			/* this will happen only in non-batch mode + algo_in_m4 */
			case DOUBLE_TAP_CMD_ACCEL_DATA :
			{
				accel[0] = DoubleTapMsg.sensorData_A.data[0] * accel_scaling;
				accel[1] = DoubleTapMsg.sensorData_A.data[1] * accel_scaling;
				accel[2] = DoubleTapMsg.sensorData_A.data[2] * accel_scaling;

				printf("TS=%d %f %f %f\n", DoubleTapMsg.sensorData_A.timestamp >> 8, accel[0], accel[1], accel[2]);
				printf("TS=%d %f\n", DoubleTapMsg.sensorData_A.timestamp >> 8, accel[2]);
#if !defined(DT_ALGO_ON_FFE) && !defined(PCG_AND_DT_ON_FFE)
//				if (CheckDoubleTap_400(accel))
//				{
//					printf("Double Tap Detected (Algo in M4)\n");
//					//toggleled();
//				}
#endif  /* !defined(DT_ALGO_ON_FFE) && !defined(PCG_AND_DT_ON_FFE) */                
			}
			break;

			/* this will happen only in batch mode + algo_in_m4 */
			case DOUBLE_TAP_CMD_ACCEL_BATCH_DATA:
			{
				int index = 0;
				struct sensorData_Accel accel_data;
				for ( index = 0; index < DoubleTapMsg.batchdata.numpackets; index++)
				{
					accel_data = *((struct sensorData_Accel *)(DoubleTapMsg.batchdata.batchdata_addr));
					accel[0] = accel_data.data[0] * accel_scaling;
					accel[1] = accel_data.data[1] * accel_scaling;
					accel[2] = accel_data.data[2] * accel_scaling;

					//printf("TS=%d %f %f %f\n", sensorData_Accel.timestamp >> 8, accel[0], accel[1], accel[2]);
					//printf("TS=%d %2.3f\n", accel_data.timestamp >> 8, accel[2]);

#if !defined(DT_ALGO_ON_FFE) && !defined(PCG_AND_DT_ON_FFE)
//					if (CheckDoubleTap_400(accel))
//					{
//						printf("Double Tap Detected (Algo in M4)\n");
//						//toggleled();
//					}
#endif /* !defined(DT_ALGO_ON_FFE) && !defined(PCG_AND_DT_ON_FFE) */

					DoubleTapMsg.batchdata.batchdata_addr =  (void *) ((unsigned int) (DoubleTapMsg.batchdata.batchdata_addr) + sizeof(struct sensorData_Accel));

					// Take care of wrap around in case of a circular batched buffer
					if ((unsigned int)(DoubleTapMsg.batchdata.batchdata_addr) >=
							(unsigned int) ((BATCH_BUFFER_DATA_ADDR_ACCEL) + BATCH_BUFFER_DATA_SIZE_ACCEL ))
					{
						DoubleTapMsg.batchdata.batchdata_addr = (void *)(BATCH_BUFFER_DATA_ADDR_ACCEL);
					}
				}
			}
			break;
#endif
			/* this will happen only in batch or non-batch mode + algo_in_ffe */
			case DOUBLE_TAP_CMD_TAP_DETECTED :
			{
				QL_LOG_INFO_150K("Double Tap Detected (Algo in FFE) at TS:[%u]\n",DoubleTapMsg.timestamp);
				//toggleled();
			}
			break;

			case DOUBLE_TAP_CMD_STOP:
			{
				DoubleTapTaskStop = 1;
				QL_LOG_INFO_150K("exiting Double Tap Task.\n");
			}
			break;
			case DOUBLE_TAP_CMD_ENABLE:
			{
				QL_SAL_SensorHandle_t sensor_handle;

				QL_LOG_DBG_150K("\nEnabling Double Tap Application\n\n");

				if (double_tap_algo_in_m4)
				{
					/* sensor pseudo driver */
					DoubleTapMsg.sensorid = QL_SAL_SENSOR_ID_ACCEL;
				}
				else
				{
					/* algorithm pseudo driver */
					DoubleTapMsg.sensorid = QL_SAL_SENSOR_ID_DOUBLE_TAP;
				}

				ret = QL_SAL_SensorOpen(&sensor_handle, DoubleTapMsg.sensorid);
				QL_ASSERT(ret == QL_STATUS_OK);

				/* enable the sensor */
				enable = 1;
				ret = QL_SAL_SensorIoctl(sensor_handle, QL_SAL_IOCTL_ENABLE, (void *)&enable);
				QL_ASSERT(ret == QL_STATUS_OK);
				ret = QL_SAL_SensorIoctl(sensor_handle, QL_SAL_IOCTL_GET_ENABLE, &temp);
				QL_ASSERT(ret == QL_STATUS_OK);
				QL_LOG_DBG_150K("QL_SAL_IOCTL_GET_ENABLE 0x%x\n",temp);
				QL_LOG_DBG_150K("\n\n");

				/* close the sensor handle, we are done. */
				ret = QL_SAL_SensorClose(sensor_handle);
				QL_ASSERT(ret == QL_STATUS_OK);
				QL_LOG_DBG_150K("QL_SAL_SensorClose OK.\n");
				QL_LOG_DBG_150K("\n\n");
			}
			break;

			default :
			{
				QL_LOG_INFO_150K("Unknown message\n");
			}
			break;

		}
	}
}

void DoubleTapAppInit(void)
{

	//PadConfig PadLedGpio = { PAD_18, PAD18_FUNC_SEL_FBIO_18, PAD_CTRL_SRC_FPGA, PAD_MODE_OUTPUT_EN, PAD_NOPULL, PAD_DRV_STRENGHT_4MA, PAD_SLEW_RATE_SLOW, PAD_SMT_TRIG_DIS};
	//HAL_PAD_Config(&PadLedGpio);
	//HAL_FB_GPIO_Write(FB_GPIO_0, 0);


	DoubleTapMsgQ = xQueueCreate(DOUBLE_TAP_MSGQ_ITEMS, sizeof(struct DoubleTapMsg));
	QL_ASSERT(DoubleTapMsgQ);

	xTaskCreate(DoubleTapAppTask, "DblTapTsk", 256/*1024*/, NULL, tskIDLE_PRIORITY + 4, &DoubleTapTaskHandle);
	QL_ASSERT(DoubleTapTaskHandle);

}
#endif /* (defined(HOST_SENSOR) || defined(COMPANION_SENSOR)) */
