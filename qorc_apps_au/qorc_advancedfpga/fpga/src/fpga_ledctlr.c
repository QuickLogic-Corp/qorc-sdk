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

#include <stdint.h>
#include <stdio.h>

#include "Fw_global_config.h"
#include "fpga_ledctlr.h"

//------------- Pointer to registers ---------------------//
fpga_ledctlr_regs_t* pledctlr_regs = (fpga_ledctlr_regs_t*)(FPGA_PERIPH_BASE);
fpga_ledctlr_regs2_t* pledctlr_regs2 = (fpga_ledctlr_regs2_t*)(FPGA_PERIPH_BASE);

//------------- Local functions -------------------------//


//------------- Local variables ------------------------//

void    fpga_ledctlr_init(void) {
    
    // Setup FPGA clocks
    S3x_Clk_Set_Rate(S3X_FB_21_CLK, 12000*1000);
    S3x_Clk_Set_Rate(S3X_FB_16_CLK, 12000*1000);
    
    S3x_Clk_Enable(S3X_FB_21_CLK);
	S3x_Clk_Enable(S3X_FB_16_CLK);
    
    // Confirm expected IP is loaded
    configASSERT(pledctlr_regs->device_id == 0xA5BD);    
}

void    fpga_ledctlr_setcolor(uint8_t ucColorValue, uint8_t ucTimerIndex) {
	switch(ucTimerIndex) {
		case 0:
			pledctlr_regs->color0 = ucColorValue;
			break;
		case 1:
			pledctlr_regs->color1 = ucColorValue;
			break;
		case 2:
			pledctlr_regs->color2 = ucColorValue;
			break;
		case 3:
			pledctlr_regs->color3 = ucColorValue;
			break;
		default:
		configASSERT(false);
		break;
	}
}

uint32_t fpga_ledctlr_getcolors(void) {
	return pledctlr_regs2->colors;
	
}

uint8_t	fpga_ledctlr_getcolor(uint8_t ucTimerIndex) {
	uint8_t	ucColorValue;
	switch(ucTimerIndex) {
		case 0:
			ucColorValue = pledctlr_regs->color0;
			break;
		case 1:
			ucColorValue = pledctlr_regs->color1;
			break;
		case 2:
			ucColorValue = pledctlr_regs->color2;
			break;
		case 3:
			ucColorValue = pledctlr_regs->color3;
			break;
		default:
		configASSERT(false);
		break;
	}
	return (ucColorValue);
}

void    fpga_ledctlr_setduration(uint16_t uhDuration, uint8_t ucTimerIndex) {
	switch(ucTimerIndex) {
		case 0:
			pledctlr_regs->duration0 = uhDuration;
			break;
		case 1:
			pledctlr_regs->duration1 = uhDuration;
			break;
		case 2:
			pledctlr_regs->duration2 = uhDuration;
			break;
		case 3:
			pledctlr_regs->duration3 = uhDuration;
			break;
		default:
		configASSERT(false);
		break;
	}
}

uint16_t	fpga_ledctlr_getduration(uint8_t ucTimerIndex) {
	uint16_t	uhDuration;
	switch(ucTimerIndex) {
		case 0:
			uhDuration = pledctlr_regs->duration0;
			break;
		case 1:
			uhDuration = pledctlr_regs->duration1;
			break;
		case 2:
			uhDuration = pledctlr_regs->duration2;
			break;
		case 3:
			uhDuration = pledctlr_regs->duration3;
			break;
		default:
		configASSERT(false);
		break;
	}
	return (uhDuration);
}
