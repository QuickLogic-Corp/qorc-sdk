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
#include "eoss3_dev.h"
#include "dbg_uart.h"
#include "tinytester.h"

//------------- Pointer to registers ---------------------//
tinytester_regs_t* ptinytester_regs = (tinytester_regs_t*)(FPGA_PERIPH_BASE);

//------------- Local functions -------------------------//


//------------- Local variables ------------------------//

//------------- Main Routines ---------------------------//
void tinytester_run(uint32_t kchannel, tinytester_channelconfig_t* atinytester_channelconfig, uint32_t kvectorOutput, uint32_t* auxOutputvector, uint32_t kvectorInput, uint32_t* auxInputvector) {
    uint32_t    uxP0 = 0;
    uint32_t    uxP1 = 0;
    uint32_t    uxP2 = 0;
    uint32_t    uxP3 = 0;
    uint32_t    uxOe = 0;
    
    configASSERT(kvectorInput >=  kvectorOutput);
    
    for (int ichannel = 0; ichannel != kchannel; ichannel++) {
        if (atinytester_channelconfig[ichannel].afDrive[0]) uxP0 |= (1 << ichannel);
        if (atinytester_channelconfig[ichannel].afDrive[1]) uxP1 |= (1 << ichannel);
        if (atinytester_channelconfig[ichannel].afDrive[2]) uxP2 |= (1 << ichannel);
        if (atinytester_channelconfig[ichannel].afDrive[3]) uxP3 |= (1 << ichannel);
        if (atinytester_channelconfig[ichannel].mode == output) uxOe |= (1 << ichannel);
    }
    // Put tester in idle mode
    tinytester_control(0x0);
    // Configure channels
    tinytester_active_on_p0(uxP0);
    tinytester_active_on_p1(uxP1);
    tinytester_active_on_p2(uxP2);
    tinytester_active_on_p3(uxP3);
    tinytester_oe(uxOe);
    // Loop thru writing and reading
    uint32_t ux;
    for (int ivector = 0; ivector != kvectorOutput; ivector++) {
        tinytester_dataout(auxOutputvector[ivector]);   // Load data
        tinytester_control(0x1);                        // Trigger cycle
        while (tinytester_statusis() != 0x10);          // Wait for state machine to go to wait state
        auxInputvector[ivector] = tinytester_datainis();// Retrieve data
        tinytester_control(0x0);                        // End cycle
    }
    // Print captures
    for (int ivector = 0; ivector != kvectorOutput; ivector++) {
        dbg_str_hex32("",auxInputvector[ivector]);
    }
}

void    tinytester_init(void) {
    // Setup FPGA clocks
    S3x_Clk_Set_Rate(S3X_FB_21_CLK, 12000*1000);
    S3x_Clk_Set_Rate(S3X_FB_16_CLK, 12000*1000);
    S3x_Clk_Enable(S3X_FB_21_CLK);
	S3x_Clk_Enable(S3X_FB_16_CLK);
    
    // Confirm expected gateware is loaded
    configASSERT(ptinytester_regs->signature == 0xDEEB);
    
    CRU->FB_SW_RESET = 0x00;    // Disable resets
}

// Misc routines
uint32_t    tinytester_signatureis(void) {
    return (ptinytester_regs->signature);
}

// Control routines
void    tinytester_control(uint32_t uxcontrol) {
    ptinytester_regs->control = uxcontrol;
}
uint32_t    tinytester_controlis(void) {
    return (ptinytester_regs->control);
}

uint32_t    tinytester_statusis(void) {
    return (ptinytester_regs->status);
}

void        tinytester_oe(uint32_t uxoe) {
    ptinytester_regs->oe = uxoe;
}
uint32_t    tinytester_oeis(void) {
    return(ptinytester_regs->oe);
}

void    tinytester_active_on_p0(uint32_t uxactive_on_p0) {
    ptinytester_regs->active_on_p0 = uxactive_on_p0;
}
uint32_t    tinytester_active_on_p0is(void) {
    return (ptinytester_regs->active_on_p0);
}

void    tinytester_active_on_p1(uint32_t uxactive_on_p1) {
    ptinytester_regs->active_on_p1 = uxactive_on_p1;
}
uint32_t    tinytester_active_on_p1is(void) {
    return (ptinytester_regs->active_on_p1);
}

void    tinytester_active_on_p2(uint32_t uxactive_on_p2) {
    ptinytester_regs->active_on_p2 = uxactive_on_p2;
}
uint32_t    tinytester_active_on_p2is(void) {
    return (ptinytester_regs->active_on_p2);
}

void    tinytester_active_on_p3(uint32_t uxactive_on_p3) {
    ptinytester_regs->active_on_p3 = uxactive_on_p3;
}
uint32_t    tinytester_active_on_p3is(void) {
    return (ptinytester_regs->active_on_p3);
}

// Data routines
void        tinytester_dataout(uint32_t uxdataout) {
    ptinytester_regs->dataout = uxdataout;
}
uint32_t    tinytester_dataoutis(void) {
    return(ptinytester_regs->dataout);
}

uint32_t    tinytester_datainis(void) {
    return (ptinytester_regs->datain);
}