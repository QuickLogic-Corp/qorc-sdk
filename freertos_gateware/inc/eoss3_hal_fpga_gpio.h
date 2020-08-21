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

/*!	\file eoss3_hal_fpga_gpio.h
 *  Author: Rajkumar Thiagarajan <rthiagarajan@quicklogic.com>
 *
 * \brief This file contains macros, structures and APIs to 
 *        read/write fpga GPIO.
 */
 
#ifndef __EOSS3_HAL_FPGA_GPIO_H
#define __EOSS3_HAL_FPGA_GPIO_H

#include <eoss3_dev.h>
#include "eoss3_hal_def.h"

#define xCUR_BOARD

#ifdef __cplusplus
 extern "C" {
#endif

/* FPGA July 28 release for REV01
  Pin Name	              PAD Number
SIN  (UART RXD)                 PAD[45]
SOUT  (UART TXD)                PAD[44]
I2C_SCL (LCD)            	PAD[6]
I2C_SDA (LCD)          		PAD[7]
GPIO_PIN[0]              	PAD[3]
GPIO_PIN[1]              	PAD[11]
GPIO_PIN[2]              	PAD[19]
GPIO_PIN[3]              	PAD[43]
GPIO_PIN[4]              	PAD[8]
GPIO_PIN[5]              	PAD[41]
GPIO_PIN[6]              	PAD[9]
GPIO_PIN[7]              	PAD[13]
GPIO_PIN[8]              	PAD[10]
GPIO_PIN[9]              	PAD[17]
GPIO_PIN[10]              	PAD[18]
GPIO_PIN[11]              	PAD[23]
GPIO_PIN[12]              	PAD[20]
GPIO_PIN[13]              	PAD[21]
GPIO_PIN[14]              	PAD[22]
GPIO_PIN[15]              	PAD[16]
GPIO_PIN[16]              	PAD[26]
GPIO_PIN[17]              	PAD[27]
GPIO_PIN[18]              	PAD[42]
GPIO_PIN[19]              	PAD[12]
GPIO_PIN[20]              	PAD[40]
GPIO_PIN[21]              	PAD[2]
I2C_SCL_SEN (HRM)               PAD[29]
I2C_SDA_SEN (HRM)          	PAD[28]
CLK_4MHZ_IN             	PAD[30]
CLK_1MHZ_OUT            	PAD[31]

*/
                                  // REV00         //  REV01    
#define FB_GPIO_0	0             // PAD3          //  PAD[3]    
#define FB_GPIO_1	1             // PAD6          //  PAD[11]   
#define FB_GPIO_2	2             // PAD8          //  PAD[19]   
#define FB_GPIO_3	3             // PAD9          //  PAD[43]   
#define FB_GPIO_4	4             // PAD10         //  PAD[8]    
#define FB_GPIO_5	5             // PAD11         //  PAD[41]   
#define FB_GPIO_6	6             // PAD12         //  PAD[9]    
#define FB_GPIO_7	7             // PAD13         //  PAD[13]   
#define FB_GPIO_8	8             // PAD23         //  PAD[10]   
#define FB_GPIO_9	9             // PAD17         //  PAD[17]   
#define FB_GPIO_10	10            // PAD18         //  PAD[18]    
#define FB_GPIO_11	11            // PAD19         //  PAD[23]    
#define FB_GPIO_12	12            // PAD20         //  PAD[20]    
#define FB_GPIO_13	13            // PAD21         //  PAD[21]    
#define FB_GPIO_14	14            // PAD22         //  PAD[22]    
#define FB_GPIO_15	15            // PAD16         //  PAD[16]    
#define FB_GPIO_16	16            // PAD26         //  PAD[26]    
#define FB_GPIO_17	17            // PAD27         //  PAD[27]    
#define FB_GPIO_18	18            // SFBIO[10]     //  PAD[42]    
#define FB_GPIO_19	19            // SFBIO[11]     //  PAD[12]    
#define FB_GPIO_20      20        // PAD7          //  PAD[40]    
#define FB_GPIO_21      21        // PAD2          //  PAD[2]     



/*! \fn HAL_StatusTypeDef HAL_FB_GPIO_Read(uint8_t ucGpioIndex, uint8_t *ucGpioVal)
    \brief fpga GPIO read function to read pad status. Before using this function 
      given pad needs to be initalized.
    \param ucGpioIndex - GPIO index that needs to be read.
    \param *ucGpioVal - GPIO status read, return value pointer.
*/   
HAL_StatusTypeDef HAL_FB_GPIO_Read(uint8_t ucGpioIndex, uint8_t *ucGpioVal);
 
 /*! \fn HAL_StatusTypeDef HAL_FB_GPIO_Write(uint8_t ucGpioIndex, uint8_t ucGpioVal)
    \brief fpga GPIO write function to read pad status. Before using this function 
      given pad needs to be initalized.
    \param ucGpioIndex - GPIO index that needs to be read.
    \param ucGpioVal - GPIO status that nees to be written.
*/
HAL_StatusTypeDef HAL_FB_GPIO_Write(uint8_t ucGpioIndex, uint8_t ucGpioVal);



#ifdef __cplusplus
}
#endif

#endif /* __EOSS3_HAL_FPGA_GPIO_H */
