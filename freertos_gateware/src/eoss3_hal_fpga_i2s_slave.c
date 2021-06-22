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
 *    File   : eoss3_hal_fpga_i2s_slave.h
 *    Purpose: This file contains functions for I2S Slave 
 *             Rx implemented in fpga.
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#include <stdint.h>
#include <string.h>   

#include "common.h"

#ifdef ENABLE_I2S_SLAVE_FB_RX

#include "eoss3_hal_i2s.h"
#include "eoss3_hal_fpga_i2s_slave.h"
#include "eoss3_hal_fpga_decimation_fir.h"
#include "eoss3_hal_gpio.h"
#include "s3x_clock.h"

//set this to 1 when FIR Decimation is enabled
#define FIR_DECIM_ENABLED   (1)

//pre-decimation RAM at 48KHz
#define FB_I2S_SLAVE_PRE_DECI_RAM_BASE           ((void*)(FB_I2S_SDMA_BASE))
#define FB_I2S_SLAVE_PRE_DECI_RAM_LENGTH_BYTES   (0x800)

//bit fields for IER Reg 
#define I2S_IER_REG_I2S_ENABLE             (0x1 << 0)
#define I2S_IER_REG_48KRAM_WRITE_ACCESS    (0x1 << 1)

//bit fields for ISR Reg 
#define I2S_ISR_REG_I2S_DISCONNECT         (0x1 << 3)
#define I2S_ISR_REG_I2S_CONNECT            (0x1 << 5)

//bit fields for IEN Reg 
#define I2S_IEN_REG_I2S_DISCONNECT_ENABLE  (0x1 << 3)
#define I2S_IEN_REG_I2S_CONNECT_ENABLE     (0x1 << 5)


//I2S Rx buffers
#define I2S_RX_SINGLE_DMA_SIZE    ((15*16*2)/4)  //15ms @16Khz in 32bit words
#define I2S_RX_DMA_BUFFER_COUNT   (10) //(4)

static uint32_t i2sRxDMABuffers[I2S_RX_SINGLE_DMA_SIZE*I2S_RX_DMA_BUFFER_COUNT]; 
static uint32_t i2sRxDMA_Timestamps[I2S_RX_DMA_BUFFER_COUNT]; 

uint8_t i2s_rx_buf_rd_index;
uint8_t i2s_rx_buf_wr_index;
static uint8_t i2s_rx_sdma_start;

static uint32_t i2s_rx_start_time_ms;

uint32_t i2s_rx_enabled = 0;
static uint32_t i2s_rx_enabled_2 = 0;

static I2S_Config_t i2s_slave_fb_rx;


//IER Reg functions
__STATIC_INLINE void i2s_fb_slave_rx_i2s_en(void)
{
    FB_I2S_SDMA->I2S_IER |= I2S_IER_REG_I2S_ENABLE;
}
__STATIC_INLINE void i2s_fb_slave_rx_i2s_dis(void)
{
    FB_I2S_SDMA->I2S_IER &= ~I2S_IER_REG_I2S_ENABLE;
}
__STATIC_INLINE void i2s_fb_slave_predeci_write_access_en(void)
{
    FB_I2S_SDMA->I2S_IER |= I2S_IER_REG_48KRAM_WRITE_ACCESS;
}
__STATIC_INLINE void i2s_fb_slave_predeci_write_access_dis(void)
{
    FB_I2S_SDMA->I2S_IER &= ~I2S_IER_REG_48KRAM_WRITE_ACCESS;
}
//ISR Reg functions
__STATIC_INLINE uint8_t is_i2s_fb_slave_i2s_connect_intr_set(void)
{
    return (FB_I2S_SDMA->I2S_ISR & I2S_ISR_REG_I2S_CONNECT);
}
__STATIC_INLINE void i2s_fb_slave_rx_isr_clear(void)
{
    FB_I2S_SDMA->I2S_ISR = 0;
}
__STATIC_INLINE void i2s_fb_slave_rx_i2s_disconnect_clear(void)
{
    FB_I2S_SDMA->I2S_ISR &= ~I2S_ISR_REG_I2S_DISCONNECT;
}
#if 1
__STATIC_INLINE void i2s_fb_slave_rx_i2s_connect_clear(void)
{
    FB_I2S_SDMA->I2S_ISR &= ~I2S_ISR_REG_I2S_CONNECT;
}
#endif
__STATIC_INLINE uint8_t is_i2s_fb_slave_i2s_disconnect_intr_set(void)
{
    return (FB_I2S_SDMA->I2S_ISR & I2S_ISR_REG_I2S_DISCONNECT);
}
//IEN Reg functions
__STATIC_INLINE void i2s_fb_slave_rx_i2s_disconnect_en(void)
{
    FB_I2S_SDMA->I2S_IEN |= I2S_IEN_REG_I2S_DISCONNECT_ENABLE;
}
__STATIC_INLINE void i2s_fb_slave_rx_i2s_disconnect_dis(void)
{
    FB_I2S_SDMA->I2S_IEN &= ~I2S_IEN_REG_I2S_DISCONNECT_ENABLE;
}
__STATIC_INLINE int is_i2s_fb_slave_rx_i2s_disconnect_dis(void)
{
    return (FB_I2S_SDMA->I2S_IEN & I2S_IEN_REG_I2S_DISCONNECT_ENABLE);
}

__STATIC_INLINE void i2s_fb_slave_rx_i2s_connect_en(void)
{
    FB_I2S_SDMA->I2S_IEN |= I2S_IEN_REG_I2S_CONNECT_ENABLE;
}
__STATIC_INLINE void i2s_fb_slave_rx_i2s_connect_dis(void)
{
    FB_I2S_SDMA->I2S_IEN &= ~I2S_IEN_REG_I2S_CONNECT_ENABLE;
}
__STATIC_INLINE int is_i2s_fb_slave_rx_i2s_connect_en(void)
{
   return (FB_I2S_SDMA->I2S_IEN & I2S_IEN_REG_I2S_CONNECT_ENABLE);
}

int get_i2s_fb_dma_count(void)
{
  return I2S_RX_SINGLE_DMA_SIZE*2;
}

void enable_i2s_disconnect(void)
{
  i2s_fb_slave_rx_i2s_disconnect_en();
}

// clear the buffers and reset the state 
static void init_i2s_rx_buffers(void)
{
  memset(i2sRxDMABuffers, 0, sizeof(i2sRxDMABuffers));
  memset(i2sRxDMA_Timestamps, 0, sizeof(i2sRxDMA_Timestamps));
 
  i2s_rx_buf_rd_index = 0;
  i2s_rx_buf_wr_index = 0;
  
  i2s_rx_sdma_start = 0;
  
  return;
}

static void set_i2s_start_time(uint32_t ticks_ms)
{
  i2s_rx_start_time_ms = ticks_ms;
}

uint32_t get_i2s_start_time(void )
{
  return i2s_rx_start_time_ms;
}

void HAL_I2S_Slave_FB_ip_check(void)
{
  uint32_t *ip_id = (uint32_t *)FPGA_PERIPH_BASE;
  printf("FPGA IP ID 0x%08X \n", *ip_id);
  ip_id++;
  printf("FPGA IP Rev 0x%08X \n", *ip_id);
  
}
//restart the Data bus
static void Reset_48K_Data_RAM(void)
{
    uint32_t *pdest = (uint32_t *)FB_I2S_SLAVE_PRE_DECI_RAM_BASE;

    //First need enable write access
    i2s_fb_slave_predeci_write_access_en();
    //can write only 32bit words
    for(int i = 0; i < (FB_I2S_SLAVE_PRE_DECI_RAM_LENGTH_BYTES/4); i++)
    {
      *pdest++ = 0;
    }
    i2s_fb_slave_predeci_write_access_dis();
    return;
}
static void HAL_I2S_Slave_FB_SDMA_Config(void) //uint32_t frame_size)
{
    //check if correct I2S IP is loaded into FPGA
//    HAL_I2S_Slave_FB_ip_check();
    
    //Reset I2S_48KHz RAM to zero before start
    Reset_48K_Data_RAM();
    
    //Clear the Interrupt status register
    i2s_fb_slave_rx_isr_clear();

    //Enable all the I2S Slave Rx interrupts
    //i2s_fb_slave_rx_i2s_disconnect_en();
    i2s_fb_slave_rx_i2s_disconnect_dis();

    i2s_fb_slave_rx_i2s_connect_en();

    //Finally Enable I2S slave Rx 
    i2s_fb_slave_rx_i2s_en();
    
    //reset the start time
    set_i2s_start_time(0);

    return;
}

static void HAL_I2S_FB_Enable_SDMA_Intr(void)
{
    //NVIC_SetPriority(FbMsg_IRQn, 6);
    NVIC_ClearPendingIRQ(FbMsg_IRQn);
//    NVIC_EnableIRQ(FbMsg_IRQn);
}

static void HAL_I2S_FB_Disable_SDMA_Intr( void )
{
    NVIC_ClearPendingIRQ(FbMsg_IRQn);
    NVIC_DisableIRQ(FbMsg_IRQn);
}

#define I2S_SDMA_WORD_SRC_DST_SIZE      (0x2) /* Word 32 bit*/
#define I2S_SDMA_WORD_SRC_INC           (0x3) /* No Increment */
#define I2S_SDMA_WORD_DST_INC           (0x2) /* Destination increment */
#define I2S_SDMA_CYCLE_CTRL             (0x1) /* Basic cycle control */

void HAL_I2S_Slave_FB_SDMA_Rx(uint32_t *input, int numSamples)
{
    int ch = i2s_slave_fb_rx.sdma_used ;
    int32_t *destAddr = (int32_t *)input;
    int32_t numWords = numSamples/2;

    uint32_t R_power = SDMA_SRAM_R_POWER_512;
    int n_minus_1 = (numWords - 1);        /* total # of DMA transfers */

    destAddr += (numWords - 1);
#if (FIR_DECIM_ENABLED == 1)
    int32_t *pSrc = (int32_t *)&FB_FIR_SDMA->FIR_DFDREG;
#else //this should be used if all are together   
    int32_t *pSrc = (int32_t *)&FB_I2S_SDMA->I2S_DFDREG;
#endif
    SDMA_SRAM_TAB[ch].SRC_DATA_END_PTR = (uint32_t)pSrc;
    SDMA_SRAM_TAB[ch].DST_DATA_END_PTR = (uint32_t)destAddr;

    SDMA_SRAM_TAB[ch].CH_CFG = ((I2S_SDMA_WORD_DST_INC << 30) |
                              (I2S_SDMA_WORD_SRC_DST_SIZE << 28) |
                                (I2S_SDMA_WORD_SRC_INC << 26) |
                                  (I2S_SDMA_WORD_SRC_DST_SIZE << 24) |
                                    (R_power) |
                                      (n_minus_1 << 4) |
                                        (I2S_SDMA_CYCLE_CTRL << 0));
    SDMA->CHNL_ENABLE_SET |= 1 << ch;
    SDMA->DMA_CFG = SDMA_DMA_CFG_MASTER_ENABLE;
    SDMA->CTRL_BASE_PTR = SDMA_SRAM_BASE;

    //SDMA->DMA_CFG = 0;
}
extern void cb_notify_i2sRx_disable_intr_from_FPGA_ISR(void);

static int fir_enabled = 0;
void HAL_I2S_Slave_FB_ISR_Handler(void)
{
  
  i2s_rx_enabled++;

#if (FIR_DECIM_ENABLED == 1)
  
    // Check interrupt status for DMA transfer Done 
    if(is_fir_fb_slave_dma_done_intr_set())
    {
        // Clear the DMA Done interrupt
        fir_fb_slave_rx_dma_done_clear();
        
        HAL_FB_I2SRx_Ref_input_DmaNext();

        fir_fb_slave_rx_dma_en();//I2S_SLAVE_FB_RX_DER_EN);

        if(i2s_rx_enabled == 4)
          fir_enabled = 1;
        else
          fir_enabled++;
            
    }

#endif    

    //check if I2S is disconnected
    if(is_i2s_fb_slave_i2s_disconnect_intr_set())
    {
      //clear it and send a message
      i2s_fb_slave_rx_i2s_disconnect_clear();
      
      //disable further interrupts
      i2s_fb_slave_rx_i2s_disconnect_dis();
      
      //send the interrupt that I2S is disconnected
      cb_notify_i2sRx_disable_intr_from_FPGA_ISR();
    }
    
    //check if I2S is connected
    //if(is_i2s_fb_slave_rx_i2s_connect_en())
    {
      if(is_i2s_fb_slave_i2s_connect_intr_set())
      {
        //clear the interrupt and disable further interrupts
        i2s_fb_slave_rx_i2s_connect_clear();
        
        //disable further interrupts
        i2s_fb_slave_rx_i2s_connect_dis();
        
        //set the first I2S sample Rx time
        set_i2s_start_time(xTaskGetTickCount());
      }
    }
    //check if i2s is disconnected
/*    if(is_i2s_fb_slave_rx_i2s_disconnect_dis())
    {
      //disable it and send a message
      i2s_fb_slave_rx_i2s_disconnect_dis();
      
      //send the message I2S is disabled
      //cb_notify_i2sRx_disable_intr_from_FPGA_ISR();
    }
*/   
    NVIC_ClearPendingIRQ(FbMsg_IRQn);

}
#if 0
void HAL_I2S_Slave_FB_SDMA_stop(void)
{
    HAL_I2S_Stop(I2S_SLAVE_FABRIC_RX);
}
#endif
static void HAL_I2S_Slave_FB_ConfigureInterrupt(void)
{
    /* Configure interrupt for I2S RX */
    FB_RegisterISR(FB_INTERRUPT_2, HAL_I2S_Slave_FB_ISR_Handler);
    FB_ConfigureInterrupt(FB_INTERRUPT_2, FB_INTERRUPT_TYPE_LEVEL,
                        FB_INTERRUPT_POL_LEVEL_HIGH,
                        FB_INTERRUPT_DEST_AP_DISBLE, FB_INTERRUPT_DEST_M4_ENABLE);

#if 0 //only one interrrupt is used for all I2S status
    /* Configure interrupt for DMA STOP when clock line is low */
    FB_RegisterISR(FB_INTERRUPT_3, HAL_I2S_Slave_FB_SDMA_stop);
    FB_ConfigureInterrupt(FB_INTERRUPT_3, FB_INTERRUPT_TYPE_LEVEL,
                        FB_INTERRUPT_POL_LEVEL_HIGH,
                        FB_INTERRUPT_DEST_AP_DISBLE, FB_INTERRUPT_DEST_M4_ENABLE);
#endif
}

/* Start I2S Fabric Slave Rx */
static uint32_t HAL_I2S_Slave_FB_Rx_Start (uint32_t *p_rx_buffer, uint32_t *p_tx_buffer, uint16_t buffer_size)
{
    configASSERT(p_rx_buffer);
    configASSERT(buffer_size == I2S_RX_SINGLE_DMA_SIZE);
    

    if(i2s_rx_sdma_start)
      return HAL_I2S_SUCCESS;
    
    i2s_rx_sdma_start = 1;
    init_i2s_rx_buffers();
      
    if (buffer_size <= 0)
        return HAL_I2S_RX_ERROR;

    /* Wakeup SDMA domain */
    PMU->MISC_SW_WU |= PMU_MISC_SW_WU_SDMA_WU;

    HAL_I2S_Slave_FB_SDMA_Rx((uint32_t*)&i2sRxDMABuffers[0], I2S_RX_SINGLE_DMA_SIZE*2);

    
#if (FIR_DECIM_ENABLED == 1)    
    //config FIR Decimation IP
    HAL_FIR_Decimation_FB_SDMA_Config();
#endif

    // Start the I2S RX DMA
    HAL_I2S_Slave_FB_SDMA_Config(); 
    HAL_I2S_FB_Enable_SDMA_Intr();
    
    //enable I2S again ???
    //i2s_fb_slave_rx_i2s_en();

    return HAL_I2S_SUCCESS;
}


static void HAL_I2S_Slave_FB_Rx_Stop (void)
{
#if (FIR_DECIM_ENABLED == 1)
    //first stop FIR decimation 
    HAL_FIR_Decimation_Stop();
#endif 
    
    i2s_fb_slave_rx_i2s_disconnect_clear();
    i2s_fb_slave_rx_i2s_disconnect_dis();

    i2s_fb_slave_rx_i2s_dis();

    eoss3_hal_fabric_i2s_slave_clks_disable();

    HAL_I2S_FB_Disable_SDMA_Intr();
    
    i2s_rx_sdma_start = 0;
 
    i2s_rx_enabled = 0;
    return;
}

// Initialize I2S Fabric 
static uint32_t HAL_I2S_Slave_FB_Init (I2S_Config_t *p_i2s_cfg, HAL_I2S_Cb_Handler_t handler)
{
    i2s_rx_enabled = 0;
    
    // Check for the validity of I2S configuration
    if (p_i2s_cfg == NULL)
    {
        printf("HAL FB I2S Configuration are not valid \n");
        return HAL_I2S_BAD_PARAMETER;
    }

    //Check the validity as well as save the I2S Configuration
    if (p_i2s_cfg->ch_sel == I2S_CHANNELS_STEREO)
        i2s_slave_fb_rx.ch_sel  = I2S_CHANNELS_STEREO;
    else if (p_i2s_cfg->ch_sel == I2S_CHANNELS_MONO)
        i2s_slave_fb_rx.ch_sel = I2S_CHANNELS_MONO;
    else
    {
        printf("HAL FB I2S  Wrong Channel Select %d \n", p_i2s_cfg->ch_sel);
        return HAL_I2S_BAD_PARAMETER;
    }

    if(p_i2s_cfg->ch_sel == I2S_CHANNELS_MONO)
    {
        if(p_i2s_cfg->mono_sel > I2S_CHANNEL_MONO_RIGHT)
        {
            printf("[I2S]Wrong Mono channel %d\n", p_i2s_cfg->mono_sel);
            return HAL_I2S_BAD_PARAMETER;
        }
    }

    i2s_slave_fb_rx.mono_sel = p_i2s_cfg->mono_sel;
    i2s_slave_fb_rx.sdma_used = p_i2s_cfg->sdma_used;

    HAL_I2S_Slave_FB_ConfigureInterrupt();

    return HAL_I2S_SUCCESS;
}

// Uninitialize I2S Master
static void HAL_I2S_Slave_FB_Uninit(void)
{
   //Note: this function does not do anything

    //FB_UnRegisterISR(FB_INTERRUPT_2,NULL);
    //FB_UnRegisterISR(FB_INTERRUPT_3,NULL);
    return;
}

uint32_t HAL_I2S_Slave_FB_Register(void)
{
    uint32_t ret_val = HAL_I2S_SUCCESS;

    I2S_Drv_t i2s_drv_fn;
    uint8_t i2s_id = I2S_SLAVE_FABRIC_RX;

    i2s_drv_fn.initfn = HAL_I2S_Slave_FB_Init;
    i2s_drv_fn.bufferfn = HAL_I2S_Slave_FB_Rx_Start;
    i2s_drv_fn.stopfn = HAL_I2S_Slave_FB_Rx_Stop;
    i2s_drv_fn.un_initfn = HAL_I2S_Slave_FB_Uninit;

    // Register to I2S Driver
    ret_val = HAL_I2S_Register_Driver(i2s_id, i2s_drv_fn);
    if (ret_val != HAL_I2S_SUCCESS)
        printf("HAL FB RX I2S Driver Registration Failed \n");

    return ret_val;
}

static void Enable_SDMA_clk(void)
{
   S3x_Clk_Enable (S3X_SDMA_CLK);
   S3x_Clk_Enable (S3X_SDMA_SRAM_CLK);
}

static void Disable_SDMA_clk(void)
{
   S3x_Clk_Disable (S3X_SDMA_SRAM_CLK);
   S3x_Clk_Disable (S3X_SDMA_CLK);
}

static void Enable_FB_clk(void)
{
   S3x_Clk_Enable(S3X_FB_21_CLK);
   S3x_Clk_Enable(S3X_FB_16_CLK);
}

static void Disable_FB_clk(void)
{
   S3x_Clk_Disable(S3X_FB_21_CLK);
   S3x_Clk_Disable(S3X_FB_16_CLK);
}
static void Set_FB_clk(void)
{
  S3x_Clk_Set_Rate(S3X_FB_21_CLK, 3*1024*1000); //for 16K sample rate = 2*32*16K = 1024000 
  S3x_Clk_Set_Rate(S3X_FB_16_CLK, 1*6*1024*1000); //for FIR decimation at 16Khz
}
void eoss3_hal_fabric_i2s_slave_clks_enable()
{
   Set_FB_clk();
   Enable_FB_clk();
   Enable_SDMA_clk();
#if 0
   printf("S3X_FB_21_CLK = %d,  ", S3x_Clk_Get_Rate(S3X_FB_21_CLK));
   printf("S3X_FB_16_CLK = %d \n", S3x_Clk_Get_Rate(S3X_FB_16_CLK));
#endif
}

void eoss3_hal_fabric_i2s_slave_clks_disable()
{
   Disable_FB_clk();
   Disable_SDMA_clk();
}


void HAL_FB_I2SRx_Ref_input_DmaStart(void)
{
  HAL_I2S_TX_RX_Buffer(I2S_SLAVE_FABRIC_RX, (uint32_t*)&i2sRxDMABuffers[0], NULL, I2S_RX_SINGLE_DMA_SIZE);
}
void HAL_FB_I2SRx_Ref_input_DmaNext(void)
{
  //store the Time stamp when the buffer is (filled) received
  //i2sRxDMA_Timestamps[i2s_rx_buf_wr_index] = xTaskGetTickCount();
  i2sRxDMA_Timestamps[i2s_rx_buf_wr_index] = xTaskGetTickCountFromISR();

  //set the next buffer to fill
  i2s_rx_buf_wr_index++;
  if(i2s_rx_buf_wr_index >= I2S_RX_DMA_BUFFER_COUNT)
    i2s_rx_buf_wr_index = 0;
  HAL_I2S_Slave_FB_SDMA_Rx((uint32_t*)&i2sRxDMABuffers[I2S_RX_SINGLE_DMA_SIZE*i2s_rx_buf_wr_index], 
                            I2S_RX_SINGLE_DMA_SIZE*2);
}

void reset_aec_counts(void) {
    i2s_rx_buf_wr_index = 0;
    i2s_rx_buf_rd_index = 0;
}

uint32_t aec_echo_time;
int aec_enabled = 0;
//this assumes that I2S data is already on
uint16_t *get_aec_signal(int aec_ref_state)
{
  
  if(i2s_rx_enabled == 0) {  
    //aec_enabled = 0;
    return 0;
  }
  if (aec_enabled == 0) {
    if(i2s_rx_buf_wr_index >= 5) {
      aec_enabled = 1;
      i2s_rx_buf_rd_index = i2s_rx_buf_wr_index -5;
    } 
  }

  if(aec_enabled == 0)
    return 0;
    
  uint32_t *signal = (uint32_t*)&i2sRxDMABuffers[I2S_RX_SINGLE_DMA_SIZE*i2s_rx_buf_rd_index];
  aec_echo_time = i2sRxDMA_Timestamps[i2s_rx_buf_rd_index];
  
  i2s_rx_buf_rd_index++;
  if(i2s_rx_buf_rd_index >= I2S_RX_DMA_BUFFER_COUNT)
    i2s_rx_buf_rd_index = 0;

  return (uint16_t *)signal;
}
uint32_t get_aec_time(void)
{
  return i2sRxDMA_Timestamps[i2s_rx_buf_rd_index];
}

#endif /* ENABLE_I2S_SLAVE_FB_RX */
