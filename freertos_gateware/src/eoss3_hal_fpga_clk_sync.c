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
 *    File   : eoss3_hal_fpga_clk_sync.c
 *    Purpose: This file contains HAL API for I2S Master ASSP Rx
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#include "string.h"
#include "s3x_clock_hal.h"
#include "eoss3_hal_pad_config.h"
#include "Fw_global_config.h"
#include "s3x_clock.h"
#include "eoss3_hal_fpga_clk_sync.h"

#ifdef FB_ENABLE_CLK_SYNC
#define FB_I2S_MIC_CNT_BASE		(FPGA_PERIPH_BASE + 0x0100)

/**
 * @brief Driver state.
 */
#define CLK_SYNC_STATE_UNINITIALIZED     (0x0)  /* Uninitialized. */
#define CLK_SYNC_STATE_INITIALIZED       (0x1)  /* Initialized but powered off. */
#define CLK_SYNC_STATE_STOP              (0x2)  /* Initialized, but stopped */
#define CLK_SYNC_STATE_START             (0x3)  /* START condition, ongoing transaction */

#define ENABLE_16KHZ_COUNTER             (0x2) /* write 2 in fpga clk_sel register to enable 16kHz counter */
#define ENABLE_32KHZ_COUNTER             (0x1) /* write 1 in fpga clk_sel register to enable 16kHz counter */
#define ENABLE_48KHZ_COUNTER             (0x0) /* write 0 in fpga clk_sel register to enable 16kHz counter */

#define FB_CLK_SYNC_COUNTER_EN           (0x1) /* FPGA clock sync counter enable */
#define FB_CLK_SYNC_COUNTER_DIS          (0x0) /* FPGA clock sync counter disable */

typedef struct
{
    /* Sync counter disable(0x0)/enable(0x1) register */
	__IO uint32_t I2S_MIC_SYNC_EN;			/*0x00 */
	/* clock sel 16 kHz(0x2), 32 kHz(0x1), 48 kHz (0x0)*/
	__IO uint32_t I2S_MIC_SYNC_CLK_SEL;			/*0x04 */
    /* Counter based on external I2S WS Clock*/
	__IO uint32_t I2S_DATA_CNT;			/*0x08 */
    /* Counter based on MIC data clock */
	__IO uint32_t MIC_DATA_CNT;			/*0x0C */
} I2S_MIC_CNT_TypeDef;

#define FB_I2S_MIC_CNT				((I2S_MIC_CNT_TypeDef *) FB_I2S_MIC_CNT_BASE)

typedef struct {
	volatile uint32_t clksync_i2s_cnt; /* clock I2S count retrieved from FPGA */
	volatile uint32_t clksync_mic_cnt; /* clock MIC count retrieved from FPGA */
	
	uint32_t clksync_brick_cnt;        /* Brick count */
	int16_t clksync_prev_cnt_offset; /* Previous count difference */
	int16_t clksync_cur_cnt_offset; /* Current count difference */
	
	int16_t clksync_rate; /* The rate of change between previous and current offsets */

	int8_t clksync_action_local;
	int8_t clksync_global_input;
	int8_t clksync_action_global;
	int8_t clksync_wait_for_brick;
	
	int8_t clksync_dead_band;
}clksync_clk_sync_t;

static clksync_clk_sync_t clk_sync_data;

static FB_clk_sync_Cfg_t clk_sync_cfg;

static uint8_t clk_sync_state = CLK_SYNC_STATE_UNINITIALIZED;
static uint8_t clk_sync_state_en = 0;

__STATIC_INLINE void HAL_FB_Clk_Sync_enable(uint32_t enable)
{
    FB_I2S_MIC_CNT->I2S_MIC_SYNC_EN = enable;
}

__STATIC_INLINE void HAL_FB_Clk_Set_sampling_freq(uint32_t sampling_freq)
{
    FB_I2S_MIC_CNT->I2S_MIC_SYNC_CLK_SEL = sampling_freq;
}

__STATIC_INLINE uint32_t HAL_FB_Clk_Get_MIC_count(void)
{
    return FB_I2S_MIC_CNT->MIC_DATA_CNT;
}

__STATIC_INLINE uint32_t HAL_FB_Clk_Get_I2S_count(void)
{
    return FB_I2S_MIC_CNT->I2S_DATA_CNT;
}

/* Configure EXTERNAL WD I2S CLK as input. This will be taken in by FPGA to 
    count number of samples Rx/Tx based on frequency */
static void HAL_FB_I2S_WD_Clk_Config(void)
{
    PadConfig xPadConf;
    /* WD_CLK left/right sync clock */
    xPadConf.ucPin = PAD_25;
    xPadConf.ucFunc = PAD25_FUNC_SEL_FBIO_25;
    xPadConf.ucCtrl = PAD_CTRL_SRC_FPGA;
    xPadConf.ucMode = PAD_MODE_INPUT_EN;
    xPadConf.ucPull = PAD_NOPULL;
    xPadConf.ucDrv = PAD_DRV_STRENGHT_4MA;
    xPadConf.ucSpeed = PAD_SLEW_RATE_SLOW;
    xPadConf.ucSmtTrg = PAD_SMT_TRIG_DIS;

    HAL_PAD_Config(&xPadConf);
}

//#define DEBUG_OFFSET 1
#ifdef DEBUG_OFFSET
#define MAX_NUM_OFBUFF  100
int32_t debugbuff[MAX_NUM_OFBUFF][10];
static int32_t count = 0;
#endif
static int32_t HAL_FB_Clksync_Algorithm(void)
{
	clk_sync_data.clksync_brick_cnt++;
	
	//get clksync counter values for i2s and mic data
	clk_sync_data.clksync_i2s_cnt = HAL_FB_Clk_Get_I2S_count();
	clk_sync_data.clksync_mic_cnt = HAL_FB_Clk_Get_MIC_count();

	clk_sync_data.clksync_cur_cnt_offset = clk_sync_data.clksync_mic_cnt - clk_sync_data.clksync_i2s_cnt;
	
	//get the rate of change between current and previous offset
	clk_sync_data.clksync_rate = clk_sync_data.clksync_cur_cnt_offset - clk_sync_data.clksync_prev_cnt_offset;
	
	if(clk_sync_data.clksync_rate > 0)
		clk_sync_data.clksync_action_local = -1;
	else if(clk_sync_data.clksync_rate < 0)
		clk_sync_data.clksync_action_local = 1;
	else
		clk_sync_data.clksync_action_local = 0;
	
	if(clk_sync_data.clksync_cur_cnt_offset > clk_sync_data.clksync_dead_band)
		clk_sync_data.clksync_global_input = -1;
	else if(clk_sync_data.clksync_cur_cnt_offset < -clk_sync_data.clksync_dead_band)
		clk_sync_data.clksync_global_input = 1;
	else
		clk_sync_data.clksync_global_input = 0;
	
	clk_sync_data.clksync_action_global = clk_sync_data.clksync_global_input + clk_sync_data.clksync_action_local;
    
	clk_sync_data.clksync_prev_cnt_offset = clk_sync_data.clksync_cur_cnt_offset;

	if(clk_sync_data.clksync_action_global == 0)
		return clk_sync_data.clksync_cur_cnt_offset;
#if 0
    int32_t hs_freq;
    /* Change I2S Master Clock frequency */
    hs_freq = S3x_Clk_Get_Rate(S3X_I2S_MASTER);
    
    hs_freq += (clk_sync_data.clksync_action_global * clk_sync_cfg.clk_osc);
    S3x_Clk_Set_Rate(S3X_I2S_MASTER, hs_freq);
#endif
#if 1
    int32_t hs_freq;
    /* Change HS OScillator Clock frequency */
	hs_freq = OSC_GET_FREQ_INC();

	hs_freq += (clk_sync_data.clksync_action_global * clk_sync_cfg.clk_osc);
        if(hs_freq < OSC_MAXIMMUM_FREQ && hs_freq > OSC_MINIMUM_FREQ)
        {
		AIP->OSC_CTRL_0 |= AIP_OSC_CTRL_EN; //enable the Clk. is this needed??
		OSC_SET_FREQ_INC(hs_freq);  //Set frequency
        }
#endif
#ifdef DEBUG_OFFSET
    /* This is for debug only */
    if(count < MAX_NUM_OFBUFF)
    {
        //int32_t tick_val = xTaskGetTickCountFromISR();
        int32_t tick_val = xTaskGetTickCount();
        debugbuff[count][0] = clk_sync_data.clksync_i2s_cnt;
        debugbuff[count][1] = clk_sync_data.clksync_mic_cnt;
        debugbuff[count][2] = clk_sync_data.clksync_cur_cnt_offset;
        debugbuff[count][3] = clk_sync_data.clksync_rate;
        debugbuff[count][4] = clk_sync_data.clksync_action_local;
        debugbuff[count][5] = clk_sync_data.clksync_global_input;
        debugbuff[count][6] = clk_sync_data.clksync_action_global;
        debugbuff[count][7] = tick_val;
        debugbuff[count][8] = S3x_Clk_Get_Rate(S3X_I2S_MASTER);
        debugbuff[count][9] = hs_freq;
        
        count++;
    }
    else
    {
        while(1);
    }
#endif
	return clk_sync_data.clksync_cur_cnt_offset;
}


uint32_t HAL_FB_Clk_Sync_init (FB_clk_sync_Cfg_t clksync_cfg)
{
    if(clk_sync_state != CLK_SYNC_STATE_UNINITIALIZED)
    {
        printf("[CLKSYNC] %s in wrong state %d\n", __func__, clk_sync_state);
        return HAL_FB_CLK_SYNC_INIT_ERR;
    }
    if((clksync_cfg.clk_osc%32768) != 0)
    {
        printf("[CLKSYNC] Wrong HSOSC adjust unit %d\n", clksync_cfg.clk_osc);
        return HAL_FB_CLK_SYNC_INIT_ERR;
    }
    clk_sync_cfg.clk_osc = clksync_cfg.clk_osc;
    
    if(clksync_cfg.sampling_freq == HAL_FB_EXT_CLK_16KHZ)
        clk_sync_cfg.sampling_freq = clksync_cfg.sampling_freq;
    else if(clksync_cfg.sampling_freq == HAL_FB_EXT_CLK_32KHZ)
        clk_sync_cfg.sampling_freq = clksync_cfg.sampling_freq;
    else if(clksync_cfg.sampling_freq == HAL_FB_EXT_CLK_32KHZ)
        clk_sync_cfg.sampling_freq = clksync_cfg.sampling_freq;
    else
    {
        printf("[CLKSYNC] Wrong sampling frequency %d\n", clksync_cfg.sampling_freq);
        return HAL_FB_CLK_SYNC_INIT_ERR;
    }

    /* Initialize structure to 0 */
    memset (&clk_sync_data, 0, sizeof(clk_sync_data));
    /* set deadband as true */
    clk_sync_data.clksync_dead_band = 2;
    /* Configure I2S WD CLK pin for fpga i/p */
    HAL_FB_I2S_WD_Clk_Config();
    
    clk_sync_state = CLK_SYNC_STATE_INITIALIZED;
    return HAL_FB_CLK_SYNC_SUCCESS;
}

uint32_t HAL_FB_Clk_Sync_start (void)
{
    if((clk_sync_state == CLK_SYNC_STATE_UNINITIALIZED) || (clk_sync_state == CLK_SYNC_STATE_START))
    {
        printf("[CLKSYNC] %s in wrong state %d\n", __func__, clk_sync_state);
        return HAL_FB_CLK_SYNC_START_ERR;
    }
    /* Enable the FPGA clocks */
    S3x_Clk_Enable(S3X_FB_16_CLK);
    S3x_Clk_Enable(S3X_FB_21_CLK);

    if(clk_sync_cfg.sampling_freq == HAL_FB_EXT_CLK_16KHZ)
        HAL_FB_Clk_Set_sampling_freq(ENABLE_16KHZ_COUNTER);
    else if(clk_sync_cfg.sampling_freq == HAL_FB_EXT_CLK_32KHZ)
        HAL_FB_Clk_Set_sampling_freq(ENABLE_32KHZ_COUNTER);
    else if(clk_sync_cfg.sampling_freq == HAL_FB_EXT_CLK_32KHZ)
        HAL_FB_Clk_Set_sampling_freq(ENABLE_48KHZ_COUNTER);
 
    clk_sync_state = CLK_SYNC_STATE_START;
    
    /* set deadband as true */
    clk_sync_data.clksync_dead_band = 2;
    clk_sync_state_en = 0;
    return HAL_FB_CLK_SYNC_SUCCESS;
}

uint32_t HAL_FB_Clk_Sync_adjust (int32_t *adjust_unit)
{
    int32_t offset_diff;

    if(clk_sync_state != CLK_SYNC_STATE_START)
    {
        printf("[CLKSYNC] %s in wrong state %d\n", __func__, clk_sync_state);
        return HAL_FB_CLK_SYNC_ADJUST_ERR;
    }

    if(clk_sync_state_en == 0)
    {
        /* call for the first time to enable the counter */
        HAL_FB_Clk_Sync_enable(FB_CLK_SYNC_COUNTER_DIS);
        
        HAL_FB_Clk_Sync_enable(FB_CLK_SYNC_COUNTER_EN);
        clk_sync_state_en++;
    }
    else
        offset_diff = HAL_FB_Clksync_Algorithm();
    if(adjust_unit != NULL)
        *adjust_unit = offset_diff;
    return HAL_FB_CLK_SYNC_SUCCESS;
}

uint32_t HAL_FB_Clk_Sync_stop (void)
{
    if(clk_sync_state != CLK_SYNC_STATE_START)
    {
        printf("[CLKSYNC] %s in wrong state %d\n", __func__, clk_sync_state);
        return HAL_FB_CLK_SYNC_STOP_ERR;
    }
    clk_sync_state_en = 0;
    HAL_FB_Clk_Sync_enable(FB_CLK_SYNC_COUNTER_DIS);
    /* Initialize structure to 0 */
    memset (&clk_sync_data, 0, sizeof(clk_sync_data));
    
    /* set deadband as true */
    clk_sync_data.clksync_dead_band = 2;
    /* Disable the FPGA clocks */
    S3x_Clk_Disable(S3X_FB_16_CLK);
    S3x_Clk_Disable(S3X_FB_21_CLK);
    
    clk_sync_state = CLK_SYNC_STATE_STOP;

    return HAL_FB_CLK_SYNC_SUCCESS;
}

uint32_t HAL_FB_Clk_Sync_Reset_counter (void)
{
    if(clk_sync_state != CLK_SYNC_STATE_START)
    {
        printf("[CLKSYNC] %s in wrong state %d\n", __func__, clk_sync_state);
        return HAL_FB_CLK_SYNC_ERR;
    }

    HAL_FB_Clk_Sync_enable(FB_CLK_SYNC_COUNTER_DIS);
    /* Initialize structure to 0 */
    memset (&clk_sync_data, 0, sizeof(clk_sync_data));

    /* set deadband as true */
    clk_sync_data.clksync_dead_band = 2;
    if(clk_sync_cfg.sampling_freq == HAL_FB_EXT_CLK_16KHZ)
        HAL_FB_Clk_Set_sampling_freq(ENABLE_16KHZ_COUNTER);
    else if(clk_sync_cfg.sampling_freq == HAL_FB_EXT_CLK_32KHZ)
        HAL_FB_Clk_Set_sampling_freq(ENABLE_32KHZ_COUNTER);
    else if(clk_sync_cfg.sampling_freq == HAL_FB_EXT_CLK_32KHZ)
        HAL_FB_Clk_Set_sampling_freq(ENABLE_48KHZ_COUNTER);
    
    return HAL_FB_CLK_SYNC_SUCCESS;
}

uint32_t HAL_FB_Clk_Sync_uninit (void)
{
    if(clk_sync_state != CLK_SYNC_STATE_STOP)
        return HAL_FB_CLK_SYNC_ERR;
    /* Initialize structure to 0 */
    memset (&clk_sync_data, 0, sizeof(clk_sync_data));
    clk_sync_state = CLK_SYNC_STATE_UNINITIALIZED;

    return HAL_FB_CLK_SYNC_SUCCESS;
}
#endif /* FB_ENABLE_CLK_SYNC */
