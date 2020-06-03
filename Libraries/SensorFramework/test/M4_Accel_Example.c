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
 *    File   : M4_Accel_Example.c
 *    Purpose: 
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#include "QL_Trace.h"
#include "QL_SAL.h"
#include "QL_SensorIoctl.h"

#include "eoss3_hal_pad_config.h"
#include "eoss3_hal_fpga_gpio.h"
#include <stdio.h>
#include <string.h>                                             // to remove warnings

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "Fw_global_config.h"
                                     // to remove warnings
#include "AccelExample.h"
#include <ffe_ipc_lib.h>
#include <ql_SensorHub_MemoryMap.h>
#include "LSM6DSL_Accel.h"
#include "dbg_uart.h"

#ifdef ENABLE_PURE_M4_GYRO
#include "LSM6DSL_Gyro.h"
#endif


/*
 * Accel Example Algorithm demo task.
 * The task will be idle, until we get a START command.
 * on START, we init the sensor (Accel) and register our callback with the sensor.
 *
 * if algo-on-m4:
 *
 * 		if non-batch mode:
 * 			we get immediate accel data in each callback.
 * 			we pass this data through the m4 algo
 *
 * 		if batch mode:
 * 			we get batch size expired, and batch info in each callback
 * 			we read all of the current batch data, and pass it through the m4 algo
 *
 * 	we also now support a STOP command. The task will then de-init and close the sensor.
 * 	the task will then wait for the next START command.
 *
 *
 */

#define ACCEL_STORE_TO_FILE 0 // 1 = store accel raw samples to a file in ascii format
#define ACCEL_STORE_TIME_SECS  120 // ~120 secs of data to store
#if ACCEL_STORE_TO_FILE
#include "ff.h"

static FATFS FatFs_system;		/* FatFs work area needed for each volume */
static FIL File_object;			/* File object needed for each open file */
static char sprintf_buf[2*512];
static int sprintf_count = 0;
static int accel_opened = 0;
static int accel_wcount = 0; //number of samples written to the file
static uint32_t written = 0;
static void save_accelsamples(short x, short y, short z)
{
    int i;
    FRESULT fresult;

//printf("%d, %d, %d\n", x, y, z);

    if(accel_opened == 0)
    {
        //printf("?");
        return;
    }
    accel_wcount++; //sample count
    //wait a few seconds for the signal to settle
    if(accel_wcount < 0)
        return;

    //convert the 16bit samples to ascii data one (x,y,z) per line
    i = sprintf(&sprintf_buf[sprintf_count],"%d, %d, %d\n", x,y,z);
    sprintf_count += i;
    //write in blocks of 512 for SD card
    if(sprintf_count >= 512)
    {
        written = 0;
        fresult = f_write(&File_object,sprintf_buf, 512, &written);
        if(fresult == FR_OK)
        {
            int j=0;
            for(i=512;i < sprintf_count;i++)
                sprintf_buf[j++] = sprintf_buf[i];

            sprintf_count = j;
            //assumption : samplerate = 104Hz
            if( accel_wcount >= (104*ACCEL_STORE_TIME_SECS))
            {
                //store the remaining in the buffer
                written = 0;
                fresult = f_write(&File_object,sprintf_buf, sprintf_count, &written);
                fresult = f_close(&File_object);
                printf("\nsamples = %d  fclose = %d\n",accel_wcount, fresult);
                accel_opened = 0;
            }
            else
                printf("="); //just a marker to indicate storage is happening
        }
        else
        {
            printf("write Err\n");
            fresult = f_close(&File_object);
            accel_opened = 0;
        }
    }

    return;
}

static void open_accel_file(void)
{
  FRESULT fresult;

  accel_opened = 0;
  fresult = f_mount(&FatFs_system, "", 1);
  printf("f_mount = %d\n", fresult);
  if (fresult != FR_OK)
      return;

  fresult = f_open(&File_object, "accel.txt", FA_WRITE | FA_CREATE_ALWAYS);
  printf("f_open(accel.txt) = %d\n", fresult);
  if (fresult != FR_OK)
      return;

  accel_opened  = 1;
  sprintf_count = 0;
  accel_wcount = -104*10; //10 sec wait at 104Hz
  return;
}



#endif

unsigned int accel_example_algo_in_m4;
unsigned int accel_example_in_batch_mode;

#ifdef ACCEL_EXAMPLE_ON_M4_LIVE
unsigned int accel_example_algo_in_m4 = 1;
unsigned int accel_example_in_batch_mode = 0;
#endif

#ifdef ACCEL_EXAMPLE_ON_M4_BATCH
unsigned int accel_example_algo_in_m4 = 1;
unsigned int accel_example_in_batch_mode = 1;
#endif

#ifdef ACCEL_EXAMPLE_ON_FFE_LIVE
unsigned int accel_example_algo_in_m4 = 0;
unsigned int accel_example_in_batch_mode = 0;
#endif

#ifdef ACCEL_EXAMPLE_ON_FFE_BATCH
unsigned int accel_example_algo_in_m4 = 0;
unsigned int accel_example_in_batch_mode = 1;
#endif

unsigned int accel_example_batch_mode_test_type = 0; // simple 0(always batch). timed 1(flush at fixed time). random 2(flush at random times).


/*
 *
MemoryAddress									Data
				Byte3 [31:24]		Byte2 [23:16]		Byte1[15:8]			Byte0 [7:0]
buf_mem0		Time Stamp													0
buf_mem0 + 4	Accel Data  (X)							Packet Length
buf_mem0 + 8	Accel Data  (Z)							Accel Data  (Y)
 *
 */
struct sensorData_Accel
{
	unsigned int timestamp;
	unsigned short len;
	signed short int data[3];
};

float accel[3];
float accel_scaling = 1.0;



/*
 *
Memory Address				Data
					Byte3 [31:24]		yte2 [23:16]		Byte1[15:8]		Byte0 [7:0]
buf_mem0									SRAM Address [31:0]
buf_mem0 + 4								Number of Packets
 *
 */
struct sensorData_AccelBatchInfo
{
	void* batchdata_addr;
	unsigned int numpackets;
};


struct AccelExampleMsg
{
	unsigned int command;
	union
	{
		unsigned int sensorid;
		unsigned int timestamp;
		struct sensorData_Accel sensorData_A;
		struct sensorData_AccelBatchInfo batchdata;
	};
};



// Batch Configuration:
// BATCH_START_ADDR: (fixed)
// BATCH_MAX_SIZE: (fixed)
// BATCH_SIZE: (variable, interrupt triggered on size)

// Sensor Configuration:
// ODR, RANGE.

// BATCH TESTING CONFIGURATION:
// Batching configs
//TBD: should go to a platform config file
#define CFG_PING_PONG_FACTOR					    3/2 // Prevents overflow @ the cost of memory. Producer writes to ping buffer while consume consumes pong buffer
#define CFG_ACCEL_EXAMPLE_BATCHING_TIME_SEC		    (1)
#define CFG_MAX_BATCH_DATA_SIZE_ACCEL_EXAMPLE 		(CFG_PING_PONG_FACTOR * CFG_ACCEL_EXAMPLE_SAMPLING_FREQ_HZ * CFG_ACCEL_EXAMPLE_BATCHING_TIME_SEC * sizeof(struct sensorData_Accel))


// COMMANDS:
#define ACCEL_EXAMPLE_CMD_START					1
#define ACCEL_EXAMPLE_CMD_ACCEL_DATA			2
#define ACCEL_EXAMPLE_CMD_TAP_DETECTED			3
#define ACCEL_EXAMPLE_CMD_ACCEL_BATCH_DATA 		4
#define ACCEL_EXAMPLE_CMD_STOP					9
#define ACCEL_EXAMPLE_CMD_ENABLE				10


#ifdef ENABLE_PURE_M4_GYRO
// COMMANDS: for Gyro Data only
#define ACCEL_EXAMPLE_CMD_GYRO_DATA			5

#define CFG_ACCEL_EXAMPLE_SAMPLING_FREQ_HZ		ACCEL_RATE_104HZ
#define CFG_ACCEL_EXAMPLE_DYNAMIC_RANGE			ACCEL_RANGE_8G

static unsigned int sensorRange = ACCEL_RANGE_8G;	//= ACCEL_RANGE_8G
static unsigned int sensorODR = ACCEL_RATE_104HZ;	// 100Hz when both gyro and accel are read by M4

static unsigned int gyro_sensorRange = GYRO_RANGE_1000; //Gyro Range
static unsigned int gyro_sensorODR = GYRO_RATE_104HZ;
float gyro_scaling = 1.0;

#else //accel only

#define CFG_ACCEL_EXAMPLE_SAMPLING_FREQ_HZ		416
#define CFG_ACCEL_EXAMPLE_DYNAMIC_RANGE			2

static unsigned int sensorRange = CFG_ACCEL_EXAMPLE_DYNAMIC_RANGE;		//= ACCEL_RANGE_8G
static unsigned int sensorODR = CFG_ACCEL_EXAMPLE_SAMPLING_FREQ_HZ;				// 400 Hz required for DT
#endif
static unsigned int enable = 0;

static TimerHandle_t  batchingTimer = NULL;
static volatile unsigned int  batchingON = 1,batchflush = 1;

/* temporary variables for GET IOCTLs */
static unsigned int temp = 0;
struct QL_SF_Ioctl_Set_Data_Buf tempDataBufInfo;
struct QL_SF_Ioctl_Set_Batch_Data_Buf tempbatch_databuf_info;
struct QL_SF_Ioctl_Req_Events tempEvent;
QL_SensorEventCallback_t tempCallback;


#define ACCEL_EXAMPLE_MSGQ_ITEMS			10
#define ACCEL_EXAMPLE_MGSQ_WAIT_TIME		portMAX_DELAY

QueueHandle_t 	AccelExampleMsgQ;
TaskHandle_t 	AccelExampleTaskHandle;

QL_SAL_SensorHandle_t AccelExampleSensorHandle_Accel;

//user application should process the Accel samples
extern void ProcessAccelData(int timestamp, float accel[3]);
extern unsigned int GetUserAccelRate(unsigned int current); //pass the current default
extern unsigned int GetUserAccelRange(unsigned int current); //pass the current default
#ifdef ENABLE_PURE_M4_GYRO
extern void ProcessGyroData(int timestamp, float accel[3]);
extern unsigned int GetUserGyroRate(unsigned int current); //pass the current default
extern unsigned int GetUserGyroRange(unsigned int current); //pass the current default
#endif

static void batchTimerCB( TimerHandle_t Timer )
{
	QL_Status ret;
	//QL_LOG_DBG_150K("In timer call back\n");
	if (batchingON){
		if(batchflush)
		{
			vTaskDelay(100);
			ret = QL_SAL_SensorIoctl(AccelExampleSensorHandle_Accel, QL_SAL_IOCTL_BATCH_FLUSH,(void *)NULL);
			QL_ASSERT(ret == QL_STATUS_OK);
			batchflush = 0;
		}
		else
		{
			batchingON = 0;
			ret = QL_SAL_SensorIoctl(AccelExampleSensorHandle_Accel, QL_SAL_IOCTL_SET_BATCH,(void *)NULL);
			QL_ASSERT(ret == QL_STATUS_OK);
		}
	}
	else
	{
		batchingON = 1;
		ret = QL_SAL_SensorIoctl(AccelExampleSensorHandle_Accel, QL_SAL_IOCTL_SET_BATCH, (void*)(CFG_ACCEL_EXAMPLE_SAMPLING_FREQ_HZ * CFG_ACCEL_EXAMPLE_BATCHING_TIME_SEC));
		QL_ASSERT(ret == QL_STATUS_OK);
		batchflush = 1;

	}
}

void setupBatchTimer(void) {

	// create a timer of 5 X Batching time
	batchingTimer = xTimerCreate("batchingTimer", ((5 *CFG_ACCEL_EXAMPLE_BATCHING_TIME_SEC *1000)/ portTICK_PERIOD_MS) , /*pdFALSE*/ pdTRUE  , ( void * ) 0, batchTimerCB);
	if(batchingTimer == NULL) {
		dbg_str("Fail to create batching timer!!! \r\n");
	} else {
		if(xTimerStart( batchingTimer, 0 ) != pdPASS) {
			dbg_str("Fail to start batching timer!!! \r\n");
		}
	}
}

void StartAccelExample(void) {
	struct AccelExampleMsg AccelExampleMsg;

	AccelExampleMsg.command = ACCEL_EXAMPLE_CMD_START;
	if(xQueueSend(AccelExampleMsgQ, &AccelExampleMsg, 100) != pdPASS)
	{
		dbg_str("Msg Send Failed\n");
	}
}



static QL_Status SensorEventCallback(void *cookie, struct QL_SensorEventInfo *event_info) {
	struct AccelExampleMsg AccelExampleMsg;

	QL_TRACE_TEST_DEBUG("+\n");
	QL_ASSERT(event_info);

	if (accel_example_algo_in_m4)
	{
		if (QL_SENSOR_EVENT_DATA == event_info->event)
		{
			AccelExampleMsg.command = ACCEL_EXAMPLE_CMD_ACCEL_DATA;
			AccelExampleMsg.sensorData_A = *((struct sensorData_Accel *)event_info->data);

			/* numpkt == LSB SensorID, 2Bytes, MSB 2 Bytes is the number of samples counted (ignore this) */
		}
		else if (QL_SENSOR_EVENT_BATCH == event_info->event )
		{
			AccelExampleMsg.command = ACCEL_EXAMPLE_CMD_ACCEL_BATCH_DATA;
			AccelExampleMsg.batchdata.batchdata_addr = event_info->data;
			AccelExampleMsg.batchdata.numpackets = event_info->numpkt;

			QL_TRACE_TEST_DEBUG("NumPkt In Batch mode = %d\n", event_info->numpkt);
        } else {
			QL_TRACE_TEST_ERROR ("Invalid event type received event= %d \n", event_info->event);
		}
	}

	if(xQueueSend(AccelExampleMsgQ, &AccelExampleMsg, portMAX_DELAY) != pdPASS) {
		QL_TRACE_TEST_ERROR("Msg Send Failed\n");
	}

	QL_TRACE_TEST_DEBUG("-\n");
	return QL_STATUS_OK;                            // to remove warnings ,  added return statement
}
#ifdef ENABLE_PURE_M4_GYRO

static QL_Status SensorGyroEventCallback(void *cookie, struct QL_SensorEventInfo *event_info) {
	struct AccelExampleMsg AccelExampleMsg;

	QL_TRACE_TEST_DEBUG("+\n");
	QL_ASSERT(event_info);

	if (accel_example_algo_in_m4)
	{
		if (QL_SENSOR_EVENT_DATA == event_info->event)
		{
			AccelExampleMsg.command = ACCEL_EXAMPLE_CMD_GYRO_DATA;
			AccelExampleMsg.sensorData_A = *((struct sensorData_Accel *)event_info->data);

			/* numpkt == LSB SensorID, 2Bytes, MSB 2 Bytes is the number of samples counted (ignore this) */
        } else {
			QL_TRACE_TEST_ERROR ("Invalid event type received event= %d \n", event_info->event);
		}
	}

	if(xQueueSend(AccelExampleMsgQ, &AccelExampleMsg, portMAX_DELAY) != pdPASS) {
		QL_TRACE_TEST_ERROR("Msg Send Failed\n");
	}

	QL_TRACE_TEST_DEBUG("-\n");
	return QL_STATUS_OK;                            // to remove warnings ,  added return statement
}


#endif

static void AccelExampleTask(void *a)
{
	QL_Status ret;
	BaseType_t qret;
	unsigned int AccelExampleTaskStop = 0;

	struct QL_SF_Ioctl_Set_Data_Buf databufinfo;

	struct AccelExampleMsg AccelExampleMsg;

	while(!AccelExampleTaskStop)
	{
		memset(&AccelExampleMsg, 0, sizeof(AccelExampleMsg));
		qret = xQueueReceive(AccelExampleMsgQ, &AccelExampleMsg, ACCEL_EXAMPLE_MGSQ_WAIT_TIME);
		QL_ASSERT(qret == pdTRUE);

		switch(AccelExampleMsg.command)
		{
        case ACCEL_EXAMPLE_CMD_START:
			{
//				QL_LOG_INFO_150K("\n\nStarting Accel Example Application\n\n");


                AccelExampleMsg.sensorid = QL_SAL_SENSOR_ID_ACCEL;

				ret = QL_SAL_SensorOpen(&AccelExampleSensorHandle_Accel, AccelExampleMsg.sensorid);
				QL_ASSERT(ret == QL_STATUS_OK);


				ret = QL_SAL_SensorIoctl(AccelExampleSensorHandle_Accel, QL_SAL_IOCTL_SET_CALLBACK, (void *)SensorEventCallback);
				QL_ASSERT(ret == QL_STATUS_OK);
				ret = QL_SAL_SensorIoctl(AccelExampleSensorHandle_Accel, QL_SAL_IOCTL_GET_CALLBACK, (void *)&tempCallback);
				QL_ASSERT(ret == QL_STATUS_OK);

                                //get User requirement
                                sensorODR = GetUserAccelRate(sensorODR);
                                sensorRange = GetUserAccelRange(sensorRange);

				ret = QL_SAL_SensorIoctl(AccelExampleSensorHandle_Accel, QL_SAL_IOCTL_SET_ODR, (void*)&sensorODR);
				QL_ASSERT(ret == QL_STATUS_OK);

				ret = QL_SAL_SensorIoctl(AccelExampleSensorHandle_Accel, QL_SAL_IOCTL_GET_ODR, &temp);
				QL_ASSERT(ret == QL_STATUS_OK);

				ret = QL_SAL_SensorIoctl(AccelExampleSensorHandle_Accel, QL_SAL_IOCTL_SET_DYNAMIC_RANGE, (void*)&sensorRange);
				QL_ASSERT(ret == QL_STATUS_OK);

				ret = QL_SAL_SensorIoctl(AccelExampleSensorHandle_Accel, QL_SAL_IOCTL_GET_DYNAMIC_RANGE, &temp);
				QL_ASSERT(ret == QL_STATUS_OK);




				if(accel_example_in_batch_mode)
				{

				}
				else
				{
					/* non- batch mode */

					if(accel_example_algo_in_m4)
					{
						databufinfo.buf = (void *)LIVE_SENSOR_DATA_ACCEL_ADDR;					/* LIVE DATA ADDR */
						databufinfo.size = LIVE_SENSOR_DATA_PACKET_SIZE;						/* LIVE DATA PACKET SIZE */
					}
					else
					{
						databufinfo.buf = (void *)LIVE_SENSOR_DATA_DT_ADDR;						/* LIVE DATA ADDR */
						databufinfo.size = LIVE_SENSOR_DATA_PACKET_SIZE;						/* LIVE DATA PACKET SIZE */
					}


					ret = QL_SAL_SensorIoctl(AccelExampleSensorHandle_Accel, QL_SAL_IOCTL_SET_DATA_BUF, (void*)&databufinfo );
					QL_ASSERT(ret == QL_STATUS_OK);
//					QL_LOG_DBG_150K("QL_SAL_IOCTL_SET_DATA_BUF 0x%x\n",databufinfo.buf);
					ret = QL_SAL_SensorIoctl(AccelExampleSensorHandle_Accel, QL_SAL_IOCTL_GET_DATA_BUF, (void*)&tempDataBufInfo);
					QL_ASSERT(ret == QL_STATUS_OK);
//					QL_LOG_DBG_150K("QL_SAL_IOCTL_GET_DATA_BUF 0x%x\n",tempDataBufInfo.buf);
//					QL_LOG_DBG_150K("\n\n");

					enable = 1;
					ret = QL_SAL_SensorIoctl(AccelExampleSensorHandle_Accel, QL_SAL_IOCTL_LIVE_DATA_ENABLE, (void*)&enable);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(AccelExampleSensorHandle_Accel, QL_SAL_IOCTL_GET_LIVE_DATA_ENABLE, (void*)&temp);
					QL_ASSERT(ret == QL_STATUS_OK);
//					QL_LOG_DBG_150K("QL_SAL_IOCTL_GET_LIVE_DATA_ENABLE 0x%x\n",temp);
//					QL_LOG_DBG_150K("\n\n");
				}



				/* enable the sensor */
				enable = 1;
				ret = QL_SAL_SensorIoctl(AccelExampleSensorHandle_Accel, QL_SAL_IOCTL_ENABLE, (void *)&enable);
				QL_ASSERT(ret == QL_STATUS_OK);
				ret = QL_SAL_SensorIoctl(AccelExampleSensorHandle_Accel, QL_SAL_IOCTL_GET_ENABLE, &temp);
				QL_ASSERT(ret == QL_STATUS_OK);
//				QL_LOG_DBG_150K("QL_SAL_IOCTL_GET_ENABLE 0x%x\n",temp);
//				QL_LOG_DBG_150K("\n\n");


				/* close the sensor handle, we are done. */
				ret = QL_SAL_SensorClose(AccelExampleSensorHandle_Accel);
				QL_ASSERT(ret == QL_STATUS_OK);
//				QL_LOG_DBG_150K("QL_SAL_SensorClose OK.\n");
//				QL_LOG_DBG_150K("\n\n");

                accel_scaling = Get_Accel_Scale_Factor();
#ifdef ENABLE_PURE_M4_GYRO
                //setup Gyro using same variable
                AccelExampleMsg.sensorid = QL_SAL_SENSOR_ID_GYRO;

				ret = QL_SAL_SensorOpen(&AccelExampleSensorHandle_Accel, AccelExampleMsg.sensorid);
				QL_ASSERT(ret == QL_STATUS_OK);

				ret = QL_SAL_SensorIoctl(AccelExampleSensorHandle_Accel, QL_SAL_IOCTL_SET_CALLBACK, (void *)SensorGyroEventCallback);
				QL_ASSERT(ret == QL_STATUS_OK);

                //get User requirement for Gyro
                gyro_sensorODR = GetUserGyroRate(gyro_sensorODR);
                gyro_sensorRange = GetUserGyroRange(gyro_sensorRange);

				ret = QL_SAL_SensorIoctl(AccelExampleSensorHandle_Accel, QL_SAL_IOCTL_SET_ODR, (void*)&gyro_sensorODR);
				QL_ASSERT(ret == QL_STATUS_OK);

				ret = QL_SAL_SensorIoctl(AccelExampleSensorHandle_Accel, QL_SAL_IOCTL_SET_DYNAMIC_RANGE, (void*)&gyro_sensorRange);
				QL_ASSERT(ret == QL_STATUS_OK);

                gyro_scaling = Get_Gyro_Scale_Factor();

#endif

				if(!accel_example_in_batch_mode && accel_example_algo_in_m4)
				{

					//vSensorHubCallbackInit(2);
                    vSensorHubCallbackInit(1000/sensorODR);//timer value is in msec
					//vSensorHubChangeTimerPeriod(2); // cannot run at 2ms properly!
					vSensorHubStartTimer();

				}
#if ACCEL_STORE_TO_FILE
                open_accel_file();
#endif
			}
			break;

			/* this will happen only in non-batch mode + algo_in_m4 */
        case ACCEL_EXAMPLE_CMD_ACCEL_DATA :
			{
				accel[0] = AccelExampleMsg.sensorData_A.data[0] * accel_scaling;
				accel[1] = AccelExampleMsg.sensorData_A.data[1] * accel_scaling;
				accel[2] = AccelExampleMsg.sensorData_A.data[2] * accel_scaling;
#if ACCEL_STORE_TO_FILE
                save_accelsamples(AccelExampleMsg.sensorData_A.data[0], AccelExampleMsg.sensorData_A.data[1], AccelExampleMsg.sensorData_A.data[2]);
#endif
                ProcessAccelData(AccelExampleMsg.sensorData_A.timestamp >> 8, accel);
			}
			break;
#ifdef ENABLE_PURE_M4_GYRO
			/* this will happen only in non-batch mode + algo_in_m4 */
        case ACCEL_EXAMPLE_CMD_GYRO_DATA :
			{
				accel[0] = AccelExampleMsg.sensorData_A.data[0] * gyro_scaling;
				accel[1] = AccelExampleMsg.sensorData_A.data[1] * gyro_scaling;
				accel[2] = AccelExampleMsg.sensorData_A.data[2] * gyro_scaling;
#if ACCEL_STORE_TO_FILE
                save_accelsamples(AccelExampleMsg.sensorData_A.data[0], AccelExampleMsg.sensorData_A.data[1], AccelExampleMsg.sensorData_A.data[2]);
#endif
                ProcessGyroData(AccelExampleMsg.sensorData_A.timestamp >> 8, accel);
			}
			break;
#endif

			/* this will happen only in batch mode + algo_in_m4 */
        case ACCEL_EXAMPLE_CMD_ACCEL_BATCH_DATA:
			{
				int index = 0;
				struct sensorData_Accel accel_data;
				for ( index = 0; index < AccelExampleMsg.batchdata.numpackets; index++)
				{
					accel_data = *((struct sensorData_Accel *)(AccelExampleMsg.batchdata.batchdata_addr));
					accel[0] = accel_data.data[0] * accel_scaling;
					accel[1] = accel_data.data[1] * accel_scaling;
					accel[2] = accel_data.data[2] * accel_scaling;

					AccelExampleMsg.batchdata.batchdata_addr =  (void *) ((unsigned int) (AccelExampleMsg.batchdata.batchdata_addr) + sizeof(struct sensorData_Accel));

					// Take care of wrap around in case of a circular batched buffer
					if ((unsigned int)(AccelExampleMsg.batchdata.batchdata_addr) >=
                        (unsigned int) ((BATCH_BUFFER_DATA_ADDR_ACCEL) + BATCH_BUFFER_DATA_SIZE_ACCEL ))
					{
						AccelExampleMsg.batchdata.batchdata_addr = (void *)(BATCH_BUFFER_DATA_ADDR_ACCEL);
					}
				}
			}
			break;


        case ACCEL_EXAMPLE_CMD_STOP:
			{
				AccelExampleTaskStop = 1;
				//QL_LOG_INFO_150K("exiting Accel Example Task.\n");
			}
			break;

        case ACCEL_EXAMPLE_CMD_ENABLE:
			{
				QL_SAL_SensorHandle_t sensor_handle;

				//QL_LOG_DBG_150K("\nEnabling Accel Example Application\n\n");


                AccelExampleMsg.sensorid = QL_SAL_SENSOR_ID_ACCEL;


				ret = QL_SAL_SensorOpen(&sensor_handle, AccelExampleMsg.sensorid);
				QL_ASSERT(ret == QL_STATUS_OK);

				/* enable the sensor */
				enable = 1;
				ret = QL_SAL_SensorIoctl(sensor_handle, QL_SAL_IOCTL_ENABLE, (void *)&enable);
				QL_ASSERT(ret == QL_STATUS_OK);
				ret = QL_SAL_SensorIoctl(sensor_handle, QL_SAL_IOCTL_GET_ENABLE, &temp);
				QL_ASSERT(ret == QL_STATUS_OK);
//				QL_LOG_DBG_150K("QL_SAL_IOCTL_GET_ENABLE 0x%x\n",temp);
//				QL_LOG_DBG_150K("\n\n");

				/* close the sensor handle, we are done. */
				ret = QL_SAL_SensorClose(sensor_handle);
				QL_ASSERT(ret == QL_STATUS_OK);
//				QL_LOG_DBG_150K("QL_SAL_SensorClose OK.\n");
//				QL_LOG_DBG_150K("\n\n");
			}
			break;

        default :
			{
				//QL_LOG_INFO_150K("Unknown message\n");
			}
			break;

		}
	}
}

void InitAccelExample(void)
{
	AccelExampleMsgQ = xQueueCreate(ACCEL_EXAMPLE_MSGQ_ITEMS, sizeof(struct AccelExampleMsg));
	QL_ASSERT(AccelExampleMsgQ);

	xTaskCreate(AccelExampleTask, "DblTapTsk", 256/*1024*/, NULL, tskIDLE_PRIORITY + 4, &AccelExampleTaskHandle);
	QL_ASSERT(AccelExampleTaskHandle);

}
