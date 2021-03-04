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
 *    File   : s3x_pi.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"
#include "s3x_clock.h"
#include "s3x_dfs.h"
#include "s3x_pi.h"
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "task.h"

#define PI_ID(x) (S3x_Gate_PI_info[x] & 0xFF)
#define PI_GINDEX(x) ((S3x_Gate_PI_info[x] & 0xFF00) >> 8)
#define PI_MSEC_TOUT 50
extern S3x_Pi S3Pi[];

/*
*----------------------------------
*    1             |    0         |
*----------------------------------|
*         |        |      |        |
* index in PI Ginfo|      PI ID    |
*         |        |      |        |
*----------------------------------
*/

UINT16_t S3x_Gate_PI_info[S3X_MAX_CLK] = {

    [S3X_CFG_DMA_A1_CLK] = ((0 << 8) | PI_A1),
    [S3X_A1_CLK] = ((1 << 8) | PI_A1),

    [S3X_I2S_A1_CLK] = ((0 << 8) | PI_I2S),

    [S3X_EFUSE_01_CLK] = ((0 << 8) | PI_EFUSE),
    [S3X_EFUSE_02_CLK] = ((1 << 8) | PI_EFUSE),

    [S3X_FFE_X4_CLK] = ((0 << 8) | PI_FFE),
    [S3X_FFE_X1_CLK] = ((1 << 8) | PI_FFE),
    [S3X_FFE_CLK] = ((2 << 8) | PI_FFE),

    [S3X_PKT_FIFO_CLK] = ((0 << 8) | PI_PF),
    [S3X_ASYNC_FIFO_0_CLK]  = ((1 << 8) | PI_PF),

    [S3X_FB_02_CLK]         = ((0 << 8) | PI_FB),
    [S3X_FB_16_CLK]         = ((1 << 8) | PI_FB),
    [S3X_FB_21_CLK]         = ((2 << 8) | PI_FB),
    [S3X_CLKGATE_FB]        = ((3 << 8) | PI_FB),

    [S3X_AUDIO_DMA_CLK]     = ((0 << 8) | PI_AD0_ADMA),

    [S3X_PDM_LEFT]          = ((0 << 8) | PI_AD1_LEFT),
    [S3X_LPSD]              = ((0 << 8) | PI_AD3_LPSD),
    [S3X_AUDIO_APB]         = ((0 << 8) | PI_AD5_APB),

    [S3X_PDM_RIGHT]         = ((0 << 8) | PI_AD2_RIGHT),
#ifdef PDM_MIC_STEREO
    [S3X_PDM_STEREO]        = ((0 << 8) | PI_AD),
#endif
    [S3X_I2S_MASTER]        = ((0 << 8) | PI_AD4_I2SM),

    [S3X_SDMA_CLK]          = ((0 << 8) | PI_SDMA),

    [S3X_A0_01_CLK] = PI_MAX,
    [S3X_SDMA_SRAM_CLK] = PI_MAX,
    [S3X_SPT_CLK] = PI_MAX,
    [S3X_A0_08_CLK] = PI_MAX,
    [S3X_CLKGATE_PIF] = PI_MAX,
    [S3X_M4_S0_S3_CLK] = PI_MAX,
    [S3X_M4_S4_S7_CLK] = PI_MAX,
    [S3X_M4_S8_S11_CLK] = PI_MAX,
    [S3X_M4_S12_S15_CLK] = PI_MAX,
    [S3X_SYNCUP_A0_AHB_CLK] = PI_MAX,
    [S3X_M4_PRPHRL_CLK] = PI_MAX,
    [S3X_ADC_CLK] = PI_MAX,
};
#if 0
void s3x_sram_in_lpm(void)
{
    /* LPMF */
    PMU_WVAL(0x230, 0x0FFF); //bug fix for FFE Batching mode
    /* DS RAM, Leave 3 block of HWA Section,
    * 1 for SHM
    */
    PMU_WVAL(0x100, 0x0FFF); //bug fix for FFE Batching mode
}
#endif

S3x_Pi* s3x_get_pi(UINT32_t clk_id)
{
    S3x_Pi *pi = NULL;
    UINT8_t pid;

    pid = PI_ID(clk_id);

    if (pid < PI_MAX)
        pi = &S3Pi[pid];

    return pi;
}

UINT8_t s3x_get_pi_gindex(UINT8_t clk_id)
{
    UINT8_t index;

    if (clk_id >= S3X_MAX_CLK)
        return INVALID_INDEX;
    index = PI_GINDEX(clk_id);
    return index;
}

/* update the curr state of pi */
UINT8_t s3x_pi_get_st (S3x_Pi *pi)
{
    UINT32_t st_reg;
    st_reg = PMU_RVAL(pi->pctrl.st_off);
    st_reg &= pi->pctrl.pmask;
    if (st_reg == pi->pctrl.pmask)
        pi->pi_cstate = PI_ACTIVE;
    else
        pi->pi_cstate &= (~PI_ACTIVE);
    return pi->pi_cstate;
}
/* put pi to cfg state */
int s3x_pi_set_cfg_st (S3x_Pi *pi)
{
    volatile UINT32_t st_reg, pi_st;
    int time_out = PI_MSEC_TOUT;
    int time_in;

    if (pi->ginfo.on_gcnt)
        return -EPI_INUSE;
    if (PI_ACTIVE & s3x_pi_get_st(pi))
    {
        if (pi->pctrl.cfg_off)
        {
            PMU_WVAL(pi->pctrl.cfg_off, pi->cfg_state);
        }
        PMU_WVAL(pi->pctrl.trig_off, pi->pctrl.trig_mask);
        time_in = xTaskGetTickCount();
        time_out = time_in + PI_MSEC_TOUT;
        while (PMU_RVAL(pi->pctrl.trig_off) && (time_in < time_out))
        {
            time_in = xTaskGetTickCount();
        }
        if (time_out == time_in)
            return -EPI_SET_STATE;
        time_in = xTaskGetTickCount();
        time_out = time_in + PI_MSEC_TOUT;
        do {
            pi_st = s3x_pi_get_st(pi);
            time_in = xTaskGetTickCount();
        } while ((pi_st & PI_ACTIVE) && (time_in < time_out));

    }
    if (time_out == time_in)
        return -EPI_SET_STATE;

    return STATUS_OK;
}

int s3x_pi_set_active_st (S3x_Pi *pi)
{
    volatile UINT32_t pi_st;
    int time_out, time_in;
    UINT32_t st_reg, reg;

    time_in = xTaskGetTickCount();
    time_out = time_in + PI_MSEC_TOUT;
    st_reg = PMU_RVAL(pi->pctrl.st_off);
    st_reg &= pi->pctrl.pmask;
    /* If PI not active */
    if (st_reg != pi->pctrl.pmask)
    {
        reg = pi->pctrl.swu_mask & (~st_reg);
        PMU_WVAL(pi->pctrl.swu_off, reg);
        do {
          time_in = xTaskGetTickCount();
          st_reg = PMU_RVAL(pi->pctrl.st_off) & pi->pctrl.pmask;
        } while((st_reg != pi->pctrl.pmask) && (time_in < time_out));
        if (time_out == time_in)
        {
          printf("%s Err PI SWU REG STATE\n", __func__);
          return -EPI_SET_STATE;
        }

        time_in = xTaskGetTickCount();
        time_out = time_in + PI_MSEC_TOUT;
        do {
            pi_st = s3x_pi_get_st(pi);
            time_in = xTaskGetTickCount();
        } while ( (!(pi_st & PI_ACTIVE)) && (time_in < time_out));
    }
    if (time_out == time_in)
    {
        printf("%s Err EPI_SET_STATE\n", __func__);
        return -EPI_SET_STATE;
    }

     pi->pi_cstate = (pi_st & PI_ACTIVE);
    return STATUS_OK;
}

/* get the gate status and set on get cnt */
int s3x_pi_get_clkg_st(S3x_Pi *pi)
{
    int i, ret;

    for (i =0; i < pi->ginfo.gcnt; i++ )
    {
        ret = _S3x_Clk_Get_Status(pi->ginfo.gid[i]);
        if (ret > 0 )
        {
            if ( pi->ginfo.gst[i] != ret)
            {
                pi->ginfo.gst[i] = ret;
                pi->ginfo.on_gcnt++;
            }
        }
    }

    return STATUS_OK;
}

int s3x_pi_set_clkg_st(S3x_Pi *pi, UINT8_t gindex, UINT8_t st)
{
    if (pi->ginfo.gst[gindex] != st)
    {
        pi->ginfo.gst[gindex] = st;
        if (st == GATE_EN)
        {
            pi->ginfo.on_gcnt++;
         }
        else if((st == GATE_DIS) &&
                    (pi->ginfo.on_gcnt))
        {
            pi->ginfo.on_gcnt--;
        }
    }
    return STATUS_OK;
}

int s3x_set_pi_state(S3x_Pi *pi)
{
    int ret;
    if (!pi->ginfo.on_gcnt)
        ret = s3x_pi_set_cfg_st(pi);
    else
        ret = s3x_pi_set_active_st(pi);

    return ret;
}

int  s3x_get_pi_init_state(S3x_Pi *pi)
{
    s3x_pi_get_clkg_st(pi);
    s3x_pi_get_st(pi);
    s3x_set_pi_state(pi);

    return STATUS_OK;
}

int S3x_pi_init(void)
{
    int i;

    for (i =0; i < PI_MAX; i++ )
    {
        if (S3Pi[i].name[0] != 0)               // Safety check -- ignore uninitialized entries 
            s3x_get_pi_init_state(&S3Pi[i]);
    }
    return STATUS_OK;
}
