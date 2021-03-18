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
*    File   : m4_app_loader.c 
*    Purpose: This file has function to load M4 App 
*             from the Flash and reboot	            
*                                                          
*=========================================================*/

#include "Fw_global_config.h"
#include "eoss3_dev.h"
#include "eoss3_hal_spi.h"
#include "spi_flash.h"
#include "crc32.h"
#include "bootloader_defines.h"
#include "dbg_uart.h"

/*
* Important: The following function must be kept in a region where copying
* does not overwrite BootLoder code. So, protected by the pragma for absolute
* address location at the end of 512Kbyte RAM. Same is done for eclipse project
* also using attributes.
*
*/
/*!
 * \fn		static void move_m4app_img(unsigned int size)
 * \brief	This M4 loader function will overwrite the BL and Issue the Reset.
 * \param		size ---	size of m4 binary
 */

#ifdef __IAR_BUILD
#pragma default_function_attributes = @ "LOADER_FUNC"
static void move_m4app_img(unsigned int size)
#else
__attribute__((section(".LOADER_FUNC"))) void move_m4app_img(unsigned int size)
#endif
{

    uint8_t *src = APP_FIRST_64K_RAM_START;
    uint8_t *dst = (uint8_t*)APP_START_OFFSET;

    __disable_irq();
    while (size--) {
        *dst++ = *src++;
    }
    // TODO : Inline NVIC_DebugReset(); & remove below code
    __DSB(); /* Ensure all outstanding memory accesses included
                  * buffered write are completed before reset
          */
    SCB->AIRCR  = ((0x5FA << SCB_AIRCR_VECTKEY_Pos) | (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
        SCB_AIRCR_VECTRESET_Msk);
    __DSB(); /* Ensure completion of memory access */
    while(1); /* control will never reach here */
}

#ifdef __IAR_BUILD
#pragma default_function_attributes =
#endif

/*
* This reads into the memory from the given Flash address.
*
*/
void read_flash(unsigned char *start_address, int length, unsigned char *destination)
{
  int count = 0;
  int readcount;
  
  while(count < length)
  {
    //read in chunks of 4K
    readcount = length - count;
    if(readcount >= 4*1024)
      readcount = 4*1024;
    if(spiflash_read_fast((uint32_t)(start_address + count), readcount, (void *)(destination + count), NULL) != FlashOperationSuccess)
    {
        dbg_str("Error reading Flash memory into RAM\r\n");
    }
    count += readcount;
  }
 
   return;
}
/* the CRC and Size of the image are stored in a sector */
uint32_t image_metadata[FLASH_APP_META_SIZE/4];
/*
* This computes the CRC32 on the M4 App code loaded into RAM.
* checks against stored CRC32 in the App Metadata sector.
*/
int check_app_crc(int image_size, uint32_t expected_crc)
{
  uint32_t app_crc32 = 0xFFFFFFFF;
  int size = image_size;
  const unsigned char *bufPtr = APP_FIRST_64K_RAM_START;
  
  //Compute if App size is less than 64K
  bufPtr = APP_FIRST_64K_RAM_START;
  if(size >=  FLASH_BOOTLOADER_SIZE)
    size = FLASH_BOOTLOADER_SIZE;
  app_crc32 = xcrc32 (bufPtr, size, app_crc32);
  
  //If more than 64K, compute for the rest 
  if(size < image_size)
  {
    size = image_size - size;
    bufPtr = APP_AFTER_64K_RAM_START;
    app_crc32 = xcrc32 (bufPtr, size, app_crc32);
  }
  if(app_crc32 != expected_crc)
  {
    dbg_str("M4 App CRC mismatch \n");
    return BL_ERROR;
  }
  return BL_NO_ERROR;
}
/*
* This function loads M4 App starting at the given address.
* It copies the first (64K) at the endof the RAM from (512K  - 64K -Stack) = 440K .
* It loads the data from (64K) address until App size from 0x0001_0000 to 0x0001_0000 + size.
* So, This limits the maximum App image size to 440K only.
*/
int load_m4app(void)
{
  int size;
  unsigned char *bufPtr;
  uint32_t app_crc, app_size;
  unsigned char *flash_start = (unsigned char *)FLASH_APP_ADDRESS;
    
  //first get the meta data sector for App
  bufPtr = (unsigned char *)image_metadata; 
  read_flash((unsigned char *)FLASH_APP_META_ADDRESS, FLASH_APP_META_SIZE, bufPtr);
  app_crc = image_metadata[0];
  app_size = image_metadata[1];
  if(app_size > FLASH_APP_SIZE)
  {
    dbg_str("M4 App size exceeded bootable size \n");
    return BL_ERROR;
  }
    
  //from flash copy the first 64K of M4 code at the end of the RAM 
  bufPtr = APP_FIRST_64K_RAM_START;
  read_flash(flash_start, FLASH_BOOTLOADER_SIZE, bufPtr);
  
  //copy the rest of the M4 code starting from 64K 
  flash_start += FLASH_BOOTLOADER_SIZE;
  bufPtr = APP_AFTER_64K_RAM_START;
  size = app_size - FLASH_BOOTLOADER_SIZE; 
  if(size > 0)
    read_flash(flash_start, size, bufPtr);
  
  //check crc
  if(check_app_crc(app_size, app_crc) == BL_ERROR)
    return BL_ERROR;
  
  //Always move the first 64K to start address and reset
  //Note: Even when the size is less than 64K this will copy 64K
  move_m4app_img(FLASH_BOOTLOADER_SIZE);
  
  return BL_ERROR; //since it should never reach this 
}
