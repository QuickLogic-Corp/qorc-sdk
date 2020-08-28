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

/*! \file HAL_fpga_sdma_api.c
 *
 *  \brief This file contains SDMA wrapper APIs to use by FPGA IP drivers
 *         for data transfer to and from FPGA IP on S3.
 *         These APIs can be used by only FPGA IP applications.
 */

/****************************************************************************

1)Supports FPGA dedicated 4 SDMA channels (CHANNEL: 12 to 15)simultaneously.
2)Dedicated DMA channel memory space in FPGA for write/Read.
  Each channel memory space is accessible through Dedicated Port register.
   SDMA Channel 12 - 0x40031000
   SDMA Channel 13 - 0x40032000
   SDMA Channel 14 - 0x40033000
   SDMA Channel 15 - 0x40034000
3)All FPGA SDMA interrupts are mapped to FPGA Inteerupt 0.
  4 SDMA channels's status is mapped into single FPGA register.

******************************************************************************/
#include "Fw_global_config.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <FreeRTOS.h>
#include "eoss3_dev.h"
#include "eoss3_hal_fpga_sdma_api.h"
#include "eoss3_hal_fpga_sdma_reg.h"
#include "s3x_clock_hal.h"
#include "eoss3_hal_sdma.h"
#include "eoss3_hal_gpio.h"
#include "s3x_clock.h"
#include "eoss3_hal_fpga_adc_api.h"
#include "dbg_uart.h"

/* Max SDMA channels supported by FPGA. */
#define MAX_FSDMA_CHANNELS 4

#define FSDMA_CHANNEL_START SDMA_SRAM_CH12

#define FSDMA_CHANNEL_END SDMA_SRAM_CH15

#define GET_SDMA_CHANNEL_NUMBER(ch) ((ch >= 0 && ch < MAX_FSDMA_CHANNELS) ? (ch + FSDMA_CHANNEL_START) : -1)

/* Flag used to poll for DMA interrupt for synchronous API calls. */
static uint8_t volatile poll_for_dma_complete = 0;

/* FPGA SDMA channel context. */
typedef struct
{
    void (*callback)(void*);
    ch_cfg_t cfg_param;
    int8_t channel_id;
    uint8_t ch_in_use;
    uint8_t ch_transfer_in_process;
} fsdma_ch_handle_t;

/* FPGA SDMA driver context. */
typedef struct __FSDMA_HandleTypeDef
{
   fsdma_ch_handle_t chanInfo[MAX_FSDMA_CHANNELS];
} fsdma_handle_t;

/* FPGA SDMA driver handle. */
static fsdma_handle_t handle;

__STATIC_INLINE void fsdma_channel_enable(int ch)
{
   FB_SDMA->FSDMA_ENR |= FSDMA_CHANNEL_EN(ch);
}

__STATIC_INLINE void fsdma_channel_disable(int ch)
{
   FB_SDMA->FSDMA_ENR &= ~FSDMA_CHANNEL_EN(ch);
}

__STATIC_INLINE void fsdma_channel_intr_clear(int ch)
{
  FB_SDMA->FSDMA_SR &= ~FSDMA_CHANNEL_INTR_CLR(ch);
}

__STATIC_INLINE bool is_fsdma_ch_intr_set(int ch)
{
    return ((FB_SDMA->FSDMA_SR >> (FSDMA_CHANNEL_POS(ch))) & 1);
}

__STATIC_INLINE void fsdma_channel_intr_enable(int ch)
{
   FB_SDMA->FSDMA_INTEN |= FSDMA_CHANNEL_INTR_EN(ch);
}

__STATIC_INLINE void fsdma_channel_intr_disable(int ch)
{
   FB_SDMA->FSDMA_INTEN &= ~FSDMA_CHANNEL_INTR_EN(ch);
}

__STATIC_INLINE int get_fsdma_channel_data_register(int ch)
{
  switch(ch)
  {
      case 0:
        return FSDMA_CH0_DATA_REGISTER;
        break;
      case 1:
        return FSDMA_CH1_DATA_REGISTER;
        break;
      case 2:
        return FSDMA_CH2_DATA_REGISTER ;
        break;
      case 3:
        return FSDMA_CH3_DATA_REGISTER;
        break;
      default:
        return -1;
  }
}

static uint32_t set_channel_rpower(uint32_t transfer_length)
{
    int length_words = transfer_length/4;
    uint32_t rpowerCode;
    if (length_words > 512) rpowerCode = 1024;
    else if (length_words > 256) rpowerCode = 512;
    else if (length_words > 128) rpowerCode = 256;
    else if (length_words > 64) rpowerCode = 128;
    else if (length_words > 32) rpowerCode = 64;
    else if (length_words > 16) rpowerCode = 32;
    else if (length_words > 8) rpowerCode = 16;
    else if (length_words > 4) rpowerCode = 8;
    else if (length_words > 2) rpowerCode = 4;
    else rpowerCode = 2;
    return rpowerCode;
}

/***********************************
FPGA SDMA driver Interrupt Handler
*************************************/

int adc_dma_count;

static void HAL_FSDMA_ISR_Handler()
{

   //HAL_GPIO_Write(GPIO_0, 0);
    
    if( adc_dma_count < MAX_ADC_STAMPS  ){
        adc_timestamps[ adc_dma_count ].fsdma_enter = DWT->CYCCNT;
        adc_dma_count += 1;
    }
    
  int i = 0;
  for(i = 0; i < 4; i++)
  {
        if(is_fsdma_ch_intr_set(i))  //Check which channel interrupt is set/
        {
#if (CONST_FREQ == 0)
          S3x_Clear_Qos_Req(S3X_SDMA_CLK, MIN_CPU_FREQ);
          S3x_Clear_Qos_Req(S3X_SDMA_CLK, MIN_OP_FREQ);
#endif

          fsdma_ch_handle_t* ch = &(handle.chanInfo[i]);
           fsdma_channel_intr_clear(i);

          //Call application registered callback with channel handle as parameter
          ch->ch_transfer_in_process = 0;
          if(ch->callback)
                ch->callback((void*)&handle.chanInfo[i]);
          else
                poll_for_dma_complete = true;
        }
  }
  //NVIC level interrupt clear
  //S3x_Clk_Disable(S3X_FB_16_CLK);
  NVIC_ClearPendingIRQ(FbMsg_IRQn);

}

/* Initilazation of FPGA SDMA HAL driver. */
HAL_StatusTypeDef HAL_FSDMA_Init(void)
{
    HAL_StatusTypeDef status = HAL_OK;

#if (CONST_FREQ == 0)
    /* Clocks initialization */
    S3x_Register_Qos_Node(S3X_SDMA_CLK);
#endif

    /* Clear any pending interrupt of FPGA */
    NVIC_ClearPendingIRQ(FbMsg_IRQn);

    /* NVIC level interrupt enable */
    NVIC_EnableIRQ(FbMsg_IRQn);

    /* ISR registration for FPGA SDMA interrupts. */
    FB_RegisterISR(FB_INTERRUPT_0, HAL_FSDMA_ISR_Handler);
    FB_ConfigureInterrupt(FB_INTERRUPT_0, FB_INTERRUPT_TYPE_LEVEL,
                          FB_INTERRUPT_POL_LEVEL_HIGH,
                          FB_INTERRUPT_DEST_AP_DISBLE, FB_INTERRUPT_DEST_M4_ENABLE);

    S3x_Clk_Enable (S3X_SDMA_CLK); //temp
    return status;
}

/*
 * API to get a channel handle to make transfer and Receive data from FPGA IP.
 *   On channel resource allocation, handle will be returned. Applciation should use
 *   this handle for next transfer/Receive APIs.
 * If transfer completes between FPGA and SRAM, callback will be triggered.
 * If callback parameter is NULL, Transfer and receive APIs are synchronous.
 */
void* HAL_FSDMA_GetChannel(void (*pCallback)(void*), ch_cfg_t *pCfg)
{
   int i = 0;
   for(i=0; i < MAX_FSDMA_CHANNELS; i++)
   {
        if (0 == handle.chanInfo[i].ch_in_use)
        {
          handle.chanInfo[i].channel_id = i;
          handle.chanInfo[i].ch_in_use = 1;
          handle.chanInfo[i].callback = pCallback;
          memcpy((void*)&handle.chanInfo[i].cfg_param, (void*)pCfg, sizeof(ch_cfg_t));
          fsdma_channel_intr_enable(i);     //DMA channel trnsfer done interrupt enable
          return ((void*)&(handle.chanInfo[i]));   //returning channel handle
        }
   }
   return NULL;
}

/*
 * API to release channel resource.
 *
 */
HAL_StatusTypeDef HAL_FSDMA_ReleaseChannel(void *handle)
{
    fsdma_ch_handle_t *ch = (fsdma_ch_handle_t*)handle;
    if(ch)
    {
        if(ch->ch_transfer_in_process){
            dbg_str("dma-error-busy\n");
         return HAL_BUSY;     // Channel data transfer is in progress. Return Error.
        }
       fsdma_channel_intr_disable(ch->channel_id); //DMA channel interrupt disable
       fsdma_channel_disable(ch->channel_id);      // DMA Channel disable
       ch->ch_in_use = 0;
    }
    return HAL_OK;
}

/*
 * Transfer data of length bytes from SRAM source to FPGA IP.
 * Assuming, Destination in FPGA IP is fixed Fifo Data register
 * length is transfer size in bytes.
 */
HAL_StatusTypeDef HAL_FSDMA_Send(void* handle, void *srcptr, uint32_t length)
{
     fsdma_ch_handle_t *ch;
     ch = (fsdma_ch_handle_t*)handle;
     HAL_StatusTypeDef status = HAL_OK;
     int channel_num;
     void *dstptr;
     SDMA_ch_cfg_t chCfg;

     if(1 == ch->ch_transfer_in_process)
       return HAL_BUSY;

     ch->ch_transfer_in_process = 1;
     channel_num = GET_SDMA_CHANNEL_NUMBER(ch->channel_id);

     /* Get channel dedicated port register. */
     dstptr = (void*)get_fsdma_channel_data_register(ch->channel_id);

     chCfg.data_size = SDMA_XFER_WORD;
     // destination address is fixed fpga DMA port register. So no increment after every word transfer.
     chCfg.dest_addr_inc = SDMA_ADDR_WIDTH_NOINC;
     chCfg.src_addr_inc  = SDMA_ADDR_WIDTH_WORD;

     // If rPower is not updated by application, set accroding to max transfer length.
   /*  if(ch->cfg_param.r_power == 0)
       chCfg.rpowerCode = set_channel_rpower(length);
     else
       chCfg.rpowerCode = 512;*/

     chCfg.rpowerCode = ch->cfg_param.r_power;

     S3x_Clk_Set_Rate(S3X_SDMA_CLK, HSOSC_72MHZ);
     //S3x_Clk_Enable (S3X_SDMA_CLK);
     //S3x_Clk_Enable(S3X_FB_16_CLK);

#if (CONST_FREQ == 0)
     S3x_Set_Qos_Req(S3X_SDMA_CLK, MIN_CPU_FREQ, HSOSC_72MHZ);
     S3x_Set_Qos_Req(S3X_SDMA_CLK, MIN_OP_FREQ, HSOSC_72MHZ);
#endif

     status = HAL_SDMA_xfer(channel_num, srcptr, dstptr, (length/4), &chCfg);
     //HAL_GPIO_Write(GPIO_0, 1);
     fsdma_channel_enable(ch->channel_id);

     if(NULL == ch->callback)
     {
         //Callback is NULL, Wait/poll for DMA complete interrupt status and then return from this API
         while(0 == poll_for_dma_complete);
         poll_for_dma_complete = 0;   // Reset the waiting flag
         ch->ch_transfer_in_process = 0;
     }
     return status;
}

/*
 * Transfer data of length bytes from FPGA IP to SRAM destination address.
 * Assuming, Source in FPGA IP is fixed Fifo Data register.
 * length is transfer size in bytes.         ~
 */

HAL_StatusTypeDef HAL_FSDMA_Receive(void* handle, void *dstptr, uint32_t length)
{
     fsdma_ch_handle_t *ch;
     ch = (fsdma_ch_handle_t*)handle;
     HAL_StatusTypeDef status = HAL_OK;
     SDMA_ch_cfg_t chCfg;
     int channel_num;
     void *srcptr;

     if(1 == ch->ch_transfer_in_process)
       return HAL_BUSY;

     ch->ch_transfer_in_process = 1;
     channel_num = GET_SDMA_CHANNEL_NUMBER(ch->channel_id);
     srcptr = (void*)get_fsdma_channel_data_register(ch->channel_id);

     chCfg.data_size     = SDMA_XFER_WORD;
     chCfg.dest_addr_inc = SDMA_ADDR_WIDTH_WORD;
     // source address is fixed fpga DMA port register. So no increment after every word transfer.
     chCfg.src_addr_inc  = SDMA_ADDR_WIDTH_NOINC;

     // If rPower is not updated by application, set accroding to max transfer length.
    /* if(ch->cfg_param.r_power == 0)
       chCfg.rpowerCode = set_channel_rpower(length);
     else
       chCfg.rpowerCode = 512;*/

     chCfg.rpowerCode = ch->cfg_param.r_power;  //mqd
     //temp code mqd
     //SDMA->CHNL_PRIORITY_SET = 1 << 13;  //Make channel 13 highest priority.
     //SDMA->CHNL_USEBURST_CLR = 1 << 13;  //DMA in SREQ mode


      S3x_Clk_Set_Rate(S3X_SDMA_CLK, HSOSC_72MHZ);
      //S3x_Clk_Enable(S3X_SDMA_CLK);
      //S3x_Clk_Enable(S3X_FB_16_CLK);

#if (CONST_FREQ == 0)
     S3x_Set_Qos_Req(S3X_SDMA_CLK, MIN_CPU_FREQ, HSOSC_72MHZ);
     S3x_Set_Qos_Req(S3X_SDMA_CLK, MIN_OP_FREQ, HSOSC_72MHZ);
#endif

     status = HAL_SDMA_xfer(channel_num, srcptr, dstptr, (length/4), &chCfg);
     // HAL_GPIO_Write(GPIO_0, 1);
     fsdma_channel_enable(ch->channel_id);


     if(NULL == ch->callback)
     {
         //Callback is NULL, Wait for DMA complete interrupt status and then return from this API
         while(0 == poll_for_dma_complete);
         poll_for_dma_complete = 0;   // Reset the waiting flag
         ch->ch_transfer_in_process = 0;
     }
     return status;
}

int HAL_FSDMA_IsTransferInProgress(void *handle)
{
    fsdma_ch_handle_t *ch = (fsdma_ch_handle_t*)handle;
    if(ch)
    {
      return ch->ch_transfer_in_process;
    }
    else
      return 0;
}