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
#include "fpga_arnold_control.h"

//------------- Pointer to registers ---------------------//
fpga_arnold_control_regs_t* parnold_control_regs = (fpga_arnold_control_regs_t*)(FPGA_PERIPH_BASE);

//------------- Local functions -------------------------//


//------------- Local variables ------------------------//

void    fpga_arnold_control_init(void) {
  // Setup FPGA clocks
  S3x_Clk_Set_Rate(S3X_FB_21_CLK, 12000*1000);
  S3x_Clk_Set_Rate(S3X_FB_16_CLK, 12000*1000);
  S3x_Clk_Enable(S3X_FB_21_CLK);
  S3x_Clk_Enable(S3X_FB_16_CLK);
    
  // Confirm expected IP is loaded
  configASSERT(parnold_control_regs->signature == 0xFEED);
}

void      fpga_arnold_set_control(uint32_t uxValue) {
  parnold_control_regs->control = uxValue;
}

uint32_t   fpga_arnold_get_control(void) {
  return (parnold_control_regs->control);
}

void      fpga_arnold_set_clkdiv(uint32_t uxValue) {
  parnold_control_regs->clkdiv = uxValue;
}

uint32_t   fpga_arnold_get_clkdiv(void) {
  return (parnold_control_regs->clkdiv);
}

uint32_t   fpga_arnold_get_signature(void) {
  return (parnold_control_regs->signature);
}

uint32_t   fpga_arnold_get_revision(void) {
  return (parnold_control_regs->revision);
}

void      fpga_arnold_set_scratch(uint32_t uxValue) {
  parnold_control_regs->scratch = uxValue;
}

uint32_t   fpga_arnold_get_scratch(void) {
  return (parnold_control_regs->scratch);
}
