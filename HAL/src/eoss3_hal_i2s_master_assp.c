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
 *    File   : eoss3_hal_i2s_master_assp.c
 *    Purpose: This file contains HAL API for I2S Master ASSP
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#include "eoss3_hal_i2s.h"
#include "s3x_clock_hal.h"
#include "eoss3_hal_pad_config.h"
#include "eoss3_hal_audio.h"
#include "eoss3_hal_leds.h"
#include "Fw_global_config.h"
#include "dma_buffer.h"
#include "s3x_clock.h"

#ifdef ENABLE_I2S_MASTER_ASSP_RX
#define SRAM_START_ADDRESS_I2S  0x20000000

#define DATA_32KHZ_SAMPLING_FREQ    32
// I2S Clock Configuration for I2S_WD_CLK=32KHz
#define I2S_MASTER_BCLKDIV_32KHZ	1
#define I2S_MASTER_LRCDIV_32KHZ 	64
#define I2S_MASTER_IWL_32KHZ 		0

#define DATA_16KHZ_SAMPLING_FREQ    16
// I2S Clock Configuration for I2S_WD_CLK=16KHz
#define I2S_MASTER_BCLKDIV_16KHZ	2
#define I2S_MASTER_LRCDIV_16KHZ 	64
#define I2S_MASTER_IWL_16KHZ 		0

#define I2S_MASTER_AUD_DMAC_AHB             (1<<0) /* DMAC, AHB */
#define I2S_MASTER_AUD_PDM2PCM_LEFT         (1<<1) /* PDM2PC_Left, FIFO_0 */
#define I2S_MASTER_AUD_PDM2PCM_RIGHT        (1<<2) /* PDM2PC_Right, FIFO_1 */
#define I2S_MASTER_AUD_LPSD                 (1<<3) /* LPSD_HW */
#define I2S_MASTER_AUD_I2S_MASTER           (1<<4) /* I2S Master */
#define I2S_MASTER_AUD_APB_SLAVE            (1<<5) /* APB Slave, Mode detect, Bypass*/

// I2S Master Rx Structure
typedef struct
{
	uint32_t rx_index;
    uint32_t *rx_buffer;
	uint16_t buffer_half_size;
	uint8_t i2s_wd_clk;
	uint8_t ch_sel;
	uint8_t mono_sel;
} I2S_Master_Assp_Rx_t;

static I2S_Master_Assp_Rx_t i2s_master_assp_rx;
static HAL_I2S_Cb_Handler_t i2s_master_cb;
static bool RunDmaForever = false;
int dma_stop_en = 0;

// Pad Configurations for I2S Pins
static void HAL_I2S_Master_Assp_Configure_Pins(void)
{
    PadConfig xPadConf;

    /* 1 for pad10 and 2 for pad28 */
    IO_MUX->I2S_DATA_SELECT = 0x02;
    	
    /* PAD28 - I2S_DIN Mux Pin Settings */
    xPadConf.ucPin = PAD_28;
    xPadConf.ucFunc = PAD28_FUNC_SEL_I2S_DIN;
    xPadConf.ucCtrl = PAD_CTRL_SRC_A0;
    xPadConf.ucMode = PAD_MODE_INPUT_EN;
    xPadConf.ucPull = PAD_PULLUP;
    xPadConf.ucDrv = PAD_DRV_STRENGHT_4MA;
    xPadConf.ucSpeed = PAD_SLEW_RATE_SLOW;
    xPadConf.ucSmtTrg = PAD_SMT_TRIG_DIS;
    HAL_PAD_Config(&xPadConf);

	/* PAD29 - I2S_CKO Mux Pin Settings */    
    xPadConf.ucPin = PAD_29;
    xPadConf.ucFunc = PAD29_FUNC_SEL_I2S_CKO;
    xPadConf.ucCtrl = PAD_CTRL_SRC_A0;
    xPadConf.ucMode = PAD_MODE_OUTPUT_EN;
    xPadConf.ucPull = PAD_PULLUP;
    xPadConf.ucDrv = PAD_DRV_STRENGHT_4MA;
    xPadConf.ucSpeed = PAD_SLEW_RATE_SLOW;
    xPadConf.ucSmtTrg = PAD_SMT_TRIG_DIS;
    HAL_PAD_Config(&xPadConf);

	/* PAD30 - 	I2S_WD_CKO Mux Pin Settings */        
    xPadConf.ucPin = PAD_30;
    xPadConf.ucFunc = PAD30_FUNC_SEL_I2S_WD_CKO;
    xPadConf.ucCtrl = PAD_CTRL_SRC_A0;
    xPadConf.ucMode = PAD_MODE_OUTPUT_EN;
    xPadConf.ucPull = PAD_PULLUP;
    xPadConf.ucDrv = PAD_DRV_STRENGHT_4MA;
    xPadConf.ucSpeed = PAD_SLEW_RATE_SLOW;
    xPadConf.ucSmtTrg = PAD_SMT_TRIG_DIS;
    HAL_PAD_Config(&xPadConf);

	return;
}

void HAL_I2S_Master_Assp_WakeupAudioPD(void)
{   
	PMU->AUD_SRAM_SW_WU = (I2S_MASTER_AUD_PDM2PCM_LEFT | I2S_MASTER_AUD_APB_SLAVE);
    PMU->AUD_SRAM_SW_WU |= I2S_MASTER_AUD_DMAC_AHB;
    PMU->AUD_SRAM_SW_WU |= I2S_MASTER_AUD_LPSD;
    PMU->AUD_SRAM_SW_WU |= I2S_MASTER_AUD_I2S_MASTER;
	PMU->AUDIO_SW_PD = (I2S_MASTER_AUD_PDM2PCM_RIGHT); // keep unused PDs off
}
/**
 * This method sets up the audio clocks at startup
 */
static void HAL_I2S_Master_Assp_Set_Clk_Control(void)
{
    uint8_t   i2s_bclkdiv;
	uint8_t   i2s_lrcdiv;
	uint8_t   i2s_iwl;
	uint32_t   i2s_master_clk;

	// Configure clock divider clock gating. Touch only G's clock divider
	const uint32_t kClockDividerGClockGating = (0x1 << 6);
	CRU->CLK_DIVIDER_CLK_GATING |= kClockDividerGClockGating;
    
    HAL_I2S_Master_Assp_WakeupAudioPD();
	S3x_Clk_Enable(S3X_AUDIO_APB);
    S3x_Clk_Enable(S3X_I2S_A1_CLK);

    /* select BCLK, LR Clock, IWL based on required sampling rate */
	if (i2s_master_assp_rx.i2s_wd_clk == DATA_32KHZ_SAMPLING_FREQ)
	{
		i2s_bclkdiv = I2S_MASTER_BCLKDIV_32KHZ;
		i2s_lrcdiv = I2S_MASTER_LRCDIV_32KHZ;
		i2s_iwl	= I2S_MASTER_IWL_32KHZ;
	}
	else if (i2s_master_assp_rx.i2s_wd_clk == DATA_16KHZ_SAMPLING_FREQ)
	{
		i2s_bclkdiv = I2S_MASTER_BCLKDIV_16KHZ;
		i2s_lrcdiv = I2S_MASTER_LRCDIV_16KHZ;
		i2s_iwl = I2S_MASTER_IWL_16KHZ;
	}
	else
	{
		printf("Unsupported WD_CLK = %d \n", i2s_master_assp_rx.i2s_wd_clk);
		return;
	}
    
	gI2sConfig.setter(I2S_BCLKDIV, i2s_bclkdiv);
    gI2sConfig.setter(I2S_CLK_INV, 0);
    gI2sConfig.setter(I2S_LRCDIV, i2s_lrcdiv);
    gI2sConfig.setter(I2S_IWL, i2s_iwl);
    
	i2s_master_clk = (i2s_master_assp_rx.i2s_wd_clk*i2s_bclkdiv*i2s_lrcdiv)*1000;
    S3x_Clk_Set_Rate(S3X_I2S_MASTER, i2s_master_clk);
    //S3x_Clk_Set_Rate(S3X_I2S_MASTER, HSOSC_2MHZ);
    S3x_Clk_Enable(S3X_I2S_MASTER);
    //S3x_Clk_Enable(S3X_I2S_MASTER);
	#define AUDIO_SRAM_HW_DS_CFG (1<<8)
	PMU->GEN_PURPOSE_0 |= (AUDIO_SRAM_HW_DS_CFG);
    
    /* Hold the clock not to go into LPM*/
    S3x_Set_Qos_Req(S3X_I2S_MASTER, MIN_CPU_FREQ, C10_N1_CLK); 
}


static void HAL_I2S_Master_Assp_Dmac_Config(void)
{
	// Configure VOICE_DMAC_CONFIG register
	gVoiceDmacConfig.setter(DMAC_EN, 1);
	gVoiceDmacConfig.setter(DMAC_START, 0);
	gVoiceDmacConfig.setter(DMAC_STOP, 0);
	gVoiceDmacConfig.setter(AHB_RDY, 1);
	gVoiceDmacConfig.setter(AHB_BURST_LENGTH, 1); // 1=4words, 2=8words(=32bytes=1msec)
	gVoiceDmacConfig.setter(PING_PONG_MODE, 0);
	gVoiceDmacConfig.setter(STEREO_DUAL_BUF_MODE, 0);
	gVoiceDmacConfig.setter(VOICE_DMAC_BURST_SPD, 0);

    	// Configure PDM Core reg
	gPdmCoreConfig.setter(PDMCORE_EN, 1);		    // enable data streaming
	//gPdmCoreConfig.setter(SOFT_MUTE, pdm_soft_mute);		    // disable soft mute
	gPdmCoreConfig.setter(DIV_MODE, 0);			    // 0: use PDM left clk in sampler 1: PDM right clk
	gPdmCoreConfig.setter(S_CYCLES, 0);				// set number of pdm clock during gain setting changes and soft mute
	gPdmCoreConfig.setter(HPGAIN, 0);				// configure PDM_CLK frequency divisor
	gPdmCoreConfig.setter(ADCHPD, 1);				// 1: disable 0: enable high pass filter

	//gPdmCoreConfig.setter(MCLKDIV,MCLKDIV_val);		// configure PDM_CLK frequency divisor
	//gPdmCoreConfig.setter(SINC_RATE, SINC_RATE_val);// SINC Decimation rate
	#if 0
	gPdmCoreConfig.setter(PGA_L, 8);			    // Left Channel PGA gain
	gPdmCoreConfig.setter(PGA_R, 8);			    // Right Channel PGA gain
	#endif
	gPdmCoreConfig.setter(PGA_L, 8);			    // Left Channel PGA gain
	gPdmCoreConfig.setter(PGA_R, 8);			    // Right Channel PGA gain

	gPdmCoreConfig.setter(DMICK_DLY, 0);		    // input data sampling with PDM clock cycle delay
	gPdmCoreConfig.setter(DIV_WD_MODE, 0);		    // Status IN detection window

    
	// configure VOICE_DMAC_LEN register
//#define SINGLE_BUFFER 1
#ifdef SINGLE_BUFFER
	gVoiceDmacLen.setter(DMAC_BLK_LEN, DMA_SINGLE_BUFFER_SIZE/2);
	gVoiceDmacLen.setter(DMAC_BUF_LEN, DMA_MULTI_BUFFER_SIZE/2);
    gVoiceDmacDstAddr.common.raw.write((int32_t)gDmaBuffer.mem);
#endif

	// configure VOICE_DMAC_DST_ADDR0 and ADDR1
	
#ifndef SINGLE_BUFFER
    gVoiceDmacLen.setter(DMAC_BLK_LEN, (i2s_master_assp_rx.buffer_half_size/2));
    gVoiceDmacLen.setter(DMAC_BUF_LEN, (i2s_master_assp_rx.buffer_half_size));
    if((int)i2s_master_assp_rx.rx_buffer < SRAM_START_ADDRESS_I2S)
        gVoiceDmacDstAddr.common.raw.write((int32_t)i2s_master_assp_rx.rx_buffer + SRAM_START_ADDRESS_I2S);
    else
      gVoiceDmacDstAddr.common.raw.write((int32_t)i2s_master_assp_rx.rx_buffer);
#endif    
	//printf("Address:%x\r\n", gDmaBuffer.mem);

}

// Configuration for I2S parameters such as I2S Interface Selection, I2S Clock Settings, etc.
static void HAL_I2S_Master_Assp_Configure(void)
{
	//Enable the Clock to enable access to Audio Voice_Config register
    HAL_I2S_Master_Assp_Set_Clk_Control();
	
	//Default Voice Config Register Configuration for I2S Master

    gVoiceConfig.setter(DMIC_SEL, 0x1);
    gVoiceConfig.setter(LPSD_SEL, 0x0);
    gVoiceConfig.setter(FIFO_0_CLEAR, 0);
	gVoiceConfig.setter(FIFO_1_CLEAR, 0);
  
	//Configure voice config reg to select I2S Interface
	gVoiceConfig.setter(DMIC_SEL, 1);

	//Disable LPSD as it is not required
	gVoiceConfig.setter(LPSD_SEL, 0);

    	// Flush the Voice FIFOs
	gVoiceConfig.setter(FIFO_0_CLEAR, 1);
	gVoiceConfig.setter(FIFO_1_CLEAR, 1);
    
	// Disable Down Sampling as it is not required
	gVoiceConfig.setter(I2S_DS_EN, 0);

	// Disable I2S PGA Gain is not required
	gVoiceConfig.setter(I2S_PGA_EN, 0);


    gVoiceConfig.setter(DIV_AP, 0x0);
	gVoiceConfig.setter(DIV_WD, 0x0);
    gVoiceConfig.setter(LPSD_NO, 0x0);

			
	if (i2s_master_assp_rx.ch_sel== I2S_CHANNELS_MONO)
		gVoiceConfig.setter(MODE_SEL, 0);
	
	if (i2s_master_assp_rx.mono_sel == I2S_CHANNEL_MONO_LEFT)
		gVoiceConfig.setter(MONO_CHN_SEL, 0);

	return;
}


//Starts DMA by performing the following sequence of operations:
// 	1. Set DMA Enable bit to 1
// 	2. Set DMA Start bit to 1
// 	3. Set AHB Ready to 1
static void HAL_I2S_Master_Assp_VoiceDmac_Start(void)
{
	if(RunDmaForever == true) {
			return;
	}

	if(gDmaStarted == true) {
			return;
	}

	// Enable the AUDIO DMA Clock 
	S3x_Clk_Enable(S3X_AUDIO_DMA_CLK);
	
	gDmaStarted = true;

    gVoiceConfig.setter(DMAC_BLK_DONE_MASK, 0);
	gVoiceConfig.setter(DMAC_BUF_DONE_MASK, 0);

    gVoiceDmacConfig.setter(DMAC_EN, 1);
    gVoiceDmacConfig.setter(DMAC_STOP, 0);
	gVoiceDmacConfig.setter(AHB_RDY, 1);
	gVoiceDmacConfig.setter(DMAC_START, 1);

}

// Stops DMA by performing the following sequence of operations:
//	1. Set AHB Ready to 0
// 	2. Set DMA Stop bit to 1
//
// Note: AD1_SW_RESET is also performed here to switch FIFO from B to A.
static void HAL_I2S_Master_Assp_VoiceDmac_Stop( void )
{
	if(RunDmaForever == true) {
		return;
	}
	if(gDmaStarted == false) {
		return;
	}

   	gDmaStarted = false;

	gVoiceDmacConfig.setter(AHB_RDY, 0);
	gVoiceDmacConfig.setter(DMAC_STOP, 1);
    gVoiceDmacConfig.setter(DMAC_EN, 0);

    gVoiceConfig.setter(DMAC_BLK_DONE_MASK, 1);
	gVoiceConfig.setter(DMAC_BUF_DONE_MASK, 1);

	CRU->AUDIO_MISC_SW_RST = CRU->AUDIO_MISC_SW_RST | (0x3);
	CRU->AUDIO_MISC_SW_RST = CRU->AUDIO_MISC_SW_RST & (UINT32_t)(~(0x3));


	S3x_Clk_Disable(S3X_AUDIO_DMA_CLK);
}

static void HAL_I2S_Master_Enable_DMA_Intr(void)
{ 
    NVIC_ClearPendingIRQ(Dmac0_Block_Done_IRQn);
    NVIC_EnableIRQ(Dmac0_Block_Done_IRQn);
    NVIC_ClearPendingIRQ(Dmac0_Buffer_Done_IRQn);
    NVIC_EnableIRQ(Dmac0_Buffer_Done_IRQn);
    
    // clear interrupts
    INTR_CTRL->OTHER_INTR &= DMIC_VOICE_DET;
    INTR_CTRL->OTHER_INTR_EN_M4 |= DMIC_VOICE_DET;
}

static void HAL_I2S_Master_Disable_DMA_Intr( void )
{
    NVIC_DisableIRQ(Dmac0_Block_Done_IRQn);
    NVIC_DisableIRQ(Dmac0_Buffer_Done_IRQn);
}

// Start I2S Master Rx
static uint32_t HAL_I2S_Master_Assp_Rx_Start (uint32_t *p_rx_buffer, uint32_t *p_tx_buffer, uint16_t buffer_size)
{
	configASSERT(p_rx_buffer);

    if (buffer_size <= 0)
        return HAL_I2S_RX_ERROR;

	i2s_master_assp_rx.rx_buffer = p_rx_buffer;
	i2s_master_assp_rx.buffer_half_size = buffer_size/2;
	i2s_master_assp_rx.rx_index = 0;
	
	// Configure I2S parameters for start
	HAL_I2S_Master_Assp_Configure();

	// Start the Voice DMA
    HAL_I2S_Master_Assp_Dmac_Config();

    HAL_I2S_Master_Assp_VoiceDmac_Start();
    
	// Unmask the Voice DMA Interrupts
	NVIC_SetPriority (Dmac0_Block_Done_IRQn, 6);
	NVIC_SetPriority (Dmac0_Buffer_Done_IRQn, 6);
    
	gVoiceConfig.setter(DMAC_BLK_DONE_MASK, 0);
	gVoiceConfig.setter(DMAC_BUF_DONE_MASK, 0);
	
    
    HAL_I2S_Master_Enable_DMA_Intr();
	return HAL_I2S_SUCCESS;	
}

// Stop I2S Master Rx
static void HAL_I2S_Master_Assp_Rx_Stop (void)
{
    HAL_I2S_Master_Disable_DMA_Intr();
	// Stop the Voice DMA
	HAL_I2S_Master_Assp_VoiceDmac_Stop();

	// Reset the rx structure
	i2s_master_assp_rx.rx_buffer = 0;
	i2s_master_assp_rx.buffer_half_size = 0;
	i2s_master_assp_rx.rx_index = 0;

    /* Release the clock when done */
    S3x_Clear_Qos_Req(S3X_I2S_MASTER, MIN_HSOSC_FREQ);
    
	// Disable I2S Master Clock
    S3x_Clk_Disable(S3X_AUDIO_APB);
    S3x_Clk_Disable(S3X_I2S_A1_CLK);

	S3x_Clk_Disable(S3X_I2S_MASTER);
	return;
}

// Initialize I2S Master
static uint32_t HAL_I2S_Master_Assp_Init (I2S_Config_t *p_i2s_cfg, HAL_I2S_Cb_Handler_t handler)
{	
	configASSERT(handler);

	// Check for the validity of I2S configuration
	if (p_i2s_cfg == NULL)
	{
		printf("I2S] I2S Configuration are not valid \n");
		return HAL_I2S_BAD_PARAMETER;
	}

	//Check the validity as well as save the I2S Configuration
	if (p_i2s_cfg->ch_sel == I2S_CHANNELS_STEREO)
        i2s_master_assp_rx.ch_sel  = I2S_CHANNELS_STEREO;
    else if (p_i2s_cfg->ch_sel == I2S_CHANNELS_MONO)
        i2s_master_assp_rx.ch_sel = I2S_CHANNELS_MONO;
    else
	{
		printf("[I2S]  Wrong Channel Select %d \n", p_i2s_cfg->ch_sel);
		return HAL_I2S_BAD_PARAMETER;
	}
	
	if (p_i2s_cfg->i2s_wd_clk == DATA_16KHZ_SAMPLING_FREQ)
        i2s_master_assp_rx.i2s_wd_clk = DATA_16KHZ_SAMPLING_FREQ;
    else if (p_i2s_cfg->i2s_wd_clk == DATA_32KHZ_SAMPLING_FREQ)
        i2s_master_assp_rx.i2s_wd_clk = DATA_32KHZ_SAMPLING_FREQ;
    else
	{
		printf("[I2S] Wrong Word Select %d \n", p_i2s_cfg->i2s_wd_clk);
		return HAL_I2S_BAD_PARAMETER;
	}

    if(p_i2s_cfg->mono_sel == I2S_CHANNEL_MONO_LEFT)
        i2s_master_assp_rx.mono_sel = I2S_CHANNEL_MONO_LEFT;
    else if(p_i2s_cfg->mono_sel == I2S_CHANNEL_MONO_RIGHT)
        i2s_master_assp_rx.mono_sel = I2S_CHANNEL_MONO_RIGHT;
    else
    {
        printf("I2S] Wrong Left/Right ch Select %d \n", p_i2s_cfg->mono_sel);
        return HAL_I2S_BAD_PARAMETER;
    }
	
	// Mux Pin Settings for I2S Master Pins
	HAL_I2S_Master_Assp_Configure_Pins();

	// Store the I2S Callback handler
	i2s_master_cb = handler;
    S3x_Register_Qos_Node(S3X_I2S_MASTER); 

	return HAL_I2S_SUCCESS;
}

// Uninitialize I2S Master
static void HAL_I2S_Master_Assp_Uninit(void)
{
	
	//HAL_I2S_Master_Assp_Rx_Stop();
	
	//Save the I2S Configuration
	i2s_master_assp_rx.i2s_wd_clk = 0;
	i2s_master_cb = NULL;
	
	return;
}

// IRQ Handler for Voice DMAC Block Done Interrupt
void HAL_I2S_Master_Assp_BlkDone_Hndlr(void)
{
	if (gDmaStarted == true)
	{	
		// Invoke the user handler callback	
		i2s_master_cb(I2S_MASTER_ASSP_RX, (i2s_master_assp_rx.rx_buffer + 
                      ((i2s_master_assp_rx.rx_index*i2s_master_assp_rx.buffer_half_size/2))),
                      NULL, i2s_master_assp_rx.buffer_half_size);

		if (0 == i2s_master_assp_rx.rx_index)
			i2s_master_assp_rx.rx_index = 1;
        else
            i2s_master_assp_rx.rx_index = 0;

		NVIC_ClearPendingIRQ(Dmac0_Block_Done_IRQn);
	}
}

// IRQ Handler for Voice DMAC Buffer Done Interrupt
void HAL_I2S_Master_Assp_BufDone_Hndlr(void)
{
	if (gDmaStarted == true)
	{
		i2s_master_assp_rx.rx_index = 0;
		// Reset DMA to start form the starting address
		gVoiceDmacConfig.setter(DMAC_START, 1);
		NVIC_ClearPendingIRQ(Dmac0_Buffer_Done_IRQn);
	}
}

uint32_t HAL_I2S_Master_Assp_Register(void)
{
    uint32_t ret_val = HAL_I2S_SUCCESS;
    
    I2S_Drv_t i2s_drv_fn;
    uint8_t i2s_id = I2S_MASTER_ASSP_RX;
    
    i2s_drv_fn.initfn = HAL_I2S_Master_Assp_Init;
    i2s_drv_fn.bufferfn = HAL_I2S_Master_Assp_Rx_Start;
    i2s_drv_fn.stopfn = HAL_I2S_Master_Assp_Rx_Stop;
    i2s_drv_fn.un_initfn = HAL_I2S_Master_Assp_Uninit;
    
    // Register to I2S Driver
    ret_val = HAL_I2S_Register_Driver(i2s_id, i2s_drv_fn);
	if (ret_val != HAL_I2S_SUCCESS)
		printf("HAL I2S Driver Registration Failed \n");
	
    return ret_val;
}
#endif /* ENABLE_I2S_MASTER_ASSP_RX */
