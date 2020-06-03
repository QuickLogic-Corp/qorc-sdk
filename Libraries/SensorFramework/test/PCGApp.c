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
 *    File   : PCGApp.c
 *    Purpose: 
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#include "QL_Trace.h"
#include "QL_SAL.h"
#include "QL_SensorIoctl.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include <SenseMe_wrapper.h>
#include <ql_SensorHub_MemoryMap.h>
#include <Fw_global_config.h>

//#include "eoss3_hal_pad_config.h"
//#include "eoss3_hal_fpga_gpio.h"
//#include <stdio.h>

#include <string.h>                     // to remove warnings
#include <PCGApp.h>

#if defined(HOST_SENSOR) || defined(COMPANION_SENSOR)

#define PCG_CMD_START				11
#define PCG_CMD_ACCEL_DATA			12
#define PCG_CMD_PCG_RESULT_FRM_FFE 	13
#define PCG_CMD_ACCEL_BATCH_DATA	14
#define PCG_CMD_ACCEL_ENABLE		15

typedef struct
{
	unsigned int timestamp;              //timestamp
	unsigned int ContextGesture; 		//MSB 16 bits Context, LSB 16bits Gesture (B2C)
	unsigned int WalkingCount;    		//32 bit count
	unsigned int RunningCount;    		//32 bit count
} pcgdata_t;


struct acceldata_t {
	unsigned int timestamp;
	unsigned short len;
	signed short int data[3];
};

struct accelBatchInfo_t
{
	void* batchdata_addr;
	unsigned int numpkt;
};

struct PCGMsg_t {
	unsigned int command;
	union {
		struct acceldata_t acceldata;
		pcgdata_t pcgdata;
		struct accelBatchInfo_t batchdata;
		unsigned int sensorid;
	};
};


unsigned int pcg_algo_in_m4;
unsigned int pcg_algo_batch_mode;

#ifdef PCG_ALGO_ON_M4_LIVE
unsigned int pcg_algo_in_m4 = 1;
unsigned int pcg_algo_batch_mode = 0;
#endif

#ifdef PCG_ALGO_ON_M4_BATCH
unsigned int pcg_algo_in_m4 = 1;
unsigned int pcg_algo_batch_mode = 1;
#endif

#ifdef PCG_ALGO_ON_FFE
unsigned int pcg_algo_in_m4 = 0;
#endif

#ifdef PCG_AND_DT_ON_FFE
unsigned int pcg_algo_in_m4 = 0;
#endif

#ifdef PCG_AND_DT_ON_M4_BATCH
unsigned int pcg_algo_in_m4 = 1;
unsigned int pcg_algo_batch_mode = 1;
#endif

#if (defined(FFE_DRIVERS) || defined (HYBRID_FFE_M4_DRIVERS) || defined(PURE_M4_DRIVERS))
static unsigned int sensorDynamicRange = 0x2;		// 0 maps to +-2G, 1 maps to +-4G, 2 maps to +-8G, 3 maps to +-16G
#endif
static unsigned int sensorODR = 50;
static unsigned int enable = 0;

/* temporary variables for GET IOCTLs */
static unsigned int temp = 0;
static struct QL_SF_Ioctl_Set_Data_Buf tempDataBufInfo;
//static struct QL_SF_Ioctl_Set_Batch_Data_Buf tempbatch_databuf_info;
//static struct QL_SF_Ioctl_Req_Events tempEvent;
static QL_SensorEventCallback_t tempCallback;


#define CFG_PING_PONG_FACTOR	1
#define CFG_PCG_SAMPLING_FREQ_HZ 50
//#define PCG_ON_M4
#define PCG_MSGQ_ITEMS	10
#define PCG_MGSQ_WAIT_TIME	portMAX_DELAY
static QueueHandle_t PCGMsgQ;
// 1 sec pcg results
#define CFG_MAX_DATA_SIZE_PCG    (CFG_PING_PONG_FACTOR * CFG_PCG_SAMPLING_FREQ_HZ * sizeof( pcgdata_t))

void StartPCG(void)
{
	struct PCGMsg_t PCGMsg;
	if(pcg_algo_in_m4)
	{
#if (defined(FFE_DRIVERS) || defined(HYBRID_FFE_M4_DRIVERS))
		PCGMsg.sensorid = QL_SAL_SENSOR_ID_ACCEL2;
#endif

#ifdef PURE_M4_DRIVERS
		PCGMsg.sensorid = QL_SAL_SENSOR_ID_ACCEL;
#endif
	}
	else
	{
		PCGMsg.sensorid = QL_SAL_SENSOR_ID_PCG;
	}

	PCGMsg.command = PCG_CMD_START;
	if(xQueueSend(PCGMsgQ, &PCGMsg, 100) != pdPASS) {
		printf("PCG Msg Send Failed\n");
	}	
}

#if (defined(PCG_ALGO_ON_M4_BATCH) || defined (PCG_AND_DT_ON_M4_BATCH))
void EnablePCG(void)
{
	struct PCGMsg_t PCGMsg;

	if(pcg_algo_in_m4)
	{
#if (defined(FFE_DRIVERS) || defined(HYBRID_FFE_M4_DRIVERS))
		PCGMsg.sensorid = QL_SAL_SENSOR_ID_ACCEL2;
#endif

#ifdef PURE_M4_DRIVERS
		PCGMsg.sensorid = QL_SAL_SENSOR_ID_ACCEL;
#endif
	}
	else
	{
		PCGMsg.sensorid = QL_SAL_SENSOR_ID_PCG;
	}

	PCGMsg.command = PCG_CMD_ACCEL_ENABLE;
	if(xQueueSend(PCGMsgQ, &PCGMsg, 100) != pdPASS) {
		printf("PCG Msg Send Failed\n");
	}
}
#endif /* (defined(PCG_ALGO_ON_M4_BATCH) || defined (PCG_AND_DT_ON_M4_BATCH)) */

#if defined(HOST_SENSOR_TEST_LED)
static void ContextTimerCallback(TimerHandle_t CtxTimer)
{ 
  static uint32_t ulToggleLed = 0;
  
  if (ulToggleLed) {
    LedOrangeOn();
    ulToggleLed = 0;
  }
  else {
    LedOrangeOff();
    ulToggleLed = 1;
  }
}

#define CONTEXT_STATIONARY_TIMING  (7*24*60*60*1000)  /* 7-DAYS */
static TimerHandle_t  ContextTimer = NULL;
static void setupContextTimer(void) {

	// create a timer of 7-DAYS
	ContextTimer = xTimerCreate("ContextTimer", (CONTEXT_STATIONARY_TIMING / portTICK_PERIOD_MS) , /*pdFALSE*/ pdTRUE  , ( void * ) 0, ContextTimerCallback);
	if(ContextTimer == NULL)
	{
		printf("Fail to create context timer!!! \r\n");
	}
	else
	{
		if(xTimerStart( ContextTimer, 0 ) != pdPASS)
		{
			printf("Fail to start context timer!!! \r\n");
		}
	}

}
#endif      /* HOST_SENSOR_TEST_LED */

static void printContext(int ctxt)
{
#if defined(HOST_SENSOR_TEST_LED)    
    static uint32_t TmrPriodChanged;
#endif      /* HOST_SENSOR_TEST_LED */
    switch (ctxt)
	{
	case UNKNOWN:
		printf("Context = UNKNOWN\n");

#if defined(HOST_SENSOR_TEST_LED)        
        if (TmrPriodChanged != 3) {
          xTimerChangePeriod( ContextTimer, 50 / portTICK_PERIOD_MS, 50 );
                  
          //LedOrangeOn();
          TmrPriodChanged = 3;
        }
#endif          /* HOST_SENSOR_TEST_LED */              
		break;
	case NOT_ON_PERSON:
		printf("Context = NOT_ON_PERSON\n");

#if defined(HOST_SENSOR_TEST_LED)        
		if (TmrPriodChanged != 4) {  
			xTimerChangePeriod( ContextTimer,  CONTEXT_STATIONARY_TIMING / portTICK_PERIOD_MS, 50 );  
			LedOrangeOff();
			TmrPriodChanged = 4;
		}
#endif          /* HOST_SENSOR_TEST_LED */        
		break;
	case WALKING:
		printf("Context = WALKING\n");

#if defined(HOST_SENSOR_TEST_LED) 
#if 0
		if (TmrPriodChanged != 1) {			
			xTimerChangePeriod( ContextTimer, 500 / portTICK_PERIOD_MS, 50 );
			TmrPriodChanged = 1;
		}
#endif        
#endif          /* HOST_SENSOR_TEST_LED */        
		break;
	case RUNNING:
		printf("Context = RUNNING\n");

#if defined(HOST_SENSOR_TEST_LED)
#if 0
		if (TmrPriodChanged != 2) {
			xTimerChangePeriod( ContextTimer, 50 / portTICK_PERIOD_MS, 50 );
			TmrPriodChanged = 2;
		}
#endif        
#endif          /* HOST_SENSOR_TEST_LED */        
		break;
	case STATIONARY:
		printf("Context = STATIONARY\n");

#if defined(HOST_SENSOR_TEST_LED)        
		LedOrangeOn();		
		if (TmrPriodChanged != 0) {
			xTimerChangePeriod( ContextTimer,  CONTEXT_STATIONARY_TIMING / portTICK_PERIOD_MS, 50 );  
			LedOrangeOn();
			TmrPriodChanged = 0;
		}
#endif          /* HOST_SENSOR_TEST_LED */        
		break;
#ifdef ON_BIKE_CTX      
	case ON_BIKE:
		printf("Context = ON_BIKE\n");
		break;
#endif      

	default:
		printf("Unexpected Context\n");
		break;
	}
}

int gestureDetected = 0;  
void PCGAlgo_Test(float AccelData[])
{
	SenseMe_output_PedConAdv outputData;
//	static int count = 0;
	static int prevWalkingSteps = 0;
	static int prevRunningSteps = 0;
	static int prevContext = 100;

	//printf("ax =%f, ay =%f, az =%f\n", AccelData[0],AccelData[1],AccelData[2] );

	//Gesture algo
//	gestureDetected = alg_b2see(AccelData);
//	if (gestureDetected)
//	{
//		printf("\n [%d] B2SEE Gesture Detected !! \n", count++);
//	}

	//Call Pedometer and context
	alg_pedcon (AccelData, &outputData);
	if ((prevWalkingSteps != outputData.walkingSteps) || (prevRunningSteps != outputData.runningSteps) ||
			(prevContext != outputData.contextDetected))
	{
		prevWalkingSteps = outputData.walkingSteps;
		prevRunningSteps = outputData.runningSteps;
		prevContext = outputData.contextDetected;
		printf("\nWalking steps = %d, Running steps = %d(Algo on M4)\n", outputData.walkingSteps, outputData.runningSteps);
#ifdef ADVPED_ALGORITHM        
		printf("Calories = %d, distanceTraveled = %d\n", outputData.calories, outputData.distanceTravelled);
#endif  
		printContext(outputData.contextDetected);
	}
}


static QL_Status SensorEventCallback_PCG(void *cookie, struct QL_SensorEventInfo *event_info)
{

	//struct acceldata_t *d;                                // to remove warnings		variable unused

	//pcgdata_t * pcgdata;
	struct PCGMsg_t PCGMsg;
	QL_TRACE_TEST_DEBUG("+\n");
//	QL_ASSERT(event_info);
//	QL_ASSERT(event_info->numpkt);
//	QL_ASSERT(event_info->data);

	QL_TRACE_TEST_DEBUG("cookie = %x, event_info->event = %x, "
			"event_info->data = %x, event_info->numpkt = %x\n",
			(int) cookie, event_info->event, event_info->data,
			event_info->numpkt);
	QL_TRACE_TEST_DEBUG("-\n");

        
	if(pcg_algo_in_m4 && !pcg_algo_batch_mode)
	{
		//d = (struct acceldata_t *)event_info->data;
		//printf("%u-%d-%d-%d-%d\n", d->timestamp, d->data[0], d->data[1], d->data[2], d->data[3]);
		PCGMsg.command = PCG_CMD_ACCEL_DATA;
		PCGMsg.acceldata = *((struct acceldata_t *)event_info->data);
	}
	else if(pcg_algo_in_m4 && pcg_algo_batch_mode ) {
		PCGMsg.command = PCG_CMD_ACCEL_BATCH_DATA;
		PCGMsg.batchdata.batchdata_addr = event_info->data;
		PCGMsg.batchdata.numpkt	= event_info->numpkt;
	}
	else
	{
		// pass PCG result
		//printf("resultfromFFE\n");
		PCGMsg.command = PCG_CMD_PCG_RESULT_FRM_FFE;
		PCGMsg.pcgdata = *((pcgdata_t *)event_info->data);
	}

	if(xQueueSend(PCGMsgQ, &PCGMsg, 100) != pdPASS) {
		printf("PCG Msg Send Failed\n");
	}


	return QL_STATUS_OK;
}


void SetUpPCGAPI(unsigned int sensorid)
{
	QL_Status ret;
	// BaseType_t qret;                          		// to remove warnings	variable unused
	// static unsigned char databuf[24] = {0};
#if (defined(FFE_DRIVERS) || defined (HYBRID_FFE_M4_DRIVERS))
	QL_SAL_SensorHandle_t sensorHandle_Accel;
#endif
	QL_SAL_SensorHandle_t handle;
	//struct QL_SF_Ioctl_Req_Events event;
	struct QL_SF_Ioctl_Set_Data_Buf databufinfo;
	struct QL_SF_Ioctl_Set_Batch_Data_Buf batch_databufinfo,tempbatch_databufinfo;;
	//struct QL_SF_Ioctl_Set_Batch_Data_Buf batch_databuf_info;

	databufinfo.buf = 0;

	ret = QL_SAL_SensorOpen(&handle,sensorid);
	QL_ASSERT(ret == QL_STATUS_OK);


	ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_SET_CALLBACK, (void *)SensorEventCallback_PCG);
	QL_ASSERT(ret == QL_STATUS_OK);
	ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_GET_CALLBACK, (void *)&tempCallback);
	QL_ASSERT(ret == QL_STATUS_OK);
	printf("QL_SAL_IOCTL_GET_CALLBACK: 0x%x\n",tempCallback);
	printf("CallBack Function Address: 0x%x\n",SensorEventCallback_PCG);
	printf("\n\n");

#if (defined(FFE_DRIVERS) || defined (HYBRID_FFE_M4_DRIVERS))
	ret = QL_SAL_SensorOpen(&sensorHandle_Accel,QL_SAL_SENSOR_ID_ACCEL);
	QL_ASSERT(ret == QL_STATUS_OK);


	ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_SET_DYNAMIC_RANGE, (void*)&sensorDynamicRange);
	QL_ASSERT(ret == QL_STATUS_OK);
	ret = QL_SAL_SensorIoctl(sensorHandle_Accel, QL_SAL_IOCTL_GET_DYNAMIC_RANGE, &temp);
	QL_ASSERT(ret == QL_STATUS_OK);
	printf("QL_SAL_IOCTL_GET_DYNAMIC_RANGE 0x%x\n",temp);
	printf("\n\n");

	ret = QL_SAL_SensorClose(sensorHandle_Accel);
	QL_ASSERT(ret == QL_STATUS_OK);
#endif // (defined(FFE_DRIVERS) || defined (HYBRID_FFE_M4_DRIVERS))

#ifdef PURE_M4_DRIVERS
	ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_SET_DYNAMIC_RANGE, (void*)&sensorDynamicRange);
	QL_ASSERT(ret == QL_STATUS_OK);
	ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_GET_DYNAMIC_RANGE, &temp);
	QL_ASSERT(ret == QL_STATUS_OK);
	printf("QL_SAL_IOCTL_GET_DYNAMIC_RANGE 0x%x\n",temp);
	printf("\n\n");
#endif /*PURE_M4_DRIVERS*/

	ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_SET_ODR, (void*)&sensorODR);
	QL_ASSERT(ret == QL_STATUS_OK);
	ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_GET_ODR, &temp);
	QL_ASSERT(ret == QL_STATUS_OK);
	printf("QL_SAL_IOCTL_GET_ODR 0x%x\n",temp);
	printf("\n\n");

	if(pcg_algo_batch_mode)
	{
		batch_databufinfo.batch_info_memPtr = (void *)BATCH_BUFFER_INFO_ADDR_ACCEL1;									/* BATCH INFO */

		batch_databufinfo.batch_mem_start = (void *)(BATCH_BUFFER_DATA_ADDR_ACCEL1) ;										/* BATCH DATA START ADDR*/
		batch_databufinfo.batch_mem_end = (void *)(BATCH_BUFFER_DATA_ADDR_ACCEL1 + BATCH_BUFFER_DATA_SIZE_ACCEL1);		/* BATCH DATA END ADDR, MAX */

		batch_databufinfo.batchSize = 20;//BATCH_BUFFER_DATA_SIZE_ACCEL - (BATCH_BUFFER_DATA_SIZE_ACCEL/2);			/* BATCH SIZE for expired interrupt */

		ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_SET_BATCH, (void*)&batch_databufinfo);
		QL_ASSERT(ret == QL_STATUS_OK);

		ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_GET_BATCH, (void*)&tempbatch_databufinfo);
		QL_ASSERT(ret == QL_STATUS_OK);
		printf("QL_SAL_IOCTL_GET_BATCH 0x%x, [ 0x%x - 0x%x ], %u\n",
				tempbatch_databufinfo.batch_info_memPtr,
				tempbatch_databufinfo.batch_mem_start,
				tempbatch_databufinfo.batch_mem_end,
				tempbatch_databufinfo.batchSize
		);
		printf("\n\n");

		enable = 1;
		ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_BATCH_ENABLE, (void*)&enable);
		QL_ASSERT(ret == QL_STATUS_OK);

		ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_BATCH_GET_ENABLE, (void*)&temp);
		QL_ASSERT(ret == QL_STATUS_OK);
		printf("QL_SAL_IOCTL_BATCH_GET_ENABLE 0x%x\n",temp);
		printf("\n\n");
		//setupBatchTimer(); // for different test mode.
	}
	else
	{
		if(pcg_algo_in_m4)
		{
			databufinfo.buf = (void *)LIVE_SENSOR_DATA_ACCEL2_ADDR;					/* LIVE DATA ADDR */
			databufinfo.size = LIVE_SENSOR_DATA_PACKET_SIZE;						/* LIVE DATA PACKET SIZE */
		}
		else
		{
			databufinfo.buf = (void *)LIVE_SENSOR_DATA_PCG_ADDR;					/* LIVE DATA ADDR */
			databufinfo.size = LIVE_SENSOR_DATA_PACKET_SIZE;						/* LIVE DATA PACKET SIZE */
		}

		ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_SET_DATA_BUF, (void*)&databufinfo );
		QL_ASSERT(ret == QL_STATUS_OK);
		printf("QL_SAL_IOCTL_SET_DATA_BUF 0x%x\n",databufinfo.buf);
		ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_GET_DATA_BUF, (void*)&tempDataBufInfo);
		QL_ASSERT(ret == QL_STATUS_OK);
		printf("QL_SAL_IOCTL_GET_DATA_BUF 0x%x\n",tempDataBufInfo.buf);
		printf("\n\n");

		enable = 1;
		ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_LIVE_DATA_ENABLE, (void*)&enable);
		QL_ASSERT(ret == QL_STATUS_OK);

		ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_GET_LIVE_DATA_ENABLE, (void*)&temp);
		QL_ASSERT(ret == QL_STATUS_OK);
		printf("QL_SAL_IOCTL_GET_LIVE_DATA_ENABLE 0x%x\n",temp);
		printf("\n\n");
	}

#if (!defined( PCG_ALGO_ON_M4_BATCH) && !defined(PCG_AND_DT_ON_M4_BATCH))
	/* enable the sensor */
	enable = 1;
	ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_ENABLE, (void *)&enable);
	QL_ASSERT(ret == QL_STATUS_OK);
	ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_GET_ENABLE, &temp);
	QL_ASSERT(ret == QL_STATUS_OK);
	printf("QL_SAL_IOCTL_GET_ENABLE 0x%x\n",temp);
	printf("\n\n");
#endif /* (!defined( PCG_ALGO_ON_M4_BATCH) || !defined(PCG_AND_DT_ON_M4_BATCH)) */

	/* close the sensor handle, we are done. */
	ret = QL_SAL_SensorClose(handle);
	QL_ASSERT(ret == QL_STATUS_OK);
	printf("QL_SAL_SensorClose OK.\n");
	printf("\n\n");


	if(!pcg_algo_batch_mode && pcg_algo_in_m4) {

#if ( !defined(USE_INTERRUPTS_FOR_LIVE_DATA) || defined(PURE_M4_DRIVERS) )
		vSensorHubCallbackInit(20);
		//vSensorHubChangeTimerPeriod(20);
		vSensorHubStartTimer();
#endif // ( !defined(USE_INTERRUPTS_FOR_LIVE_DATA) || defined(PURE_M4_DRIVERS) )
	}
}

#if (defined (PCG_ALGO_ON_M4_BATCH) || defined PCG_ALGO_ON_M4_LIVE || defined(PCG_ALGO_ON_FFE) || defined(PCG_AND_DT_ON_FFE) || defined (PCG_AND_DT_ON_M4_BATCH))
void EnablePCGAPI(unsigned int sensorid)
{
	QL_Status ret;
	QL_SAL_SensorHandle_t sensor_handle;
	ret = QL_SAL_SensorOpen(&sensor_handle,sensorid);
	QL_ASSERT(ret == QL_STATUS_OK);

	/* enable the sensor */
	enable = 1;
	ret = QL_SAL_SensorIoctl(sensor_handle, QL_SAL_IOCTL_ENABLE, (void *)&enable);
	QL_ASSERT(ret == QL_STATUS_OK);
	ret = QL_SAL_SensorIoctl(sensor_handle, QL_SAL_IOCTL_GET_ENABLE, &temp);
	QL_ASSERT(ret == QL_STATUS_OK);
	printf("QL_SAL_IOCTL_GET_ENABLE 0x%x\n",temp);
	printf("\n\n");

	/* close the sensor handle, we are done. */
	ret = QL_SAL_SensorClose(sensor_handle);
	QL_ASSERT(ret == QL_STATUS_OK);
	printf("QL_SAL_SensorClose OK.\n");
	printf("\n\n");
}
#endif /* (defined (PCG_ALGO_ON_M4_BATCH) || defined PCG_ALGO_ON_M4_LIVE) */

static void PCGAppTask(void *a)
{
	//	QL_Status ret;                 							 // to remove warnings
	BaseType_t qret;
	unsigned int PCGTaskStop = 0;
	//static unsigned char databuf[24] = {0};               
	//QL_SAL_SensorHandle_t handle;
	//struct QL_SF_Ioctl_Req_Events event;
	//struct QL_SF_Ioctl_Set_Data_Buf databufinfo;
	struct PCGMsg_t PCGMsg;
	float accel[3] = {0};
	static const float accel_scaling[3] = {0.0023926, 0.0023926, 0.0023926};

#if defined(HOST_SENSOR_TEST_LED)
    setupContextTimer();
#endif      /* HOST_SENSOR_TEST_LED */

	while(!PCGTaskStop) {
		memset(&PCGMsg, 0, sizeof(PCGMsg));
		qret = xQueueReceive(PCGMsgQ, &PCGMsg, PCG_MGSQ_WAIT_TIME);
		QL_ASSERT(qret == pdTRUE);
		switch(PCGMsg.command) {

		case PCG_CMD_START: 
			QL_ASSERT(PCGMsg.sensorid);
			SetUpPCGAPI(PCGMsg.sensorid);
			break;

		case PCG_CMD_ACCEL_DATA :
			accel[0] = PCGMsg.acceldata.data[0] * accel_scaling[0];
			accel[1] = PCGMsg.acceldata.data[1] * accel_scaling[1];
			accel[2] = PCGMsg.acceldata.data[2] * accel_scaling[2];
			//printf("TS=%d %f %f %f\n", PCGMsg.acceldata.timestamp >> 8, accel[0], accel[1], accel[2]);
			PCGAlgo_Test(accel);
			break;

		case PCG_CMD_PCG_RESULT_FRM_FFE :
			//printf("TS=%d %f %f %f\n", PCGMsg.acceldata.timestamp >> 8, accel[0], accel[1], accel[2]);
			printf("TS=%d Walking Step = %d, Running Step = %d\n", PCGMsg.pcgdata.timestamp >> 8, PCGMsg.pcgdata.WalkingCount, PCGMsg.pcgdata.RunningCount);
			printContext(PCGMsg.pcgdata.ContextGesture >> 16);
			//printf("ContextGesture = %x\n", PCGMsg.pcgdata.ContextGesture);
			break;
		case PCG_CMD_ACCEL_BATCH_DATA :
		{
			int index = 0;
			struct acceldata_t accel_data;
			for ( index = 0; index < PCGMsg.batchdata.numpkt; index++)
			{
				accel_data = *((struct acceldata_t *)(PCGMsg.batchdata.batchdata_addr));
				accel[0] = accel_data.data[0] * accel_scaling[0];
				accel[1] = accel_data.data[1] * accel_scaling[1];
				accel[2] = accel_data.data[2] * accel_scaling[2];

				//printf("TS=%d %f %f %f\n", accel_data.timestamp >> 8, accel[0], accel[1], accel[2]);

				PCGAlgo_Test(accel);

				PCGMsg.batchdata.batchdata_addr =  (void *) ((unsigned int) (PCGMsg.batchdata.batchdata_addr) + sizeof(struct acceldata_t));

				// Take care of wrap around in case of a circular batched buffer
				if((int)(PCGMsg.batchdata.batchdata_addr) >= (int)((BATCH_BUFFER_DATA_ADDR_ACCEL1) + BATCH_BUFFER_DATA_SIZE_ACCEL1 ))
				{
					PCGMsg.batchdata.batchdata_addr = (void *)(BATCH_BUFFER_DATA_ADDR_ACCEL1);
				}
			}
		}
		break;
		case PCG_CMD_ACCEL_ENABLE:
			QL_ASSERT(PCGMsg.sensorid);
#if (defined (PCG_ALGO_ON_M4_BATCH) || defined PCG_ALGO_ON_M4_LIVE || defined(PCG_ALGO_ON_FFE) || defined(PCG_AND_DT_ON_FFE) || defined (PCG_AND_DT_ON_M4_BATCH))
			EnablePCGAPI(PCGMsg.sensorid);
#endif /*(defined (PCG_ALGO_ON_M4_BATCH) || defined PCG_ALGO_ON_M4_LIVE || defined(PCG_ALGO_ON_FFE) || defined(PCG_AND_DT_ON_FFE) || defined (PCG_AND_DT_ON_M4_BATCH))*/
			break;

		default :
			printf("Unknown message\n");
			break;
		}
	}
}

void PCGAppInit(void)
{
	TaskHandle_t PCGTaskHandle;
	PCGMsgQ = xQueueCreate(PCG_MSGQ_ITEMS, sizeof(struct PCGMsg_t));
	QL_ASSERT(PCGMsgQ);
	xTaskCreate(PCGAppTask, "PCGTsk", 1024, NULL, tskIDLE_PRIORITY + 17, &PCGTaskHandle);
	QL_ASSERT(PCGTaskHandle);
}

#endif /* HOST_SENSOR */
