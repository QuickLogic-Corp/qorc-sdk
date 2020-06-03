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
 *    File   : Accel_LSM6DSL.c
 *    Purpose: Accel driver for LSM6DSL sensor
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
#include "QL_SensorCommon.h"
#include "QL_SDF_Accel.h"
#include "LSM6DSL.h"
#include "LSM6DSL_Accel.h"

#ifdef PURE_M4_DRIVERS


I2C_Config xI2CConfig;
static TimerHandle_t AccelBtimer=NULL;

static QL_Accel_Ffe_Drv accel_drv_priv;

//static uint8_t AccelAxisDir;
static uint8_t AccelAxisRange;
static unsigned int AccelODR;
static uint8_t IsAccelEnabled;
static unsigned int batchPeriod;
static unsigned int batchcount,samplescnt,sensorid;


static Accel_data	Buf_Bdata[100]; //for Batch Data

static Accel_data *Bufp = &Buf_Bdata[0];

static struct QL_SF_Ioctl_Req_Events AccelCBinfo[QL_FFE_MAX_SENSORS];
static struct QL_SensorEventInfo Batchevent;


// Structure maps ODR (freq) to control bits.  *MUST* be sorted flow low to high ODR
static struct mpODR_CTRL {
    int   odr;
    char    ctrl;
} LSM6DSL_mpODR_CTRL[] = {
    {(int)ACCEL_RATE_1_6HZ,     0x0B},
    {(int)ACCEL_RATE_12_5HZ,    0x01},
    {ACCEL_RATE_26HZ,           0x02},
    {ACCEL_RATE_52HZ,           0x03},
    {ACCEL_RATE_104HZ,          0x04},
    {ACCEL_RATE_208HZ,          0x05},
    {ACCEL_RATE_416HZ,          0x06},
    {ACCEL_RATE_833HZ,          0x07},
    {ACCEL_RATE_1660HZ,         0x08},
    {ACCEL_RATE_3330HZ,         0x09},
    {ACCEL_RATE_6660HZ,         0x0A}
};

#define LSM6DSL_DEV_I2C_BUS			0x0

int convert_m4_freq_to_accel_freq(int freq) {
    int impODR_CTRL;
    
    for (impODR_CTRL = 0; impODR_CTRL != sizeof(LSM6DSL_mpODR_CTRL)/sizeof(LSM6DSL_mpODR_CTRL[0]); impODR_CTRL++) {
        if (LSM6DSL_mpODR_CTRL[impODR_CTRL].odr >= freq) {
            return (LSM6DSL_mpODR_CTRL[impODR_CTRL].ctrl);
        }
    }
    return (LSM6DSL_mpODR_CTRL[impODR_CTRL-1].ctrl);
}


int convert_accel_freq_to_m4_freq(int freq) {
    // *Tim* Not obvious why we have the function.  Holdover?
    int impODR_CTRL;
    
    for (impODR_CTRL = 0; impODR_CTRL != sizeof(LSM6DSL_mpODR_CTRL)/sizeof(LSM6DSL_mpODR_CTRL[0]); impODR_CTRL++) {
        if (LSM6DSL_mpODR_CTRL[impODR_CTRL].odr == freq) {
            return (LSM6DSL_mpODR_CTRL[impODR_CTRL].odr);
        }
    }
    
	return ACCEL_RATE_104HZ;	/*ACCEL_RATE_104HZ*/
}


// 0 maps to +-2G, 1 maps to +-4G, 2 maps to +-8G, 3 maps to +-16G
int convert_m4_range_to_accel_range(int range){
	switch(range){
	case ACCEL_RANGE_2G:
		return LSM6DSL_ACCEL_RANGE_2G;
    case ACCEL_RANGE_4G:
		return LSM6DSL_ACCEL_RANGE_4G;
    case ACCEL_RANGE_8G:
		return LSM6DSL_ACCEL_RANGE_8G;
    case ACCEL_RANGE_16G:
		return LSM6DSL_ACCEL_RANGE_16G;
	default:
		return LSM6DSL_DEFAULT_ACCEL_RANGE;
	}
}

int convert_accel_range_to_m4_range(int range){
	switch(range){
	case LSM6DSL_ACCEL_RANGE_2G:
		return ACCEL_RANGE_2G;
	case LSM6DSL_ACCEL_RANGE_4G:
		return ACCEL_RANGE_4G;
    case LSM6DSL_ACCEL_RANGE_8G:
		return ACCEL_RANGE_8G;
    case LSM6DSL_ACCEL_RANGE_16G:
		return ACCEL_RANGE_16G;
	default:
		return DEFAULT_ACCEL_RANGE;
	}
}

double Get_Accel_Scale_Factor(void){
	//double scale;
	//scale = (double)((double)AccelAxisRange / 32767.0) * (9.8);
	switch(AccelAxisRange) {
	case LSM6DSL_ACCEL_RANGE_2G:
		return 0.000598145;
	case LSM6DSL_ACCEL_RANGE_4G:
		return 0.001196289;
	case LSM6DSL_ACCEL_RANGE_8G:
		return 0.002392578;
	case LSM6DSL_ACCEL_RANGE_16G:
		return 0.004785156;
	}
	return 0;
}

float Get_Accel_Scale_Factor_1000mG(){
	float scale;
	scale = (1000.0 / 32767) * ((float)AccelAxisRange);
	return scale;
}

static QL_Status Read_AccelData(uint8_t reg, void *buf,int length){
	Accel_data Adata;
	uint8_t AccelData[6];
	uint8_t *tbuf = AccelData;
	Accel_data *pbuf = (Accel_data *)buf;

	for(int i=0;i< length; i++){
		if(HAL_I2C_Read(LSM6DSL_SLAVE_ADDR, reg++, tbuf++, 1) != HAL_OK) return QL_STATUS_ERROR;
	}

	Adata.dataX = 0xFFFF & ( AccelData[1] << 8 | AccelData[0] << 0);
	Adata.dataY = 0xFFFF & ( AccelData[3] << 8 | AccelData[2] << 0);
	Adata.dataZ = 0xFFFF & ( AccelData[5] << 8 | AccelData[4] << 0);

	//memcpy(pbuf,&Adata,sizeof(Adata));
	pbuf->dataX = Adata.dataX;
	pbuf->dataY = Adata.dataY;
	pbuf->dataZ = Adata.dataZ;
    
   	return QL_STATUS_OK;
}

void vAccelBtimerCB( TimerHandle_t xATimer ) {

	Read_AccelData(LSM6DSL_OUTX_L_XL,Bufp,6);
	//printf(" vAccelBtimerCB : Accel data read X=%hd,Y=%hd,Z=%hd\n",Bufp->dataX, Bufp->dataY, Bufp->dataZ);
	Bufp++;
	samplescnt++;
	batchcount--;

	if(batchcount == 0) {
		if(xTimerChangePeriod(AccelBtimer,pdMS_TO_TICKS(1000),pdMS_TO_TICKS(10)) == pdPASS);
		if(xTimerStop(AccelBtimer,pdMS_TO_TICKS(10)) == pdPASS);

		if(AccelCBinfo[sensorid].event_callback !=NULL) {
			Batchevent.event = QL_SENSOR_EVENT_BATCH;
			Batchevent.numpkt = samplescnt;
			Batchevent.data = &Buf_Bdata[0];
			AccelCBinfo[sensorid].event_callback(AccelCBinfo[sensorid].cookie,&Batchevent);
			//Reset Buf pointer
			Bufp =&Buf_Bdata[0];
			samplescnt=0;
		}
	}
}

static QL_Status M4_LSM6DSL_Accel_Open(struct QL_SDF_SensorDrv *s, QL_SDF_SensorDrvInstanceHandle_t *instance_handle) {

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

static QL_Status M4_LSM6DSL_Accel_Close(QL_SDF_SensorDrvInstanceHandle_t instance_handle) {
	/* KK: check validity of handle. This is probably not needed and can be removed */
	QL_ASSERT((unsigned int) instance_handle < CONFIG_MAX_FFE_ACCEL_INSTANCE);

	/* move the state of this instance back to ready to use, SENSOR_STATE_PROBED */
	accel_drv_priv.devInstances[(unsigned int)instance_handle].state = SENSOR_STATE_PROBED;

	/* indicate to framework that the instance was closed successfully */
	return QL_STATUS_OK;
}

static QL_Status M4_LSM6DSL_Accel_Read(QL_SDF_SensorDrvInstanceHandle_t instance_handle, void *buf, int size) {
	uint8_t reg;
	QL_Status status = QL_STATUS_OK;

	QL_ASSERT( (unsigned int)instance_handle < CONFIG_MAX_FFE_ACCEL_INSTANCE);

//	printf("sensorid: %u,cmd: %u\r\n", sensorid, cmd);

	/* check if the instance is in open state first */
	if (SENSOR_STATE_OPENED != accel_drv_priv.devInstances[(unsigned int) instance_handle].state)
	{
		return QL_STATUS_ERROR;
	}

	if(buf == NULL){
		printf("%s - buffer is NULL\n",__func__);
		return QL_STATUS_ERROR;
	}
	reg = LSM6DSL_OUTX_L_XL;
	status = Read_AccelData( reg, buf, size);
	return status;
}


static QL_Status QL_M4_OperationForAccel(unsigned int sensorID, unsigned int operation, void *arg) {

	uint8_t i2c_read_data = 0x0;
	uint8_t i2c_write_data = 0x0;
	unsigned int argval=0;

	switch(operation) {
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

	case QL_SAL_IOCTL_SET_DYNAMIC_RANGE:
	{
		AccelAxisRange = *(unsigned int *)arg;
        argval = convert_m4_range_to_accel_range(AccelAxisRange);
        AccelAxisRange = argval;
                
        if(HAL_I2C_Read(LSM6DSL_SLAVE_ADDR, LSM6DSL_CTRL1_XL, &i2c_read_data, 1) != HAL_OK) return QL_STATUS_ERROR;
		//i2c_write_data = (i2c_read_data & 0x0C) | (argval<<2);
        i2c_write_data = (i2c_read_data & 0xF3) | (argval<<2);
		if(HAL_I2C_Write(LSM6DSL_SLAVE_ADDR, LSM6DSL_CTRL1_XL, &i2c_write_data, 1) != HAL_OK) return QL_STATUS_ERROR;
		break;
	}

	case QL_SAL_IOCTL_GET_DYNAMIC_RANGE:
	{
		if(HAL_I2C_Read(LSM6DSL_SLAVE_ADDR, LSM6DSL_CTRL1_XL, &i2c_read_data, 1) != HAL_OK) return QL_STATUS_ERROR;
		*(unsigned int *)arg = convert_accel_range_to_m4_range((unsigned int) ((i2c_read_data>>2) & 0x03));
		break;
	}

	case QL_SAL_IOCTL_SET_ODR:
	{
		AccelODR = *(unsigned int *)arg;
		argval = convert_m4_freq_to_accel_freq(AccelODR);
        
		if(HAL_I2C_Read(LSM6DSL_SLAVE_ADDR, LSM6DSL_CTRL1_XL, &i2c_read_data, 1) != HAL_OK) return QL_STATUS_ERROR;
		i2c_write_data =  (i2c_read_data & 0x0F) | (argval<<4);
		if(HAL_I2C_Write(LSM6DSL_SLAVE_ADDR, LSM6DSL_CTRL1_XL, &i2c_write_data, 1) != HAL_OK) return QL_STATUS_ERROR;

        break;
	}

	case QL_SAL_IOCTL_GET_ODR:
	{
		if(HAL_I2C_Read(LSM6DSL_SLAVE_ADDR, LSM6DSL_CTRL1_XL, &i2c_read_data, 1) != HAL_OK) return QL_STATUS_ERROR;
		*(unsigned int *)arg = convert_accel_freq_to_m4_freq((unsigned int) ((i2c_read_data>>4) & 0x0F));
		break;
	}

	case QL_SAL_IOCTL_SET_BATCH:
	{
		batchPeriod = *(unsigned int *)arg;

		unsigned int sample = (unsigned int)(((float) (1.0/(float)AccelODR)) * 1000);
		sample = sample > 0 ? sample :1;

		if(batchPeriod % sample != 0)
			batchcount = (batchPeriod / sample) + 1;
		else
			batchcount = (batchPeriod / sample);
		sensorid = sensorID;
		vTaskDelay(pdMS_TO_TICKS(100));
		xTimerChangePeriod(AccelBtimer,pdMS_TO_TICKS(sample),portMAX_DELAY);
		xTimerStart(AccelBtimer,portMAX_DELAY);
		break;
	}

	case QL_SAL_IOCTL_GET_BATCH:
	{
		*(unsigned int *)arg = (unsigned int) batchPeriod;
		break;
	}

	case QL_SAL_IOCTL_BATCH_FLUSH:
	{
		if(samplescnt != 0) {
			//STOP the Timer
			if(xTimerChangePeriod(AccelBtimer,pdMS_TO_TICKS(1000),pdMS_TO_TICKS(10)) == pdPASS);
			if(xTimerStop(AccelBtimer,pdMS_TO_TICKS(10)) == pdPASS);

			// Send the event Call back
			if(AccelCBinfo[sensorid].event_callback !=NULL) {
				Batchevent.event = QL_SENSOR_EVENT_BATCH;
				Batchevent.numpkt = samplescnt;
				Batchevent.data = &Buf_Bdata[0];
				AccelCBinfo[sensorid].event_callback(AccelCBinfo[sensorid].cookie,&Batchevent);
				//Reset Buf pointer
				Bufp =&Buf_Bdata[0];
				samplescnt=0;
				batchcount=0;
			}
		}
		break;
	}

	case QL_SAL_IOCTL_ALLOC_DATA_BUF:
	{
		// call the platform function

		if (QL_STATUS_OK != QL_SDF_SensorAllocMem(	sensorID,
				(void **)&(((struct QL_SF_Ioctl_Set_Data_Buf*)arg)->buf) ,
				((struct QL_SF_Ioctl_Set_Data_Buf*)arg)->size) )
		{
			QL_TRACE_SENSOR_ERROR("QL_SDF_SensorAllocMem failed to get memory for FFE sensor ID = %d\n", sensorID);
			return	QL_STATUS_ERROR;
		}
	}
	break;

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
		//Start the Accelerometer.
		IsAccelEnabled = (uint8_t) (*(unsigned int*)arg);
		break;
	}

	case QL_SAL_IOCTL_GET_ENABLE:
	{
		*(unsigned int *)arg = (unsigned int)IsAccelEnabled;
		break;
	}

	default:
		QL_TRACE_SENSOR_ERROR(" Operation - %d , Failed. Sensor ID %d\n",operation,sensorID);
		break;
	}

	return QL_STATUS_OK;
}

static QL_Status M4_LSM6DSL_Accel_Ioctl(QL_SDF_SensorDrvInstanceHandle_t instance_handle, unsigned int cmd, void *arg) {
	unsigned int sensorid;

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
	//TODO: Check for the Adaptability
//	if(QL_STATUS_OK != Accel_FFE_IsArgValid(cmd, arg))
//	{
//		QL_TRACE_SENSOR_ERROR("Command %d Arguments Invalid for sensor ID = %d\n", cmd, sensorid);
//		return QL_STATUS_ERROR;
//	}

	return QL_M4_OperationForAccel(sensorid, cmd, arg);
}

QL_Status M4_LSM6DSL_Accel_Init(void)
{
	/*
	 * LSM6DSL ACCEL Init sequence
	 *
	 * (1) Read CHIP ID and verify (CHIP ID REG)
	 * (2) Soft Reset ???
	 * (3) Wait for 50 ms after Soft Reset, Check if any error (ERROR STATUS REG)
	 * (4) Set default Rate(ODR), range and power mode(LP/HP)
	 * (5) Wait atleast 5ms, Check if any error (ERROR STATUS REG) and power mode (PMU STATUS REG) is as set
	 *
	 */

	uint8_t i2c_read_data = 0x0;
	uint8_t i2c_write_data = 0x0;

	xI2CConfig.eI2CFreq = I2C_400KHZ; //I2C_200KHZ;
	xI2CConfig.eI2CInt = I2C_DISABLE;
	xI2CConfig.ucI2Cn = LSM6DSL_DEV_I2C_BUS;					/* SM0 == I2C0 */

	HAL_I2C_Init(xI2CConfig);

	/* chip id check */
	if(HAL_I2C_Read(LSM6DSL_SLAVE_ADDR, LSM6DSL_WHO_AM_I, &i2c_read_data, 1) != HAL_OK) return QL_STATUS_ERROR;
	if(i2c_read_data != LSM6DSL_DEVICE_ID) return QL_STATUS_ERROR;

	printf("LMS6DSL chip id check OK (chip id = 0x%x)\n", i2c_read_data);

	/* soft reset */
	i2c_write_data = LSM6DSL_SW_RESET;
	if( HAL_I2C_Write(LSM6DSL_SLAVE_ADDR, LSM6DSL_CTRL3_C, &i2c_write_data, 1) != HAL_OK ) return QL_STATUS_ERROR;

    
    /* wait for 50ms */
	volatile int x = 50000;
	while(x--);
	//vTaskDelay(pdMS_TO_TICKS(50));
    
#if 1 //Do not remove this until Merced Hardware is fixed. Interrupt pin is shared with NM500 Data bus        
    /* Set interrupt line to OpenDrain mode */
    /* 0x20 = H_LACTIVE 0: interrupt pads active high, 1: interrupt pads active low */
    /* 0x10 = PP_OD bit 0: PushPull mode, 1: Open Drain mode */
    /* 0x04 = IF_INC bit 0: Disable addr auto increment, 1: enable address auto increment */
    /* 0x40 = BDU bit 0: 0: continuous update; 1: output registers not updated until MSB and LSB have
                    been read */
    //i2c_write_data = 0x14; // From J-Link script
    i2c_write_data = 0x54; // From J-Link script + Block Data Update
    if(HAL_I2C_Write(LSM6DSL_SLAVE_ADDR, LSM6DSL_CTRL3_C, &i2c_write_data, 1) != HAL_OK) return QL_STATUS_ERROR;
       
    if(HAL_I2C_Read(LSM6DSL_SLAVE_ADDR, LSM6DSL_CTRL3_C, &i2c_read_data, 1) != HAL_OK) return QL_STATUS_ERROR;
	printf("LMS6DSL LSM6DSL_CTRL3_C = 0x%02x)\n", i2c_read_data);
    
#endif

	/* init sequence: set ODR, range and power mode */
    AccelAxisRange = LSM6DSL_DEFAULT_ACCEL_RANGE;
	i2c_write_data = (LSM6DSL_DEFAULT_ODR_ACCEL_RATE<<4) | (LSM6DSL_DEFAULT_ACCEL_RANGE<<2);
	if(HAL_I2C_Write(LSM6DSL_SLAVE_ADDR, LSM6DSL_CTRL1_XL, &i2c_write_data, 1) != HAL_OK) return QL_STATUS_ERROR;

	printf("LSM6DSL Write ODR ... OK\n");

	/* wait for atleast 5 ms */
	volatile int z = 5000;
	while(z--);
	//vTaskDelay(pdMS_TO_TICKS(5));

	printf("LSM6DSL Init Sequence OK, no errors\n");

	/* at this point probe() is ok */
	printf("LSM6DSL Probe on M4 OK\n\n");

	return QL_STATUS_OK;
}

QL_Status M4_LSM6DSL_Accel_Probe(struct QL_SDF_SensorDrv *s)
{
	unsigned int index;

	QL_ASSERT(s != NULL);
	QL_ASSERT(s == (struct QL_SDF_SensorDrv *)&accel_drv_priv); // we should get back the same instance that we provided during registration

	if(QL_STATUS_OK != M4_LSM6DSL_Accel_Init())
	{
		QL_TRACE_SENSOR_ERROR("M4_LSM6DSL_Accel_probe() failed for SAL sensor with ID = %d\n",accel_drv_priv.drvData.id);

		/* probe failed, update the state of each of the instances to not ready, SENSOR_STATE_INVALID */
		for ( index = 0 ; index < CONFIG_MAX_FFE_ACCEL_INSTANCE; index++ )
		{
			accel_drv_priv.devInstances[index].state = SENSOR_STATE_INVALID;
		}

		return QL_STATUS_ERROR;
	}
	QL_TRACE_SENSOR_DEBUG("LSM6DSL_Accel_Probe_In_M4() OK for SAL sensor with ID = %d\n",accel_drv_priv.drvData.id);

	/* probe is ok, update the state of each of the instances to ready to use, SENSOR_STATE_PROBED */
	for ( index = 0 ; index < CONFIG_MAX_FFE_ACCEL_INSTANCE; index++ )
	{
		accel_drv_priv.devInstances[index].state = SENSOR_STATE_PROBED;
	}

	return QL_STATUS_OK;
}

static uint8_t i2c_reg_ctrl1_xl_previous = (LSM6DSL_DEFAULT_ODR_ACCEL_RATE<<4);

static QL_Status M4_LSM6DSL_Accel_Suspend(QL_SDF_SensorDrvInstanceHandle_t instance_handle) {

	uint8_t cmd;
    // Get previousconfig
    if(HAL_I2C_Read((UINT8_t)LSM6DSL_SLAVE_ADDR,(UINT8_t)LSM6DSL_CTRL1_XL,&i2c_reg_ctrl1_xl_previous,1) != HAL_OK) {
		return QL_STATUS_ERROR;
	}
	cmd = i2c_reg_ctrl1_xl_previous & 0x0f;  // Set ODR to zero
	// Write the command register for Accel Low power mode
	if(HAL_I2C_Write((UINT8_t)LSM6DSL_SLAVE_ADDR,(UINT8_t)LSM6DSL_CTRL1_XL,(UINT8_t*)cmd,1) != HAL_OK) {
		return QL_STATUS_ERROR;
	}
	// Wait for 4MS
	vTaskDelay(pdMS_TO_TICKS(4));

        printf("Accel_Suspend\n");//*Tim*
	return QL_STATUS_OK;
}


static QL_Status M4_LSM6DSL_Accel_Resume(QL_SDF_SensorDrvInstanceHandle_t instance_handle) {

	// Write the command register for Accel power mode
	if(HAL_I2C_Write(LSM6DSL_SLAVE_ADDR, LSM6DSL_CTRL1_XL, &i2c_reg_ctrl1_xl_previous, 1) != HAL_OK) return QL_STATUS_ERROR;
	// Wait for 4MS
	vTaskDelay(pdMS_TO_TICKS(4));
    
	return QL_STATUS_OK;;
}

__PLATFORM_INIT__ QL_Status Accel_init(void)
{
#if 0
	//S3B_SENSOR_INTR_0 -> GPIO3
	//Configure GPIO in interrupt mode.

	GPIOCfgTypeDef  xGpioCfg;

	xGpioCfg.usPadNum = PAD_14;
	xGpioCfg.ucGpioNum = GPIO_3;
	xGpioCfg.ucFunc = PAD14_FUNC_SEL_GPIO_3;
	xGpioCfg.intr_type = LEVEL_TRIGGERED;//EDGE_TRIGGERED;
	xGpioCfg.pol_type = RISE_HIGH;//FALL_LOW;//RISE_HIGH;
	xGpioCfg.ucPull = PAD_NOPULL;

	HAL_GPIO_IntrCfg(&xGpioCfg);
#endif

	unsigned int index;

	accel_drv_priv.drvData.id = QL_SAL_SENSOR_ID_ACCEL;
	accel_drv_priv.drvData.type = QL_SDF_SENSOR_TYPE_BASE;		/* physical sensor */
	//accel_drv_priv.drvData.max_sample_rate = 0; 				// KK: this is not needed here at all, can be removed ?
	//accel_drv_priv.drvData.name = "Accel"; 					// KK: this is not used anywhere, can be removed ?

	accel_drv_priv.drvData.ops.open = M4_LSM6DSL_Accel_Open;
	accel_drv_priv.drvData.ops.close = M4_LSM6DSL_Accel_Close;
	accel_drv_priv.drvData.ops.read = M4_LSM6DSL_Accel_Read;
	accel_drv_priv.drvData.ops.probe = M4_LSM6DSL_Accel_Probe;
	accel_drv_priv.drvData.ops.ioctl = M4_LSM6DSL_Accel_Ioctl;
	accel_drv_priv.drvData.ops.suspend = M4_LSM6DSL_Accel_Suspend;
	accel_drv_priv.drvData.ops.resume = M4_LSM6DSL_Accel_Resume;

	/* the FFE side ID is common for all instances, it is a sensor/driver level detail */
	//accel_drv_priv.ffe_accel_id = QL_FFE_SENSOR_ID_ACCEL1;

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

//	AccelBtimer = xTimerCreate("Accel_LSM6DSL_Btimer",pdMS_TO_TICKS(1000),pdTRUE, (void*)0xDEA,(TimerCallbackFunction_t)vAccelBtimerCB);
//
//	if(AccelBtimer == NULL){
//		QL_TRACE_SENSOR_ERROR(" %s: Failed to Create Batch Timer\n",__func__);
//		return QL_STATUS_ERROR;
//	}

	return QL_STATUS_OK;
}


#endif // PURE_M4_DRIVERS
