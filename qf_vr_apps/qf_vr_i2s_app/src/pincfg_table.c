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

#include "Fw_global_config.h"
#include "eoss3_hal_pad_config.h"
#include "eoss3_hal_gpio.h"

PadConfig pincfg_table[] = 
{
  { // setup UART TX
    .ucPin = PAD_44,
    .ucFunc = PAD44_FUNC_SEL_UART_TXD,
    .ucCtrl = PAD_CTRL_SRC_A0,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  { // setup UART RX
    .ucPin = PAD_45,                     // Options: 14, 16, 25, or 45
    .ucFunc = PAD45_FUNC_SEL_UART_RXD,
    .ucCtrl = PAD_CTRL_SRC_A0,
    .ucMode = PAD_MODE_INPUT_EN,
    .ucPull = PAD_NOPULL,
  },

  { // setup debug input pins Pad 14 - S3_SW_DP_CLK
    .ucMode = PAD_MODE_INPUT_EN,
    .ucCtrl = PAD_CTRL_SRC_OTHER,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
    .ucPin = PAD_14,
    .ucFunc = PAD14_FUNC_SEL_SW_DP_CLK,
    .ucPull = PAD_NOPULL,
  },

  { //setup debug input pins Pad 15 - S3_SW_DP_IO
    .ucMode = PAD_MODE_INPUT_EN,
    .ucCtrl = PAD_CTRL_SRC_OTHER,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
    .ucPin = PAD_15,
    .ucFunc = PAD15_FUNC_SEL_SW_DP_IO,
    .ucPull = PAD_NOPULL
  },

  {
    //Currently Pad 9 is not configured and has an internal pull-down and is 
    // connected to SWD_DATA that has an external pull-up
    //Pad 9 - remove default pull-down configuration, ensure pin is configured as input
    .ucPin = PAD_9,
    .ucFunc = PAD9_FUNC_SEL_GPIO_1,
    .ucMode = PAD_MODE_INPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS
  },

  // LEDS
   { // setup blue LED
    .ucPin = PAD_18,
    .ucFunc = PAD18_FUNC_SEL_GPIO_4,
    .ucCtrl = PAD_CTRL_SRC_A0,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGTH_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  { // setup green LED
    .ucPin = PAD_21,
    .ucFunc = PAD21_FUNC_SEL_GPIO_5,
    .ucCtrl = PAD_CTRL_SRC_A0,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGTH_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
#if 0 //GPIO_6 is used by  PAD_31
  { // setup red LED
    .ucPin = PAD_22,
    .ucFunc = PAD22_FUNC_SEL_GPIO_6,
    .ucCtrl = PAD_CTRL_SRC_A0,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGTH_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
#endif  
#if 0
   {
    // Pad6 is the user button
    .ucPin = PAD_6,
    .ucFunc = PAD6_FUNC_SEL_GPIO_0,
    .ucMode = PAD_MODE_INPUT_EN,
    .ucPull = PAD_PULLUP,
    .ucDrv = PAD_DRV_STRENGTH_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS
  },
#endif
  
   { // Pad 33 -- Test clock
    .ucPin = PAD_33,
    .ucFunc = PAD33_FUNC_SEL_FBIO_33,
    .ucCtrl = PAD_CTRL_SRC_FPGA,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
#if 0
  // SPI Slave
   { // Pad 16 -- SPI Slave CLK
    .ucPin = PAD_16,
    .ucFunc = PAD16_FUNC_SEL_SPIs_CLK,
    .ucCtrl = PAD_CTRL_SRC_FPGA,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  { // Pad 17 -- SPI Slave MISO
    .ucPin = PAD_17,
    .ucFunc = PAD17_FUNC_SEL_SPIs_MISO,
    .ucCtrl = PAD_CTRL_SRC_FPGA,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  { // Pad 19 -- SPI Slave MOSI
    .ucPin = PAD_19,
    .ucFunc = PAD19_FUNC_SEL_SPIs_MOSI,
    .ucCtrl = PAD_CTRL_SRC_FPGA,
    .ucMode = PAD_MODE_INPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
   { // Pad 20 -- SPI Slave SSn
    .ucPin = PAD_20,
    .ucFunc = PAD20_FUNC_SEL_SPIs_SSn,
    .ucCtrl = PAD_CTRL_SRC_FPGA,
    .ucMode = PAD_MODE_INPUT_EN,
    .ucPull = PAD_NOPULL,
  },
#endif
  //------------- USB ---------------//
   { // Pad 37 -- USB Pullup control
    .ucPin = PAD_37,
    .ucFunc = PAD37_FUNC_SEL_FBIO_37,
    .ucCtrl = PAD_CTRL_SRC_FPGA,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  { // Pad 41 -- USB D-
    .ucPin = PAD_41,
    .ucFunc = PAD41_FUNC_SEL_FBIO_41,
    .ucCtrl = PAD_CTRL_SRC_FPGA,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  { // Pad 41 -- USB D+
    .ucPin = PAD_42,
    .ucFunc = PAD42_FUNC_SEL_FBIO_42,
    .ucCtrl = PAD_CTRL_SRC_FPGA,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },

#if (AUDIO_DRIVER == 1) && (PDM_PAD_28_29 == 1)
  { // Pad 28 -- PDM DATA
    .ucPin = PAD_28,
    .ucFunc = PAD28_FUNC_SEL_PDM_DIN,
    .ucCtrl = PAD_CTRL_SRC_A0,
    .ucMode = PAD_MODE_INPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  { // Pad 29 -- PDM CLK
    .ucPin = PAD_29,
    .ucFunc = PAD29_FUNC_SEL_PDM_CKO,
    .ucCtrl = PAD_CTRL_SRC_A0,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
#endif
#if  (AUDIO_DRIVER == 1) && (PDM_PAD_8_10 == 1)
  {
    /* Configuring Pad_8 as PDM Clk out*/
    .ucPin = PAD_8;
    .ucFunc = PAD8_FUNC_PDM_CKO;
    .ucCtrl = PAD_CTRL_SRC_A0;
    .ucMode = PAD_MODE_OUTPUT_EN;
    .ucPull = PAD_NOPULL;
    .ucDrv = PAD_DRV_STRENGHT_4MA;
    .ucSpeed = PAD_SLEW_RATE_SLOW;
    .ucSmtTrg = PAD_SMT_TRIG_DIS;
  },
  {
    /* Configuring PAD_10 as PDM Data In*/
    .ucPin = PAD_10;
    .ucFunc = PAD10_FUNC_SEL_PDM_DIN;
    .ucCtrl = PAD_CTRL_SRC_A0;
    .ucMode = PAD_MODE_INPUT_EN;
    .ucPull = PAD_NOPULL;
    .ucDrv = PAD_DRV_STRENGHT_4MA;
    .ucSpeed = PAD_SLEW_RATE_SLOW;
    .ucSmtTrg = PAD_SMT_TRIG_DIS;
  },
#endif
#if (ENABLE_I2S_TX_SLAVE == 1)
  { 
    /* WD_CLK left/right sync clock */
    .ucPin = PAD_23,
    .ucFunc = PAD23_FUNC_SEL_AP_I2S_WD_CLK_IN,
    .ucCtrl = PAD_CTRL_SRC_A0,
    .ucMode = PAD_MODE_INPUT_EN,
    .ucPull = PAD_PULLUP,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  {
    /* I2S DOUT for Tx */
    .ucPin = PAD_24,
    .ucFunc = PAD24_FUNC_SEL_AP_I2S_DOUT,
    .ucCtrl = PAD_CTRL_SRC_A0,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_PULLUP,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  {
    /* I2S CLK IN from master */
    .ucPin = PAD_31,
    .ucFunc = PAD31_FUNC_SEL_AP_I2S_CLK_IN,
    .ucCtrl = PAD_CTRL_SRC_A0,
    .ucMode = PAD_MODE_INPUT_EN,
    .ucPull = PAD_PULLUP,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
#endif
#if (FEATURE_D2HPROTOCOL_DEVICE == 1)  
  {
    /* D2H Ack  */
    .ucPin = PAD_11,
    .ucFunc = PAD11_FUNC_SEL_GPIO_2,
    .ucCtrl = PAD_CTRL_SRC_A0,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_PULLUP,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS
  }, 
  /* D2H Interrupt is always using PAD_43.
     It needs set up differently. (?)
     So, not setup here.   
  */
#endif
  
#if (VOICE_AP_BYPASS_MODE == 1)
  {
    //S3B_AP_PDM_CKO_IN
    .ucPin = PAD_38;
    .ucFunc = PAD38_FUNC_SEL_AP_PDM_CKO_IN;
    .ucCtrl = PAD_CTRL_SRC_A0;
    .ucMode = PAD_MODE_INPUT_EN;
    .ucPull = PAD_NOPULL;
    .ucDrv = PAD_DRV_STRENGHT_4MA;
    .ucSpeed = PAD_SLEW_RATE_SLOW;
    .ucSmtTrg = PAD_SMT_TRIG_DIS;
  },
  {
    .ucPin = PAD_39;
    .ucFunc = PAD39_FUNC_SEL_AP_PDM_IO;
    .ucCtrl = PAD_CTRL_SRC_A0;
    .ucMode = PAD_MODE_OUTPUT_EN;
    .ucPull = PAD_NOPULL;
    .ucDrv = PAD_DRV_STRENGHT_4MA;
    .ucSpeed = PAD_SLEW_RATE_SLOW;
    .ucSmtTrg = PAD_SMT_TRIG_DIS;
  },
#endif
#if (FEATURE_FLL_I2S_DEVICE == 1)

  { // Pad 4 -- I2S master-in clk
    .ucPin = PAD_4,
    .ucFunc = PAD4_FUNC_SEL_FBIO_4,
    .ucCtrl = PAD_CTRL_SRC_FPGA,
    .ucMode = PAD_MODE_INPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  { // Pad 2 -- FLL Speedup interrupt
    .ucPin = PAD_2,
    .ucFunc = PAD2_FUNC_SEL_FBIO_2,
    .ucCtrl = PAD_CTRL_SRC_FPGA,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  { // Pad 27 -- FLL Slowdown interrupt
    .ucPin = PAD_27,
    .ucFunc = PAD27_FUNC_SEL_FBIO_27,
    .ucCtrl = PAD_CTRL_SRC_FPGA,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  { // Pad 8 -- Debug FLL out bitclk Master
    .ucPin = PAD_8,
    .ucFunc = PAD8_FUNC_SEL_FBIO_8,
    .ucCtrl = PAD_CTRL_SRC_FPGA,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  { // Pad 3 -- Debug FLL out bitclk Slave
    .ucPin = PAD_3,
    .ucFunc = PAD3_FUNC_SEL_FBIO_3,
    .ucCtrl = PAD_CTRL_SRC_FPGA,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  
  
#endif  
};

GPIOCfgTypeDef  gpiocfg_table[] =
{
#if (FEATURE_D2HPROTOCOL_DEVICE == 1)  
  {  //for H2D interrupt 
     .usPadNum = PAD_12,
     .ucGpioNum = GPIO_6,
     .ucFunc = PAD12_FUNC_SEL_SENS_INT_6,
     .intr_type = EDGE_TRIGGERED,
     .pol_type = RISE_HIGH,  // Active HIGH (1 for level triggered is active high)
     .ucPull = PAD_NOPULL
  },
  {  //for H2D ack 
     .usPadNum = PAD_13,
     .ucGpioNum = GPIO_7,
     .ucFunc = PAD13_FUNC_SEL_SENS_INT_7,
     .intr_type = EDGE_TRIGGERED,
     .pol_type = RISE_HIGH,  // Active HIGH (1 for level triggered is active high)
     .ucPull = PAD_NOPULL
  },
#endif

};

int sizeof_pincfg_table = sizeof(pincfg_table)/sizeof(pincfg_table[0]);
int sizeof_gpiocfg_table = sizeof(gpiocfg_table)/sizeof(gpiocfg_table[0]);
