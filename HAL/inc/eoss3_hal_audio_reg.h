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

/*
 * eoss3_hal_audio_reg.h
 *
 */

#ifndef __EOSS3_HAL_AUDIO_REG_H__
#define __EOSS3_HAL_AUDIO_REG_H__
#include <stdbool.h>

/* VOICE_DMAC_CONFIG Register */
typedef union ql_arch_voice_dmac_cfg_reg
{
    uint32_t voice_dmac_regVal;
    struct
    {
        uint32_t   dmac_en:1;    /* LSB */
        uint32_t   dmac_start:1;
        uint32_t   dmac_stop:1;
        uint32_t   ahb_rdy:1;
        uint32_t   ahb_burst_length:2;
        uint32_t   ping_pong_mode:1;
        uint32_t   stereo_dual_buf_mode:1;
        uint32_t   voice_dmac_burst_spd:8;
        uint32_t   RESERVED:16;
    }fields;
}ql_arch_voice_dmac_cfg_reg;

/* VOICE_CONFIG Register */
typedef union ql_arch_voice_config_reg
{
    uint32_t voice_config_regVal;
    struct
    {
        uint32_t dmic_sel : 1;
        uint32_t lpsd_sel : 1;
        uint32_t mode_sel : 1;
        uint32_t mono_chn_sel : 1;
        uint32_t i2s_ds_en : 1;
        uint32_t pdm_voice_scenario : 3;
        uint32_t pdm_mic_switch_to_ap : 1;
        uint32_t lpsd_use_dc_block : 1;
        uint32_t lpsd_mux : 1;
        uint32_t lpsd_no : 1;
        uint32_t i2s_pga_en : 1;
        uint32_t reserve1 : 2;
        uint32_t div_ap : 3;
        uint32_t div_wd : 6;
        uint32_t fifo_0_clear : 1;
        uint32_t fifo_1_clear : 1;
        uint32_t lpsd_voice_detected_mask : 1;
        uint32_t dmic_voice_detected_mask : 1;
        uint32_t dmac_blk_done_mask : 1;
        uint32_t dmac_buf_done_mask : 1;
        uint32_t ap_pdm_clk_on_mask : 1;
        uint32_t ap_pdm_clk_off_mask : 1;
    }fields;
}ql_arch_voice_config_reg;

/* I2S_CONFIG Register */
typedef union _t_ql_arch_i2s_config_reg
{
    uint32_t i2s_config_regVal;
    struct
    {
      uint32_t i2s_lrcdiv   : 12;  // 0-11
      uint32_t i2s_bclkdiv  : 6;  // 12-17
      uint32_t i2s_clk_inv  : 1;
      uint32_t i2s_iwl      : 1;
      uint32_t RESERVED     : 11;
    }fields;
}ql_arch_i2s_config_reg;

/* PDM_CORE_CONFIG register */
typedef union ql_arch_core_pdm_config_reg
{
    uint32_t pdm_core_config_regVal;
    struct
    {
        uint32_t pdmcore_en  :  1;
        uint32_t soft_mute   :  1;
        uint32_t div_mode    :  1;
        uint32_t s_cycles    :  3;
        uint32_t hpgain      :  4;
        uint32_t adchpd      :  1;
        uint32_t mclkdiv     :  2;
        uint32_t sinc_rate   :  7;
        uint32_t pga_l       :  5;
        uint32_t pga_r       :  5;
        uint32_t dmick_dly   :  1;
        uint32_t div_wd_mode :  1;
    }fields;
}ql_arch_core_pdm_config_reg;

/* LPSD_CONFIG Register */
typedef union ql_arch_lpsd_config_reg
{
    uint32_t lpsd_config_regVal;
    struct
    {
        uint32_t   lpsd_thd:16;
        uint32_t   lpsd_ratio_stop:8;
        uint32_t   lpsd_ratio_run:8;
    }fields;
}ql_arch_lpsd_config_reg;

#endif /* __EOSS3_HAL_AUDIO_REG_H__ */

