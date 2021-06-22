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
 *    File   : s3x_clock.c
 *    Purpose: 
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"
#include "eoss3_dev.h"
#include "eoss3_hal_gpio.h"
#include "s3x_clock.h"
#include "s3x_dfs.h"
#include "s3x_pi.h"
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "task.h"

#define USECNT(x)   (S3x_Clk_Domain_Gate_info[x] & 0xFF)

#define SET_USECNT(x, y) (S3x_Clk_Domain_Gate_info[x] =\
                  ((S3x_Clk_Domain_Gate_info[x] & 0xFFFFFF00) | (y & 0xFF)))

#define GATE_MASK(x) ((S3x_Clk_Domain_Gate_info[x] >> 8) & 0xFFFF)

#define DOMAIN_ID(x) ((S3x_Clk_Domain_Gate_info[x] >> 24) & 0xFF)

extern S3x_ClkD S3clk [];
extern UINT8_t S3clkd_size;

Hsosc_Param Hsosc = {0};

/* Hal layer CLK to Domain and Gate mapping */
/*
*----------------------------------
*    3    |    2   |  1   |    0   |
*----------------------------------|
*         |        |      |        |
*Domain Id|  Gate Mask    | Use_Cnt|
*         |        |      |        |
*----------------------------------
*/

UINT32_t S3x_Clk_Domain_Gate_info[S3X_MAX_CLK] = {

    [S3X_A0_01_CLK] = (CLK_C01 << 24 | C01_CLKGATE_A0 << 8 | 0xFF),
    [S3X_SDMA_SRAM_CLK] = (CLK_C01 << 24 | C01_CLKGATE_SDMA_SRAM << 8 | 0xFF),
    [S3X_PKT_FIFO_CLK] = ( CLK_C01 << 24 | C01_CLKGATE_PK_FIFO << 8 | 0xFF),
    [S3X_FFE_CLK] = (CLK_C01 << 24 | C01_CLKGATE_FFE << 8 | 0xFF),
    [S3X_CFG_DMA_A1_CLK] = ( CLK_C01 << 24 | C01_CLKGATE_APB2AHB_CFGDMA << 8 | 0xFF),
    [S3X_I2S_A1_CLK] = ( CLK_C01 << 24 | C01_CLKGATE_I2S << 8 | 0xFF),
    [S3X_SDMA_CLK] = (CLK_C01 << 24 | C01_CLKGATE_SDMA << 8 | 0xFF),
    [S3X_EFUSE_01_CLK] = (CLK_C01 << 24 | C01_CLKGATE_EFUSE << 8 | 0xFF),
    [S3X_SPT_CLK] = (CLK_C01 << 24 | C01_CLKGATE_SPT << 8 | 0xFF),
    [S3X_A1_CLK] = (CLK_C02 << 24 | C02_CLKGATE_A1 << 8 | 0xFF),
    [S3X_FB_02_CLK] = (CLK_C02 << 24 | C02_CLKGATE_FB << 8 | 0xFF),
    [S3X_EFUSE_02_CLK] = (CLK_C02 << 24 | C02_CLKGATE_EFUSE << 8 | 0xFF),
    [S3X_FFE_X4_CLK] = (CLK_C08X4 << 24 | C08X4_CLKGATE_FFE_X4CLK << 8 | 0xFF),
    [S3X_FFE_X1_CLK] = (CLK_C08X1 << 24 |  C08X1_CLKGATE_FFE_X1CLK << 8 | 0xFF),
    [S3X_A0_08_CLK] = (CLK_C08X1 << 24 | C08X1_CLKGATE_A0 << 8 | 0xFF),
    [S3X_ASYNC_FIFO_0_CLK] = (CLK_C08X1 << 24 | C08X1_CLKGATE_ASYNC_PF0 << 8 | 0xFF),
    [S3X_AUDIO_APB] = (CLK_C09 << 24 | C09_CLKGATE_AUDIO_APB << 8 | 0xFF),
    [S3X_CLKGATE_PIF] = (CLK_C09 << 24 | C09_CLKGATE_PIF << 8 | 0xFF),
    [S3X_CLKGATE_FB] = (CLK_C09 << 24 | C09_CLKGATE_FB << 8 | 0xFF),
    [S3X_M4_S0_S3_CLK] = (CLK_C10 << 24 | C10_CLKGATE_M4_SR0_SR3 << 8 | 0xFF),
    [S3X_M4_S4_S7_CLK] = (CLK_C10 << 24 | C10_CLKGATE_M4_SR4_SR7 << 8 | 0xFF),
    [S3X_M4_S8_S11_CLK] = (CLK_C10 << 24 | C10_CLKGATE_M4_SR8_SR11 << 8 | 0xFF),
    [S3X_M4_S12_S15_CLK] = (CLK_C10 << 24 | C10_CLKGATE_M4_SR12_SR15 << 8 | 0xFF),
    [S3X_AUDIO_DMA_CLK] = (CLK_C10 << 24 |  C10_CLKGATE_AUDIO_DMA << 8 | 0xFF),
    [S3X_SYNCUP_A0_AHB_CLK] = (CLK_C10 << 24 | C10_CLKGATE_SYNC_A0_AHB << 8 | 0xFF),
    [S3X_M4_PRPHRL_CLK] = (CLK_C11 << 24 | C11_CLKGATE_M4_PERIPHERAL << 8 | 0xFF),
    [S3X_FB_16_CLK] = (CLK_C16 << 24 | C16_CLKGATE_FB << 8 | 0xFF),
    [S3X_ADC_CLK] = (CLK_C19 << 24 | C19_CLKGATE_ADC << 8 | 0xFF),
    [S3X_FB_21_CLK] = (CLK_C21 << 24 | C21_CLKGATE_FB << 8 | 0xFF),
    [S3X_PDM_LEFT] = (CLK_C30 << 24 | C30_CLKGATE_PDM_LEFT << 8 | 0xFF),
    [S3X_PDM_RIGHT] = (CLK_C30 << 24 | C30_CLKGATE_PDM_RIGHT << 8 | 0xFF),
#ifdef PDM_MIC_STEREO
    [S3X_PDM_STEREO] = (CLK_C30 << 24 | C30_CLKGATE_PDM_STEREO << 8 | 0xFF),
#endif
    [S3X_I2S_MASTER] = (CLK_C30 << 24 | C30_CLKGATE_I2S_MASTER << 8 | 0xFF),
    [S3X_LPSD] = (CLK_C31 << 24 | C31_CLKGATE_LPSD << 8 | 0xFF),
};

#ifdef DEF_CRU_DBG
UINT32_t S3x_Clk_Dmon_Pad_Off[DPAD_MAX] = {
    /* dmon 7 */
    [DPAD_12] = PAD_12_OFF,
    [DPAD_32] = PAD_32_OFF,
    [DPAD_41] = PAD_41_OFF,
    /* dmon 6 */
    [DPAD_13] = PAD_13_OFF,
    [DPAD_33] = PAD_33_OFF,
    [DPAD_42] = PAD_42_OFF,
};
#endif

S3x_ClkD* S3x_Id_To_Domain(uint8_t id)
{
    if (id < S3clkd_size)
        return &S3clk[id];
    else
        return NULL;
}

UINT32_t S3x_Clkd_Get_Cpu_Rate(void)
{
    return S3clk[CLK_C10].curr_rate;
}

int S3x_Clkd_Set_Cpu_Rate(uint32_t rate)
{
    return s3x_clkd_srate(&S3clk[CLK_C10], rate);
}

int s3x_clkd_get_init_rate(uint8_t id)
{
    UINT8_t domain_id;

    if (id >= S3X_MAX_CLK)
        return -EINVALID_VAL;

    domain_id = DOMAIN_ID(id);
    return S3clk[domain_id].init_state.irate;

}
 /* get the div val and write it */
int s3x_clkd_update_div_val(S3x_ClkD *clkd, UINT32_t src_rate)
{
    UINT16_t rdiv;
    int div, rate;


    /*If expected rate is set, then use its value to update the divider*/
    if(clkd->expected_rate != 0)
    {
        rate = clkd->expected_rate;
    }
    else
    {
        rate = clkd->curr_rate;
    }
    div = s3x_clkd_cal_div (clkd, rate, src_rate);

    if (div < 0)
        return -1;
    clkd->src_rate = src_rate;
    s3x_clkd_write_div(clkd, div, rate, NULL);

    rdiv = s3x_clkd_read_div(clkd);

    if (rdiv)
        clkd->curr_rate = clkd->src_rate / rdiv  ;
    else
        clkd->curr_rate = clkd->src_rate;


    return STATUS_OK;
}

int s3x_clkd_update_fd_rate(S3x_ClkD *clkd)
{
    int src_rate;
    src_rate = clkd->sync_clk.sclk.src_rate;
    clkd->curr_rate = (src_rate / clkd->div_val);

    return STATUS_OK;
}

/*Update div of SD clock with SRC rate */
static void s3x_clkd_update_sd_div(S3x_ClkD *clkd, UINT32_t rate)
{
    int i, id = 0;


    for(i = 0; i < clkd->sync_clk.sd_clk.cnt; i++) {
        id =  clkd->sync_clk.sd_clk.sd_id[i];
        S3clk[id].sync_clk.sclk.src_rate = rate;
        if(S3clk[id].type == FD_CLK)
        {
          s3x_clkd_update_fd_rate(&S3clk[id]);
          continue;
        }
        else if(!(S3clk[id].curr_rate))
        {
            continue;
        }
        s3x_clkd_update_div_val(&S3clk[id], rate);
    }

}

/* Update all div val with new HSOSC rate
only change in Div val no change in Freq */
void s3x_clkd_update_all_src_div(UINT32_t rate, UINT32_t mode)
{
    int i;
    UINT32_t div_reg;

    if (mode == SCALE_DOWN)
    {
        for(i = 0; i < S3clkd_size; i++) {
            /* if clk src is disable skip */
            if(!(S3clk[i].src_div))
                continue;

            if((S3clk[i].type == (UINT8_t)SRC_CLK))
            {
                s3x_clkd_update_div_val(&S3clk[i], rate);
                if(S3clk[i].sync_clk.sd_clk.cnt)
                {
                     s3x_clkd_update_sd_div(&S3clk[i], S3clk[i].curr_rate);
                }
            }
            /* temp bumpup CPU to finish faster*/
            if (S3clk[i].clkd_id == CLK_C10)
            {
                //HAL_GPIO_Write(GPIO_1, 0);
                div_reg = CRU_RVAL(0);
                CRU_WVAL(0, 0);
            }
        }
        CRU_WVAL(0, div_reg);
    }
    else
    {
          for(i = (S3clkd_size -1); i >= 0; i--) {
              /* temp bumpup CPU to finish faster*/
                CRU_WVAL(0, 0);

                if(!(S3clk[i].src_div))
                    continue;

                if((S3clk[i].type == (UINT8_t)SRC_CLK))
                {
                    s3x_clkd_update_div_val(&S3clk[i], rate);
                    if(S3clk[i].sync_clk.sd_clk.cnt)
                    {
                         s3x_clkd_update_sd_div(&S3clk[i], S3clk[i].curr_rate);
                    }
                }
            }
    }


}

UINT32_t inline S3x_Clkd_Get_Hsosc_Rate(void)
{

    if (Hsosc.init)
        return Hsosc.rate;

    Hsosc.en =  AIP->OSC_CTRL_0 & AIP_OSC_CTRL_EN;
    if (Hsosc.en)
        Hsosc.rate = OSC_GET_FREQ_INC();
    else
        Hsosc.rate = 0;

    return Hsosc.rate;
}

UINT32_t s3x_clkd_set_HSOSC_qos_rate(UINT32_t rate)
{
    if (rate >  Hsosc.qos_rate)
    {
        Hsosc.qos_rate = rate;
        if (Hsosc.rate < rate)
            S3x_Clkd_Change_Hsosc(rate);
    }
    else if (!rate)
    {
        Hsosc.qos_rate = S3x_get_qos_rate(MIN_HSOSC_FREQ);
    }

    return STATUS_OK;
}

UINT32_t inline s3x_clkd_get_active_HSOSC_qos(void)
{
      return Hsosc.qos_rate;
}

int S3x_Clkd_Change_Hsosc(UINT32_t rate)
{
    UINT32_t mode;
    //UINT32_t intr_st;

    /*Instead of returning invalid rate, set the hsosc rate to its limits*/
    if(rate < OSC_MINIMUM_FREQ)
    {
        rate = OSC_MINIMUM_FREQ;
    }
    else if (rate > OSC_MAXIMMUM_FREQ)
    {
        rate = OSC_MAXIMMUM_FREQ;
    }
    if (rate !=  Hsosc.rate)
    {

 //       intr_st = taskENTER_CRITICAL_FROM_ISR();
        if (rate < Hsosc.qos_rate)
            rate  = Hsosc.qos_rate;

        if (rate > Hsosc.rate)
            mode = SCALE_UP;
        else
            mode =  SCALE_DOWN ;

        AIP->OSC_CTRL_0 |= AIP_OSC_CTRL_EN;
        AIP->OSC_CTRL_0 &= ~AIP_OSC_CTRL_FRE_SEL;
        Hsosc.en = 1;
        OSC_SET_FREQ_INC(rate);

        Hsosc.rate =  rate;
        if (Hsosc.init)
            s3x_clkd_update_all_src_div(rate, mode);

        //taskEXIT_CRITICAL_FROM_ISR(intr_st);
    }

    return STATUS_OK;
}

/*Src div off is diff for Src clk and SD clk this fun
*  will get appropiate offset
*/
static inline void s3x_clkd_get_src_div_param(S3x_ClkD *clkd,
                              int *src_div_off, int *src_div_shift)
{
   if((clkd->type == SRC_CLK) ||  (clkd->type == FD_CLK)){
        *src_div_off = SRC_CLK_DIV_OFF;
        *src_div_shift = clkd->cru_ctrl.src_div_shift;
    } else { //SD clk
        *src_div_off = clkd->cru_ctrl.div_off;
        *src_div_shift = clkd->cru_ctrl.div_en_shift;
    }
}

/*
* get status of src divider A,B,C...
* return unshifted value
*/
static inline int s3x_clkd_get_src_div_st(S3x_ClkD *clkd)
{
    int src_div_off, src_div_shift;
    int src_div;
    s3x_clkd_get_src_div_param(clkd, &src_div_off, &src_div_shift);
    src_div = CRU_RVAL(src_div_off);
    src_div &= (1 << src_div_shift);
    return  src_div;
}

/*
* for src down clk need st of parent div (A,B, C)
* return unshifted value
*/
static inline int s3x_clkd_get_parent_src_div_st(S3x_ClkD *clkd)
{
    int src_div_off, src_div_shift;
    int src_div;
    int src_d;

    src_d = clkd->sync_clk.sclk.src_domain;
    s3x_clkd_get_src_div_param(&S3clk[src_d],
                        &src_div_off, &src_div_shift);
    src_div = CRU_RVAL(src_div_off);
    src_div &= (1 << src_div_shift);
    return  src_div;
}

/* enable or disable src div */
static int s3x_clkd_set_src_div_st(S3x_ClkD *clkd, UINT8_t en)
{
    int src_div_off, src_div_shift;
    UINT32_t reg, mask, rate;

    s3x_clkd_get_src_div_param(clkd, &src_div_off, &src_div_shift);
    mask = (1 << src_div_shift);
    reg = CRU_RVAL(src_div_off);

    if (en)
        reg |= mask;
    else
        reg &= (~mask);

    if (en)
    {
        rate = s3x_clkd_grate(clkd);
        if (rate != clkd->curr_rate)
            s3x_clkd_srate(clkd, clkd->curr_rate);
            //s3x_update_clk_rate (clkd, clkd->curr_rate, Hsosc.rate);

    }
    clkd->src_div = en;
    return (CRU_WVAL(src_div_off, reg));
}

/* check gate of SD clk, this is used to dis SRC div */
static int s3x_check_sd_mask_st(S3x_ClkD *clkd)
{
    int i, id, st =0;

    for(i = 0; i < clkd->sync_clk.sd_clk.cnt; i++) {
        id =  clkd->sync_clk.sd_clk.sd_id[i];
        if (S3clk[id].gate_val) {
            st = 1;
            break;
        }
    }

    return st;

}

/* disabe the SRC div
 in case we have SD clk disable only if all gates
 are off for SD clks
 */
static void s3x_disable_src_div(S3x_ClkD *clkd)
{
    int en = 0;

    if ((clkd->type == SRC_CLK) && clkd->sync_clk.sd_clk.cnt)
        en = s3x_check_sd_mask_st(clkd);

    if (!en)
        s3x_clkd_set_src_div_st(clkd, 0);
}

/* Get the Src clock and cal div for req rate */
static inline int s3x_clkd_cal_div(S3x_ClkD *clkd, UINT32_t rate, UINT32_t src)
{
    //UINT32_t src;
    //src = s3x_clkd_get_src_rate(clkd);
    if (src >= rate)
    {
        return (src  / rate);
    }
    else if (src < rate)
    {
        if((clkd->type == SD_CLK) &&
            (S3clk[clkd->sync_clk.sclk.src_domain].expected_rate != 0))
        {
                src = S3clk[clkd->sync_clk.sclk.src_domain].expected_rate;
                if (src < rate)
                {
                    return -1;
                }
                else
                {
                     return (src  / rate);
                }
        }
        else
        {
            return -1;
        }
    }

    return 0;
}


/* Call cal div if rate is not sufficient then change
* Src rate and cal div again
*/
int s3x_clkd_get_div (S3x_ClkD *clkd, UINT32_t rate)
{
    int ret ;

    clkd->src_rate = s3x_clkd_get_src_rate(clkd);

    ret = s3x_clkd_cal_div(clkd, rate, clkd->src_rate);
    if (ret >= 0)
        return ret;

     if (clkd->type == SD_CLK)
            ret = s3x_clkd_srate(&S3clk[clkd->sync_clk.
                                 sclk.src_domain], rate);
        else
            ret = S3x_Clkd_Change_Hsosc(rate);

     clkd->src_rate = s3x_clkd_get_src_rate(clkd);
     if (ret >= 0)
        return s3x_clkd_cal_div(clkd, rate, clkd->src_rate);

    return ret;
}


/* Update the div to reg */
static void s3x_clkd_write_div(S3x_ClkD *clkd, UINT16_t div, UINT32_t rate, uint32_t* pcruval)
{
    int div_adjust = 0;
    /*Seeting div adjust for Source and Sync Down Clock*/
    if(clkd->type == SRC_CLK)
    {
        div_adjust = 2;
    }
    else if (clkd->type == SD_CLK)
    {
        div_adjust = 1;
    }
    else
    {
        /* FD clk no need to write */
        return;
    }
     clkd->expected_rate = 0;
    /*Setting/Clearing expected rate*/
     /*if div is exceeding div_max value, store the expected rate and set div to max value*/
    if (div > (clkd->cru_ctrl.div_max + div_adjust))
    {
        div = clkd->cru_ctrl.div_max + div_adjust;
        clkd->expected_rate = rate;
    }
    /*If with calculated divider value results to exceeding default max rate, increment the divider and store the expected rate */
    else if ((div != 1) && ((clkd->src_rate / div) > clkd->def_max_rate ))
    {
        clkd->expected_rate = rate;
        ++div;
    }
    /*If the rate that can be set is not exactly the desired rate, then store the expected rate*/
    else if ((clkd->src_rate / div) != rate)
    {
        clkd->expected_rate = rate;
    }

    if (div == 1)
    {
        if (clkd->type == SRC_CLK)
        {
             /* disable divider, pass through */
            div = 0;
            /*check if bypassing causes curr_rate to exceed def_max_rate*/
            /*if yes, then set enable divider and divide by 2 instead of bypassing*/
            if((clkd->src_rate > clkd->def_max_rate))
            {
                div |= (1 << clkd->cru_ctrl.div_en_shift);
            }
            clkd->div_val = 1;
            goto exit;
        }
        else if (clkd->type == SD_CLK)
        {
            /*check if bypassing causes curr_rate to exceed def_max_rate*/
            /*if yes, then set divider as 2 instead of bypassing*/
            if(clkd->src_rate > clkd->def_max_rate)
            {
                ++div ;
            }
        }
    }
    clkd->div_val = div;
    div -= div_adjust ;
    div |= (1 << clkd->cru_ctrl.div_en_shift);

exit:
    if (pcruval && clkd->clkd_id == CLK_C10) { // Sleep routine wants to defer this
       *pcruval = div;
        //CRU_WVAL((clkd->cru_ctrl.div_off), div);
    } else {
        CRU_WVAL((clkd->cru_ctrl.div_off), div);
    }

}


/*Read div from reg*/
static inline UINT16_t s3x_clkd_read_div(S3x_ClkD *clkd)
{
    UINT16_t div, div_mask;

    if (clkd->type == FD_CLK) {
      div = clkd->div_val;
      goto exit;
    }
    div = CRU_RVAL(clkd->cru_ctrl.div_off);
    if (!(div & (1 << clkd->cru_ctrl.div_en_shift))){
        div = 1;
        goto exit;
    }

    div_mask = ((1 << clkd->cru_ctrl.div_en_shift) - 1);

    div = div & div_mask;
    if (clkd->type == SRC_CLK)
        div += 2;
    else if (clkd->type == SD_CLK)
        div += 1;
exit:
    return div;
}

/*Get the init time status */
static int s3x_clkd_get_def_st(S3x_ClkD *clkd)
{
    int div_st;

    div_st = s3x_clkd_get_src_div_st(clkd);
    if (!div_st) {
        /* SRC div is Off, Clkd is off */
        CRU_WVAL(clkd->cru_ctrl.gate_off, 0);
        clkd->gate_val = 0;
    } else { /*div is enable check gate */
        clkd->gate_val = (CRU_RVAL(clkd->cru_ctrl.gate_off)
                                    & clkd->cru_ctrl.gate_mask);

    }

    return STATUS_OK;
}

/* Get ref src OSC or RTC */
static void s3x_get_ref_clkd_src(S3x_ClkD *clkd)
{
  if (clkd->cru_ctrl.src_sel_off)
    clkd->ref_src = CRU_RVAL(clkd->cru_ctrl.src_sel_off);
}

/* HSOSC or RTC rate */
UINT32_t s3x_get_ref_clk_rate()
{
    return S3x_Clkd_Get_Hsosc_Rate();
}

/*
* Return the rate of ref clock,
* If SD clk return rate of SRC clk
*/
UINT32_t inline s3x_clkd_get_src_rate(S3x_ClkD *clkd)
{
    UINT32_t rate;
    if(clkd->type == SRC_CLK)
        rate = s3x_get_ref_clk_rate();
    /*SRC clk must be initialzed first as per order in S3clk struct */
    else
        rate = clkd->sync_clk.sclk.src_rate;
    return rate;
}

/* this will not check clock is enable or not
caller must enure clock is enable */
static int  s3x_clkd_grate(S3x_ClkD *clkd)
{
    UINT32_t src_rate, intr_st;
    UINT32_t div = 1, reg;
    UINT16_t div_shift_mask;
    int ret = STATUS_OK;

    int intrpt_state = 0;//by default non-ISR state
    //check if inside an interrupt
    if(SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) {
    intr_st = taskENTER_CRITICAL_FROM_ISR();
      intrpt_state = 1;
    } else {
      taskENTER_CRITICAL();
    }

    src_rate = s3x_clkd_get_src_rate(clkd);

    if (clkd->type == FD_CLK)//Fix divider X1
        div = clkd->div_val;
    else {
        /* get divider value */
        reg = CRU_RVAL(clkd->cru_ctrl.div_off);
        div_shift_mask = (1 << clkd->cru_ctrl.div_en_shift);

        if((reg & div_shift_mask) || (clkd->type == SD_CLK)) {
            /* div is enabled */
            div = reg & (div_shift_mask - 1);
            if (div > clkd->cru_ctrl.div_max)
                return - 1;/*map error code*/
            else if (clkd->type == SRC_CLK)
                div += 2;
            else
                div += 1;
        }
    }

    ret = src_rate / div;

    if(intrpt_state) {
    taskEXIT_CRITICAL_FROM_ISR(intr_st);
    } else {
      taskEXIT_CRITICAL();
    }
    return ret;
}

/* Update src_clk field for SD clk */
static void s3x_clkd_update_sd_src_rate(S3x_ClkD *clkd)
{
    int i, id;

    for(i = 0; i < clkd->sync_clk.sd_clk.cnt; i++) {
        id =  clkd->sync_clk.sd_clk.sd_id[i];
        S3clk[id].sync_clk.sclk.src_rate = clkd->curr_rate;
   }

}

/*
Rate is invalid in case
* 1. if rate is greater then design max
* 2. rate is less then Source down clock
*/
static inline int s3x_clkd_validate_rate(S3x_ClkD *clkd, UINT32_t rate)
{
    int ret = STATUS_OK, i, comp_rate;

    /* Check Rate greater then max possible for clkd */
    if (clkd->def_max_rate < rate){
        ret =  -EINVALID_RATE;
        /* Check if we have SD clk, if rate of SD clk
        * is greater then rate, we are not allowed to go down
        */
    } else if ((clkd->type == SRC_CLK)  &&
                clkd->sync_clk.sd_clk.cnt){
            for(i = 0; i < clkd->sync_clk.sd_clk.cnt; i++){
              if(S3clk[clkd->sync_clk.sd_clk.sd_id[i]].type == FD_CLK)
              {
                  continue;
              }
              /*check if expected rate for syn down clock is set, use it for validation*/
                if((S3clk[clkd->sync_clk.sd_clk.sd_id[i]].expected_rate) != 0)
                {
                    comp_rate = S3clk[clkd->sync_clk.sd_clk.sd_id[i]].expected_rate;
                }
                else
                {
                    comp_rate = S3clk[clkd->sync_clk.sd_clk.sd_id[i]].curr_rate;
                }
                if (comp_rate > rate){
                    ret = -ESD_RATE_HIGHER;
                    break;
                }
            }
    } else if (clkd->type == FD_CLK) {
         ret = -EFIX_RATE_CLK;
    }

    return ret;
}


/* Set the clock rate
* if Source clock is lower then Derived clock
* it will increase the Source clock, which might
* result in change in divider of all clock in
* case we need to change HSOSC rate
*/
static int s3x_clkd_srate(S3x_ClkD *clkd, UINT32_t rate)
{
    int ret = STATUS_OK;
    UINT32_t rdiv, intr_st, cpu_rate;

    int intrpt_state = 0;//by default non-ISR state
    //check if inside an interrupt
    if(SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) {
    intr_st = taskENTER_CRITICAL_FROM_ISR();
      intrpt_state = 1;
    } else {
      taskENTER_CRITICAL();
    }

    if (clkd->qos)
    {
        if ((clkd->qos->active_req & MIN_OP_FREQ) &&
                (rate < clkd->qos->rate[GET_QOS_INDEX(MIN_OP_FREQ)]))
        {
            rate  = clkd->qos->rate[GET_QOS_INDEX(MIN_OP_FREQ)];
        }
    }
    if (clkd->clkd_id == CLK_C10)
    {
        if (rate < S3x_get_qos_rate(MIN_CPU_FREQ))
            rate = S3x_get_qos_rate(MIN_CPU_FREQ);
    }

    if ((clkd->curr_rate == rate) && clkd->src_div)
      goto exit;

    ret = s3x_clkd_validate_rate(clkd, rate);
    if (ret < 0)
        goto exit;

    ret = s3x_clkd_get_div(clkd, rate);

     if (ret < 0)
        goto exit;

    s3x_clkd_write_div(clkd, ret, rate, NULL);

    rdiv = s3x_clkd_read_div(clkd);

    if (rdiv)
        clkd->curr_rate = clkd->src_rate / rdiv  ;
    else
        clkd->curr_rate = clkd->src_rate;

    cpu_rate  = CRU_RVAL(0);
    CRU_WVAL(0, 0);

    /* IN case SD clk, update their div */
    if ((clkd->type == SRC_CLK) &&
            clkd->sync_clk.sd_clk.cnt) {
        s3x_clkd_update_sd_div(clkd, clkd->curr_rate);
    }

    CRU_WVAL(0, cpu_rate);

exit:
    if(intrpt_state) {
    taskEXIT_CRITICAL_FROM_ISR(intr_st);
    } else {
      taskEXIT_CRITICAL();
    }
    return ret;

}

// force C10 to same as HSOSC
void s3x_force_set_cpu(S3x_ClkD *clkd, UINT32_t rate) {
    (void)rate; // ignore the given value
    s3x_clkd_write_div(clkd, 1, clkd->src_rate, NULL);
    clkd->curr_rate = clkd->src_rate  ;
}

int s3x_update_clk_rate(S3x_ClkD *clkd, UINT32_t rate, UINT32_t src_rate, uint32_t* pcruval)
{
    int ret = STATUS_OK;
    UINT32_t div, intr_st;

    taskENTER_CRITICAL();
    if (clkd->qos)
    {
        if ((clkd->qos->active_req & MIN_OP_FREQ) &&
                (rate < clkd->qos->rate[GET_QOS_INDEX(MIN_OP_FREQ)]))
        {
            rate  = clkd->qos->rate[GET_QOS_INDEX(MIN_OP_FREQ)];
        }
    }
    if (clkd->clkd_id == CLK_C10)
    {
        UINT32_t qos_rate;
        qos_rate =  S3x_get_qos_rate(MIN_CPU_FREQ);
        if (rate < qos_rate)
            rate = qos_rate;
    }


    if (clkd->type  == SD_CLK)
    {
        S3x_ClkD *src_clkd;
        src_clkd = S3x_Id_To_Domain(clkd->sync_clk.sclk.src_domain);
        src_rate = src_clkd->curr_rate;
        clkd->sync_clk.sclk.src_rate = src_rate;

    }
    if (src_rate < rate)
    {
        ret = -EINVALID_VAL;
        goto exit;
    }

    clkd->src_rate = src_rate;

    div =  (clkd->src_rate  / rate);

    s3x_clkd_write_div(clkd, div, rate, pcruval);
    //TIM23 if (pcruval) CRU_WVAL(0x000, *pcruval);

    clkd->curr_rate = clkd->src_rate / div  ;
exit:
    taskEXIT_CRITICAL();
    return ret;

}



int  _s3x_clkd_srate(S3x_ClkD *clkd, UINT32_t rate)
{
    return s3x_clkd_srate(clkd, rate);
}
/* Get  Gate mask value
*/
int s3x_clkd_get_gate_mask_st(S3x_ClkD *clkd,  UINT16_t mask)
{
    UINT32_t reg, rd_mask;

    reg = CRU_RVAL(clkd->cru_ctrl.gate_off);
    rd_mask = reg & clkd->cru_ctrl.gate_mask;


    if(rd_mask & mask)
        return 1;
    else
        return STATUS_OK;

}

/* Set  Gate mask value
*/
int s3x_clkd_set_gate_mask(S3x_ClkD *clkd,  UINT16_t mask, UINT8_t en)
{
    UINT32_t reg, rd_mask, wt_mask;

    reg = CRU_RVAL(clkd->cru_ctrl.gate_off);
    rd_mask = reg & clkd->cru_ctrl.gate_mask;
    wt_mask = mask & clkd->cru_ctrl.gate_mask;

    if (en) {
        if (!((~rd_mask) & wt_mask))
            goto exit;
        rd_mask |= wt_mask;

    } else {
       if(!(rd_mask & wt_mask))
            goto exit;
        rd_mask &= (~wt_mask);

    }

if(clkd->flags & LOCK_KEY)
   MISC_CTRL->LOCK_KEY_CTRL = KEY_VALUE;

    clkd->gate_val = rd_mask;
    CRU_WVAL(clkd->cru_ctrl.gate_off, rd_mask);
    clkd->gate_val = rd_mask;

if(clkd->flags & LOCK_KEY)
        MISC_CTRL->LOCK_KEY_CTRL = 0;


exit:
     return STATUS_OK;
}

/* this will enable all gates passed in mask,
    will not change rate & src */
static int  s3x_clkd_enable_gate(S3x_ClkD *clkd,  UINT16_t mask)
{
    int src;

    /* for SD clk enable parent src divider */
    if (clkd->type == SD_CLK)
    {
        src = s3x_clkd_get_parent_src_div_st(clkd);
        if (!src)
            s3x_clkd_set_src_div_st(&S3clk[clkd->sync_clk.sclk.src_domain], 1);

    }
    s3x_clkd_set_src_div_st(clkd, 1);
    s3x_clkd_set_gate_mask(clkd, mask, 1);
    return STATUS_OK;
}
/* this will disable all gates passed in mask,
    will not change rate & src */

static int s3x_clkd_disable_gate(S3x_ClkD *clkd,  UINT16_t mask)
{

    s3x_clkd_set_gate_mask(clkd, mask, 0);

    if(!(clkd->gate_val) && (!(clkd->flags & HW_GATED)))
        s3x_disable_src_div(clkd);

    return STATUS_OK;
}

static int s3x_clkd_handle_gate(S3x_ClkD *clkd,  UINT16_t mask, int en)
{
    int ret;
    if (en)
        ret = s3x_clkd_enable_gate(clkd, mask);
    else
        ret = s3x_clkd_disable_gate(clkd, mask);
    return ret;
}

int _S3x_clkd_set_cpu_qos_rate(S3x_ClkD *clkd, UINT32_t rate)
{
    UINT32_t crate;

    crate = S3x_Clkd_Get_Cpu_Rate();
    if (crate < rate)
        S3x_Clkd_Set_Cpu_Rate(rate);
    return 0;
}

int _S3x_register_qos_node(UINT32_t id)
{
     UINT8_t domain_id;
     int ret = STATUS_OK;

     if (id >= S3X_MAX_CLK){
        ret = -EINVALID_VAL;
        goto exit;
    }

    domain_id = DOMAIN_ID(id);
    ret = S3x_register_clkd_qnode(&S3clk[domain_id]);
exit:
    return ret;
}

int _S3x_set_qos_req(UINT32_t id, QOS_REQ_TYPE req, UINT32_t val)
{
     UINT8_t domain_id;
     int ret = STATUS_OK;

    if (id >= S3X_MAX_CLK){
        ret = -EINVALID_VAL;
        goto exit;
    }

    domain_id = DOMAIN_ID(id);
    ret = S3x_set_clkd_qos_req(&S3clk[domain_id], req, val);
exit:
    return ret;

}

int _S3x_get_qos_req(UINT32_t id, QOS_REQ_TYPE req)
{
     UINT8_t domain_id;
     int ret = STATUS_OK;

    if (id >= S3X_MAX_CLK){
        ret = -EINVALID_VAL;
        goto exit;
    }

    domain_id = DOMAIN_ID(id);
    ret = S3x_get_clkd_qos_req(&S3clk[domain_id], req);
exit:
    return ret;

}
/* HAL API to Enable clk
*/
int _S3x_Clk_Enable(UINT32_t id)
{
    UINT8_t domain_id, use_cnt, gindex;
    UINT16_t gate_mask;
    int ret = STATUS_OK;
    S3x_Pi *pi;
    UINT8_t lock = 0;

    if (id >= S3X_MAX_CLK){
        ret = -EINVALID_VAL;
        goto exit1;
    }

    gate_mask = GATE_MASK(id);
    domain_id = DOMAIN_ID(id);
    if (xTaskGetSchedulerState() != taskSCHEDULER_SUSPENDED)
    {
        xSemaphoreTake(S3clk[domain_id].clkd_sem, portMAX_DELAY);
        lock = 1;
    }

    use_cnt = USECNT(id);
    if(use_cnt && use_cnt != (0xFF)){
        goto exit;

    } else {
        use_cnt = 0;
        pi = s3x_get_pi(id);
        if (pi)
        {
            gindex = s3x_get_pi_gindex(id);
            if (gindex != INVALID_INDEX)
                s3x_pi_set_clkg_st(pi, gindex, GATE_EN);
            s3x_pi_set_active_st(pi);
        }
        ret = s3x_clkd_handle_gate(&S3clk[domain_id], gate_mask, 1);
    }
exit:
    if (!ret){
        if (use_cnt == (0xFE)){
            //QL_LOG_ERR_150K("ERR USE CNT REACHED MAX VALUE for ID %d\n", id);
        } else
            use_cnt++;
        SET_USECNT(id, use_cnt);
    }
    if (lock)
        xSemaphoreGive(S3clk[domain_id].clkd_sem);
exit1:
    return ret;
}

/* HAL API to Disable clk
*/

int _S3x_Clk_Disable(UINT32_t id)
{
    UINT8_t domain_id, use_cnt, gindex;
    UINT16_t gate_mask;
    int ret = STATUS_OK;
    S3x_Pi *pi;
    UINT8_t lock = 0;

    if (id >= S3X_MAX_CLK) {
        ret = -EINVALID_VAL;
        goto exit2;
    }

    gate_mask = GATE_MASK(id);
    domain_id = DOMAIN_ID(id);
    if (xTaskGetSchedulerState() != taskSCHEDULER_SUSPENDED)
    {
        xSemaphoreTake(S3clk[domain_id].clkd_sem, portMAX_DELAY);
        lock = 1;
    }

    use_cnt = USECNT(id);
    if ((!use_cnt) || (use_cnt == 0xFF)){/*already disabled or nt enabled by user*/
        ret = -ENOT_ENABLE;
        goto exit1;
    }  else if (use_cnt > 1) {
        goto exit;
    }

    if(!(S3clk[domain_id].flags & KEEP_ON))
    {
        ret = s3x_clkd_handle_gate(&S3clk[domain_id], gate_mask, 0);

        pi = s3x_get_pi(id);
        if (pi)
        {
            gindex = s3x_get_pi_gindex(id);
            if (gindex != INVALID_INDEX)
                s3x_pi_set_clkg_st(pi, gindex, GATE_DIS);
            s3x_pi_set_cfg_st(pi);
        }
    }

exit:
    if(!ret) {
        use_cnt--;
        SET_USECNT(id, use_cnt);
    }

exit1:
    if (lock)
        xSemaphoreGive(S3clk[domain_id].clkd_sem);
exit2:
     return ret;
}

/* HAL API to set Status of clk
*  ret status 1 enable 0 disable
*/

int _S3x_Clk_Get_Status(UINT32_t id)
{
   UINT8_t domain_id;
   UINT16_t gate_mask;
   int ret;

    if (id >= S3X_MAX_CLK){
        ret = -EINVALID_VAL;
        goto exit;
    }
    gate_mask =  GATE_MASK(id);
    domain_id = DOMAIN_ID(id);

    ret = s3x_clkd_get_gate_mask_st(&S3clk[domain_id], gate_mask);
exit:
    return ret;
}
/* HAL API to get clock rate
*/

int _S3x_Clk_Get_Rate(UINT32_t id)
{
    UINT8_t domain_id;

    if (id >= S3X_MAX_CLK)
        return -EINVALID_VAL;

    domain_id = DOMAIN_ID(id);

    return s3x_clkd_grate(&S3clk[domain_id]);
}
int S3x_Get_CPU_CLOCK_HZ(void)
{
 return (_S3x_Clk_Get_Rate(S3X_M4_S0_S3_CLK));
}
/* HAL API to set clock rate
*/


int _S3x_Clk_Set_Rate(UINT32_t id, UINT32_t rate)
{
    UINT8_t domain_id, policy_mr;

    if (id >= S3X_MAX_CLK)
        return -EINVALID_VAL;

    domain_id = DOMAIN_ID(id);

    policy_mr = s3x_get_policy_max_rate(domain_id);

    if ((policy_mr == 0) || (policy_mr >= rate))
        return s3x_clkd_srate(&S3clk[domain_id], rate);
    else
        return ERATE_NOT_VALID_WITH_DFS;
}

int _S3x_Clk_Get_Usecnt(UINT32_t id)
{
    UINT8_t use_cnt;

    if (id >= S3X_MAX_CLK)
        return -EINVALID_VAL;

    use_cnt = USECNT(id);

    /*If not enabled by user, then return use_cnt as zero*/
    if(use_cnt == 0xFF)
    {
        use_cnt = 0;
    }
    return use_cnt;
}

void S3x_Clk_Init ()
{
    int i ;
    UINT16_t dmask;

    S3x_Clkd_Change_Hsosc(HSOSC_DEF_RATE);
    Hsosc.init = 1;

    for(i =0; i < S3clkd_size; i++) {
        S3clk[i].clkd_sem = xSemaphoreCreateBinary();
        configASSERT(S3clk[i].clkd_sem);
        snprintf( S3clk[i].clkd_sem_name,
                 sizeof(S3clk[i].clkd_sem_name)-1,
                 "Clk_%d",i );
        vQueueAddToRegistry(S3clk[i].clkd_sem, S3clk[i].clkd_sem_name );
        if (S3clk[i].clkd_sem != NULL)
            xSemaphoreGive(S3clk[i].clkd_sem);

        s3x_clkd_get_def_st(&S3clk[i]);
        s3x_get_ref_clkd_src(&S3clk[i]);
        s3x_clkd_grate(&S3clk[i]);

        if(S3clk[i].type == SRC_CLK)
            s3x_clkd_update_sd_src_rate(&S3clk[i]);
        if(S3clk[i].init_state.irate)
            s3x_clkd_srate(&S3clk[i],
                S3clk[i].init_state.irate);

        if(S3clk[i].init_state.imask)
        {
            if (S3clk[i].init_state.en)
            {
                dmask = ~S3clk[i].init_state.imask;
                s3x_clkd_handle_gate(&S3clk[i], dmask, 0);
            }
            s3x_clkd_handle_gate(&S3clk[i],
                S3clk[i].init_state.imask,
                S3clk[i].init_state.en);
        }
         s3x_clkd_grate(&S3clk[i]);

    }

}

#ifdef DEF_CRU_DBG

void S3x_Sel_CRU_Dbg(CRU_DBG_SEL cru, DMON_PAD pad)
{
    UINT32_t off;

    if (cru >= SEL_CRU_MAX)
        return;
    /* Sel A0 debug monitor routed from*/
    MISC_CTRL->SUBSYS_DBG_MON_SEL = 0;
    /* Sel CRU debug monitors*/
    MISC_CTRL->A0_DBG_MON_SEL = 1;

    if (pad && (pad < DPAD_MAX))
    {
        off = S3x_Clk_Dmon_Pad_Off[pad];
    }
    else
        off = PAD_12_OFF;

    /* Sel debug mon */
    MUX_WVAL(off, 0x2);

    /* Sel CRU */
    CRU->CRU_DEBUG = (cru & 0xF);
}

#endif
