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
 *    File   : eoss3_hal_ffe.c
 *    Purpose: This file contains HAL API for FFE
 *                                                          
 *=========================================================*/

/* Standard includes. */
#include "Fw_global_config.h"

#include <stdio.h>
#include <string.h>

#include "eoss3_dev.h"
#include "eoss3_hal_i2c.h"
#include "eoss3_hal_ffe.h"
//#include "eoss3_hal_rcc.h"
//#include "eoss3_hal_power.h"
//#include "eoss3_hal_clock.h"
#include "s3x_clock_hal.h"
#include "s3x_clock.h"
#include "eoss3_hal_pad_config.h"
#include "eoss3_hal_pkfb.h"
#include <test_types.h>
#include "Fw_global_config.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

/// @cond HAL_FFE_GLOBAL_VAR

FIFO_Config xFifoConfig;

UINT8_t ucFFE2M4Fifo0 = 1; //to initialize the Fifo from FFE to M4, set it 1
UINT8_t ucFFE2M4Fifo1 = 0;


volatile UINT32_t *FFE_CM  				= (volatile UINT32_t*)CM_BASE;
volatile UINT32_t *FFE_DM0 				= (volatile UINT32_t*)DM0_BASE;
volatile UINT32_t *FFE1_DM 				= (volatile UINT32_t*)DM_FFE1_BASE;
volatile UINT32_t *FFE_SM0 				= (volatile UINT32_t*)SM0_BASE;
volatile UINT32_t *FFE_SM1 				= (volatile UINT32_t*)SM1_BASE;
volatile UINT32_t *FFE_CFG				= (volatile UINT32_t*)FFE_CFG_BASE;
volatile UINT32_t *FFE_CFG_BYTE			        = (volatile UINT32_t*)FFE_CFG_BASE;


static xSensor_alg_config_1 alg_config_1[ALGS_COUNT];
static xSensor_alg_config_2 alg_config_2[ALGS_COUNT];
static xExport_var_t	expo_var[EXPORT_VAR_COUNT];
static UINT8_t ucSensId[32];
static UINT32_t uiAlgAddr[64];
static UINT16_t usFFE_Mailbox1 = 0;
static UINT16_t usFFE_Mailbox2 = 0;
static UINT32_t usFFE_ticks_ms = 0;

FIFO_Config xFifoCfg[4] = {{FIFO0, FIFO_SRC_M4, FIFO_DEST_AP},
							{FIFO1, FIFO_SRC_FFE0, FIFO_DEST_M4},
							{FIFO2, FIFO_SRC_FFE0, FIFO_DEST_AP},
							{FIFO8K, FIFO_SRC_FFE0, FIFO_DEST_M4}};

#define SET_ALG_PARAM_VAL_IN_EXPO_SEC(_id_, _alg_type_, _param_name_, _u32_param_value_)	\
do {												\
	uint32_t __val__ = _u32_param_value_;							\
	HAL_FFE_WriteMem((FFE_DM0 + uiAlgAddr[_id_] + (offsetof(_alg_type_, _param_name_)/4)), sizeof(__val__), &__val__);\
} while (0)

/// @endcond

/// @cond HAL_FFE_INTERNAL_API
/*!
 * \fn          UINT32_t get_algsenabled_cnt(UINT32_t ulMailboxVal)
 * \brief       Computes the total number of algorithms enabled based on mailbox value
 * \param       ulMailboxVal -- Mailbox value
 * \return      Alg count
 */
static UINT32_t FFE_GetAlgsEnabledCnt(UINT32_t ulMailboxVal)
{
	UINT16_t usIndex, usCnt = 0;

	for(usIndex = 0; usIndex < 32; usIndex++)
	{
		if((ulMailboxVal % 2) != 0)
		{
			usCnt++;
		}

		ulMailboxVal = ulMailboxVal >> 1;
	}

	return usCnt;
}

/*!
 * \fn 	        void FFE_Compute_New_Samplerate(UINT16_t usMailbox)
 * \brief       Based on mailbox value, it will check if any other algorithm is enabled,
 *              if enabled then change the FFE sample time to the next max sample rate. *
 * \param       usMailbox -- Mailbox bit.
 */
static void FFE_Compute_New_Samplerate(UINT16_t usMailbox)
{
	UINT8_t ucBitpos = 0;

	UINT32_t ulAlgCnt = 0;
	xSensor_alg_config_1 *config1 = NULL;
	xSensor_alg_config_2 *config2 = NULL;

	ulAlgCnt = FFE_GetAlgsEnabledCnt(usMailbox);

	do
	{
		ucBitpos = FFE_CheckBitEnable(usMailbox);

		if(FFE_Get_AlgConfig(ucSensId[ucBitpos],&config1,&config2) == HAL_OK)
		{
			if(config1)
			{
				//check if the dependent sensor rate is greater than the
				//current FFE sampling time
				if(config1->usDep1_Sensor_Rate > usFFE_ticks_ms)
				{
					//set FFE sampling time to this alg dependent sensor rate
					FFE_Set_Sampletime(config1->usDep1_Sensor_Rate);
				}
			}
			else if(config2)
			{
				if(config2->usDep1_Sensor_Rate > usFFE_ticks_ms)
				{
					//set FFE sampling time to this alg dependent sensor rate
					FFE_Set_Sampletime(config2->usDep1_Sensor_Rate);
				}
				else if(config2->usDep2_Sensor_Rate > usFFE_ticks_ms)
				{
					//set FFE sampling time to this alg dependent sensor rate
					FFE_Set_Sampletime(config2->usDep2_Sensor_Rate);
				}
			}
		}

		usMailbox = usMailbox ^ (1 << ucBitpos);
		ulAlgCnt--;

	}while(ulAlgCnt > 0);
}

/*!
 * \fn          void FFE_UpdateCfg(UINT8_t ucMbUpdate, xSensor_alg_config_1 *cfg1, xSensor_alg_config_2 *cfg2)
 * \brief       This function updates the new FFE configuration.
 * \param       ucMbUpdate -- Flag to set if Mailbox update is needed or not
 * \param       *cfg1	   -- Pointer to Sensor_alg_Config1 structure
 * \param       *cfg2 	   -- Pointer to Sensor_alg_config2 structure *
 */
static void FFE_UpdateCfg(UINT8_t ucMbUpdate, xSensor_alg_config_1 *cfg1, xSensor_alg_config_2 *cfg2)
{
	if(ucMbUpdate)
	{
		if(ucMbUpdate & UPDATE_MB1)
		{
			//update mailbox1 value
			HAL_FFE_WriteMem((FFE_DM0 + expo_var[EXPO_FFE_MAILBOX1].usAddr),4,&usFFE_Mailbox1);
		}
		//else if(ucMbUpdate & UPDATE_MB2)
                if(ucMbUpdate & UPDATE_MB2)
		{
			//update mailbox2 value
			HAL_FFE_WriteMem((FFE_DM0 + expo_var[EXPO_FFE_MAILBOX2].usAddr),4,&usFFE_Mailbox2);
		}
	}

	if(cfg1 != NULL)
	{
		HAL_FFE_WriteMem((FFE_DM0 + uiAlgAddr[cfg1->usSensor_Id]),sizeof(xSensor_alg_config_1),cfg1);
	}
	else if(cfg2 != NULL)
	{
		HAL_FFE_WriteMem((FFE_DM0 + uiAlgAddr[cfg2->usSensor_Id]),sizeof(xSensor_alg_config_2),cfg2);
	}
}

void FFE_Set_BatchParams(uint32_t buffer_mem0, uint32_t buffer_mem1, uint32_t internal_buffering_time)
{
	HAL_FFE_WriteMem32((FFE_DM0 + expo_var[16].usAddr), buffer_mem0);
	HAL_FFE_WriteMem32((FFE_DM0 + expo_var[17].usAddr), buffer_mem1);
	HAL_FFE_WriteMem((FFE_DM0 + expo_var[20].usAddr), 4, &internal_buffering_time);
}

/*!
 * \fn          static HAL_StatusTypeDef FFE_Get_AlgConfig(UINT8_t ucId, xSensor_alg_config_1 **config_1, xSensor_alg_config_2 **config_2)
 * \brief       Traverse through sensor config structure list to identify corresponding sensor configuration
 * \param       Id      -- sensor ID
 * \param       config1 -- pointer to sensor configuration for single dependency sensor
 * \param       config2 -- pointer to sensor configuration for two dependency sensor
 * \return      HAL Status
 */
static HAL_StatusTypeDef FFE_Get_AlgConfig(UINT8_t ucId, xSensor_alg_config_1 **config_1, xSensor_alg_config_2 **config_2)
{
	UINT8_t ucIter = 0;

	for(ucIter = 0; ucIter < ALGS_COUNT; ucIter++)
	{
		if(alg_config_1[ucIter].usSensor_Id == ucId)
		{
			*config_1 = &alg_config_1[ucIter];
			return HAL_OK;
		}
		else if(alg_config_2[ucIter].usSensor_Id == ucId)
		{
			*config_2 = &alg_config_2[ucIter];
			return HAL_OK;
		}
#ifndef FEATURE_POWER_MANAGER
		printf("ucId = %d\n", ucId);
		printf("alg_config_1[%d].usSensor_Id = %d\n", ucIter, alg_config_1[ucIter].usSensor_Id);
		printf("alg_config_2[%d].usSensor_Id = %d\n", ucIter, alg_config_2[ucIter].usSensor_Id);
#endif
	}

	return HAL_ERROR;
}

/*!
 * \fn 		static HAL_StatusTypeDef FFE_Parse_ExpoSection(void)
 * \brief 	This function parses the export variables section in M4 SRAM
 * \param 	None
 * \return      HAL Status
 */
static HAL_StatusTypeDef FFE_Parse_ExpoSection(void)
{
	UINT16_t usIndex = 0;
	volatile UINT32_t temp = 0;
	UINT32_t ulExpSecLen = 0;

	ulExpSecLen = *FFE_CFG++;

	if(ulExpSecLen == 0)
	{
		printf("Invalid Export Section Length\r\n");
		return HAL_ERROR;
	}

	do
	{
		temp = *FFE_CFG;

		//IDs 1-99 are reserved for export variables.
		if((temp &  0xFF) < EXPORT_VAR_LIMIT)
		{
			memcpy(&expo_var[temp & 0xFF], (void*)FFE_CFG, sizeof(xExport_var_t));
			ulExpSecLen -= sizeof(xExport_var_t);;
			FFE_CFG = FFE_CFG + 2;
		}
		else
		{
			memcpy(&expo_var[EXPO_DUMP], (void*)FFE_CFG, sizeof(xExport_var_t));

			switch(expo_var[EXPO_DUMP].ulLen)
			{
			case ALG_CONFIG1_TYPE:
				HAL_FFE_ReadMem((FFE_DM0 + expo_var[EXPO_DUMP].usAddr),sizeof(xSensor_alg_config_1),&alg_config_1[usIndex]);
				uiAlgAddr[alg_config_1[usIndex].usSensor_Id] = expo_var[EXPO_DUMP].usAddr;
			break;

			case ALG_CONFIG2_TYPE:
				HAL_FFE_ReadMem((FFE_DM0 + expo_var[EXPO_DUMP].usAddr),sizeof(xSensor_alg_config_2),&alg_config_2[usIndex]);
				uiAlgAddr[alg_config_2[usIndex].usSensor_Id] = expo_var[EXPO_DUMP].usAddr;
				break;
			}

			FFE_CFG = FFE_CFG +2;
			usIndex++;
			ulExpSecLen -= sizeof(xExport_var_t);
		}

	}while(ulExpSecLen != 0);

	//Get the algorithm Id based on its mailbox bit definition
	FFE_Get_SensIdInfo(&ucSensId[0]);

	return HAL_OK;
}

/*!
 * \fn 		static UINT8_t FFE_CheckBitEnable(UINT32_t ulVar)
 * \brief 	Compute the enabled bit position from Mailbox value
 * \param 	ulVar -- Algorithm mailbox value
 * \return	bit index which is set to 1.
 */
static UINT8_t FFE_CheckBitEnable(UINT32_t ulVar)
{
	UINT8_t ucIndex = 0;

	while(ulVar != 0)
	{
		if(ulVar & 0x1)
			return ucIndex;

		ulVar >>= 1;
		ucIndex++;
	}

	return ucIndex;
}

/*!
 * \fn 		static void FFE_Get_SensIdInfo(UINT8_t *ucSensId)
 * \brief 	Traverse through the alg structure and store the sensor IDs
 * \brief 	in an array based on mailbox bit position
 * \param 	ucSensID -- Sensor ID
 */
static void FFE_Get_SensIdInfo(UINT8_t *ucSensId)
{
	UINT8_t ucIter = 0;
	UINT8_t ucBitpos = 0;

	for(ucIter = 0; ucIter < ALGS_COUNT; ucIter++)
	{
		if(alg_config_1[ucIter].mb_value != 0)
		{
			ucBitpos = FFE_CheckBitEnable(alg_config_1[ucIter].mb_value);
			ucSensId[ucBitpos] = alg_config_1[ucIter].usSensor_Id;
		}
		if(alg_config_2[ucIter].mb_value != 0)
		{
			ucBitpos = FFE_CheckBitEnable(alg_config_2[ucIter].mb_value);
			ucSensId[ucBitpos] = alg_config_2[ucIter].usSensor_Id;
		}
	}
}

/*!
 * \fn		static void FFE_Enable(UINT8_t ucEnable)
 * \brief 	Function to enable power & clock domains for FFE subsystem
 * \param	ucEnable --- Enable Flag
 */
static void FFE_Enable(UINT8_t ucEnable)
{
        if(ucEnable)
	{
#ifdef FEATURE_POWER_MANAGER
		S3x_Clk_Set_Rate(S3X_FFE_X4_CLK, (HSOSC_12MHZ));
        S3x_Clk_Enable(S3X_FFE_X4_CLK);
        S3x_Clk_Enable(S3X_FFE_X1_CLK);
        S3x_Clk_Set_Rate(S3X_FFE_CLK, HSOSC_3MHZ);
        S3x_Clk_Enable(S3X_FFE_CLK);
        S3x_Clk_Enable(S3X_SPT_CLK);
#else
		//enable FFE power & clock domain
		PMU->FFE_PWR_MODE_CFG = 0x0;
		PMU->FFE_PD_SRC_MASK_N = 0x0;
		PMU->FFE_WU_SRC_MASK_N = 0x0;

		//wake up FFE
		PMU->FFE_FB_PF_SW_WU = 0x1;
		//check if FFE is in Active mode
		while(!(PMU->FFE_STATUS & 0x1));

		//enable C08-X4
		S3x_Clk_Set_Rate(S3X_FFE_X4_CLK, (HSOSC_12MHZ));

		S3x_Clk_Enable(S3X_FFE_X4_CLK);
        S3x_Clk_Enable(S3X_FFE_X1_CLK);
        S3x_Clk_Enable(S3X_FFE_CLK);
        S3x_Clk_Enable(S3X_SPT_CLK);
		S3x_Clk_Enable(S3X_PKT_FIFO_CLK);
#endif
		printf("c8_x1 freq = %ld\r\n",S3x_Clk_Get_Rate(S3X_FFE_X1_CLK));
		printf("C01_clk_gate = %x\r\n",CRU->C01_CLK_GATE);
		printf("c8_x4 freq = %ld\r\n",S3x_Clk_Get_Rate(S3X_FFE_X4_CLK));
	}
}

/*!
 * \fn 		static void FFE_Set_Sampletime(UINT16_t usTimeinMS)
 * \brief 	This function sets the new FFE sample time
 * \param 	usTimeinMS -- Sample Time in milliseconds
 */
static void FFE_Set_Sampletime(UINT16_t usTimeinMS)
{
	if(usTimeinMS)
	{
		HAL_FFE_WriteMem((FFE_DM0 + expo_var[EXPO_FFE_TICK_MS].usAddr),4,&usTimeinMS);

		//Enable SPT - without this the timer wont work and the timestamp will be always zero
                SPT->SPT_CFG &= (~(0xFF << 10)); // hyungjoon48.lim. confirmed by QL 0824
		SPT->SPT_CFG |= (((usTimeinMS & 0xFF) << 10) | 0x1);
#ifndef FEATURE_POWER_MANAGER
		printf("SPT_CFG = %x\r\n", SPT->SPT_CFG);
#endif
	}
	else
		printf("Invalid Sample time\r\n");
}

/*!
 * \fn 		static void FFE_AlgEnable(UINT8_t *ucBuf,xSensor_alg_config_1 *config1,xSensor_alg_config_2 *config2)
 * \brief	Function to enable the corresponding Algorithm inside FFE
 * \param	ucBuf	---	User Command buffer
 * \param	config1	--- pointer to alg config1 structure
 * \param	config2	--- pointer to alg config2 structure
 */
static void FFE_AlgEnable(UINT8_t *ucBuf,xSensor_alg_config_1 *config1,xSensor_alg_config_2 *config2)
{
	UINT8_t ucMb_Update = 0;
	UINT8_t ucSensId = *ucBuf;
	UINT8_t ucEnFlag = ucBuf[1];
	UINT8_t ucCfgUpdate = 0;

	if((config1 == NULL && config2 == NULL) || ucSensId == 0)
	{
		printf("FFE_AlgEnable: Invalid parameters received\r\n");
		return;
	}

	if(ucSensId < MB1_SENSID_CUTOFF)
	{
		if(ucEnFlag && config1)
			usFFE_Mailbox1 |= (config1->mb_value) | 0x001 /* hack to enable accel */;
		else if(ucEnFlag == 0 && config1)
			usFFE_Mailbox1 &= ~(config1->mb_value);
		else if(ucEnFlag && config2)
			usFFE_Mailbox1 |= (config2->mb_value)| 0x001 /* hack to enable accel */;
		else if(ucEnFlag == 0 && config2)
			usFFE_Mailbox1 &= ~(config2->mb_value);

		ucMb_Update = UPDATE_MB1;
	}
	else
	{
		if(ucEnFlag && config1)
			usFFE_Mailbox2 |= (config1->mb_value);
		else if(ucEnFlag == 0 && config1)
			usFFE_Mailbox2 &= ~(config1->mb_value);
		else if(ucEnFlag && config2)
			usFFE_Mailbox2 |= (config2->mb_value);
		else if(ucEnFlag == 0 && config2)
			usFFE_Mailbox2 &= ~(config2->mb_value);

		ucMb_Update = UPDATE_MB2;
	}

	if(ucEnFlag)
	{
		if(config1)
		{
			config1->usFifo_op_addr = ucBuf[2];
                        config1->usDep1_Sensor_Rate = ucBuf[3] * 100;
                        config1->output_rate = ucBuf[3] * 100;

			//check if the dependent sensor rate is greater than the
			//current FFE sampling time

#ifdef FFE_NEWARCH
      			if(config1->usDep1_Sensor_Rate > 1000/usFFE_ticks_ms)
			{

			  	//set FFE sampling time to this alg dependent sensor rate
				usFFE_ticks_ms = ((1000 / config1->usDep1_Sensor_Rate)) ;
				FFE_Set_Sampletime(usFFE_ticks_ms);
#else

                         if(config1->usDep1_Sensor_Rate < usFFE_ticks_ms)
			{

				usFFE_ticks_ms = config1->usDep1_Sensor_Rate;
				FFE_Set_Sampletime(config1->usDep1_Sensor_Rate);
#endif

#ifndef FEATURE_POWER_MANAGER
				printf("usFFE_ticks_ms = %x\r\n", usFFE_ticks_ms);
#endif
			}

			//update ffe DM with new config change and unlock ffe to execute
			FFE_UpdateCfg(ucMb_Update,config1,NULL);
		}
		else if(config2)
		{
			config2->usFifo_op_addr = ucBuf[2];
#ifdef FFE_NEWARCH
                        if(config2->usDep1_Sensor_Rate > 1000/usFFE_ticks_ms)
			{
			  	usFFE_ticks_ms = (1000 / config2->usDep1_Sensor_Rate);
				FFE_Set_Sampletime(usFFE_ticks_ms);
#else
                        if(config2->usDep1_Sensor_Rate < usFFE_ticks_ms)
			{
			  	//set FFE sampling time to this alg dependent sensor rate
				FFE_Set_Sampletime(config2->usDep1_Sensor_Rate);
				usFFE_ticks_ms = config2->usDep1_Sensor_Rate;
#endif
#ifndef FEATURE_POWER_MANAGER
				printf("usFFE_ticks_ms = %x\r\n", usFFE_ticks_ms);
#endif
			}
#ifdef FFE_NEWARCH
      			if(config2->usDep2_Sensor_Rate > 1000/usFFE_ticks_ms)
			{

			  	usFFE_ticks_ms = (1000 / config2->usDep2_Sensor_Rate);
				FFE_Set_Sampletime(usFFE_ticks_ms);
#else
      			if(config2->usDep2_Sensor_Rate < usFFE_ticks_ms)
			{

			  	//set FFE sampling time to this alg dependent sensor rate
				FFE_Set_Sampletime(config2->usDep2_Sensor_Rate);
				usFFE_ticks_ms = config2->usDep2_Sensor_Rate;
#endif
#ifndef FEATURE_POWER_MANAGER
				printf("usFFE_ticks_ms = %x\r\n", usFFE_ticks_ms);
#endif
			}

			//update ffe DM with new config change and unlock ffe to execute
			FFE_UpdateCfg(ucMb_Update,NULL,config2);
		}
	}
	else
	{
		if((usFFE_Mailbox1 != 0) || (usFFE_Mailbox2 != 0))
		{
			if(usFFE_Mailbox1)
			{
				FFE_Compute_New_Samplerate(usFFE_Mailbox1);

				ucCfgUpdate = UPDATE_MB1;
			}
			else if(usFFE_Mailbox2)
			{
				FFE_Compute_New_Samplerate(usFFE_Mailbox2);

				ucCfgUpdate = UPDATE_MB2;
			}

			//update ffe DM with new config change and unlock ffe to execute
			FFE_UpdateCfg(ucCfgUpdate,NULL,NULL);
			ucCfgUpdate = 0;
		}
		else
		{
			//update ffe DM with new config change and unlock ffe to execute
			FFE_UpdateCfg(ucCfgUpdate,NULL,NULL);
			ucCfgUpdate = 0;
		}
	}
}

/*!
 * \fn 		static void FFE_SampleRateCfg(UINT8_t *ucBuf, xSensor_alg_config_1 *config1,xSensor_alg_config_2 *config2)
 * \brief	Function to configure the FFE sample rate inside FFE
 * \param	ucBuf	--- User Command buffer
 * \param	config1	--- pointer to alg config1 structure
 * \param	config2	--- pointer to alg config2 structure
 */
static void FFE_SampleRateCfg(UINT8_t *ucBuf, xSensor_alg_config_1 *config1,xSensor_alg_config_2 *config2)
{
	UINT8_t ucCfgUpdate = 0;
	UINT32_t ulAlgCnt = 0;

	if((config1 == NULL && config2 == NULL) )
	{
		printf("FFE_SampleRateCfg: Invalid parameters received\r\n");
		return;
	}

	//update the new value and write to FFE DM.
	if(config1 != NULL)
	{
		if(ucBuf[1] == 0)
			return;

		if(ucBuf[1] != config1->output_rate)
		{
			if(config1->usSensor_Id == config1->usDep1_Sensor_Id)
			{
				//handle physical sensor
				config1->usDep1_Sensor_Rate = ucBuf[1];
				config1->output_rate = ucBuf[1];

				if(usFFE_Mailbox1)
					ulAlgCnt = FFE_GetAlgsEnabledCnt(usFFE_Mailbox1);
				else
				  	ulAlgCnt = FFE_GetAlgsEnabledCnt(usFFE_Mailbox2);

				if(ulAlgCnt == 1)
				{
#ifdef FFE_NEWARCH
					usFFE_ticks_ms = (1000 / ucBuf[1]);
#else
					usFFE_ticks_ms = ucBuf[1];
#endif

					//update the FFE sample time with the new rate
					FFE_Set_Sampletime(usFFE_ticks_ms);
#ifndef FEATURE_POWER_MANAGER
					printf("usFFE_ticks_ms = %x\r\n", usFFE_ticks_ms);
#endif

				}
				else if((1000 / ucBuf[1]) < usFFE_ticks_ms)
				{
#ifdef FFE_NEWARCH
					usFFE_ticks_ms = (1000 / ucBuf[1]);
#else
					usFFE_ticks_ms = ucBuf[1];
#endif

					//update the FFE sample time with the new rate
					FFE_Set_Sampletime(usFFE_ticks_ms);
#ifndef FEATURE_POWER_MANAGER
					printf("usFFE_ticks_ms = %x\r\n", usFFE_ticks_ms);
#endif
				}

				//update FFE about config change
				FFE_UpdateCfg(0,config1,NULL);
			}
			else
			{
				if(config1->usDep1_Sensor_Rate >= ucBuf[1])
				{
					config1->output_rate = ucBuf[1];

					//update FFE about config change
					FFE_UpdateCfg(0,config1,NULL);
				}
				else //invalid request
					return;
			}
		}
	}
	else if(config2 != NULL)
	{
		if(ucBuf[1] == 0)
			return;

		if(config2->usDep1_Sensor_Rate >= ucBuf[1])
		{
			config2->output_rate = ucBuf[1];
			config2->usDep1_Sensor_Rate = ucBuf[1];
			ucCfgUpdate = 1;
		}
		if(config2->usDep2_Sensor_Rate >= ucBuf[1])
		{
			config2->usDep2_Sensor_Rate = ucBuf[1];
			config2->output_rate = ucBuf[1];
			ucCfgUpdate = 1;
		}
		else
			//Invalid Request
			return;

		if(ucBuf[1] < usFFE_ticks_ms)
		{
#ifdef FFE_NEWARCH
			usFFE_ticks_ms = (1000 / ucBuf[1]);
#else
			usFFE_ticks_ms = ucBuf[1];
#endif
			//update the FFE sample time with the new rate
			FFE_Set_Sampletime(usFFE_ticks_ms);
		}
		if(ucCfgUpdate)
		{
			ucCfgUpdate = 0;

			//update FFE about config change
			FFE_UpdateCfg(0,NULL,config2);
		}
	}
}

/*!
 * \fn 		static void FFE_CfgBatchSize(UINT8_t *ucBuf, xSensor_alg_config_1 *config1,xSensor_alg_config_2 *config2)
 * \brief	Function to configure the batch size inside FFE
 * \param	ucBuf	--- User Command buffer
 * \param	config1	--- pointer to alg config1 structure
 * \param	config2	--- pointer to alg config2 structure
 */
static void FFE_CfgBatchSize(UINT8_t *ucBuf, xSensor_alg_config_1 *config1,xSensor_alg_config_2 *config2)
{
	uint16_t batch_size = 20000;
	if((config1 == NULL && config2 == NULL))
	{
		printf("FFE_CfgBatchSize: Invalid parameters received\r\n");
		return;
	}

	if(config1)
	{
		 if(ucBuf[1])	/* commented code: > config1->usBatch_size)*/
		{
		 /* hardcoding batching size to 20000 */
		 config1->usBatch_size = batch_size;/* commented code : ucBuf[1];*/

			//update FFE about config change
			FFE_UpdateCfg(0,config1,NULL);
		}
		else
		{
 			config1->usBatch_size = 0;/*ucBuf[1];*/
			//update FFE about config change
			FFE_UpdateCfg(0,config1,NULL);
		}
	}
	else if(config2)
	{
                if(ucBuf[0] == 0x22)
                {
                        if(ucBuf[1]) /*> config2->usBatch_size)*/
                        {
                                config2->usBatch_size = (uint32_t)(1000*ucBuf[1]); // hyungjoon48.lim set batch size
                                //update FFE about config change
                                //Replace Batch size update API from "FFE_UpdateCfg" to
                                //"SET_ALG_PARAM_VAL_IN_EXPO_SEC" to avoid overwriting of other parameters
                                //from QL 0919
//                                FFE_UpdateCfg(0,NULL,config2);
                                SET_ALG_PARAM_VAL_IN_EXPO_SEC(config2->usSensor_Id, xSensor_alg_config_2, usBatch_size, config2->usBatch_size);
                        }
                        else
                        {
                                config2->usBatch_size = 0;/*ucBuf[1];*/
                                //update FFE about config change
                                //Replace Batch size update API from "FFE_UpdateCfg" to
                                //"SET_ALG_PARAM_VAL_IN_EXPO_SEC" to avoid overwriting of other parameters
                                //from QL 0919
//                                 FFE_UpdateCfg(0,NULL,config2);
                                 SET_ALG_PARAM_VAL_IN_EXPO_SEC(config2->usSensor_Id, xSensor_alg_config_2, usBatch_size, config2->usBatch_size);
                        }
                }
                else
                {
                        if(ucBuf[1]) /*> config2->usBatch_size)*/
                        {
                                config2->usBatch_size = batch_size;/*ucBuf[1];*/
                                //update FFE about config change
                                //Replace Batch size update API from "FFE_UpdateCfg" to
                                //"SET_ALG_PARAM_VAL_IN_EXPO_SEC" to avoid overwriting of other parameters
                                //from QL 0919
//                                FFE_UpdateCfg(0,NULL,config2);
                                SET_ALG_PARAM_VAL_IN_EXPO_SEC(config2->usSensor_Id, xSensor_alg_config_2, usBatch_size, config2->usBatch_size);
                        }
                        else
                        {
                                config2->usBatch_size = 0;/*ucBuf[1];*/
                                //update FFE about config change
                                //Replace Batch size update API from "FFE_UpdateCfg" to
                                //"SET_ALG_PARAM_VAL_IN_EXPO_SEC" to avoid overwriting of other parameters
                                //from QL 0919
//                                FFE_UpdateCfg(0,NULL,config2);
                                SET_ALG_PARAM_VAL_IN_EXPO_SEC(config2->usSensor_Id, xSensor_alg_config_2, usBatch_size, config2->usBatch_size);
                        }
                }
	}
}


static void FFE_CfgBatchFlush(UINT8_t *ucBuf, xSensor_alg_config_1 *config1,xSensor_alg_config_2 *config2)
{
	xSensor_alg_config_1 cfg1;
	xSensor_alg_config_2 cfg2;

	if((config1 == NULL && config2 == NULL))
	{
		printf("FFE_CfgBatchSize: Invalid parameters received\r\n");
		return;
	}
	if(config1)
	{
		if(ucBuf[1])
		{
		 	SET_ALG_PARAM_VAL_IN_EXPO_SEC(config1->usSensor_Id, xSensor_alg_config_1, usBatch_flush, 1);

			HAL_FFE_ReadMem((FFE_DM0 + uiAlgAddr[config1->usSensor_Id]),sizeof(xSensor_alg_config_1), &cfg1);
#ifndef FEATURE_POWER_MANAGER
			printf("cfg1.usBatch_flush = %x, cfg1.uiBatching_Mode = %x, cfg1.uiBatched_Packet_Count = %x, cfg1.uiElapsed_Batch_Time = %x\n",
			       	cfg1.usBatch_flush, cfg1.uiBatching_Mode, cfg1.uiBatched_Packet_Count, cfg1.uiElapsed_Batch_Time);
#endif
		}
	}
	else if(config2)
	{
		if(ucBuf[1])
		{
		 	SET_ALG_PARAM_VAL_IN_EXPO_SEC(config2->usSensor_Id, xSensor_alg_config_2, usBatch_flush, 1);

			HAL_FFE_ReadMem((FFE_DM0 + uiAlgAddr[config2->usSensor_Id]),sizeof(xSensor_alg_config_2), &cfg2);
#ifndef FEATURE_POWER_MANAGER
			printf("cfg2.usBatch_flush = %x, cfg2.uiBatching_Mode = %x, cfg2.uiBatched_Packet_Count = %x, cfg2.uiElapsed_Batch_Time = %x\n",
			       	cfg2.usBatch_flush, cfg2.uiBatching_Mode, cfg2.uiBatched_Packet_Count, cfg2.uiElapsed_Batch_Time);
#endif
		}
	}
}

/*!
 * \fn 		static void FFE_AlgOutputCfg(UINT8_t *ucBuf, xSensor_alg_config_1 *config1,xSensor_alg_config_2 *config2)
 * \brief	Function to configure the sensor packet output mode configure inside FFE
 * \param	ucBuf	--- User Command buffer
 * \param	config1	--- pointer to alg config1 structure
 * \param	config2	--- pointer to alg config2 structure
 */
static void FFE_AlgOutputCfg(UINT8_t *ucBuf, xSensor_alg_config_1 *config1,xSensor_alg_config_2 *config2)
{
	if((config1 == NULL && config2 == NULL))
	{
		printf("FFE_AlgOutputCfg: Invalid parameters received\r\n");
		return;
	}

	//update the new value and write to FFE DM.
	if(config1 != NULL)
	{
		config1->usBG_Enable = ucBuf[1];

		//update FFE about config change
		FFE_UpdateCfg(0,config1,NULL);
	}
	else if(config2 != NULL)
	{
		config2->usBG_Enable = ucBuf[1];

		//update FFE about config change
		FFE_UpdateCfg(0,NULL,config2);
	}
}

/*!
 * \fn 		static HAL_StatusTypeDef FFE_ConfigureSM(UINT8_t ucSM_type)
 * \brief 	This function configures SM 0/1 SDA and SCL pads
 * \param 	SM_type -- Sensor Manager type (SM0/SM1)
 * \return	Status
 */
static HAL_StatusTypeDef FFE_ConfigureSM(UINT8_t ucSM_type)
{
          PadConfig  padcfg;
  switch(ucSM_type)
	{
	case SM0:

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
/*
		printf("PAD 0 io_mux = %x\r\n",IO_MUX->PAD_0_CTRL);
		printf("PAD 1 io_mux = %x\r\n",IO_MUX->PAD_1_CTRL);
		printf("SDA0_SEL = %x\r\n",IO_MUX->SDA0_SEL_REG);
		printf("SCL0_SEL = %x\r\n",IO_MUX->SCL0_SEL_REG);
*/
		break;

	case SM1:

		break;
	default:
			printf("Invalid SM type: %d\r\n",ucSM_type);
			return HAL_ERROR;
	}

	return HAL_OK;
}

/// @endcond

/*!
 * \fn 		HAL_StatusTypeDef HAL_FFE_GetAlgParams(UINT8_t usSensId,xSensor_alg_config_1 *config1,xSensor_alg_config_2 *config2);
 * \brief 	This function returns the Alg configuration parameters
 * \param 	usSensId   --- sensor ID
 * \param   config1    --- pointer to Config 1 structure
 * \param	config2    --- pointer to Config 2 structure
 * \return	HAL Status
 */
HAL_StatusTypeDef HAL_FFE_GetAlgParams(UINT8_t ucSensId,xSensor_alg_config_1 **config1,xSensor_alg_config_2 **config2)
{
	if(ucSensId == 0)
	{
		printf("Invalid Sensor Id\r\n");
		return HAL_ERROR;
	}

	if(FFE_Get_AlgConfig(ucSensId,config1,config2) != HAL_OK)
	{
		printf("HAL_FFE_GetAlgParams: FFE_Get_AlgConfig failed\r\n");
		return HAL_ERROR;
	}
	return HAL_OK;
}

/*!
 * \fn 		HAL_StatusTypeDef HAL_FFE_SetAlgParams(UINT8_t *ucBuf, UINT8_t ucCmdType)
 * \brief 	This function configures the Alg parameters inside FFE
 * \param 	ucBuf	     --- User Command buffer
 * \param       ucCmdType    --- Command Type
 * \return	HAL Status
 */
HAL_StatusTypeDef HAL_FFE_SetAlgParams(UINT8_t *ucBuf, UINT8_t ucCmdType)
{
	UINT8_t ucSensId = ucBuf[0];
	UINT8_t *ucCmdBuf = ucBuf;
	xSensor_alg_config_1 *config1 = NULL;
	xSensor_alg_config_2 *config2 = NULL;

	if(ucSensId == 0)
	{
		printf("Invalid Sensor Id\r\n");
		return HAL_ERROR;
	}

	if(FFE_Get_AlgConfig(ucSensId,&config1,&config2) == HAL_OK)
	{
		switch(ucCmdType)
		{
		case eFFE_SENSOR_ENABLE:
			if(config1)
				FFE_AlgEnable(ucCmdBuf,config1,NULL);
			else if(config2)
				FFE_AlgEnable(ucCmdBuf,NULL,config2);
			break;

		case eFFE_SENSOR_RATE_CHANGE:
			if(config1)
				FFE_SampleRateCfg(ucCmdBuf,config1,NULL);
			else if(config2)
				FFE_SampleRateCfg(ucCmdBuf,NULL,config2);
			break;

		case eFFE_SENSOR_BATCH_SIZE:
			if(config1)
				FFE_CfgBatchSize(ucCmdBuf,config1,NULL);
			else if(config2)
				FFE_CfgBatchSize(ucCmdBuf,NULL,config2);
			break;

		case eFFE_SENSOR_OUTPUT_DISABLE:
			if(config1)
				FFE_AlgOutputCfg(ucCmdBuf,config1,NULL);
			else if(config2)
				FFE_AlgOutputCfg(ucCmdBuf,NULL,config2);
			break;

		case eFFE_SENSOR_BATCH_FLUSH:
			if(config1)
				FFE_CfgBatchFlush(ucCmdBuf,config1,NULL);
			else if(config2)
				FFE_CfgBatchFlush(ucCmdBuf,NULL,config2);
			break;
		}
	}
	else
	{
		printf("Sensor Id: %d doesn't match with the existing alg_config structure\r\n",ucBuf[0]);
	}

	return HAL_OK;
}

HAL_StatusTypeDef HAL_FFE_WriteMem32(volatile UINT32_t *ulMem_addr, UINT32_t value)
{
	if(ulMem_addr == NULL )
	{
		printf("HAL_FFE_WriteMem32: Invalid parameters received\r\n");
		return HAL_ERROR;
	}
	*ulMem_addr = value;
	return HAL_OK;
}

/*!
 * \fn 		HAL_StatusTypeDef HAL_FFE_WriteMem(volatile UINT32_t *ulMem_addr, UINT32_t ulLen, void *buf)
 * \brief 	This function write into FFE Data memory section
 * \param 	ulMem_addr -- DM Memory Address
 * \param 	ulLen 	   -- Number of bytes to write
 * \param 	buf        -- Data buffer to write
 * \return      HAL Status
 */
HAL_StatusTypeDef HAL_FFE_WriteMem(volatile UINT32_t *ulMem_addr, UINT32_t ulLen, void *buf)
{
#ifdef FEATURE_POWER_MANAGER
    UINT8_t ffeSleepModeOn = 0;
#endif
	if(ulLen == 0 || buf == NULL )
	{
		printf("HAL_FFE_WriteMem: Invalid parameters received\r\n");
		return HAL_ERROR;
	}
#ifdef FEATURE_POWER_MANAGER
        if (PMU->FFE_PD_SRC_MASK_N) {
          ffeSleepModeOn = 1;
          PMU->FFE_PD_SRC_MASK_N = 0;
        }
        //HAL_SetPowerDomainState(FFE,WAKEUP);
#endif

	UINT32_t ulIndex = 0;
	UINT32_t ulTemp = 0;
	UINT32_t *ulTempBuf = (UINT32_t*)buf;

	for(ulIndex = 0; ulIndex < (ulLen/4); ulIndex++)
	{
		ulTemp = *ulTempBuf++;
		*ulMem_addr = ulTemp << 16;
		ulMem_addr++;
	}
#ifdef FEATURE_POWER_MANAGER
        if (SPT->SPT_CFG && ffeSleepModeOn) {
          PMU->FFE_PD_SRC_MASK_N = 1;
        }
#endif
	return HAL_OK;
}

/*!
 * \fn 		HAL_StatusTypeDef HAL_FFE_ReadMem(volatile UINT32_t *ulMem_addr, UINT32_t ulLen, void *buf)
 * \brief 	This function read from FFE Data memory section
 * \param 	ulMem_addr -- DM Memory Address
 * \param 	ulLen 	   -- Number of bytes to read
 * \param 	buf        -- Data buffer to read
 * \return      HAL Status
 */
HAL_StatusTypeDef HAL_FFE_ReadMem(volatile UINT32_t *ulMem_addr, UINT32_t ulLen, void *buf)
{
#ifdef FEATURE_POWER_MANAGER
    UINT8_t ffeSleepModeOn = 0;
#endif
	if(ulLen == 0 || buf == NULL )
	{
		printf("HAL_FFE_ReadMem: Invalid parameters received\r\n");
		return HAL_ERROR;
	}
#ifdef FEATURE_POWER_MANAGER
        if (PMU->FFE_PD_SRC_MASK_N) {
          ffeSleepModeOn = 1;
          PMU->FFE_PD_SRC_MASK_N = 0;
        }
        //HAL_SetPowerDomainState(FFE,WAKEUP);
#endif
	UINT32_t ulIndex = 0;
	UINT32_t ulTemp = 0;
	UINT32_t *ulTempBuf = (UINT32_t*)buf;

	for(ulIndex = 0; ulIndex < (ulLen/4); ulIndex++)
	{
		ulTemp = *ulMem_addr++;
		*ulTempBuf = ulTemp >> 16;
		ulTempBuf++;
	}
#ifdef FEATURE_POWER_MANAGER
        if (SPT->SPT_CFG && ffeSleepModeOn) {
          PMU->FFE_PD_SRC_MASK_N = 1;
        }
#endif
	return HAL_OK;
}

/*!
 * \fn 		HAL_StatusTypeDef HAL_FFE_Config(FFE_HandleTypeDef *hFFE)
 * \brief 	This function configure the FFE
 * \param 	hFFE -- pointer to FFE Handle structure
 * \return      HAL Status
 */
HAL_StatusTypeDef HAL_FFE_Config(FFE_HandleTypeDef *hFFE)
{
	UINT32_t ulCnt = 0;
	UINT32_t ulRegVal = 0;
//        UINT32_t ffeBusy = 0;                                                 // to remove warnings
#ifdef FEATURE_POWER_MANAGER
    UINT8_t ffeSleepModeOn = 0;
#endif
#ifdef FEATURE_POWER_MANAGER
        if (PMU->FFE_PD_SRC_MASK_N) {
          ffeSleepModeOn = 1;
          PMU->FFE_PD_SRC_MASK_N = 0;
        }
       //HAL_SetPowerDomainState(FFE,WAKEUP);
#endif

	if(hFFE->ucStopFFE)
	{
#ifndef FEATURE_POWER_MANAGER
		printf("Stopping FFE \r\n");
#endif

		//Disable SPT to stop timer ticking
		SPT->SPT_CFG = 0x0;

		//read status register to check if FFE is in Idle state or not
		for(ulCnt = 0; ulCnt < 10; ulCnt++)
		{
			ulRegVal = EXT_REGS_FFE->STATUS;

			if(ulRegVal & (FFE_SMO_BUSY | FFE_SM1_BUSY | FFE_FFEO_BUSY))
			{
#ifndef FEATURE_POWER_MANAGER
				printf("FFE/SM is currently busy\r\n");
#endif

				//add delay
			}
			else
				break;
		}

		//complete the stop procedure
		if(ulRegVal == 0)
		{
			//Enable reset on all FIFOs

		}

		usFFE_ticks_ms = 0;
		hFFE->ucStopFFE = 0;
#ifndef FEATURE_POWER_MANAGER
		printf("FFE is stopped\r\n");
#endif

	}
	else if(hFFE->ucRunContinous)
	{
		//check status register
		ulRegVal = EXT_REGS_FFE->STATUS;

		if(ulRegVal & 0x04)
#ifndef FEATURE_POWER_MANAGER
			printf("Warning: Status register shows activity (%02x)\n",ulRegVal);
#endif
		//Enable SPT - without this the timer wont work and the timestamp will be always zero


		//set the sample period
		//printf("FFE Sampling Period  = %d\r\n",(((1000 / hFFE->ulFFEFreq) - 1) & 0xFF));
		printf("FFE Sampling Period1  = %x\r\n",(((1000 / hFFE->ulFFEFreq)) & 0xFF) << 10);
#ifdef FFE_NEWARCH
                uint32_t currentSamplingPeriod = (SPT->SPT_CFG >> 10) & 0xFF;
                if (currentSamplingPeriod == 0 ||  currentSamplingPeriod > (1000 / hFFE->ulFFEFreq)) {
                  SPT->SPT_CFG = 0x1;
                  SPT->SPT_CFG |= ((((1000 / hFFE->ulFFEFreq)) & 0xFF) << 10);
                }
#else
		SPT->SPT_CFG |= ((hFFE->ulFFEFreq & 0xFF) << 10);
#endif
		//printf("SPT_CFG = %x\r\n", SPT->SPT_CFG);
		hFFE->ucRunContinous = 0;
	}
	else if(hFFE->ucRunOnce)
	{
		for(ulCnt = 0; ulCnt < hFFE->ulRunCnt; ++ulCnt)
		{
			//Runs FFE once
			EXT_REGS_FFE->CMD = FFE_CMD_RUN_FFE0_ONCE;

			//Wait till FFE is not busy
			printf("Waiting for FFE to be idle...\r\n");
			do {
				ulRegVal = EXT_REGS_FFE->STATUS;
			} while(ulRegVal & 0x4);

			printf("FFE Idle\r\n");
		}

		//Disable SPT
		SPT->SPT_CFG = 0x0;

		hFFE->ucRunOnce = 0;
	}
	else if(hFFE->ucReleaseFFE)
	{
		//check if m4_request_state is 1
		HAL_FFE_ReadMem((FFE_DM0 + expo_var[EXPO_M4_REQ_STATE].usAddr),4,&ulRegVal);
		if(ulRegVal == 1)
		{
			ulRegVal = 0;
			HAL_FFE_WriteMem((FFE_DM0 + expo_var[EXPO_M4_REQ_STATE].usAddr),4,&ulRegVal);
#ifndef FEATURE_POWER_MANAGER
			printf("FFE is released to run\r\n");
#endif
		}
		else
			;//printf("FFE is not in halt state\r\n");

		hFFE->ucReleaseFFE = 0;
	}
	else if(hFFE->ucHaltFFE)
	{
		if(usFFE_ticks_ms)
		{
			//set m4_request_state to 1 to halt FFE execution at the end of this cycle
			HAL_FFE_ReadMem((FFE_DM0 + expo_var[EXPO_M4_REQ_STATE].usAddr),4,&ulRegVal);
			if(ulRegVal == 0)
			{
				ulRegVal = 1;
				HAL_FFE_WriteMem((FFE_DM0 + expo_var[EXPO_M4_REQ_STATE].usAddr),4,&ulRegVal);
				ulRegVal = 0;

				//wait for ffe_response_state to become 1
				//while(usFFE_ticks_ms)
				while(1)
				{
					HAL_FFE_ReadMem((FFE_DM0 + expo_var[EXPO_FFE_RESP_STATE].usAddr),4,&ulRegVal);
					if(ulRegVal)
						break;
				}
#ifndef FEATURE_POWER_MANAGER
				printf("FFE is halted\r\n");
#endif
			}
			else
                        {
#ifndef FEATURE_POWER_MANAGER
				printf("m4_request_state is already enabled\r\n");
#endif
                        }
		}
		hFFE->ucHaltFFE = 0;
	}
#ifdef FEATURE_POWER_MANAGER
        if (SPT->SPT_CFG && ffeSleepModeOn) {
          PMU->FFE_PD_SRC_MASK_N = 1;
        }
#endif
	return HAL_OK;
}

/*!
 * \fn 		HAL_StatusTypeDef HAL_FFE_Init(void)
 * \brief 	This function initialize the FFE
 * \return      HAL Status
 */
HAL_StatusTypeDef HAL_FFE_Init(void)
{
	UINT32_t ulRegValue = 0;

	FFE_Enable(1);

	//Run FFE for 1 cycle at least before reading the export section

	//Enable SPT - without this the timer wont work and the time stamp will be always zero

#ifdef FEATURE_POWER_MANAGER
        //HAL_I2C0_Select();
        //HAL_I2C_SetClockFreq(I2C_200KHZ); //set I2C to run at 100 Khz.
#endif
	EXT_REGS_FFE->CMD = FFE_CMD_RUN_FFE0_ONCE;

        // Make sure the FFE is started…
        do {
             ulRegValue = EXT_REGS_FFE->STATUS;
        } while(ulRegValue == 0x0);


	//Wait till FFE is not busy
//	printf("Waiting for FFE to be idle...\r\n");
	do {
		ulRegValue = EXT_REGS_FFE->STATUS;
	} while(ulRegValue & 0x4);



	//Disable SPT
	SPT->SPT_CFG = 0x0;

	//vTaskDelay(1000);

	//parse export section
	if(FFE_Parse_ExpoSection() != HAL_OK)
	{
		printf("FFE_Parse_ExpoSection Failed\r\n");
		return HAL_ERROR;
	}

	//configure FIFO for FFE
	if(ucFFE2M4Fifo0)
	{
		xFifoConfig.eFifoID = FIFO1;
		xFifoConfig.eSrc = FIFO_SRC_FFE0;
		xFifoConfig.eDest = FIFO_DEST_M4;

		if(HAL_FIFO_Init(xFifoConfig) != HAL_OK)
		{
			printf("HAL_FIFO_Init Failed\r\n");
			return HAL_ERROR;
		}
	}

	if(ucFFE2M4Fifo1)
	{
		xFifoConfig.eFifoID = FIFO8K;
		xFifoConfig.eSrc = FIFO_SRC_FFE0;
		xFifoConfig.eDest = FIFO_DEST_M4;

		if(HAL_FIFO_Init(xFifoConfig) != HAL_OK)
		{
			printf("HAL_FIFO_Init Failed\r\n");
			return HAL_ERROR;
		}
	}

	//Configure SM
	if(FFE_ConfigureSM(SM0) != HAL_OK)
	{
		return HAL_ERROR;
	}


	//Enable FFE0 related interrupts
	EXT_REGS_FFE->INTERRUPT_EN = (FFE_INTR_PKFB_OVF_INTR|FFE_INTR_I2C_MS_0_ERROR|FFE_INTR_I2C_MS_1_ERROR|
								  FFE_INTR_FFE0_SM0_OVERRUN|FFE_INTR_FFE0_SM1_OVERRUN|FFE_INTR_FFE0_OVERRUN|
								  FFE_INTR_SM_MULT_WR_INTR);
        INTR_CTRL->OTHER_INTR_EN_M4 |= FFE0_INTR_OTHERS_EN_M4;


	HAL_FFE_ENABLE_INT();
	return HAL_OK;
}

/*!
 * \fn 		HAL_StatusTypeDef HAL_FFE_DeInit(FFE_HandleTypeDef *hFFE)
 * \brief 	This function deinitialize the FFE
 * \return      HAL Status
 */
HAL_StatusTypeDef HAL_FFE_DeInit(FFE_HandleTypeDef *hFFE)
{
	return HAL_OK;
}
