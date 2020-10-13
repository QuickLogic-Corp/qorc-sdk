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
 *    File   : bootloader_defines.h                                
 *    Purpose: 	            
 *                                                          
 *=========================================================*/
#ifndef __BL_DEFINES_H
#define __BL_DEFINES_H

#include "flash_defines.h"

#define BL_STACK_N_LOADER_SIZE      0x02000  //last 8K bytes of RAM has copy routine

/* copy the first 64K of App at the end of the 512K RAM. It will be moved only if CRC checks */
#define APP_FIRST_64K_RAM_START      ((unsigned char *)(0x80000 - BL_STACK_N_LOADER_SIZE - FLASH_BOOTLOADER_SIZE)) 
#define APP_AFTER_64K_RAM_START      ((unsigned char *)(FLASH_BOOTLOADER_SIZE))


/* This is where App Execution Start address is after loaded in M4 memory */
#define APP_START_OFFSET              0x0000000 

//define the type of errors for BootLoader functions
typedef enum Srec_Error {  
  BL_NO_ERROR = 0,
  BL_ERROR,
} BL_Error_t;


#define DISPLAY_WRITE_FEEDBACK  1 //enable to display a "." while write flash pages

#endif //__BL_DEFINES_H
