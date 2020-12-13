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

#ifndef __EOSS3_HAL_GPIO_H
#define __EOSS3_HAL_GPIO_H

/*!	\file eoss3_hal_gpio.h
 *
 *  \brief This file contains macros, structures and APIs to
 *         mainpulate the GPIOs in the EOS S3.
 */

//#include "test_types.h"
#include <eoss3_dev.h>

#include "eoss3_hal_def.h"

#ifdef __cplusplus
 extern "C" {
#endif

/*! \def GPIO_0
    \brief GPIO index 0.
*/
#define GPIO_0	0
   
/*! \def GPIO_1
    \brief GPIO index 1.
*/   
#define GPIO_1	1
   
/*! \def GPIO_2
    \brief GPIO index 3.
*/   
#define GPIO_2	2
   
/*! \def GPIO_3
    \brief GPIO index 3.
*/   
#define GPIO_3	3
   
/*! \def GPIO_4
    \brief GPIO index 4.
*/   
#define GPIO_4	4
   
/*! \def GPIO_5
    \brief GPIO index 5.
*/   
#define GPIO_5	5
   
/*! \def GPIO_6
    \brief GPIO index 6.
*/   
#define GPIO_6	6
   
/*! \def GPIO_7
    \brief GPIO index 7.
*/   
#define GPIO_7	7

#define PAD_COUNT	46

#define S_INTR_SEL_BASE		((volatile uint32_t *)0x40004D3C)

typedef enum
{
	LEVEL_TRIGGERED = 0,
	EDGE_TRIGGERED = 1
}GPIO_INTR_TYPE;

typedef enum
{
	FALL_LOW = 0,
	RISE_HIGH = 1
}GPIO_POL_TYPE;

/*!
 * \brief GPIO Configuration Structure definition
 */
typedef struct __GPIOCfgTypeDef
{
    uint8_t   ucGpioNum;                    /*! GPIO Num : 0 -7*/
  	uint16_t  usPadNum;                     /*! PAD Num/Pin num corresponding to GPIO Num : 0-45 (used for PAD config)*/
    uint32_t  ucFunc;                       /*! Pad function selection (used for PAD config)*/
	uint8_t ucPull;                         /*! Pad Pull up config (used for PAD config)*/
	GPIO_INTR_TYPE	intr_type;              /*! GPIO Interrupt type*/
	GPIO_POL_TYPE	pol_type;	            /*! GPIO Pol type*/
	
}GPIOCfgTypeDef;

/*! \fn HAL_GPIO_Read(uint8_t ucGpioIndex, uint8_t *ucGpioVal)
    \brief GPIO read function to read pad status. Before using this function 
      given pad needs to be initalized.
    \param ucGpioIndex - GPIO index that needs to be read.
    \param *ucGpioVal - GPIO status read, return value pointer.
    \return None
*/   
void HAL_GPIO_Read(uint8_t ucGpioIndex, uint8_t *ucGpioVal);


 /*! \fn HAL_GPIO_Write(uint8_t ucGpioIndex, uint8_t ucGpioVal)
    \brief GPIO write function to write to pad . Before using this function 
      given pad needs to be initalized.
    \param ucGpioIndex - GPIO index that needs to be written.
    \param ucGpioVal - GPIO status that needs to be written.
    \return None
*/
void HAL_GPIO_Write(uint8_t ucGpioIndex, uint8_t ucGpioVal);

 /*! \fn HAL_StatusTypeDef  HAL_GPIO_IntrCfg(GPIOCfgTypeDef *hGpioCfg)
    \brief Function to write to configure pad as GPIO interrupt. PAD initialization
    will be done based on the parameter passed.
    GPIO Interrupt Usage :

    EDGE TRIGGERED
    Set the interrupt type as Edge Triggered and polarity as Rising/Falling Edge.
    At the Rising/Falling Edge, interrupt will be generated which is reflected in INTR_CTRL->GPIO_INTR register (0x40004800)
    This interrupt can be cleared by writing 1 to the corresponding bit in INTR_CTRL->GPIO_INTR register.

    LEVEL TRIGGERED
    Set the interrupt type as Level Triggered and polarity as Active High/Low.
    Depending on the polarity , interrupt will be generated. The value of the GPIO is reflected in GPIO_INTR_RAW (0x40004804) regardless of the polarity.
    Check the corresponding bit of the GPIO_INTR_RAW for the desired value and proceed with the ISR.
    This interupt needs to be cleared from Source of the interrupt.
    For example, if the GPIO_0 is configured as LEVEL TRIGGERED Active Low interrupt, then the interrupt can be cleared by writng 1 (pulled HIGH) to GPIO_0 by Source (AP)
    (M4 can disable the interrupt and convey the AP to clear the interrupt(by setting the line as High) and re-enable the interrupt once AP clears the interrupt) (* depending on the usecase).
    Note : In case of level triggered, interrupt will NOT be reflected in INTR_CTRL->GPIO_INTR register (0x40004800).
    
    \param hGpioCfg - pointer to GPIO configuration structure.
    \return HAL_StatusTypeDef   status of GPIO interrupt configuration operation
*/
HAL_StatusTypeDef  HAL_GPIO_IntrCfg(GPIOCfgTypeDef *hGpioCfg);

/** Configures PADs as interrupts using given configurations in `p_table`
 *  Also, initializes PADs using the configurations provided in `p_table`
 * 
 * @param p_table table of GPIOCfgTypeDef structures
 * @param nitems  number of GPIOCfgTypeDef structures to configure
 * 
 * @return None 
 */
void configure_s3_gpio_interrupts(GPIOCfgTypeDef *p_table, int nitems);

#ifdef __cplusplus
}
#endif

#endif /* __EOSS3_HAL_PADS_H */
