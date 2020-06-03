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

/*!	\file eoss3_hal_ffe.h
 *
 *  Created on: Feb 15, 2016
 *      Author: Rajkumar Thiagarajan
 * 
 *  \brief This file contains macros and APIs
 *         for FFE. 
 */

#ifndef HAL_INC_EOSS3_HAL_FFE_H_
#define HAL_INC_EOSS3_HAL_FFE_H_

#include <common.h>
#include <stdint.h>
#include <stddef.h>

#include "test_types.h"
#include "eoss3_hal_def.h"

/// @cond HAL_FFE_INTERNAL_MACROS

/*! \def EXPORT_VAR_COUNT
    \brief A macro to define export variable count.
*/
#define EXPORT_VAR_COUNT			  64

/*! \def FFE_CFG_BASE
    \brief A macro to define M4 SRAM offset which stores the export section.
*/
#define FFE_CFG_BASE				  0x2007F800
//#define FFE_CFG_BASE				  0x2007FE80  //Export section parsing base address for new celeris patch [Syama]

/*! \def ALGS_COUNT
    \brief A macro to define algorithm count.
*/
#define ALGS_COUNT					  10

/*! \def EXPO_FFE_MAILBOX1
    \brief A macro to define export variable FFE_MAILBOX1.
*/
#define EXPO_FFE_MAILBOX1			  1

/*! \def EXPO_FFE_MAILBOX2
    \brief A macro to define export variable FFE_MAILBOX2.
*/
#define EXPO_FFE_MAILBOX2			  2

/*! \def EXPO_SM_MAILBOX
    \brief A macro to define export variable SM_MAILBOX.
*/
#define EXPO_SM_MAILBOX				  3

/*! \def EXPO_FFE_TICK_MS
    \brief A macro to define export variable FFE_TICK_MS.
*/
#define EXPO_FFE_TICK_MS			  4

/*! \def EXPO_INTR_MASK
    \brief A macro to define export variable INTR_MASK.
*/
#define EXPO_INTR_MASK				  5

/*! \def EXPO_M4_REQ_STATE
    \brief A macro to define export variable M4_REQ_STATE.
*/
#define EXPO_M4_REQ_STATE			  6

/*! \def EXPO_FFE_RESP_STATE
    \brief A macro to define export variable FFE_RESP_STATE.
*/
#define EXPO_FFE_RESP_STATE			  7

/*! \def EXPO_ACCEL_POLL_FREQ
    \brief A macro to define export variable ACCEL_POLL_FREQ.
*/
#define EXPO_ACCEL_POLL_FREQ		  8

/*! \def EXPO_MAG_POLL_FREQ
    \brief A macro to define export variable MAG_POLL_FREQ.
*/
#define EXPO_MAG_POLL_FREQ		  	  9

/*! \def EXPO_GYRO_POLL_FREQ
    \brief A macro to define export variable GYRO_POLL_FREQ.
*/
#define EXPO_GYRO_POLL_FREQ		  	  10

/*! \def EXPO_DUMP
    \brief A macro to define dump location for export structure.
*/
#define EXPO_DUMP					  63

/*! \def ALG_CONFIG1_TYPE
    \brief A macro to define algorithm structure type with single dependent sensor.
*/
#define ALG_CONFIG1_TYPE			 13

/*! \def ALG_CONFIG2_TYPE
    \brief A macro to define algorithm structure type with two dependent sensor.
*/
#define ALG_CONFIG2_TYPE			  15

/*! \def EXPORT_VAR_LIMIT
    \brief A macro to define export variable count.
*/
#define EXPORT_VAR_LIMIT			  0x64

/*! \def MB1_SENSID_CUTOFF
    \brief A macro to define Sensor Id cutoff for FFE Mailbox 1
*/
#define MB1_SENSID_CUTOFF			  0x30

/*! \def UPDATE_MB1
    \brief A macro to inform FFE Mailbox 1 needed to be updated.
*/
#define UPDATE_MB1					  0x81

/*! \def UPDATE_MB2
    \brief A macro to inform FFE Mailbox 2 needed to be updated.
*/
#define UPDATE_MB2					  0x82
/*! \def SM0
    \brief A macro to define SM type0.
*/
#define SM0							  0

/*! \def SM1
    \brief A macro to define SM type1.
*/
#define SM1							  1

/// @endcond

/// @cond HAL_FFE_INTERNAL_STRUCTURES
/*! \struct xSensor_alg_config_1
 *	\brief	Sensor Configuration Parameters for single sensor dependency
 */
typedef struct
{
	UINT32_t usSensor_Id;				/*!< Sensor Id */
	UINT32_t usDep1_Sensor_Id;			/*!< Dependent Sensor Id */
	UINT32_t usDep1_Sensor_Rate;		/*!< Dependent Sensor Rate*/
	UINT32_t usFifo_op_addr;			/*!< Fifo output address */
	UINT32_t usBatch_size;				/*!< Batching Size in bytes */
	UINT32_t usBatch_flush;
	UINT32_t usBatch_group_index;
	UINT32_t uiBatching_Mode;
	UINT32_t uiElapsed_Batch_Time;
	UINT32_t uiBatched_Packet_Count;
	UINT32_t usBG_Enable;				/*!< Always Enabled flag */
	UINT32_t output_rate;				/*!< Algorithm Output Rate */
	UINT32_t mb_value;					/*!< Mailbox Value */

}xSensor_alg_config_1;

/*! \struct xSensor_alg_config_2
 *	\brief	Sensor Configuration Parameters for two sensor dependency
 */
typedef struct
{
	UINT32_t usSensor_Id;				/*!< Sensor Id */
	UINT32_t usDep1_Sensor_Id;			/*!< Dependent1 Sensor Id */
	UINT32_t usDep1_Sensor_Rate;		/*!< Dependent1 Sensor Rate*/
	UINT32_t usDep2_Sensor_Id;			/*!< Dependent2 Sensor Id */
	UINT32_t usDep2_Sensor_Rate;		/*!< Dependent2 Sensor Rate*/
	UINT32_t usFifo_op_addr;			/*!< Fifo output address */
	UINT32_t usBatch_size;				/*!< Batching Size in bytes */
	UINT32_t usBatch_flush;
	UINT32_t usBatch_group_index;
	UINT32_t uiBatching_Mode;
	UINT32_t uiElapsed_Batch_Time;
	UINT32_t uiBatched_Packet_Count;
	UINT32_t usBG_Enable;				/*!< Always Enabled flag */
	UINT32_t output_rate;				/*!< Algorithm Output Rate */
	UINT32_t mb_value;					/*!< Mailbox Value */

}xSensor_alg_config_2;

/*! \struct xExport_var_t
 *	\brief	Export variable structure
 */
typedef struct
{
	UINT8_t 	ucId;		/*!< Export Section ID */
	UINT8_t 	ucMemType;      /*!< Export Section Memory Type */
	UINT16_t	usAddr;         /*!< Export Section Address offset */
	UINT32_t 	ulLen;		/*!< Export Section length */

}xExport_var_t;
/// @endcond

/*! \enum       eHAL_FFE_CmdType
 *  \brief	HAL FFE Command Type
 */
typedef enum
{
	eFFE_SENSOR_ENABLE = 1,
	eFFE_SENSOR_RATE_CHANGE,
	eFFE_SENSOR_BATCH_SIZE,
	eFFE_SENSOR_BATCH_FLUSH,
	eFFE_SENSOR_OUTPUT_DISABLE
}eHAL_FFE_CmdType;

/*! \struct     FFE_FifoCfgTypeDef
 *  \brief	HAL FIFO Configuration structure
 */
typedef struct
{
	UINT8_t			ucFifoSrc;			/*!< FIFO source */
	UINT8_t			ucFifoDest;			/*!< FIFO destination */
	UINT8_t			ucFifoType;			/*!< FIFO type */
	UINT8_t 		ucIntSetting;		        /*!< FIFO Interrupt Settings */
	UINT16_t		usPopThreshIntCnt;	        /*!< FIFO threshold in terms of words (32-bit) */
	UINT16_t 		usPushThreshIntCnt;	        /*!< FIFO Push Threshold Interrupt Count */

}FFE_FifoCfgTypeDef;

/*! \struct     FFE_HandleTypeDef
 *  \brief	HAL FFE Configuration structure
 */
typedef struct
{
	FFE_FifoCfgTypeDef			FifoCfg;						/*!< config parameters for FIFO */
	UINT8_t 				ucRunOnce;						/*!< Flag to enable FFE to run in single step mode */
	UINT8_t  				ucRunContinous;					        /*!< Flag to enable FFE to run in continuous mode */
	UINT8_t  				ucStopFFE;						/*!< Flag to stop the FFE execution */
	UINT8_t					ucHaltFFE;						/*!< Flag to halt the FFE execution */
	UINT8_t					ucReleaseFFE;					        /*!< Flag to release the FFE halt */
	UINT32_t 				ulRunCnt;						/*!< FFE run count */
	UINT32_t 				ulFFEFreq;						/*!< FFE frequency */

}FFE_HandleTypeDef;

/*!
 * \fn 		HAL_StatusTypeDef HAL_FFE_GetAlgParams(UINT8_t usSensId,xSensor_alg_config_1 *config1,xSensor_alg_config_2 *config2);
 * \brief 	This function returns the Alg configuration parameters
 * \param 	ucSensId   --- sensor ID
 * \param   config1    --- pointer to Config 1 structure
 * \param	config2    --- pointer to Config 2 structure
 * \return	HAL Status
 */
HAL_StatusTypeDef HAL_FFE_GetAlgParams(UINT8_t ucSensId,xSensor_alg_config_1 **config1,xSensor_alg_config_2 **config2);
/*!
 * \fn 		HAL_StatusTypeDef HAL_FFE_SetAlgParams(UINT8_t *ucBuf, UINT8_t ucCmdType)
 * \brief 	This function configures the Alg parameters inside FFE
 * \param 	ucBuf	     --- User Command buffer
 * \param       ucCmdType    --- Command Type
 * \return	HAL Status
 */
HAL_StatusTypeDef HAL_FFE_SetAlgParams(UINT8_t *ucBuf, UINT8_t ucCmdType);
/*!
 * \fn 		HAL_StatusTypeDef HAL_FFE_WriteMem(volatile UINT32_t *ulMem_addr, UINT32_t ulLen, void *buf)
 * \brief 	This function write into FFE Data memory section
 * \param 	ulMem_addr -- DM Memory Address
 * \param 	ulLen 	   -- Number of bytes to write
 * \param 	buf        -- Data buffer to write
 * \return      HAL Status
 */
HAL_StatusTypeDef HAL_FFE_WriteMem32(volatile UINT32_t *ulMem_addr, UINT32_t value);

HAL_StatusTypeDef HAL_FFE_WriteMem(volatile UINT32_t *ulMem_addr, UINT32_t ulLen, void *buf);
/*!
 * \fn 		HAL_StatusTypeDef HAL_FFE_ReadMem(volatile UINT32_t *ulMem_addr, UINT32_t ulLen, void *buf)
 * \brief 	This function read from FFE Data memory section
 * \param 	ulMem_addr -- DM Memory Address
 * \param 	ulLen 	   -- Number of bytes to read
 * \param 	buf        -- Data buffer to read
 * \return      HAL Status
 */
HAL_StatusTypeDef HAL_FFE_ReadMem(volatile UINT32_t *ulMem_addr, UINT32_t ulLen, void *buf);
/*!
 * \fn 		HAL_StatusTypeDef HAL_FFE_Config(FFE_HandleTypeDef *hFFE)
 * \brief 	This function configure the FFE 
 * \param 	hFFE -- pointer to FFE Handle structure
 * \return      HAL Status
 */
HAL_StatusTypeDef HAL_FFE_Config(FFE_HandleTypeDef *hFFE);
/*!
 * \fn 		HAL_StatusTypeDef HAL_FFE_Init(void)
 * \brief 	This function initialize the FFE 
 * \return      HAL Status
 */
HAL_StatusTypeDef HAL_FFE_Init(void);
/*!
 * \fn 		HAL_StatusTypeDef HAL_FFE_DeInit(FFE_HandleTypeDef *hFFE)
 * \brief 	This function deinitialize the FFE 
 * \return      HAL Status
 */
HAL_StatusTypeDef HAL_FFE_DeInit(FFE_HandleTypeDef *hFFE);


void HAL_FFE_ENABLE_INT(void);
/// @cond HAL_FFE_INTERNAL_API

/*!
 * \fn          UINT32_t get_algsenabled_cnt(UINT32_t ulMailboxVal)
 * \brief       Computes the total number of algorithms enabled based on mailbox value
 * \param       ulMailboxVal -- Mailbox value
 * \return      Alg count
 */
static UINT32_t FFE_GetAlgsEnabledCnt(UINT32_t ulMailboxVal);
/*!
 * \fn 	        void FFE_Compute_New_Samplerate(UINT16_t usMailbox)
 * \brief       Based on mailbox value, it will check if any other algorithm is enabled,
 *              if enabled then change the FFE sample time to the next max sample rate. *
 * \param       usMailbox -- Mailbox bit.
 */
static void FFE_Compute_New_Samplerate(UINT16_t usMailbox);
/*!
 * \fn          void FFE_UpdateCfg(UINT8_t ucMbUpdate, xSensor_alg_config_1 *cfg1, xSensor_alg_config_2 *cfg2)
 * \brief       This function updates the new FFE configuration.
 * \param       ucMbUpdate -- Flag to set if Mailbox update is needed or not
 * \param       *cfg1	   -- Pointer to Sensor_alg_Config1 structure
 * \param       *cfg2 	   -- Pointer to Sensor_alg_config2 structure * 
 */
static void FFE_UpdateCfg(UINT8_t ucMbUpdate, xSensor_alg_config_1 *cfg1, xSensor_alg_config_2 *cfg2);
/*!
 * \fn          static HAL_StatusTypeDef FFE_Get_AlgConfig(UINT8_t ucId, xSensor_alg_config_1 **config_1, xSensor_alg_config_2 **config_2)
 * \brief       Traverse through sensor config structure list to identify corresponding sensor configuration
 * \param       Id      -- sensor ID
 * \param       config1 -- pointer to sensor configuration for single dependency sensor
 * \param       config2 -- pointer to sensor configuration for two dependency sensor
 * \return      HAL Status
 */
static HAL_StatusTypeDef FFE_Get_AlgConfig(UINT8_t ucId, xSensor_alg_config_1 **config_1, xSensor_alg_config_2 **config_2);
/*!
 * \fn 		static HAL_StatusTypeDef FFE_Parse_ExpoSection(void)
 * \brief 	This function parses the export variables section in M4 SRAM
 * \param 	None
 * \return      HAL Status
 */
static HAL_StatusTypeDef FFE_Parse_ExpoSection(void);
/*!
 * \fn 		static UINT8_t FFE_CheckBitEnable(UINT32_t ulVar)
 * \brief 	Compute the enabled bit position from Mailbox value
 * \param 	ulVar -- Algorithm mailbox value
 * \return	bit index which is set to 1.
 */
static UINT8_t FFE_CheckBitEnable(UINT32_t ulVar);
/*!
 * \fn 		static void FFE_Get_SensIdInfo(UINT8_t *ucSensId)
 * \brief 	Traverse through the alg structure and store the sensor IDs
 * \brief 	in an array based on mailbox bit position
 * \param 	ucSensID -- Sensor ID
 */
static void FFE_Get_SensIdInfo(UINT8_t *ucSensId);
/*!
 * \fn		static void FFE_Enable(UINT8_t ucEnable)
 * \brief 	Function to enable power & clock domains for FFE subsystem
 * \param	ucEnable --- Enable Flag
 */
static void FFE_Enable(UINT8_t ucEnable);
/*!
 * \fn 		static void FFE_Set_Sampletime(UINT16_t usTimeinMS)
 * \brief 	This function sets the new FFE sample time
 * \param 	usTimeinMS -- Sample Time in milliseconds
 */
static void FFE_Set_Sampletime(UINT16_t usTimeinMS);
/*!
 * \fn 		static void FFE_AlgEnable(UINT8_t *ucBuf,xSensor_alg_config_1 *config1,xSensor_alg_config_2 *config2)
 * \brief	Function to enable the corresponding Algorithm inside FFE
 * \param	ucBuf	---	User Command buffer
 * \param	config1	--- pointer to alg config1 structure
 * \param	config2	--- pointer to alg config2 structure
 */
static void FFE_AlgEnable(UINT8_t *ucBuf,xSensor_alg_config_1 *config1,xSensor_alg_config_2 *config2);
/*!
 * \fn 		static void FFE_SampleRateCfg(UINT8_t *ucBuf, xSensor_alg_config_1 *config1,xSensor_alg_config_2 *config2)
 * \brief	Function to configure the FFE sample rate inside FFE
 * \param	ucBuf	--- User Command buffer
 * \param	config1	--- pointer to alg config1 structure
 * \param	config2	--- pointer to alg config2 structure
 */
static void FFE_SampleRateCfg(UINT8_t *ucBuf, xSensor_alg_config_1 *config1,xSensor_alg_config_2 *config2);
/*!
 * \fn 		static void FFE_CfgBatchSize(UINT8_t *ucBuf, xSensor_alg_config_1 *config1,xSensor_alg_config_2 *config2)
 * \brief	Function to configure the batch size inside FFE
 * \param	ucBuf	--- User Command buffer
 * \param	config1	--- pointer to alg config1 structure
 * \param	config2	--- pointer to alg config2 structure
 */
static void FFE_CfgBatchSize(UINT8_t *ucBuf, xSensor_alg_config_1 *config1,xSensor_alg_config_2 *config2);
/*!
 * \fn 		static void FFE_AlgOutputCfg(UINT8_t *ucBuf, xSensor_alg_config_1 *config1,xSensor_alg_config_2 *config2)
 * \brief	Function to configure the sensor packet output mode configure inside FFE
 * \param	ucBuf	--- User Command buffer
 * \param	config1	--- pointer to alg config1 structure
 * \param	config2	--- pointer to alg config2 structure
 */
static void FFE_AlgOutputCfg(UINT8_t *ucBuf, xSensor_alg_config_1 *config1,xSensor_alg_config_2 *config2);
/*!
 * \fn 		static HAL_StatusTypeDef FFE_ConfigureSM(UINT8_t ucSM_type)
 * \brief 	This function configures SM 0/1 SDA and SCL pads
 * \param 	SM_type -- Sensor Manager type (SM0/SM1)
 * \return	Status
 */
static HAL_StatusTypeDef FFE_ConfigureSM(UINT8_t ucSM_type);


/// @endcond

#endif /* HAL_INC_EOSS3_HAL_FFE_H_ */
