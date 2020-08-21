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
  __IO uint32_t SEN1_SETTING;   /*0x10 */    //Sensor1 settings.
  __IO uint32_t SEN2_SETTING;   /*0x14 */    //Sensor2 settings.
  __IO uint32_t SEN3_SETTING;   /*0x18 */    //Sensor3 settings.
  __IO uint32_t SEN4_SETTING;   /*0x1C */    //Sensor4 settings.
  /* Timer works on C21 Clock (C21 = 2 MHz, SW setting, 0.5usec pulse) Load 0x100 for 16kHz, 0x29 for 100kHz*/
  __IO uint32_t TIMER_COUNT;    /*0x20 */    //Time count for highest sampling rate.
  __IO uint32_t TIMER_ENABLE;   /*0x24 */    //Timer Enable
} FPGA_ADC_TypeDef;

#define FPGA_ADC         ((FPGA_ADC_TypeDef *) FPGA_ADC_BASE)


#define FPGA_CHIP_ID_LT1859 (0x55ADC)
#define FPGA_CHIP_ID_LTC1859_REV 0x0100
#define SENSOR_SAMPLE_RATE_100KHZ  (100000)
#define SENSOR_SAMPLE_RATE_16KHZ   (16000)

#define SENSOR_SAMPLE_RATE_100KHZ_DIV_VALUE  (0x29)
#define SENSOR_SAMPLE_RATE_16KHZ_DIV_VALUE  (0x100)

#define FOUR_MEGA_HZ  (4096000)

#define ADC_FIFO_RESET_BIT       (0x1)
#define ADC_TIMER_ENABLE_BIT     (0x1)

#define CHANNEL0_ENABLE_MASK     (0x1 << 0)
#define CHANNEL1_ENABLE_MASK     (0x1 << 1)
#define CHANNEL2_ENABLE_MASK     (0x1 << 2)
#define CHANNEL3_ENABLE_MASK     (0x1 << 3)

#endif //HAL_EOSS3_HAL_FPGA_ADC_REG_H_
