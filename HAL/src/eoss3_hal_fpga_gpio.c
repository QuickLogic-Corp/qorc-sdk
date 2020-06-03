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
 *    File   : eoss3_hal_fpga_gpio.c
 *    Purpose: This file contains macros, structures and APIs to
 *             read/write FPGA GPIO
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#include <stdint.h>
#include "eoss3_hal_gpio.h"
#include "eoss3_hal_fpga_gpio.h"
#include "eoss3_hal_def.h"


#if 0
HAL_StatusTypeDef HAL_FB_GPIO_Read(uint8_t ucGpioIndex, uint8_t *ucGpioVal)
{
	if(ucGpioIndex <= FB_GPIO_21)
	{
		*ucGpioVal = FB_GPIO->GPIO_INPUT >> ucGpioIndex;
		return HAL_OK;
	}
	else
	  return HAL_ERROR;
}
#endif

HAL_StatusTypeDef HAL_FB_GPIO_Read(uint8_t ucGpioIndex, uint8_t *ucGpioVal)
{
	if(ucGpioIndex <= FB_GPIO_21)
	{ 
		*ucGpioVal = (FB_GPIO->GPIO_INPUT >> ucGpioIndex) & 0x1; 
		return HAL_OK; 
	}
	else
		return HAL_ERROR;
}


HAL_StatusTypeDef HAL_FB_GPIO_Write(uint8_t ucGpioIndex, uint8_t ucGpioVal)
{
	if((ucGpioIndex <= FB_GPIO_21) && (ucGpioVal <= 1) )
	{
		FB_GPIO->GPIO_DIR_CTRL |= (1 << ucGpioIndex);
		
	  	if(ucGpioVal)
		{
			FB_GPIO->GPIO_OUTPUT |=  (1 << ucGpioIndex);
		}
		else
		{
			FB_GPIO->GPIO_OUTPUT &= ~(1 << ucGpioIndex);
		}
		
		return HAL_OK;
	}
	else
	  return HAL_ERROR;
}
