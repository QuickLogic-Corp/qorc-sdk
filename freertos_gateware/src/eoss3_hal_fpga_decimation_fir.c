/*==========================================================
 * Copyright 2021 QuickLogic Corporation
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
 *    File   : eoss3_hal_fpga_decimation_fir.c
 *    Purpose: This file contains FIR functions for Rx data in fpga.
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#include <stdint.h>
#include <string.h>   

#ifdef ENABLE_FB_FIR_DECIMATION_RX

#include "eoss3_hal_i2s.h"
#include "eoss3_hal_fpga_decimation_fir.h"
#include "eoss3_hal_gpio.h"
#include "s3x_clock.h"

//#define FIR_RX_SINGLE_DMA_SIZE (240) // ???

//bit fields for IER Reg 
#define FIR_IER_REG_COEF_READ_ACCESS       (0x1 << 3)

//bit fields for ISR Reg 
#define FIR_ISR_REG_DMA_DONE               (0x1 << 0)

//bit fields for IEN Reg 
#define FIR_IEN_REG_DMA_DONE_ENABLE        (0x1 << 0)
   
//bit fields for DFSTS Reg 
#define FIR_DFSTS_REG_FIFO_LEVEL_MASK      (0x0FFFF)
#define FIR_DFSTS_REG_FIFO_EMPTY_MASK      (0x10000)
#define FIR_DFSTS_REG_FIFO_FULL_MASK       (0x20000)

//bit fields for DFDREG Reg 
#define FIR_DFDREG_REG_EVEN_DATA_MASK      (0x0000FFFF)
#define FIR_DFDREG_REG_ODD_DATA_MASK       (0xFFFF0000)

//bit fields for DER Reg 
#define FIR_DER_REG_DMA_ENABLE             (0x1 << 0)

//bit fields for DSR Reg 
#define FIR_DSR_REG_DMA_STS_BUSY           (0x1 << 0)
#define FIR_DSR_REG_DMA_STS_DONE           (0x1 << 1)
#define FIR_DSR_REG_DMA_STS_ACTIVE         (0x1 << 2)
#define FIR_DSR_REG_DMA_STS_REQUEST        (0x1 << 3)

//bit fields for DCNT Reg 
#define FIR_DCNT_REG_DMA_COUNT_MASK        (0x01FF)

//bit fields for FDCR Reg 
#define FIR_FDCR_REG_DECIMATION_ENABLE     (0x1 << 0)
#define FIR_FDCR_REG_DECIMATION_INT_ENABLE (0x1 << 1)

//bit fields for FDSR Reg 
#define FIR_FDSR_REG_DECIMATION_ENABLE     (0x1 << 0)

#define FB_DECIM_FIR_COEF_RAM_BASE          ((void*)(FB_DECIMATION_FIR_REG_OFFSET))
#define FB_DECIM_FIR_COEF_RAM_LENGTH_BYTES  (0x800)
#define FB_DECIM_FIR_121_TAP_FILTER_SIZE    (121)

static short FB_Coef_48K_to16K[121] = {
    0, -2, -3, -1, 3, 7, 5, -3, -12, -12, 0, 16, 22, 8, -18, -34, -23, 13, 47, 44, //1-20
    0, -55, -72, -26, 54, 102, 66, -38, -128, -119, 0, 142, 183, 65, -134, -247, -158, 90, 302, 279, //21 -40
    0, -330, -423, -150, 311, 579, 374, -216, -736, -696, 0, 878, 1178, 443, -993, -2045, -1513, 1067, 4945, 8433, //41-60
    9831, //61 = Center Tap
    8433, 4945, 1067, -1513, -2045, -993, 443, 1178, 878, 0, -696, -736, -216, 374, 579, 311, -150, -423, -330, 0,//62-81
    279, 302, 90, -158, -247, -134, 65, 183, 142, 0, -119, -128, -38, 66, 102, 54, -26, -72, -55, 0, //82-101
    44, 47, 13, -23, -34, -18, 8, 22, 16, 0, -12, -12, -3, 5, 7, 3, -1, -3, -2, // 0,  //102 - 121
};

#if 0
//IER Reg functions
__STATIC_INLINE void fir_fb_slave_predeci_write_access_en(void)
{
    FB_FIR_SDMA->FIR_IER |= FIR_IER_REG_COEF_READ_ACCESS;
}
__STATIC_INLINE void fir_fb_slave_predeci_write_access_dis(void)
{
    FB_FIR_SDMA->FIR_IER &= ~FIR_IER_REG_COEF_READ_ACCESS;
}
#endif
//ISR Reg functions
//__STATIC_INLINE 
uint8_t is_fir_fb_slave_dma_done_intr_set(void)
{
    return (FB_FIR_SDMA->FIR_ISR & FIR_ISR_REG_DMA_DONE);
}
__STATIC_INLINE void fir_fb_slave_rx_isr_clear(void)
{
    FB_FIR_SDMA->FIR_ISR = 0;
}
//__STATIC_INLINE 
void fir_fb_slave_rx_dma_done_clear(void)
{
    FB_FIR_SDMA->FIR_ISR &= ~FIR_ISR_REG_DMA_DONE;
}
//DCNT Reg functions
__STATIC_INLINE void fir_fb_slave_rx_dmacount(void)
{
    //FB_FIR_SDMA->FIR_DCNT = FIR_RX_SINGLE_DMA_SIZE *2; 
  //FB_FIR_SDMA->FIR_DCNT = FIR_RX_SINGLE_DMA_SIZE; 
  FB_FIR_SDMA->FIR_DCNT = get_i2s_fb_dma_count(); 
}
//DER Reg functions
//__STATIC_INLINE 
void fir_fb_slave_rx_dma_en(void)
{
    FB_FIR_SDMA->FIR_DER = FIR_DER_REG_DMA_ENABLE;
}
//IEN Reg functions
__STATIC_INLINE void fir_fb_slave_rx_dma_done_en(void)
{
    FB_FIR_SDMA->FIR_IEN |= FIR_IEN_REG_DMA_DONE_ENABLE;
}
__STATIC_INLINE void fir_fb_slave_rx_dma_done_dis(void)
{
    FB_FIR_SDMA->FIR_IEN &= ~FIR_IEN_REG_DMA_DONE_ENABLE;
}
//DFRST Reg functions
__STATIC_INLINE void fir_fb_slave_fir_decimation_fifo_reset(void)
{
    FB_FIR_SDMA->FIR_DFRST = 0x1;
}
//FDCR Reg functions
__STATIC_INLINE void fir_fb_slave_fir_decimation_en(void)
{
    FB_FIR_SDMA->FIR_FDCR |= FIR_FDCR_REG_DECIMATION_ENABLE;
}
__STATIC_INLINE void fir_fb_slave_fir_decimation_dis(void)
{
    FB_FIR_SDMA->FIR_FDCR &= ~FIR_FDCR_REG_DECIMATION_ENABLE;
}


static void Init_Decimation_Coef_RAM(void)
{
    uint32_t *pdest = FB_DECIM_FIR_COEF_RAM_BASE;
    short    *psrc  = FB_Coef_48K_to16K;
    short     i     = 0;
    //can write only 32bit words
    for(i = 0; i < FB_DECIM_FIR_121_TAP_FILTER_SIZE; i++)
    {
#if 0 //just for test, make all coefs 0 except for center tap
      if(i == 61)
        *pdest = 0x4000; //1 in 2.14 format
      else
        *pdest = 0;
#else      
      *pdest = *psrc;
#endif      
      psrc++;
      pdest++;
    }
    //Set remaining FIR coefficient RAM to zeros
    for(;i < (FB_DECIM_FIR_COEF_RAM_LENGTH_BYTES/4); i++)
    {
      *pdest++ = 0;
    }
    return;
}
static void HAL_FIR_Slave_FB_SDMA_Config(void)
{
    //check if correct I2S IP is loaded into FPGA
    //HAL_FIR_Slave_FB_ip_check();
    
    //Load FIR Cofficient RAM with coefficient data 
    Init_Decimation_Coef_RAM();

    //Set SDMA count to 15*16 size
    fir_fb_slave_rx_dmacount();

    //Clear the Interrupt status register
    fir_fb_slave_rx_isr_clear();

    //Enable all the I2S Slave Rx interrupts
    fir_fb_slave_rx_dma_done_en();

    // Enable I2S Slave Rx Fabric SDMA
    fir_fb_slave_rx_dma_en();

    fir_fb_slave_fir_decimation_fifo_reset();

    //Enabling Fabric I2S slave Rx FIR decimation block
    fir_fb_slave_fir_decimation_en();

    return;
}

//need an extern to config
void HAL_FIR_Decimation_FB_SDMA_Config(void)
{
    HAL_FIR_Slave_FB_SDMA_Config();
}

//start FIR Decimation 
uint32_t HAL_FIR_Decimation_Start (void)
{
    HAL_FIR_Decimation_FB_SDMA_Config();

    return HAL_I2S_SUCCESS;
}
//stop FIR decimation
void HAL_FIR_Decimation_Stop (void)
{
    fir_fb_slave_rx_dma_done_clear();
#if 0
    fir_fb_slave_rx_dma_done_dis();

    fir_fb_slave_fir_decimation_dis();
#endif
    fir_fb_slave_fir_decimation_fifo_reset();

    return;
}
//init FIR decimation
uint32_t HAL_FIR_Decimation_Init (void)
{
    return HAL_I2S_SUCCESS;
}
// uninit FIR deciimation
void HAL_FIR_Decimation_Uninit(void)
{
    return;
}


#endif /* ENABLE_FB_FIR_DECIMATION_RX */
