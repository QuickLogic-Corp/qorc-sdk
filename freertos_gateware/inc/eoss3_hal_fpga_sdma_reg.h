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

/*! \file eoss3_hal_fpga_sdma_reg.h
 *
 *  \brief This file contains Register definitions of FPGA SDMA interface IP.
 */
#ifndef HAL_EOSS3_HAL_FPGA_SDMA_REG_H_
#define HAL_EOSS3_HAL_FPGA_SDMA_REG_H_

#define FB_FSDMA_BASE   (FPGA_PERIPH_BASE + 0x10000)
#define FB_SDMA         ((FSDMA_TypeDef *) FB_FSDMA_BASE)

#define FSDMA_CH0_DATA_REGISTER  (FB_FSDMA_BASE + 0x1000)
#define FSDMA_CH1_DATA_REGISTER  (FB_FSDMA_BASE + 0x2000)
#define FSDMA_CH2_DATA_REGISTER  (FB_FSDMA_BASE + 0x3000)
#define FSDMA_CH3_DATA_REGISTER  (FB_FSDMA_BASE + 0x4000)

/* FPGA SDMA register definitions. */
typedef struct
{
  __IO uint32_t FSDMA_ENR;     /*0x0 */    //DMA Enable Register
  __IO uint32_t FSDMA_SR;      /*0x4 */    // DMA status Register
  __IO uint32_t FSDMA_INTEN;   /*0x8 */    //DMA Interrupt Enable Register

} FSDMA_TypeDef;

#define FSDMA_CHANNEL_EN(ch)       (0x1 << ch)   //DMA Channel Enable
#define FSDMA_CHANNEL_INTR_CLR(ch) (0x1 << ch)   //DMA Interrupt Status [Read - Status, Write - clearing interrupt]
#define FSDMA_CHANNEL_INTR_EN(ch)  (0x1 << ch)   //DMA Interrupt Enable at IP level [NVIC level enabling is seperate from this]
#define FSDMA_CHANNEL_POS(ch)      (ch)          // Get Channel Position value

#endif //HAL_EOSS3_HAL_FPGA_SDMA_REG_H_
