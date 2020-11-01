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
 *   File   : device_bootstrap.c
 *   Purpose: Setup device bootstrap
 *
 *=========================================================*/

#include "Fw_global_config.h"
#include "eoss3_dev.h"
#include "eoss3_hal_pad_config.h"
#include "eoss3_hal_gpio.h"

/* This function is ised to configure and
set pads which are connected to GPIO19 GPIO 20 of device
QL_MOSI/PAD38 of host is connected to GPIO 19 of device
This needs to to set to 0
QL_CS/ PAD23 of host is connected to GPIO 20 of device
This needs to be set to 1 for loading device firmaware from host
IMP : After firmware load, these pads should be reconfigured to be used for spi transaction
Do this by calling spi_master_pad_setup() function
*/
void config_set_pad_for_device_bootstrap(void)
{
    PadConfig xPadConf;

    xPadConf.ucPin = PAD_38;
    xPadConf.ucFunc = PAD38_FUNC_SEL_GPIO_6;
    xPadConf.ucCtrl = PAD_CTRL_SRC_A0;
    xPadConf.ucMode = PAD_MODE_OUTPUT_EN;
    xPadConf.ucPull = PAD_PULLDOWN;
    xPadConf.ucDrv = PAD_DRV_STRENGHT_4MA;
    xPadConf.ucSpeed = PAD_SLEW_RATE_SLOW;
    xPadConf.ucSmtTrg = PAD_SMT_TRIG_DIS;

    HAL_PAD_Reconfig(&xPadConf);

    HAL_GPIO_Write(GPIO_6, 0);

    xPadConf.ucPin = PAD_23;
    xPadConf.ucFunc = PAD23_FUNC_SEL_GPIO_7;
    xPadConf.ucCtrl = PAD_CTRL_SRC_A0;
    xPadConf.ucMode = PAD_MODE_OUTPUT_EN;
    xPadConf.ucPull = PAD_PULLUP;
    xPadConf.ucDrv = PAD_DRV_STRENGHT_4MA;
    xPadConf.ucSpeed = PAD_SLEW_RATE_FAST;
    xPadConf.ucSmtTrg = PAD_SMT_TRIG_DIS;

    HAL_PAD_Reconfig(&xPadConf);

    HAL_GPIO_Write(GPIO_7, 1);

    return;
}

void spi_master_pad_setup()
{
    // Configure rest of the pads
    PadConfig padcfg;

    //output pins
    padcfg.ucCtrl = PAD_CTRL_SRC_A0;
    padcfg.ucMode = PAD_MODE_OUTPUT_EN;
    padcfg.ucPull = PAD_NOPULL;
    padcfg.ucDrv = PAD_DRV_STRENGHT_4MA;
    padcfg.ucSpeed = PAD_SLEW_RATE_FAST;
    padcfg.ucSmtTrg = PAD_SMT_TRIG_DIS;

    //Pad 23 -- SPI Master SSN2
    padcfg.ucPin = PAD_23;
    padcfg.ucFunc = PAD23_FUNC_SEL_SPIm_SSn2;
    HAL_PAD_Reconfig(&padcfg);

    // Pad 34 -- SPI Master CLK
    padcfg.ucPin = PAD_34;
    padcfg.ucFunc = PAD34_FUNC_SEL_SPIm_CLK;
    HAL_PAD_Reconfig(&padcfg);

    //Pad 35 -- SPI Master SSN3
    padcfg.ucPin = PAD_35;
    padcfg.ucFunc = PAD35_FUNC_SEL_SPIm_SSn3;
    HAL_PAD_Reconfig(&padcfg);

    //Pad 38 -- SPI Master MOSI
    padcfg.ucPin = PAD_38;
    padcfg.ucFunc = PAD38_FUNC_SEL_SPIm_MOSI;
    HAL_PAD_Reconfig(&padcfg);

    //Pad 39 -- SPI Master SSn1
    padcfg.ucPin = PAD_39;
    padcfg.ucFunc = PAD39_FUNC_SEL_SPIm_SSn1;
    HAL_PAD_Reconfig(&padcfg);

    //input pins
    padcfg.ucMode = PAD_MODE_INPUT_EN;
    padcfg.ucPull = PAD_PULLUP;

    //Pad 36 -- SPI Master MISO
    padcfg.ucPin = PAD_36;
    padcfg.ucFunc = PAD36_FUNC_SEL_SPIm_MISO;
    HAL_PAD_Reconfig(&padcfg);

    return;
}
