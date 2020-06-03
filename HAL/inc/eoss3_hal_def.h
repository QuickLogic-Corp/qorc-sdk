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
 *    File   : eoss3_hal_def.h
 *    Purpose: This file contains macros for HAL API status
 *                                                          
 *=========================================================*/

/*!	\file eoss3_hal_def.h
 *
 *  Created on: Feb 15, 2016
 *      Author: Rajkumar Thiagarajan
 *
 *  \brief .
 */

#ifndef HAL_INC_EOSS3_HAL_DEF_H_
#define HAL_INC_EOSS3_HAL_DEF_H_

#include <common.h>
#include <stdint.h>
#include <stddef.h>
#include "test_types.h"
/*! \enum HAL_StatusTypeDef
 * \brief HAL Status values
 */
typedef enum
{
  HAL_OK       = 0x00,
  HAL_ERROR    = 0x01,
  HAL_BUSY     = 0x02,
  HAL_TIMEOUT  = 0x03
} HAL_StatusTypeDef;

///@cond HAL_DEF_MACROS
#define BYTE_IDX_0			(0x0)
#define BYTE_IDX_1			(0x1)
#define BYTE_IDX_2			(0x2)
#define BYTE_IDX_3			(0x3)
#define BYTE_IDX_4			(0x4)
#define BYTE_IDX_5			(0x5)
#define BYTE_IDX_6			(0x6)
#define BYTE_IDX_7			(0x7)
///@endcond

typedef void (*HAL_FBISRfunction) (void);
#define FB_INTERRUPT_0                  0
#define FB_INTERRUPT_1                  1
#define FB_INTERRUPT_2                  2
#define FB_INTERRUPT_3                  3
#define MAX_FB_INTERRUPTS               4

#define FB_INTERRUPT_TYPE_LEVEL         0
#define FB_INTERRUPT_TYPE_EDGE          1

#define FB_INTERRUPT_POL_EDGE_FALL      0
#define FB_INTERRUPT_POL_EDGE_RISE      1

#define FB_INTERRUPT_POL_LEVEL_LOW      0
#define FB_INTERRUPT_POL_LEVEL_HIGH     1

#define FB_INTERRUPT_DEST_AP_ENABLE     1
#define FB_INTERRUPT_DEST_AP_DISBLE     0    

#define FB_INTERRUPT_DEST_M4_ENABLE     1
#define FB_INTERRUPT_DEST_M4_DISBLE     0 

void FB_RegisterISR(UINT32_t fbIrq, HAL_FBISRfunction ISRfn);
void FB_ConfigureInterrupt(UINT32_t fbIrq, UINT8_t type, UINT8_t polarity, UINT8_t destAP,UINT8_t destM4 );

    
#endif /* HAL_INC_EOSS3_HAL_DEF_H_ */
