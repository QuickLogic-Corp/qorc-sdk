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
 *    File   : board_QuickAI.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"
#include "eoss3_hal_gpio.h"
#include "eoss3_hal_pads.h"
#include "eoss3_hal_pad_config.h"
#include "eoss3_hal_spi.h"
#include "eoss3_hal_uart.h"
#include "s3x_clock.h"
#include "spi_flash.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "string.h"

extern PadConfig pincfg_table[];
extern GPIOCfgTypeDef  gpiocfg_table[];
extern int sizeof_pincfg_table ;
extern int sizeof_gpiocfg_table;

static void ldo_init(void);
static void system_init(void);
static void uart_setup(void);
static void SPIM_Setup(void);

void qf_hardwareSetup(void) {

    SCnSCB->ACTLR |= SCnSCB_ACTLR_DISDEFWBUF_Msk;
    S3x_pwrcfg_init();
    system_init();
    ldo_init();
    //configure_s3_gpio_interrupts(gpiocfg_table, sizeof_gpiocfg_table);
    configure_s3_pads(pincfg_table, sizeof_pincfg_table);
    uart_setup();
    SPIM_Setup();
}

//Initialize all System Clocks,Gate settings and  used Power Resources
static void system_init(void)
{
    S3x_Clk_Enable(S3X_M4_S0_S3_CLK);
    S3x_Clk_Enable(S3X_M4_S4_S7_CLK);
    S3x_Clk_Enable(S3X_M4_S8_S11_CLK);
    S3x_Clk_Enable(S3X_M4_S12_S15_CLK);

    /* FPU settings ------------------------------------------------------------*/
    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */

    /* Configure Memory to support light sleep and retention mode */
    PMU->M4SRAM_SSW_LPMF = 0xFFFF;
    PMU->M4SRAM_SSW_LPMH_MASK_N = 0xFFFF;

    S3x_Clk_Enable(S3X_FB_16_CLK);
    S3x_Clk_Enable(S3X_FB_21_CLK);

    S3x_Clk_Enable(S3X_A1_CLK);
    S3x_Clk_Enable(S3X_CFG_DMA_A1_CLK);

    INTR_CTRL->OTHER_INTR = 0xffffff;
    DMA_SPI_MS->DMA_CTRL = DMA_CTRL_STOP_BIT;
}

static void ldo_init(void)
{
    /* LDO Settings */
    //AIP->LD0_30_CTRL_0 = 0x1ac; // LDO Enable       /* 0x1ac -> Vo =1.01V, imax = 7.2mA, LDO enabled. */
    //AIP->LD0_50_CTRL_0 = 0x1ac; // LDO Enable
    AIP->LD0_30_CTRL_0 = 0x28c; // LDO Enable       /* 0x28c -> Vo =1.15V, imax = 7.2mA, LDO enabled. */
    AIP->LD0_50_CTRL_0 = 0x28c; // LDO Enable
}
/*
* Setup the SPI master for Flash chip
*/
static void SPIM_Setup(void)
{
  
    //SPI master init for SPI flash
    spiFlashHandle.Init.ucFreq       = SPI_BAUDRATE_5MHZ; //above 5MHz does not work
    spiFlashHandle.Init.ucSPIInf     = SPI_4_WIRE_MODE;
    spiFlashHandle.Init.ucSSn        = SPI_SLAVE_1_SELECT;
    spiFlashHandle.Init.ulCLKPhase   = SPI_PHASE_1EDGE;
    spiFlashHandle.Init.ulCLKPolarity = SPI_POLARITY_LOW;
    spiFlashHandle.Init.ulDataSize   = SPI_DATASIZE_8BIT;
    spiFlashHandle.Init.ulFirstBit   = SPI_FIRSTBIT_MSB;
    spiFlashHandle.ucSPIx            = SPI1_MASTER_SEL;

    if(HAL_SPI_Init(&spiFlashHandle) != HAL_OK)
    {
        printf("HAL_SPI1_Init failed\r\n");
        configASSERT(0); //will hang here with out flash access
        return;
    }
    
    //make sure there is a Flash chip
    if(read_flash_id() == 0)
    {
       configASSERT(0); //will hang here if cannot read flash
    }
    
	return;
}
/*
* Setup the UART for 115200bps
*/
static void uart_setup(void)
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
    
	uart_init( uart_id, NULL, NULL, &uartObj);
}
