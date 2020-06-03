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
 *    File   : ql_SensorHubTimerCallback.c
 *    Purpose: 
 * The SensorHub Timer Callback is required for:
 * (1) FFE Drivers/Hybrid Drivers
 * [non-batch mode only]: The live sensor data is written into SRAM
 * by the FFE side, and we need to poll this data at regular intervals
 * matching the FFE time period in all cases. Once we get the sensor data
 * we call the callback registered for the driver with data events.
 *
 * (2) Pure M4 Drivers
 * [non-batch mode] The live sensor data is "read" using the sensor driver
 * directly using I2C at the callback, and we call the callbacks registered
 * for each driver with data events
 *
 * [batch mode] the live sensor data is "read" using the sensor driver using
 * I2C, and put into the batch memory already setup in the driver.
 * The driver can have an IOCTL to handle this internally, and can call the call
 * back registered with the driver with batch events.
 * batch flush command however is not supported (yet) with Pure M4 Drivers.
 * this would be done later.
*=========================================================*/
#include "Fw_global_config.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "Fw_global_config.h"
#include <ql_SensorHub_MemoryMap.h>

#include "QL_Trace.h"
#include "QL_SAL.h"
#include "QL_SensorIoctl.h"

#include <ffe_ipc_lib.h>
#include <string.h>

#if (defined(HOST_SENSOR) || defined(COMPANION_SENSOR))

QL_SAL_SensorHandle_t sensorHubTimerSensorHandle_Accel;
QL_SAL_SensorHandle_t sensorHubTimerSensorHandle_Accel2;
QL_SAL_SensorHandle_t sensorHubTimerSensorHandle_Gyro;
QL_SAL_SensorHandle_t sensorHubTimerSensorHandle_Mag;


QL_SensorEventCallback_t callback_Accel;
QL_SensorEventCallback_t callback_Accel2;
QL_SensorEventCallback_t callback_Mag;
QL_SensorEventCallback_t callback_Gyro;

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

TimerHandle_t xSensorHubTimer = NULL;

void vSensorHubTimerCB( TimerHandle_t xSensorHubTimer )
{
	//printf("CB\n");
#if (defined(FFE_DRIVERS) || defined(HYBRID_FFE_M4_DRIVERS))
	struct sensorData_Accel accelData2;
	struct sensorData_Mag magData;
	struct sensorData_Gyro gyroData;
#endif

#ifdef PURE_M4_DRIVERS
	short signed int accelBuf[3] = {0};
	QL_Status ret;
#endif

#if (defined(FFE_DRIVERS) || defined (HYBRID_FFE_M4_DRIVERS) || defined(PURE_M4_DRIVERS))
	static struct sensorData_Accel accelData;
	struct QL_SensorEventInfo event_info;
	void* cookie;
#endif

        //vSensorHubStartTimer(); // restart timer

#if (defined(FFE_DRIVERS) || defined(HYBRID_FFE_M4_DRIVERS))

	memcpy((void *)&accelData, (const void *)LIVE_SENSOR_DATA_ACCEL_ADDR, sizeof(accelData));
	memcpy((void *)&accelData2, (const void *)LIVE_SENSOR_DATA_ACCEL2_ADDR, sizeof(accelData));
	memcpy((void *)&magData, (const void *)LIVE_SENSOR_DATA_MAG_ADDR, sizeof(magData));
	memcpy((void *)&gyroData, (const void *)LIVE_SENSOR_DATA_GYRO_ADDR, sizeof(gyroData));

	if(callback_Accel != NULL)
	{
		cookie = (void *)0xFEED;
		event_info.event = QL_SENSOR_EVENT_DATA;
		event_info.data = &accelData;
		callback_Accel(cookie, &event_info);
	}

	if(callback_Accel2 != NULL)
	{
		cookie = (void *)0xFEED;
		event_info.event = QL_SENSOR_EVENT_DATA;
		event_info.data = &accelData2;
		callback_Accel2(cookie, &event_info);
	}

	if(callback_Mag != NULL)
	{
		cookie = (void *)0xFEED;
		event_info.event = QL_SENSOR_EVENT_DATA;
		event_info.data = &magData;
		callback_Mag(cookie, &event_info);
	}

	if(callback_Gyro != NULL)
	{
		cookie = (void *)0xFEED;
		event_info.event = QL_SENSOR_EVENT_DATA;
		event_info.data = &gyroData;
		callback_Gyro(cookie, &event_info);
	}

#endif // #if (defined(FFE_DRIVERS) || defined(HYBRID_FFE_M4_DRIVERS))


#if (defined(PURE_M4_DRIVERS))

	ret = QL_SAL_SensorRead(sensorHubTimerSensorHandle_Accel, accelBuf, 6);
	QL_ASSERT(ret == QL_STATUS_OK);


	// TODO KK: set the scale factor correctly here ?
	//double lScaleFactor = Get_Accel_Scale_Factor();

	accelData.data[0] = accelBuf[0];// * lScaleFactor;
	accelData.data[1] = accelBuf[1];// * lScaleFactor;
	accelData.data[2] = accelBuf[2];// * lScaleFactor;
	accelData.timestamp = xTaskGetTickCount() << 8; // to match how FFE sends TS
	accelData.len = 12;

        //printf("TS:[%u],%d,Y=%d,Z=%d\n",accelData.timestamp >> 8, accelData.data[0], accelData.data[1], accelData.data[2]);

	if(callback_Accel != NULL)
	{
		cookie = (void *)0xFEED;
		event_info.event = QL_SENSOR_EVENT_DATA;
		event_info.data = &accelData;
		callback_Accel(cookie, &event_info);
	}

#ifdef ENABLE_PURE_M4_GYRO
    //use the the same accel buf
	ret = QL_SAL_SensorRead(sensorHubTimerSensorHandle_Gyro, accelBuf, 6);
	QL_ASSERT(ret == QL_STATUS_OK);

	static struct sensorData_Gyro gyroData;

	gyroData.data[0] = accelBuf[0];// * lScaleFactor;
	gyroData.data[1] = accelBuf[1];// * lScaleFactor;
	gyroData.data[2] = accelBuf[2];// * lScaleFactor;
	gyroData.timestamp = xTaskGetTickCount() << 8; // to match how FFE sends TS
	gyroData.len = 12;
  	if(callback_Gyro != NULL)
	{
		cookie = (void *)0xFEED;
		event_info.event = QL_SENSOR_EVENT_DATA;
		event_info.data = &gyroData;
		callback_Gyro(cookie, &event_info);
	}

#endif

#endif // if (defined(PURE_M4_DRIVERS))

        vSensorHubStartTimer(); // restart timer

}



QL_Status vSensorHubCreatetimer(unsigned int periodInms)
{
	if(xSensorHubTimer == NULL)
	{
		xSensorHubTimer = xTimerCreate("SensorHubTimer", pdMS_TO_TICKS(periodInms),pdFALSE, (void*)0,(TimerCallbackFunction_t)vSensorHubTimerCB);
		if(xSensorHubTimer == NULL)
		{
			return QL_STATUS_ERROR;
		}
	}

	return QL_STATUS_OK;
}

void vSensorHubStartTimer()
{
	xTimerStart(xSensorHubTimer,pdMS_TO_TICKS(10));
}

void vSensorHubStopTimer()
{
	xTimerStop(xSensorHubTimer,pdMS_TO_TICKS(10));
}

// this should be called to set the right time period.
void vSensorHubChangeTimerPeriod(unsigned int timeInms)
{
	xTimerChangePeriod(xSensorHubTimer,pdMS_TO_TICKS(timeInms),pdMS_TO_TICKS(10));
}


/* sensor callbacks should have been registered BEFORE calling this ! */
QL_Status vSensorHubCallbackInit(unsigned int periodInms)
{
	QL_Status ret= QL_STATUS_OK;
	if(vSensorHubCreatetimer(periodInms) != QL_STATUS_OK)
	{
		QL_LOG_ERR_150K("Error in creating SensorHubTimer");
		return QL_STATUS_ERROR;
	}

	//printf("SensorHubTimer created\n");

#ifdef PURE_M4_DRIVERS

	ret = QL_SAL_SensorOpen(&sensorHubTimerSensorHandle_Accel, QL_SAL_SENSOR_ID_ACCEL);
	QL_ASSERT(ret == QL_STATUS_OK);

	ret = QL_SAL_SensorIoctl(sensorHubTimerSensorHandle_Accel, QL_SAL_IOCTL_GET_CALLBACK, (void *)&callback_Accel);
	QL_ASSERT(ret == QL_STATUS_OK);

	//ret = QL_SAL_SensorClose(sensorHubTimerSensorHandle_Accel);
	//QL_ASSERT(ret == QL_STATUS_OK);

#ifdef ENABLE_PURE_M4_GYRO

	ret = QL_SAL_SensorOpen(&sensorHubTimerSensorHandle_Gyro, QL_SAL_SENSOR_ID_GYRO);
	QL_ASSERT(ret == QL_STATUS_OK);

	ret = QL_SAL_SensorIoctl(sensorHubTimerSensorHandle_Gyro, QL_SAL_IOCTL_GET_CALLBACK, (void *)&callback_Gyro);
	QL_ASSERT(ret == QL_STATUS_OK);


#endif

#endif // PURE_M4_DRIVERS

#if (defined (FFE_DRIVERS) || defined (HYBRID_FFE_M4_DRIVERS))
	ret = QL_SAL_SensorOpen(&sensorHubTimerSensorHandle_Accel, QL_SAL_SENSOR_ID_ACCEL);
	QL_ASSERT(ret == QL_STATUS_OK);

	ret = QL_SAL_SensorIoctl(sensorHubTimerSensorHandle_Accel, QL_SAL_IOCTL_GET_CALLBACK, (void *)&callback_Accel);
	QL_ASSERT(ret == QL_STATUS_OK);

	ret = QL_SAL_SensorClose(sensorHubTimerSensorHandle_Accel);
	QL_ASSERT(ret == QL_STATUS_OK);



	ret = QL_SAL_SensorOpen(&sensorHubTimerSensorHandle_Accel2, QL_SAL_SENSOR_ID_ACCEL2);
	QL_ASSERT(ret == QL_STATUS_OK);

	ret = QL_SAL_SensorIoctl(sensorHubTimerSensorHandle_Accel2, QL_SAL_IOCTL_GET_CALLBACK, (void *)&callback_Accel2);
	QL_ASSERT(ret == QL_STATUS_OK);

	ret = QL_SAL_SensorClose(sensorHubTimerSensorHandle_Accel2);
	QL_ASSERT(ret == QL_STATUS_OK);



	ret = QL_SAL_SensorOpen(&sensorHubTimerSensorHandle_Mag, QL_SAL_SENSOR_ID_MAG);
	QL_ASSERT(ret == QL_STATUS_OK);

	ret = QL_SAL_SensorIoctl(sensorHubTimerSensorHandle_Mag, QL_SAL_IOCTL_GET_CALLBACK, (void *)&callback_Mag);
	QL_ASSERT(ret == QL_STATUS_OK);

	ret = QL_SAL_SensorClose(sensorHubTimerSensorHandle_Mag);
	QL_ASSERT(ret == QL_STATUS_OK);



	ret = QL_SAL_SensorOpen(&sensorHubTimerSensorHandle_Gyro, QL_SAL_SENSOR_ID_GYRO);
	QL_ASSERT(ret == QL_STATUS_OK);

	ret = QL_SAL_SensorIoctl(sensorHubTimerSensorHandle_Gyro, QL_SAL_IOCTL_GET_CALLBACK, (void *)&callback_Gyro);
	QL_ASSERT(ret == QL_STATUS_OK);

	ret = QL_SAL_SensorClose(sensorHubTimerSensorHandle_Gyro);
	QL_ASSERT(ret == QL_STATUS_OK);
#endif // (defined (FFE_DRIVERS) || defined (HYBRID_FFE_M4_DRIVERS))

	return ret;
}


QL_Status vSensorHubCallbackDeInit()
{
	vSensorHubStopTimer();
	return QL_STATUS_OK;
}

#endif  /* (defined(HOST_SENSOR) || defined(COMPANION_SENSOR)) */
