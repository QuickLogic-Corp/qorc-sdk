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
 *    File   : eoss3_hal_spi.c
 *    Purpose: This file contains HAL API for SPI
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"
#ifndef SPI_WB_INTERFACE
/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "dbg_uart.h"
#include "sec_debug.h"
#include "eoss3_dev.h"
#include "eoss3_hal_spi.h"
#include "s3x_clock_hal.h"
#include "eoss3_hal_pad_config.h"

SPI_HandleTypeDef *SPIHandle;
static SemaphoreHandle_t xLockSPI1Bus, xWaitForIntr;

static int SPI_Instance_count = 0; // Number of SPI driver object instantiated
typedef union SPI_WR_WORD
{
  UINT8_t tb_r[2];
  UINT16_t tw_r;
}SPI_Wr;

/// @cond HAL_SPI_LOCAL_FUN
/*!
* \fn      void assert_failed(UINT8_t *file, UINT32_t line)
* \brief   This function trigger assert on invalid parameter
*/
void assert_failed(const char *file, int line)
{
	dbg_str_str("Wrong parameter value: file", file);
    dbg_str_int("line",line);
    dbg_fatal_error("assert-hal-spi");
  return;
}

/// @endcond

void unlock_spi1_bus(UINT8_t Ctx)
{
    if(xLockSPI1Bus)
    {
      if (Ctx == CTX_ISR)
      {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;;
        if (xSemaphoreGiveFromISR(xLockSPI1Bus,&xHigherPriorityTaskWoken) == pdTRUE)
        {
          portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        }
      }
      else
      {
        if (xSemaphoreGive(xLockSPI1Bus) != pdTRUE)
        {
          printf("[HAL SPI] : Error : unable to release lock to spibus1\n");
        }
      }
    }
}

void lock_spi1_bus(void)
{
    if(xLockSPI1Bus)
    {
      if (xSemaphoreTake(xLockSPI1Bus, portMAX_DELAY) != pdTRUE)
      {
            dbg_fatal_error("[HAL SPI] : Error unable to take lock to spibus1\n");
        return;
      }

    }
}


static void SPI_Enable(SPI_HandleTypeDef *hspi, UINT8_t ucEnable)
{
  if (ucEnable == 1)
    SPI_MS->SSIENR = SSIENR_SSI_EN;
  else
    SPI_MS->SSIENR = SSIENR_SSI_DISABLE;
}

/*!
* \fn      static HAL_StatusTypeDef SPI_PollTxFifoEmptyFlag(void)
* \brief   Function to poll TxFIFO Empty flag set
*/
static void SPI_PollTxFifoEmptyFlag(void)
{
  volatile UINT32_t temp = 0;

  do
  {
    temp = SPI_MS->SR;
    if((temp & SR_TFE) && !(temp & SR_BUSY))
    {
      break;
    }
  } while(1);

}

/*!
* \fn      static void SPI_FiFoWrite(SPI_HandleTypeDef *hspi)
* \brief   Function to write SPI data register
* \param   hspi --- handle to SPI structure
*
* @note SPI transmit FIFO depth is 131 entries. This facilitates
* transfer of 131 bytes of 8-bit wide transfers or 262 bytes of
* 16-bit wide transfer.
* To transfer larger data chunk up the data into smaller blocks.
*/
static void SPI_FiFoWrite(SPI_HandleTypeDef *hspi)
{
  SPI_Wr spi_w;
  UINT8_t *txb = (UINT8_t *) (hspi->pTxBuffer);
  UINT32_t ulLen = hspi->usTxXferSize;

  SPI_MS->SER = 0;
  if (ulLen > 0)
  {
    taskENTER_CRITICAL();
    if (hspi->Init.ucCmdType == PROGRAM_CMD)
    {
      /* byte len to wordLen */
      ulLen >>= 1;

      while (ulLen > 0) {
        spi_w.tb_r[1] = *txb++;
        spi_w.tb_r[0] = *txb++;
        SPI_MS->DR0 = spi_w.tw_r;
        ulLen--;
      }
    }
    else
    {
      //slave select enable
      while((SPI_MS->SR & SR_TFE) == 0) ; //wait until Tx Fifo Empty is set
      do {     
        SPI_MS->DR0 = *txb++;
        ulLen--;
      } while (ulLen > 0);
    }
    taskEXIT_CRITICAL();
  }

  //slave select enable
  if(hspi->Init.ucSSn == SPI_SLAVE_1_SELECT)
    SPI_MS->SER = SER_SS_0_N_SELECTED;
  else if(hspi->Init.ucSSn == SPI_SLAVE_2_SELECT)
    SPI_MS->SER = SER_SS_1_N_SELECTED;
  else
    SPI_MS->SER = SER_SS_2_N_SELECTED;
}

/*!
* \fn          static void SPI_FiFoRead(SPI_HandleTypeDef *hspi)
* \brief   Function to read SPI data register during SPI flash read
* \param   hspi --- SPI handle
*/
static void SPI_FiFoRead(SPI_HandleTypeDef *hspi)
{
  UINT8_t *rxbuf = (UINT8_t*) (hspi->pRxBuffer);
  UINT16_t len = hspi->usRxXferSize;
  int temp;
  if (len > 0)
  {
    do {
      temp = 1000;
      //Poll Rx FIFO Not Empty
      do {
        temp--;
        if (SPI_MS->SR & SR_RFNE)
        {
          *rxbuf++ = SPI_MS->DR0;
          len--;
        }
        else
          break;
      }while(temp > 0);
    } while (len > 0);
  }
}

/*!
* \fn          static void SPI_FlashRead(SPI_HandleTypeDef *hspi)
* \brief   Function to read SPI data register during SPI flash read
* \param   hspi --- SPI handle
*/
static void SPI_FlashRead(SPI_HandleTypeDef *hspi)
{
  UINT8_t *rxbuf = (UINT8_t*) (hspi->pRxBuffer);
  UINT16_t len = hspi->usRxXferSize;

  SPI_MS->DR0 = 0;
  if(hspi->Init.ucSSn == SPI_SLAVE_1_SELECT)
    SPI_MS->SER = SER_SS_0_N_SELECTED;
  else if(hspi->Init.ucSSn == SPI_SLAVE_2_SELECT)
    SPI_MS->SER = SER_SS_1_N_SELECTED;
  else
    SPI_MS->SER = SER_SS_2_N_SELECTED;

  if (len > 0)
  {
    do {
      //Poll Rx FIFO Not Empty
      do {
        if (SPI_MS->SR & SR_RFNE)
          break;
      } while (1);

      *rxbuf++ = SPI_MS->DR0;
      len--;
    } while (len > 0);
  }
}


HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef *hspi, UINT8_t *pData, UINT32_t ulTxLen, void (*HAL_SPI_Callback)(void))
{
  UINT32_t ulDomainClk = 0;
  uint32_t maxlen;
  if (pData == NULL || ulTxLen == 0)
  {
    printf("[HAL_SPI_Transmit]: Invalid parameters received: len = %d\r\n",ulTxLen);
    return HAL_ERROR;
  }
  lock_spi1_bus();
  assert_param(IS_SPI_SSN_VALID(hspi->Init.ucSSn));
  assert_param(IS_SPIx_VALID(hspi->ucSPIx));
  
  SPIHandle = hspi;
  //Initialize the structure
  hspi->pTxBuffer = pData;
  hspi->pRxBuffer = NULL;
  hspi->TxISR = NULL;
  hspi->RxISR = NULL;
  hspi->usRxXferSize = 0;
  hspi->HAL_SPI_TxRxComplCallback = NULL;
  hspi->usTxXferCount = ulTxLen;

  //Invalid parameter check
  if (hspi->Init.ucCmdType == READ_CMD
      || hspi->Init.ucCmdType == CMD_WithResponse)
  {
    printf(
           "HAL_SPI_Transmit: Invalid parameter ucCmdType = %d received\r\n",
           hspi->Init.ucCmdType);
    unlock_spi1_bus(CTX_TASK);
    return HAL_ERROR;
  }
  //Disable chip
  SPI_Enable(hspi, 0);

  ulDomainClk = S3x_Clk_Get_Rate(S3X_A1_CLK);
  //check if the C02 domain is less than the request freq. If so, set the BAUDR to 2.
  if(ulDomainClk <= hspi->Init.ucFreq)
  {
    SPI_MS->BAUDR = 2;
  }
  else
  {
    SPI_MS->BAUDR = ((ulDomainClk/hspi->Init.ucFreq) & 0x1) ?
                    ((ulDomainClk/hspi->Init.ucFreq) + 1) :
                        (ulDomainClk/hspi->Init.ucFreq);
  }

  //slave select disable
  SPI_MS->SER = 0;

  //update the spi transfer mode
  if (hspi->Init.ucCmdType == PROGRAM_CMD)
  {
    SPI_MS->CTRLR0 = (CTRLR0_TMOD_TX | CTRLR0_DFS_16_BIT |
                      (hspi->Init.ulCLKPolarity << BYTE_IDX_7) |
                        (hspi->Init.ulCLKPhase << BYTE_IDX_6));
    maxlen = SPI1_XFER_LEN_MAX; // 260-bytes or 130 16-bit words
  }
  else if (hspi->Init.ucCmdType == CMD_NoResponse)
  {
    SPI_MS->CTRLR0 = (CTRLR0_TMOD_TX | CTRLR0_DFS_8_BIT |
                      (hspi->Init.ulCLKPolarity << BYTE_IDX_7) |
                        (hspi->Init.ulCLKPhase << BYTE_IDX_6));
    maxlen = SPI1_XFER_LEN_MAX/2; // 130-bytes
  }
  //Disable SPI master interrupts
  SPI_MS->IMR = ((SPI_MS->IMR) & ~(ISR_TXEIM_ACTIVE));
            
  //Enable SPI
  SPI_Enable(hspi, 1);
  do {
    hspi->usTxXferSize = (hspi->usTxXferCount <= maxlen) ?
      hspi->usTxXferCount : maxlen;

      if((hspi->usTxXferCount <=  maxlen) && HAL_SPI_Callback)
          hspi->HAL_SPI_TxRxComplCallback = HAL_SPI_Callback;
      else
          hspi->State = HAL_SPI_STATE_TX_BUSY;
      //Enable the SPI_MS interrupt

      SPI_FiFoWrite(hspi);
      SPI_MS->IMR |= ISR_TXEIM_ACTIVE;
      //Enable interrupts  
      if(hspi->State == HAL_SPI_STATE_TX_BUSY)
      { 
        /* block until xWaitForIntr semaphore is given from IAR after
         * transction is complete
         */
        if (xSemaphoreTake(xWaitForIntr, portMAX_DELAY) != pdTRUE)
        {
            printf("[HAL SPI] : Error unable to take lock to spibus1\n");
            return HAL_TIMEOUT;
        }

        hspi->State = HAL_SPI_STATE_READY;
      }
      hspi->pTxBuffer = hspi->pTxBuffer + hspi->usTxXferSize;
      hspi->usTxXferCount -= hspi->usTxXferSize;

  } while (hspi->usTxXferCount > 0);

  if (!hspi->HAL_SPI_TxRxComplCallback)
  {
   //Disable chip
   SPI_MS->SSIENR = SSIENR_SSI_DISABLE;
   //slave select disable
   SPI_MS->SER = 0;
   unlock_spi1_bus(CTX_TASK);
  }
  return HAL_OK;
}


HAL_StatusTypeDef HAL_SPI_TransmitReceive(SPI_HandleTypeDef *hspi, UINT8_t *pTxData,
                                          const UINT32_t usTxSize, UINT8_t *pRxData,
                                          const UINT32_t usRxSize,
                                          void (*HAL_SPI_Callback)(void))
{
  UINT32_t ulDomainClk = 0;
  if (pTxData == NULL || pRxData == NULL || usTxSize == 0 || usRxSize == 0)
    return HAL_ERROR;
  lock_spi1_bus();
  assert_param(IS_SPI_SSN_VALID(hspi->Init.ucSSn));
  assert_param(IS_SPIx_VALID(hspi->ucSPIx));

  SPIHandle = hspi;
  //Initialize the structure
  hspi->pTxBuffer = pTxData;
  hspi->pRxBuffer = pRxData;
  hspi->usTxXferSize = usTxSize;
  hspi->usTxXferCount = usTxSize;
  hspi->usRxXferCount = usRxSize;
  hspi->usRxXferSize = usRxSize;
  hspi->TxISR = NULL;
  hspi->RxISR = NULL;

  //Invalid parameter check
  if (hspi->Init.ucCmdType == CMD_NoResponse || hspi->Init.ucCmdType == PROGRAM_CMD)
  {
    printf("HAL_SPI_TransmitReceive: Invalid CmdType = %d \r\n",hspi->Init.ucCmdType);
    unlock_spi1_bus(CTX_TASK);
    return HAL_ERROR;
  }

  //Disable chip
  SPI_MS->SSIENR = SSIENR_SSI_DISABLE;
  ulDomainClk = S3x_Clk_Get_Rate(S3X_A1_CLK);
  //check if the C02 domain is less than the request freq. If so, set the BAUDR to 2.
  if(ulDomainClk <= hspi->Init.ucFreq)
  {
    SPI_MS->BAUDR = 2;
  }
  else
  {
    SPI_MS->BAUDR = ((ulDomainClk/hspi->Init.ucFreq) & 0x1) ?
      ((ulDomainClk/hspi->Init.ucFreq) + 1) :
      (ulDomainClk/hspi->Init.ucFreq);
  }

  //slave select enable
  SPI_MS->SER = 0;

  //Disable SPI master interrupts
  SPI_MS->IMR = ((SPI_MS->IMR) & ~(ISR_TXEIM_ACTIVE));

  //configure the SPI transfer mode

  if (hspi->Init.ucCmdType == CMD_WithResponse)
  {
    SPI_MS->CTRLR0 = (CTRLR0_TMOD_EEPROM | CTRLR0_DFS_8_BIT |
                      (hspi->Init.ulCLKPolarity << BYTE_IDX_7) |
                        (hspi->Init.ulCLKPhase << BYTE_IDX_6));
    SPI_MS->CTRLR1 = (hspi->usRxXferSize - 1);
  }
  else if (hspi->Init.ucCmdType == READ_CMD)
  {
    SPI_MS->CTRLR0 = (CTRLR0_TMOD_EEPROM | CTRLR0_DFS_8_BIT |
                      (hspi->Init.ulCLKPolarity << BYTE_IDX_7) |
                        (hspi->Init.ulCLKPhase << BYTE_IDX_6));

    if(hspi->usRxXferSize % 4)
      SPI_MS->CTRLR1 = (((hspi->usRxXferSize + 3) / 4) * 4)-1;
    else
      SPI_MS->CTRLR1 = hspi->usRxXferSize - 1;
    //set the DMA destination address
    DMA_SPI_MS->DMA_DEST_ADDR = (UINT32_t) pRxData;

    //set the DMA transfer count
    if(hspi->usRxXferSize % 4)
      DMA_SPI_MS->DMA_XFER_CNT = (((hspi->usRxXferSize + 3) / 4) * 4)-1;
    else
      DMA_SPI_MS->DMA_XFER_CNT = hspi->usRxXferSize - 1;
    //Enable DMA interrupts
    DMA_SPI_MS->DMA_INTR_MASK = DMA_RX_DATA_AVAIL_INTR_MSK;
    //Enable the RX FIFO Full interrupt for DMA to run
    SPI_MS->IMR |= ISR_RXFIM_ACTIVE;

  }
  //Enable SPI
  //SPI_MS->SSIENR = SSIENR_SSI_EN;		/*Fix : Enable SPI after DMA Enable*/

  //poll for TxFifo Empty
  SPI_PollTxFifoEmptyFlag();

  if (hspi->Init.ucCmdType == READ_CMD)
  {

    if(HAL_SPI_Callback)
    {//non blocking callback from ISR
      hspi->HAL_SPI_TxRxComplCallback = HAL_SPI_Callback;
    }
    else//Blocking mode ISR will signal completion
    {
      hspi->State = HAL_SPI_STATE_RX_BUSY;
      hspi->HAL_SPI_TxRxComplCallback =  NULL;
    }
    //enable DMA for read
    DMA_SPI_MS->DMA_CTRL = DMA_CTRL_START_BIT;
  }
  
  //Enable SPI
  SPI_MS->SSIENR = SSIENR_SSI_EN;

  // For EEPROM tranfer mode, Tx FIFO must be filled without interruptions
  SPI_FiFoWrite(hspi);

  //poll for TxFifo Empty
  SPI_PollTxFifoEmptyFlag();

  if (hspi->Init.ucCmdType == CMD_WithResponse)
  {
    SPI_FiFoRead(hspi);
    unlock_spi1_bus(CTX_TASK);
  }
  else if(hspi->State == HAL_SPI_STATE_RX_BUSY)
  {
    if (xSemaphoreTake(xWaitForIntr, 1000 /* portMAX_DELAY */) != pdTRUE)
    {
      printf("[HAL SPI] : Error unable to take lock to spibus1\n");
      unlock_spi1_bus(CTX_TASK);
      return HAL_TIMEOUT;
    }
    hspi->State = HAL_SPI_STATE_READY;
    unlock_spi1_bus(CTX_TASK);
  }
  return HAL_OK;
}


/*!
*\fn       HAL_StatusTypeDef HAL_SPI_Receive(SPI_HandleTypeDef *hspi, UINT8_t *pRxData,UINT16_t usRxSize, void (*HAL_SPI_Callback)(void))
*\brief    Read an amount of data
*\param    hspi --- SPI handle
*\param    pRxData --- Pointer to data buffer
*\param    usRxSize --- amount of data to be read
*\param    callback function
*\return   HAL status
*/
HAL_StatusTypeDef HAL_SPI_Receive(SPI_HandleTypeDef *hspi, UINT8_t *pRxData,const UINT16_t usRxSize,
                                  void (*HAL_SPI_RxCompCallback)(void))
{
  UINT32_t ulDomainClk = 0;
  if (pRxData == NULL || usRxSize == 0)
    return HAL_ERROR;
  lock_spi1_bus();
  assert_param(IS_SPI_SSN_VALID(hspi->Init.ucSSn));
  assert_param(IS_SPIx_VALID(hspi->ucSPIx));

  SPIHandle = hspi;
  //Initialize the structure
  hspi->pRxBuffer = pRxData;
  hspi->usRxXferCount = usRxSize;
  hspi->usRxXferSize = usRxSize;

  hspi->pTxBuffer = NULL;
  hspi->usTxXferCount = 0;
  hspi->usTxXferSize = 0;

  hspi->TxISR = NULL;
  hspi->RxISR = NULL;

  //Invalid parameter check
  if (hspi->Init.ucCmdType != READ_CMD)
  {
    printf("HAL_SPI_ReceiveOnly: Invalid CmdType = %d\r\n",hspi->Init.ucCmdType);
    unlock_spi1_bus(CTX_TASK);
    return HAL_ERROR;
  }

  //Disable chip
  SPI_MS->SSIENR = SSIENR_SSI_DISABLE;
  ulDomainClk = S3x_Clk_Get_Rate(S3X_A1_CLK);
  //check if the C02 domain is less than the request freq. If so, set the BAUDR to 2.
  if(ulDomainClk <= hspi->Init.ucFreq)
  {
    SPI_MS->BAUDR = 2;
  }
  else
  {
    SPI_MS->BAUDR = ((ulDomainClk/hspi->Init.ucFreq) & 0x1) ?
      ((ulDomainClk/hspi->Init.ucFreq) + 1) :
      (ulDomainClk/hspi->Init.ucFreq);
  }

  //slave select enable
  /* Thsi should come as input  param*/
  SPI_MS->SER = 0;

  //Disable SPI master interrupts
  SPI_MS->IMR = ((SPI_MS->IMR) & ~(ISR_TXEIM_ACTIVE));
  //update the spi transfer mode
  SPI_MS->CTRLR0 = (CTRLR0_TMOD_RX | CTRLR0_DFS_8_BIT |
                    (hspi->Init.ulCLKPolarity << BYTE_IDX_7) |
                      (hspi->Init.ulCLKPhase << BYTE_IDX_6));

  //use DMA if the read count is multiple of 4. Else use polling mode to read data
  if(hspi->usRxXferSize % 4)
  {
    //polling mode
    SPI_MS->CTRLR1 = hspi->usRxXferSize - 1;
    //Enable SPI
    SPI_MS->SSIENR = SSIENR_SSI_EN;
    SPI_FlashRead(hspi);
    if(HAL_SPI_RxCompCallback)
      HAL_SPI_RxCompCallback();
    unlock_spi1_bus(CTX_TASK);
  }
  else
  {

    if(HAL_SPI_RxCompCallback)//non blocking callback from ISR
      hspi->HAL_SPI_TxRxComplCallback = HAL_SPI_RxCompCallback;
    else//Blocking mode ISR will signal completion
      hspi->State = HAL_SPI_STATE_RX_BUSY;
    //DMA mode
    SPI_MS->CTRLR1 = (hspi->usRxXferSize-1);

    //set the DMA destination address
    DMA_SPI_MS->DMA_DEST_ADDR = (UINT32_t) pRxData;

    //set the DMA transfer count
    DMA_SPI_MS->DMA_XFER_CNT = hspi->usRxXferSize - 1;

    //Enable DMA interrupts
    DMA_SPI_MS->DMA_INTR_MASK = DMA_RX_DATA_AVAIL_INTR_MSK;
    //Enable the RX FIFO Full interrupt for DMA to run
    SPI_MS->IMR |= ISR_RXFIM_ACTIVE;

        /* Tim SPI FIX - WED - 27/mar/19 */
        taskENTER_CRITICAL(); 

    //slave select enable
    if(hspi->Init.ucSSn == SPI_SLAVE_1_SELECT)
      SPI_MS->SER = SER_SS_0_N_SELECTED;
    else if(hspi->Init.ucSSn == SPI_SLAVE_2_SELECT)
      SPI_MS->SER = SER_SS_1_N_SELECTED;
    else
      SPI_MS->SER = SER_SS_2_N_SELECTED;

    //enable DMA for read
    DMA_SPI_MS->DMA_CTRL = DMA_CTRL_START_BIT;

        //Enable SPI
        SPI_MS->SSIENR = SSIENR_SSI_EN;
        
        SPI_MS->DR0 = 0;

        taskEXIT_CRITICAL();
        
    if(hspi->State == HAL_SPI_STATE_RX_BUSY)
    {
      if (xSemaphoreTake(xWaitForIntr, portMAX_DELAY) != pdTRUE)
      {
        printf("[HAL SPI] : Error unable to take lock to spibus1\n");
        unlock_spi1_bus(CTX_TASK);
        return HAL_TIMEOUT;
      }
      hspi->State = HAL_SPI_STATE_READY;
      unlock_spi1_bus(CTX_TASK);
    }
  }
  return HAL_OK;

}

/*!
* \fn      void SPI_DMA_Complete(SPI_HandleTypeDef  *hspi)
* \brief   Callback for SPI Flash Read DMA complete
* \param   SPI Handle
* \return  None
*/
//void SPI_DMA_Complete(SPI_HandleTypeDef *hspi) {
void SPI_DMA_Complete(void)
{
  //Disable chip
  SPI_MS->SSIENR = SSIENR_SSI_DISABLE;

  //slave select enable
  SPI_MS->SER = 0;

  //Disable SPI master interrupts
  SPI_MS->IMR = ((SPI_MS->IMR) & ~(ISR_TXEIM_ACTIVE));

  //Disable DMA interrupts
  DMA_SPI_MS->DMA_INTR_MASK &= ~(DMA_RX_DATA_AVAIL_INTR_MSK);

  //Stop DMA
  DMA_SPI_MS->DMA_CTRL = DMA_CTRL_STOP_BIT;

  if(SPIHandle->HAL_SPI_TxRxComplCallback)
  {
    SPIHandle->HAL_SPI_TxRxComplCallback();
    unlock_spi1_bus(CTX_ISR);
  }
  if (SPIHandle->State == HAL_SPI_STATE_RX_BUSY)
  {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if(xWaitForIntr)
    {
      if (xSemaphoreGiveFromISR(xWaitForIntr,&xHigherPriorityTaskWoken) == pdTRUE)
      {
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
      }
      else
        printf("[HAL SPI] : Error : unable to release lock to spibus1\n");
    }
  }

}

/*!
*\fn           void HAL_SPI_IRQHandler(SPI_HandleTypeDef *hspi)
*\brief    SPI Interrupt handler
*\param    hspi --- SPI handle
*/
void HAL_SPI_IRQHandler(void)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  SPI_HandleTypeDef *hspi = SPIHandle;
  if (hspi->TxISR != NULL)
    hspi->TxISR(hspi);

  if (hspi->State == HAL_SPI_STATE_TX_BUSY)
  {
    if(xWaitForIntr)
    {
      if (xSemaphoreGiveFromISR(xWaitForIntr,&xHigherPriorityTaskWoken) == pdTRUE)
      {
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
      }
      else
        printf("[HAL SPI] : Error : unable to release lock to spibus1\n");
    }

  }
  else if(SPIHandle->HAL_SPI_TxRxComplCallback)
  {
    SPIHandle->HAL_SPI_TxRxComplCallback();
    unlock_spi1_bus(CTX_ISR);
  }

}

/*!
* \fn      HAL_StatusTypeDef HAL_SPI_Init(SPI_HandleTypeDef  *hspi)
* \brief   Initializes the SPI Master controller
* \param   hspi --- SPI handle
* \return  HAL status
*/
HAL_StatusTypeDef HAL_SPI_Init(SPI_HandleTypeDef *hspi)
{
  UINT32_t ulDomainClk = 0;

  if (hspi == NULL)
    return HAL_ERROR;

  if(xLockSPI1Bus == NULL)
  {
    //printf("HAL SPI mutex created\r\n");

    xLockSPI1Bus = xSemaphoreCreateBinary();
    if( xLockSPI1Bus == NULL )
    {
      //printf("[HAL SPI] : Error : Unable to Create Mutex\n");
      return HAL_ERROR;
    }
        vQueueAddToRegistry(xLockSPI1Bus, "Spi_Lock" );
    xSemaphoreGive(xLockSPI1Bus);
  }
  if(xWaitForIntr == NULL)
  {
    /* Note:: xSemaphoreTake(xWaitForIntr) causes the calling api to block 
      * until xSemaphoreGiveFromISR(xWaitForIntr,&xHigherPriorityTaskWoken) is 
      * called from ISR. This is used for blocking SPI transcation.
      */
    xWaitForIntr = xSemaphoreCreateBinary();
    if( xWaitForIntr == NULL )
    {
      //printf("[HAL SPI] : Error : Unable to Create Mutex\n");
            if(xLockSPI1Bus){
                vSemaphoreDelete(xLockSPI1Bus);
                vQueueUnregisterQueue(xLockSPI1Bus);
            }
            xLockSPI1Bus = NULL;
      return HAL_ERROR;
    }
        vQueueAddToRegistry( xWaitForIntr, "Spi_WFI" );
  }
  S3x_Clk_Enable(S3X_A1_CLK);

  //check the parameters
  assert_param(IS_SPI_DATASIZE(hspi->Init.ulDataSize));
  assert_param(IS_SPI_CPOL(hspi->Init.ulCLKPolarity));
  assert_param(IS_SPI_CPHA(hspi->Init.ulCLKPhase));
  assert_param(IS_SPI_FIRST_BIT(hspi->Init.ulFirstBit));
  assert_param(IS_SPI_BIDIR_MODE(hspi->Init.ucSPIInf));

  //SPI configuration

  //Configure PADS for sensor SPI Master lines
  //SPI_PadConfig();								//part of hw setup

  //Disable chip
  SPI_MS->SSIENR = SSIENR_SSI_DISABLE;

  //clear pending SPI interrupts
  SPI_MS->ICR |= 0x01;

  //Configure Ctrl0 register
  SPI_MS->CTRLR0 |= ((hspi->Init.ulCLKPolarity << BYTE_IDX_7) |
                     (hspi->Init.ulCLKPhase << BYTE_IDX_6));
  //Baud rate divider (Pending) (max rate is 20Mhz)
  ulDomainClk = S3x_Clk_Get_Rate(S3X_A1_CLK);
  //check if the C02 domain is less than the request freq. If so, set the BAUDR to 2.
  if(ulDomainClk <= hspi->Init.ucFreq)
  {
    SPI_MS->BAUDR = 2;
  }
  else
  {
    SPI_MS->BAUDR = ((ulDomainClk/hspi->Init.ucFreq) & 0x1) ?
      ((ulDomainClk/hspi->Init.ucFreq) + 1) :
      (ulDomainClk/hspi->Init.ucFreq);
  }

  //clear the SPI_MS_INTR & CFG_DMA_DONE interrupts
  INTR_CTRL->OTHER_INTR &= (CFG_DMA_DONE_EN_M4 | SPI_MS_INTR_EN_M4);
  //Enable cfg dma interrupt
  INTR_CTRL->OTHER_INTR_EN_M4 |= CFG_DMA_DONE_EN_M4;
  INTR_CTRL->OTHER_INTR_EN_M4 |= SPI_MS_INTR_EN_M4;
  NVIC_ClearPendingIRQ(CfgDma_IRQn);
  NVIC_ClearPendingIRQ(SpiMs_IRQn);
  NVIC_EnableIRQ(CfgDma_IRQn);
  NVIC_EnableIRQ(SpiMs_IRQn);
  
  /*set spi handle*/
  SPIHandle = hspi;
  
  S3x_Clk_Disable(S3X_A1_CLK);
  
  /* Increment the instance count */
  SPI_Instance_count++;
  
  return HAL_OK;
}

/*!
* \fn      HAL_StatusTypeDef HAL_SPI_DeInit(SPI_HandleTypeDef  *hspi)
* \param   hspi    --- SPI Handle
* \return  HAL status
*/
HAL_StatusTypeDef HAL_SPI_DeInit(SPI_HandleTypeDef *hspi)
{
  if (hspi == NULL)
    return HAL_ERROR;
  assert_param(IS_SPIx_VALID(hspi->ucSPIx));

  S3x_Clk_Enable(S3X_A1_CLK);
  //Disable chip
  SPI_MS->SSIENR = SSIENR_SSI_DISABLE;
  
  SPI_Instance_count--;
  if(SPI_Instance_count == 0) /* if there are no other instances */
  {
    if(xLockSPI1Bus){
       vSemaphoreDelete(xLockSPI1Bus);
        vQueueUnregisterQueue(xLockSPI1Bus);
    }
    if(xWaitForIntr){
       vSemaphoreDelete(xWaitForIntr);
        vQueueUnregisterQueue(xWaitForIntr);
    }
    xLockSPI1Bus = NULL;
    xWaitForIntr = NULL;
  }
  
  /*Clear SPI handle*/
  SPIHandle = NULL;
  S3x_Clk_Disable(S3X_A1_CLK);
  return HAL_OK;
}

/*!
* \fn          static void SPI_FlashRead_Response(SPI_HandleTypeDef *hspi)
* \brief   Function to read SPI data register during SPI TransmitReceive2 (non-DMA)
* \param   hspi --- SPI handle
*/
static void SPI_FlashRead_Response(SPI_HandleTypeDef *hspi) {
	UINT8_t *rxbuf = (UINT8_t*) (hspi->pRxBuffer);
	UINT16_t len = hspi->usRxXferSize;
	int temp = 0;
    while (len > 0)
    {
        //Poll Rx FIFO Not Empty
        if (SPI_MS->SR & SR_RFNE)
        {
            *rxbuf++ = SPI_MS->DR0;
            len--;
            temp = 0; //reset the timeout
        }
        else
        {
            temp++;
            //Each byte has max 100 SPI Status reads time out
            //Note: SPI status reads over AHB bus are much slower than M4 bus reads.
            //So, this will make the bus busy all the time. But OK since no other Task
            //is working untill this is finished or task switch happens.
            if(temp > 100)
            {
              //Warning: there is no way to convey this error condition 
              //to calling function
              while(len)
              {
                *rxbuf++ = 0;
                len--;
              }
              break; //error condition
            }
        }
	} 
}

HAL_StatusTypeDef HAL_SPI_TransmitReceive2(SPI_HandleTypeDef *hspi, UINT8_t *pTxData, const UINT32_t usTxSize, UINT8_t *pRxData,const UINT32_t usRxSize,
                                           void (*HAL_SPI_TxRxComplCallback)(void))
{
	int iDomainClk = 0;
    
	if (pTxData == NULL || pRxData == NULL || usTxSize == 0 || usRxSize == 0)
		return HAL_ERROR;
    
	if (hspi->ucSPIx == SPI1_MASTER_SEL)
	{
		lock_spi1_bus();
	}
    
    assert_param(IS_SPI_SSN_VALID(hspi->Init.ucSSn));
    assert_param(IS_SPIx_VALID(hspi->ucSPIx));
    
    SPIHandle = hspi;
    //Initialize the structure
    hspi->pTxBuffer = pTxData;
    hspi->pRxBuffer = pRxData;
    hspi->usTxXferSize = usTxSize;
    hspi->usTxXferCount = usTxSize;
    hspi->usRxXferCount = usRxSize;
    hspi->usRxXferSize = usRxSize;
    hspi->TxISR = NULL;
    hspi->RxISR = NULL;
    
    if(HAL_SPI_TxRxComplCallback) {
        configASSERT(0);
    }
    
    if (hspi->ucSPIx == SPI0_MASTER_SEL) {
        configASSERT(0);
    } else if (hspi->ucSPIx == SPI1_MASTER_SEL) {
        //Invalid parameter check
        if (hspi->Init.ucCmdType == CMD_NoResponse || hspi->Init.ucCmdType == PROGRAM_CMD) {
            //QL_LOG_ERR_150K("SPI_TxRx: Inval param CmdType = %d\r\n",hspi->Init.ucCmdType);
            
            unlock_spi1_bus(CTX_TASK);
            return HAL_ERROR;
        }
        
        //poll for TxFifo Empty before pushing new data
        SPI_PollTxFifoEmptyFlag();
        //Disable chip
        SPI_MS->SSIENR = SSIENR_SSI_DISABLE;
        
        iDomainClk = S3x_Clk_Get_Rate(S3X_FB_02_CLK);
        
        //check if the C02 domain is less than the request freq. If so, set the BAUDR to 2.
        if(iDomainClk < hspi->Init.ucFreq)
            SPI_MS->BAUDR = 2;
        else
            SPI_MS->BAUDR = ((iDomainClk/hspi->Init.ucFreq) & 0x1) ? ((iDomainClk/hspi->Init.ucFreq) + 1) : (iDomainClk/hspi->Init.ucFreq);
            
            
			//slave select enable
			SPI_MS->SER = 0;
            
			//Disable SPI master interrupts
			SPI_MS->IMR = ((SPI_MS->IMR) & ~(ISR_TXEIM_ACTIVE));
            
			//configure the SPI transfer mode
            
			if (hspi->Init.ucCmdType == CMD_WithResponse) {
				SPI_MS->CTRLR0 = (CTRLR0_TMOD_TX_RX | CTRLR0_DFS_8_BIT | (hspi->Init.ulCLKPolarity << BYTE_IDX_7) | (hspi->Init.ulCLKPhase << BYTE_IDX_6));
				SPI_MS->CTRLR1 = (hspi->usRxXferSize - 1);
                
			} else if (hspi->Init.ucCmdType == READ_CMD) {
				SPI_MS->CTRLR0 = (CTRLR0_TMOD_TX_RX | CTRLR0_DFS_8_BIT | (hspi->Init.ulCLKPolarity << BYTE_IDX_7) | (hspi->Init.ulCLKPhase << BYTE_IDX_6));
                SPI_MS->CTRLR1 = hspi->usRxXferSize - 1;
                
				if(HAL_SPI_TxRxComplCallback) {
					configASSERT(0);
				}
			}
            
			//Enable SPI
			SPI_MS->SSIENR = SSIENR_SSI_EN;
            
			//poll for TxFifo Empty before pushing new data
            SPI_PollTxFifoEmptyFlag();
			SPI_FiFoWrite(hspi);
            
			if (hspi->Init.ucCmdType == READ_CMD) {
				SPI_FlashRead_Response(hspi);
			}
            
			if (!(hspi->Init.ucCmdType == READ_CMD && HAL_SPI_TxRxComplCallback) || (hspi->Init.ucCmdType == READ_CMD && !(HAL_SPI_TxRxComplCallback)))
			{
				unlock_spi1_bus(CTX_TASK);
			}
            
			return HAL_OK;
    }
    else {
        dbg_fatal_error("SPI_TxRx: Nosuprt in SPI1 master\r\n");
    }
    
    return HAL_OK;
}

#endif
