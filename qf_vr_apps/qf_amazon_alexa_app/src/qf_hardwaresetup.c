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

void isr_init_hwp(void);
void system_init(void);
static void uart_pad_setup(void);
//static void i2c_init_for_m4();
//static void nRF51_pad_setup(void);
static void setup_debugpins(void);
static void Audio_pad_setup(void);
#if ENABLE_I2S_TX_SLAVE
static void I2STx_slave_pad_setup(void);
#endif
void qf_hardwareSetup(void) {

    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */
    
    SCnSCB->ACTLR |= SCnSCB_ACTLR_DISDEFWBUF_Msk;
    S3x_pwrcfg_init();
    system_init();
    uart_pad_setup();
    Audio_pad_setup();
#if ENABLE_I2S_TX_SLAVE
    I2STx_slave_pad_setup();
#endif

    //Setup BLE interface is through SPI slave
//    nRF51_pad_setup();

    //setup pins to be able to use debugger after flash boot
    setup_debugpins();

    // Configure rest of the pads
    PadConfig padcfg;

    //output pins
    padcfg.ucCtrl = PAD_CTRL_SRC_A0; //PAD_CTRL_SRC_OTHER ;//PAD_CTRL_SRC_A0;
    padcfg.ucMode = PAD_MODE_OUTPUT_EN;
    padcfg.ucPull = PAD_NOPULL;
    padcfg.ucDrv = PAD_DRV_STRENGHT_4MA;
    padcfg.ucSpeed = PAD_SLEW_RATE_FAST;
    padcfg.ucSmtTrg = PAD_SMT_TRIG_DIS;
    
    // Pad 6 -- Gpio0
    //padcfg.ucPin = PAD_6;
    //padcfg.ucFunc = PAD6_FUNC_SEL_GPIO_0;
    //HAL_PAD_Config(&padcfg);
    
    
    // Pad 21 -- Gpio5
    padcfg.ucPin = PAD_21;
    padcfg.ucFunc = PAD21_FUNC_SEL_GPIO_5;
    HAL_PAD_Config(&padcfg);
    
    // Pad 22 -- Gpio6
    //padcfg.ucPin = PAD_22;
    //padcfg.ucFunc = PAD22_FUNC_SEL_GPIO_6;
    //HAL_PAD_Config(&padcfg);

    
    // Pad 34 -- SPI Master CLK
    padcfg.ucPin = PAD_34;
    padcfg.ucFunc = PAD34_FUNC_SEL_SPIm_CLK;
    HAL_PAD_Config(&padcfg);

    //Pad 35 -- SPI Master SSN3
    padcfg.ucPin = PAD_35;
    padcfg.ucFunc = PAD35_FUNC_SEL_SPIm_SSn3;
    HAL_PAD_Config(&padcfg);

    //Pad 38 -- SPI Master MOSI
    padcfg.ucPin = PAD_38;
    padcfg.ucFunc = PAD38_FUNC_SEL_SPIm_MOSI;
    HAL_PAD_Config(&padcfg);

    //Pad 39 -- SPI Master SSn1
    padcfg.ucPin = PAD_39;
    padcfg.ucFunc = PAD39_FUNC_SEL_SPIm_SSn1;
    HAL_PAD_Config(&padcfg);
     
    //Pad 41 -- I2S Bit Clk
    padcfg.ucPin = PAD_41;
    padcfg.ucFunc = PAD41_FUNC_SEL_FBIO_41;
    HAL_PAD_Config(&padcfg);
    
    //Pad 42 -- I2S Word Clk
    padcfg.ucPin = PAD_42;
    padcfg.ucFunc = PAD42_FUNC_SEL_FBIO_42;
    HAL_PAD_Config(&padcfg);
    
/////input pins////////////////////////////////////////////////////////////
    padcfg.ucMode = PAD_MODE_INPUT_EN;
    padcfg.ucPull = PAD_PULLUP;

    //Pad 36 -- SPI Master MISO
    padcfg.ucPin = PAD_36;
    padcfg.ucFunc = PAD36_FUNC_SEL_SPIm_MISO;
    HAL_PAD_Config(&padcfg);

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

#if USE_FPGA_UART
	brate = BAUD_9600;
	uart_id = UART_ID_FPGA;
#else
    uart_id = UART_ID_HW;
#ifndef BOOT_LOADER
	brate = BAUD_115200;
#else
	brate = BAUD_460800; //for boot loader
#endif
#endif
	uartObj.baud = brate;
	uartObj.wl = WORDLEN_8B;
	uartObj.parity = PARITY_NONE;
	uartObj.stop = STOPBITS_1;
	uartObj.mode = TX_RX_MODE;
	uartObj.hwCtrl = HW_FLOW_CTRL_DISABLE;
	uartObj.intrMode = UART_INTR_ENABLE;
    uartHandlerUpdate(uart_id,&uartObj);
    
    PadConfig padConfigTx;
    padConfigTx.ucPin = PAD_44;
    padConfigTx.ucFunc = PAD44_FUNC_SEL_UART_TXD;
    padConfigTx.ucCtrl = PAD_CTRL_SRC_A0;
    padConfigTx.ucMode = PAD_MODE_OUTPUT_EN;
    padConfigTx.ucPull = PAD_NOPULL;
    padConfigTx.ucDrv = PAD_DRV_STRENGHT_4MA;
    padConfigTx.ucSpeed = PAD_SLEW_RATE_SLOW;
    padConfigTx.ucSmtTrg = PAD_SMT_TRIG_DIS; 
    PadConfig padConfigRx;
    padConfigRx.ucPin = PAD_45;                     // Options: 14, 16, 25, or 45
    padConfigRx.ucFunc = PAD45_FUNC_SEL_UART_RXD;
    padConfigRx.ucCtrl = PAD_CTRL_SRC_A0;
    padConfigRx.ucMode = PAD_MODE_INPUT_EN;
    padConfigRx.ucPull = PAD_NOPULL;
	uart_init( uart_id, &padConfigTx, &padConfigRx, &uartObj);
}
#if 0 //SPI slave does not work if pads are setup. Just for reference.
//Configure pads to interface nRF51822 BLE chip with S3 SPI slave
//A total of 4 pads are used for the interface. But the another Pad (PAD_43)
//for Interrupt from AP is set in the main.
//Note: PAD_17 should not be used by FPGA for NEUROMEM DCO.
void nRF51_pad_setup(void)
{

    PadConfig xPadConf;

    //setup input pins
    xPadConf.ucMode = PAD_MODE_INPUT_EN;
    xPadConf.ucCtrl = PAD_CTRL_SRC_OTHER;
    xPadConf.ucDrv = PAD_DRV_STRENGHT_2MA;
    xPadConf.ucSpeed = PAD_SLEW_RATE_SLOW;
    xPadConf.ucSmtTrg = PAD_SMT_TRIG_DIS;

    //Pad 16 - SPI Slave SCLK
    xPadConf.ucPin = PAD_16;
    xPadConf.ucFunc = PAD16_FUNC_SEL_SPIs_CLK;
    xPadConf.ucPull = PAD_NOPULL;
    HAL_PAD_Config(&xPadConf);

    //Pad 19 - SPI Slave MOSI
    xPadConf.ucPin = PAD_19;
    xPadConf.ucFunc = PAD19_FUNC_SEL_SPIs_MOSI;
    xPadConf.ucPull = PAD_PULLDOWN;
    HAL_PAD_Config(&xPadConf);

    //Pad 20 - SPI Slave SSn
    xPadConf.ucPin = PAD_20;
    xPadConf.ucFunc = PAD20_FUNC_SEL_SPIs_SSn;
    xPadConf.ucPull = PAD_PULLUP;
    HAL_PAD_Config(&xPadConf);

    //setup output pins
    //Pad 17 - SPI Slave MISO
    xPadConf.ucPin = PAD_17;
    xPadConf.ucFunc = PAD17_FUNC_SEL_SPIs_MISO;
    xPadConf.ucMode = PAD_MODE_OUTPUT_EN;

    xPadConf.ucPull = PAD_NOPULL;
    xPadConf.ucDrv = PAD_DRV_STRENGHT_4MA;
    HAL_PAD_Config(&xPadConf);

    return;
}
#endif

//PAD 14, 15 should be setup as Debugger mode pins SWD to be able to use
//JTAG debugger after boot from flash
static void setup_debugpins(void)
{

    PadConfig xPadConf;

    //setup input pins
    xPadConf.ucMode = PAD_MODE_INPUT_EN;
    xPadConf.ucCtrl = PAD_CTRL_SRC_OTHER;
    xPadConf.ucDrv = PAD_DRV_STRENGHT_4MA;
    xPadConf.ucSpeed = PAD_SLEW_RATE_SLOW;
    xPadConf.ucSmtTrg = PAD_SMT_TRIG_DIS;

    // PAD_14 - S3_SW_DP_CLK
    xPadConf.ucPin = PAD_14;
    xPadConf.ucFunc = PAD14_FUNC_SEL_SW_DP_CLK;
    xPadConf.ucPull = PAD_NOPULL;
    HAL_PAD_Config(&xPadConf);

    //PAD_15 - S3_SW_DP_IO
    xPadConf.ucPin = PAD_15;
    xPadConf.ucFunc = PAD15_FUNC_SEL_SW_DP_IO;
    xPadConf.ucPull = PAD_PULLUP;
    HAL_PAD_Config(&xPadConf);

    return;

}

/* Select one of the 3 Pad configurarions for PDM Audio */
#if PDM_PAD_28_29
static void Audio_pad_setup(void)
{
  PadConfig xPadConf;

  IO_MUX->PDM_DATA_SELE = 0x02;   // 1 for pad10, 2 for pad28
  /* Configuring Pad_29 as PDM CLK Out*/
  xPadConf.ucPin = PAD_29;
  xPadConf.ucFunc = PAD29_FUNC_SEL_PDM_CKO;
  xPadConf.ucCtrl = PAD_CTRL_SRC_A0;
  xPadConf.ucMode = PAD_MODE_OUTPUT_EN;
  xPadConf.ucPull = PAD_NOPULL;
  xPadConf.ucDrv = PAD_DRV_STRENGHT_4MA;
  xPadConf.ucSpeed = PAD_SLEW_RATE_SLOW;
  xPadConf.ucSmtTrg = PAD_SMT_TRIG_DIS;
  
  HAL_PAD_Config(&xPadConf);
  
  /*Configuring Pad_28 as PDM Data In*/
  xPadConf.ucPin = PAD_28;
  xPadConf.ucFunc = PAD28_FUNC_SEL_PDM_DIN;
  xPadConf.ucCtrl = PAD_CTRL_SRC_A0;
  xPadConf.ucMode = PAD_MODE_INPUT_EN;
  //xPadConf.ucPull = PAD_NOPULL;
  //xPadConf.ucDrv = PAD_DRV_STRENGHT_4MA;
  //xPadConf.ucSpeed = PAD_SLEW_RATE_SLOW;
  //xPadConf.ucSmtTrg = PAD_SMT_TRIG_DIS;
  
  HAL_PAD_Config(&xPadConf);
}
#endif /* PDM_PAD_28_29 */
#if PDM_PAD_8_10
static void Audio_pad_setup(void)
{
  PadConfig xPadConf;
  
  /* PDM PAD 10 and PAD 8*/
  // Select pad10 or 28 for pdm din
  IO_MUX->PDM_DATA_SELE = 0x01;   // 1 for pad10, 2 for pad28
  
  /* Configuring Pad_8 as PDM Clk out*/
  xPadConf.ucPin = PAD_8;
  xPadConf.ucFunc = PAD8_FUNC_PDM_CKO;
  xPadConf.ucCtrl = PAD_CTRL_SRC_A0;
  xPadConf.ucMode = PAD_MODE_OUTPUT_EN;
  xPadConf.ucPull = PAD_NOPULL;
  xPadConf.ucDrv = PAD_DRV_STRENGHT_4MA;
  xPadConf.ucSpeed = PAD_SLEW_RATE_SLOW;
  xPadConf.ucSmtTrg = PAD_SMT_TRIG_DIS;
  
  HAL_PAD_Config(&xPadConf);
  
  
  //-------------------------------------------------------------------------------------------
  // IO_MUX pad controls for audio
  // Pad 10 or pad 28 is selected for i2s data in or pdm din
  // Pad 10 or pad 28 is configured as input for i2s data in or pdm din
  //-------------------------------------------------------------------------------------------
  
  /* Configuring PAD_10 as PDM Data In*/
  xPadConf.ucPin = PAD_10;
  xPadConf.ucFunc = PAD10_FUNC_SEL_PDM_DIN;
  xPadConf.ucCtrl = PAD_CTRL_SRC_A0;
  xPadConf.ucMode = PAD_MODE_INPUT_EN;
  //xPadConf.ucPull = PAD_NOPULL;
  //xPadConf.ucDrv = PAD_DRV_STRENGHT_4MA;
  //xPadConf.ucSpeed = PAD_SLEW_RATE_SLOW;
  //xPadConf.ucSmtTrg = PAD_SMT_TRIG_DIS;
  
  HAL_PAD_Config(&xPadConf);
}
#endif /* PDM_PAD_8_10 */
#if VOICE_AP_BYPASS_MODE
//void VoiceAPbypassPadCtrl(void)
static void Audio_pad_setup(void)
{
  PadConfig padcfg;
  /* Select for PAD 38 */
  IO_MUX->PDM_CLKIN_SEL = 0x01;
  //S3B_AP_PDM_STAT_O
  //	padcfg.ucPin = PAD_34;
  //	padcfg.ucFunc = PAD34_FUNC_SEL_AP_PDM_STAT_O;
  //	padcfg.ucCtrl = PAD_CTRL_SRC_A0;
  //	padcfg.ucMode = PAD_MODE_OUTPUT_EN;
  //	padcfg.ucPull = PAD_NOPULL;
  //	padcfg.ucDrv = PAD_DRV_STRENGHT_4MA;
  //	padcfg.ucSpeed = PAD_SLEW_RATE_SLOW;
  //	padcfg.ucSmtTrg = PAD_SMT_TRIG_DIS;
  //
  //	HAL_PAD_Config(&padcfg);
  
  //S3B_AP_PDM_CKO_IN
  padcfg.ucPin = PAD_38;
  padcfg.ucFunc = PAD38_FUNC_SEL_AP_PDM_CKO_IN;
  padcfg.ucCtrl = PAD_CTRL_SRC_A0;
  padcfg.ucMode = PAD_MODE_INPUT_EN;
  padcfg.ucPull = PAD_NOPULL;
  padcfg.ucDrv = PAD_DRV_STRENGHT_4MA;
  padcfg.ucSpeed = PAD_SLEW_RATE_SLOW;
  padcfg.ucSmtTrg = PAD_SMT_TRIG_DIS;
  
  HAL_PAD_Config(&padcfg);
  
  //S3B_AP_PDM_IO
  padcfg.ucPin = PAD_39;
  padcfg.ucFunc = PAD39_FUNC_SEL_AP_PDM_IO;
  padcfg.ucCtrl = PAD_CTRL_SRC_A0;
  padcfg.ucMode = PAD_MODE_OUTPUT_EN;
  //padcfg.ucPull = PAD_NOPULL;
  //padcfg.ucDrv = PAD_DRV_STRENGHT_4MA;
  //padcfg.ucSpeed = PAD_SLEW_RATE_SLOW;
  //padcfg.ucSmtTrg = PAD_SMT_TRIG_DIS;
  
  HAL_PAD_Config(&padcfg);
}

#endif /* VOICE_AP_BYPASS_MODE */

#if ENABLE_I2S_TX_SLAVE
static void I2STx_slave_pad_setup(void)
{
    PadConfig xPadConf;
    /* WD_CLK left/right sync clock */
    xPadConf.ucPin = PAD_23;
    xPadConf.ucFunc = PAD23_FUNC_SEL_AP_I2S_WD_CLK_IN;
    xPadConf.ucCtrl = PAD_CTRL_SRC_A0;
    xPadConf.ucMode = PAD_MODE_INPUT_EN;
    xPadConf.ucPull = PAD_PULLUP;
    xPadConf.ucDrv = PAD_DRV_STRENGHT_4MA;
    xPadConf.ucSpeed = PAD_SLEW_RATE_SLOW;
    xPadConf.ucSmtTrg = PAD_SMT_TRIG_DIS;
    HAL_PAD_Config(&xPadConf);

    /* I2S DOUT for Tx */
    xPadConf.ucPin = PAD_24;
    xPadConf.ucFunc = PAD24_FUNC_SEL_AP_I2S_DOUT;
    xPadConf.ucCtrl = PAD_CTRL_SRC_A0;
    xPadConf.ucMode = PAD_MODE_OUTPUT_EN;
    xPadConf.ucPull = PAD_PULLUP;
    xPadConf.ucDrv = PAD_DRV_STRENGHT_4MA;
    xPadConf.ucSpeed = PAD_SLEW_RATE_SLOW;
    xPadConf.ucSmtTrg = PAD_SMT_TRIG_DIS;
    HAL_PAD_Config(&xPadConf);

    /* I2S CLK IN from master */
    xPadConf.ucPin = PAD_31;
    xPadConf.ucFunc = PAD31_FUNC_SEL_AP_I2S_CLK_IN;
    xPadConf.ucCtrl = PAD_CTRL_SRC_A0;
    xPadConf.ucMode = PAD_MODE_INPUT_EN;
    xPadConf.ucPull = PAD_PULLUP;
    xPadConf.ucDrv = PAD_DRV_STRENGHT_4MA;
    xPadConf.ucSpeed = PAD_SLEW_RATE_SLOW;
    xPadConf.ucSmtTrg = PAD_SMT_TRIG_DIS;
    HAL_PAD_Config(&xPadConf);
}
#endif //#if ENABLE_I2S_TX_SLAVE


