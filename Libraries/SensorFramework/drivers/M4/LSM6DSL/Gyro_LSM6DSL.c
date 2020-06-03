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
 *    File   : Gyro_LSM6DSL.c
 *    Purpose: Gyro driver for LSM6DSL sensor
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"
#include <FreeRTOS.h>
#include <eoss3_hal_i2c.h>
#include <task.h>
#include <timers.h>
#include "QL_Trace.h"
#include "QL_SDF.h"
#include "QL_SensorIoctl.h"
#include "ffe_ipc_lib.h"
#include "QL_FFE_SensorConfig.h"
//#include "ql_testTask.h"
#include "Fw_global_config.h"
#include "QL_SensorCommon.h"
#include "QL_SDF_Accel.h"
#include "QL_SDF_Gyro.h"
#include "LSM6DSL.h"
#include "LSM6DSL_Accel.h"
#include "LSM6DSL_Gyro.h"

#ifdef PURE_M4_DRIVERS

static QL_Gyro_Ffe_Drv gyro_drv_priv;

static uint8_t GyroAxisRange;
static unsigned int GyroODR;
static uint8_t IsGyroEnabled;


//Accel probe will call this if Gyro is needed
void set_M4_LSM6DSL_Gyro_Open(int index, int state)
{
  gyro_drv_priv.devInstances[index].state = state;
}

// Structure maps ODR (freq) to control bits.  *MUST* be sorted flow low to high ODR
static struct gyroODR_CTRL {
    int   odr;
    char    ctrl;
} LSM6DSL_gyroODR_CTRL[] = {
    {(int)GYRO_RATE_0HZ,       0x00},
    {(int)GYRO_RATE_12_5HZ,    0x01},
    {GYRO_RATE_26HZ,           0x02},
    {GYRO_RATE_52HZ,           0x03},
    {GYRO_RATE_104HZ,          0x04},
    {GYRO_RATE_208HZ,          0x05},
    {GYRO_RATE_416HZ,          0x06},
    {GYRO_RATE_833HZ,          0x07},
    {GYRO_RATE_1660HZ,         0x08},
    {GYRO_RATE_3330HZ,         0x09},
    {GYRO_RATE_6660HZ,         0x0A}
};

int convert_m4_freq_to_gyro_freq(int freq) {
    int impODR_CTRL;

    for (impODR_CTRL = 0; impODR_CTRL != sizeof(LSM6DSL_gyroODR_CTRL)/sizeof(LSM6DSL_gyroODR_CTRL[0]); impODR_CTRL++) {
        if (LSM6DSL_gyroODR_CTRL[impODR_CTRL].odr >= freq) {
            return (LSM6DSL_gyroODR_CTRL[impODR_CTRL].ctrl);
        }
    }
    return (LSM6DSL_gyroODR_CTRL[impODR_CTRL-1].ctrl);
}


int convert_gyro_freq_to_m4_freq(int freq) {
    // *Tim* Not obvious why we have the function.  Holdover?
    int impODR_CTRL;

    for (impODR_CTRL = 0; impODR_CTRL != sizeof(LSM6DSL_gyroODR_CTRL)/sizeof(LSM6DSL_gyroODR_CTRL[0]); impODR_CTRL++) {
        if (LSM6DSL_gyroODR_CTRL[impODR_CTRL].odr == freq) {
            return (LSM6DSL_gyroODR_CTRL[impODR_CTRL].odr);
        }
    }

	return GYRO_RATE_104HZ;	/*GYRO_RATE_104HZ*/
}


// 0 maps to 250dps, 1 maps to 500, 2 maps to 1000dps, 3 maps to 2000dps
int convert_m4_range_to_gyro_range(int range){
	switch(range){
	case GYRO_RANGE_250:
		return LSM6DSL_GYRO_RANGE_250;
    case GYRO_RANGE_500:
		return LSM6DSL_GYRO_RANGE_500;
    case GYRO_RANGE_1000:
		return LSM6DSL_GYRO_RANGE_1000;
    case GYRO_RANGE_2000:
		return LSM6DSL_GYRO_RANGE_2000;
	default:
		return LSM6DSL_DEFAULT_GYRO_RANGE;
	}
}

int convert_gyro_range_to_m4_range(int range){
	switch(range){
	case LSM6DSL_GYRO_RANGE_250:
		return GYRO_RANGE_250;
	case LSM6DSL_GYRO_RANGE_500:
		return GYRO_RANGE_500;
    case LSM6DSL_GYRO_RANGE_1000:
		return GYRO_RANGE_1000;
    case LSM6DSL_GYRO_RANGE_2000:
		return GYRO_RANGE_2000;
	default:
		return DEFAULT_GYRO_RANGE;
	}
}

float Get_Gyro_Scale_Factor(void){
	//double scale;
	//scale = (double)((double)GyroAxisRange / 32767.0) ;
	switch(GyroAxisRange) {
	case LSM6DSL_GYRO_RANGE_250:
		return 250.0f/32767;
	case LSM6DSL_GYRO_RANGE_500:
		return 500.0f/32767;
	case LSM6DSL_GYRO_RANGE_1000:
		return 1000.0f/32767;
	case LSM6DSL_GYRO_RANGE_2000:
		return 2000.0f/32767;
	}
	return 0.0f;
}

static QL_Status Read_GyroData(uint8_t reg, void *buf,int length){
	Gyro_data Gdata;
	uint8_t GyroData[6];
	uint8_t *tbuf = GyroData;
	Gyro_data *pbuf = (Gyro_data *)buf;

	for(int i=0;i< length; i++){
		if(HAL_I2C_Read(LSM6DSL_SLAVE_ADDR, reg++, tbuf++, 1) != HAL_OK) return QL_STATUS_ERROR;
	}

	Gdata.dataX = 0xFFFF & ( GyroData[1] << 8 | GyroData[0] << 0);
	Gdata.dataY = 0xFFFF & ( GyroData[3] << 8 | GyroData[2] << 0);
	Gdata.dataZ = 0xFFFF & ( GyroData[5] << 8 | GyroData[4] << 0);

	//memcpy(pbuf,&Adata,sizeof(Adata));
	pbuf->dataX = Gdata.dataX;
	pbuf->dataY = Gdata.dataY;
	pbuf->dataZ = Gdata.dataZ;

   	return QL_STATUS_OK;
}


static QL_Status M4_LSM6DSL_Gyro_Open(struct QL_SDF_SensorDrv *s, QL_SDF_SensorDrvInstanceHandle_t *instance_handle) {

	unsigned int index ;

	*instance_handle = (QL_SDF_SensorDrvInstanceHandle_t)0xFFFF;
	QL_ASSERT(s != NULL);
	QL_ASSERT(s == (struct QL_SDF_SensorDrv *) &gyro_drv_priv); // we should get back the same  that we provided during registration

	for ( index = 0 ; index < CONFIG_MAX_FFE_GYRO_INSTANCE; index++ )
	{
//		printf("instance index: %d, state: %d\n",index, gyro_drv_priv.devInstances[index].state);
		/* if we have an instance slot with state == SENSOR_STATE_PROBED, this is ready to use */
		if (SENSOR_STATE_PROBED == gyro_drv_priv.devInstances[index].state)
		{
			/* move this instance's state to SENSOR_STATE_OPENED to mark that this slot is now in use */
			gyro_drv_priv.devInstances[index].state = SENSOR_STATE_OPENED;

			/* pass back the index of the instance in our private table to Framework, as our "Instance Handle" */
			*instance_handle = (QL_SDF_SensorDrvInstanceHandle_t) index;

			/* indicate to framework that open succeeded */
			return QL_STATUS_OK;
		}
	}

	/* if we reach here, we do not have any free slots for further instances, indicate to framework that open failed */
	return QL_STATUS_ERROR;
}

static QL_Status M4_LSM6DSL_Gyro_Close(QL_SDF_SensorDrvInstanceHandle_t instance_handle) {
	/* KK: check validity of handle. This is probably not needed and can be removed */
	QL_ASSERT((unsigned int) instance_handle < CONFIG_MAX_FFE_GYRO_INSTANCE);

	/* move the state of this instance back to ready to use, SENSOR_STATE_PROBED */
	gyro_drv_priv.devInstances[(unsigned int)instance_handle].state = SENSOR_STATE_PROBED;

	/* indicate to framework that the instance was closed successfully */
	return QL_STATUS_OK;
}

static QL_Status M4_LSM6DSL_Gyro_Read(QL_SDF_SensorDrvInstanceHandle_t instance_handle, void *buf, int size) {
	uint8_t reg;
	QL_Status status = QL_STATUS_OK;

	QL_ASSERT( (unsigned int)instance_handle < CONFIG_MAX_FFE_GYRO_INSTANCE);

//	printf("sensorid: %u,cmd: %u\r\n", sensorid, cmd);

	/* check if the instance is in open state first */
	if (SENSOR_STATE_OPENED != gyro_drv_priv.devInstances[(unsigned int) instance_handle].state)
	{
		return QL_STATUS_ERROR;
	}

	if(buf == NULL){
		printf("%s - buffer is NULL\n",__func__);
		return QL_STATUS_ERROR;
	}
	reg = LSM6DSL_OUTX_L_G; //set the register address to Gyro but call the same read function as accel
	status = Read_GyroData( reg, buf, size);
	return status;
}


static QL_Status QL_M4_OperationForGyro(unsigned int sensorID, unsigned int operation, void *arg) {

	uint8_t i2c_read_data = 0x0;
	uint8_t i2c_write_data = 0x0;
	unsigned int argval=0;

	switch(operation) {
	case QL_SAL_IOCTL_SET_CALLBACK:
	{
		gyro_drv_priv.drvData.sensor_callback = (QL_SensorEventCallback_t)arg;
	}
	break;

	case QL_SAL_IOCTL_GET_CALLBACK:
	{
		*(QL_SensorEventCallback_t *)arg = gyro_drv_priv.drvData.sensor_callback;
	}
	break;

	case QL_SAL_IOCTL_SET_DYNAMIC_RANGE:
	{
		GyroAxisRange = *(unsigned int *)arg;
        argval = convert_m4_range_to_gyro_range(GyroAxisRange);

        if(HAL_I2C_Read(LSM6DSL_SLAVE_ADDR, LSM6DSL_CTRL2_G, &i2c_read_data, 1) != HAL_OK) return QL_STATUS_ERROR;
		//i2c_write_data = (i2c_read_data & 0x0C) | (argval<<2);
        i2c_write_data = (i2c_read_data & 0xF3) | (argval<<2);
		if(HAL_I2C_Write(LSM6DSL_SLAVE_ADDR, LSM6DSL_CTRL2_G, &i2c_write_data, 1) != HAL_OK) return QL_STATUS_ERROR;
		break;
	}

	case QL_SAL_IOCTL_GET_DYNAMIC_RANGE:
	{
		if(HAL_I2C_Read(LSM6DSL_SLAVE_ADDR, LSM6DSL_CTRL2_G, &i2c_read_data, 1) != HAL_OK) return QL_STATUS_ERROR;
		*(unsigned int *)arg = convert_gyro_range_to_m4_range((unsigned int) ((i2c_read_data>>2) & 0x03));
		break;
	}

	case QL_SAL_IOCTL_SET_ODR:
	{
		GyroODR = *(unsigned int *)arg;
		argval = convert_m4_freq_to_gyro_freq(GyroODR);

		if(HAL_I2C_Read(LSM6DSL_SLAVE_ADDR, LSM6DSL_CTRL2_G, &i2c_read_data, 1) != HAL_OK) return QL_STATUS_ERROR;
		i2c_write_data =  (i2c_read_data & 0x0F) | (argval<<4);
		if(HAL_I2C_Write(LSM6DSL_SLAVE_ADDR, LSM6DSL_CTRL2_G, &i2c_write_data, 1) != HAL_OK) return QL_STATUS_ERROR;

        break;
	}

	case QL_SAL_IOCTL_GET_ODR:
	{
		if(HAL_I2C_Read(LSM6DSL_SLAVE_ADDR, LSM6DSL_CTRL2_G, &i2c_read_data, 1) != HAL_OK) return QL_STATUS_ERROR;
		*(unsigned int *)arg = convert_gyro_freq_to_m4_freq((unsigned int) ((i2c_read_data>>4) & 0x0F));
		break;
	}

	case QL_SAL_IOCTL_SET_BATCH:
	{
		// not implemented
		break;
	}

	case QL_SAL_IOCTL_GET_BATCH:
	{
		// not implemented
		break;
	}

	case QL_SAL_IOCTL_BATCH_FLUSH:
	{
		// not implemented
		break;
	}
	case QL_SAL_IOCTL_ALLOC_DATA_BUF:
	{
		// not implemented
		break;
	}

	case QL_SAL_IOCTL_SET_DATA_BUF:
	{
		// not implemented
		break;
	}
	case QL_SAL_IOCTL_GET_DATA_BUF:
	{
		// not implemented
		break;
	}

	case QL_SAL_IOCTL_LIVE_DATA_ENABLE:
	{
		// not implemented
	}
	break;

	case QL_SAL_IOCTL_GET_LIVE_DATA_ENABLE:
	{
		// not implemented
	}
	break;

	case QL_SAL_IOCTL_ENABLE:
	{
		//Start the Gyro.
		IsGyroEnabled = (uint8_t) (*(unsigned int*)arg);
		break;
	}

	case QL_SAL_IOCTL_GET_ENABLE:
	{
		*(unsigned int *)arg = (unsigned int)IsGyroEnabled;
		break;
	}

	default:
		QL_TRACE_SENSOR_ERROR(" Operation - %d , Failed. Sensor ID %d\n",operation,sensorID);
		break;
	}

	return QL_STATUS_OK;
}

static QL_Status M4_LSM6DSL_Gyro_Ioctl(QL_SDF_SensorDrvInstanceHandle_t instance_handle, unsigned int cmd, void *arg) {
	unsigned int sensorid;

	sensorid = gyro_drv_priv.ffe_gyro_id;
	QL_ASSERT( (unsigned int)instance_handle < CONFIG_MAX_FFE_GYRO_INSTANCE);

//	printf("sensorid: %u,cmd: %u\r\n", sensorid, cmd);

	/* check if the instance is in open state first */
	if (SENSOR_STATE_OPENED != gyro_drv_priv.devInstances[(unsigned int) instance_handle].state)
	{
		return QL_STATUS_ERROR;
	}

	/* before invoking the command implementation, ensure that the arguments are valid */
	/* the arg check function *must* be specific to the end sensor being used and has to be checked ! */


	return QL_M4_OperationForGyro(sensorid, cmd, arg);
}

QL_Status M4_LSM6DSL_Gyro_Init(void)
{
	/*
	 * LSM6DSL  Gyro Init sequence
	 *
	 * (1) Make sure M4_LSM6DSL_Accel_Init() is called before calling this
     * (2) Set default Rate(ODR), range and power mode(LP/HP)
	 * (3) Wait atleast 5ms, Check if any error (ERROR STATUS REG) and power mode (PMU STATUS REG) is as set
	 *
	 */

	uint8_t i2c_write_data = 0x0;

	/* init sequence: set ODR, range and power mode */
    GyroAxisRange = LSM6DSL_DEFAULT_GYRO_RANGE;
	i2c_write_data = (LSM6DSL_DEFAULT_ODR_GYRO_RATE<<4) | (LSM6DSL_DEFAULT_GYRO_RANGE<<2);
	if(HAL_I2C_Write(LSM6DSL_SLAVE_ADDR, LSM6DSL_CTRL2_G, &i2c_write_data, 1) != HAL_OK) return QL_STATUS_ERROR;

	printf("LSM6DSL Gyro Write ODR ... OK\n");

	/* wait for atleast 5 ms */
	volatile int z = 5000;
	while(z--);
	//vTaskDelay(pdMS_TO_TICKS(5));

	printf("LSM6DSL Gyro Init Sequence OK, no errors\n");

	/* at this point probe() is ok */
	printf("LSM6DSL Gyro Probe on M4 OK\n\n");

	return QL_STATUS_OK;
}

QL_Status M4_LSM6DSL_Gyro_Probe(struct QL_SDF_SensorDrv *s)
{
	unsigned int index;

	QL_ASSERT(s != NULL);
	QL_ASSERT(s == (struct QL_SDF_SensorDrv *)&gyro_drv_priv); // we should get back the same instance that we provided during registration

	if(QL_STATUS_OK != M4_LSM6DSL_Gyro_Init())
	{
		QL_TRACE_SENSOR_ERROR("M4_LSM6DSL_Gyro_probe() failed for SAL sensor with ID = %d\n",gyro_drv_priv.drvData.id);

		/* probe failed, update the state of each of the instances to not ready, SENSOR_STATE_INVALID */
		for ( index = 0 ; index < CONFIG_MAX_FFE_GYRO_INSTANCE; index++ )
		{
			gyro_drv_priv.devInstances[index].state = SENSOR_STATE_INVALID;
		}

		return QL_STATUS_ERROR;
	}
	QL_TRACE_SENSOR_DEBUG("LSM6DSL_Gyro_Probe_In_M4() OK for SAL sensor with ID = %d\n",gyro_drv_priv.drvData.id);

	/* probe is ok, update the state of each of the instances to ready to use, SENSOR_STATE_PROBED */
	for ( index = 0 ; index < CONFIG_MAX_FFE_GYRO_INSTANCE; index++ )
	{
		gyro_drv_priv.devInstances[index].state = SENSOR_STATE_PROBED;
	}

	return QL_STATUS_OK;
}

static uint8_t i2c_reg_ctrl1_gyro_previous = (LSM6DSL_DEFAULT_ODR_GYRO_RATE<<4);

static QL_Status M4_LSM6DSL_Gyro_Suspend(QL_SDF_SensorDrvInstanceHandle_t instance_handle) {

	uint8_t cmd;
    // Get previousconfig
    if(HAL_I2C_Read((UINT8_t)LSM6DSL_SLAVE_ADDR,(UINT8_t)LSM6DSL_CTRL2_G,&i2c_reg_ctrl1_gyro_previous,1) != HAL_OK) {
		return QL_STATUS_ERROR;
	}
	cmd = i2c_reg_ctrl1_gyro_previous & 0x0f;  // Set ODR to zero
	// Write the command register for Gyro Low power mode
	if(HAL_I2C_Write((UINT8_t)LSM6DSL_SLAVE_ADDR,(UINT8_t)LSM6DSL_CTRL2_G,(UINT8_t*)cmd,1) != HAL_OK) {
		return QL_STATUS_ERROR;
	}
	// Wait for 4MS
	vTaskDelay(pdMS_TO_TICKS(4));

        printf("Gyro_Suspend\n");//*Tim*
	return QL_STATUS_OK;
}


static QL_Status M4_LSM6DSL_Gyro_Resume(QL_SDF_SensorDrvInstanceHandle_t instance_handle) {

	// Write the command register for Gyro power mode
	if(HAL_I2C_Write(LSM6DSL_SLAVE_ADDR, LSM6DSL_CTRL2_G, &i2c_reg_ctrl1_gyro_previous, 1) != HAL_OK) return QL_STATUS_ERROR;
	// Wait for 4MS
	vTaskDelay(pdMS_TO_TICKS(4));

	return QL_STATUS_OK;;
}

__PLATFORM_INIT__ QL_Status Gyro_init(void)
{

	unsigned int index;

	gyro_drv_priv.drvData.id = QL_SAL_SENSOR_ID_GYRO;
	gyro_drv_priv.drvData.type = QL_SDF_SENSOR_TYPE_BASE;		/* physical sensor */

	gyro_drv_priv.drvData.ops.open = M4_LSM6DSL_Gyro_Open;
	gyro_drv_priv.drvData.ops.close = M4_LSM6DSL_Gyro_Close;
	gyro_drv_priv.drvData.ops.read = M4_LSM6DSL_Gyro_Read;
	gyro_drv_priv.drvData.ops.probe = M4_LSM6DSL_Gyro_Probe;
	gyro_drv_priv.drvData.ops.ioctl = M4_LSM6DSL_Gyro_Ioctl;
	gyro_drv_priv.drvData.ops.suspend = M4_LSM6DSL_Gyro_Suspend;
	gyro_drv_priv.drvData.ops.resume = M4_LSM6DSL_Gyro_Resume;

	/* the FFE side ID is common for all instances, it is a sensor/driver level detail */
	//gyro_drv_priv.ffe_gyro_id = QL_FFE_SENSOR_ID_GYRO;

	/* init framework level probe status to default, failed state - updated when probe is called */
	gyro_drv_priv.drvData.probe_status = SENSOR_PROBE_STATUS_FAILED;

	/* all instances will default to the INVALID state, until probed */
	for ( index = 0 ; index < CONFIG_MAX_FFE_GYRO_INSTANCE; index++ )
	{
		gyro_drv_priv.devInstances[index].state = SENSOR_STATE_INVALID;
	}


	/* register with SDF */
	if (QL_STATUS_OK != QL_SDF_SensorDrvRegister((struct QL_SDF_SensorDrv *) &gyro_drv_priv))
	{
		QL_TRACE_SENSOR_ERROR("Failed to register Driver to SDF\n");
		return QL_STATUS_ERROR;
	}


	return QL_STATUS_OK;
}


#endif // PURE_M4_DRIVERS
