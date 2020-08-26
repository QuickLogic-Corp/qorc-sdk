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

/*! \file eoss3_hal_fpga_adc_reg.h
 *
 *  \brief This file contains Register definitions of FPGA ADC interface IP.
 */
#ifndef HAL_EOSS3_HAL_FPGA_ADC_REG_H_
#define HAL_EOSS3_HAL_FPGA_ADC_REG_H_

#define FPGA_ADC_BASE   (FPGA_PERIPH_BASE)

/* FPGA AGC register definitions. */
typedef struct
{
  __IO uint32_t CHIP_ID;        /*0x0  */    //IP DEVICE ID = {0x55ADC}
  __IO uint32_t REV_NO;         /*0x4  */    //IP Revision number value = 0x0100 [version 1.0]
  __IO uint32_t FIFO_RESET;     /*0x8  */    //RX FIFO FLUSH HW auto clear
  __IO uint32_t SEN_ENR;        /*0xC  */    //Sensor Enable Register. Bit Mask for Sensor Channels
} FPGA_ADC_AD7476_TypeDef;

#define FPGA_ADC         ((FPGA_ADC_AD7476_TypeDef *) FPGA_ADC_BASE)


#define FPGA_CHIP_ID_AD7476     (0xADC0001)
#define FPGA_CHIP_ID_AD7476_REV  (0x0100)

#define FOUR_MEGA_HZ  (4096000)

#define ADC_FIFO_RESET_BIT       (0x1)

#define CHANNEL0_ENABLE_MASK     (0x1 << 0)

#endif //HAL_EOSS3_HAL_FPGA_ADC_REG_H_
