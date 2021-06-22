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
#if (FEATURE_FLL_I2S_DEVICE == 1)

#include <FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include "eoss3_dev.h"
#include "eoss3_hal_fpga_FLL.h"
#include "s3x_clock_hal.h"
#include "s3x_clock.h"

//define the local clk  (i2s bit clock) only if not defined
#ifndef FLL_I2S_LOCAL_CLK
#define FLL_I2S_LOCAL_CLK  (1*1024*1000) //for 16K sample rate = 2*32*16K = 1024000
#endif

/* Note: The Sample Time is used only to take a decision whether to generate
* an interrupt or not. But the interrupt may not be generated every Sample Time.
* Only if both the Word Counts and Sample Counts indicate that the Clk is fast or
* slow then only the interrupt is generated.
* The Gap Time is used to just reduce the number of times the Sample Timer is counted
* down there by reducing the number if instances that the interrupt generator compares
* the Word counts and Sample Counts internally.
* Too large a Sample Time will force interrupts to appear every time.
* Too small a Sample Time may never lead to generation of interruption, even of Word
* differ. So, the Word Counts may drift.
*/
#ifndef FLL_SAMPLE_TIME_DEFAULT
#define FLL_SAMPLE_TIME_DEFAULT  (10*1024) //=10ms at 16K sample rate
#endif
#ifndef FLL_GAP_TIME_DEFAULT
#define FLL_GAP_TIME_DEFAULT     (90*1024) //=90ms  at 16K sample rate
#endif
#ifndef MAX_FLL_DAC_DRIFT
#define MAX_FLL_DAC_DRIFT    (30)  // >= 30*32768Hz. Note: 1 DAC count is more than 32768Hz
#endif

void enable_hsosc_dac_debug(void);
void disable_hsosc_dac_debug(void);
int get_hsosc_dac(void);

int slow_count =0;
int speed_count =0;
int dac_start = 0;
int dac_end = 0;

int slow_drift = 0;
int speed_drift = 0;

//Enable the FLL
void HAL_FB_FLL_Enable(void) {
  
  enable_hsosc_dac_debug();
  S3x_Clk_Enable(S3X_FB_21_CLK);
  S3x_Clk_Enable(S3X_FB_16_CLK);
  FB_FLL->CNTRL_REG = 1;

  slow_count =0;
  speed_count =0;
  dac_start = get_hsosc_dac();
  dac_end = dac_start;
  slow_drift = 0;
  speed_drift = 0;

  
  return;
}
//Disable the FLL
void HAL_FB_FLL_Disable(void) {

#if 0 //enable for debug only  

  int local = FB_FLL->LOCAL_WORD_COUNT;
  int master = FB_FLL->MASTER_WORD_COUNT;
  printf("FLL DBG word counts: Local = %d, Master = %d, diff = %d \n",local, master, local-master);
  printf("FLL DBG diff counts: Diff = %d, LSBs = %d \n",FB_FLL->WORD_COUNT_DIFF,FB_FLL->LSB_WORD_COUNTS);

#endif

  FB_FLL->CNTRL_REG = 0;
#if (AEC_ENABLED == 0)
  S3x_Clk_Disable(S3X_FB_21_CLK);
  S3x_Clk_Disable(S3X_FB_16_CLK);
#endif
  
  dac_end = get_hsosc_dac();
  disable_hsosc_dac_debug();
  
#if 0 //enable for debug 
  //convert to KHz(1024Hz)
  int dac_start_hz = (dac_start +3)*32;
  int dac_end_hz = (dac_end +3)*32;
  int clk_change = (speed_count - slow_count)*32;//this the total HSOSC change in KHz(1024)
  int i2s_clk_change = clk_change/12; //this is change in I2S clk assuming Streaming QoS Clk is 12MHz
  
  printf(" FLL ISR counts: Slow = %d, Speed = %d, total clk change = %d KHz, i2s clk change = %d KHz \n",
          slow_count,speed_count, clk_change, i2s_clk_change); 
  printf(" FLL Drift counts: Slow drift = %d, Speed drift = %d, total Slow = %d, total Speed = %d \n",
          slow_drift,speed_drift,slow_count+slow_drift,speed_count+speed_drift); 
  printf(" HSOSC DAC: Start = %d (%d KHz), End = %d (%d KHz), diff = %d (%d KHz) \n", 
          dac_start,dac_start_hz, dac_end, dac_end_hz, dac_end - dac_start, dac_end_hz - dac_start_hz);
#endif
  
  return;
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

#if (AEC_ENABLED == 0)  
  // set the Local clk rate and wishbone clock rate
  S3x_Clk_Set_Rate(S3X_FB_21_CLK, FLL_I2S_LOCAL_CLK); //for 16K sample rate = 2*32*16K = 1024000
  S3x_Clk_Set_Rate(S3X_FB_16_CLK, FLL_I2S_LOCAL_CLK); //since no data transfer, keep it low
#endif
  
  // set the time counts to default
  HAL_FB_FLL_Set_Sample_Time(FLL_SAMPLE_TIME_DEFAULT);
  HAL_FB_FLL_Set_Gap_Time(FLL_GAP_TIME_DEFAULT);
  //HAL_FB_FLL_Set_Gap_Time(FLL_SAMPLE_TIME_DEFAULT);
  
  //S3x_Clk_Enable(S3X_FB_21_CLK);
  //S3x_Clk_Enable(S3X_FB_16_CLK);

  //Clear any pending interrupt of FPGA 
  NVIC_ClearPendingIRQ(FbMsg_IRQn);

  //Register FPGA ISR callbacks FPGA Slow Down and Speed up interrupts 
  FB_RegisterISR(FB_INTERRUPT_0, slow_down_fn);
  FB_ConfigureInterrupt(FB_INTERRUPT_0, FB_INTERRUPT_TYPE_EDGE,
                        FB_INTERRUPT_POL_LEVEL_HIGH,
                        FB_INTERRUPT_DEST_AP_DISBLE, FB_INTERRUPT_DEST_M4_ENABLE);
  
  FB_RegisterISR(FB_INTERRUPT_1, speed_up_fn);
  FB_ConfigureInterrupt(FB_INTERRUPT_1, FB_INTERRUPT_TYPE_EDGE,
                        FB_INTERRUPT_POL_LEVEL_HIGH,
                        FB_INTERRUPT_DEST_AP_DISBLE, FB_INTERRUPT_DEST_M4_ENABLE);
#if (AEC_ENABLED == 0)
  //Enable the FPGA interrupts
  NVIC_EnableIRQ(FbMsg_IRQn);
#endif
  //Enable the FLL by default
  //HAL_FB_FLL_Enable();
  
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
  bits = bits; // & 0x1FF; //only 9bits
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
  //int dummy;
  //AIP->OSC_CTRL_0 
  AIP->OSC_CTRL_4 = 0x00; //reset 
  AIP->OSC_CTRL_4 = 0x10; //enable read
  int count =0;
  while(count != 12)
  {
    bits = bits << 1;
    AIP->OSC_CTRL_6 = 0x1; //clk high
    AIP->OSC_CTRL_6 = 0x0; //clk low
    //dummy = AIP->OSC_STA_1;
    bits |= (AIP->OSC_STA_1 & 0x1); //sdo bit
    count++;
  }
  
  //AIP->OSC_CTRL_4 = 0x00; //reset 
  AIP->OSC_CTRL_4 = 0x08; //reserved 
  //printf("OSC_DAC = 0x%8x \n", bits);
  return bits; // & 0x1ff;
}
void enable_hsosc_dac_debug(void)
{
  AIP->OSC_CTRL_3 = 0; //monitor disable, refok = 0
  AIP->RTC_CTRL_1 = 0x1F;//1K divider
}
void disable_hsosc_dac_debug(void)
{
  int rate = 0x1F; // ???
  AIP->OSC_CTRL_3 = 0x8; //monitor enable, refok = 0
  AIP->RTC_CTRL_1 = rate;//1K divider
}
//returns 0 if DAC value change is with in a max value
int check_fll_drift(int dac_bits)
{
  int drift = abs(dac_bits - dac_start);
  //if the drift is a lot change the dac_start
  if(drift > (8*MAX_FLL_DAC_DRIFT))
  {
    //dac_start = get_hsosc_dac();
    dac_start = dac_bits; 
    return 1;
  }
  if(drift < MAX_FLL_DAC_DRIFT)
    return 0;
  else
    return 1; //drift is over the limit
}
void slow_down_ISR(void)
{
    int bits, bits2;
    //enable_hsosc_dac_debug();
    bits = get_hsosc_dac();

    //check if the DAC value changed above a limit
    if(check_fll_drift(bits)) {
      slow_drift++;
      return;
    }
    
    bits2 = (bits - 1); // & 0x1FF;
    if(bits2 < bits)
    {
      set_hsosc_dac(bits2);
    }
    //int rate = 0x1F; // ???
    //disable_hsosc_dac_debug( rate);
    slow_count++;
    return;
}

void speed_up_ISR(void)
{
    int bits, bits2;
    //enable_hsosc_dac_debug();
    bits = get_hsosc_dac();

    //check if the DAC value changed above a limit
    if(check_fll_drift(bits)) {
      speed_drift++;
      return;
    }

    bits2 = (bits + 1);// & 0x1FF;
    if(bits2 > bits)
    {
      set_hsosc_dac(bits2);
    }
    //int rate = 0x1F; // ???
    //disable_hsosc_dac_debug();
    speed_count++;
    return;
}

#endif // FEATURE_FLL_I2S_DEVICE