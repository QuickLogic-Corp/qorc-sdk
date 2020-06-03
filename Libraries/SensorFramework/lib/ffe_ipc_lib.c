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
 *    File   : ffe_ipc_lib.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"
#if FFE_DRIVERS

#include "QL_FFE_SensorConfig.h"
#include "QL_Trace.h"
#include "ffe_ipc_lib.h"
#include "eoss3_hal_pkfb.h"
//#include "eoss3_hal_rcc.h"
//#include "eoss3_hal_power.h"
//#include "eoss3_hal_clock.h"
#include "s3x_clock_hal.h"
#include "eoss3_hal_pad_config.h"
#include "eoss3_hal_wb.h"
#include "eoss3_hal_i2c.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include <QL_SensorIoctl.h>
#include <QL_SAL.h>

#include <ql_SensorHub_MemoryMap.h>
#ifndef FFE_MIN_TICK_PERIOD_MS
#define FFE_MIN_TICK_PERIOD_MS 10  // FFE tick time period minimum in milli seconds
#endif
#ifndef FFE_MAX_TICK_PERIOD_MS
#define FFE_MAX_TICK_PERIOD_MS 200	// FFE tick time period maximum in milli seconds
#endif


/* KK: move all these defines to project level config file TODO */
//#define FFE_USE_PACKET_FIFO

/* control if we execute a FFE Halt and FFE Release Sequence while changing FFE Sensor Params */
//#define FFE_USE_HALT_RELEASE_FOR_SENSOR_CONFIG_CHANGE

/*
 * This flag controls whether M4 applies its own logic to decide whether
 * the FFE Tick Period should be changed, depending on the sensor configurations.
 * Without this flag, the calculation of the appropriate value for the FFE
 * Tick Period lies with the client who is controlling the FFE timing
 * and the M4 only acts as a medium to achieve the control.
 */
#define FFE_TICK_MS_DECISION_WITH_M4


/* should move to a common sensors structure header */
/* KK: to do while reviewing, after functional */
typedef struct
{
	unsigned int timestamp;              //timestamp
	unsigned int ContextGesture; 		//MSB 16 bits Context, LSB 16bits Gesture (B2C)
	unsigned int WalkingCount;    		//32 bit count
	unsigned int RunningCount;    		//32 bit count
} pcgdata_t;


#ifdef FFE_USE_PACKET_FIFO
/* Task and Queue Configuration for FFE IPC Task */
#define FFE_IPC_TASK_MSGQ_LENGTH			(512)
#define FFE_IPC_TASK_STACK_SIZE				(1024)
#define FFE_IPC_TASK_PRIORITY				(tskIDLE_PRIORITY + 16)
#define FFE_IPC_FIFO_ID						(FIFO1)	//TODO : this needs to be changed
#define FFE_IPC_MGSQ_WAIT_TIME				(portMAX_DELAY)


/*
 * control packet indicates the Sensor ID, number of Data Packets available, and the memptr at which data packets are available
 * and passed in via the Packet FIFO, whereas the data packets are placed into SRAM ?
 * macros to obtain the parts of the control packet are defined here:
 *
 */
#define CPKT_GET_FFE_SENSOR_ID(_cpkt_)		(_cpkt_.data[0] & 0xFFFF)
#define CPKT_GET_NUMPKTS(_cpkt_)			((_cpkt_.data[0] >> 16) & 0xFFFF)
#define CPKT_GET_MEMPTR(_cpkt_)				((unsigned char *)(_cpkt_.data[1]))

static QueueHandle_t xHandleQueueFFEIPCHandler;
static TaskHandle_t xHandleTaskFFEIPCHandler;

#else

#define FFE_IPC_TASK_MSGQ_LENGTH			(512)
#define FFE_IPC_TASK_STACK_SIZE				(1024)//(1024)
#define FFE_IPC_TASK_PRIORITY				(tskIDLE_PRIORITY + 17)
#define FFE_IPC_MGSQ_WAIT_TIME				(portMAX_DELAY)

static QueueHandle_t 	xHandleQueueFFEIPCHandler;
static TaskHandle_t 	xHandleTaskFFEIPCHandler;

struct xFFEIPCMSGPacket
{
	uint32_t event;
	uint8_t data[16]; // upto 16 bytes in event data
};

#endif // FFE_USE_PACKET_FIFO



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

struct sensorData_Accel accelData;
struct sensorData_Accel accelData2;
struct sensorData_Mag magData;
struct sensorData_Gyro gyroData;


/* export section related variables */
/* FFE side representation with local data structures maintained with the FFE IPC Library */
//static struct QL_SF_Ioctl_Req_Events cbinfo[QL_FFE_MAX_SENSORS];
static struct QL_FFE_Sensor QL_FFE_Sensors[QL_FFE_MAX_SENSORS];

static unsigned int *ExportSectionBase = (unsigned int*)0x40040034;
static unsigned int ffe_tick_ms;

#ifdef USE_INTERRUPTS_FOR_LIVE_DATA
static QL_SensorEventCallback_t callback_Accel = NULL;
static QL_SensorEventCallback_t callback_Accel1 = NULL;
static QL_SensorEventCallback_t callback_Mag = NULL;
static QL_SensorEventCallback_t callback_Gyro = NULL;
#endif /* USE_INTERRUPTS_FOR_LIVE_DATA */

/* all the sections in the export section */
#define FFE_SECTION_BASE_ADDR_GLOBAL					(ExportSectionBase)

#define FFE_SECTION_BASE_ADDR_ATTRIB_ACCEL				(ExportSectionBase + (offsetof(struct QL_ExportSection, QL_FFE_SensorAccelAttrib) / 4))
#define FFE_SECTION_BASE_ADDR_ATTRIB_ACCEL2				(ExportSectionBase + (offsetof(struct QL_ExportSection, QL_FFE_SensorAccelAttrib) / 4))
#define FFE_SECTION_BASE_ADDR_ATTRIB_GYRO				(ExportSectionBase + (offsetof(struct QL_ExportSection, QL_FFE_SensorGyroAttrib) / 4))
//#define FFE_SECTION_BASE_ADDR_ATTRIB_MAG				(ExportSectionBase + (offsetof(struct QL_ExportSection, QL_FFE_SensorMagAttrib) / 4))

#define FFE_SECTION_BASE_ADDR_CONFIG_ACCEL				(ExportSectionBase + (offsetof(struct QL_ExportSection, QL_FFE_SensorAccel[0]) / 4))
#define FFE_SECTION_BASE_ADDR_CONFIG_ACCEL2				(ExportSectionBase + (offsetof(struct QL_ExportSection, QL_FFE_SensorAccel[1]) / 4)) // CHANGE TBD.
#define FFE_SECTION_BASE_ADDR_CONFIG_GYRO				(ExportSectionBase + (offsetof(struct QL_ExportSection, QL_FFE_SensorGyro) / 4))
#define FFE_SECTION_BASE_ADDR_CONFIG_MAG				(ExportSectionBase + (offsetof(struct QL_ExportSection, QL_FFE_SensorMag) / 4))
#define FFE_SECTION_BASE_ADDR_CONFIG_PCG				(ExportSectionBase + (offsetof(struct QL_ExportSection, QL_FFE_SensorPCG) / 4))
#define FFE_SECTION_BASE_ADDR_CONFIG_DOUBLETAP			(ExportSectionBase + (offsetof(struct QL_ExportSection, QL_FFE_SensorDoubletap) / 4))
#define FFE_SECTION_BASE_ADDR_CONFIG_SENSIML_APP		(ExportSectionBase + (offsetof(struct QL_ExportSection, QL_FFE_SensiML_App) / 4))


/*
 * set param value of param named _param_name_ inside a structure of type _section_type_spec_ to _u32_param_value_
 * and the actual instance of that structure is located exactly at address _section_base_addr_
 * example usage
 * SET_PARAM_VAL_TO_FFE_X(FFE_SENSORTASK_SECTION_BASE_ADDR, struct KYC_FFESensorTask, taskPeriod, 20);
 */
#define SET_PARAM_VAL_TO_FFE_X(_section_base_addr_, _section_type_spec_, _param_name_, _u32_param_value_)											\
		do																																			\
		{																																			\
			unsigned int __val__ = _u32_param_value_;																								\
			HAL_FFE_WriteMem((_section_base_addr_ + (offsetof(_section_type_spec_, _param_name_)/4)), sizeof(__val__), &__val__);					\
		} 																																			\
		while (0)


/*
 * get param value of param named _param_name_ inside a structure of type _section_type_spec_ stored into address _u32_param_address_
 * from an actual instance of that structure which is located exactly at address _section_base_addr_
 * example usage
 * unsigned int currentTaskPeriod = 0;
 * GET_PARAM_VAL_FROM_FFE_X(FFE_SENSORTASK_SECTION_BASE_ADDR, struct KYC_FFESensorTask, taskPeriod, &currentTaskPeriod);
 */
#define GET_PARAM_VAL_FROM_FFE_X(_section_base_addr_, _section_type_spec_, _param_name_, _u32_param_address_)										\
		do																																			\
		{																																			\
			unsigned int __val__ = 0;																												\
			/*printf("get param 0x%x\n",(_section_base_addr_ + (offsetof(_section_type_spec_, _param_name_)/4)));*/									\
		HAL_FFE_ReadMem((_section_base_addr_ + (offsetof(_section_type_spec_, _param_name_)/4)), sizeof(__val__), &__val__);					\
		*(unsigned int *)_u32_param_address_ = __val__;																							\
	}																																			\
while (0)


	/* set/get config values to/from sensor specific config section as a complete 32-bit value, not DSP Q15.16 format ! */
#define SET_PARAM_VAL_TO_FFE_32BIT_X(_section_base_addr_, _section_type_spec_, _param_name_, _u32_param_value_)										\
		do																																			\
		{																																			\
			HAL_FFE_WriteMem32((_section_base_addr_ + (offsetof(_section_type_spec_, _param_name_)/4)), _u32_param_value_);							\
			/*printf("set config 0x%x\n",(_section_base_addr_ + (offsetof(struct QL_FFE_Sensor, _param_name_)/4)));*/								\
		}																																			\
		while (0)


#define GET_PARAM_VAL_FROM_FFE_32BIT_X(_section_base_addr_, _section_type_spec_, _param_name_, _u32_param_address_)									\
		do																																			\
		{																																			\
			HAL_FFE_ReadMem32((_section_base_addr_ + (offsetof(_section_type_spec_, _param_name_)/4)), _u32_param_address_);						\
			/*printf("get config 0x%x\n",(_section_base_addr_ + (offsetof(struct QL_FFE_Sensor, _param_name_)/4)));*/								\
		} 																																			\
		while (0)



#define EXPORT_SECTION_SENSOR_ATTRIB			1
#define EXPORT_SECTION_SENSOR_CONFIG			2


	/* convenience macro to get base address of each section from sensor name + section name */
#define SENSOR_SECTION_BASE_ADDR(section_name, sensor_name) 		(FFE_SECTION_BASE_ADDR_##section_name##_##sensor_name)

	/* note that the sensor ID is the FFE Sensor ID. this may not be the same as SAL side Sensor ID */
	unsigned int * getExportSectionBaseAddress(int sectionID, int sensorID)
	{
	switch (sectionID)
	{
	/* attribute section for the sensor */
	case EXPORT_SECTION_SENSOR_ATTRIB:
	{
		switch (sensorID)
		{
		case QL_FFE_SENSOR_ID_ACCEL1:
		{
			return SENSOR_SECTION_BASE_ADDR(ATTRIB,ACCEL);
		}
		case QL_FFE_SENSOR_ID_ACCEL2:
		{
			// note attribute is shared between ACCEL and ACCEL2 instances !
			return SENSOR_SECTION_BASE_ADDR(ATTRIB,ACCEL2);
		}
		case QL_FFE_SENSOR_ID_GYRO:
		{
			return SENSOR_SECTION_BASE_ADDR(ATTRIB,GYRO);
		}
		case QL_FFE_SENSOR_ID_MAG:
		{
			// FFE does not support attribute section for M yet.
			//return SENSOR_SECTION_BASE_ADDR(ATTRIB,MAG);
			return NULL;
		}
		}
	}
	break;

	/* config section for the sensor */
	case EXPORT_SECTION_SENSOR_CONFIG:
	{
		switch (sensorID)
		{
		case QL_FFE_SENSOR_ID_ACCEL1:
		{
			return SENSOR_SECTION_BASE_ADDR(CONFIG,ACCEL);
		}

		case QL_FFE_SENSOR_ID_ACCEL2:
		{
			return SENSOR_SECTION_BASE_ADDR(CONFIG,ACCEL2);
		}

		case QL_FFE_SENSOR_ID_GYRO:
		{
			return SENSOR_SECTION_BASE_ADDR(CONFIG,GYRO);
		}

		case QL_FFE_SENSOR_ID_MAG:
		{
			return SENSOR_SECTION_BASE_ADDR(CONFIG,MAG);
		}

		case QL_FFE_SENSOR_ID_PCG:
		{
			return SENSOR_SECTION_BASE_ADDR(CONFIG,PCG);
		}

		case QL_FFE_SENSOR_ID_DOUBLETAP:
		{
			return SENSOR_SECTION_BASE_ADDR(CONFIG,DOUBLETAP);
		}
        case QL_SENSI_ML_ID_APP1:
		{
			return SENSOR_SECTION_BASE_ADDR(CONFIG,SENSIML_APP);
		}
		
		}

	}
	break;

	}

	/* all default cases end up here - indicating invalid section request */
	return NULL;
	}



	/*!
	 * \fn 		HAL_StatusTypeDef HAL_FFE_WriteMem(volatile UINT32_t *ulMem_addr, UINT32_t ulLen, void *buf)
	 * \brief 	This function write into FFE Data memory section
	 * \param 	ulMem_addr -- DM Memory Address
	 * \param 	ulLen 	   -- Number of bytes to write
	 * \param 	buf        -- Data buffer to write
	 * \return      HAL Status
	 */
	static QL_Status HAL_FFE_WriteMem(volatile UINT32_t *ulMem_addr, UINT32_t ulLen, void *buf)
	{
		QL_ASSERT(ulLen);
		QL_ASSERT(buf);

		UINT32_t ulIndex = 0;
		UINT32_t ulTemp = 0;
		UINT32_t *ulTempBuf = (UINT32_t*)buf;

		for(ulIndex = 0; ulIndex < (ulLen/4); ulIndex++)
		{
			ulTemp = *ulTempBuf++;
			*ulMem_addr = ulTemp << 16;
			ulMem_addr++;
		}
		return QL_STATUS_OK;
	}

	/*!
	 * \fn 		HAL_StatusTypeDef HAL_FFE_ReadMem(volatile UINT32_t *ulMem_addr, UINT32_t ulLen, void *buf)
	 * \brief 	This function read from FFE Data memory section
	 * \param 	ulMem_addr -- DM Memory Address
	 * \param 	ulLen 	   -- Number of bytes to read
	 * \param 	buf        -- Data buffer to read
	 * \return      HAL Status
	 */
	static QL_Status HAL_FFE_ReadMem(volatile UINT32_t *ulMem_addr, UINT32_t ulLen, void *buf)
	{
		if(ulLen == 0 || buf == NULL )
		{
			QL_TRACE_FFE_ERROR("Invalid parameters received\r\n");
			return QL_STATUS_ERROR;
		}
		UINT32_t ulIndex = 0;
		UINT32_t ulTemp = 0;
		UINT32_t *ulTempBuf = (UINT32_t*)buf;

		for(ulIndex = 0; ulIndex < (ulLen/4); ulIndex++)
		{
			ulTemp = *ulMem_addr++;
			*ulTempBuf = ulTemp >> 16;
			ulTempBuf++;
		}
		return QL_STATUS_OK;
	}

	/* write a complete 32 Bit Value as is, without interpreting this as a Q15.16 number on the FFE ! */
	static void HAL_FFE_WriteMem32(volatile UINT32_t *ulMem_addr, UINT32_t value)
	{
		QL_ASSERT(ulMem_addr);
		*ulMem_addr = value;
	}

	/* read a complete 32 Bit Value as is, without interpreting this as a Q15.16 number on the FFE ! */
	static void HAL_FFE_ReadMem32(volatile UINT32_t *ulMem_addr, UINT32_t* value_addr)
	{
		QL_ASSERT(ulMem_addr);
		*value_addr = *ulMem_addr;
	}


	static void FFE_SetTickPeriod(unsigned int timeInms)
	{
		//printf("timeInms: %u\n",timeInms);
		/* valid sample period passed in ? */
		if(timeInms > 0)
		{
			/* if the new time to be set is the same as current, do nothing */
			if(ffe_tick_ms == timeInms)
			{
				return;
			}

			/* wait until the FFE completes its cycle, before changing the tick period */
			/*
			 * check if the FFE is running, and if it is, then subscribe to be notified when FFE cycle is complete
			 * and as soon as it is completed, change the tick period
			 */
			if(FFE_IsBusy())
			{
				/* subscribe to FFE Interrupt at end of cycle and block if available */

				/* Hack - Blocking */
				while(FFE_IsBusy());
			}

			/* update our local variable to reflect the sample period being written, always */
			ffe_tick_ms = timeInms;


			/* update FFE global section */
			SET_PARAM_VAL_TO_FFE_X(FFE_SECTION_BASE_ADDR_GLOBAL, struct QL_ExportSection, ffe_tick_ms, timeInms);
/*
			printf("timeInms: 0x%x\n",timeInms);
			printf("timeInms: 0x%x\n",timeInms << 10);
			printf("((timeInms & 0xFF) << 10): 0x%x\n", ((timeInms & 0xFF) << 10));
*/			
			/* Update SPT - without this the timer wont work and the timestamp will be always zero */
			SPT->SPT_CFG &= (~(0xFF << 10));
			SPT->SPT_CFG |= ((timeInms & 0xFF) << 10);
			QL_TRACE_FFE_DEBUG("SPT_CFG = 0x%x\n", SPT->SPT_CFG);
		}
	}


	static void FFE_EnableSPT(unsigned int enable)
	{
		if(enable == 1)
		{
			/* set the SPT Enable Flag */
			SPT->SPT_CFG |= 0x1;
		}
		else
		{
			/* reset the SPT Enable Flag */
			SPT->SPT_CFG &= (~(0x1));
		}
		QL_TRACE_FFE_DEBUG("SPT_CFG = 0x%x\n", SPT->SPT_CFG);
	}


	static unsigned int FFE_GetTickPeriod()
	{
		/* read the tick period from the SPT Config and pass it back */
		return (SPT->SPT_CFG >> 10 & 0xFF);
	}


	unsigned int FFE_GetEnableSPT()
	{
		/* read the enable flag from the SPT Config and pass it back */
		return (SPT->SPT_CFG & 0x1);
	}


	int FFE_IsBusy(void)
	{
		// if FFE is not running, then FFE is not busy
		if(FFE_GetEnableSPT() == 0) return 0;

		return (EXT_REGS_FFE->STATUS & FFE_FFEO_BUSY);
	}


	void FFE_Halt(void)
	{
		unsigned int state;

		// if FFE is not running, then no need of halt-release
		if(FFE_GetEnableSPT() == 0) return;

		GET_PARAM_VAL_FROM_FFE_X(FFE_SECTION_BASE_ADDR_GLOBAL, struct QL_ExportSection, m4_req_state, &state);

		if(state == 0)
		{
			/* FFE is not halted, request for halt now */
			state = 1;
			SET_PARAM_VAL_TO_FFE_X(FFE_SECTION_BASE_ADDR_GLOBAL, struct QL_ExportSection, m4_req_state, state);

			/* wait for ffe_response_state to become 1, indicating FFE is halted */

			state = 0;
			//while(usFFE_ticks_ms)
			while(1)
			{
				GET_PARAM_VAL_FROM_FFE_X(FFE_SECTION_BASE_ADDR_GLOBAL, struct QL_ExportSection, ffe_resp_state, &state);
				if(state)
				{
					/* FFE is now halted */
					break;
				}
			}
		}
		else
		{
			QL_TRACE_FFE_ERROR("FFE is in halt state already\n");
		}
	}


	/* KK: why are we not waiting for complete handshake while releasing ?
	 * because on the FFE side no such requirement seems to be there... */
	void FFE_Release(void)
	{
		unsigned int state;

		// if FFE is not running, then no need of halt-release
		if(FFE_GetEnableSPT() == 0) return;


		GET_PARAM_VAL_FROM_FFE_X(FFE_SECTION_BASE_ADDR_GLOBAL, struct QL_ExportSection, m4_req_state, &state);

		if(state == 1)
		{
			state = 0;
			SET_PARAM_VAL_TO_FFE_X(FFE_SECTION_BASE_ADDR_GLOBAL, struct QL_ExportSection, m4_req_state, state);
		}
		else
		{
			QL_TRACE_FFE_ERROR("FFE is not in halt state\n");
		}
	}


	QL_Status QL_FFE_OperationGlobal(unsigned int operation, void* arg)
	{
		QL_Status retVal = QL_STATUS_NOT_IMPLEMENTED;
		/* check if we need to halt FFE before changing any of these variables */

		switch(operation)
		{
		case QL_FFE_OPS_FFE_ENABLE:
		{
			/* stop or start FFE */
			unsigned int enable = *(unsigned int *)arg;
			FFE_EnableSPT(enable);
			retVal = QL_STATUS_OK;
		}
		break;

		case QL_FFE_OPS_FFE_GET_ENABLE:
		{
			/* check if FFE is running or stopped */
			*(unsigned int *)arg = FFE_GetEnableSPT();
			retVal = QL_STATUS_OK;
		}
		break;

		case QL_FFE_OPS_FFE_SET_PERIOD:
		{
			/* set FFE Tick Period */
			unsigned int ffeTickInms = *(unsigned int *)arg;
			FFE_SetTickPeriod(ffeTickInms);
			retVal = QL_STATUS_OK;
		}
		break;

		case QL_FFE_OPS_FFE_GET_PERIOD:
		{
			/* get FFE Tick Period */
			*(unsigned int *)arg = FFE_GetTickPeriod();
			retVal = QL_STATUS_OK;
		}
		break;

		case QL_FFE_OPS_FFE_HALT:
		{
			/* FFE Halt or Release */
			unsigned int halt = *(unsigned int *)arg;
			if(halt == 1)
			{
				FFE_Halt();
			}
			else
			{
				FFE_Release();
			}
			retVal = QL_STATUS_OK;
		}
		break;

		default:
		{
			// do nothing.
		}
		break;

		}

		return retVal;
	}


#ifdef FFE_TICK_MS_DECISION_WITH_M4
	void QL_FFE_DeduceAndExecuteFFEStateChange()
	{
		/*
		 * we maintain the state of each of the sensors on the FFE side, as it goes through the
		 * QL_FFE_OperationForSensor() only.
		 *
		 * if we see that, FFE is currently disabled, and any one of the sensors is enabled, then
		 * we enable FFE.
		 *
		 * if we see that, FFE is current enabled, and ALL of the sensors are disabled, then we disable
		 * FFE. When we disable the FFE, we also reset the SPT counter value to 0.
		 *
		 * Note that, this is separate from the FFE Tick Period, which is done in a similar fashion, but
		 * we deduce that from the ODR set for each of the sensors.
		 */

		int i = 0;
		unsigned int expected_ffe_state = 0; // disabled assumed

		for (i = 0 ; i < QL_FFE_MAX_SENSORS ; i++)
		{
			if (QL_FFE_Sensors[i].enable == 1)
			{
				expected_ffe_state = 1; // at least one sensor is enabled, ffe should be enabled.
				break;
			}
		}
		// if none of the sensors were enabled, then expected_ffe_state == 0.


		if((expected_ffe_state == 1) && (FFE_GetEnableSPT() == 0))
		{
			// expected FFE state is enabled, but it is currently disabled, so enable FFE now.
			FFE_EnableSPT(1);
		}
		else if((expected_ffe_state == 0) && (FFE_GetEnableSPT() != 0))
		{
			// expected FFE state is disabled, but it is currently enabled, so disable FFE now.
			FFE_EnableSPT(0);
		}
	}

	void QL_FFE_DeduceAndSetFFETickPeriod()
	{
		/*
		 * we check the ODR of each sensor on the FFE side, as it goes through QL_FFE_OperationForSensor() only.
		 *
		 * if we see that the ODR needs the FFE tick period to change, then we change this.
		 * Otherwise we leave it at current tick period.
		 */
		int i = 0;
		unsigned int highest_odr = 0;
		unsigned int ffe_tick_period_required = 0;

		for (i = 0 ; i < QL_FFE_MAX_SENSORS ; i++)
		{
			if (QL_FFE_Sensors[i].outputRate > highest_odr)
			{
				highest_odr = QL_FFE_Sensors[i].outputRate;
			}
		}
		// at the end of the loop, if any of the sensor ODRs are set to non zero
		// we should have the highest_odr required.
		// obtain the ffe_tick_period_required for this ODR:
		if(highest_odr > 0)
		{
			// ffe_tick_period_required should be non zero.
			//printf("highest_odr: %u\n", highest_odr);
			ffe_tick_period_required = (1000/highest_odr); // odr in Hz, tick period in ms.
			//printf("ffe_tick_period_required: %u\n", ffe_tick_period_required);
			if(ffe_tick_period_required < FFE_MIN_TICK_PERIOD_MS)
			{
				ffe_tick_period_required = FFE_MIN_TICK_PERIOD_MS; // ffe supports a minimum of 5 ms, not lower, currently.
			}
			else if (ffe_tick_period_required > FFE_MAX_TICK_PERIOD_MS)
			{
				ffe_tick_period_required = FFE_MAX_TICK_PERIOD_MS;	// ffe does not support greater than 100ms tick period.
			}
			// else, we have a valid ffe tick period (5 - 100 ms)

#ifdef PCG_AND_DT_ON_FFE
			/* special case for PCG + DT
			 * PCG requires ODR of 50 Hz
			 * DT requires ODR of 400 Hz
			 *
			 * However, due to the nature of the FFE side implementation using the Accel's Sensor FIFO
			 * which both these algorithms use internally (same Accel sensor), we need to set the ODR
			 * according to the slower ODR not the faster one, hence according to 50Hz of PCG. This requires
			 * the FFE Tick Period to be 20ms.
			 */

			ffe_tick_period_required = 20;
#endif // PCG_AND_DT_ON_FFE

			FFE_SetTickPeriod(ffe_tick_period_required);
		}
		// else, we can ignore when all sensors have ODR of 0.
	}
#endif // FFE_TICK_MS_DECISION_WITH_M4

	QL_Status QL_FFE_OperationForSensor(unsigned int sensorID, unsigned int operation, void* arg)
	{
#ifdef FFE_USE_HALT_RELEASE_FOR_SENSOR_CONFIG_CHANGE
		unsigned int isFFERunning;
#endif //FFE_USE_HALT_RELEASE_FOR_SENSOR_CONFIG_CHANGE
		QL_Status retVal = QL_STATUS_NOT_IMPLEMENTED;
		/* sequence for operations involving modification of ffe params - global or sensor specific:
		 *
		 * if(FFE is Running) Halt FFE
		 *
		 * Update Parameter Value (Global/Sensor Specific)
		 *
		 * if(FFE was Running) Release FFE
		 *
		 * */


#ifdef FFE_USE_HALT_RELEASE_FOR_SENSOR_CONFIG_CHANGE
		/* use the local static variable ffe_tick_ms as an indicator that the FFE has already been inited and running */\
		isFFERunning = ffe_tick_ms;
		if (isFFERunning)
		{
			FFE_Halt();
		}
#endif // FFE_USE_HALT_RELEASE_FOR_SENSOR_CONFIG_CHANGE

		/* perform operation */
		switch(operation)
		{
		case QL_FFE_SENSOR_OPS_ENABLE:
		{
			unsigned int sensor_enable = *(unsigned int *)arg;
			SET_PARAM_VAL_TO_FFE_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_CONFIG, sensorID), struct QL_FFE_Sensor, enable, sensor_enable);

			QL_FFE_Sensors[sensorID-1].enable = sensor_enable;

#ifdef FFE_TICK_MS_DECISION_WITH_M4
			// check if FFE state change is needed, and if so execute state change.
			QL_FFE_DeduceAndExecuteFFEStateChange();
#endif // FFE_TICK_MS_DECISION_WITH_M4

			retVal = QL_STATUS_OK;
		}
		break;

		case QL_FFE_SENSOR_OPS_GET_ENABLE:
		{
			GET_PARAM_VAL_FROM_FFE_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_CONFIG, sensorID), struct QL_FFE_Sensor, enable, arg);
			retVal = QL_STATUS_OK;
		}
		break;
//#if 0
		case QL_FFE_SENSOR_OPS_LIVE_DATA_ENABLE:
		{
			unsigned int l_live_data_enable = *(unsigned int *)arg;
			SET_PARAM_VAL_TO_FFE_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_CONFIG, sensorID), struct QL_FFE_Sensor, live_data_enable, l_live_data_enable);
			retVal = QL_STATUS_OK;
		}
		break;

		case QL_FFE_SENSOR_OPS_GET_LIVE_DATA_ENABLE:
		{
			GET_PARAM_VAL_FROM_FFE_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_CONFIG, sensorID), struct QL_FFE_Sensor, live_data_enable, arg);
			retVal = QL_STATUS_OK;
		}
		break;

		case QL_FFE_SENSOR_OPS_SET_MEM_BUFFER:
		{
			unsigned int l_live_data_mem_start = (unsigned int)(((struct QL_SF_Ioctl_Set_Data_Buf*)arg)->buf);
			/*unsigned int l_live_data_mem_size =  ((struct QL_SF_Ioctl_Set_Data_Buf*)arg)->size;*/

			if(l_live_data_mem_start != 0x0)
			{
				SET_PARAM_VAL_TO_FFE_32BIT_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_CONFIG, sensorID), struct QL_FFE_Sensor, live_data_memPtr, l_live_data_mem_start);
				// size is fixed due to the format, so no size in the config struct.
			}

			retVal = QL_STATUS_OK;
		}
		break;

		case QL_FFE_SENSOR_OPS_GET_MEM_BUFFER:
		{
			unsigned int l_live_data_mem_start;

			GET_PARAM_VAL_FROM_FFE_32BIT_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_CONFIG, sensorID), struct QL_FFE_Sensor, live_data_memPtr, &l_live_data_mem_start);

			((struct QL_SF_Ioctl_Set_Data_Buf*)arg)->buf = (unsigned char *)l_live_data_mem_start;
			((struct QL_SF_Ioctl_Set_Data_Buf*)arg)->size = 0; // no size in the config struct yet.

			retVal = QL_STATUS_OK;
		}
		break;

		case QL_FFE_SENSOR_OPS_SET_BATCH:
		{

			unsigned int l_batch_mem_start = (unsigned int)(((struct QL_SF_Ioctl_Set_Batch_Data_Buf*)arg)->batch_mem_start);
			unsigned int l_batch_mem_end = (unsigned int)(((struct QL_SF_Ioctl_Set_Batch_Data_Buf*)arg)->batch_mem_end);
			unsigned int l_batchSize = (unsigned int)(((struct QL_SF_Ioctl_Set_Batch_Data_Buf*)arg)->batchSize);
			unsigned int l_batch_info_memPtr = (unsigned int)(((struct QL_SF_Ioctl_Set_Batch_Data_Buf*)arg)->batch_info_memPtr);

			SET_PARAM_VAL_TO_FFE_32BIT_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_CONFIG, sensorID), struct QL_FFE_Sensor, batch_mem_start, l_batch_mem_start);
			SET_PARAM_VAL_TO_FFE_32BIT_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_CONFIG, sensorID), struct QL_FFE_Sensor, batch_mem_end, l_batch_mem_end);
			SET_PARAM_VAL_TO_FFE_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_CONFIG, sensorID), struct QL_FFE_Sensor, batchSize, l_batchSize);
			SET_PARAM_VAL_TO_FFE_32BIT_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_CONFIG, sensorID), struct QL_FFE_Sensor, batch_info_memPtr, l_batch_info_memPtr);

			retVal = QL_STATUS_OK;
		}
		break;

		case QL_FFE_SENSOR_OPS_GET_BATCH:
		{

			unsigned int l_batch_mem_start = 0;
			unsigned int l_batch_mem_end = 0;
			unsigned int l_batchSize = 0;
			unsigned int l_batch_info_memPtr = 0;

			GET_PARAM_VAL_FROM_FFE_32BIT_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_CONFIG, sensorID), struct QL_FFE_Sensor, batch_mem_start, &l_batch_mem_start);
			GET_PARAM_VAL_FROM_FFE_32BIT_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_CONFIG, sensorID), struct QL_FFE_Sensor, batch_mem_end, &l_batch_mem_end);
			GET_PARAM_VAL_FROM_FFE_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_CONFIG, sensorID), struct QL_FFE_Sensor, batchSize, &l_batchSize);
			GET_PARAM_VAL_FROM_FFE_32BIT_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_CONFIG, sensorID), struct QL_FFE_Sensor, batch_info_memPtr, &l_batch_info_memPtr);

			((struct QL_SF_Ioctl_Set_Batch_Data_Buf*)arg)->batch_mem_start = (void *)l_batch_mem_start;
			((struct QL_SF_Ioctl_Set_Batch_Data_Buf*)arg)->batch_mem_end = (void *)l_batch_mem_end;
			((struct QL_SF_Ioctl_Set_Batch_Data_Buf*)arg)->batchSize = (unsigned int)l_batchSize;
			((struct QL_SF_Ioctl_Set_Batch_Data_Buf*)arg)->batch_info_memPtr = (void *)l_batch_info_memPtr;

			retVal = QL_STATUS_OK;
		}
		break;

		case QL_FFE_SENSOR_OPS_BATCH_ENABLE:
		{
			unsigned int batchEnabled = *(unsigned int *)arg;
			SET_PARAM_VAL_TO_FFE_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_CONFIG, sensorID), struct QL_FFE_Sensor, batch_enable, batchEnabled);

			retVal = QL_STATUS_OK;
		}
		break;

		case QL_FFE_SENSOR_OPS_BATCH_GET_ENABLE:
		{
			GET_PARAM_VAL_FROM_FFE_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_CONFIG, sensorID), struct QL_FFE_Sensor, batch_enable, arg);
			retVal = QL_STATUS_OK;
		}
		break;


		case QL_FFE_SENSOR_OPS_BATCH_FLUSH:
		{
			SET_PARAM_VAL_TO_FFE_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_CONFIG, sensorID), struct QL_FFE_Sensor, batchFlush, 1);
			retVal = QL_STATUS_OK;
		}
		break;
		case QL_FFE_SENSOR_OPS_SET_DYNAMIC_RANGE:
		{
			unsigned int sensorDynamicRange = *(unsigned int *)arg;
			SET_PARAM_VAL_TO_FFE_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_ATTRIB, sensorID), struct QL_FFE_SensorAttrib_t, range, sensorDynamicRange);
			retVal = QL_STATUS_OK;
		}
		break;

		case QL_FFE_SENSOR_OPS_GET_DYNAMIC_RANGE:
		{
			GET_PARAM_VAL_FROM_FFE_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_ATTRIB, sensorID),struct QL_FFE_SensorAttrib_t, range, arg);
			retVal = QL_STATUS_OK;
		}
		break;

		case QL_FFE_SENSOR_OPS_SET_ODR:
		{
			unsigned int sensorODR  = *(unsigned int *)arg;
			SET_PARAM_VAL_TO_FFE_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_CONFIG, sensorID), struct QL_FFE_Sensor, outputRate, sensorODR);

			// update the ODR in our internal data.
			QL_FFE_Sensors[sensorID-1].outputRate = sensorODR;

#ifdef FFE_TICK_MS_DECISION_WITH_M4
			// check if the FFE tick period needs to change, and if so, set it.
			QL_FFE_DeduceAndSetFFETickPeriod();
#endif // FFE_TICK_MS_DECISION_WITH_M4

			retVal = QL_STATUS_OK;


		}
		break;

		case QL_FFE_SENSOR_OPS_GET_ODR:
		{
			GET_PARAM_VAL_FROM_FFE_X(getExportSectionBaseAddress(EXPORT_SECTION_SENSOR_CONFIG, sensorID),struct QL_FFE_Sensor, outputRate, arg);
			retVal = QL_STATUS_OK;
		}
		break;
        case QL_FFE_SENSOR_OPS_SET_BATCH_PACKET_IDS:
        {
            unsigned int sensiml_packet_ids[MAX_SENSIML_PACKETS];
            memcpy(sensiml_packet_ids, arg, sizeof(sensiml_packet_ids));
            UINT32_t* addr = (FFE_SECTION_BASE_ADDR_GLOBAL + (offsetof(struct QL_ExportSection, sensiMLAppOptions.sensiml_packetids_selected)/4));
            HAL_FFE_WriteMem(addr, sizeof(sensiml_packet_ids), (void*)&sensiml_packet_ids);
            retVal = QL_STATUS_OK;
        }
        break;

        case QL_FFE_SENSOR_OPS_GET_BATCH_PACKET_IDS:
        {
             unsigned int sensiml_packet_ids[MAX_SENSIML_PACKETS];
             UINT32_t* addr = (FFE_SECTION_BASE_ADDR_GLOBAL + (offsetof(struct QL_ExportSection, sensiMLAppOptions.sensiml_packetids_selected)/4));
             HAL_FFE_ReadMem(addr, sizeof(sensiml_packet_ids), (void*)&sensiml_packet_ids);
             memcpy(arg, sensiml_packet_ids, sizeof(sensiml_packet_ids));
             retVal = QL_STATUS_OK;
        }
        break;
		}

#ifdef FFE_USE_HALT_RELEASE_FOR_SENSOR_CONFIG_CHANGE
		/* if the FFE was halted, then release the FFE */
		if (isFFERunning)			//TODO : Revisit
		{
			FFE_Release();
		}
#endif // FFE_USE_HALT_RELEASE_FOR_SENSOR_CONFIG_CHANGE

		return retVal;
	}


	QL_Status QL_FFE_Sensor_Probe(unsigned int sensorid)
	{
		/* now, we do a dummy probe, assume FFE is always right */
		return QL_STATUS_OK;
	}



	//TODO : This board specific configuration. Has to be moved to place where
	//		board specific pad configuraion is expected

	/* internal function */
	static void QL_FFE_ConfigureSMPads(void)
	{
		PadConfig  padcfg;

		padcfg.ucPin = PAD_0;
		padcfg.ucFunc = PAD0_FUNC_SEL_SCL_0;
		padcfg.ucCtrl = PAD_CTRL_SRC_OTHER;
		padcfg.ucMode = PAD_MODE_INPUT_EN;
		padcfg.ucPull = PAD_PULLUP;
		padcfg.ucDrv = PAD_DRV_STRENGHT_4MA;
		padcfg.ucSpeed = PAD_SLEW_RATE_SLOW;
		padcfg.ucSmtTrg = PAD_SMT_TRIG_DIS;

		HAL_PAD_Config(&padcfg);

		padcfg.ucPin = PAD_1;
		padcfg.ucFunc = PAD1_FUNC_SEL_SDA_0;
		padcfg.ucCtrl = PAD_CTRL_SRC_OTHER;
		padcfg.ucMode = PAD_MODE_INPUT_EN;
		padcfg.ucPull = PAD_PULLUP;
		padcfg.ucDrv = PAD_DRV_STRENGHT_4MA;
		padcfg.ucSpeed = PAD_SLEW_RATE_SLOW;
		padcfg.ucSmtTrg = PAD_SMT_TRIG_DIS;

		HAL_PAD_Config(&padcfg);

#if 0 // UNUSED CODE
printf("PAD 0 io_mux = %x\r\n",IO_MUX->PAD_0_CTRL);
printf("PAD 1 io_mux = %x\r\n",IO_MUX->PAD_1_CTRL);
printf("SDA0_SEL = %x\r\n",IO_MUX->SDA0_SEL_REG);
printf("SCL0_SEL = %x\r\n",IO_MUX->SCL0_SEL_REG);
#endif
	}


	/* configure the FFE (SM) to take control of the I2C bus */
	/* KK: this should be a part of the public FFE API, and configurable, according to
	 * which I2C bus we are targeting */
	void QL_FFE_TakeControlOfI2CBus()
	{
		/*
		 * this function ensures that the SM0/SM1 get the control of the
		 * I2C0/I2C1 bus and FFE can now continue to use the SM0/SM1 to obtain sensor data
		 *
		 * Configure the WB MUX select to allow SM to become the I2C Master.
		 * Otherwise, the WB Master will be the I2C Master (which is M4)
		 *
		 */
		EXT_REGS_FFE->CSR = (WB_CSR_I2C0MUX_SEL_SM0 | WB_CSR_I2C1MUX_SEL_SM1);
	}

	void QL_FFE_GiveControlOfI2CBus()
	{
		/*
		 * this function ensures that the SM0/SM1 give up the control of the
		 * I2C0/I2C1 bus and M4 can now use the I2C Bus directly
		 *
		 * Configure the WB MUX select to allow WB Master(M4) to become the I2C Master.
		 * Otherwise, the SM will be the I2C Master
		 *
		 */
		EXT_REGS_FFE->CSR = (WB_CSR_I2C0MUX_SEL_WBMASTER | WB_CSR_I2C1MUX_SEL_WBMASTER);
	}


	/* configure the necessary clocks and power domains for the FFE to run */
	/* KK: this should be a part of the HAL FFE API, can also be made configurable, according to FFE speeds required */
	// internal use
	void QL_FFE_ConfigClockPower()
	{
		/*
		 * Init the Clocks and Power Domains required for FFE to run.
		 */
#if 0
		//enable FFE power & clock domain
		PMU->FFE_PWR_MODE_CFG = 0x0;
		PMU->FFE_PD_SRC_MASK_N = 0x0;
		PMU->FFE_WU_SRC_MASK_N = 0x0;

		//wake up FFE
		PMU->FFE_FB_PF_SW_WU = 0x1;
		//check if FFE is in Active mode
		while(!(PMU->FFE_STATUS & 0x1));
#endif
		S3x_Clk_Enable(S3X_FFE_X4_CLK);
		S3x_Clk_Enable(S3X_FFE_X1_CLK);
		S3x_Clk_Enable(S3X_FFE_CLK);
		S3x_Clk_Enable(S3X_FFE_CLK);
		S3x_Clk_Enable(S3X_SPT_CLK);

		//	PMU->FFE_PWR_MODE_CFG = 0x0;
		//	PMU->FFE_PD_SRC_MASK_N = 0x0;
		//	PMU->FFE_WU_SRC_MASK_N = 0x0;
		//
		//	//wake up FFE
		//	PMU->FFE_FB_PF_SW_WU = 0x1;
		//	//check if FFE is in Active mode
		//	while(!(PMU->FFE_STATUS & 0x1));
		//
		//	//Enable C08-X4 clock
		//	CRU->CLK_CTRL_C_0 = 0x204;
		//	CRU->CLK_SWITCH_FOR_C = 0;
		//	CRU->C01_CLK_GATE = 0x29F;
		//
		//	//Enable C08-X1 clock
		//	CRU->C08_X1_CLK_GATE = 0xF;
		//	CRU->C08_X4_CLK_GATE = 0x1;
		//
		//	CRU->FFE_SW_RESET = 0x3;
		//
		//	CRU->FFE_SW_RESET = 0x0;

	}

	/* reset the SPT Configuration, used before re-initing FFE parameters */
	void QL_FFE_ResetSPT()
	{
		/* clear the SPT - both SPT enable, as well as FFE Period */
		SPT->SPT_CFG = 0x0;
	}


	//TODO : revisit this
	//Filling up of export section will depend upon project so this
	//will have to change every time with project

	void QL_FFE_InitExportSection()
	{
		//unsigned int val;
		memset(ExportSectionBase, 0, sizeof(struct QL_ExportSection));

		QL_FFE_Sensors[QL_FFE_SENSOR_ID_ACCEL1-1].SensorId 				= QL_FFE_SENSOR_ID_ACCEL1;
		QL_FFE_Sensors[QL_FFE_SENSOR_ID_ACCEL2-1].SensorId 				= QL_FFE_SENSOR_ID_ACCEL2;
		QL_FFE_Sensors[QL_FFE_SENSOR_ID_DOUBLETAP-1].SensorId 			= QL_FFE_SENSOR_ID_DOUBLETAP;
		QL_FFE_Sensors[QL_FFE_SENSOR_ID_PCG-1].SensorId 				= QL_FFE_SENSOR_ID_PCG;
		QL_FFE_Sensors[QL_FFE_SENSOR_ID_GYRO-1].SensorId 				= QL_FFE_SENSOR_ID_GYRO;
		QL_FFE_Sensors[QL_FFE_SENSOR_ID_MAG-1].SensorId 				= QL_FFE_SENSOR_ID_MAG;
        QL_FFE_Sensors[QL_SENSI_ML_ID_APP1-1].SensorId 		        	= QL_SENSI_ML_ID_APP1;
		/* KK: sensor IDs are only for M4 reference, it does not matter for FFE */
		/* we save the local copies of the data for M4 side manipulations */

		// update the FFE export section, with sensor IDs correctly set.
		// we do not know if FFE uses them or not, but in any case we should be doing this from M4.
		HAL_FFE_WriteMem(ExportSectionBase, sizeof(QL_FFE_Sensors), (unsigned int*)QL_FFE_Sensors);
	}


#ifdef FFE_USE_PACKET_FIFO
	static void vFFE_IPC_TaskHandler(void *arg)
	{
		int ffe_sensor_index, i, exittask = 0;
		unsigned int ffe_sensor_id;
		struct controlPkt cpkt;
		struct QL_SensorEventInfo eventinfo;
		QL_Status ret;
		BaseType_t qret;

        wait_ffe_fpga_load();
            
		while (!exittask)
		{
			memset(&cpkt, 0, sizeof(cpkt));
			qret = xQueueReceive(xHandleQueueFFEIPCHandler, &cpkt, FFE_IPC_MGSQ_WAIT_TIME);
			QL_ASSERT(qret == pdTRUE);
			ffe_sensor_id = CPKT_GET_FFE_SENSOR_ID(cpkt);
			for (i = 0 ; i < QL_FFE_MAX_SENSORS ; i++)
			{
				if (QL_FFE_Sensors[i].SensorId == ffe_sensor_id)
				{
					ffe_sensor_index = i;
					break;
				}
			}
			eventinfo.data = CPKT_GET_MEMPTR(cpkt);
			eventinfo.numpkt = CPKT_GET_NUMPKTS(cpkt);

			QL_TRACE_FFE_DEBUG("ffe_sensor_id = 0x%x, ffe_sensor_index = 0x%x\n", ffe_sensor_id, ffe_sensor_index);
			QL_TRACE_FFE_DEBUG("eventinfo.data = 0x%x, eventinfo.numpkt = 0x%x\n", eventinfo.data, eventinfo.numpkt);

			QL_ASSERT(ffe_sensor_index != -1);
			QL_ASSERT(cbinfo[ffe_sensor_index].event_callback);

			QL_TRACE_FFE_DEBUG("event_callback = 0x%x, cookie = %x\n", cbinfo[ffe_sensor_index].event_callback, cbinfo[ffe_sensor_index].cookie);

			if (eventinfo.numpkt > 1)
			{
				eventinfo.event = QL_SENSOR_EVENT_BATCH;
			}
			else
			{
				eventinfo.event = QL_SENSOR_EVENT_DATA;
			}

			ret = cbinfo[ffe_sensor_index].event_callback(cbinfo[ffe_sensor_index].cookie, &eventinfo);
			if(ret != QL_STATUS_OK)
			{
				QL_TRACE_FFE_ERROR("Callback to sensorid %d returned error\n", ffe_sensor_id);
			}
		}
	}


	void QL_FFE_FIFO_ISR(void)
	{
		static struct controlPkt cpkt;
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		int numpkts;

		memset(&cpkt, 0, sizeof(cpkt));
		numpkts = FIFOx_GET_POP_COUNT(FFE_IPC_FIFO_ID)/(sizeof(cpkt)/sizeof(unsigned int));	//TODO : revist - calculate the size per packet properly
		while (numpkts--)
		{
			cpkt.data[0] = FIFOx_DATA_READ(FFE_IPC_FIFO_ID);
			cpkt.data[1] = FIFOx_DATA_READ(FFE_IPC_FIFO_ID);
			if(xQueueSendFromISR(xHandleQueueFFEIPCHandler, &cpkt, &xHigherPriorityTaskWoken ) != pdPASS)
			{
				QL_TRACE_FFE_ERROR("xQueueSendFromISR\n");
			}
		}
		//TODO : clear/re-enable only the interrupt which is being used rather that 2 int given below
		INTR_CTRL->FFE_INTR |= (FFE0_1_INTR_DETECT | FFE0_0_INTR_DETECT);
		INTR_CTRL->FFE_INTR_EN_M4 |= (FFE0_1_INTR_EN_M4 | FFE1_0_INTR_EN_M4);

		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}


	/* configure the Packet FIFO for using this as comm between FFE and M4 */
	void QL_FFE_ConfigPacketFIFO()
	{
		xFifoConfig.eFifoID = FFE_IPC_FIFO_ID;
		xFifoConfig.eSrc = FIFO_SRC_FFE0;
		xFifoConfig.eDest = FIFO_DEST_M4;

		if(HAL_FIFO_Init(xFifoConfig) != HAL_OK)
		{
			QL_TRACE_FFE_ERROR("HAL_FIFO_Init Failed\r\n");
			QL_ASSERT(0);
		}
		//configuring required interrupts
		//TODO : Revisit to check whether all these interrupts are required
		EXT_REGS_FFE->INTERRUPT_EN = ( 	FFE_INTR_PKFB_OVF_INTR 		|
				FFE_INTR_I2C_MS_0_ERROR 	|
				FFE_INTR_I2C_MS_1_ERROR 	|
				FFE_INTR_FFE0_SM0_OVERRUN 	|
				FFE_INTR_FFE0_SM1_OVERRUN 	|
				FFE_INTR_FFE0_OVERRUN 		|
				FFE_INTR_SM_MULT_WR_INTR	);

		INTR_CTRL->OTHER_INTR_EN_M4 |= FFE0_INTR_OTHERS_EN_M4;

		INTR_CTRL->FFE_INTR |= (FFE0_1_INTR_DETECT | FFE0_0_INTR_DETECT);
		//Enable the FFE interrupt
		INTR_CTRL->FFE_INTR_EN_M4 |= (FFE0_1_INTR_EN_M4 | FFE1_0_INTR_EN_M4);

		//Clear the interrupt
		INTR_CTRL->OTHER_INTR |= (FFE0_INTR_OTHERS_EN_M4);

		//Enable the FFE other interrupt
		INTR_CTRL->OTHER_INTR_EN_M4 |= FFE0_INTR_OTHERS_EN_M4;
		NVIC_ClearPendingIRQ(Ffe0Msg_IRQn);
		NVIC_SetPriority(Ffe0Msg_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
		NVIC_EnableIRQ(Ffe0Msg_IRQn);
		NVIC_ClearPendingIRQ(Ffe0_IRQn);
		NVIC_SetPriority(Ffe0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
		NVIC_EnableIRQ(Ffe0_IRQn);

	}


	portBASE_TYPE xInitFFEIPCTask()
	{
		portBASE_TYPE ret;

		/* create message queue for ffe ipc lib Task */
		xHandleQueueFFEIPCHandler = xQueueCreate( FFE_IPC_TASK_MSGQ_LENGTH, sizeof(struct controlPkt) );
		QL_ASSERT(xHandleQueueFFEIPCHandler);
        vQueueAddToRegistry( xHandleQueueFFEIPCHandler , "Ffe_IpcQ" );

		/* create the ffe ipc lib task */
		ret = xTaskCreate(vFFE_IPC_TaskHandler, "vFFE_IPC_TaskHandler", FFE_IPC_TASK_STACK_SIZE, NULL, FFE_IPC_TASK_PRIORITY,
				&xHandleTaskFFEIPCHandler);
		QL_ASSERT(ret == pdPASS);

		return ret;
	}

#else
	static void vFFE_IPC_TaskHandler(void *arg)
	{
		int exittask = 0;
		struct xFFEIPCMSGPacket ffe_msg;
		BaseType_t qret;
		//			int ffe_sensor_index, i, exittask = 0;
		//			unsigned int ffe_sensor_id;
		//			struct controlPkt cpkt;
		//			struct QL_SensorEventInfo eventinfo;
		//			QL_Status ret;
		unsigned int sensorid;
		QL_SensorEventCallback_t sensorCallback;
		QL_SAL_SensorHandle_t sensorHandle;
		struct QL_SensorEventInfo event_info;
		void* cookie;
		QL_Status ret;

        wait_ffe_fpga_load();
        
		while (!exittask)
		{
			memset(&ffe_msg, 0, sizeof(ffe_msg));
			qret = xQueueReceive(xHandleQueueFFEIPCHandler, &ffe_msg, FFE_IPC_MGSQ_WAIT_TIME);
			QL_ASSERT(qret == pdTRUE);

			/*
			 * note that multiple events can be reported in a single message to this task
			 * and we need to handle each of them in different ways, hence the if, and not switch.
			 */
#if 0
			if(ffe_msg.event & INT_CMD_ACCEL_BATCH_PERIOD_EXPIRE)
			{
				QL_TRACE_FFE_DEBUG("FFE: ACCEL BATCH Event Message\n");
				// fire callback registered with the sensor driver.
				sensorid = QL_SAL_SENSOR_ID_ACCEL;
				ret = QL_SAL_SensorOpen(&sensorHandle, sensorid);
				QL_ASSERT(ret == QL_STATUS_OK);

				ret = QL_SAL_SensorIoctl(sensorHandle, QL_SAL_IOCTL_GET_CALLBACK, (void *)&sensorCallback);
				QL_ASSERT(ret == QL_STATUS_OK);

				if(sensorCallback != NULL)
				{
					cookie = (void *)0xFEED;
					event_info.event = QL_SENSOR_EVENT_BATCH;
					event_info.data = (void *)(*(unsigned int *)BATCH_BUFFER_INFO_ADDR_ACCEL);
					event_info.numpkt = *(unsigned int *)(BATCH_BUFFER_INFO_ADDR_ACCEL + 4);
					sensorCallback(cookie, &event_info);
				}

				ret = QL_SAL_SensorClose(sensorHandle);
				QL_ASSERT(ret == QL_STATUS_OK);
			}

			if(ffe_msg.event & INT_CMD_ACCEL1_BATCH_PERIOD_EXPIRE)
			{
				QL_TRACE_FFE_DEBUG("FFE:ACCEL2 BATCH Event Message\n");
				// fire callback registered with the sensor driver.
				sensorid = QL_SAL_SENSOR_ID_ACCEL2;
				ret = QL_SAL_SensorOpen(&sensorHandle, sensorid);
				QL_ASSERT(ret == QL_STATUS_OK);

				ret = QL_SAL_SensorIoctl(sensorHandle, QL_SAL_IOCTL_GET_CALLBACK, (void *)&sensorCallback);
				QL_ASSERT(ret == QL_STATUS_OK);

				if(sensorCallback != NULL)
				{
					cookie = (void *)0xFEED;
					event_info.event = QL_SENSOR_EVENT_BATCH;
					event_info.data = (void *)(*(unsigned int *)BATCH_BUFFER_INFO_ADDR_ACCEL1);
					event_info.numpkt = *(unsigned int *)(BATCH_BUFFER_INFO_ADDR_ACCEL1 + 4);
					sensorCallback(cookie, &event_info);
				}

				ret = QL_SAL_SensorClose(sensorHandle);
				QL_ASSERT(ret == QL_STATUS_OK);
			}

			if(ffe_msg.event & INT_CMD_GYRO_BATCH_PERIOD_EXPIRE)
			{
				QL_TRACE_FFE_DEBUG("FFE:GYRO BATCH Event Message\n");
				// fire callback registered with the sensor driver.
				sensorid = QL_SAL_SENSOR_ID_GYRO;
				ret = QL_SAL_SensorOpen(&sensorHandle, sensorid);
				QL_ASSERT(ret == QL_STATUS_OK);

				ret = QL_SAL_SensorIoctl(sensorHandle, QL_SAL_IOCTL_GET_CALLBACK, (void *)&sensorCallback);
				QL_ASSERT(ret == QL_STATUS_OK);

				if(sensorCallback != NULL)
				{
					cookie = (void *)0xFEED;
					event_info.event = QL_SENSOR_EVENT_BATCH;
					event_info.data = (void *)(*(unsigned int *)BATCH_BUFFER_INFO_ADDR_GYRO);
					event_info.numpkt = *(unsigned int *)(BATCH_BUFFER_INFO_ADDR_GYRO + 4);
					sensorCallback(cookie, &event_info);
				}

				ret = QL_SAL_SensorClose(sensorHandle);
				QL_ASSERT(ret == QL_STATUS_OK);
			}

			if(ffe_msg.event & INT_CMD_MAG_BATCH_PERIOD_EXPIRE)
			{
				QL_TRACE_FFE_DEBUG("FFE:MAG BATCH Event Message\n");
				// fire callback registered with the sensor driver.
				sensorid = QL_SAL_SENSOR_ID_MAG;
				ret = QL_SAL_SensorOpen(&sensorHandle, sensorid);
				QL_ASSERT(ret == QL_STATUS_OK);

				ret = QL_SAL_SensorIoctl(sensorHandle, QL_SAL_IOCTL_GET_CALLBACK, (void *)&sensorCallback);
				QL_ASSERT(ret == QL_STATUS_OK);

				if(sensorCallback != NULL)
				{
					cookie = (void *)0xFEED;
					event_info.event = QL_SENSOR_EVENT_BATCH;
					event_info.data = (void *)(*(unsigned int *)BATCH_BUFFER_INFO_ADDR_MAG);
					event_info.numpkt = *(unsigned int *)(BATCH_BUFFER_INFO_ADDR_MAG + 4);
					sensorCallback(cookie, &event_info);
				}

				ret = QL_SAL_SensorClose(sensorHandle);
				QL_ASSERT(ret == QL_STATUS_OK);
			}
#endif
			if(ffe_msg.event & INT_CMD_DOUBLE_TAP_EVENT)
			{
				QL_TRACE_FFE_DEBUG("FFE: Double Tap Event Message\n");
				// fire callback registered with the sensor driver.
				sensorid = QL_SAL_SENSOR_ID_DOUBLE_TAP;
				ret = QL_SAL_SensorOpen(&sensorHandle, sensorid);
				QL_ASSERT(ret == QL_STATUS_OK);

				ret = QL_SAL_SensorIoctl(sensorHandle, QL_SAL_IOCTL_GET_CALLBACK, (void *)&sensorCallback);
				QL_ASSERT(ret == QL_STATUS_OK);

				if(sensorCallback != NULL)
				{
					cookie = (void *)0xFEED;
					event_info.data = (void *)((*(unsigned int *)LIVE_SENSOR_DATA_DT_ADDR) >> 8); // read the time stamp of 4 bytes from SRAM.
					sensorCallback(cookie, &event_info);
				}

				ret = QL_SAL_SensorClose(sensorHandle);
				QL_ASSERT(ret == QL_STATUS_OK);
			}
            if(ffe_msg.event & INT_CMD_SENSIML_BATCH_PERIOD_EXPIRE)
			{
				//QL_TRACE_FFE_DEBUG("FFE: Sensiml batch period expire Event Message\n");
				// fire callback registered with the sensor driver.
				sensorid = QL_SAL_SENSOR_ID_SENSIML_APP1;
				ret = QL_SAL_SensorOpen(&sensorHandle, sensorid);
				QL_ASSERT(ret == QL_STATUS_OK);

				ret = QL_SAL_SensorIoctl(sensorHandle, QL_SAL_IOCTL_GET_CALLBACK, (void *)&sensorCallback);
				QL_ASSERT(ret == QL_STATUS_OK);

				if(sensorCallback != NULL)
				{
					cookie = (void *)0xFEED;
					event_info.event = QL_SENSOR_EVENT_BATCH;
					event_info.data = (void *)(*(unsigned int *)BATCH_BUFFER_INFO_ADDR_SENSIML_APP);
                                        event_info.numpkt = *(unsigned int *)(BATCH_BUFFER_INFO_ADDR_SENSIML_APP + 4);
                                        sensorCallback(cookie, &event_info);
				}

				ret = QL_SAL_SensorClose(sensorHandle);
				QL_ASSERT(ret == QL_STATUS_OK);
			}
                        
#if 0
			if(ffe_msg.event & INT_CMD_PCG_EVENT)
			{
				QL_TRACE_FFE_DEBUG("FFE: PCG Event Message\n");
				// fire callback registered with the sensor driver.
				sensorid = QL_SAL_SENSOR_ID_PCG;
				ret = QL_SAL_SensorOpen(&sensorHandle, sensorid);
				QL_ASSERT(ret == QL_STATUS_OK);

				ret = QL_SAL_SensorIoctl(sensorHandle, QL_SAL_IOCTL_GET_CALLBACK, (void *)&sensorCallback);
				QL_ASSERT(ret == QL_STATUS_OK);

				if(sensorCallback != NULL)
				{
					pcgdata_t pcgDataFromSRAM;
					memcpy((void *)&pcgDataFromSRAM, (void *)LIVE_SENSOR_DATA_PCG_ADDR, sizeof(pcgDataFromSRAM));
					cookie = (void *)0xFEED;
					event_info.data = (void *)&pcgDataFromSRAM;
					sensorCallback(cookie, &event_info);
				}

				ret = QL_SAL_SensorClose(sensorHandle);
				QL_ASSERT(ret == QL_STATUS_OK);
			}

#ifdef USE_INTERRUPTS_FOR_LIVE_DATA
			if(ffe_msg.event & INT_CMD_ACCEL_LIVE_DATA_AVAILABLE)
			{
				if(callback_Accel == NULL)
				{
					sensorid = QL_SAL_SENSOR_ID_ACCEL;
					ret = QL_SAL_SensorOpen(&sensorHandle, sensorid);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(sensorHandle, QL_SAL_IOCTL_GET_CALLBACK, (void *)&callback_Accel);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorClose(sensorHandle);
					QL_ASSERT(ret == QL_STATUS_OK);
				}

				QL_TRACE_FFE_DEBUG("FFE: ACCEL DATA Message\n");
				// fire callback registered with the sensor driver.

				if(callback_Accel != NULL)
				{
					//Accel pcgDataFromSRAM;
					memcpy((void *)&accelData, (void *)LIVE_SENSOR_DATA_ACCEL_ADDR, sizeof(accelData));
					cookie = (void *)0xFEED;
					event_info.event = QL_SENSOR_EVENT_DATA;
					event_info.data = (void *)&accelData;
					callback_Accel(cookie, &event_info);
				}
			}

			if(ffe_msg.event & INT_CMD_ACCEL1_LIVE_DATA_AVAILABLE)
			{
				if(callback_Accel1 == NULL)
				{
					sensorid = QL_SAL_SENSOR_ID_ACCEL2;
					ret = QL_SAL_SensorOpen(&sensorHandle, sensorid);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(sensorHandle, QL_SAL_IOCTL_GET_CALLBACK, (void *)&callback_Accel1);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorClose(sensorHandle);
					QL_ASSERT(ret == QL_STATUS_OK);
				}

				QL_TRACE_FFE_DEBUG("FFE: ACCEL2 DATA Message\n");
				// fire callback registered with the sensor driver.

				if(callback_Accel1 != NULL)
				{
					//Accel pcgDataFromSRAM;
					memcpy((void *)&accelData2, (void *)LIVE_SENSOR_DATA_ACCEL2_ADDR, sizeof(accelData));
					cookie = (void *)0xFEED;
					event_info.event = QL_SENSOR_EVENT_DATA;
					event_info.data = (void *)&accelData2;
					callback_Accel1(cookie, &event_info);
				}
			}

			if(ffe_msg.event & INT_CMD_GYRO_LIVE_DATA_AVAILABLE)
			{
				if(callback_Gyro == NULL)
				{
					sensorid = QL_SAL_SENSOR_ID_GYRO;
					ret = QL_SAL_SensorOpen(&sensorHandle, sensorid);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(sensorHandle, QL_SAL_IOCTL_GET_CALLBACK, (void *)&callback_Gyro);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorClose(sensorHandle);
					QL_ASSERT(ret == QL_STATUS_OK);
				}

				QL_TRACE_FFE_DEBUG("FFE: GYRO DATA Message\n");
				// fire callback registered with the sensor driver.

				if(callback_Gyro != NULL)
				{
					//Accel pcgDataFromSRAM;
					memcpy((void *)&gyroData, (void *)LIVE_SENSOR_DATA_GYRO_ADDR, sizeof(gyroData));
					cookie = (void *)0xFEED;
					event_info.event = QL_SENSOR_EVENT_DATA;
					event_info.data = (void *)&gyroData;
					callback_Gyro(cookie, &event_info);
				}
			}

			if(ffe_msg.event & INT_CMD_MAG_LIVE_DATA_AVAILABLE)
			{
				if(callback_Mag == NULL)
				{
					sensorid = QL_SAL_SENSOR_ID_MAG;
					ret = QL_SAL_SensorOpen(&sensorHandle, sensorid);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorIoctl(sensorHandle, QL_SAL_IOCTL_GET_CALLBACK, (void *)&callback_Mag);
					QL_ASSERT(ret == QL_STATUS_OK);

					ret = QL_SAL_SensorClose(sensorHandle);
					QL_ASSERT(ret == QL_STATUS_OK);
				}

				QL_TRACE_FFE_DEBUG("FFE: MAG DATA Message\n");
				// fire callback registered with the sensor driver.

				if(callback_Mag != NULL)
				{
					//Accel pcgDataFromSRAM;
					memcpy((void *)&magData, (void *)LIVE_SENSOR_DATA_MAG_ADDR, sizeof(magData));
					cookie = (void *)0xFEED;
					event_info.event = QL_SENSOR_EVENT_DATA;
					event_info.data = (void *)&magData;
					callback_Mag(cookie, &event_info);
				}
			}

#endif // USE_INTERRUPTS_FOR_LIVE_DATA
#endif
		}
	}

	void QL_FFE_ConfigInterrupts()
	{
		/* enable the following bits being set in the INTR controller */
		INTR_CTRL->FFE_INTR |= (
				FFE0_7_INTR_DETECT |
				FFE0_6_INTR_DETECT |
				FFE0_5_INTR_DETECT |
				FFE0_4_INTR_DETECT |
				FFE0_3_INTR_DETECT |
				FFE0_2_INTR_DETECT |
				FFE0_1_INTR_DETECT |
				FFE0_0_INTR_DETECT
		);

		/* enable the FFE to interrupt M4 for the following interrupts */
		INTR_CTRL->FFE_INTR_EN_M4 |= (
				FFE0_7_INTR_EN_M4 |
				FFE0_6_INTR_EN_M4 |
				FFE0_5_INTR_EN_M4 |
				FFE0_4_INTR_EN_M4 |
				FFE0_3_INTR_EN_M4 |
				FFE0_2_INTR_EN_M4 |
				FFE0_1_INTR_EN_M4 |
				FFE0_0_INTR_EN_M4
		);

		/* enable the FFE0 MSG INTR at the NVIC */
		NVIC_ClearPendingIRQ(Ffe0Msg_IRQn);
		NVIC_SetPriority(Ffe0Msg_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
		NVIC_EnableIRQ(Ffe0Msg_IRQn);
	}


	void QL_FFE_DIRECT_ISR(void)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		struct xFFEIPCMSGPacket ffe_msg;
		memset(&ffe_msg, 0, sizeof(ffe_msg));


		/* check direct masks */
		/* if indirect cmd is set, check indirect cmd masks */
		unsigned int directInterruptRegVal = 0x0;
		unsigned int interruptsHandled = 0x0;

		/* add code to read from register here.  */
		directInterruptRegVal = INTR_CTRL->FFE_INTR;

		//printf("ISR: 0x%x\n",directInterruptRegVal);
		if(directInterruptRegVal & INT_COMMAND)
		{
			interruptsHandled |= INT_COMMAND;

			// handle the indirect interrupts -> read from export section location for the interrupt status.
			unsigned int indirectInterruptStatusRegVal = 0x0;
			GET_PARAM_VAL_FROM_FFE_X(FFE_SECTION_BASE_ADDR_GLOBAL, struct QL_ExportSection, ffe_interrupt_cmd, &indirectInterruptStatusRegVal);

			//printf("indirectInterruptStatusRegVal: 0x%x\n",indirectInterruptStatusRegVal);

//			if(indirectInterruptStatusRegVal & INT_CMD_ACCEL_BATCH_PERIOD_EXPIRE)
//			{
//
//			}
//
//			if(indirectInterruptStatusRegVal & INT_CMD_ACCEL1_BATCH_PERIOD_EXPIRE)
//			{
//
//			}
//
//			if(indirectInterruptStatusRegVal & INT_CMD_GYRO_BATCH_PERIOD_EXPIRE)
//			{
//
//			}
//
//			if(indirectInterruptStatusRegVal & INT_CMD_MAG_BATCH_PERIOD_EXPIRE)
//			{
//
//			}
//
//			if(indirectInterruptStatusRegVal & INT_CMD_DOUBLE_TAP_EVENT)
//			{
//
//			}
//
//			if(indirectInterruptStatusRegVal & INT_CMD_PCG_EVENT)
//			{
//
//			}

			ffe_msg.event = indirectInterruptStatusRegVal; // there might be multiple events reported in the same ISR now !!

			if(xQueueSendFromISR(xHandleQueueFFEIPCHandler, &ffe_msg, &xHigherPriorityTaskWoken ) != pdPASS)
			{
				QL_TRACE_FFE_ERROR("xQueueSendFromISR FAILED.\n");
			}

		}

		// clear the handled interrupts:
		INTR_CTRL->FFE_INTR |= interruptsHandled;
		//INTR_CTRL->FFE_INTR_EN_M4 |= (FFE0_1_INTR_EN_M4 | FFE1_0_INTR_EN_M4); // re-enable is required only if we are disabling the interrupts

		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}



	portBASE_TYPE xInitFFEIPCTask()
	{
		portBASE_TYPE ret;

		/* create message queue for ffe ipc lib Task */
		xHandleQueueFFEIPCHandler = xQueueCreate( FFE_IPC_TASK_MSGQ_LENGTH, sizeof(struct xFFEIPCMSGPacket) );
		QL_ASSERT(xHandleQueueFFEIPCHandler);
        vQueueAddToRegistry( xHandleQueueFFEIPCHandler , "Ffe_IpcQ" );

		/* create the ffe ipc lib task */
		ret = xTaskCreate(	vFFE_IPC_TaskHandler,
				"vFFE_IPC_TaskHandler",
				FFE_IPC_TASK_STACK_SIZE,
				NULL,
				FFE_IPC_TASK_PRIORITY,
				&xHandleTaskFFEIPCHandler);

		QL_ASSERT(ret == pdPASS);

		return ret;
	}
#endif // FFE_USE_PACKET_FIFO



int ffe_counts;
	void QL_FFE0MSG_ISR(void)
	{
        /* odd means inside the irq even means done */
        ffe_counts++;
#ifdef FFE_USE_PACKET_FIFO
		QL_FFE_FIFO_ISR();
#else
		QL_FFE_DIRECT_ISR();
#endif // FFE_USE_PACKET_FIFO
        ffe_counts++;

	}

	/* parts needed for the FFE IPC Library to be run
	 *
	 * PAD Configuration ( not sure what exactly this does, but is required, and is board specific )
	 *
	 * CLOCK and POWER Configuration for the FFE to run.
	 *
	 * I2C BUS Configuration for SM0/SM1
	 *
	 * SPT Configuration for FFE to run.
	 *
	 * Export Section Configuration (this is project dependent, as the Export Section will be designed according to requirements)
	 *
	 * If Packet FIFO is used for sensor data/alorithm data from FFE to M4:
	 * 		FIFO CONFIGURATION (HW PacketFIFO as well as ExportSection for FIFO)
	 * 		Packet FIFO ISR Handler (picks up data using control packet in ISR, and data in SRAM and pushes message to FFE IPC Task)
	 * 		TASK and QUEUE (for processing the messages from Packet FIFO ISR Handler)
	 *
	 *
	 */


	/* Interfaces to the FFE functionality required from the external world (M4)
	 *
	 * This depends on the requirements, but in general we would need:
	 * FFE Enable/Disable, Tick Period Control
	 * I2C Bus Control handover to SM ( from this lib ), taking control from SM is via HAL_I2C/HAL_WB -> *This should be added to HAL_WB ?*
	 *
	 *
	 * The below are very much project dependent, but can be made more generic as these would be required for any sensor hub application
	 * FFE Algorithm Enable/Disable, Period Control (specific to project)
	 * * Algo Specific Configuration via Export Section -> includes algo callback, mem region, perio, attributes etc.
	 * e.g. PCG, DT etc.
	 * FFE SensorTask Enable/Disable, Period Controle (specific to project)
	 * * Sensor Specific Configuration via Export Section -> includes sensor callback, mem region, period, attributes etc.
	 * * e.g. Accel, Gyro, Mag etc.
	 *
	 *
	 *
	 *
	 */

	QL_Status ffe_ipc_library_init(void)
	{
		/* common config always required for FFE operation */
		QL_FFE_ConfigClockPower();
		QL_FFE_ConfigureSMPads();
                
        QL_FFE_GiveControlOfI2CBus();

        // i2c bus config and m4 takes ownership of bus
        I2C_Config xI2CConfig;

        // Configure I2C frequency to 400KHz.(confirm FFE clock freq)
#if QAI_CHILKAT
        xI2CConfig.eI2CFreq = I2C_200KHZ;//ChilKat does not support high frequency
#else        
        xI2CConfig.eI2CFreq = I2C_400KHZ;
#endif        
        xI2CConfig.eI2CInt = I2C_DISABLE;
        xI2CConfig.ucI2Cn = 0;					/* SM0 == I2C0 */

        HAL_I2C_Init(xI2CConfig);
                
        QL_FFE_TakeControlOfI2CBus();

		/* this will be project specific */
		QL_FFE_InitExportSection();

		/* reset the SPT timer, so FFE is disabled unless explicitly enabled via the FFE OPS */
		QL_FFE_ResetSPT();

		/* init the FFE IPC Task to handle the messages from FFE ISR */
		if(pdPASS != xInitFFEIPCTask())
		{
			return QL_STATUS_ERROR;
		}

#ifdef FFE_USE_PACKET_FIFO

		/* configures Packet FIFO, as well as interrupts */
		QL_FFE_ConfigPacketFIFO();

#else // #ifdef FFE_USE_PACKET_FIFO

		/* configures the FFE interrupts */
		QL_FFE_ConfigInterrupts();

#endif // #ifdef FFE_USE_PACKET_FIFO

		return QL_STATUS_OK;
	}

    //to be done after loading FFE in a task from flash
  	int ffe_ipc_library_init_2(void)
	{

        QL_FFE_TakeControlOfI2CBus();

		/* this will be project specific */
		QL_FFE_InitExportSection();

		/* reset the SPT timer, so FFE is disabled unless explicitly enabled via the FFE OPS */
		QL_FFE_ResetSPT();

#ifdef FFE_USE_PACKET_FIFO

		/* configures Packet FIFO, as well as interrupts */
		QL_FFE_ConfigPacketFIFO();

#else // #ifdef FFE_USE_PACKET_FIFO

		/* configures the FFE interrupts */
		QL_FFE_ConfigInterrupts();

#endif // #ifdef FFE_USE_PACKET_FIFO

		return QL_STATUS_OK;
	}

#endif /* FFE_DRIVERS */
