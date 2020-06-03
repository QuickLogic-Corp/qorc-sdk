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
 *    File   : QL_SensorFramework.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"
#include <stdio.h>
#include "dbg_uart.h"
#include "QL_Trace.h"
#include "QL_SAL.h"
#include "QL_SDF.h"
#include "QL_SensorIoctl.h"

#include "FreeRTOS.h"
#include "semphr.h"

#define MUTEX_DELAY	portMAX_DELAY

#define MAX_SENSOR_INSTANCES	10
#define MAX_SENSORS	8



/*
 *
 * There are 2 concepts that the Sensor Framework deals with:
 * Driver -> this is a construct which is associated with one sensor, and identified with the "Sensor ID"
 * Instance of Driver -> this is a construct which is associated with one client and one sensor which allows the client to interact with the sensor
 * 		The Driver for the sensor handles all the instances (handles from various clients) which allows multiple clients
 * 		to interact with the same sensor. Hence, it makes logical sense for the Driver to internally manage the instances
 * 		and the Sensor Framework will request the Driver for an instance when the client tries to obtain a handle, as well
 * 		as the client will provide the obtained handle when it wants to interact with the sensor further.
 * 		All of the interaction with the "instance" of a client is handled by the Sensor Driver transparently.
 *
 * The Sensor Framework will maintain a list of Drivers registered with it via the QL_SDF_SensorDrvRegister() call.
 * We will store the pointers to the driver structures in a table internal to the framework, and each driver is
 * identified with the Sensor ID. (static struct QL_SDF_SensorDrv *SensorDriver[MAX_SENSORS];)
 *
 * Implementation Note:
 * To speed up the Driver Lookup from the table, we maintain the table as a sparse array, where the index of the array
 * corresponds to the sensor ID itself. This requires the Sensor ID values to be limited so that the size of the array
 * is limited, which is currently the case anyway. Current design places maximum Sensor ID value to be 16. (MAX_SENSORS)
 *
 * When a client attempts to obtain a handle to a new instance of a registered driver, via the QL_SAL_SensorOpen() call,
 * the Sensor Framework will ask the Driver to create a new instance, and return an "Instance Handle" to it.
 * This "Instance Handle" is combined with the the "Sensor ID" to create a system wide unique "Client Handle" which can
 * identify the Driver as well as the particular instance of the driver.
 * If the Driver cannot allow an instance (if the internal limit is reached) then we return an error to the client to
 * indicate that a new instance of the driver could not be provided.
 *
 * As the "Client Handle" created contains both the Driver (Sensor) ID as well as the Driver's "Instance Handle" we can
 * be guaranteed that this will be unique. Further, when the client calls the other functions of the framework to interact
 * with the sensor : QL_SAL_SensorClose() QL_SAL_SensorIoctl() ... the client provides the "Client Handle" to the framework.
 * The Sensor Framework splits this into the Driver ID and the Driver's "Instance Handle".
 * Driver lookup happens using the Driver ID from the internal table, and the framework passes in the "Instance Handle" so that
 * the driver can lookup and manipulate the internal data structures accordingly.
 * For the clients, the unique client handle is an opaque type : QL_SAL_SensorHandle_t, so that the client is not aware
 * of the internal implementation of the Sensor Framework.
 *
 * Guidelines for Driver Implementation:
 * 1. The Driver's open() call should provide an "Instance Handle" of 2 bytes to the Sensor Framework.
 * 2. The "Instance Handle" returned by the Driver's open() call should be unique internally to the Driver.
 * 3. The Driver should implement logic, which will let it determine the current state of the instances and determine if
 * 		there are free slots available to provide to clients.
 *
 *
 */


/* internal table which maintains the list of registered Sensor Drivers */
static struct QL_SDF_SensorDrv *SensorDriver[MAX_SENSORS];


/* Sensor Framework API to register a Driver for a Sensor */
QL_Status QL_SDF_SensorDrvRegister(struct QL_SDF_SensorDrv *s)
{
	QL_ASSERT(s != NULL);
	QL_ASSERT(s->id != QL_SAL_SENSOR_ID_INVALID);
	//	QL_ASSERT(s->name != NULL); // KK: not used by anyone.

	/* ID of the sensor whose driver is being registered is the index in the Driver Table, check if this is free */
	/* Ensure that the ID of the driver does not exceed the max allowed (table size == MAX_SENSORS) */

	if(s->id >= MAX_SENSORS)
	{
		/* ID out of range */
		QL_TRACE_SENSOR_DEBUG("Sensor Driver with ID %d registration FAILED (ID out of range)\n", s->id);
		return QL_STATUS_ERROR;
	}

	if(SensorDriver[s->id] != NULL)
	{
		/* Driver already registered with this ID */
		QL_TRACE_SENSOR_DEBUG("Sensor Driver with ID %d registration FAILED (Driver Already registered with this ID)\n", s->id);
		return QL_STATUS_ERROR;
	}

	/* all ok, store the driver into the table */
	SensorDriver[s->id] = s;

	QL_TRACE_SENSOR_DEBUG("Sensor Driver with ID %d registered successfully\n", s->id);

	return QL_STATUS_OK;
}


/* Sensor Framework API to un-register a Driver for a Sensor */
QL_Status QL_SDF_SensorDrvUnregister(struct QL_SDF_SensorDrv *s)
{
	if(s->id >= MAX_SENSORS)
	{
		/* ID out of range */
		QL_TRACE_SENSOR_DEBUG("Sensor Driver with ID %d deregistration FAILED (ID out of range)\n", s->id);
		return QL_STATUS_ERROR;
	}

	if(SensorDriver[s->id] == NULL)
	{
		/* Driver already registered with this ID */
		QL_TRACE_SENSOR_DEBUG("Sensor Driver with ID %d deregistration FAILED (No Driver registered with this ID)\n", s->id);
		return QL_STATUS_ERROR;
	}

	/* all ok, clear the driver from the table */
	SensorDriver[s->id] = NULL;

	QL_TRACE_SENSOR_DEBUG("Sensor Driver with ID %d deregistered successfully\n", s->id);

	return QL_STATUS_OK;
}


/* Sensor Framework API to allocate data memory for a particular Sensor Driver */
//TBD: move to a platform.c file 
//TBD: should come from a section automatically 
#include <ql_SensorHub_MemoryMap.h>
#define CFG_AON_SENSOR_MEM_START  BATCH_BUFFER_INFO_SECTION_ADDR//0x2007C000 //(0x2007BFFF +1)
#define CFG_AON_SENSOR_MEM_END	  0x20080000

static unsigned int curSensorMemPtr = CFG_AON_SENSOR_MEM_START;
void * QL_Plat_SensorAllocMem(int size)
{
	void * addr = 0;

	//TBD: lock it here
	if ((curSensorMemPtr + size) < CFG_AON_SENSOR_MEM_END )
	{
		// First time
		addr = ( void *)curSensorMemPtr;
		curSensorMemPtr += size;

	}
	return addr;

}

QL_Status QL_SDF_SensorAllocMem(unsigned int sensorid, void **membuf, int size)
{
	QL_Status status = QL_STATUS_OK;

	// call the platform routine here
	*membuf = QL_Plat_SensorAllocMem(size);

	if ( NULL == *membuf )
	{
		QL_TRACE_SENSOR_ERROR("memory could not be allocated for sensor ID %d with size %d bytes\n", sensorid, size);
		status= QL_STATUS_ERROR;
	}
	else
	{
		QL_TRACE_SENSOR_DEBUG("memory allocated ok for %d bytes at 0x%x\n",size,(unsigned int)*membuf);
		//TBD
		//keep the inventory in sensor driver list
	}

	return status;
}


/* helper to create "Client Handle" from "Sensor ID" and "Instance Handle" */
#define getClientHandle(sensorID, drvInstanceHandle)	((QL_SAL_SensorHandle_t) ( ((sensorID & 0xFFFF) << 16) | (drvInstanceHandle & 0xFFFF) ))

/* helpers to get "Driver ID" and "Instance Handle" from "Client Handle" */
#define getSensorID(clientHandle)						((int) ( ( ((unsigned int)clientHandle) >> 16 ) & 0xFFFF ))
#define getDrvInstanceHandle(clientHandle)				((QL_SDF_SensorDrvInstanceHandle_t) ( ((unsigned int)clientHandle) & 0xFFFF ))



/* Sensor Framework API to obtain Handle to a new instance of a Sensor Driver from a client */
QL_Status QL_SAL_SensorOpen(QL_SAL_SensorHandle_t *handle, unsigned int id)
{
	int drvInstanceHandle;

	QL_ASSERT(handle);
	QL_ASSERT(id != QL_SAL_SENSOR_ID_INVALID);

	/*
	 * open() of the driver populates the handle passed in from here
	 * this handle is required to be passed in to the driver for all further communication
	 * so that the driver identifies the correct instance being manipulated.
	 * Agreement with the Driver: the driver passes back a "Instance Handle" of 2 Bytes, which it can use to identify the correct instance further.
	 * Sensor Framework creates a unique "Client Handle" using a combination of the "Instance Handle" provided by the driver, and the id of the driver
	 * and passes this "Client Handle" back to the client.
	 * the client should pass in this "Client Handle" for further comm with its instance, using which the Sensor Framework can identify
	 * the driver (using the ID part) and the driver can identify the instance (using the driver's "Instance Handle").
	 */

	if(id >= MAX_SENSORS)
	{
		/* ID out of range */
		QL_TRACE_SENSOR_DEBUG("Cannot Open Driver for ID: %d (ID out of range)\n", id);
		return QL_STATUS_ERROR;
	}

	if(SensorDriver[id] == NULL)
	{
		/* No Driver registered with this ID */
		QL_TRACE_SENSOR_DEBUG("Cannot Open Driver for ID: %d (No Sensor Driver registered)\n", id);
		return QL_STATUS_ERROR;
	}

	if(SensorDriver[id]->probe_status != SENSOR_PROBE_STATUS_OK)
	{
		QL_TRACE_SENSOR_DEBUG("Cannot call open() for ID: %d (Sensor Probe has failed)\n", id);
		return QL_STATUS_ERROR;
	}

	if( SensorDriver[id]->ops.open(SensorDriver[id], (QL_SDF_SensorDrvInstanceHandle_t *)&drvInstanceHandle) != QL_STATUS_OK /*|| drvInstanceHandle < 0*/)
	{
		/* Driver could not provide an instance to the Client */
		QL_TRACE_SENSOR_DEBUG("Cannot Obtain Instance from Driver for ID: %d (Driver rejected request)\n", id);
		return QL_STATUS_ERROR;
	}


	/* create the unique handle for client : LSB 2B = driver instance handle, MSB 2B = sensor id */
	*handle = getClientHandle(id, drvInstanceHandle);

	return QL_STATUS_OK;
}


/* Sensor Framework API to release previously obtained instance of a Sensor Driver from a client */
QL_Status QL_SAL_SensorClose(QL_SAL_SensorHandle_t handle)
{
	QL_Status ret = QL_STATUS_OK;

	/* not checking for Driver ID validity from handle */

	ret = SensorDriver[getSensorID(handle)]->ops.close(getDrvInstanceHandle(handle));

	return ret;
}


/* Sensor Framework API to call IOCTL on an instance of a Sensor Driver from a client */
QL_Status QL_SAL_SensorIoctl(QL_SAL_SensorHandle_t handle, unsigned int cmd, void *arg)
{
	QL_Status ret = QL_STATUS_ERROR;

	QL_ASSERT(cmd);
 
    if( (cmd == QL_SAL_IOCTL_SET_ODR) && (DBG_flags & DBG_FLAG_sensor_rate) ){
        dbg_str("snsr-odr-");
        dbg_str_int(SensorDriver[getSensorID(handle)]->name, *((int *)(arg)) );
    }        

	if(SensorDriver[getSensorID(handle)]->probe_status == SENSOR_PROBE_STATUS_OK)
	{
		/* not checking for Driver ID validity from handle */

		ret = SensorDriver[getSensorID(handle)]->ops.ioctl(getDrvInstanceHandle(handle), cmd, arg);
	}

	return ret;
}


/* Sensor Framework API to read data from an instance of a Sensor Driver from a client */
QL_Status QL_SAL_SensorRead(QL_SAL_SensorHandle_t handle, void *buf, int size)
{
	QL_Status ret = QL_STATUS_ERROR;

	if(SensorDriver[getSensorID(handle)]->probe_status == SENSOR_PROBE_STATUS_OK)
	{
		/* not checking for Driver ID validity from handle */

		ret = SensorDriver[getSensorID(handle)]->ops.read(getDrvInstanceHandle(handle), buf, size);
	}

	return ret;
}



/* Sensor Framework API Init : This *must* be called after all drivers register themselves to the framework */
/* Assumption is that this is called only from *one* place in the main logic, no locking required as of now */
QL_Status QL_SensorFramework_Init(void)
{
	int i;

	QL_TRACE_SENSOR_DEBUG("+\n");

	/* KK: removing locking, as this function will be called only from one place in the main logic */
	//	sensorInstanceLock = xSemaphoreCreateMutex();
	//	if (sensorInstanceLock == NULL)
	//	{
	//		QL_TRACE_SENSOR_ERROR("Failed creating mutex\n");
	//		return QL_STATUS_ERROR;
	//	}

	for (i = 0; i < MAX_SENSORS ; i++)
	{
		/* check if the entry in the table is allotted to a driver */
		if(SensorDriver[i] != NULL)
		{
			/* check if probe function exists for the driver */
			if (!SensorDriver[i]->ops.probe)
			{
				QL_TRACE_SENSOR_ERROR("Probe Function not available for sensor driver with id %d\n", SensorDriver[i]->id);
				/* REVISIT : purposefully not returning from here to proceed with other drivers */
				/* KK: if probe() function is not defined for this sensor, then this would mean it does not require one */
				SensorDriver[i]->probe_status = SENSOR_PROBE_STATUS_OK;
			}
			else
			{
				/*
				 * TODO : maintain state to prevent open to driver for which probe fails
				 */
				if(QL_STATUS_OK != SensorDriver[i]->ops.probe(SensorDriver[i]))
				{
					QL_TRACE_SENSOR_ERROR("Probe Failed for Sensor[id = %d]\n",	SensorDriver[i]->id);
					SensorDriver[i]->probe_status = SENSOR_PROBE_STATUS_FAILED;
				}
				else
				{
					QL_TRACE_SENSOR_DEBUG("Probe Success for Sensor[id = %d]\n",SensorDriver[i]->id);
					SensorDriver[i]->probe_status = SENSOR_PROBE_STATUS_OK;
				}
			}
		}
	}

	QL_TRACE_SENSOR_DEBUG("-\n");

	/* note: we always return OK, even if probe of drivers failed. FIXME */
	return QL_STATUS_OK;
}
