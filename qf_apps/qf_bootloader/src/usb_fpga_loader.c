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
*    File   : usb_fpga_loader.c 
*    Purpose: This file has function to load USB FPGA image 
*             from the Flash and wait for reset 
*                                                          
*=========================================================*/

#include "Fw_global_config.h"
#include "eoss3_dev.h"
#include "eoss3_hal_spi.h"
#include "spi_flash.h"
#include "crc32.h"
#include "bootloader_defines.h"
#include "s3x_clock_hal.h"
#include "s3x_clock.h"
#include "s3x_pi.h"
#include "fpga_loader.h"
#include "eoss3_hal_pads.h"
#include "eoss3_hal_pad_config.h"
#include "dbg_uart.h"

extern uint32_t image_metadata[FLASH_APP_META_SIZE/4];
extern void read_flash(unsigned char *start_address, int length, unsigned char *destination);
extern void toggle_downloading_led(int toggle_time_msec);
/*
* The USB FPGA IP directly accesses the LEDs.
*/
static void setup_led_fpga_pins(void)
{
    // Configure led pins for FPGA 
    PadConfig padcfg;
    
    //output pins 
    padcfg.ucCtrl = PAD_CTRL_SRC_FPGA;
    padcfg.ucMode = PAD_MODE_OUTPUT_EN;
    padcfg.ucPull = PAD_NOPULL;
    padcfg.ucDrv = PAD_DRV_STRENGHT_4MA;
    padcfg.ucSpeed = PAD_SLEW_RATE_SLOW;
    padcfg.ucSmtTrg = PAD_SMT_TRIG_DIS;

    // Pad 18 -- blue LED 
    padcfg.ucPin = PAD_18;
    padcfg.ucFunc = PAD18_FUNC_SEL_FBIO_18;
    HAL_PAD_Reconfig(&padcfg);
    
    //Pad 21 -- green LED 
    padcfg.ucPin = PAD_21;
    padcfg.ucFunc = PAD21_FUNC_SEL_FBIO_21;
    HAL_PAD_Reconfig(&padcfg);
    
    //Pad 22 -- red LED 
    padcfg.ucPin = PAD_22;
    padcfg.ucFunc = PAD22_FUNC_SEL_FBIO_22;
    HAL_PAD_Reconfig(&padcfg);
}

/*
* The USB FPGA IP directly accesses the SPI Flash.
* For that the SPI master pins have to reconfigured to be used
* by FPGA before loading the FPGA.
*/
static void setup_spim_fpga_pins(void)
{
    // Configure SPI master the pads
    PadConfig padcfg;
    
    //output pins 
    padcfg.ucCtrl = PAD_CTRL_SRC_FPGA;
    padcfg.ucMode = PAD_MODE_OUTPUT_EN;
    padcfg.ucPull = PAD_NOPULL;
    padcfg.ucDrv = PAD_DRV_STRENGHT_4MA;
    padcfg.ucSpeed = PAD_SLEW_RATE_SLOW;
    padcfg.ucSmtTrg = PAD_SMT_TRIG_DIS;

    // Pad 34 -- SPI Master CLK
    padcfg.ucPin = PAD_34;
    padcfg.ucFunc = PAD34_FUNC_SEL_FBIO_34;
    HAL_PAD_Reconfig(&padcfg);
    
    //Pad 38 -- SPI Master MOSI
    padcfg.ucPin = PAD_38;
    padcfg.ucFunc = PAD38_FUNC_SEL_FBIO_38;
    HAL_PAD_Reconfig(&padcfg);
    
    //Pad 39 -- SPI Master SSn1
    padcfg.ucPin = PAD_39;
    padcfg.ucFunc = PAD39_FUNC_SEL_FBIO_39;
    HAL_PAD_Reconfig(&padcfg);
    
    //input pins
    padcfg.ucMode = PAD_MODE_INPUT_EN;
    //padcfg.ucPull = PAD_PULLUP;

    padcfg.ucDrv = 0;
    padcfg.ucSpeed = 0;
    padcfg.ucSmtTrg = 0;
    
    //Pad 36 -- SPI Master MISO
    padcfg.ucPin = PAD_36;
    padcfg.ucFunc = PAD36_FUNC_SEL_FBIO_36;
    HAL_PAD_Reconfig(&padcfg);
    
  return;
}

/*
* This computes the CRC32 on the USB FPGA code loaded into RAM.
* checks against stored CRC32 in the USB FPGA Metadata sector.
*/
int check_fpga_crc(int image_size, uint32_t expected_crc)
{
  uint32_t image_crc32 = 0xFFFFFFFF;
  int size = image_size;
  const unsigned char *bufPtr;
  
  //FPGA image is loaded immediatle after the 64K Bootloader
  bufPtr = APP_AFTER_64K_RAM_START;
  image_crc32 = xcrc32 (bufPtr, size, image_crc32);
  
  if(image_crc32 != expected_crc)
  {
    dbg_str("USB FPGA image CRC mismatch \n");
    return BL_ERROR;
  }
  return BL_NO_ERROR;
}
/*
* This function loads USB FPGA image into RAM immediateely after the Bootloader
* The Size and CRC32 are checked using the image Metadata sector values
* If they pass, the FPGA is loaded and wait for reset button to be pressed. 
*/
int load_usb_flasher(void)
{
  unsigned char *bufPtr;
  uint32_t image_crc, image_size;
  
  //get the meta data sector for USB FPGA 
  bufPtr = (unsigned char *)image_metadata; 
  read_flash((unsigned char *)FLASH_USBFPGA_META_ADDRESS, FLASH_USBFPGA_META_SIZE, bufPtr);
  image_crc = image_metadata[0];
  image_size = image_metadata[1];
  if(image_size > FLASH_USBFPGA_SIZE)
  {
    dbg_str("USB FPGA Image size exceeded bootable size \n");
    return BL_ERROR;
  }
  
  //FPGA image is loaded immediately after the 64K Bootloader
  bufPtr = APP_AFTER_64K_RAM_START;
  read_flash((unsigned char *)FLASH_USBFPGA_ADDRESS, image_size, bufPtr);
  
  //check crc
  if(check_fpga_crc(image_size, image_crc) == BL_ERROR)
    return BL_ERROR;
  
  //change the SPI master pins for use by FPGA
  setup_spim_fpga_pins();
  setup_led_fpga_pins();

  S3x_Clk_Disable(S3X_FB_21_CLK);
  S3x_Clk_Disable(S3X_FB_16_CLK);

  S3x_Clk_Enable(S3X_A1_CLK);
  S3x_Clk_Enable(S3X_CFG_DMA_A1_CLK);

  //load the FPGA from RAM
  load_fpga(image_size, (uint32_t *)bufPtr);

  S3x_Clk_Set_Rate(S3X_FB_21_CLK, 48*1000*1000);
  S3x_Clk_Set_Rate(S3X_FB_16_CLK, 12*1000*1000);
  S3x_Clk_Enable(S3X_FB_21_CLK);
  S3x_Clk_Enable(S3X_FB_16_CLK);

  // remind what to do when done programming
  dbg_str("FPGA Programmed\nPresss Reset button after flashing ..\n");
  //wait for the reset button to be pressed
  while(1)
  {
     vTaskDelay(5*1000);

     /* Check FPGA interrupt 0 status */
     int int0status = INTR_CTRL->FB_INTR_RAW & 1;
     if (int0status)
     {
         /* "boot sent" command received from USB-to-Flash reset the CPU  */
         dbg_str("Boot command received resetting...");
         NVIC_SystemReset();
     }
  }

  //return BL_ERROR;
}
