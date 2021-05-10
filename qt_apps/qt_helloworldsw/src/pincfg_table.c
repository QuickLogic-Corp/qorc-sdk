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
  
  {  // configure SCL0
    .ucPin = PAD_0,
    .ucFunc = PAD0_FUNC_SEL_SCL_0,
    .ucCtrl = PAD_CTRL_SRC_OTHER,
    .ucMode = PAD_MODE_INPUT_EN,
    .ucPull = PAD_PULLUP,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },

  {  // configure SDA0
    .ucPin = PAD_1,
    .ucFunc = PAD1_FUNC_SEL_SDA_0,
    .ucCtrl = PAD_CTRL_SRC_OTHER,
    .ucMode = PAD_MODE_INPUT_EN,
    .ucPull = PAD_PULLUP,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
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
  
  // SPI Master
   { // Pad 34 -- SPI Master CLK
    .ucPin = PAD_34,
    .ucFunc = PAD34_FUNC_SEL_FBIO_34,
    .ucCtrl = PAD_CTRL_SRC_FPGA,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  { // Pad 38 -- SPI Master MOSI
    .ucPin = PAD_38,
    .ucFunc = PAD38_FUNC_SEL_FBIO_38,
    .ucCtrl = PAD_CTRL_SRC_FPGA,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  { // Pad 39 -- SPI Master SSn1
    .ucPin = PAD_39,
    .ucFunc = PAD39_FUNC_SEL_FBIO_39,
    .ucCtrl = PAD_CTRL_SRC_FPGA,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
   { // Pad 36 -- SPI Master MISO
    .ucPin = PAD_36,
    .ucFunc = PAD36_FUNC_SEL_FBIO_36,
    .ucCtrl = PAD_CTRL_SRC_FPGA,
    .ucMode = PAD_MODE_INPUT_EN,
    .ucPull = PAD_NOPULL,
  },
  
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

};

GPIOCfgTypeDef  gpiocfg_table[] =
{
  {
    //Example setting for PAD_41 (sensor interrupt 6)
    .usPadNum = PAD_41,
    .ucGpioNum = GPIO_6,
    .ucFunc = PAD41_FUNC_SEL_SENS_INT_6,
    .intr_type = EDGE_TRIGGERED,
    .pol_type = RISE_HIGH,
    .ucPull = PAD_PULLDOWN,
  },

};

int sizeof_pincfg_table = sizeof(pincfg_table)/sizeof(pincfg_table[0]);
int sizeof_gpiocfg_table = sizeof(gpiocfg_table)/sizeof(gpiocfg_table[0]);
