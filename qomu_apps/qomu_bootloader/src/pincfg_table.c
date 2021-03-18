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
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
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

  //------------- LEDs ---------------//
  { // Pad 6 -- green LED
    .ucPin = PAD_6,
    .ucFunc = PAD6_FUNC_SEL_GPIO_0,
    .ucCtrl = PAD_CTRL_SRC_A0,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  { // Pad 30 -- blue LED
    .ucPin = PAD_30,
    .ucFunc = PAD30_FUNC_SEL_GPIO_3,
    .ucCtrl = PAD_CTRL_SRC_A0,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  
  
  // SPI Master
   { // Pad 34 -- SPI Master CLK
    .ucPin = PAD_34,
    .ucFunc = PAD34_FUNC_SEL_SPIm_CLK,
    .ucCtrl = PAD_CTRL_SRC_A0,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  { // Pad 38 -- SPI Master MOSI
    .ucPin = PAD_38,
    .ucFunc = PAD38_FUNC_SEL_SPIm_MOSI,
    .ucCtrl = PAD_CTRL_SRC_A0,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  { // Pad 39 -- SPI Master SSn1
    .ucPin = PAD_39,
    .ucFunc = PAD39_FUNC_SEL_SPIm_SSn1,
    .ucCtrl = PAD_CTRL_SRC_A0,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
   { // Pad 36 -- SPI Master MISO
    .ucPin = PAD_36,
    .ucFunc = PAD36_FUNC_SEL_SPIm_MISO,
    .ucCtrl = PAD_CTRL_SRC_A0,
    .ucMode = PAD_MODE_INPUT_EN,
    .ucPull = PAD_NOPULL, //PAD_PULLUP,//PAD_NOPULL,
  },
#if (FEATURE_USBSERIAL == 1)
  //------------- USB ---------------//
   { // Pad 23 -- USB Pullup control
    .ucPin = PAD_23,
    .ucFunc = PAD23_FUNC_SEL_FBIO_23,
    .ucCtrl = PAD_CTRL_SRC_FPGA,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  { // Pad 28 -- USB D-
    .ucPin = PAD_28,
    .ucFunc = PAD28_FUNC_SEL_FBIO_28,
    .ucCtrl = PAD_CTRL_SRC_FPGA,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGHT_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  },
  { // Pad 31 -- USB D+
    .ucPin = PAD_31,
    .ucFunc = PAD31_FUNC_SEL_FBIO_31,
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
  { //Pad 10 -- ToucPad (sensor interrupt 4)
    .usPadNum = PAD_10,
	.ucGpioNum = GPIO_4,
	.ucFunc = PAD10_FUNC_SEL_SENS_INT_4,
    .intr_type = EDGE_TRIGGERED,
    .pol_type = RISE_HIGH,
	.ucPull = PAD_NOPULL,
  },

};

int sizeof_pincfg_table = sizeof(pincfg_table)/sizeof(pincfg_table[0]);
int sizeof_gpiocfg_table = sizeof(gpiocfg_table)/sizeof(gpiocfg_table[0]);
