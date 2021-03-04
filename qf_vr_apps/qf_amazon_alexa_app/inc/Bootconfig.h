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
 *    File   : Bootconfig.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __BOOT_CONFIG_H
#define __BOOT_CONFIG_H

//These must enabled to load the FPGA and FFE first if used in the rest of the code
//Note: Enable only during debug. Should not enabled for Flashing the bin file since
// the bootloaders loads them 
#if defined(ENABLE_LOAD_FPGA)
#error Set this here, not on command line.
#endif
#define ENABLE_LOAD_FPGA 0 /* exactly 0 or 1, nothing else */

#if defined(ENABLE_LOAD_FFE)
#error Set this here, not on command line.
#endif
#define ENABLE_LOAD_FFE  0 /* exactly 0 or 1, nothing else */

void fpga_load(const char* filename);
void ffe_load(const char* filename);
void start_load_from_flash(void);

//#define LOAD_MERCED_1_1_FPGA 

#endif	// __BOOT_CONFIG_H
