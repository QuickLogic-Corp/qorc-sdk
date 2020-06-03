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
 *    File   : s3x_clock_hal.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __S3X_CLOCK_HAL_H
#define __S3X_CLOCK_HAL_H


#include <FreeRTOS.h>
#include <task.h>
#include "eoss3_dev.h"
#include "test_types.h"
#define MHZ 1000000
#define KHZ 1000


typedef enum
{
    /*C01 clk gate*/
   S3X_A0_01_CLK,           //  0 to A0 ********** A0 from C08 also. Need to check ******************
   S3X_SDMA_SRAM_CLK,       //  1 to SDMA SRAM
   S3X_PKT_FIFO_CLK,        //  2 to Packet FIFO
   S3X_FFE_CLK,             //  3 to FFE
   S3X_CFG_DMA_A1_CLK,      //  4 to CFG DMA Bridge inside A1 / AHB2APB bridge
   S3X_I2S_A1_CLK,          //  5 to I2S module inside A1
   S3X_SDMA_CLK,            //  6 to SDMA
   S3X_EFUSE_01_CLK,        // 7 to eFuse        *********** eFUSE from C02 also. Need to check***************
   S3X_SPT_CLK,             // 8 to SPT


   /*C02 clk gate*/
   S3X_A1_CLK,              // 9 to A1 including CFGSM
   S3X_FB_02_CLK,           // 10 to FB           *********** FB from C16 also. Need to check******************
   S3X_EFUSE_02_CLK,        // 11 to eFuse        *********** eFUSE from C01 also. Need to check***************


   /*C08 X4 clk gate*/
   S3X_FFE_X4_CLK,          // 12 to FFE X4 clk

   /*C08 X1 clk gate*/
   S3X_FFE_X1_CLK,          // 13 to FFE X1 clk
   S3X_A0_08_CLK,           // 14 to A0 ********** A0 from C01 also. Need to check ******************
   S3X_ASYNC_FIFO_0_CLK,    // 15 to PF ASYNC FIFO 0

  /*C9 Clk Gate */
   S3X_AUDIO_APB,           // 16
   S3X_CLKGATE_PIF,         // 17
   S3X_CLKGATE_FB,          //18
   /*C10 clk gate*/
    /*TODO : Check for S3X_M4_BM_TB_CLK. Current status - Not handled*/
   S3X_M4_BM_TB_CLK, //  19 to M4 Bus Matrix and Trace block
                     //  This bit will be set if any of the Memories (M4S0~M4S3)been wakeup  by Hardware.
   S3X_M4_S0_S3_CLK,// 20 to M4 SRAM Instance, M4S0~M4S3.
   S3X_M4_S4_S7_CLK,// 21 to M4 SRAM Instance, M4S4~M4S7
   S3X_M4_S8_S11_CLK,// 22 to M4 SRAM Instance, M4S8~M4S11
   S3X_M4_S12_S15_CLK,// 23 to M4 SRAM Instance, M4S12~M4S15
   S3X_AUDIO_DMA_CLK,// 24 to AUDIO DMA
   S3X_SYNCUP_A0_AHB_CLK,// 25 to the SYNC Up on A0 and AHB Interface of Batching Memory

   /*C11 clk gate*/
   S3X_M4_PRPHRL_CLK,// 26 to M4 peripherals - AHB/APB bridge, UART, WDT and TIMER

   /*CS clk gate*/
   S3X_SWD_PIN_CLK, //  27 to SWD Clk from PIN

   /*C16 clk gate*/
   S3X_FB_16_CLK, // 28 to FB *********** FB from C02 also. Need to check******************

   /*CLK reserved 0*/

   /*C19 clk gate*/
   S3X_ADC_CLK,// 29 To ADC

   /*C21 clk gate*/
   S3X_FB_21_CLK, // 30 To FB(additional clock) ********** FB from C16 also. Need to check******************

   /* C30 C31 */
   S3X_PDM_LEFT,    //31
   S3X_PDM_RIGHT,   //32
   S3X_PDM_STEREO,  //33
   S3X_I2S_MASTER,  //34
   S3X_LPSD,        //35
   S3X_MAX_CLK      //36
}S3x_CLK_ID;
#define MAX_QOS_REQ 3
typedef enum {
    MIN_HSOSC_FREQ = 0x1,
    MIN_CPU_FREQ = 0x2,
    MIN_OP_FREQ = 0x4,
    OP_REQ_END = 0x4
} QOS_REQ_TYPE;

/*To enable clock. Pass the clock ID as defined in the enum S3x_CLK_ID*/
int S3x_Clk_Enable(UINT32_t clk_id);

/*To disable clock. Pass the clock ID as defined in the enum S3x_CLK_ID*/
int S3x_Clk_Disable(UINT32_t clk_id);

/*To set clock rate. Pass the cloack ID and the desired rate*/
int S3x_Clk_Set_Rate(UINT32_t clk_id, UINT32_t rate);

/**To set clock rate. Pass the clock ID and the desired range of rate (min  and max rate value)*/
//int S3x_Clk_Set_Rate(UINT32_t clk_id, UINT32_t rate_min,  UINT32_t rate_max);

/*To get the rate for the corresponding clock ID. Pass the clock ID as defined in the enum S3x_CLK_ID*/
int S3x_Clk_Get_Rate(UINT32_t clk_id);

/*To get Status for the clock*/
int S3x_Clk_Get_Status(UINT32_t clk_id);

/*To get the use count for the clock ID (gate)*/
int S3x_Clk_Get_Usecnt(UINT32_t clk_id);

int S3x_Register_Qos_Node(UINT32_t clk_id);

int S3x_Set_Qos_Req(UINT32_t clk_id, QOS_REQ_TYPE, UINT32_t val);

int S3x_Get_Qos_Req(UINT32_t clk_id, QOS_REQ_TYPE req);

int S3x_Clear_Qos_Req(UINT32_t clk_id, QOS_REQ_TYPE);

#endif      /* __S3X_CLOCK_HAL_H  */
