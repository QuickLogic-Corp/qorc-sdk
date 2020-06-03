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
 *    File   : Accel2_FFE_Drv.c
 *    Purpose: 
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#include <stdio.h>
#include "QL_Trace.h"
#include "QL_SDF.h"
#include "QL_SensorIoctl.h"
#include "ffe_ipc_lib.h"
#include "QL_FFE_SensorConfig.h"
//#include "ql_testTask.h"
#include "Fw_global_config.h"
#include "QL_SensorCommon.h"
#include "QL_SDF_Accel.h"
//#define __PLATFORM_INIT__                                     

#ifdef FFE_DRIVERS


/*
 * if the probe is to be run on M4 instead of FFE, enable this flag
 * Default behavior is probe running on FFE
 */
//#define CONFIG_ACCEL2_HYBRID_DRV_PROBE_IN_M4

// similarly, we can have flags for all driver functions, to switch from FFE to M4
// or we can have one flag for all drivers which go together...

/*
 * State Machine for the Sensor and its Internal Instances :
 *
 * all instances are at SENSOR_STATE_INVALID initially
 * if probe() is ok, all instances are moved to SENSOR_STATE_PROBED
 * if probe() fails, all instances are moved back to SENSOR_STATE_INVALID
 * open() will move state from SENSOR_STATE_PROBED -> SENSOR_STATE_OPENED
 * close() will move state from SENSOR_STATE_OPENED -> SENSOR_STATE_PROBED
 * we do not need SENSOR_STATE_CLOSED !
 * right now we do not have an explicit call to deinit the sensor, i.e. a complementary to probe() -> teardown() ?
 * (so we can transition from SENSOR_STATE_PROBED -> SENSOR_STATE_INVALID)
 * and if required, we will add this. currently we do not see any need for this.
 *
 */

/*
 * Private Structure to maintain the states of the sensor, as well as the states of individual instances held by clients *
 * FFE_SENSOR_ID will need to be maintained - common for all instances
 * FFE_INSTANCE_STATE - individually for each instance
 */

static QL_Accel_Ffe_Drv accel_drv_priv;

// private function declarations
static QL_Status Accel_probe(struct QL_SDF_SensorDrv *s);
static QL_Status Accel_Open(struct QL_SDF_SensorDrv *s, QL_SDF_SensorDrvInstanceHandle_t *instance_handle);
static QL_Status Accel_Close(QL_SDF_SensorDrvInstanceHandle_t instance_handle);
static QL_Status Accel_read(QL_SDF_SensorDrvInstanceHandle_t instance_handle, void *buf, int size);
static QL_Status Accel_ioctl(QL_SDF_SensorDrvInstanceHandle_t instance_handle, unsigned int cmd, void *arg);
static QL_Status Accel_suspend(QL_SDF_SensorDrvInstanceHandle_t instance_handle);
static QL_Status Accel_resume(QL_SDF_SensorDrvInstanceHandle_t instance_handle);
//QL_Status Accel_FFE_IsArgValid(unsigned int cmd, void* arg);



static QL_Status Accel_probe(struct QL_SDF_SensorDrv *s)
{
	unsigned int index;

	QL_ASSERT(s != NULL);
	QL_ASSERT(s == (struct QL_SDF_SensorDrv *)&accel_drv_priv); // we should get back the same instance that we provided during registration

	/*
	 * probe should be called once, to check if sensor is alive, and the states of each instance should be changed
	 * to reflect the result of the probe.
	 *
	 */

	if (QL_STATUS_OK != QL_FFE_Sensor_Probe(accel_drv_priv.ffe_accel_id))
	{
		QL_TRACE_SENSOR_ERROR("QL_FFE_Sensor_Probe() failed for FFE sensor with ID = %d\n",accel_drv_priv.ffe_accel_id);

		/* probe failed, update the state of each of the instances to not ready, SENSOR_STATE_INVALID */
		for ( index = 0 ; index < CONFIG_MAX_FFE_ACCEL_INSTANCE; index++ )
		{
			accel_drv_priv.devInstances[index].state = SENSOR_STATE_INVALID;
		}

		return QL_STATUS_ERROR;
	}

	QL_TRACE_SENSOR_DEBUG("QL_FFE_Sensor_Probe() OK for FFE sensor with ID = %d\n",accel_drv_priv.ffe_accel_id);


	/* probe is ok, update the state of each of the instances to ready to use, SENSOR_STATE_PROBED */
	for ( index = 0 ; index < CONFIG_MAX_FFE_ACCEL_INSTANCE; index++ )
	{
		accel_drv_priv.devInstances[index].state = SENSOR_STATE_PROBED;
	}

	return QL_STATUS_OK;
}


static QL_Status Accel_Open(struct QL_SDF_SensorDrv *s, QL_SDF_SensorDrvInstanceHandle_t *instance_handle)
{
	unsigned int index ;

	*instance_handle = (QL_SDF_SensorDrvInstanceHandle_t)0xFFFF;
	QL_ASSERT(s != NULL);
	QL_ASSERT(s == (struct QL_SDF_SensorDrv *) &accel_drv_priv); // we should get back the same  that we provided during registration

	for ( index = 0 ; index < CONFIG_MAX_FFE_ACCEL_INSTANCE; index++ )
	{
//		printf("instance index: %d, state: %d\n",index, accel_drv_priv.devInstances[index].state);
		/* if we have an instance slot with state == SENSOR_STATE_PROBED, this is ready to use */
		if (SENSOR_STATE_PROBED == accel_drv_priv.devInstances[index].state)
		{
			/* move this instance's state to SENSOR_STATE_OPENED to mark that this slot is now in use */
			accel_drv_priv.devInstances[index].state = SENSOR_STATE_OPENED;

			/* pass back the index of the instance in our private table to Framework, as our "Instance Handle" */
			*instance_handle = (QL_SDF_SensorDrvInstanceHandle_t) index;

			/* indicate to framework that open succeeded */
			return QL_STATUS_OK;
		}
	}

	/* if we reach here, we do not have any free slots for further instances, indicate to framework that open failed */
	return QL_STATUS_ERROR;
}


static QL_Status Accel_Close(QL_SDF_SensorDrvInstanceHandle_t instance_handle)
{
	/* KK: check validity of handle. This is probably not needed and can be removed */
	QL_ASSERT((unsigned int) instance_handle < CONFIG_MAX_FFE_ACCEL_INSTANCE);

	/* move the state of this instance back to ready to use, SENSOR_STATE_PROBED */
	accel_drv_priv.devInstances[(unsigned int)instance_handle].state = SENSOR_STATE_PROBED;

	/* indicate to framework that the instance was closed successfully */
	return QL_STATUS_OK;
}


static QL_Status Accel_read(QL_SDF_SensorDrvInstanceHandle_t instance_handle, void *buf, int size)
{
	/* KK: read data from sensor is not implemented yet, no use case ? */
	return QL_STATUS_NOT_IMPLEMENTED;
}

#if 0 //Not used
static QL_Status Accel_FFE_IsArgValid(unsigned int cmd, void* arg)
{
	/* for all the "set"/"get" calls, we check that the arguments passed in are valid */
	/* for example, Accelerometer Dynamic Range -> arg = 0/1/2/3, any other is invalid */
	/* this function has to be tailored for the specific sensor instantiated, it will differ, if a different sensor is employed */
	switch (cmd)
	{
		case QL_SAL_IOCTL_SET_DYNAMIC_RANGE:
		{
			unsigned int dynamicRange = *(unsigned int *)arg;
			if(!(dynamicRange <= 3)) return QL_STATUS_ERROR;
		}
		break;

		case QL_SAL_IOCTL_SET_ODR:
		{
			unsigned int odr = *(unsigned int *)arg;
			if(!((odr >= 25) && (odr <= 1600) && (odr%25 == 0)))  return QL_STATUS_ERROR;
		}
		break;
	}

	/* default, for unknown command, we do not know to check for validity, so it will be a dummy OK */
	/* alternative is to return error by default, so that we have a check for each and every command possible added here ? */
	return QL_STATUS_OK;
}
#endif


static QL_Status Accel_ioctl(QL_SDF_SensorDrvInstanceHandle_t instance_handle, unsigned int cmd, void *arg)
{
	unsigned int sensorid;
	QL_Status status = QL_STATUS_OK;

	sensorid = accel_drv_priv.ffe_accel_id;
	QL_ASSERT( (unsigned int)instance_handle < CONFIG_MAX_FFE_ACCEL_INSTANCE);

//	printf("sensorid: %u,cmd: %u\r\n", sensorid, cmd);

	/* check if the instance is in open state first */
	if (SENSOR_STATE_OPENED != accel_drv_priv.devInstances[(unsigned int) instance_handle].state)
	{
		return QL_STATUS_ERROR;
	}

	/* before invoking the command implementation, ensure that the arguments are valid */
	/* the arg check function *must* be specific to the end sensor being used and has to be checked ! */
//	if(QL_STATUS_OK != Accel_FFE_IsArgValid(cmd, arg))
//	{
//		QL_TRACE_SENSOR_ERROR("Command %d Arguments Invalid for sensor ID = %d\n", cmd, sensorid);
//		return QL_STATUS_ERROR;
//	}

	switch (cmd)
	{
		case QL_SAL_IOCTL_SET_CALLBACK:
		{
			accel_drv_priv.drvData.sensor_callback = (QL_SensorEventCallback_t)arg;
		}
		break;

		case QL_SAL_IOCTL_GET_CALLBACK:
		{
			*(QL_SensorEventCallback_t *)arg = accel_drv_priv.drvData.sensor_callback;
		}
		break;

		case QL_SAL_IOCTL_BATCH_ENABLE:
		{
			if (QL_STATUS_OK != QL_FFE_OperationForSensor(sensorid, QL_FFE_SENSOR_OPS_BATCH_ENABLE, arg))
			{
				QL_TRACE_SENSOR_ERROR("QL_FFE_SENSOR_OPS_BATCH_ENABLE failed for FFE sensor ID = %d \n", sensorid);
				status = QL_STATUS_ERROR;
			}
		}
		break;

		case QL_SAL_IOCTL_BATCH_GET_ENABLE:
		{
			if (QL_STATUS_OK != QL_FFE_OperationForSensor(sensorid, QL_FFE_SENSOR_OPS_BATCH_GET_ENABLE, arg))
			{
				QL_TRACE_SENSOR_ERROR("QL_FFE_SENSOR_OPS_BATCH_GET_ENABLE failed for FFE sensor ID = %d \n", sensorid);
				status = QL_STATUS_ERROR;
			}
		}
		break;

		case QL_SAL_IOCTL_SET_BATCH:
		{
			if (QL_STATUS_OK != QL_FFE_OperationForSensor(sensorid, QL_FFE_SENSOR_OPS_SET_BATCH, arg))
			{
				QL_TRACE_SENSOR_ERROR("QL_FFE_SENSOR_OPS_SET_BATCH failed for FFE sensor ID = %d \n", sensorid);
				status = QL_STATUS_ERROR;
			}
		}
		break;

		case QL_SAL_IOCTL_GET_BATCH:
		{
			if (QL_STATUS_OK != QL_FFE_OperationForSensor(sensorid, QL_FFE_SENSOR_OPS_GET_BATCH, arg))
			{
				QL_TRACE_SENSOR_ERROR("QL_FFE_SENSOR_OPS_GET_BATCH failed for FFE sensor ID = %d \n", sensorid);
				status = QL_STATUS_ERROR;
			}
		}
		break;


		case QL_SAL_IOCTL_BATCH_FLUSH:
		{
//			static int flushcounter=0;
//			printf ( "batch flush count = %d\n", ++flushcounter); //TBD: take it out
			if (QL_STATUS_OK != QL_FFE_OperationForSensor(sensorid, QL_FFE_SENSOR_OPS_BATCH_FLUSH, arg))
			{
				QL_TRACE_SENSOR_ERROR("QL_FFE_SENSOR_OPS_BATCH_FLUSH failed for FFE sensor ID = %d\n", sensorid);
				status = QL_STATUS_ERROR;
			}
		}
		break;

		case QL_SAL_IOCTL_ENABLE:
		{
			if (QL_STATUS_OK != QL_FFE_OperationForSensor(sensorid, QL_FFE_SENSOR_OPS_ENABLE, arg))
			{
				QL_TRACE_SENSOR_ERROR("QL_FFE_SENSOR_OPS_ENABLE failed for FFE sensor ID = %d\n", sensorid);
				status = QL_STATUS_ERROR;
			}
		}
		break;


		case QL_SAL_IOCTL_GET_ENABLE:
		{
			if (QL_STATUS_OK != QL_FFE_OperationForSensor(sensorid, QL_FFE_SENSOR_OPS_GET_ENABLE, arg))
			{
				QL_TRACE_SENSOR_ERROR("QL_FFE_SENSOR_OPS_GET_ENABLE failed for FFE sensor ID = %d\n", sensorid);
				status = QL_STATUS_ERROR;
			}
		}
		break;

		case QL_SAL_IOCTL_ALLOC_DATA_BUF:
		{
			// call the platform function

			if (QL_STATUS_OK != QL_SDF_SensorAllocMem(
					sensorid,
					(void **)&(((struct QL_SF_Ioctl_Set_Data_Buf*)arg)->buf),
					((struct QL_SF_Ioctl_Set_Data_Buf*)arg)->size
			)
			)
			{
				QL_TRACE_SENSOR_ERROR("QL_SDF_SensorAllocMem failed to get memory for FFE sensor ID = %d\n", sensorid);
				status = QL_STATUS_ERROR;
			}
		}
		break;

		case QL_SAL_IOCTL_SET_DATA_BUF:
		{
			if (QL_STATUS_OK != QL_FFE_OperationForSensor(sensorid, QL_FFE_SENSOR_OPS_SET_MEM_BUFFER, arg))
			{
				QL_TRACE_SENSOR_ERROR("QL_FFE_SENSOR_OPS_SET_MEM_BUFFER failed for FFE sensor ID = %d\n", sensorid);
				status = QL_STATUS_ERROR;
			}
		}
		break;

		case QL_SAL_IOCTL_GET_DATA_BUF:
		{
			if (QL_STATUS_OK != QL_FFE_OperationForSensor(sensorid, QL_FFE_SENSOR_OPS_GET_MEM_BUFFER, arg))
			{
				QL_TRACE_SENSOR_ERROR("QL_FFE_SENSOR_OPS_GET_MEM_BUFFER failed for FFE sensor ID = %d\n", sensorid);
				status = QL_STATUS_ERROR;
			}
		}
		break;

		case QL_SAL_IOCTL_LIVE_DATA_ENABLE:
			{
				if (QL_STATUS_OK != QL_FFE_OperationForSensor(sensorid, QL_FFE_SENSOR_OPS_LIVE_DATA_ENABLE, arg))
				{
					QL_TRACE_SENSOR_ERROR("QL_FFE_SENSOR_OPS_LIVE_DATA_ENABLE failed for FFE sensor ID = %d\n", sensorid);
					status = QL_STATUS_ERROR;
				}
			}
			break;

			case QL_SAL_IOCTL_GET_LIVE_DATA_ENABLE:
			{
				if (QL_STATUS_OK != QL_FFE_OperationForSensor(sensorid, QL_FFE_SENSOR_OPS_GET_LIVE_DATA_ENABLE, arg))
				{
					QL_TRACE_SENSOR_ERROR("QL_FFE_SENSOR_OPS_LIVE_DATA_ENABLE failed for FFE sensor ID = %d\n", sensorid);
					status = QL_STATUS_ERROR;
				}
			}
			break;

		case QL_SAL_IOCTL_SET_DYNAMIC_RANGE:
		{

			if (QL_STATUS_OK != QL_FFE_OperationForSensor(sensorid, QL_FFE_SENSOR_OPS_SET_DYNAMIC_RANGE, arg))
			{
				QL_TRACE_SENSOR_ERROR("QL_FFE_SENSOR_OPS_SET_DYNAMIC_RANGE failed for FFE sensor ID = %d\n", sensorid);
				status = QL_STATUS_ERROR;
			}
		}
		break;

		case QL_SAL_IOCTL_GET_DYNAMIC_RANGE:
		{

			if (QL_STATUS_OK != QL_FFE_OperationForSensor(sensorid, QL_FFE_SENSOR_OPS_GET_DYNAMIC_RANGE, arg))
			{
				QL_TRACE_SENSOR_ERROR("QL_FFE_SENSOR_OPS_GET_DYNAMIC_RANGE failed for FFE sensor ID = %d\n", sensorid);
				status = QL_STATUS_ERROR;
			}
		}
		break;

		case QL_SAL_IOCTL_SET_ODR:
		{

			if (QL_STATUS_OK != QL_FFE_OperationForSensor(sensorid, QL_FFE_SENSOR_OPS_SET_ODR, arg))
			{
				QL_TRACE_SENSOR_ERROR("QL_FFE_SENSOR_OPS_SET_ODR failed for FFE sensor ID = %d\n", sensorid);
				status = QL_STATUS_ERROR;
			}
		}
		break;

		case QL_SAL_IOCTL_GET_ODR:
		{

			if (QL_STATUS_OK != QL_FFE_OperationForSensor(sensorid, QL_FFE_SENSOR_OPS_GET_ODR, arg))
			{
				QL_TRACE_SENSOR_ERROR("QL_FFE_SENSOR_OPS_GET_ODR failed for FFE sensor ID = %d\n", sensorid);
				status = QL_STATUS_ERROR;
			}
		}
		break;

		default:
		{
			/* ignore unknown IOCTLs */
			status = QL_STATUS_ERROR;
		}
		break;
	}

	return status;
}


static QL_Status Accel_suspend(QL_SDF_SensorDrvInstanceHandle_t instance_handle)
{
	/* no use case for suspend of sensor instance */
	return QL_STATUS_NOT_IMPLEMENTED;
}


static QL_Status Accel_resume(QL_SDF_SensorDrvInstanceHandle_t instance_handle)
{
	/* no use case for resume of sensor instance */
	return QL_STATUS_NOT_IMPLEMENTED;
}



static const char _ascii_accel[] = "accel";
__PLATFORM_INIT__ QL_Status Accel2_init()
{
	unsigned int index;
    accel_drv_priv.drvData.name = _ascii_accel;
	accel_drv_priv.drvData.id = QL_SAL_SENSOR_ID_ACCEL2;
	accel_drv_priv.drvData.type = QL_SDF_SENSOR_TYPE_BASE;		/* physical sensor */
	//accel_drv_priv.drvData.max_sample_rate = 0; 				// KK: this is not needed here at all, can be removed ?
	//accel_drv_priv.drvData.name = "Accel"; 					// KK: this is not used anywhere, can be removed ?

	accel_drv_priv.drvData.ops.open = Accel_Open;
	accel_drv_priv.drvData.ops.close = Accel_Close;
	accel_drv_priv.drvData.ops.read = Accel_read;
	accel_drv_priv.drvData.ops.probe = Accel_probe;
	accel_drv_priv.drvData.ops.ioctl = Accel_ioctl;
	accel_drv_priv.drvData.ops.suspend = Accel_suspend;
	accel_drv_priv.drvData.ops.resume = Accel_resume;

	/* the FFE side ID is common for all instances, it is a sensor/driver level detail */
	accel_drv_priv.ffe_accel_id = QL_FFE_SENSOR_ID_ACCEL2;

	/* init framework level probe status to default, failed state - updated when probe is called */
	accel_drv_priv.drvData.probe_status = SENSOR_PROBE_STATUS_FAILED;

	/* all instances will default to the INVALID state, until probed */
	for ( index = 0 ; index < CONFIG_MAX_FFE_ACCEL_INSTANCE; index++ )
	{
		accel_drv_priv.devInstances[index].state = SENSOR_STATE_INVALID;
	}

	/* Do any platform related pin mux / I2C setting */

	/* register with SDF */
	if (QL_STATUS_OK != QL_SDF_SensorDrvRegister((struct QL_SDF_SensorDrv *) &accel_drv_priv))
	{
		QL_TRACE_SENSOR_ERROR("Failed to register Driver to SDF\n");
		return QL_STATUS_ERROR;
	}

	return QL_STATUS_OK;
}


#endif // FFE_DRIVERS
