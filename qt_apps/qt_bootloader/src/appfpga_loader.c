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
*    File   : appfpga_loader.c 
*    Purpose: This file has function to load App FPGA image 
*             from the Flash
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
#include "eoss3_hal_fpga_usbserial.h"
#include "dbg_uart.h"

extern uint32_t image_metadata[FLASH_APP_META_SIZE/4];
extern void read_flash(unsigned char *start_address, int length, unsigned char *destination);
extern void toggle_downloading_led(int toggle_time_msec);

/*
* This computes the CRC32 on the USB FPGA code loaded into RAM.
* checks against stored CRC32 in the USB FPGA Metadata sector.
*/
int check_appfpga_crc(int image_size, uint32_t expected_crc)
{
  uint32_t image_crc32 = 0xFFFFFFFF;
  int size = image_size;
  const unsigned char *bufPtr;
  
  //FPGA image is loaded immediatle after the 64K Bootloader
  bufPtr = APP_AFTER_64K_RAM_START;
  image_crc32 = xcrc32 (bufPtr, size, image_crc32);
  
  if(image_crc32 != expected_crc)
  {
    dbg_str("Application FPGA image CRC mismatch \n");
    return BL_ERROR;
  }
  return BL_NO_ERROR;
}
/*
* This function loads USB FPGA image into RAM immediateely after the Bootloader
* The Size and CRC32 are checked using the image Metadata sector values
* If they pass, the FPGA is loaded and wait for reset button to be pressed. 
*/
int load_appfpga(void)
{
  unsigned char *bufPtr;
  uint32_t image_crc, image_size;

  uint32_t* fpga_bin_header;
  uint32_t fpga_bin_header_size;
  uint32_t fpga_bin_version;
  uint32_t* fpga_bitstream_ptr;
  uint32_t fpga_bitstream_size;
  uint32_t fpga_bitstream_crc;
  uint32_t* fpga_meminit_ptr;
  uint32_t fpga_meminit_size;
  uint32_t fpga_meminit_crc;
  uint32_t* fpga_iomux_ptr;
  uint32_t fpga_iomux_size;
  uint32_t fpga_iomux_crc;
  
  //get the meta data sector for USB FPGA 
  bufPtr = (unsigned char *)image_metadata; 
  read_flash((unsigned char *)FLASH_APPFPGA_META_ADDRESS, FLASH_APPFPGA_META_SIZE, bufPtr);
  image_crc = image_metadata[0];
  image_size = image_metadata[1];
  if(image_size > FLASH_APPFPGA_SIZE)
  {
    dbg_str("App FPGA Image size exceeded bootable size \n");
    return BL_ERROR;
  }
  
  //FPGA image is loaded immediately after the 64K Bootloader
  bufPtr = APP_AFTER_64K_RAM_START;
  read_flash((unsigned char *)FLASH_APPFPGA_ADDRESS, image_size, bufPtr);
  
  //check crc
  if(check_appfpga_crc(image_size, image_crc) == BL_ERROR)
    return BL_ERROR;

  S3x_Clk_Disable(S3X_FB_21_CLK);
  S3x_Clk_Disable(S3X_FB_16_CLK);

  S3x_Clk_Enable(S3X_A1_CLK);
  S3x_Clk_Enable(S3X_CFG_DMA_A1_CLK);

  // The FPGA bin now contains HEADER + BITSTREAM + MEMINIT + IOMUX
  // HEADER = 8 x 4B
  fpga_bin_header = (uint32_t*)bufPtr;
  fpga_bin_header_size = 8*4; // 32B

  fpga_bin_version = *(fpga_bin_header);
  
  fpga_bitstream_size = *(fpga_bin_header + 1);
  fpga_bitstream_crc = *(fpga_bin_header + 2);
  
  fpga_meminit_size = *(fpga_bin_header + 3);
  fpga_meminit_crc = *(fpga_bin_header + 4);

  fpga_iomux_size = *(fpga_bin_header + 5);
  fpga_iomux_crc = *(fpga_bin_header + 6);

  // word 7 of the fpga bin header is reserved for future use.

  fpga_bitstream_ptr = fpga_bin_header + 8;
  fpga_meminit_ptr = fpga_bitstream_ptr + (fpga_bitstream_size/4);
  fpga_iomux_ptr = fpga_meminit_ptr + (fpga_meminit_size/4);

//   dbg_str("\r\n");
//   dbg_hex32(fpga_bin_version);dbg_str("\r\n");
//   dbg_int(fpga_bitstream_size);dbg_str("\r\n");
//   dbg_hex32(fpga_bitstream_crc);dbg_str("\r\n");
//   dbg_int(fpga_meminit_size);dbg_str("\r\n");
//   dbg_hex32(fpga_meminit_crc);dbg_str("\r\n");
//   dbg_int(fpga_iomux_size);dbg_str("\r\n");
//   dbg_hex32(fpga_iomux_crc);dbg_str("\r\n");dbg_str("\r\n");
  
  load_fpga_with_mem_init(fpga_bitstream_size, fpga_bitstream_ptr, fpga_meminit_size, fpga_meminit_ptr);
  fpga_iomux_init(fpga_iomux_size, fpga_iomux_ptr);

  S3x_Clk_Enable(S3X_FB_21_CLK);                          // Start FPGA clock
  S3x_Clk_Enable(S3X_FB_16_CLK);

  dbg_str("Application FPGA Loaded\r\n");

  return BL_NO_ERROR;
}
