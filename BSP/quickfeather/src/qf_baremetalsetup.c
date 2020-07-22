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
 *   File   : qf_hardwareSetup.c
 *   Purpose: Contains functionality to setup QuickFeather Board 
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "Fw_global_config.h"

#include "eoss3_dev.h"

#include "eoss3_hal_gpio.h"
#include "eoss3_hal_pad_config.h"
#include "eoss3_hal_uart.h"
#include "s3x_clock.h"
#include "Bootconfig.h"

void configure_s3_pads(PadConfig *p_table, int nitems);
extern PadConfig pincfg_table[];
extern int sizeof_pincfg_table ;
void configure_s3_gpio_interrupts(GPIOCfgTypeDef *p_table, int nitems);
extern GPIOCfgTypeDef  gpiocfg_table[];
extern int sizeof_gpiocfg_table;

static void AIP_init(void);
static void PMU_init(void);
static void CRU_init(void);
static void ldo_init(void);
static void system_init(void);
static void uart_setup(void);
//static void i2c_init_for_m4();

void qf_baremetalSetup(void) {

    SCnSCB->ACTLR |= SCnSCB_ACTLR_DISDEFWBUF_Msk;
	AIP_init();		// Initialize AIP registers (HSOSC)
	PMU_init();		// Initialize power registers
	CRU_init();		// Initialize clock registers
	
    //S3x_pwrcfg_init();
    system_init();
    ldo_init();
    //configure_s3_gpio_interrupts(gpiocfg_table, sizeof_gpiocfg_table);
    configure_s3_pads(pincfg_table, sizeof_pincfg_table);
    uart_setup();
}

void AIP_init(void) {
	AIP->OSC_CTRL_1 = 2194;		// (72,000,000/32,768) - 3
}

void CRU_init(void) {
	CRU->CLK_CTRL_A_0	 = 0;
	CRU->CLK_CTRL_B_0 	= 0x246;
	CRU->CLK_CTRL_C_0 	= 0x222;
	CRU->CLK_CTRL_D_0 	= 0x222;
	CRU->CLK_CTRL_F_0 	= 0x204;
	CRU->CLK_CTRL_G_0 	= 0x221;
	CRU->CLK_CTRL_H_0 	= 0x3FE;
	CRU->CLK_CTRL_I_0 	= 0x200;
	CRU->C01_CLK_GATE 	= 0x011;
	CRU->C02_CLK_GATE 	= 0x001;
	CRU->C10_FCLK_GATE 	= 0x05F;
	MISC->LOCK_KEY_CTRL	= 0x1ACCE551;
	CRU->C11_CLK_GATE 	= 0x001;
	CRU->C16_CLK_GATE 	= 0x001;
	CRU->C21_CLK_GATE 	= 0x001;
	CRU->C01_CLK_DIV  	= 0x017;
	CRU->C09_CLK_DIV  	= 0x01B;
	CRU->C31_CLK_DIV  	= 0x003;
	CRU->CLK_DIVIDER_CLK_GATING  = 0x33B;
}

void PMU_init(void) {
	PMU->FB_STATUS = 0x001;
	PMU->A1_PWR_MODE_CFG = 0x002;
	PMU->MISC_STATUS = 0x000;
	PMU->AUDIO_STATUS = 0x03F;		// Power up all audio domains
	
	/* Configure Memory to support light sleep and retention mode */
    PMU->M4SRAM_SSW_LPMF = 0xFFFF;
    PMU->M4SRAM_SSW_LPMH_MASK_N = 0xFFFF;
}

//Initialize all System Clocks,Gate settings and  used Power Resources
void system_init(void)
{
    /* FPU settings ------------------------------------------------------------*/
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */
#endif

    INTR_CTRL->OTHER_INTR = 0xffffff;
    DMA_SPI_MS->DMA_CTRL = DMA_CTRL_STOP_BIT;
}

static void ldo_init(void)
{
    /* LDO Settings */
    AIP->LD0_30_CTRL_0 = 0x1ac; // LDO Enable       /* 0x1ac -> Vo =1.01V, imax = 7.2mA, LDO enabled. */
    AIP->LD0_50_CTRL_0 = 0x1ac; // LDO Enable
    AIP->LD0_30_CTRL_0 = 0x28c; // LDO Enable       /* 0x28c -> Vo =1.15V, imax = 7.2mA, LDO enabled. */
    AIP->LD0_50_CTRL_0 = 0x28c; // LDO Enable
}   

static void uart_setup()
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
	uartObj.intrMode = UART_INTR_DISABLE;
	uartObj.fBaremetal = true;
    uartHandlerUpdate(uart_id,&uartObj);

	uart_init( uart_id, NULL, NULL, &uartObj);
}
