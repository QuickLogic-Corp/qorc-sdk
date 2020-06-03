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
 *-  Copyright Notice  -------------------------------------
 *                                                          
 *    Licensed Materials - Property of QuickLogic Corp.     
 *    Copyright (C) 2019 QuickLogic Corporation             
 *    All rights reserved                                   
 *    Use, duplication, or disclosure restricted            
 *                                                          
 *    File   : s3x_pi.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __S3X_PI_H
#define __S3X_PI_H


#include "FreeRTOS.h"
#include "task.h"

#include "test_types.h"
#include "s3x_err_base.h"
#include "s3x_clock_hal.h"

#define PI_MAX_GCNT 5

#define PMU_RVAL(x) (*((int *)((int)PMU + x)))
#define PMU_WVAL(x, y) (*(int *)((int )PMU + x) = y)

typedef struct _S3x_Pi_Ctrl
{
    UINT16_t st_off;
    UINT16_t cfg_off;
    UINT16_t trig_off;
    UINT16_t swu_off;
    UINT16_t pmask;
    //UINT8_t pst;
    UINT8_t trig_mask;
    //UINT8_t ip_st_mask;
    //UINT32_t ip_st_reg;
    UINT8_t swu_mask;

} S3x_Pi_Ctrl;

typedef struct _Pi_Clk_Gate_Info
{
    UINT8_t gcnt;
    UINT8_t gid[PI_MAX_GCNT];
    UINT8_t gst[PI_MAX_GCNT];
    UINT8_t on_gcnt;
} Pi_Clk_Gate_Info;

typedef struct _S3x_Pi
{
    UINT8_t name[8];
    S3x_Pi_Ctrl pctrl;
    Pi_Clk_Gate_Info ginfo;
    UINT8_t pi_cstate;
    UINT8_t cfg_state;
} S3x_Pi;

/* PD state*/
typedef enum {
    PI_ACTIVE = 0x1,
    PI_DS = 0x2,
    PI_SHDN = 0x4,
} PI_STATE;

typedef enum {
    PI_NO_CFG = 0,
    PI_SET_DS,
    PI_SET_SHDN,
} PI_CFG_STATE;

/* PI ID */
typedef enum {
    PI_A1,
    PI_I2S,
    PI_EFUSE,
    PI_FFE,
    PI_PF,
    PI_FB,
    PI_AD0_ADMA,
    PI_AD1_LEFT,
    PI_AD2_RIGHT,
    PI_AD3_LPSD,
    PI_AD4_I2SM,
    PI_AD5_APB,
    PI_AD,
    PI_SDMA,
    PI_MAX,
} PI_ID;

int S3x_pi_init(void);
S3x_Pi* s3x_get_pi(UINT32_t clk_id);
int s3x_pi_set_cfg_st (S3x_Pi *pi);
int s3x_pi_set_active_st (S3x_Pi *pi);
//void s3x_sram_in_lpm(void);
UINT8_t s3x_pi_get_st (S3x_Pi *pi);
int s3x_pi_set_clkg_st(S3x_Pi *pi, UINT8_t gindex, UINT8_t en);
UINT8_t s3x_get_pi_gindex(UINT8_t clk_id);
#endif      /* __S3X_PI_H  */
