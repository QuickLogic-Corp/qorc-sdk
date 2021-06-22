/*==========================================================
 * Copyright 2021 QuickLogic Corporation
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
 *    File   : eoss3_hal_fpga_i2s_slave.h
 *    Purpose: This file contains API declaration for I2S Slave 
 *             Rx implemented in fpga.
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"


#ifndef __EOSS3_HAL_FB_I2S_SLAVE_H_
#define __EOSS3_HAL_FB_I2S_SLAVE_H_

#include "eoss3_dev.h"

#define FB_I2S_SDMA_BASE    (FPGA_PERIPH_BASE + 0x2000)
#define FB_I2S_REG_OFFSET   (FPGA_PERIPH_BASE + 0x2000 + 0x1000)
#define FB_I2S_SDMA         ((I2S_SDMA_TypeDef *) FB_I2S_REG_OFFSET)

typedef struct
{
  __IO uint32_t I2S_IER;            /*0x00 I2S Enable Register */
  __IO uint32_t I2S_ACSLIPR;        /*0x04 ACSLIP Reset (deprecated)*/
  __IO uint32_t I2S_ISR;            /*0x08 Interrupt Status Register */
  __IO uint32_t I2S_IEN;            /*0x0C Interrupt Enable Register */
  __IO uint32_t I2S_DFSTS;          /*0x10 Decimation FIFO Status Register */
  __IO uint32_t I2S_DFDREG;         /*0x14 Decimation Data Register */
#if 0
  __IO uint32_t I2S_ACSLIP;         /*0x18 ACSLIP Register (deprecated) */
  __IO uint32_t I2S_DFRST;          /*0x1C Decimation FIFO Reset Register */
  __IO uint32_t I2S_DER;            /*0x20 DMA Enable Register */
  __IO uint32_t I2S_DSR;            /*0x24 DMA Status Register */
  __IO uint32_t I2S_DCNT;           /*0x28 DMA Count Register */
  __IO uint32_t I2S_ACSLTMR;        /*0x2C ACSLIP Timer (deprecated) */
  __IO uint32_t I2S_FDCR;           /*0x30 FIR Decimation Control Register */
  __IO uint32_t I2S_FDSR;           /*0x34 FIR Decimation Status Register */
#endif
} I2S_SDMA_TypeDef;


extern void eoss3_hal_fabric_i2s_slave_clks_disable();
extern void eoss3_hal_fabric_i2s_slave_clks_enable();

extern void HAL_FB_I2SRx_Ref_input_DmaStart(void);
extern void HAL_FB_I2SRx_Ref_input_DmaNext(void);


#endif //__EOSS3_HAL_FB_I2S_SLAVE_H_