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

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "RtosTask.h"

/*    Include the generic headers required for sensorHub */
#include "eoss3_hal_gpio.h"
#include "eoss3_hal_rtc.h"
#include "eoss3_hal_uart.h"
#include "ql_time.h"
#include "s3x_clock_hal.h"
#include "s3x_clock.h"
#include "s3x_pi.h"
//#include "QL_Trace.h"
#include "dbg_uart.h"

//#include "cli.h"

#include "sec_debug.h"
//#include "eoss3_hal_spi.h"
//#include "spi_flash.h"
//#include "h2d_protocol.h"
//#include "ql_hostTask.h"

#include "fpga_loader.h"
#include "gateware.h"           // FPGA bitstream

//extern const struct cli_cmd_entry my_main_menu[];


const char *SOFTWARE_VERSION_STR;


/*
 * Global variable definition
 */



#if DBG_FLAGS_ENABLE
uint32_t DBG_flags = DBG_flags_default;
#endif

extern void start_load_from_flash(void);   

#if defined(ENABLE_LOAD_FPGA) || defined(ENABLE_LOAD_FFE)
uint32_t scratch_1Kbyte_ram[256];
#endif



extern void qf_hardwareSetup();
static void nvic_init(void);

int main(void)
{

    SOFTWARE_VERSION_STR = "OP2-QuickFeather-loadflash-uart";
    
    qf_hardwareSetup();
    
    dbg_str("\n\n");
    dbg_str( "##########################\n");
    dbg_str( "Quicklogic Open Platform 2.0\n");
    dbg_str( "SW Version: ");
    dbg_str( SOFTWARE_VERSION_STR );
    dbg_str( "\n" );
    dbg_str( __DATE__ " " __TIME__ "\n" );
    dbg_str( "##########################\n\n");

    nvic_init();

    /* Enable FPGA to M4 Interrupt 0, Edge triggered Rising edge polarity */
    FB_ConfigureInterrupt ( 0 /* int 0 */, FB_INTERRUPT_TYPE_EDGE, FB_INTERRUPT_POL_EDGE_RISE, FB_INTERRUPT_DEST_AP_DISBLE, FB_INTERRUPT_DEST_M4_ENABLE );

    S3x_Clk_Disable(S3X_FB_21_CLK);
    S3x_Clk_Disable(S3X_FB_16_CLK);
    
    S3x_Clk_Enable(S3X_A1_CLK);
    S3x_Clk_Enable(S3X_CFG_DMA_A1_CLK);
#if (FEATURE_USBSERIAL == 1)
    load_fpga(axFPGABitStream_length,axFPGABitStream);
    // Use 0x6140 as the USB serial product ID (USB PID)
    HAL_usbserial_init2(false, false, 0x6140);          // Start USB serial not using interrupts
    for (int i = 0; i != 4000000; i++) ;   // Give it time to enumerate
#endif
    LoadFlash_Task_Init();
    /* Start the tasks and timer running */
    vTaskStartScheduler();
#if 0 
    S3x_Clk_Set_Rate(S3X_FB_21_CLK, 48000*1000);
    S3x_Clk_Set_Rate(S3X_FB_16_CLK, 12000*1000);
    S3x_Clk_Enable(S3X_FB_21_CLK);
	S3x_Clk_Enable(S3X_FB_16_CLK);
#endif  
    while(1);
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
 }   

//needed for startup_EOSS3b.s asm file
void SystemInit(void)
{

}


