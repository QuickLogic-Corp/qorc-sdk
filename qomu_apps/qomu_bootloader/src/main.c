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
 *    Purpose: 
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "eoss3_dev.h"
#include "core_cm4.h"
#include "string.h"
#include "dbg_uart.h"
#include "s3x_clock_hal.h"
#include "s3x_clock.h"
#include "s3x_pi.h"
#include "eoss3_hal_spi.h"
#include "spi_flash.h"
#include "eoss3_hal_uart.h"
#include "eoss3_hal_timer.h"
#include "qomu_hardwaresetup.h"

static void nvic_init(void);
extern void BL_Task_Init(void);

const char *SOFTWARE_VERSION_STR="QF_BL 1.0" ;

// The entry Function of the BL when the system boots up.
int main(void)
{
#if defined(__ICCARM__)
  SOFTWARE_VERSION_STR = "qorc-sdk/qf-apps/qf_bootloader(v2.1) (IAR)";
#elif defined(__GNUC__)
  SOFTWARE_VERSION_STR = "qorc-sdk/qf-apps/qf_bootloader(v2.1) (GCC)";
#endif

  qomu_hardwaresetup();

  dbg_str("\n\n");
  dbg_str( "##########################\n");
  dbg_str( "Quicklogic Qomu Bootloader\n");
  dbg_str( "SW Version: ");
  dbg_str( SOFTWARE_VERSION_STR );
  dbg_str( "\n" );
  dbg_str( __DATE__ " " __TIME__ "\n" );
  dbg_str( "##########################\n\n");

  nvic_init();

  /* Enable FPGA to M4 Interrupt 0, Edge triggered Rising edge polarity */
  FB_ConfigureInterrupt ( 0 /* int 0 */, FB_INTERRUPT_TYPE_EDGE, FB_INTERRUPT_POL_EDGE_RISE, FB_INTERRUPT_DEST_AP_DISBLE, FB_INTERRUPT_DEST_M4_ENABLE );

  BL_Task_Init();
  /* Start the tasks and timer running. */
  vTaskStartScheduler();
  while(1);
}

static void nvic_init(void)
 {
    // To initialize system, this interrupt should be triggered at main.
    // So, we will set its priority just before calling vTaskStartScheduler(), not the time of enabling each irq.
    NVIC_SetPriority(Uart_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(SpiMs_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	NVIC_SetPriority(CfgDma_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

 }    

//needed for startup_EOSS3b.s asm file
void SystemInit(void)
{

}


