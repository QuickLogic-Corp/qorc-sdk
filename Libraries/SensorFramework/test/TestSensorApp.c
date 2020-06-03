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
 *    File   : TestSensorApp.c
 *    Purpose: 
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#include "QL_Trace.h"
#include "QL_SAL.h"
#include "QL_SensorIoctl.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#ifdef HOST_SENSOR

static QL_Status SensorEventCallback(void *cookie, struct QL_SensorEventInfo *event_info)
{
	QL_TRACE_TEST_DEBUG("+\n");
	QL_ASSERT(event_info);
	QL_ASSERT(event_info->numpkt);
	QL_ASSERT(event_info->data);
	
	QL_TRACE_TEST_DEBUG("cookie = %x, event_info->event = %x, "
				"event_info->data = %x, event_info->numpkt = %x\n",
				(int) cookie, event_info->event, event_info->data,
				event_info->numpkt);
	QL_TRACE_TEST_DEBUG("-\n");
	printf("DoubleTap Detected\n");
        return QL_STATUS_OK;                    			// to remove warnings  added return statement
}

static void TestSensorAppTask(void *a)
{
	QL_Status ret;
	static unsigned char databuf[24] = {0};
	QL_SAL_SensorHandle_t handle;
	struct QL_SF_Ioctl_Req_Events event;
	struct QL_SF_Ioctl_Set_Data_Buf databufinfo;
	databufinfo.buf = databuf;
	databufinfo.size = sizeof(databuf);

	ret = QL_SAL_SensorOpen(&handle, QL_SAL_SENSOR_ID_DOUBLE_TAP);
	QL_ASSERT(ret == QL_STATUS_OK);
	
	event.cookie = (void*) 0xBAD;
	event.event_callback = SensorEventCallback;

	ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_SET_CALLBACK, &event);
	QL_ASSERT(ret == QL_STATUS_OK);

	ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_SET_ODR, (void*)400);
	QL_ASSERT(ret == QL_STATUS_OK);

	ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_SET_DATA_BUF, (void*)&databufinfo);
	QL_ASSERT(ret == QL_STATUS_OK);

	ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_ENABLE, (void*)1);
	QL_ASSERT(ret == QL_STATUS_OK);
	
	
	while(1) {
		vTaskDelay(1000);
	};
}

void TestSensorAppInit(void)
{
	TaskHandle_t SensorTestTaskHandle;
	xTaskCreate(TestSensorAppTask, "SenTstTsk", 1024, NULL, tskIDLE_PRIORITY + 2, &SensorTestTaskHandle);
	QL_ASSERT( SensorTestTaskHandle );
}

#endif /* HOST_SENSOR */
