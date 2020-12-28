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

/*!	\file eoss3_hal_gpio.h
 *
 *  \brief This file contains API implementation to
 *         mainpulate the GPIOs in the EOS S3.
 */

#include "Fw_global_config.h"

#include <stdint.h>
#include "eoss3_hal_gpio.h"
#include "eoss3_hal_pad_config.h"
#include "eoss3_hal_def.h"
#include "FreeRTOSConfig.h"

/*!
 * \brief array containing reg offset (wrt S_INTR_SEL_BASE) and correponding register value to be selected for each PAD
    Bit 7:4 - register offset from S_INTR_SEL_BASE (0x40004D3C)
    Bit 3:0 - register value to be written for selecting the particular PAD
*/
const uint8_t gpio_intr_cfg[PAD_COUNT] = {0x0,0x0,0x11,0x01,0x21,0x31,
					   0x12,0x41,0x22,0x32,0x42,0x51,
					   0x61,0x71,0x52,0x62,0x0,0x0,
					   0x13,0x0,0x0,0x23,0x32,0x72,
					   0x14,0x24,0x43,0x53,0x34,0x44,
					   0x54,0x63,0x64,0x73,0x74,0x15,
					   0x16,0x25,0x26,0x35,0x36,0x65,
					   0x75,0x0,0x45,0x55
  
};

void HAL_GPIO_Read(uint8_t ucGpioIndex, uint8_t *ucGpioVal)
{
	if(ucGpioIndex <= GPIO_7)
	{
		*ucGpioVal = ((MISC_CTRL->IO_INPUT >> ucGpioIndex) & 1);
	}
}

void HAL_GPIO_Write(uint8_t ucGpioIndex, uint8_t ucGpioVal)
{
	if((ucGpioIndex <= GPIO_7) && (ucGpioVal <= 1) )
	{
		if(ucGpioVal)
		{
			MISC_CTRL->IO_OUTPUT |=  (1 << ucGpioIndex);
		}
		else
		{
			MISC_CTRL->IO_OUTPUT &= ~(1 << ucGpioIndex);
		}
	}
}

HAL_StatusTypeDef  HAL_GPIO_IntrCfg(GPIOCfgTypeDef *hGpioCfg)
{
	uint32_t *pRegOffset;
  	PadConfig xPadConf;
        
	//configure the PAD as GPIO
  	xPadConf.ucPin = hGpioCfg->usPadNum;
	xPadConf.ucFunc = hGpioCfg->ucFunc;
	xPadConf.ucCtrl = PAD_CTRL_SRC_A0;
	xPadConf.ucMode = PAD_MODE_INPUT_EN;
	xPadConf.ucPull = hGpioCfg->ucPull;
	xPadConf.ucDrv = PAD_DRV_STRENGHT_4MA;
	xPadConf.ucSpeed = PAD_SLEW_RATE_SLOW;
	xPadConf.ucSmtTrg = PAD_SMT_TRIG_EN;

	HAL_PAD_Config(&xPadConf);
	NVIC_DisableIRQ(Gpio_IRQn);
	pRegOffset = (uint32_t*)S_INTR_SEL_BASE;
	pRegOffset += (((gpio_intr_cfg[hGpioCfg->usPadNum] & 0xF0) >> 4));
	*pRegOffset = gpio_intr_cfg[hGpioCfg->usPadNum] & 0xF;
		
	//configure GPIO     
    INTR_CTRL->GPIO_INTR_TYPE &= (~((uint32_t) (0x1) << hGpioCfg->ucGpioNum)); //zero the bit position
	INTR_CTRL->GPIO_INTR_TYPE |= (hGpioCfg->intr_type << hGpioCfg->ucGpioNum); //update the bit with desired value
        
    INTR_CTRL->GPIO_INTR_POL &= (~((uint32_t) (0x1) << hGpioCfg->ucGpioNum)); //zero the bit position
	INTR_CTRL->GPIO_INTR_POL |= (hGpioCfg->pol_type << hGpioCfg->ucGpioNum); //update the bit with desired value
	
//	printf("gpio_intr_type = %x, gpio_intr_pol = %x\r\n",INTR_CTRL->GPIO_INTR_TYPE,INTR_CTRL->GPIO_INTR_POL);
	
    //Clear the interrupt register
    INTR_CTRL->GPIO_INTR |= (1 << hGpioCfg->ucGpioNum);
	//configure GPIO interrupt
	INTR_CTRL->GPIO_INTR_EN_M4 |= (1 << hGpioCfg->ucGpioNum);
	
	NVIC_ClearPendingIRQ(Gpio_IRQn);
	NVIC_SetPriority(Gpio_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	NVIC_EnableIRQ(Gpio_IRQn);
	
	return HAL_OK;
}

void configure_s3_gpio_interrupts(GPIOCfgTypeDef *p_table, int nitems)
{
  for (int k = 0; k < nitems; k++)
  {
    HAL_GPIO_IntrCfg(&p_table[k]);
  }
}
