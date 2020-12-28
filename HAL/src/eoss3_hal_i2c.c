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

/*!	\file eoss3_hal_i2c.c
 *
 *  \brief This file contains API implementation for the I2C
 *         Controller(s) in the EOS S3
 */
#include "Fw_global_config.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <eoss3_dev.h>
#include <eoss3_hal_def.h>
#include <s3x_clock_hal.h>
//#include <eoss3_hal_rcc.h>
#include <eoss3_hal_i2c.h>
#include <eoss3_hal_pad_config.h>
#include "test_types.h"
#include <eoss3_hal_wb.h>
//#include "eoss3_hal_power.h"
#include "Fw_global_config.h"

#ifdef __RTOS
#include <FreeRTOS.h>
#include <semphr.h>
#endif
#define delayCycles(_x_)	do {							\
					volatile unsigned int _delayCycleCount = _x_;	\
					while (_delayCycleCount--);			\
				} while(0)

/* This variable holds I2C status */
I2C_State eI2CState = I2C_RESET;
UINT8_t   ucI2CSlaveID = WB_ADDR_I2C0_SLAVE_SEL;

HAL_StatusTypeDef HAL_I2C0_Select()
{
  ucI2CSlaveID = WB_ADDR_I2C0_SLAVE_SEL;
  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C1_Select()
{
  ucI2CSlaveID = WB_ADDR_I2C1_SLAVE_SEL;
  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Init(I2C_Config xI2CConfig)
{
    if(xI2CConfig.ucI2Cn == 0)
    {
    	HAL_WB_Init(WB_ADDR_I2C0_SLAVE_SEL);
	HAL_I2C0_Select();
    }
    if(xI2CConfig.ucI2Cn == 1)
    {
	HAL_WB_Init(WB_ADDR_I2C1_SLAVE_SEL);
	HAL_I2C1_Select();
    }

    HAL_WB_Transmit(I2C_MCR, (UINT8_t)~(1<<I2C_CR_EN_BIT), ucI2CSlaveID);

    /* Set I2C clock frequency */
    HAL_I2C_SetClockFreq(xI2CConfig.eI2CFreq);

    /* Check if Interrupt needs to be enable */
    if(xI2CConfig.eI2CInt)
          HAL_WB_Transmit(I2C_MCR, 1<<I2C_CR_IEN_BIT, ucI2CSlaveID);

    HAL_WB_Transmit(I2C_MCR, 1<<I2C_CR_EN_BIT, ucI2CSlaveID);

	eI2CState = I2C_READY;
	return HAL_OK;
}
/*
* Note:  12-14-2018
* The Datasheet seems to be not updated to the correct RTL implementation.
* The macro HAL_I2C_CLK_PRESCALE is incorrect due to this.
*      I2C_FREQ ? (SYS_FREQ/(I2C_FREQ*5))-1
*
* The computation should be according to the following formula.
* For pre-scale  = 0:
*
*  Default I2C frequency  (with control register bit [4] = bit[5] = 0)  =  Sys Freq. / 12
* 
* For pre-scale  > 0:
*
*  I2C SCL Freq  = Sys Freq / (5 * (pre-scale +1) + X ,       
*        1. Where X  is added because of the clock stretching support, filtering of the feedback path 
*           to know whether clock is being stretched or not. 
*        2. X varies according to the pre-scale value.
*           For pre-scale (1-3 ), X = 5
*           For pre-scale (4-7 ), X = 7
*           For pre-scale (8-11 ), X = 9 
*           For pre-scale (12-15 ), X =11 and So On
*
* So, the values are precomputed for a lookup in a Table
*/
#define CHECK_DIV_ARRAY_SIZE  16
//static int divArray[CHECK_DIV_ARRAY_SIZE]      = {12,15,20,25,32,37,42,47,54,59,64,69,76,81,86,91};
static int checkDivArray[CHECK_DIV_ARRAY_SIZE] = {13,17,22,29,34,39,44,50,56,61,66,72,78,83,88,91};
//int checkDivArraySize = 16;
static int CalculatePreScale(float divFactor)
{
    int i;
    for (i = 0; i < CHECK_DIV_ARRAY_SIZE; i++)
    {
        if (divFactor <= checkDivArray[i])
        {
            return i;
        }
    }
    return CHECK_DIV_ARRAY_SIZE-1;
}

HAL_StatusTypeDef HAL_I2C_SetClockFreq(UINT32_t uiClkFreq)
{
    UINT32_t uiClock, uiPrescale;
    UINT8_t val;
   //Set the frequency
    uiClock = S3x_Clk_Get_Rate(S3X_FFE_X1_CLK);

    // Program prescale value
#if 1 //use new formula to compute    
    uiPrescale = CalculatePreScale(uiClock*1.0/(uiClkFreq*100*1000));
#else  //this is incorrect according to RTL   
    uiPrescale = HAL_I2C_CLK_PRESCALE(uiClock, (uiClkFreq*100*1000));
#endif    
//    HAL_I2C_WRITE_PRESCALE(uiPrescale);
    HAL_WB_Transmit(I2C_PRELO, uiPrescale&0xFF, ucI2CSlaveID);
    HAL_WB_Transmit(I2C_PREHI, (uiPrescale>>8)&0xFF, ucI2CSlaveID);

    HAL_WB_Receive(I2C_PRELO, &val, ucI2CSlaveID);

    return HAL_OK;
}


HAL_StatusTypeDef HAL_I2C_Write(UINT8_t ucDevAddress, UINT8_t ucAddress, UINT8_t *pucDataBuf, UINT32_t uiLength)
{
  UINT8_t *pucData = pucDataBuf;
  UINT32_t uiLen = uiLength-1;
  UINT8_t ucI2CStatus = 0;

  if(eI2CState != I2C_READY)
    return HAL_BUSY;

  if(!ucDevAddress || pucDataBuf == NULL)
    return HAL_ERROR;

  if(!uiLength)
    return HAL_OK;

  eI2CState = I2C_BUSY;

  /* Request for write */
  if(HAL_WB_Transmit(I2C_TXRX_DR, ((ucDevAddress<<1) & (~1)), ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Generate command with start condition and write cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, CMD_START_BIT | CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  /* Read acknowledge from slave */
  if((ucI2CStatus & (1<<SR_RXACK_BIT)))
  {
      	eI2CState = I2C_READY;
    	return HAL_ERROR;
  }
  /* Write address */
  if(HAL_WB_Transmit(I2C_TXRX_DR, ucAddress, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Generate command with stop condition and write cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  while(uiLen--)
  {
    /* Check if Transfer completed */
    do
    {
      HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
    }while(ucI2CStatus & (1<<SR_TIP_BIT));

    /* Read acknowledge from slave */
    if((ucI2CStatus & (1<<SR_RXACK_BIT)))
    {
      	eI2CState = I2C_READY;
      	return HAL_ERROR;
    }

    /* Write data */
    if(HAL_WB_Transmit(I2C_TXRX_DR, *pucData++, ucI2CSlaveID) != HAL_OK)
    {
      	eI2CState = I2C_READY;
	return HAL_ERROR;
    }

    /* Generate command with write cycle */
    if(HAL_WB_Transmit(I2C_CMD_SR, CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
    {
      	eI2CState = I2C_READY;
	return HAL_ERROR;
    }
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  /* Write data */
  if(HAL_WB_Transmit(I2C_TXRX_DR, *pucData++, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Generate command with stop condition and write cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, CMD_STOP_BIT | CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  eI2CState = I2C_READY;

  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Read(UINT8_t ucDevAddress, UINT8_t ucAddress, UINT8_t *pucDataBuf, UINT32_t uiLength)
{
  UINT8_t *pucData = pucDataBuf;
  UINT32_t uiLen = uiLength-1;
  UINT8_t ucI2CStatus = 0;

  if(eI2CState != I2C_READY)
    return HAL_BUSY;

  if(!ucDevAddress || pucDataBuf == NULL)
    return HAL_ERROR;

  if(!uiLength)
    return HAL_OK;

  eI2CState = I2C_BUSY;

  /* Request for write */
  if(HAL_WB_Transmit(I2C_TXRX_DR, ((ucDevAddress<<1) & (~1)), ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Generate command with start condition and write cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, CMD_START_BIT | CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  /* Read acknowledge from slave */
  if((ucI2CStatus & (1<<SR_RXACK_BIT)))
  {
        eI2CState = I2C_READY;
    	return HAL_ERROR;
  }

  /* Write address */
  if(HAL_WB_Transmit(I2C_TXRX_DR, ucAddress, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Generate command with stop condition and write cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, CMD_STOP_BIT | CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  /* Request for Read */
  if(HAL_WB_Transmit(I2C_TXRX_DR, ((ucDevAddress<<1) | 1), ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Generate command with start condition and write cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, CMD_START_BIT | CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  /* Read acknowledge from slave */
  if((ucI2CStatus & (1<<SR_RXACK_BIT)))
  {
      	eI2CState = I2C_READY;
    	return HAL_ERROR;
  }

  while(uiLen--)
  {
    /* Generate command with ACK and READ cycle */
    if(HAL_WB_Transmit(I2C_CMD_SR, CMD_READ_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
    {
      	eI2CState = I2C_READY;
    	return HAL_ERROR;
    }

    /* Check if Transfer completed */

    do
    {
       HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
    }while(ucI2CStatus & (1<<SR_TIP_BIT));

    /* Read acknowledge from slave */
    if((ucI2CStatus & (1<<SR_RXACK_BIT)))
    {
        eI2CState = I2C_READY;
    	return HAL_ERROR;
    }

  /* Read data */
    HAL_WB_Receive(I2C_TXRX_DR, pucData++, ucI2CSlaveID);
  }

  /* Generate command with ACK and READ cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, CMD_STOP_BIT | CMD_NACK_BIT | CMD_READ_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
    	return HAL_ERROR;
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  /* Read data */
  HAL_WB_Receive(I2C_TXRX_DR, pucData, ucI2CSlaveID);

  eI2CState = I2C_READY;

  return HAL_OK;
}


HAL_StatusTypeDef HAL_I2C_Read16(UINT8_t ucDevAddress, UINT16_t ucAddress, UINT8_t *pucDataBuf, UINT32_t uiLength)
{
  UINT8_t *pucData = pucDataBuf;
  UINT32_t uiLen = uiLength-1;
  UINT8_t ucI2CStatus = 0;

  if(eI2CState != I2C_READY)
    return HAL_BUSY;

  if(!ucDevAddress || pucDataBuf == NULL)
    return HAL_ERROR;

  if(!uiLength)
    return HAL_OK;

  eI2CState = I2C_BUSY;

  /* Request for write */
  if(HAL_WB_Transmit(I2C_TXRX_DR, ((ucDevAddress<<1) & (~1)), ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Generate command with start condition and write cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, CMD_START_BIT | CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  /* Read acknowledge from slave */
  if((ucI2CStatus & (1<<SR_RXACK_BIT)))
  {
        eI2CState = I2C_READY;
    	return HAL_ERROR;
  }

  /* Write address */
  if(HAL_WB_Transmit(I2C_TXRX_DR, (ucAddress>>8)&0xff, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

    /* Generate command with write cycle */
    if(HAL_WB_Transmit(I2C_CMD_SR, CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
    {
      	eI2CState = I2C_READY;
	return HAL_ERROR;
    }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  if(HAL_WB_Transmit(I2C_TXRX_DR, ucAddress&0xff, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }
  
  /* Generate command with stop condition and write cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, CMD_STOP_BIT | CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  /* Request for Read */
  if(HAL_WB_Transmit(I2C_TXRX_DR, ((ucDevAddress<<1) | 1), ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Generate command with start condition and write cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, CMD_START_BIT | CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  /* Read acknowledge from slave */
  if((ucI2CStatus & (1<<SR_RXACK_BIT)))
  {
      	eI2CState = I2C_READY;
    	return HAL_ERROR;
  }

  while(uiLen--)
  {
    /* Generate command with ACK and READ cycle */
    if(HAL_WB_Transmit(I2C_CMD_SR, CMD_READ_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
    {
      	eI2CState = I2C_READY;
    	return HAL_ERROR;
    }

    /* Check if Transfer completed */

    do
    {
       HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
    }while(ucI2CStatus & (1<<SR_TIP_BIT));

    /* Read acknowledge from slave */
    if((ucI2CStatus & (1<<SR_RXACK_BIT)))
    {
        eI2CState = I2C_READY;
    	return HAL_ERROR;
    }

  /* Read data */
    HAL_WB_Receive(I2C_TXRX_DR, pucData++, ucI2CSlaveID);
  }

  /* Generate command with ACK and READ cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, CMD_STOP_BIT | CMD_NACK_BIT | CMD_READ_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
    	return HAL_ERROR;
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  /* Read data */
  HAL_WB_Receive(I2C_TXRX_DR, pucData, ucI2CSlaveID);

  eI2CState = I2C_READY;

  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Write16(UINT8_t ucDevAddress, UINT16_t ucAddress, UINT8_t *pucDataBuf, UINT32_t uiLength)
{
  UINT8_t *pucData = pucDataBuf;
  UINT32_t uiLen = uiLength-1;
  UINT8_t ucI2CStatus = 0;

  if(eI2CState != I2C_READY)
    return HAL_BUSY;

  if(!ucDevAddress || pucDataBuf == NULL)
    return HAL_ERROR;

  if(!uiLength)
    return HAL_OK;

  eI2CState = I2C_BUSY;
  //HAL_SetPowerDomainState(FFE,WAKEUP);
  delayCycles(50);

  /* Request for write */
  if(HAL_WB_Transmit(I2C_TXRX_DR, ((ucDevAddress<<1) & (~1)), ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Generate command with start condition and write cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, CMD_START_BIT | CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  /* Read acknowledge from slave */
  if((ucI2CStatus & (1<<SR_RXACK_BIT)))
  {
      	eI2CState = I2C_READY;
    	return HAL_ERROR;
  }
  
  /* Write address */
  if(HAL_WB_Transmit(I2C_TXRX_DR, (ucAddress>>8)&0xff, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

    /* Generate command with write cycle */
    if(HAL_WB_Transmit(I2C_CMD_SR, CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
    {
      	eI2CState = I2C_READY;
	return HAL_ERROR;
    }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  /* Write address */
  if(HAL_WB_Transmit(I2C_TXRX_DR, ucAddress&0xff, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Generate command with stop condition and write cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  while(uiLen--)
  {
    /* Check if Transfer completed */
    do
    {
      HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
    }while(ucI2CStatus & (1<<SR_TIP_BIT));

    /* Read acknowledge from slave */
    if((ucI2CStatus & (1<<SR_RXACK_BIT)))
    {
      	eI2CState = I2C_READY;
      	return HAL_ERROR;
    }

    /* Write data */
    if(HAL_WB_Transmit(I2C_TXRX_DR, *pucData++, ucI2CSlaveID) != HAL_OK)
    {
      	eI2CState = I2C_READY;
	return HAL_ERROR;
    }

    /* Generate command with write cycle */
    if(HAL_WB_Transmit(I2C_CMD_SR, CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
    {
      	eI2CState = I2C_READY;
	return HAL_ERROR;
    }
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  /* Write data */
  if(HAL_WB_Transmit(I2C_TXRX_DR, *pucData++, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Generate command with stop condition and write cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, CMD_STOP_BIT | CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  eI2CState = I2C_READY;

  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Read_UsingRestart(UINT8_t ucDevAddress, UINT8_t ucAddress, UINT8_t *pucDataBuf, UINT32_t uiLength)
{
  UINT8_t *pucData = pucDataBuf;
  UINT32_t uiLen = uiLength-1;
  UINT8_t ucI2CStatus = 0;

  if(eI2CState != I2C_READY)
    return HAL_BUSY;

  if(!ucDevAddress || pucDataBuf == NULL)
    return HAL_ERROR;

  if(!uiLength)
    return HAL_OK;

  eI2CState = I2C_BUSY;

  /* Request for write */
  if(HAL_WB_Transmit(I2C_TXRX_DR, ((ucDevAddress<<1) & (~1)), ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Generate command with start condition and write cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, CMD_START_BIT | CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  /* Read acknowledge from slave */
  if((ucI2CStatus & (1<<SR_RXACK_BIT)))
  {
        eI2CState = I2C_READY;
    	return HAL_ERROR;
  }

  /* Write address */
  if(HAL_WB_Transmit(I2C_TXRX_DR, ucAddress, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Generate command with stop condition and write cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, /* CMD_STOP_BIT | */ CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
  	return HAL_ERROR;
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  /* Request for Read */
  if(HAL_WB_Transmit(I2C_TXRX_DR, ((ucDevAddress<<1) | 1), ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Generate command with start condition and write cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, CMD_START_BIT | CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  /* Read acknowledge from slave */
  if((ucI2CStatus & (1<<SR_RXACK_BIT)))
  {
      	eI2CState = I2C_READY;
    	return HAL_ERROR;
  }

  while(uiLen--)
  {
    /* Generate command with ACK and READ cycle */
    if(HAL_WB_Transmit(I2C_CMD_SR, CMD_READ_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
    {
      	eI2CState = I2C_READY;
    	return HAL_ERROR;
    }

    /* Check if Transfer completed */

    do
    {
       HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
    }while(ucI2CStatus & (1<<SR_TIP_BIT));

    /* Read acknowledge from slave */
    if((ucI2CStatus & (1<<SR_RXACK_BIT)))
    {
        eI2CState = I2C_READY;
    	return HAL_ERROR;
    }

  /* Read data */
    HAL_WB_Receive(I2C_TXRX_DR, pucData++, ucI2CSlaveID);
  }

  /* Generate command with ACK and READ cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, CMD_STOP_BIT | CMD_NACK_BIT | CMD_READ_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
    	return HAL_ERROR;
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  /* Read data */
  HAL_WB_Receive(I2C_TXRX_DR, pucData, ucI2CSlaveID);

  eI2CState = I2C_READY;

  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_ReadRawData(UINT8_t ucDevAddress, UINT8_t *pucDataBuf, UINT32_t uiLength)
{
  UINT8_t *pucData = pucDataBuf;
  UINT32_t uiLen = uiLength-1;
  UINT8_t ucI2CStatus = 0;

  if(eI2CState != I2C_READY)
    return HAL_BUSY;

  if(!ucDevAddress || pucDataBuf == NULL)
    return HAL_ERROR;

  if(!uiLength)
    return HAL_OK;

  eI2CState = I2C_BUSY;
  /* Request for Read */
  if(HAL_WB_Transmit(I2C_TXRX_DR, ((ucDevAddress<<1) | 1), ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Generate command with start condition and write cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, CMD_START_BIT | CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  /* Read acknowledge from slave */
  if((ucI2CStatus & (1<<SR_RXACK_BIT)))
  {
      	eI2CState = I2C_READY;
    	return HAL_ERROR;
  }

  while(uiLen--)
  {
    /* Generate command with ACK and READ cycle */
    if(HAL_WB_Transmit(I2C_CMD_SR, CMD_READ_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
    {
      	eI2CState = I2C_READY;
    	return HAL_ERROR;
    }

    /* Check if Transfer completed */

    do
    {
       HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
    }while(ucI2CStatus & (1<<SR_TIP_BIT));

    /* Read acknowledge from slave */
    if((ucI2CStatus & (1<<SR_RXACK_BIT)))
    {
        eI2CState = I2C_READY;
    	return HAL_ERROR;
    }

  /* Read data */
    HAL_WB_Receive(I2C_TXRX_DR, pucData++, ucI2CSlaveID);
  }

  /* Generate command with ACK and READ cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, CMD_STOP_BIT | CMD_NACK_BIT | CMD_READ_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
    	return HAL_ERROR;
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  /* Read data */
  HAL_WB_Receive(I2C_TXRX_DR, pucData, ucI2CSlaveID);

  eI2CState = I2C_READY;

  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_WriteRawData(UINT8_t ucDevAddress, UINT8_t *pucDataBuf, UINT32_t uiLength, int stop)
{
  UINT8_t *pucData = pucDataBuf;
  UINT32_t uiLen = (uiLength > 0) ? uiLength-1 : 0;
  UINT8_t ucI2CStatus = 0;

  if(eI2CState != I2C_READY)
    return HAL_BUSY;

  if(!ucDevAddress || pucDataBuf == NULL)
    return HAL_ERROR;

  eI2CState = I2C_BUSY;

  /* Request for write */
  if(HAL_WB_Transmit(I2C_TXRX_DR, ((ucDevAddress<<1) & (~1)), ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Generate command with start condition and write cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, CMD_START_BIT | CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
    	eI2CState = I2C_READY;
	return HAL_ERROR;
  }
  for (uint32_t ibyte = 0; ibyte < uiLen; ibyte++)
  {
    /* Check if Transfer completed */
    do
    {
      HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
    }while(ucI2CStatus & (1<<SR_TIP_BIT));

    /* Read acknowledge from slave */
    if((ucI2CStatus & (1<<SR_RXACK_BIT)))
    {
      	eI2CState = I2C_READY;
      	return HAL_ERROR;
    }

    /* Write data */
    if(HAL_WB_Transmit(I2C_TXRX_DR, *pucData++, ucI2CSlaveID) != HAL_OK)
    {
      	eI2CState = I2C_READY;
	return HAL_ERROR;
    }

    /* Generate command with write cycle */
    if(HAL_WB_Transmit(I2C_CMD_SR, CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
    {
      	eI2CState = I2C_READY;
 	return HAL_ERROR;
    }
  }

  /* Check if Transfer completed */
  do
  {
     HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  if (uiLength > 0)
  {
	  /* Write data */
	  if(HAL_WB_Transmit(I2C_TXRX_DR, *pucData++, ucI2CSlaveID) != HAL_OK)
	  {
			eI2CState = I2C_READY;
		return HAL_ERROR;
	  }
  }
  if (stop) {
	  stop = CMD_STOP_BIT;
  } else
  {
	  stop = 0;
  }
  /* Generate command with stop condition and write cycle */
  if(HAL_WB_Transmit(I2C_CMD_SR, stop | CMD_WRITE_SLAVE_BIT, ucI2CSlaveID) != HAL_OK)
  {
		eI2CState = I2C_READY;
	return HAL_ERROR;
  }

  /* Check if Transfer completed */
  do
  {
	 HAL_WB_Receive(I2C_CMD_SR, &ucI2CStatus, ucI2CSlaveID);
  }while(ucI2CStatus & (1<<SR_TIP_BIT));

  eI2CState = I2C_READY;

  return HAL_OK;
}
