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

#ifndef __FPGA_ARNOLD_CONTROL_H_
#define __FPGA_ARNOLD_CONTROL_H_

#include <stdbool.h>

#include <FreeRTOS.h>
#include <queue.h>

#include <eoss3_dev.h>
#include <eoss3_hal_def.h>
#include "s3x_clock.h"


#define FPGA_LEDCTLR_ID_VALUE         0xA5BD
#define FPGA_LEDCTLR_REV_NUM          0x0100

typedef struct fpga_ledctlr_regs {
    uint32_t    signature;	  // 0x00
    uint32_t    revision;		  // 0x04
    uint16_t    scratch;		  // 0x08
    uint16_t    reserved1;		// 0x0A
    uint32_t    reserved2;		// 0x0C
    uint32_t    control; 			// 0x10
    uint32_t    clkdiv; 			// 0x14

} fpga_arnold_control_regs_t;


void    	fpga_arnold_control_init(void);
void    	fpga_arnold_set_control(uint32_t uxValue);
uint32_t 	fpga_arnold_get_control(void);
void    	fpga_arnold_set_clkdiv(uint32_t uxValue);
uint32_t 	fpga_arnold_get_clkdiv(void);
uint32_t 	fpga_arnold_get_signature(void);
uint32_t 	fpga_arnold_get_revision(void);
void    	fpga_arnold_set_scratch(uint32_t uxValue);
uint32_t 	fpga_arnold_get_scratch(void);

#endif // __FPGA_ARNOLD_CONTROL_H_