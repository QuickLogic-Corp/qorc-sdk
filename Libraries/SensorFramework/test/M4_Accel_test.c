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
 *    File   : M4_Accel_test.c
 *    Purpose: 
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#include "QL_Trace.h"
#include "QL_SAL.h"
#include "QL_SensorIoctl.h"
#include "QL_SDF_Accel.h"

#include "FreeRTOS.h"
#include "timers.h"

#include <SenseMe_wrapper.h>
#include "DoubleTapLib.h"

#ifdef PURE_M4_DRIVERS


QL_SAL_SensorHandle_t AccelAppSensorHandle;
Accel_data Adata;
TimerHandle_t xATimer=NULL;
static QL_Status TestSensorEventCallback(void *cookie, struct QL_SensorEventInfo *event_info);

void vAccelTimerCB( TimerHandle_t xATimer ) {
	float accel[3]={0};
	double ScaleFactor;
	static int TapCount=0;

	QL_Status ret;
	ret = QL_SAL_SensorRead(AccelAppSensorHandle, &Adata,6);
	QL_ASSERT(ret == QL_STATUS_OK);
	//printf("Accel data read X=%hu,Y=%hu,Z=%hu\n",Adata.dataX, Adata.dataY, Adata.dataZ);

	ScaleFactor = Get_Accel_Scale_Factor();

	accel[0] =  Adata.dataX * ScaleFactor;
	accel[1] = Adata.dataY * ScaleFactor;
	accel[2] = Adata.dataZ * ScaleFactor;
	//printf("Accel data scaled X=%f,Y=%f,Z=%f\n",accel[0], accel[1], accel[2]);
#if !defined(DT_ALGO_ON_FFE) && !defined(PCG_AND_DT_ON_FFE)	
	if (CheckDoubleTap_400(accel))
	{
		printf("Double Tap Detected (Algo in M4)\n");
		TapCount++;
		if(TapCount >= 5) {
			xTimerChangePeriod(xATimer,pdMS_TO_TICKS(1000),pdMS_TO_TICKS(100));
			TapCount = 0;
		}
	}
#endif /* !defined(DT_ALGO_ON_FFE) && !defined(PCG_AND_DT_ON_FFE) */    
}

void vOpenM4Accel(void){
	QL_Status ret;
	struct QL_SF_Ioctl_Req_Events event,tempEvent;
	unsigned int sensorDynamicRange = 2;					// 0 maps to +-2G, 1 maps to +-4G, 2 maps to +-8G, 3 maps to +-16G
	unsigned int sensorODR = 25;
	unsigned int sensorAxisDirection = 0,temp;
	unsigned int batch = 400;

	ret = QL_SAL_SensorOpen(&AccelAppSensorHandle,QL_SAL_SENSOR_ID_DOUBLE_TAP);
	QL_ASSERT(ret == QL_STATUS_OK);
	printf("QL_SAL_SensorOpen OK.\n");

	event.cookie = (void*) 0xBAD;
	event.event_callback = TestSensorEventCallback;
	ret = QL_SAL_SensorIoctl(AccelAppSensorHandle, QL_SAL_IOCTL_SET_CALLBACK, (void*)&event);
	QL_ASSERT(ret == QL_STATUS_OK);
	ret = QL_SAL_SensorIoctl(AccelAppSensorHandle, QL_SAL_IOCTL_GET_CALLBACK, &tempEvent);
	QL_ASSERT(ret == QL_STATUS_OK);
	printf("QL_SAL_IOCTL_GET_CALLBACK: 0x%x, %x\n",tempEvent.event_callback, tempEvent.cookie);
	printf("CallBack Function Address: 0x%x\n",TestSensorEventCallback);
	printf("\n\n");


	ret = QL_SAL_SensorIoctl(AccelAppSensorHandle, QL_SAL_IOCTL_SET_DYNAMIC_RANGE, (void*)&sensorDynamicRange);
	QL_ASSERT(ret == QL_STATUS_OK);
	ret = QL_SAL_SensorIoctl(AccelAppSensorHandle, QL_SAL_IOCTL_GET_DYNAMIC_RANGE, &temp);
	QL_ASSERT(ret == QL_STATUS_OK);
	printf("QL_SAL_IOCTL_GET_DYNAMIC_RANGE 0x%x\n",temp);
	printf("\n\n");

	ret = QL_SAL_SensorIoctl(AccelAppSensorHandle, QL_SAL_IOCTL_SET_ODR, (void*)&sensorODR);
	QL_ASSERT(ret == QL_STATUS_OK);
	ret = QL_SAL_SensorIoctl(AccelAppSensorHandle, QL_SAL_IOCTL_GET_ODR, &temp);
	QL_ASSERT(ret == QL_STATUS_OK);
	printf("QL_SAL_IOCTL_GET_ODR 0x%x\n",temp);
	printf("\n\n");

#if 0
	ret = QL_SAL_SensorIoctl(AccelAppSensorHandle, QL_SAL_IOCTL_SET_AXIS_DIRECTION, (void*)&sensorAxisDirection);
	QL_ASSERT(ret == QL_STATUS_OK);
	ret = QL_SAL_SensorIoctl(AccelAppSensorHandle, QL_SAL_IOCTL_GET_AXIS_DIRECTION, &temp);
	QL_ASSERT(ret == QL_STATUS_OK);
	printf("QL_SAL_IOCTL_GET_AXIS_DIRECTION 0x%x\n",temp);
#endif

	ret = QL_SAL_SensorIoctl(AccelAppSensorHandle, QL_SAL_IOCTL_SET_BATCH, (void*)&batch);
	QL_ASSERT(ret == QL_STATUS_OK);
	ret = QL_SAL_SensorIoctl(AccelAppSensorHandle, QL_SAL_IOCTL_GET_BATCH, &temp);
	QL_ASSERT(ret == QL_STATUS_OK);
	printf("QL_SAL_IOCTL_GET_BATCH 0x%x\n",temp);
	printf("\n\n");

	//xTimerChangePeriod(xATimer,pdMS_TO_TICKS(2),portMAX_DELAY);
	printf("\n\n");
}

void vCloseM4Accel(void)
{
	QL_Status ret;
	ret = QL_SAL_SensorClose(AccelAppSensorHandle);
	QL_ASSERT(ret == QL_STATUS_OK);
}

static QL_Status TestSensorEventCallback(void *cookie, struct QL_SensorEventInfo *event_info){

	QL_TRACE_TEST_DEBUG("+\n");
	QL_ASSERT(event_info);
	QL_ASSERT(event_info->numpkt);
	QL_ASSERT(event_info->data);

	Accel_data* pdata;
	printf("\n\n");
	printf("cookie = %x, event_info->event = %x, "
			"event_info->data = %x, event_info->numpkt = %x\n",
			(int) cookie, event_info->event, event_info->data,
			event_info->numpkt);

	pdata = (Accel_data *) event_info->data;

	if (QL_SENSOR_EVENT_BATCH == event_info->event )
	{
		for(int i=0;i< event_info->numpkt;i++){
			printf("Accel data read X=%hd,Y=%hd,Z=%hd\n",pdata->dataX, pdata->dataY, pdata->dataZ);
			pdata++;
		}
	}
}

void Accel_test(int cmd) {

	if(xATimer == NULL) {
		xATimer = xTimerCreate("Accel_Timer", pdMS_TO_TICKS(2),pdTRUE, (void*)0,(TimerCallbackFunction_t)vAccelTimerCB);
		if(xATimer == NULL) {
			printf("Error in creating the Timer");
			return;
		}
	}

	switch(cmd) {
	case 0:
		xTimerStop(xATimer,portMAX_DELAY);
		vCloseM4Accel();
		break;
	case 1:
		vOpenM4Accel();
		//xTimerStart(xATimer,portMAX_DELAY);
		break;
	}
}

#endif // PURE_M4_DRIVERS
