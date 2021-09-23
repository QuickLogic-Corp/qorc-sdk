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
//#include "QL_Trace.h"

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

static void ldo_init(void);
void system_init(void);
static void uart_setup(void);
//static void i2c_init_for_m4();

void qf_hardwareSetup(void) {

    SCnSCB->ACTLR |= SCnSCB_ACTLR_DISDEFWBUF_Msk;
    S3x_pwrcfg_init();
    system_init();
    ldo_init();
    //configure_s3_gpio_interrupts(gpiocfg_table, sizeof_gpiocfg_table);
    configure_s3_pads(pincfg_table, sizeof_pincfg_table);
    uart_setup();
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

static void ldo_init(void)
{
    /* LDO Settings */
#if ( ENABLE_INTERNAL_LDO == 1)
    AIP->LD0_30_CTRL_0 = 0x1ac; // LDO Enable       /* 0x1ac -> Vo =1.01V, imax = 7.2mA, LDO enabled. */
    AIP->LD0_50_CTRL_0 = 0x1ac; // LDO Enable
    AIP->LD0_30_CTRL_0 = 0x28c; // LDO Enable       /* 0x28c -> Vo =1.15V, imax = 7.2mA, LDO enabled. */
    AIP->LD0_50_CTRL_0 = 0x28c; // LDO Enable
#else
    AIP->LD0_30_CTRL_0 = 0x1a1; // LDO Disable     /* 0x1a1 -> Vo = 1.01 V, LDO Disabled */
    AIP->LD0_50_CTRL_0 = 0x1a1; // LDO Disable
#endif
}

static void uart_setup()
{
	int uart_id;
	UartBaudRateType brate;
	UartHandler uartObj;
    memset( (void *)&(uartObj), 0, sizeof(uartObj) );

    uart_id = UART_ID_HW;
#if (CONST_FREQ == 1)
	brate = BAUD_460800;
#else
	brate = BAUD_57600;
#endif
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
