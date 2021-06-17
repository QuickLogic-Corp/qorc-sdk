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
#include "eoss3_hal_i2c.h"
#include "eoss3_hal_timer.h"
#include "ql_time.h"
#include "s3x_clock_hal.h"
#include "s3x_clock.h"
#include "s3x_pi.h"
#include "dbg_uart.h"
#include "sensor_ssss.h"
#include "ssi_comms.h"
#include "cli.h"
#include "fpga_loader.h"    // API for loading FPGA
#include "gateware.h"           // FPGA bitstream to load into FPGA
#include "kb.h"
#include "sensor_audio_config.h"
#include "sensor_audio_process.h"
extern const struct cli_cmd_entry my_main_menu[];


const char *SOFTWARE_VERSION_STR;


/*
 * Global variable definition
 */

static const fw_global_config_t fw_global_config_vars =
{
    .ssi_sensor_select_audio = SSI_SENSOR_SELECT_AUDIO,
	.sensor_audio_livestream_enabled = SENSOR_AUDIO_LIVESTREAM_ENABLED,
	.sensor_audio_recog_enabled = SENSOR_AUDIO_RECOG_ENABLED,

	.ssi_sensor_select_ssss = SSI_SENSOR_SELECT_SSSS,
    .sensor_ssss_livestream_enabled = SENSOR_SSSS_LIVESTREAM_ENABLED,
    .sensor_ssss_recog_enabled = SENSOR_SSSS_RECOG_ENABLED
};

#if 1
static I2C_Config i2c0config =
{
  .eI2CFreq = I2C_400KHZ,    // 400kHz
  .eI2CInt = I2C_DISABLE,    // enabled interrupt
  .ucI2Cn = 0
};
#endif
extern void qf_hardwareSetup();
static void nvic_init(void);

int main(void)
{

    SOFTWARE_VERSION_STR = "qorc-sdk/qt_apps/qt_ssi_app";
    
    qf_hardwareSetup();
    nvic_init();
#if (FEATURE_USBSERIAL == 1)
    S3x_Clk_Disable(S3X_FB_21_CLK);
    S3x_Clk_Disable(S3X_FB_16_CLK);
    S3x_Clk_Enable(S3X_A1_CLK);
    S3x_Clk_Enable(S3X_CFG_DMA_A1_CLK);
    load_fpga(axFPGABitStream_length,axFPGABitStream);
    // Use 0x6141 as USB serial product ID (USB PID)
    HAL_usbserial_init2(false, true, 0x6141);        // Start USB serial not using interrupts
    for (int i = 0; i != 4000000; i++) ;   // Give it time to enumerate
#endif
    dbg_str("\n\n");
    dbg_str( "##########################\n");
    dbg_str( "Quicklogic QuickThing+ SensiML AI Data Collection/Recognition App\n");
    dbg_str( "SW Version: ");
    dbg_str( SOFTWARE_VERSION_STR );
    dbg_str( "\n" );
    dbg_str( __DATE__ " " __TIME__ "\n" );
    dbg_str( "##########################\n\n");
	
	dbg_str( "\n\nHello world!!\n\n");	// <<<<<<<<<<<<<<<<<<<<<  Change me!
    HAL_Delay_Init();
#if (PDM_PAD_28_29 == 1)
    IO_MUX->PDM_DATA_SELE = 0x02;   // 1 for pad10, 2 for pad28
#endif
#if (PDM_PAD_8_10 == 1)
    IO_MUX->PDM_DATA_SELE = 0x01;   // 1 for pad10, 2 for pad28
#endif
#if (VOICE_AP_BYPASS_MODE == 1)
    /* Select for PAD 38 */
    IO_MUX->PDM_CLKIN_SEL = 0x01;
#endif
    HAL_I2C_Init(i2c0config);

#if (S3AI_FIRMWARE_IS_RECOGNITION == 1)
    kb_model_init(); /* initialize the knowledgepack */
#endif

#if (SSI_SENSOR_SELECT_SSSS == 1)
    sensor_ssss_block_processor();
#endif

#if (SSI_SENSOR_SELECT_AUDIO == 1)
    audio_block_processor();

#if (SENSOR_AUDIO_RECOG_ENABLED == 1)
    sensor_audio_add();
    sensor_audio_startstop(1);
#endif

#endif /* SSI_SENSOR_SELECT_AUDIO */

#if ( ((SSI_SENSOR_SELECT_SSSS == 1) && (SENSOR_SSSS_LIVESTREAM_ENABLED == 1)) || \
      ((SSI_SENSOR_SELECT_AUDIO == 1) && (SENSOR_AUDIO_LIVESTREAM_ENABLED == 1)) )
    StartSimpleStreamingInterfaceTask();
#endif

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


