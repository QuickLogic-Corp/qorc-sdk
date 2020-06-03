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
 *    File   : eoss3_hal_pkfb.c
 *    Purpose: This file contains source related to Packet FIFO
 *             communication interfac
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#include <stdint.h>
#include "eoss3_dev.h"
#include "eoss3_hal_pkfb.h"
//#include "eoss3_hal_rcc.h"
//#include "eoss3_hal_clock.h"
#include "s3x_clock_hal.h"

void HAL_FIFO_Enable(FIFO_Type eFifoID)
{
	FIFOx_CTRL_ENABLE(eFifoID);
}

void HAL_FIFO_Disable(FIFO_Type eFifoID)
{
	FIFOx_CTRL_DISABLE(eFifoID);
}

UINT32_t HAL_FIFO_Read(FIFO_Type eFifoID, INT32_t iLen, UINT32_t *puiRxBuf)
{
	UINT32_t uiAvailableLength = 0;
	UINT32_t uiReadLength = 0, uiReturnValue = 0;
	UINT32_t *puiData = puiRxBuf;

	if(iLen == 0 && puiData == 0)
		return 0;

    /* Check how many words available in FIFO for read*/
	uiAvailableLength = FIFOx_GET_POP_COUNT(eFifoID);
	if(iLen == -1)
		iLen = uiAvailableLength;

///////////////////////////////////// how to read pop vs write push
	/* Check if there is length mismatch. Fifo size is not enough. */
	if(iLen > uiAvailableLength)
		uiReadLength = uiAvailableLength;
	else
		uiReadLength = iLen;

	uiReturnValue = uiReadLength;

	while(uiReadLength--)
		*puiData++ = FIFOx_DATA_READ(eFifoID);

	return uiReturnValue;
}

UINT32_t HAL_FIFO8K_Read(FIFO_Type eFifoID, INT32_t iLen, UINT16_t *pusRxBuf)
{
	UINT32_t uiAvailableLength = 0;
	UINT32_t uiReadLength = 0, uiReturnValue = 0;
	UINT16_t *pusData = pusRxBuf;

	if(iLen == 0 && pusData == 0)
		return 0;

    /* Check how many words available in FIFO for read*/
	uiAvailableLength = FIFOx_GET_POP_COUNT(eFifoID);
	if(iLen == -1)
		iLen = uiAvailableLength;

///////////////////////////////////// how to read pop vs write push
	/* Check if there is length mismatch. Fifo size is not enough. */
	if(iLen > uiAvailableLength)
		uiReadLength = uiAvailableLength;
	else
		uiReadLength = iLen;

	uiReturnValue = uiReadLength;

	while(uiReadLength--)
		*pusData++ = FIFOx_DATA_READ(eFifoID);

	return uiReturnValue;
}

UINT32_t HAL_FIFO_Write(FIFO_Type eFifoID, INT32_t iLen, UINT32_t *puiTxBuf)
{
	UINT32_t uiAvailableLength = 0;
	UINT32_t uiWriteLength = 0, uiReturnValue = 0;
	UINT32_t *puiData = puiTxBuf;

	if(iLen <= 0 && puiData == 0)
		return 0;

//	uiReturnValue = IS_FIFOx_STATUS_SRAM_ACTIVE(eFifoID);

    /* Check how many words available in FIFO for write*/
	uiAvailableLength = FIFOx_GET_PUSH_COUNT(eFifoID);
	if(iLen > uiAvailableLength)
		uiWriteLength = uiAvailableLength;
	else
		uiWriteLength = iLen;

	uiReturnValue = uiWriteLength;

	while(uiWriteLength--)
	{
		FIFOx_DATA_WRITE(eFifoID, *puiData++);
	}

	return uiReturnValue;
}

UINT32_t HAL_FIFO8K_Write(FIFO_Type eFifoID, INT32_t iLen, UINT16_t *pusTxBuf)
{
	UINT32_t uiAvailableLength = 0;
	UINT32_t uiWriteLength = 0, uiReturnValue = 0;
	UINT16_t *pusData = pusTxBuf;

	if(iLen <= 0 && pusData == 0)
		return 0;

//	uiReturnValue = IS_FIFOx_STATUS_SRAM_ACTIVE(eFifoID);

    /* Check how many words available in FIFO for write*/
	uiAvailableLength = FIFOx_GET_PUSH_COUNT(eFifoID);
	if(iLen > uiAvailableLength)
		uiWriteLength = uiAvailableLength;
	else
		uiWriteLength = iLen;

	uiReturnValue = uiWriteLength;

	while(uiWriteLength--)
	{
		FIFOx_DATA_WRITE(eFifoID, *pusData++);
	}

	return uiReturnValue;
}

void HAL_FIFO_PushSleepEnable(FIFO_Type eFifoID)
{
	FIFOx_PUSH_SLEEP_ENABLE(eFifoID);
}

void HAL_FIFO_PushSleepDisable(FIFO_Type eFifoID)
{
	FIFOx_PUSH_SLEEP_DISABLE(eFifoID);
}

void HAL_FIFO_PopSleepEnable(FIFO_Type eFifoID)
{
	FIFOx_POP_SLEEP_ENABLE(eFifoID);
}

void HAL_FIFO_PopSleepDisable(FIFO_Type eFifoID)
{
	FIFOx_POP_SLEEP_DISABLE(eFifoID);
}

UINT32_t HAL_FIFO_PopCount(FIFO_Type eFifoID)
{
	return FIFOx_GET_POP_COUNT(eFifoID);
}

UINT32_t HAL_FIFO_PushCount(FIFO_Type eFifoID)
{
	return FIFOx_GET_PUSH_COUNT(eFifoID);
}

HAL_StatusTypeDef HAL_FIFO_Init(FIFO_Config xFifoConfig)
{
	/* Initialize power for the FIFOs */
	HAL_FIFO_PowerInit(xFifoConfig.eFifoID);

	/* Initialize clock for the FIFOs */
	HAL_FIFO_ClkInit(xFifoConfig.eFifoID);

	/* Set up the FIFO Source */
	switch(xFifoConfig.eSrc)
	{
	case FIFO_SRC_M4:
		FIFOx_CTRL_PUSH_M4(xFifoConfig.eFifoID);
		break;
	case FIFO_SRC_FFE0:
		FIFOx_CTRL_PUSH_FFE(xFifoConfig.eFifoID);
		FIFOx_CTRL_FFE0_SEL(xFifoConfig.eFifoID);
		break;
	case FIFO_SRC_FFE1:
		FIFOx_CTRL_PUSH_FFE(xFifoConfig.eFifoID);
		FIFOx_CTRL_FFE1_SEL(xFifoConfig.eFifoID);
		break;
	}

	/* Setup FIFO destination */
	switch(xFifoConfig.eDest)
	{
	case FIFO_DEST_M4:
		FIFOx_CTRL_POP_M4(xFifoConfig.eFifoID);
		break;
	case FIFO_DEST_AP:
		FIFOx_CTRL_POP_AP(xFifoConfig.eFifoID);
		break;
	}

	return HAL_OK;
}

HAL_StatusTypeDef HAL_FIFO_ConfigInt(FIFO_IntConfig xFifoIntConfig)
{
  return HAL_OK;
}

HAL_StatusTypeDef HAL_FIFO_PowerInit(FIFO_Type eFifoID)
{
	UINT32_t uiPFStatus = 0;

	/* Enable Power for FIFO */
	uiPFStatus = PMU->PF_STATUS;
	/* Replace below source with HAL APIs for power domain */
	PMU->FFE_FB_PF_SW_WU |= 0x4;			// Wake up power domain. Auto clear.

	while(uiPFStatus != 1)
	{
		/* REMOVE THIS LOOP AFTER UNIT TESTING/VERIFICATION FOR FIRST TIME */
		uiPFStatus = PMU->PF_STATUS;
	}
    return HAL_OK;
}

void HAL_FIFO_ClkInit(FIFO_Type eFifoID)
{
	S3x_Clk_Enable(S3X_PKT_FIFO_CLK);
	FIFOx_CTRL_ENABLE(eFifoID);

}
void HAL_FIFO_ClkDeInit(FIFO_Type eFifoID)
{
  //HAL_SetClkGate(EFUSE_SDMA_I2S_FFE_PF_CLK_TOP, C01_CLK_GATE_PK_FIFO,1);
	/* TODO check why its is enable here */
  S3x_Clk_Disable(S3X_PKT_FIFO_CLK);
}
