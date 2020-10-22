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
 *    Purpose: main for QuickFeather helloworldsw and LED/UserButton test
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"   // This defines application specific charactersitics

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "RtosTask.h"

/*    Include the generic headers required for QORC */
#include "eoss3_hal_gpio.h"
#include "eoss3_hal_rtc.h"
#include "eoss3_hal_fpga_usbserial.h"
#include "eoss3_hal_uart.h"
#include "ql_time.h"
#include "s3x_clock_hal.h"
#include "s3x_clock.h"
#include "s3x_pi.h"
#include "dbg_uart.h"

#include "cli.h"

#include "fpga_loader.h"    // API for loading FPGA
#include "top_bit.h"        // FPGA bitstream to load into FPGA

extern const struct cli_cmd_entry my_main_menu[];


const char *SOFTWARE_VERSION_STR;


/*
 * Global variable definition
 */


extern void qf_hardwareSetup();
static void nvic_init(void);

int main(void)
{

    SOFTWARE_VERSION_STR = "qorc-sdk/qf_apps/qf_fpgauart_app";
    
    qf_hardwareSetup();
    nvic_init();
    S3x_Clk_Disable(S3X_FB_21_CLK);
    S3x_Clk_Disable(S3X_FB_16_CLK);
    S3x_Clk_Enable(S3X_A1_CLK);
    S3x_Clk_Enable(S3X_CFG_DMA_A1_CLK);
    load_fpga(sizeof(axFPGABitStream), axFPGABitStream);
#if (FEATURE_USBSERIAL == 1)
    // Use 0x6141 as USB serial product ID (USB PID)
    HAL_usbserial_init2(false, false, 0x6141);        // Start USB serial not using interrupts
    for (int i = 0; i != 4000000; i++) ;   // Give it time to enumerate
#endif

#if (FEATURE_FPGA_UART == 1)
    int uart_id;
    UartBaudRateType brate;
    UartHandler uartObj;
    memset( (void *)&(uartObj), 0, sizeof(uartObj) );

    uart_id = UART_ID_FPGA;
    brate = BAUD_115200;
    uartObj.baud = brate;
    uartObj.wl = WORDLEN_8B;
    uartObj.parity = PARITY_NONE;
    uartObj.stop = STOPBITS_1;
    uartObj.mode = TX_RX_MODE;
    uartObj.hwCtrl = HW_FLOW_CTRL_DISABLE;
    uartObj.intrMode = UART_INTR_ENABLE;
    uartHandlerUpdate(uart_id,&uartObj);

    for (int i = 0; i != 4000000; i++) ;   // [TODO] Analyze and remove need for this delay
    uart_init( uart_id, NULL, NULL, &uartObj);

    uint32_t device_id = *(uint32_t *)FPGA_PERIPH_BASE ;

    if (device_id == 0xABCD0002)
    {
      uart_id = UART_ID_FPGA_UART1;
      brate = BAUD_115200;
      uartObj.baud = brate;
      uartObj.wl = WORDLEN_8B;
      uartObj.parity = PARITY_NONE;
      uartObj.stop = STOPBITS_1;
      uartObj.mode = TX_RX_MODE;
      uartObj.hwCtrl = HW_FLOW_CTRL_DISABLE;
      uartObj.intrMode = UART_INTR_ENABLE;
      uartHandlerUpdate(uart_id,&uartObj);

      uart_init( uart_id, NULL, NULL, &uartObj);
    }
#endif

    dbg_str("\n\n");
    dbg_str( "##########################\n");
    dbg_str( "Quicklogic QuickFeather FPGA-UART Test Application\n");
    dbg_str( "SW Version: ");
    dbg_str( SOFTWARE_VERSION_STR );
    dbg_str( "\n" );
    dbg_str( __DATE__ " " __TIME__ "\n" );
    dbg_str( "##########################\n\n");
	
	dbg_str( "\n\nHello world!!\n\n");	// <<<<<<<<<<<<<<<<<<<<<  Change me!

    CLI_start_task( my_main_menu );
        
    /* Start the tasks and timer running. */
    vTaskStartScheduler();
    dbg_str("\n");
      
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


