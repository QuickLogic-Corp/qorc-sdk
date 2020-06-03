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
 *    File   : QL_SDF.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __QL_SDF_H__
#define __QL_SDF_H__
#include "QL_Trace.h"
#include "QL_SensorCommon.h"
#include "QL_SensorIoctl.h"

/*
 * Header File declaring the interfaces for the Sensor Drivers to use.
 *
 * Each Sensor Driver should:
 * 1. Define functions required by the QL_SDF_SensorDrvOps structure
 * 2. Create the overall QL_SDF_SensorDrv structure, and fill in: id, type, name, ops (at least)
 * 3. Register itself to the Sensor Framework using QL_SDF_SensorDrvRegister()
 * 4. (Optional) Unregister itself from Sensor Framework using QL_SDF_SensorDrvUnregister() when not required
 *
 *KK: Description of each function in the QL_SDF_SensorDrvOps structure, that the client *must* implement TODO.
 */


typedef enum {QL_SDF_SENSOR_TYPE_BASE, QL_SDF_SENSOR_TYPE_COMPOSITE} QL_SDF_SensorDevType_t;

/* forward declaration for Sensor Driver struct */
struct QL_SDF_SensorDrv;

/* opaque handle for Sensor Framework to interact with Sensor Driver Instance */
struct QL_SDF_SensorDrvInstanceHandle;
typedef struct QL_SDF_SensorDrvInstanceHandle *QL_SDF_SensorDrvInstanceHandle_t; 

enum
{
	SENSOR_PROBE_STATUS_FAILED,
	SENSOR_PROBE_STATUS_OK
};

struct QL_SDF_SensorDrvOps
{
	/*
	 * struct QL_SDF_SensorDrv *s - input param
	 * QL_SDF_SensorDrvInstanceHandle_t *instance_handle - output param given by driver to identify this instance
	 */
	QL_Status (*open)		(struct QL_SDF_SensorDrv *s, QL_SDF_SensorDrvInstanceHandle_t *instance_handle);
	QL_Status (*close)		(QL_SDF_SensorDrvInstanceHandle_t instance_handle);
	QL_Status (*read)		(QL_SDF_SensorDrvInstanceHandle_t instance_handle, void *buf, int size);
	QL_Status (*probe)		(struct QL_SDF_SensorDrv *s);
	QL_Status (*ioctl)		(QL_SDF_SensorDrvInstanceHandle_t instance_handle, unsigned int cmd, void *arg);
	QL_Status (*suspend)	(QL_SDF_SensorDrvInstanceHandle_t instance_handle);
	QL_Status (*resume)		(QL_SDF_SensorDrvInstanceHandle_t instance_handle);
};


/*
 * Sensor Driver Framework will have the control over callbacks, moving this from FFE IPC LIB, because this
 * is now no longer the exclusive domain of FFE Drivers, we have M4 Drivers as well, which will not use any part
 * of FFE IPC LIB (or should not)
 *
 * IOCTLs are already present as part of the driver to register callbacks.
 * Any Task (can be FFE Task or new ones) that want to signal events to clients who have registered callbacks through
 * the sensor driver, will now go through the Sensor Framework.
 * They will invoke the Sensor Framework API (getSensorCallback()) and pass on the Event Information.
 */
struct QL_SDF_SensorDrv
{
	unsigned int id;							/* SAL side ID of the sensor, that clients should use */
	const char* name;						            /* sensor driver identifier, current unused except as decor */
	unsigned int max_sample_rate; 				/* KK: can this be removed, does not seem to be needed here ? */
	QL_SDF_SensorDevType_t type;				/* KK: Base(Physical) or Composite(Virtual, Derived) */
	struct QL_SDF_SensorDrvOps ops;				/* the ops structure */
	unsigned int probe_status;					/* store the probe status of the sensor at framework level */
	void *sensor_infra_info;					/* KK: what is the purpose of this, does not seem to be used anywhere ? */
	QL_SensorEventCallback_t sensor_callback;	/* KK: added callback to the sensor driver framework -> can be expanded to multiple callbacks*/
};

/* Sensor Framework API for the Sensor Drivers to register/unregister themselves, and to obtain memory for data buffers */
QL_Status QL_SDF_SensorDrvRegister(struct QL_SDF_SensorDrv *s);
QL_Status QL_SDF_SensorDrvUnregister(struct QL_SDF_SensorDrv *s);
QL_Status QL_SDF_SensorAllocMem(unsigned int sensorid, void **membuf, int size);

#endif /*#ifndef __QL_SDF_H__ */
