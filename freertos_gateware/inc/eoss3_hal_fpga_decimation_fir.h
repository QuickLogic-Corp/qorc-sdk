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
 *    File   : eoss3_hal_fpga_decimation_fir.h
 *    Purpose: This file contains FIR functions for Rx data in fpga.
  *                                                          
 *=========================================================*/
#include "Fw_global_config.h"


#ifndef __EOSS3_HAL_FB_DECIMATION_FIR_H_
#define __EOSS3_HAL_FB_DECIMATION_FIR_H_

#include "eoss3_dev.h"

#define FB_DECIMATION_FIR_REG_OFFSET (FPGA_PERIPH_BASE + 0x4000  + 0x1000)
#define FB_DECIMATION_FIR_SDMA_BASE  (FPGA_PERIPH_BASE + 0x4000)
#define FB_FIR_SDMA      ((FIR_SDMA_TypeDef *) FB_DECIMATION_FIR_SDMA_BASE)

typedef struct
{
  __IO uint32_t FIR_IER;            /*0x00 I2S Enable Register */
  __IO uint32_t FIR_ACSLIPR;        /*0x04 ACSLIP Reset (deprecated)*/
  __IO uint32_t FIR_ISR;            /*0x08 Interrupt Status Register */
  __IO uint32_t FIR_IEN;            /*0x0C Interrupt Enable Register */
  __IO uint32_t FIR_DFSTS;          /*0x10 Decimation FIFO Status Register */
  __IO uint32_t FIR_DFDREG;         /*0x14 Decimation Data Register */
  __IO uint32_t FIR_ACSLIP;         /*0x18 ACSLIP Register (deprecated) */
  __IO uint32_t FIR_DFRST;          /*0x1C Decimation FIFO Reset Register */
  __IO uint32_t FIR_DER;            /*0x20 DMA Enable Register */
  __IO uint32_t FIR_DSR;            /*0x24 DMA Status Register */
  __IO uint32_t FIR_DCNT;           /*0x28 DMA Count Register */
  __IO uint32_t FIR_ACSLTMR;        /*0x2C ACSLIP Timer (deprecated) */
  __IO uint32_t FIR_FDCR;           /*0x30 FIR Decimation Control Register */
  __IO uint32_t FIR_FDSR;           /*0x34 FIR Decimation Status Register */
  
  __IO uint32_t FIR_RSRVD1;         /*0x38 Reserved 1 */
  __IO uint32_t FIR_RSRVD2;         /*0x3c Reserved 2 */
  __IO uint32_t FIR_FIFO;           /*0x40 FIR FIFO Status Register */

} FIR_SDMA_TypeDef;

extern void HAL_FIR_Decimation_FB_SDMA_Config(void);

extern uint8_t is_fir_fb_slave_dma_done_intr_set(void);
extern void fir_fb_slave_rx_dma_done_clear(void);
extern void fir_fb_slave_rx_dma_en(void);
extern void HAL_FIR_Decimation_Stop (void);

#endif //__EOSS3_HAL_FB_DECIMATION_FIR_H_