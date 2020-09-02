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

#ifndef __EOSS3_HAL_FPGA_CLK_SYNC_H_
#define __EOSS3_HAL_FPGA_CLK_SYNC_H_
/*! \file eoss3_hal_fpga_clk_sync.h
 *
 * \brief This file contains HAL API for I2S Master ASSP Rx
 */

 /**
 * @brief return value definitions
 */
 
#include <stdint.h>

/**
 * @brief return value definitions
 */
#define HAL_FB_CLK_SYNC_RET_VAL		(0x0)
#define HAL_FB_CLK_SYNC_ERR			(0x20)

#define HAL_FB_CLK_SYNC_SUCCESS		(HAL_FB_CLK_SYNC_ERR + 0x0)

#define HAL_FB_CLK_SYNC_INIT_ERR		(HAL_FB_CLK_SYNC_ERR + 0x1)
#define HAL_FB_CLK_SYNC_START_ERR		(HAL_FB_CLK_SYNC_ERR + 0x2)
#define HAL_FB_CLK_SYNC_STOP_ERR		(HAL_FB_CLK_SYNC_ERR + 0x3)
#define HAL_FB_CLK_SYNC_ADJUST_ERR		(HAL_FB_CLK_SYNC_ERR + 0x4)

/* HS Oscillator frequency increase in multiple of 32 kHz */
#define HAL_HSOSC_ADJ_UNIT				(32768 * 1)


/* Sampling rate of word select clock, This is external clock from the other chip */
#define HAL_FB_EXT_CLK_48KHZ			48000 /* External clock is 48 kHz */
#define HAL_FB_EXT_CLK_32KHZ			32000 /* External clock is 32 kHz */
#define HAL_FB_EXT_CLK_16KHZ			16000 /* External clock is 16 kHz */

/**
 * @brief HSOSC clock synchronization structure
 */
typedef struct
{
    uint16_t sampling_freq;     /* Sampling frequency of external clock */
    uint32_t clk_osc; 		/* Clock oscialltor adjustment unit */
} FB_clk_sync_Cfg_t;

/**
 * @brief HAL_FB_Clk_Sync_init Clock synchronization initialization.
 *
 * @param[in] clksync_cfg, configurations for clock sync.
 *
 * @retval uint32_t, Returns a HAL_FB_CLK_SYNC_SUCCESS in case of success.
 *                  In case of failure returns HAL_FB_CLK_SYNC_INIT_ERR.
 *
 */
uint32_t HAL_FB_Clk_Sync_init (FB_clk_sync_Cfg_t clksync_cfg);

/**
 * @brief HAL_FB_Clk_Sync_start this API will enable the FB Clocks and enables the
 *              clock synchronization flag
 *
 * @param[in] void
 *
 * @retval uint32_t, Returns a HAL_FB_CLK_SYNC_SUCCESS in case of success.
 *                  In case of failure returns HAL_FB_CLK_SYNC_START_ERR.
 *
 */
uint32_t HAL_FB_Clk_Sync_start (void);

/**
 * @brief HAL_FB_Clk_Sync_adjust this API will check the sample count and re-adjust clock
 *          if there is any difference in the samples. This has to be called every time when
 *          the data arrives. To be called from dma handler.
 *
 * @param[out] difference in samples
 *
 * @retval uint32_t, Returns a HAL_FB_CLK_SYNC_SUCCESS in case of success.
 *                  In case of failure returns HAL_FB_CLK_SYNC_ERR.
 *
 */
uint32_t HAL_FB_Clk_Sync_adjust (int32_t *adjust_unit);

/**
 * @brief HAL_FB_Clk_Sync_stop This API will disable the FB Clock and disables
 *          clock synchronization
 *
 * @param[in] void
 *
 * @retval uint32_t, Returns a HAL_FB_CLK_SYNC_SUCCESS in case of success.
 *      In case of failure returns HAL_FB_CLK_SYNC_STOP_ERR.
 *
 */
uint32_t HAL_FB_Clk_Sync_stop (void);

/**
 * @brief HAL_FB_Clk_Sync_Reset_counter This API will reset the I2S, MIC counter
 *              and the clock synchronization starts again.
 *
 * @param[in] void
 *
 * @retval uint32_t, Returns a HAL_FB_CLK_SYNC_SUCCESS in case of success.
 *                  In case of failure returns HAL_FB_CLK_SYNC_ERR.
 *
 */
uint32_t HAL_FB_Clk_Sync_Reset_counter (void);

/**
 * @brief HAL_FB_Clk_Sync_uninit This API will uninit the clock synchronization
 * process.
 *
 * @param[in] void
 *
 * @retval uint32_t, Returns a HAL_FB_CLK_SYNC_SUCCESS in case of success.
 *                  In case of failure returns HAL_FB_CLK_SYNC_ERR.
 *
 */
uint32_t HAL_FB_Clk_Sync_uninit (void);
#endif /* __EOSS3_HAL_FPGA_CLK_SYNC_H_ */
