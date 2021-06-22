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
 *    File   : eoss3_hal_fpga_FLL.h
 *    Purpose: This file contains API declaration for FLL 
 *              (Freqency Locked Loop) implmented in fpga.
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#ifndef __EOSS3_HAL_HAL_FB_FLL_H_
#define __EOSS3_HAL_HAL_FB_FLL_H_

#include <stdint.h>
#include <stddef.h>
#include "eoss3_dev.h"
#include "eoss3_hal_def.h"

//See the FLL IP Documentation for register and address definitions
typedef struct
{
  __IO uint32_t CNTRL_REG;     /* 0x00 = FLL Control - enable (1) or disable(0) */
  __IO uint32_t SAMPLE_TIMER;  /* 0x04 = #of Local Clks used to compare against input clk */
  __IO uint32_t GAP_TIMER;     /* 0x08 = #of Local Clks to wait before starting next comparision */
  
  /* Note : These are added for Debug purposes only. Code should not depend on 
   *        them, since they may be removed any time
  */
#if 0
  __IO uint32_t WORD_COUNT_DIFF;     /* 0x0C = difference between local and master word counts */
  __IO uint32_t LSB_WORD_COUNTS;     /* 0x10 = LSB bits of {Local[15:0]:Master[15:0]} word counts */
  __IO uint32_t LOCAL_WORD_COUNT;    /* 0x14 = Continuos count of Local Clks divided byt 64 = Local Word Count*/  
  __IO uint32_t MASTER_WORD_COUNT;   /* 0x18 = Continuos count of Master Clks divided byt 64 = Master Word Count*/  
  __IO uint32_t MASTER_SAMPLE_COUNT; /* 0x1C = Master sample count at the given instance */    
#endif  
} FPGA_FLL_TypeDef;


#define FB_FLL_BASE  (FPGA_PERIPH_BASE+0x1000)
#define FB_FLL       ((FPGA_FLL_TypeDef *)FB_FLL_BASE)

// These are the FPGA Interrupts from the FLL
#define FPGA_FLL_SLOWDOWN_INT   (0)
#define FPGA_FLL_SPEEDUP_INT    (1)

extern void HAL_FB_FLL_Enable(void);
extern void HAL_FB_FLL_Disable(void);
extern void HAL_FB_FLL_Set_Sample_Time( int sample_count);
extern int HAL_FB_FLL_Get_Sample_Time(void);
extern void HAL_FB_FLL_Set_Gap_Time( int gap_count);
extern int HAL_FB_FLL_Get_Gap_Time(void);
extern void HAL_FB_FLL_Init(HAL_FBISRfunction slow_down_fn, HAL_FBISRfunction speed_up_fn);

extern void slow_down_ISR(void);
extern void speed_up_ISR(void);

#endif /* !__EOSS3_HAL_HAL_FB_FLL_H_ */
