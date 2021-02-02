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
#include "ql_time.h"
#include "s3x_clock_hal.h"
#include "s3x_clock.h"
#include "s3x_pi.h"
#include "QL_Trace.h"
#include "dbg_uart.h"
#include "eoss3_hal_spi.h"

#include "cli.h"

#include "sec_debug.h"
#include "eoss3_hal_spi.h"
#include "spi_flash.h"
#include "h2d_protocol.h"
#include "ql_hostTask.h"

#if (FEATURE_USBSERIAL == 1)    
#include "eoss3_hal_fpga_usbserial.h"
#if (FEATURE_I2S_MASTER_CLKS == 1)
#include "usb2serial_i2sclk_bit_2020_11_19b.h" //this also has I2s
// length of axFPGABitStream array in bytes
int   axFPGABitStream_USB_I2S_length = sizeof(axFPGABitStream_USB_I2S);
#else
#include "gateware.h"
#endif
#endif
#include "eoss3_hal_uart.h"

#ifndef USBSERIAL_PRODUCTID
#define USBSERIAL_PRODUCTID     (0x6141)
#endif

extern const struct cli_cmd_entry my_main_menu[];
const struct cli_cmd_entry*  CLI_FSMConfigData = my_main_menu;

UINT8_t hw_rev;
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



#if DBG_FLAGS_ENABLE
uint32_t DBG_flags = DBG_flags_default;
#endif

#ifdef WATCHDOG_SUPPORT
extern void init_watchdog_reload_task(void);
#endif

extern void start_load_from_flash(void);   

#if defined(ENABLE_LOAD_FPGA) || defined(ENABLE_LOAD_FFE)
uint32_t scratch_1Kbyte_ram[256];
#endif

//neede for startup_EOSS3b.s asm file
void SystemInit(void)
{

}
extern void CreateHardfault(void);
extern void hardwareSetup_Chandalar();

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
 }    
 
//static void clock_gating(void)
//{
//    const UINT32_t kClockDividerGClockGating = (0x1 << 6);
//    int x;
//    for (x = 0; x != 100; x++) {
//        CRU->CLK_DIVIDER_CLK_GATING &= ~kClockDividerGClockGating;
//    }
//    CRU->CLK_DIVIDER_CLK_GATING |= kClockDividerGClockGating;
//    
//    CRU->AUDIO_MISC_SW_RST = CRU->AUDIO_MISC_SW_RST | (0x6);
//    CRU->AUDIO_MISC_SW_RST = CRU->AUDIO_MISC_SW_RST & (UINT32_t)(~(0x6));
//}

/* Handle for SPI transactions */
SPI_HandleTypeDef spi_m_handle;

int spi_master_init(uint32_t baud_rate)
{
	HAL_StatusTypeDef ret = HAL_OK;

	/* Initialize SPI */
	spi_m_handle.Init.ucFreq = baud_rate;	//SPI_BAUDRATE_1MHZ;
	spi_m_handle.Init.ucSPIInf = SPI_4_WIRE_MODE;
	spi_m_handle.Init.ucSSn = SPI_SLAVE_2_SELECT;
	spi_m_handle.Init.ulCLKPhase = SPI_PHASE_1EDGE;
	spi_m_handle.Init.ulCLKPolarity = SPI_POLARITY_LOW;
	spi_m_handle.Init.ulDataSize = SPI_DATASIZE_8BIT;
	spi_m_handle.Init.ulFirstBit = SPI_FIRSTBIT_MSB;
	spi_m_handle.ucSPIx = SPI1_MASTER_SEL;

	S3x_Clk_Enable(S3X_A1_CLK);
	S3x_Clk_Enable(S3X_CFG_DMA_A1_CLK);
	ret = HAL_SPI_Init(&spi_m_handle);
	if( HAL_OK != ret)
    {
		//printf("HAL_SPI1_Init failed\r\n");
		configASSERT(0);
	}

	spi_m_handle.Init.ucCmdType = CMD_NoResponse;
    return ret;
}

int main(void)
{

    SOFTWARE_VERSION_STR = "QORC-SDK-Host-App";
    
    qf_hardwareSetup();
    
    /* DCL will set time later */
    HAL_RTC_Init(0);

#if (FEATURE_USBSERIAL == 1)    
    S3x_Clk_Disable(S3X_FB_21_CLK);
    S3x_Clk_Disable(S3X_FB_16_CLK);
    
    S3x_Clk_Enable(S3X_A1_CLK);
    S3x_Clk_Enable(S3X_CFG_DMA_A1_CLK);
    
    // Use 0x6141 as the USB serial product ID (USB PID)
#if (FEATURE_I2S_MASTER_CLKS == 1)
    load_fpga(axFPGABitStream_USB_I2S_length,axFPGABitStream_USB_I2S);
    HAL_usbserial_i2s_init(false, false, USBSERIAL_PRODUCTID, 0x1A5BD); // Start USB serial not using interrupts
#else
    load_fpga(axFPGABitStream_length,axFPGABitStream);
    HAL_usbserial_init2(false, false, USBSERIAL_PRODUCTID);          // Start USB serial not using interrupts    
#endif    
    for (int i = 0; i != 4000000; i++) ;   // Give it time to enumerate
    
    /* Use the following APIs to send and receive data from USB-serial 
     * uart_tx_raw_buf ( UART_ID_USBSERIAL, txbuf, txbuflen );
     * uart_rx_raw_buf ( UART_ID_USBSERIAL, txbuf, txbuflen );
     */
char info_usbserial[] =  "\nUSB Serial init done \nThis is used for debugging Audio data only\n\n\n";   
uart_tx_raw_buf( UART_ID_USBSERIAL,info_usbserial, sizeof(info_usbserial)); 
    
#endif

    dbg_str("\n\n");
    dbg_str( "##########################\n");
    dbg_str( "Quicklogic Open Platform 2.0\n");
    dbg_str( "SW Version: ");
    dbg_str( SOFTWARE_VERSION_STR );
    dbg_str( "\n" );
    dbg_str( __DATE__ " " __TIME__ "\n" );
    dbg_str( "##########################\n\n");
#if (FEATURE_D2HPROTOCOL_HOST == 1)
    dbg_str("Using 1-wire D2H Protocol\n\n");
#else
    dbg_str("Using 4-pin  D2H Protocol\n\n");
#endif
    //ldo_init();
    nvic_init();

    spi_master_init(SPI_BAUDRATE_1MHZ);

    StartControlTask();
    /* Start the tasks and timer running. */
    vTaskStartScheduler();
    dbg_str("\n");
      
    while(1);
}

uint32_t getRoundedClkDivider(uint32_t sysClk, uint32_t domainClk)
{
    uint32_t clkDiv;
    float computeClk;

    computeClk = (float) sysClk / (float) domainClk;
    computeClk += 0.5; //round it up
    clkDiv = (uint32_t)computeClk;

    return clkDiv;
}
uint32_t getExactClkDivider(uint32_t sysClk, uint32_t domainClk)
{
    uint32_t clkDiv, computeClk;

    clkDiv = sysClk / domainClk;
    computeClk = clkDiv * domainClk;

    //This function is called to make sure, the clks are exact multiple
    //So, if not, just hang until fixed
    if(computeClk != sysClk)
    {
        while(1){ };
    }

    return clkDiv;
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


