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

/*
* \file  eoss3_hal_i2s_slave_assp.c
*
* \brief   This file contains HAL API for I2S Slave ASSP in Tx Mode
*/

/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include "Fw_global_config.h"
#include "eoss3_hal_i2s.h"
#include "eoss3_dev.h"
#include "eoss3_hal_sdma.h"
#include "s3x_clock_hal.h"
#include "s3x_clock.h"

static unsigned char spurious_intr=0;

uint8_t bufindex_for_stream = 0;

#define I2S_SLAVE_ASSP_IER_EN       0x1 /* IER enable */
#define I2S_SLAVE_ASSP_IER_DIS      0x0 /* IER disable */

#define I2S_SLAVE_ASSP_TX_BLOCK_EN  0x1 /* Transmitter block enable */
#define I2S_SLAVE_ASSP_TX_BLOCK_DIS 0x0 /* Transmitter block disable */

#define I2S_SLAVE_ASSP_STEREO_EN    0x1 /* Stereo enable */
#define I2S_SLAVE_ASSP_MONO_EN      0x0 /* Mono enable */

#define I2S_SLAVE_ASSP_RESET_FIFO   0x1 /* FIFO reset */

#define I2S_SLAVE_ASSP_TX_OVERRUN   (0x1 << 5) /* Masks Tx FIFO Overrun interrupt */

#define I2S_SLAVE_ASSP_TX_FIFOEMPTY (0x1 << 4) /* Masks Tx FIFO Empty interrupt */

#define I2S_SDMA_WORD_SRC_DST_SIZE      (0x2) /* Word 32 bit*/
#define I2S_SDMA_WORD_SRC_INC           (0x2) /* Increment by word */
#define I2S_SDMA_WORD_DST_INC           (0x3) /* Destination no increment */
#define I2S_SDMA_CYCLE_CTRL             (0x1) /* Basic cycle control */

typedef struct {
  uint32_t *p_tx_buffer;
  uint16_t  buffer_size;
} I2S_SLAVE_buf_info_t;

/* save the slave config for further usage */
static I2S_Config_t i2s_slave_assp_tx;
static HAL_I2S_Cb_Handler_t i2s_slave_cb;
static uint8_t i2s_sdma_config_status = 0;


static I2S_SLAVE_buf_info_t i2s_slave_buf_info;

__STATIC_INLINE void i2s_assp_slave_enable(uint32_t enable)
{
  I2S_SLAVE->IER = enable;
}

__STATIC_INLINE void i2s_assp_slave_Tx_enable(uint32_t enable)
{
  I2S_SLAVE->ITER  = enable;
}

__STATIC_INLINE void i2s_assp_slave_stereo_en(uint32_t enable)
{
  I2S_SLAVE->I2S_STEREO_EN = enable;
}

__STATIC_INLINE void i2s_assp_slave_fifo_reset(void)
{
  I2S_SLAVE->TXFFR = I2S_SLAVE_ASSP_RESET_FIFO;
}

__STATIC_INLINE void i2s_assp_slave_tx_overrun_mask(void)
{
  I2S_SLAVE->IMR0 |= I2S_SLAVE_ASSP_TX_OVERRUN;
}

__STATIC_INLINE void i2s_assp_slave_tx_overrun_unmask(void)
{
  I2S_SLAVE->IMR0 &= ~I2S_SLAVE_ASSP_TX_OVERRUN;
}

__STATIC_INLINE void i2s_assp_slave_tx_fifoempty_mask(void)
{
  I2S_SLAVE->IMR0 |= I2S_SLAVE_ASSP_TX_FIFOEMPTY;
}

__STATIC_INLINE void i2s_assp_slave_tx_fifoempty_unmask(void)
{
  I2S_SLAVE->IMR0 &= ~I2S_SLAVE_ASSP_TX_FIFOEMPTY;
}

/**
* @brief HAL_I2S_Slave_Assp_Init, called to initialize the buffers required
*
*/
static uint32_t HAL_I2S_Slave_Assp_Init(I2S_Config_t *p_i2s_cfg, HAL_I2S_Cb_Handler_t handler)
{
  configASSERT(handler);
  if(p_i2s_cfg->sdma_used != 0)
  {
    printf("[I2S] Slave I2S doesnt support SDMA %d\n", p_i2s_cfg->sdma_used);
    return HAL_I2S_BAD_PARAMETER;
  }

  if (p_i2s_cfg->ch_sel == I2S_CHANNELS_STEREO)
    i2s_slave_assp_tx.ch_sel  = I2S_CHANNELS_STEREO;
  else if (p_i2s_cfg->ch_sel == I2S_CHANNELS_MONO)
    i2s_slave_assp_tx.ch_sel = I2S_CHANNELS_MONO;
  else
  {
    printf("[I2S]  Wrong Channel Select %d \n", p_i2s_cfg->ch_sel);
    return HAL_I2S_BAD_PARAMETER;
  }

  if(p_i2s_cfg->ch_sel == I2S_CHANNELS_MONO )
  {
    if(p_i2s_cfg->mono_sel > I2S_CHANNEL_MONO_RIGHT)
    {
      printf("[I2S]Wrong Mono channel %d\n", p_i2s_cfg->mono_sel);
      return HAL_I2S_BAD_PARAMETER;
    }
  }

  i2s_slave_assp_tx.mono_sel = p_i2s_cfg->mono_sel;
  i2s_slave_assp_tx.sdma_used = p_i2s_cfg->sdma_used;

  /* call back */
  i2s_slave_cb = handler;
  S3x_Register_Qos_Node(S3X_I2S_A1_CLK);
  return HAL_I2S_SUCCESS;
}

static void HAL_I2S_Slave_Assp_SDMA_Tx(uint32_t *input, int numBytes)
{
  int ch = i2s_slave_assp_tx.sdma_used ;
  int32_t *pSrc = (int32_t *)input;
  i2s_slave_buf_info.buffer_size  = numBytes;
  i2s_slave_buf_info.p_tx_buffer  = input;
  int R_power = 1;
  int n_minus_1 = (numBytes/4 - 1);    /* total # of DMA transfers */
  int32_t *destAddr = (int32_t *)&I2S_SLAVE->TXDMA;
  pSrc += (numBytes/4 - 1);
  SDMA_SRAM_TAB[ch].SRC_DATA_END_PTR = (uint32_t)pSrc;
  SDMA_SRAM_TAB[ch].DST_DATA_END_PTR = (uint32_t)destAddr;
  SDMA_SRAM_TAB[ch].CH_CFG = ((I2S_SDMA_WORD_DST_INC << 30) |
                              (I2S_SDMA_WORD_SRC_DST_SIZE << 28) |
                                (I2S_SDMA_WORD_SRC_INC << 26) |
                                  (I2S_SDMA_WORD_SRC_DST_SIZE << 24) |
                                    (R_power << 14) |
                                      (n_minus_1 << 4) |
                                        (I2S_SDMA_CYCLE_CTRL << 0));

  SDMA->CHNL_ENABLE_SET = (1 << ch);
  SDMA->CHNL_USEBURST_SET = 1;
  SDMA->DMA_CFG = (1 << ch);
}


/**
* @brief HAL_I2S_Slave_Assp_Start_Tx, this will enable clocks and
* initiates the tarnsaction
*
*/
static uint32_t HAL_I2S_Slave_Assp_Tx_Buffer(uint32_t *p_rx_buffer,
                                            uint32_t *p_tx_buffer,
                                            uint16_t buffer_size)
{
  
  if(0 == i2s_sdma_config_status)
  {
    configASSERT(p_tx_buffer);
    if (buffer_size <= 0)
      return HAL_I2S_TX_ERROR;
    S3x_Set_Qos_Req(S3X_I2S_A1_CLK, MIN_CPU_FREQ, C10_N1_CLK);

    /* Wake up I2S SDMA power domain*/
    PMU->MISC_SW_WU |= (PMU_MISC_SW_WU_SDMA_WU | PMU_MISC_SW_WU_I2S_WU);

    /* Enable the I2S and SDMA clocks */
    S3x_Clk_Enable (S3X_SDMA_SRAM_CLK);
    S3x_Clk_Enable (S3X_I2S_A1_CLK);
    S3x_Clk_Enable (S3X_SDMA_CLK);
    S3x_Clk_Enable (S3X_A1_CLK);

    /* I2S enable register */
    i2s_assp_slave_enable(I2S_SLAVE_ASSP_IER_EN);;
    I2S_SLAVE->IER = 1;
    /* Tx block enable */
    i2s_assp_slave_Tx_enable(I2S_SLAVE_ASSP_TX_BLOCK_DIS);

    /* reset all FIFO's */
    i2s_assp_slave_fifo_reset();

    if (i2s_slave_assp_tx.ch_sel == I2S_CHANNELS_STEREO)
      i2s_assp_slave_stereo_en(I2S_SLAVE_ASSP_STEREO_EN); /* enable stereo */
    else
      i2s_assp_slave_stereo_en(I2S_SLAVE_ASSP_MONO_EN); /* enable mono */
    
    /* Interrupt mask register */
    i2s_assp_slave_tx_overrun_unmask();
    i2s_assp_slave_tx_fifoempty_unmask();

    /* enable Transmit Block */
    i2s_assp_slave_Tx_enable(I2S_SLAVE_ASSP_TX_BLOCK_EN);

    SDMA->CTRL_BASE_PTR = SDMA_SRAM_BASE;
    SDMA_BRIDGE->DMA_REQ = 1<<16;

    NVIC_ClearPendingIRQ(Sdma_Err_IRQn);
    NVIC_ClearPendingIRQ(I2SSlv_M4_IRQn);
    NVIC_ClearPendingIRQ(Sdma_Done0_IRQn);

  }

  HAL_I2S_Slave_Assp_SDMA_Tx(p_tx_buffer, buffer_size);

  if(0 == i2s_sdma_config_status)
  {
    i2s_sdma_config_status = 1;
    NVIC_EnableIRQ(Sdma_Done0_IRQn);
    NVIC_EnableIRQ(Sdma_Err_IRQn);
    NVIC_EnableIRQ(I2SSlv_M4_IRQn);
  }
  return HAL_I2S_SUCCESS;
}

/**
* @brief HAL_I2S_Slave_ASSP_Stop_Tx, this will disable clocks and
* stops the tarnsaction
*
*/
static void HAL_I2S_Slave_ASSP_Stop_Tx(void)
{
  spurious_intr=0;

  i2s_sdma_config_status = 0;
  i2s_assp_slave_tx_fifoempty_mask();
  i2s_assp_slave_tx_overrun_mask();
  /* Tx block Disable */
  i2s_assp_slave_Tx_enable(I2S_SLAVE_ASSP_TX_BLOCK_DIS);

  /* Disable I2S enable register */
  i2s_assp_slave_enable(I2S_SLAVE_ASSP_IER_DIS);

  /* Disable the I2S and SDMA clocks */
  S3x_Clk_Disable (S3X_SDMA_SRAM_CLK);
  S3x_Clk_Disable (S3X_I2S_A1_CLK);
  S3x_Clk_Disable (S3X_SDMA_CLK);
  S3x_Clk_Disable (S3X_A1_CLK);

  NVIC_ClearPendingIRQ(Sdma_Done0_IRQn);
  NVIC_ClearPendingIRQ(Sdma_Err_IRQn);
  NVIC_ClearPendingIRQ(I2SSlv_M4_IRQn);

  NVIC_DisableIRQ(Sdma_Done0_IRQn);
  NVIC_DisableIRQ(Sdma_Err_IRQn);
  NVIC_DisableIRQ(I2SSlv_M4_IRQn);
  S3x_Clear_Qos_Req(S3X_I2S_A1_CLK, MIN_CPU_FREQ);
  return;
}

/**
* @brief HAL_I2S_Slave_Assp_UnInit, this will uninitalize the data structures,
* initialized during init
*
*/
static void HAL_I2S_Slave_Assp_UnInit(void)
{
  i2s_slave_assp_tx.ch_sel = 0;
  i2s_slave_assp_tx.i2s_wd_clk = 0;
  i2s_slave_assp_tx.mono_sel = 0;
  i2s_slave_assp_tx.sdma_used = 0;
  i2s_slave_cb = NULL;
  return;
}

//extern int16_t  mem_buffer[2][15*16];
/**
* @brief HAL_I2S_SLAVE_SDMA_done_handler, this will call the callback registered
* by the user. This should be called by sdma0donehandler()
*
*/
void HAL_I2S_SLAVE_SDMA_Assp_Done(void)
{
  NVIC_ClearPendingIRQ(Sdma_Done0_IRQn);  

  //A spurious interrupt is seen for first chunk, TODO: check its cause
  if(!spurious_intr)
  {
    spurious_intr=1;
    return;
  }

  // Invoke the user handler callback
  i2s_slave_cb(I2S_SLAVE_ASSP_TX, NULL,
               i2s_slave_buf_info.p_tx_buffer,
               i2s_slave_buf_info.buffer_size); 
#if AUDIO_LED_TEST
  LedGreenBlink();
#endif
}

/**
* @brief HAL_I2S_Slave_register, this will call the callback registered
* by the user
*
*/
uint32_t HAL_I2S_Slave_Assp_Register(void)
{
  uint32_t ret_val = HAL_I2S_SUCCESS;
  I2S_Drv_t i2s_drv_fn;
  uint8_t i2s_id = I2S_SLAVE_ASSP_TX;
  i2s_drv_fn.initfn = HAL_I2S_Slave_Assp_Init;
  i2s_drv_fn.bufferfn = HAL_I2S_Slave_Assp_Tx_Buffer;
  i2s_drv_fn.stopfn = HAL_I2S_Slave_ASSP_Stop_Tx;
  i2s_drv_fn.un_initfn = HAL_I2S_Slave_Assp_UnInit;
  ret_val = HAL_I2S_Register_Driver(i2s_id, i2s_drv_fn);
  if(ret_val != HAL_I2S_SUCCESS)
    printf("HAL I2S Driver Slave ASSP register failed\n");

  return ret_val;
}
