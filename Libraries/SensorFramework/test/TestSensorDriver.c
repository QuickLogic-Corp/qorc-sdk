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
 *    File   : TestSensorDriver.c
 *    Purpose: 
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#include "QL_SDF.h"
#include "QL_SAL.h"

#include "QL_SensorIoctl.h"
#include "QL_Trace.h"
#include <stdio.h>
#include <string.h>												// to remove warnings

#include "FreeRTOS.h"
#include "task.h"

#define MAX_INSTANCE 2

#ifdef HOST_SENSOR

struct test_sensor_instance {
	QL_SensorEventCallback_t cb;
	void *cookie;
};

static struct test_sensor_instance instance[MAX_INSTANCE];
static struct QL_SDF_SensorDrv test_sensor_drv;
static int dataToBePushed;

static QL_Status test_sendev_open(struct QL_SDF_SensorDrv *s, QL_SDF_SensorDrvInstanceHandle_t *instance_handle)
{
	QL_ASSERT(s);
	QL_ASSERT(instance_handle);

	QL_TRACE_TEST_DEBUG("+\n");
	if (s != &test_sensor_drv) {
		QL_TRACE_TEST_ERROR("passed handle to sensor driver is invalid\n");
		return QL_STATUS_ERROR;
	}
	
	*instance_handle = (QL_SDF_SensorDrvInstanceHandle_t)1;
	QL_TRACE_TEST_DEBUG("-\n");
	return QL_STATUS_OK;
}

static QL_Status test_sendev_close(QL_SDF_SensorDrvInstanceHandle_t instance_handle)
{
	return QL_STATUS_OK;
}

static QL_Status test_sendev_probe(struct QL_SDF_SensorDrv *sdev)
{
	QL_TRACE_TEST_DEBUG("+\n");
	return QL_STATUS_OK;
}

static QL_Status test_sendev_read(QL_SDF_SensorDrvInstanceHandle_t instance_handle, void *buf, int size)
{
	QL_TRACE_TEST_DEBUG("+\n");
	return QL_STATUS_OK;
}
	
static QL_Status test_sendev_ioctl(QL_SDF_SensorDrvInstanceHandle_t instance_handle, unsigned int cmd, void *arg)
{
	QL_TRACE_TEST_DEBUG("+\n");
	switch (cmd) {
	case QL_SAL_IOCTL_SET_CALLBACK :
		QL_TRACE_TEST_DEBUG("QL_SAL_IOCTL_REQ_EVENTS handle = %d\n", (int)instance_handle);
		instance[(int)instance_handle].cb = ((struct QL_SF_Ioctl_Req_Events*) arg)->event_callback;
		instance[(int)instance_handle].cookie = ((struct QL_SF_Ioctl_Req_Events*) arg)->cookie;
		dataToBePushed = 1;
		break;
	default :
		QL_TRACE_TEST_ERROR("unknown sensor\n");
	};
	QL_TRACE_TEST_DEBUG("-\n");
	return QL_STATUS_OK;
}

static struct QL_SDF_SensorDrv test_sensor_drv = {
	.id = QL_SAL_SENSOR_ID_ACCEL,
	.name = "ACCEL_TEST_SENSOR",
	.ops = {
		.open = &test_sendev_open,
		.close = &test_sendev_close,
		.probe = &test_sendev_probe,
		.read = &test_sendev_read,
		.ioctl = &test_sendev_ioctl,
	},
};

static QL_Status test_sendev_pushdata(int instanceIndex, int pushcount)
{
	struct QL_SensorEventInfo event_info;
	unsigned char data[5], i = 1;
	QL_Status ret = QL_STATUS_OK;


	while (pushcount--) {
		event_info.event = QL_SENSOR_EVENT_DATA;
		event_info.data = data;
		event_info.numpkt = 1;
		memset(data, i++, sizeof(data));
		ret = instance[instanceIndex].cb(instance[instanceIndex].cookie, &event_info);
		if (ret != QL_STATUS_OK) {
			QL_TRACE_TEST_ERROR("callback failed\n");	
		}
	}
	return ret;
}

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
        
        return QL_STATUS_OK;                    // to remove warnings      added return statement
}


extern QL_Status QL_SensorFramework_Init(void);                 // to remove warnings
static void SensorTestTaskHandler(void *arg)
{
	QL_Status ret;
	QL_SAL_SensorHandle_t handle;
	struct QL_SF_Ioctl_Req_Events event;

	event.cookie = (void*) 0xBAD;
	event.event_callback = SensorEventCallback;

	vTaskDelay(1000);
	QL_SensorFramework_Init();

	ret = QL_SAL_SensorOpen(&handle, QL_SAL_SENSOR_ID_ACCEL);
	QL_ASSERT(ret == QL_STATUS_OK);
	ret = QL_SAL_SensorIoctl(handle, QL_SAL_IOCTL_SET_CALLBACK, &event);
	QL_ASSERT(ret == QL_STATUS_OK);

	while(1) {
		vTaskDelay(1000);
		if (dataToBePushed) {
			test_sendev_pushdata(1, 5);
		}
	};
}

int init_test_sensor(void)
{
	QL_Status ret;
	TaskHandle_t SensorTestTaskHandle;

	ret = QL_SDF_SensorDrvRegister(&test_sensor_drv);
	if (ret != QL_STATUS_OK) {
		QL_TRACE_SENSOR_ERROR("sensor_register returned error\n");		
	}
	xTaskCreate(SensorTestTaskHandler, "SenTstTsk", 1024, NULL, tskIDLE_PRIORITY + 2, &SensorTestTaskHandle);
	QL_ASSERT( SensorTestTaskHandle );

	return 0;
}

#endif /* HOST_SENSOR */
