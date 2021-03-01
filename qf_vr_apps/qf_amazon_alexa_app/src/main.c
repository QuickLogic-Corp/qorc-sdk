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
 *    File   : main.c
 *    Purpose: main for QL smart remote test application
 *
 *=========================================================*/

#include "Fw_global_config.h"
#include "Bootconfig.h"
#include <float.h>   
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "RtosTask.h"

/*    Include the generic headers required for sensorHub */
#include "eoss3_hal_gpio.h"
#include "eoss3_hal_i2c.h"
#include "eoss3_hal_rtc.h"
#include <ql_time.h>
#include "s3x_clock_hal.h"
#include "s3x_clock.h"
#include "s3x_pi.h"
#include "QL_Trace.h"
#include "ql_controlTask.h"
#include "ql_audio.h"
#include "qlsh_commands.h"
#include "dbg_uart.h"
#include "ql_audio.h"
#include "eoss3_hal_uart.h"

//#include "cli.h"
#include "Fw_task.h"

#include "sec_debug.h"
#include "eoss3_hal_spi.h"
//#include "spi_flash.h"

//#include "vm1010.h"
#include "host_interface.h"
#include "d2h_protocol.h"

//#include "fpga_loader.h"
//#include "FPGA_VoxAI.h"
#include "eoss3_hal_pad_config.h"

extern void ql_smart_remote_example();
extern const struct cli_cmd_entry my_main_menu[];

struct tm calendar = {0};

const char *SOFTWARE_VERSION_STR;


/*
 * Global variable definition
 */
#ifdef __EOSS3_CHIP
volatile uint32_t SystemCoreClock = 72000000;
#else
#error "Need to define SystemCoreClock"
volatile uint32_t SystemCoreClock = 0;
#endif

void softreset_m4(void)
{
  __DSB(); 
  SCB->AIRCR  = ((0x5FA << SCB_AIRCR_VECTKEY_Pos) | (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
      SCB_AIRCR_VECTRESET_Msk);
  __DSB(); 
  while(1); /* control will never reach here */
}
GPIOCfgTypeDef D2H_FSMConfigData = 
{ 
   GPIO_7,
   PAD_23,
   PAD23_FUNC_SEL_SENS_INT_7,
   PAD_NOPULL,
   EDGE_TRIGGERED,
   RISE_HIGH
};					
//this is Ack coming from Host as an interrupt
GPIOCfgTypeDef H2D_AckConfigData = 
{ 
   GPIO_6,//Gpio number has to be interrupt number 
   PAD_31,
   PAD31_FUNC_SEL_SENS_INT_6,
   PAD_NOPULL,
   EDGE_TRIGGERED,
   RISE_HIGH
};
//Using PAD_24 as Ack from S3 and PAD_31 as Ack from Host 
void setup_d2h_hardware(void) {
  
  //these setup only 2 pins
  h2d_config_intr(&D2H_FSMConfigData);
  
  //these 2 setups are for 4 pin protocol
  
  //configure H2D Ack as input interrupt
  HAL_GPIO_IntrCfg(&H2D_AckConfigData);

  //Configure D2H Ack as output 
  PadConfig padcfg;

  //output pin
  padcfg.ucCtrl = PAD_CTRL_SRC_A0; 
  padcfg.ucMode = PAD_MODE_OUTPUT_EN;
  padcfg.ucPull = PAD_NOPULL;
  padcfg.ucDrv = PAD_DRV_STRENGHT_4MA;
  padcfg.ucSpeed = PAD_SLEW_RATE_FAST;
  padcfg.ucSmtTrg = PAD_SMT_TRIG_DIS;
  
  //Pad 24 -- Gpio0
  padcfg.ucPin = PAD_24;
  padcfg.ucFunc = PAD24_FUNC_SEL_GPIO_0;
  HAL_PAD_Config(&padcfg);

  return;
}

void start_d2h_protocol_task(void) {
  
  //setup gpios and interrupts
  setup_d2h_hardware();
    
  D2H_Platform_Info d2h_plat_info;
  d2h_plat_info.H2D_gpio = GPIO_7;
  d2h_plat_info.D2H_gpio = 0xFF;      // D2H intr is through PAD 43. AP intr

#if (USE_4PIN_D2H_PROTOCOL == 1)
  
  d2h_plat_info.H2D_ack = GPIO_6; //GPIO_4;
  d2h_plat_info.D2H_ack = GPIO_0;      // D2H intr is through PAD 24.

#endif
  
  d2h_protocol_init(&d2h_plat_info);
  
  return;
}
extern int  VR_FSMConfigData;

/*
 * Private variable definition
 */

/* neede for startup_EOSS3b.s asm file */
void SystemInit(void)
{

}

extern void qf_hardwareSetup();

static void ldo_init(void)
{
    /* LDO Settings */
#if ( ENABLE_INTERNAL_LDO == 1)
    AIP->LD0_30_CTRL_0 = 0x1ac; // LDO Enable       /* 0x1ac -> Vo =1.01V, imax = 7.2mA, LDO enabled. */
    AIP->LD0_50_CTRL_0 = 0x1ac; // LDO Enable
#else
    AIP->LD0_30_CTRL_0 = 0x1a1; // LDO Disable     /* 0x1a1 -> Vo = 1.01 V, LDO Disabled */
    AIP->LD0_50_CTRL_0 = 0x1a1; // LDO Disable
#endif
}

static void nvic_init(void)
 {
    // To initialize system, this interrupt should be triggered at main.
    // So, we will set its priority just before calling vTaskStartScheduler(), not the time of enabling each irq.
    NVIC_SetPriority(Ffe0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(SpiMs_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(CfgDma_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(Uart_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(FbMsg_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(Sdma_Done0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(Gpio_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

 }

void banner(void)
{
    dbg_str("\n\n");
    dbg_str( "  ############################################################\n\n");
    dbg_str( "       Quicklogic Open Platform 2.0                         \n");                    
    dbg_str( "       SW Version: ");                           
    dbg_str( SOFTWARE_VERSION_STR );
    dbg_str( " \n       " );
    dbg_str( __DATE__ " " __TIME__ " \n" );

    dbg_str( "\n  ############################################################\n\n");
#if 0 //it is FOSS now for AWWE
    
    dbg_str( "  *--  Copyright Notice:  -----------------------------------*    \n");
    dbg_str( "  *                                                          *    \n");
    dbg_str( "  *    Licensed Materials - Property of QuickLogic Corp.     *    \n");
    dbg_str( "  *    Copyright (C) 2019 QuickLogic Corporation             *    \n");
    dbg_str( "  *    All rights reserved                                   *    \n");
    dbg_str( "  *    Use, duplication, or disclosure restricted            *    \n");
    dbg_str( "  *----------------------------------------------------------*    \n\n");
#endif
    
#if (PDM_MIC_STEREO == 1)
    dbg_str("   + PCM Stereo Transport Enabled \n");
#elif (ENABLE_RAW_TX_SPI == 1)
    dbg_str("   + PCM mono Transport Enabled \n");
#else
    dbg_str("   - No Transport \n");
#endif
}

int main(void)
{
 
    SOFTWARE_VERSION_STR = "OP2-AWWE-App";

    // Initialize FSM ConfigData variables
    // this variable specifies the amount of audio data in milli-secs
    // to be pre-processed to minimize the voiceSpot engine detection
    // latency
    VR_FSMConfigData = 375;

    qf_hardwareSetup();
    //setup D2H protocol pins and interrupts and start the task
    start_d2h_protocol_task();
    hif_task_Start();
  
    uart_set_lpm_state(UART_ID_HW,1);
    HAL_RTC_Init(0);
    banner(); 
    //ldo_init();     
    nvic_init();
    ql_smart_remote_example();
    /* for debug - view and control over COM */
#if FEATURE_CLI_DEBUG_INTERFACE
    CLI_start_task( my_main_menu );
#endif
    dbg_startbufferedprinttask(PRIORITY_LOWER);
    StartControlTask(); 
    /* Start the tasks and timer running. */
    vTaskStartScheduler();
    while(1);
}

//Initialize all System Clocks,Gate settings and  used Power Resources
void system_init(void)
{
    S3x_Clk_Enable(S3X_M4_S0_S3_CLK);
    S3x_Clk_Enable(S3X_M4_S4_S7_CLK);
    S3x_Clk_Enable(S3X_M4_S8_S11_CLK);
    S3x_Clk_Enable(S3X_M4_S12_S15_CLK);

    /* FPU settings ------------------------------------------------------------*/
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */
#endif

    /* Configure Memory to support light sleep and retention mode */
    PMU->M4SRAM_SSW_LPMF = 0xFFFF;
    PMU->M4SRAM_SSW_LPMH_MASK_N = 0xFFFF;

    //S3x_Clk_Enable(S3X_FB_16_CLK);
    //S3x_Clk_Enable(S3X_FB_21_CLK);

    S3x_Clk_Enable(S3X_A1_CLK);
    S3x_Clk_Enable(S3X_CFG_DMA_A1_CLK);

    INTR_CTRL->OTHER_INTR = 0xffffff;
    DMA_SPI_MS->DMA_CTRL = DMA_CTRL_STOP_BIT;
}

void i2c_init_for_m4(void)
{
    I2C_Config xI2CConfig;

    xI2CConfig.eI2CFreq = I2C_200KHZ;
    xI2CConfig.eI2CInt = I2C_DISABLE;
    xI2CConfig.ucI2Cn = 0;
    HAL_I2C_Init(xI2CConfig);
    xI2CConfig.ucI2Cn = 1;
    HAL_I2C_Init(xI2CConfig);
}


void GenerateInterruptToBLE(void)
{
   int count =0;
   volatile uint32_t sts=0;
#if 1 //this is failing some times. so disable. check only for interrupt clear
   //first check interrupt is enabled from other side
   do
   {
      sts=INTR_CTRL->SOFTWARE_INTR_2_EN_AP;
      count++;
   } while(!sts);
   count = 0;
#endif

   //next make sure the interrupt is cleared from other side
   do
   {
      sts=INTR_CTRL->SOFTWARE_INTR_2;
      count++;
#if 1
      if(count > (100*1000))
      {
        dbg_str("Error-INTR_2\n");
        break;
      }
#endif
   }while(sts);


   INTR_CTRL->SOFTWARE_INTR_2_EN_AP = 1; // Enable interrupt to AP
   INTR_CTRL->SOFTWARE_INTR_2 = 1;
   return;
}
void ClearInterruptToBLE(void)
{
    INTR_CTRL->SOFTWARE_INTR_2 = 0;
}


void update_calendar(void)
{
    time_t ltime;
    ltime = ql_time(NULL);
    ql_localtime_r(&ltime, &calendar);
}


