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
 *    File   : eoss3_hal_pads.h
 *    Purpose: This file contains macros, structures and APIs to
 *             configure pads 
 *                                                          
 *=========================================================*/

#ifndef __EOSS3_HAL_PADS_H
#define __EOSS3_HAL_PADS_H

#include <eoss3_dev.h>
#include <stdbool.h>

#ifdef __cplusplus
 extern "C" {
#endif

/*Sensorhub3B Pads, BGA package */
#define PAD_0                 ((uint8_t) 0)  /* Pin 0 */
#define PAD_1                 ((uint8_t) 1)  /* Pin 1 */
#define PAD_2                 ((uint8_t) 2)
#define PAD_3                 ((uint8_t) 3)
#define PAD_4                 ((uint8_t) 4)
#define PAD_5                 ((uint8_t) 5)
#define PAD_6                 ((uint8_t) 6)
#define PAD_7                 ((uint8_t) 7)
#define PAD_8                 ((uint8_t) 8)
#define PAD_9                 ((uint8_t) 9)
#define PAD_10                 ((uint8_t) 10)

#define PAD_11                 ((uint8_t) 11)
#define PAD_12                 ((uint8_t) 12)
#define PAD_13                 ((uint8_t) 13)
#define PAD_14                 ((uint8_t) 14)
#define PAD_15                 ((uint8_t) 15)
#define PAD_16                 ((uint8_t) 16)
#define PAD_17                 ((uint8_t) 17)
#define PAD_18                 ((uint8_t) 18)
#define PAD_19                 ((uint8_t) 19)
#define PAD_20                 ((uint8_t) 20)

#define PAD_21                 ((uint8_t) 21)
#define PAD_22                 ((uint8_t) 22)
#define PAD_23                 ((uint8_t) 23)
#define PAD_24                 ((uint8_t) 24)
#define PAD_25                 ((uint8_t) 25)
#define PAD_26                 ((uint8_t) 26)
#define PAD_27                 ((uint8_t) 27)
#define PAD_28                 ((uint8_t) 28)
#define PAD_29                 ((uint8_t) 29)
#define PAD_30                 ((uint8_t) 30)

#define PAD_31                 ((uint8_t) 31)
#define PAD_32                 ((uint8_t) 32)
#define PAD_33                 ((uint8_t) 33)
#define PAD_34                 ((uint8_t) 34)
#define PAD_35                 ((uint8_t) 35)
#define PAD_36                 ((uint8_t) 36)
#define PAD_37                 ((uint8_t) 37)
#define PAD_38                 ((uint8_t) 38)
#define PAD_39                 ((uint8_t) 39)
#define PAD_40                 ((uint8_t) 40)

#define PAD_41                 ((uint8_t) 41)
#define PAD_42                 ((uint8_t) 42)
#define PAD_43                 ((uint8_t) 43)
#define PAD_44                 ((uint8_t) 44)
#define PAD_45                 ((uint8_t) 45)

/// @endcond
   
/// @cond EXTENDED_REGS_OFFSET
/*Extended Configuration registers offset*/
#define SDA0_SEL 	0x100
#define SDA1_SEL	0x104
#define	SDA2_SEL	0x108
#define SCL0_SEL	0x10C
#define SCL1_SEL	0x110
#define SCL2_SEL	0x114
#define	SPIs_CLK_SEL	0x118
#define SPIs_SSn_SEL	0x11C
#define SPIs_MOSI_SEL	0x120
#define SPIm_MISO_SEL	0x124
#define PDM_DATA_SEL	0x128
#define I2S_DATA_SEL	0x12C
#define UART_RXD_SEL	0x134
#define IRDA_SIRIN_SEL	0x138
#define S_INTR_0_SEL	0x13C
#define S_INTR_1_SEL	0x140
#define S_INTR_2_SEL	0x144
#define S_INTR_3_SEL	0x148
#define S_INTR_4_SEL	0x14C
#define S_INTR_5_SEL	0x150
#define S_INTR_6_SEL	0x154
#define S_INTR_7_SEL	0x158
#define nUARTCTS_SEL	0x15C
#define IO_REG_SEL		0x160
#define SW_CLK_SEL		0x170
#define SW_IO_SEL		0x174
#define FBIO_SEL_1		0x180
#define	FBIO_SEL_2		0x184
#define SPI_SENSOR_MISO_SEL	0x190
#define SPI_SENSOR_MOSI_SEL	0x194
#define I2S_WD_CLKIN_SEL	0x1A0
#define I2S_CLKIN_SEL	0x1A4
#define PDM_STAT_IN_SEL	0x1A8
#define PDM_CLKIN_SEL	0x1AC3
/*Ext registers End*/

#define EXT_REG_OFFSET_SHIFT 	20
#define EXT_REG_OFFSET_BASE	SDA0_SEL

/// @endcond
  
 /*Pads Functions, INPUT functions configuration does not need any value
  * so it has been taken as virtual function ID eg. sensor interrupt=0x8*/

/// @cond PAD_CONFIG_ALTERNATE_FUNCTIONS
#define PAD0_FUNC_SEL_SCL_0                         ((uint32_t) (0x00  | (SCL0_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD0_FUNC_SEL_FBIO_0                        ((uint32_t) (0x01  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD0_FUNC_SEL_RESERVE2                      ((uint32_t) (0x02))
#define PAD0_FUNC_SEL_RESERVE3                      ((uint32_t) (0x03))

#define PAD1_FUNC_SEL_SDA_0                         ((uint32_t) (0x00  | (SDA0_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD1_FUNC_SEL_FBIO_1                        ((uint32_t) (0x01  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))

#define PAD2_FUNC_SEL_FBIO_2                        ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD2_FUNC_SEL_SPI_SENSOR_SSn_2              ((uint32_t) (0x01))
#define PAD2_FUNC_SEL_DEBUG_MON_0					((uint32_t) (0x02))
#define PAD2_FUNC_SEL_BATT_MON						((uint32_t) (0x03))
#define PAD2_FUNC_SEL_SENS_INT_1					((uint32_t) (0x00  | (S_INTR_1_SEL << EXT_REG_OFFSET_SHIFT)))

//#define PAD3_FUNC_SEL_S_INTR_0					((uint32_t) (0x00)) /*old AP_INTR , FIXME: Input line  Intr*/
#define PAD3_FUNC_SEL_FBIO_3                        ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD3_FUNC_SEL_SENS_INT_0					((uint32_t) (0x00  | (S_INTR_0_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD4_FUNC_SEL_FBIO_4                        ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD4_FUNC_SEL_SPI_SENSOR_SSn_3              ((uint32_t) (0x01))
#define PAD4_FUNC_SEL_DEBUG_MON_1                   ((uint32_t) (0x02))
#define PAD4_FUNC_SEL_SDA_1_DPU                     ((uint32_t) (0x03))
#define PAD4_FUNC_SEL_SENS_INT_2					((uint32_t) (0x00  | (S_INTR_2_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD5_FUNC_SEL_FBIO_5                        ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD5_FUNC_SEL_SPI_SENSOR_SSn_4              ((uint32_t) (0x01))
#define PAD5_FUNC_SEL_DEBUG_MON_2                   ((uint32_t) (0x02))
#define PAD5_FUNC_SEL_SDA_0_DPU                     ((uint32_t) (0x03))
#define PAD5_FUNC_SEL_SENS_INT_3					((uint32_t) (0x00  | (S_INTR_3_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD6_FUNC_SEL_FBIO_6                        ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD6_FUNC_SPI_SENSOR_MOSI                   ((uint32_t) (0x01  | (SPI_SENSOR_MOSI_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD6_FUNC_SEL_DEBUG_MON_3                   ((uint32_t) (0x02))
#define PAD6_FUNC_SEL_GPIO_0                        ((uint32_t) (0x03  | (IO_REG_SEL << EXT_REG_OFFSET_SHIFT)))
//#define PAD6_FUNC_SEL_FCLK                        ((uint32_t) (0x0)) Fixme:Not used
#define PAD6_FUNC_SEL_IRDA_SIRIN                    ((uint32_t) (0x00  | (IRDA_SIRIN_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD6_FUNC_SEL_SENS_INT_1                    ((uint32_t) (0x00  | (S_INTR_1_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD7_FUNC_SEL_FBIO_7                        ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD7_FUNC_SPI_SENSOR_SSN5                   ((uint32_t) (0x01))
#define PAD7_FUNC_SEL_DEBUG_MON_4                   ((uint32_t) (0x02))
#define PAD7_FUNC_SEL_SWV                           ((uint32_t) (0x03))
//#define PAD7_FUNC_SEL_SENS_INT_5                    ((uint32_t) (0x00  | (S_INTR_5_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD7_FUNC_SEL_SENS_INT_4                   ((uint32_t) (0x00  | (S_INTR_4_SEL << EXT_REG_OFFSET_SHIFT)))
   
#define PAD8_FUNC_SEL_FBIO_8                       	((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD8_FUNC_PDM_CKO                        	((uint32_t) (0x01))
#define PAD8_FUNC_I2S_CKO                      		((uint32_t) (0x02))
#define PAD8_FUNC_IRDA_SIROUT                       ((uint32_t) (0x03))
#define PAD8_FUNC_SEL_SPI_SENSOR_MISO				((uint32_t) (0x00  | (SPI_SENSOR_MISO_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD8_FUNC_SEL_SENS_INT_2                    ((uint32_t) (0x00  | (S_INTR_2_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD9_FUNC_SEL_FBIO_9                        ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD9_FUNC_SEL_SPI_SENSOR_SSn_1              ((uint32_t) (0x01))
#define PAD9_FUNC_SEL_I2S_WD_CKO                    ((uint32_t) (0x02))
#define PAD9_FUNC_SEL_GPIO_1                        ((uint32_t) (0x03  | (IO_REG_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD9_FUNC_SEL_PDM_STAT_IN                   ((uint32_t) (0x00  | (PDM_STAT_IN_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD9_FUNC_SEL_SENS_INT_3                    ((uint32_t) (0x00  | (S_INTR_3_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD10_FUNC_SEL_FBIO_10                      ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD10_FUNC_SEL_SPI_SENSOR_CLK               ((uint32_t) (0x01))
#define PAD10_FUNC_SEL_RESERVE                     	((uint32_t) (0x02))
#define PAD10_FUNC_SEL_SWV                        	((uint32_t) (0x03))
#define PAD10_FUNC_SEL_I2S_DIN						((uint32_t) (0x00  | (I2S_DATA_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD10_FUNC_SEL_PDM_DIN						((uint32_t) (0x00  | (PDM_DATA_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD10_FUNC_SEL_SENS_INT_4                   ((uint32_t) (0x00  | (S_INTR_4_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD11_FUNC_SEL_FBIO_11                      ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD11_FUNC_SEL_SPI_SENSOR_SSn_6             ((uint32_t) (0x01))
#define PAD11_FUNC_SEL_DEBUG_MON_5                  ((uint32_t) (0x02))
#define PAD11_FUNC_SEL_GPIO_2                       ((uint32_t) (0x03  | (IO_REG_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD11_FUNC_SEL_SENS_INT_5                   ((uint32_t) (0x00  | (S_INTR_5_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD12_FUNC_SEL_FBIO_12                      ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD12_FUNC_SEL_SPI_SENSOR_SSn_7             ((uint32_t) (0x01))
#define PAD12_FUNC_SEL_DEBUG_MON_6                  ((uint32_t) (0x02))
#define PAD12_FUNC_SEL_IRDA_SIROUT                  ((uint32_t) (0x03))
#define PAD12_FUNC_SEL_SENS_INT_6                   ((uint32_t) (0x00  | (S_INTR_6_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD13_FUNC_SEL_FBIO_13                      ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD13_FUNC_SEL_SPI_SENSOR_SSn_8             ((uint32_t) (0x01))
#define PAD13_FUNC_SEL_DEBUG_MON_7                  ((uint32_t) (0x02))
#define PAD13_FUNC_SEL_SWV                          ((uint32_t) (0x03))
#define PAD13_FUNC_SEL_SENS_INT_7                   ((uint32_t) (0x00  | (S_INTR_7_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD14_FUNC_SEL_FBIO_14                      ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD14_FUNC_SEL_IRDA_SIROUT                  ((uint32_t) (0x01))
#define PAD14_FUNC_SEL_SCL_1                        ((uint32_t) (0x02  | (SCL1_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD14_FUNC_SEL_GPIO_3                       ((uint32_t) (0x03  | (IO_REG_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD14_FUNC_SEL_SW_DP_CLK                    ((uint32_t) (0x00  | (SW_CLK_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD14_FUNC_SEL_UART_RXD                     ((uint32_t) (0x00  | (UART_RXD_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD14_FUNC_SEL_SENS_INT_5                   ((uint32_t) (0x00  | (S_INTR_5_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD15_FUNC_SEL_SW_DP_IO                     ((uint32_t) (0x00))
#define PAD15_FUNC_SEL_FBIO_15                      ((uint32_t) (0x01  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD15_FUNC_SEL_SDA_1                        ((uint32_t) (0x02  | (SDA1_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD15_FUNC_SEL_UART_TXD                     ((uint32_t) (0x03))
#define PAD15_FUNC_SEL_DEBUG_MON_8                  ((uint32_t) (0x03))
#define PAD15_FUNC_SEL_IRDA_SIRIN                   ((uint32_t) (0x00  | (IRDA_SIRIN_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD15_FUNC_SEL_SENS_INT_6                   ((uint32_t) (0x00  | (S_INTR_6_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD16_FUNC_SEL_FBIO_16                      ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD16_FUNC_SEL_SPIs_CLK                     ((uint32_t) (0x00  | (SPIs_CLK_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD16_FUNC_SEL_UART_RXD                     ((uint32_t) (0x00  | (UART_RXD_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD17_FUNC_SEL_SPIs_MISO                    ((uint32_t) (0x00))
#define PAD17_FUNC_SEL_FBIO_17                      ((uint32_t) (0x01  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD17_FUNC_SEL_nUARTCTS                     ((uint32_t) (0x00  | (nUARTCTS_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD18_FUNC_SEL_FBIO_18                      ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD18_FUNC_SEL_SWV		                    ((uint32_t) (0x01))
#define PAD18_FUNC_SEL_DEBUG_MON_0                  ((uint32_t) (0x02))
#define PAD18_FUNC_SEL_GPIO_4                     	((uint32_t) (0x03  | (IO_REG_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD18_FUNC_SEL_SENS_INT_1                   ((uint32_t) (0x00  | (S_INTR_1_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD19_FUNC_SEL_FBIO_19                      ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD19_FUNC_SEL_UART_TXD                     ((uint32_t) (0x02))
#define PAD19_FUNC_SEL_SPIs_MOSI                    ((uint32_t) (0x00  | (SPIs_MOSI_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD20_FUNC_SEL_FBIO_20                      ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD20_FUNC_SEL_UART_TXD                     ((uint32_t) (0x01))
#define PAD20_FUNC_SEL_SPIs_SSn                      ((uint32_t) (0x00  | (SPIs_SSn_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD21_FUNC_SEL_FBIO_21                      ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD21_FUNC_SEL_nUARTRTS                     ((uint32_t) (0x01))
#define PAD21_FUNC_SEL_DEBUG_MON_1                  ((uint32_t) (0x02))
#define PAD21_FUNC_SEL_GPIO_5                       ((uint32_t) (0x03  | (IO_REG_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD21_FUNC_SEL_IRDA_SIRIN                   ((uint32_t) (0x00  | (IRDA_SIRIN_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD21_FUNC_SEL_SENS_INT_2                   ((uint32_t) (0x00  | (S_INTR_2_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD22_FUNC_SEL_FBIO_22                      ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD22_FUNC_SEL_IRDA_SIROUT                  ((uint32_t) (0x01))
#define PAD22_FUNC_SEL_DEBUG_MON_2                  ((uint32_t) (0x02))
#define PAD22_FUNC_SEL_GPIO_6                       ((uint32_t) (0x03  | (IO_REG_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD22_FUNC_SEL_nUARTCTS                     ((uint32_t) (0x00  | (nUARTCTS_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD22_FUNC_SEL_SENS_INT_3                   ((uint32_t) (0x00  | (S_INTR_3_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD23_FUNC_SEL_FBIO_23                      ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD23_FUNC_SEL_SPIm_SSn2                    ((uint32_t) (0x01))
#define PAD23_FUNC_SEL_SWV                     		((uint32_t) (0x02))
#define PAD23_FUNC_SEL_GPIO_7                       ((uint32_t) (0x03  | (IO_REG_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD23_FUNC_SEL_AP_I2S_WD_CLK_IN             ((uint32_t) (0x00  | (I2S_WD_CLKIN_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD23_FUNC_SEL_SENS_INT_7		            ((uint32_t) (0x00  | (S_INTR_7_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD24_FUNC_SEL_FBIO_24                      ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD24_FUNC_SEL_AP_I2S_DOUT                  ((uint32_t) (0x01))
#define PAD24_FUNC_SEL_UART_TXD                     ((uint32_t) (0x02))
#define PAD24_FUNC_SEL_GPIO_0                       ((uint32_t) (0x03  | (IO_REG_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD24_FUNC_SEL_IRDA_SIRIN                   ((uint32_t) (0x00  | (IRDA_SIRIN_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD24_FUNC_SEL_SENS_INT_1		            ((uint32_t) (0x00  | (S_INTR_1_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD25_FUNC_SEL_FBIO_25                      ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD25_FUNC_SEL_SPIm_SSN3                    ((uint32_t) (0x01))
#define PAD25_FUNC_SEL_SWV                          ((uint32_t) (0x02))
#define PAD25_FUNC_SEL_IRDA_SIROUT                  ((uint32_t) (0x03))
#define PAD25_FUNC_SEL_UART_RXD                     ((uint32_t) (0x00  | (UART_RXD_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD25_FUNC_SEL_SENS_INT_2		            ((uint32_t) (0x00  | (S_INTR_2_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD26_FUNC_SEL_FBIO_26                      ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD26_FUNC_SEL_SPI_SENSOR_SSn_4             ((uint32_t) (0x01))
#define PAD26_FUNC_SEL_DEBUG_MON_3                  ((uint32_t) (0x02))
#define PAD26_FUNC_SEL_GPIO_1                       ((uint32_t) (0x03  | (IO_REG_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD26_FUNC_SEL_SENS_INT_4		            ((uint32_t) (0x00  | (S_INTR_4_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD27_FUNC_SEL_FBIO_27                      ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD27_FUNC_SEL_SPI_SENSOR_SSn_5             ((uint32_t) (0x01))
#define PAD27_FUNC_SEL_DEBUG_MON_4                  ((uint32_t) (0x02))
#define PAD27_FUNC_SEL_SPIm_SSn2                    ((uint32_t) (0x03))
#define PAD27_FUNC_SEL_SPIm_SSN2                    (PAD27_FUNC_SEL_SPIm_SSn_2) //deprecated
#define PAD27_FUNC_SEL_SENS_INT_5		            ((uint32_t) (0x00  | (S_INTR_5_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD28_FUNC_SEL_FBIO_28                      ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD28_FUNC_SEL_SPI_SENSOR_MOSI              ((uint32_t) (0x01  | (SPI_SENSOR_MOSI_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD28_FUNC_SEL_DEBUG_MON_5                  ((uint32_t) (0x02))
#define PAD28_FUNC_SEL_GPIO_2                   	((uint32_t) (0x03  | (IO_REG_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD28_FUNC_SEL_I2S_DIN             			((uint32_t) (0x00  | (I2S_DATA_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD28_FUNC_SEL_PDM_DIN             			((uint32_t) (0x00  | (PDM_DATA_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD28_FUNC_SEL_IRDA_SIRIN             		((uint32_t) (0x00  | (IRDA_SIRIN_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD28_FUNC_SEL_SENS_INT_3		            ((uint32_t) (0x00  | (S_INTR_3_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD29_FUNC_SEL_FBIO_29                      ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD29_FUNC_SEL_PDM_CKO                    	((uint32_t) (0x01))
#define PAD29_FUNC_SEL_I2S_CKO                     	((uint32_t) (0x02))
#define PAD29_FUNC_SEL_IRDA_SIROUT                  ((uint32_t) (0x03))
#define PAD29_FUNC_SEL_SPI_SENSOR_MISO             	((uint32_t) (0x00  | (SPI_SENSOR_MISO_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD29_FUNC_SEL_SENS_INT_4		            ((uint32_t) (0x00  | (S_INTR_4_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD30_FUNC_SEL_FBIO_30                      ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD30_FUNC_SEL_SPI_SENSOR_SSn_1             ((uint32_t) (0x01))
#define PAD30_FUNC_SEL_I2S_WD_CKO                   ((uint32_t) (0x02))
#define PAD30_FUNC_SEL_GPIO_3                       ((uint32_t) (0x03  | (IO_REG_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD30_FUNC_SEL_PDM_STAT_IN             		((uint32_t) (0x00  | (PDM_STAT_IN_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD30_FUNC_SEL_SENS_INT_5		            ((uint32_t) (0x00  | (S_INTR_5_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD31_FUNC_SEL_FBIO_31                      ((uint32_t) (0x00  | (FBIO_SEL_1 << EXT_REG_OFFSET_SHIFT)))
#define PAD31_FUNC_SEL_SPI_SENSOR_CLK               ((uint32_t) (0x01))
#define PAD31_FUNC_SEL_RESERVE                      ((uint32_t) (0x02))
#define PAD31_FUNC_SEL_GPIO_4                     	((uint32_t) (0x03  | (IO_REG_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD31_FUNC_SEL_AP_I2S_CLK_IN             	((uint32_t) (0x00  | (I2S_CLKIN_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD31_FUNC_SEL_SENS_INT_6		            ((uint32_t) (0x00  | (S_INTR_6_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD32_FUNC_SEL_FBIO_32                      ((uint32_t) (0x00  | (FBIO_SEL_2 << EXT_REG_OFFSET_SHIFT)))
#define PAD32_FUNC_SEL_SPI_SENSOR_SSn_5             ((uint32_t) (0x01))
#define PAD32_FUNC_SEL_DEBUG_MON_6                  ((uint32_t) (0x02))
#define PAD32_FUNC_SEL_SDA_1                        ((uint32_t) (0x03  | (SDA1_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD32_FUNC_SEL_SENS_INT_6		            ((uint32_t) (0x00  | (S_INTR_6_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD33_FUNC_SEL_FBIO_33                      ((uint32_t) (0x00  | (FBIO_SEL_2 << EXT_REG_OFFSET_SHIFT)))
#define PAD33_FUNC_SEL_SPI_SENSOR_SSn_6             ((uint32_t) (0x01))
#define PAD33_FUNC_SEL_DEBUG_MON_7                  ((uint32_t) (0x02))
#define PAD33_FUNC_SEL_SCL_1                     	((uint32_t) (0x03  | (SCL1_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD33_FUNC_SEL_SENS_INT_7		            ((uint32_t) (0x00  | (S_INTR_7_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD34_FUNC_SEL_SPIm_CLK                  	((uint32_t) (0x00))
#define PAD34_FUNC_SEL_FBIO_34                      ((uint32_t) (0x01  | (FBIO_SEL_2 << EXT_REG_OFFSET_SHIFT)))
#define PAD34_FUNC_SEL_AP_PDM_STAT_O                ((uint32_t) (0x02  | (PDM_STAT_IN_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD34_FUNC_SEL_DEBUG_MON_0                  ((uint32_t) (0x03))
#define PAD34_FUNC_SEL_SENS_INT_7		            ((uint32_t) (0x00  | (S_INTR_7_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD35_FUNC_SEL_FBIO_35                      ((uint32_t) (0x00  | (FBIO_SEL_2 << EXT_REG_OFFSET_SHIFT)))
#define PAD35_FUNC_SEL_SPIm_SSn3                    ((uint32_t) (0x01))
#define PAD35_FUNC_SEL_SPIm_SSN3                    ((uint32_t) (0x01)) // Deprecated
#define PAD35_FUNC_SEL_SPI_SENSOR_SSn_7             ((uint32_t) (0x02))
#define PAD35_FUNC_SEL_DEBUG_MON_1                  ((uint32_t) (0x03))
#define PAD36_FUNC_SEL_SENS_INT_1		            ((uint32_t) (0x00  | (S_INTR_1_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD36_FUNC_SEL_FBIO_36                      ((uint32_t) (0x00  | (FBIO_SEL_2 << EXT_REG_OFFSET_SHIFT)))
#define PAD36_FUNC_SEL_SWV                        	((uint32_t) (0x01))
#define PAD36_FUNC_SEL_SPI_SENSOR_SSn_2             ((uint32_t) (0x02))
#define PAD36_FUNC_SEL_GPIO_5                  		((uint32_t) (0x03  | (IO_REG_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD36_FUNC_SEL_SPIm_MISO                  	((uint32_t) (0x00  | (SPIm_MISO_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD36_FUNC_SEL_SENS_INT_1		            ((uint32_t) (0x00  | (S_INTR_1_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD37_FUNC_SEL_FBIO_37                      ((uint32_t) (0x00  | (FBIO_SEL_2 << EXT_REG_OFFSET_SHIFT)))
#define PAD37_FUNC_SEL_SDA_2_DPU                    ((uint32_t) (0x01))
#define PAD37_FUNC_SEL_SPI_SENSOR_SSn_8             ((uint32_t) (0x02))
#define PAD37_FUNC_SEL_DEBUG_MON_2                  ((uint32_t) (0x03))
#define PAD37_FUNC_SEL_SENS_INT_2		            ((uint32_t) (0x00  | (S_INTR_2_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD38_FUNC_SEL_SPIm_MOSI                  	((uint32_t) (0x00))
#define PAD38_FUNC_SEL_FBIO_38                      ((uint32_t) (0x01  | (FBIO_SEL_2 << EXT_REG_OFFSET_SHIFT)))
#define PAD38_FUNC_SEL_DEBUG_MON_3                  ((uint32_t) (0x02))
#define PAD38_FUNC_SEL_GPIO_6                       ((uint32_t) (0x03  | (IO_REG_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD38_FUNC_SEL_AP_PDM_CKO_IN             	((uint32_t) (0x00))
#define PAD38_FUNC_SEL_SENS_INT_2		            ((uint32_t) (0x00  | (S_INTR_2_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD39_FUNC_SEL_SPIm_SSn1                  	((uint32_t) (0x00))
#define PAD39_FUNC_SEL_FBIO_39                      ((uint32_t) (0x01  | (FBIO_SEL_2 << EXT_REG_OFFSET_SHIFT)))
#define PAD39_FUNC_SEL_AP_PDM_IO                    ((uint32_t) (0x02))
#define PAD39_FUNC_SEL_DEBUG_MON_4                  ((uint32_t) (0x03))
#define PAD39_FUNC_SEL_SENS_INT_3		            ((uint32_t) (0x00  | (S_INTR_3_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD40_FUNC_SEL_FBIO_40                      ((uint32_t) (0x00  | (FBIO_SEL_2 << EXT_REG_OFFSET_SHIFT)))
#define PAD40_FUNC_SEL_SCL_2                        ((uint32_t) (0x01  | (SCL2_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD40_FUNC_SEL_DEBUG_MON_5                  ((uint32_t) (0x02))
#define PAD40_FUNC_SEL_RESERVED                     ((uint32_t) (0x03))
#define PAD40_FUNC_SEL_IRDA_SIRIN             		((uint32_t) (0x00  | (IRDA_SIRIN_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD40_FUNC_SEL_SENS_INT_3		            ((uint32_t) (0x00  | (S_INTR_3_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD41_FUNC_SEL_FBIO_41                      ((uint32_t) (0x00  | (FBIO_SEL_2 << EXT_REG_OFFSET_SHIFT)))
#define PAD41_FUNC_SEL_SDA_2                        ((uint32_t) (0x01  | (SDA2_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD41_FUNC_SEL_DEBUG_MON_6                  ((uint32_t) (0x02))
#define PAD41_FUNC_SEL_IRDA_SIROUT                  ((uint32_t) (0x03))
#define PAD41_FUNC_SEL_SENS_INT_6		            ((uint32_t) (0x00  | (S_INTR_6_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD42_FUNC_SEL_FBIO_42                      ((uint32_t) (0x00  | (FBIO_SEL_2 << EXT_REG_OFFSET_SHIFT)))
#define PAD42_FUNC_SEL_SWV                        	((uint32_t) (0x01))
#define PAD42_FUNC_SEL_DEBUG_MON_7                  ((uint32_t) (0x02))
#define PAD42_FUNC_SEL_SDA_1_DPU                    ((uint32_t) (0x03))
#define PAD42_FUNC_SEL_SENS_INT_7		            ((uint32_t) (0x00  | (S_INTR_7_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD43_FUNC_SEL_AP_INTR                  	((uint32_t) (0x00))
#define PAD43_FUNC_SEL_FBIO_43                      ((uint32_t) (0x01  | (FBIO_SEL_2 << EXT_REG_OFFSET_SHIFT)))

#define PAD44_FUNC_SEL_SW_DP_IO                  	((uint32_t) (0x00))
#define PAD44_FUNC_SEL_FBIO_44                      ((uint32_t) (0x01  | (FBIO_SEL_2 << EXT_REG_OFFSET_SHIFT)))
#define PAD44_FUNC_SEL_SDA_1                  		((uint32_t) (0x02  | (SDA1_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD44_FUNC_SEL_UART_TXD                  	((uint32_t) (0x03))
#define PAD44_FUNC_SEL_IRDA_SIRIN             		((uint32_t) (0x00  | (IRDA_SIRIN_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD44_FUNC_SEL_SENS_INT_4		            ((uint32_t) (0x00  | (S_INTR_4_SEL << EXT_REG_OFFSET_SHIFT)))

#define PAD45_FUNC_SEL_FBIO_45                      ((uint32_t) (0x00  | (FBIO_SEL_2 << EXT_REG_OFFSET_SHIFT)))
#define PAD45_FUNC_SEL_IRDA_SIROUT                  ((uint32_t) (0x01))
#define PAD45_FUNC_SEL_SCL_1	                  	((uint32_t) (0x02  | (SCL1_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD45_FUNC_SEL_GPIO_7                  		((uint32_t) (0x03  | (IO_REG_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD45_FUNC_SEL_SW_DP_CLK             		((uint32_t) (0x00))
#define PAD45_FUNC_SEL_UART_RXD             		((uint32_t) (0x00  | (UART_RXD_SEL << EXT_REG_OFFSET_SHIFT)))
#define PAD45_FUNC_SEL_SENS_INT_5		            ((uint32_t) (0x00  | (S_INTR_5_SEL << EXT_REG_OFFSET_SHIFT)))

/// @endcond
#ifdef __cplusplus
}
#endif

#endif /* __EOSS3_HAL_PADS_H */
