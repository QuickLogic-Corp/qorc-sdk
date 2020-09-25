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
 *    File   : qf_hardwaresetup.c
 *    Purpose: 
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "Fw_global_config.h"
#include "QL_Trace.h"

#include "eoss3_hal_gpio.h"
#include "eoss3_hal_pad_config.h"
#include "eoss3_hal_uart.h"
#include "s3x_clock.h"
#include "Bootconfig.h"
#include "pincfg_table.h"

void isr_init_hwp(void);
void system_init(void);
static void uart_pad_setup(void);
void qf_hardwareSetup(void) {

    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */
    
    SCnSCB->ACTLR |= SCnSCB_ACTLR_DISDEFWBUF_Msk;
    S3x_pwrcfg_init();
    system_init();
    //configure_s3_gpio_interrupts(gpiocfg_table, sizeof_gpiocfg_table);
    configure_s3_pads(pincfg_table, sizeof_pincfg_table);
    uart_pad_setup();
    isr_init_hwp();
}

void isr_init_hwp()
{
    /*
	GPIOCfgTypeDef  xGpioCfg;


	xGpioCfg.usPadNum = PAD_3;
	xGpioCfg.ucGpioNum = GPIO_0;
	xGpioCfg.ucFunc = PAD3_FUNC_SEL_SENS_INT_0;
	xGpioCfg.intr_type = LEVEL_TRIGGERED;//EDGE_TRIGGERED;
	xGpioCfg.pol_type = FALL_LOW;//FALL_LOW;//RISE_HIGH;
	xGpioCfg.ucPull = PAD_PULLUP;

	HAL_GPIO_IntrCfg(&xGpioCfg);

	xGpioCfg.usPadNum = PAD_37;
	xGpioCfg.ucGpioNum = GPIO_2;
	xGpioCfg.ucFunc = PAD37_FUNC_SEL_FBIO_37;
	xGpioCfg.intr_type = EDGE_TRIGGERED;
	xGpioCfg.pol_type = FALL_LOW;
	HAL_GPIO_IntrCfg(&xGpioCfg);
    */

}


static void uart_pad_setup()
{
	int uart_id;
	UartBaudRateType brate;
	UartHandler uartObj;
    memset( (void *)&(uartObj), 0, sizeof(uartObj) );

    uart_id = UART_ID_HW;
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

/* Select one of the 3 Pad configurarions for PDM Audio */
#if PDM_PAD_28_29
static void Audio_pad_setup(void)
{
  IO_MUX->PDM_DATA_SELE = 0x02;   // 1 for pad10, 2 for pad28
}
#endif /* PDM_PAD_28_29 */
#if PDM_PAD_8_10
static void Audio_pad_setup(void)
{
  /* PDM PAD 10 and PAD 8*/
  // Select pad10 or 28 for pdm din
  IO_MUX->PDM_DATA_SELE = 0x01;   // 1 for pad10, 2 for pad28
  
  //-------------------------------------------------------------------------------------------
  // IO_MUX pad controls for audio
  // Pad 10 or pad 28 is selected for i2s data in or pdm din
  // Pad 10 or pad 28 is configured as input for i2s data in or pdm din
  //-------------------------------------------------------------------------------------------
  
}
#endif /* PDM_PAD_8_10 */
#if VOICE_AP_BYPASS_MODE
//void VoiceAPbypassPadCtrl(void)
static void Audio_pad_setup(void)
{
  /* Select for PAD 38 */
  IO_MUX->PDM_CLKIN_SEL = 0x01;
}

#endif /* VOICE_AP_BYPASS_MODE */

