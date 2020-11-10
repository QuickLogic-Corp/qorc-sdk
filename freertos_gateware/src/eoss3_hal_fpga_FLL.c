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
 *    File   : eoss3_hal_fpga_FLL.c
 *    Purpose: 
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#include <FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include "eoss3_dev.h"
#include "eoss3_hal_fpga_FLL.h"
#include "s3x_clock_hal.h"
#include "s3x_clock.h"

//define the local clk  (i2s bit clock) only if not defined
#ifndef FLL_I2S_LOCAL_CLK
#define FLL_I2S_LOCAL_CLK  (1*1024*1000 + 4*32768) //for 16K sample rate = 2*32*16K = 1024000
#endif

//Enable the FLL
void HAL_FB_FLL_Enable(void) {
  FB_FLL->CNTRL_REG = 1;
}
//Disable the FLL
void HAL_FB_FLL_Disable(void) {
  FB_FLL->CNTRL_REG = 0;
}
//Set the Time in local clks to compare external and local clks
void HAL_FB_FLL_Set_Sample_Time( int sample_count) {
  FB_FLL->SAMPLE_TIMER = sample_count;
}
//Get current Sample Time
int HAL_FB_FLL_Get_Sample_Time(void) { 
  return FB_FLL->SAMPLE_TIMER;
}
//Set the Time in local clks to wait before next comparision
void HAL_FB_FLL_Set_Gap_Time( int gap_count) {
  FB_FLL->GAP_TIMER = gap_count;
}
//Get current Gap Time
int HAL_FB_FLL_Get_Gap_Time(void) {
  return FB_FLL->GAP_TIMER;
}
//This initilizes Clks and FPGA ISR functions for interrupts from the FLL 
void HAL_FB_FLL_Init(HAL_FBISRfunction slow_down_fn, HAL_FBISRfunction speed_up_fn) {

  // if null functions are passed use the default ISR functions
  if(slow_down_fn == 0)
    slow_down_fn = slow_down_ISR;
  if(speed_up_fn == 0)
    speed_up_fn  = speed_up_ISR;
  
  // set the Local clk rate and wishbone clock rate
  S3x_Clk_Set_Rate(S3X_FB_21_CLK, FLL_I2S_LOCAL_CLK); //for 16K sample rate = 2*32*16K = 1024000
  S3x_Clk_Set_Rate(S3X_FB_16_CLK, 1*1024*1000); //since no data transfer, keep it low

  S3x_Clk_Enable(S3X_FB_21_CLK);
  S3x_Clk_Enable(S3X_FB_16_CLK);

  //Clear any pending interrupt of FPGA 
  NVIC_ClearPendingIRQ(FbMsg_IRQn);

  //Register FPGA ISR callbacks FPGA Slow Down and Speed up interrupts 
  FB_RegisterISR(FB_INTERRUPT_0, slow_down_fn);
  FB_ConfigureInterrupt(FB_INTERRUPT_0, FB_INTERRUPT_TYPE_LEVEL,
                        FB_INTERRUPT_POL_LEVEL_HIGH,
                        FB_INTERRUPT_DEST_AP_DISBLE, FB_INTERRUPT_DEST_M4_ENABLE);
  
  FB_RegisterISR(FB_INTERRUPT_1, speed_up_fn);
  FB_ConfigureInterrupt(FB_INTERRUPT_0, FB_INTERRUPT_TYPE_LEVEL,
                        FB_INTERRUPT_POL_LEVEL_HIGH,
                        FB_INTERRUPT_DEST_AP_DISBLE, FB_INTERRUPT_DEST_M4_ENABLE);

  //Enable the FPGA interrupts
  NVIC_EnableIRQ(FbMsg_IRQn);

  //Enable the FLL by default
  HAL_FB_FLL_Enable();
  
  return;
}
// These are for incrementing and decrementing HSOSC DAC directly
void set_hsosc_dac(int bits)
{
  //int bits = 0;
  //AIP->OSC_CTRL_0 
  AIP->OSC_CTRL_4 = 0x00; //reset 
  AIP->OSC_CTRL_4 = 0x30; //enable write
  int count =0;
  AIP->OSC_CTRL_6 = 0x0; //clk low
  bits = bits & 0x1FF; //only 9bits
  while(count != 12)
  {
    AIP->OSC_CTRL_5 = (bits >> (11-count)) & 0x1; //sdi bit - MSB first
    AIP->OSC_CTRL_6 = 0x1; //clk high - rising edge to latch data
    AIP->OSC_CTRL_6 = 0x0; //clk low
    count++;
  }
  
  //AIP->OSC_CTRL_4 = 0x00; //reset 
  AIP->OSC_CTRL_4 = 0x08; //reserved 
  return;
}
int get_hsosc_dac(void)
{
  int bits = 0;
  int dummy;
  //AIP->OSC_CTRL_0 
  AIP->OSC_CTRL_4 = 0x00; //reset 
  AIP->OSC_CTRL_4 = 0x10; //enable read
  int count =0;
  while(count != 12)
  {
    bits = bits << 1;
    AIP->OSC_CTRL_6 = 0x1; //clk high
    AIP->OSC_CTRL_6 = 0x0; //clk low
    dummy = AIP->OSC_STA_1;
    bits |= (AIP->OSC_STA_1 & 0x1); //sdo bit
    count++;
  }
  
  //AIP->OSC_CTRL_4 = 0x00; //reset 
  AIP->OSC_CTRL_4 = 0x08; //reserved 
  //printf("OSC_DAC = 0x%8x \n", bits);
  return bits & 0x1ff;
}
inline void enable_hsosc_dac_debug(void)
{
  AIP->OSC_CTRL_3 = 0; //monitor disable, refok = 0
  AIP->RTC_CTRL_1 = 0x1F;//1K divider
}
inline void disable_hsosc_dac_debug(int rate)
{
  AIP->OSC_CTRL_3 = 0x8; //monitor enable, refok = 0
  AIP->RTC_CTRL_1 = rate;//1K divider
  
}

void slow_down_ISR(void)
{
    int bits, bits2;
    enable_hsosc_dac_debug();
    bits = get_hsosc_dac();
    bits2 = (bits - 1) & 0x1FF;
    if(bits2 < bits)
    {
      set_hsosc_dac(bits2);
    }
    int rate = 0x1F; // ???
    disable_hsosc_dac_debug( rate);
    
    return;
}

void speed_up_ISR(void)
{
    int bits, bits2;
    enable_hsosc_dac_debug();
    bits = get_hsosc_dac();
    bits2 = (bits + 1) & 0x1FF;
    if(bits2 > bits)
    {
      set_hsosc_dac(bits2);
    }
    int rate = 0x1F; // ???
    disable_hsosc_dac_debug( rate);
    
    return;
}
