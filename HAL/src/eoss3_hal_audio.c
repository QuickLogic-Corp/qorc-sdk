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
*    File   : eoss3_hal_audio.c
*    Purpose:
*
*=========================================================*/
#include "Fw_global_config.h"

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <eoss3_dev.h>
#include "s3x_clock_hal.h"
#include "eoss3_hal_audio.h"
#include "eoss3_hal_audio_config.h"
#include "eoss3_hal_audio_reg.h"
#include "dma_buffer.h"
#include "qlsh_commands.h"
#include "process_ids.h"
#include "ql_util.h"
#include "dbg_uart.h"

#if AUDIO_LED_TEST
#include "leds.h"
#endif

#include "dbg_queue_monitor.h"

#ifdef ENABLE_VOICE_SOLUTION

struct  {
  uint8_t lpsd_mode;
  uint16_t  ksamplesToDrop;
  uint32_t pdm_pga_l_value;     
  uint32_t pdm_pga_r_value;     
  uint32_t pdm_soft_mute;       
  uint32_t pdm_voice_scenario;  
  uint32_t pdm_mic_switch_to_ap;
  uint32_t num_drop_count;
  uint32_t dma_isr_count;
  QAI_DataBlock_t  *pdata_block_prev;
} o_hal_info = {
 .lpsd_mode = 1,
 .ksamplesToDrop = 0,
 .pdm_pga_l_value      = PDM2PCM_PGA_GAIN_VAL,
 .pdm_pga_r_value      = PDM2PCM_PGA_GAIN_VAL,
 .pdm_soft_mute        = DEFAULT_SOFT_MUTE,
 .pdm_voice_scenario   = DEFAULT_PDM_VOICE_SCENARIO,
 .pdm_mic_switch_to_ap = DEFAULT_PDM_MIC_SWITCH_TO_AP,
 .dma_isr_count = 0,
 .pdata_block_prev = NULL
};

static uint32_t kchannels = 1;

HAL_Audio_Event_Notifier_t* pEventNotifier;

/* call this when the system restarts with WoS */
void eoss3_hal_audio_reset(void)
{
  o_hal_info.pdata_block_prev = NULL;
  o_hal_info.dma_isr_count = 0;
}

static inline void hangForever(const char *message)
{
  //printf("Message from caller: %s\r\n", message);
  while(true) {}
}

/*
 * This method wakes up the required audio clocks
 */
void enableAudioClocks(bool fUsingLeftChannel, bool fUsingRightChannel)
{
    S3x_Clk_Enable(S3X_PDM_LEFT); // LPSD always needs Left channel PDM clock
    if (fUsingRightChannel)
         S3x_Clk_Enable(S3X_PDM_RIGHT);
  return;
}

/**
 * This method shuts down the audio clocks
 */
void disableAudioClocks(bool fUsingLeftChannel, bool fUsingRightChannel)
{
    S3x_Clk_Disable(S3X_PDM_LEFT);
    if (fUsingRightChannel)
         S3x_Clk_Disable(S3X_PDM_RIGHT);
}


static void enableLPSDInt( void )
{
  NVIC_ClearPendingIRQ(Lpsd_IRQn);
  NVIC_EnableIRQ(Lpsd_IRQn);
  NVIC_ClearPendingIRQ(Lpsd_Voice_Off_IRQn);
  NVIC_EnableIRQ(Lpsd_Voice_Off_IRQn);
  INTR_CTRL->OTHER_INTR &= LPSD_VOICE_DET;
  INTR_CTRL->OTHER_INTR_EN_M4 |= LPSD_VOICE_DET;
}

static void disableLPSDInt( void )
{
  NVIC_DisableIRQ(Lpsd_IRQn);
  NVIC_DisableIRQ(Lpsd_Voice_Off_IRQn);
}

//TIM
void    HAL_Audio_LPSD_Int(bool fEnable) {
    if (fEnable) {
        enableLPSDInt();
    } else {
        disableLPSDInt();
    }
}

static void enableDmaInt( void )
{
  NVIC_ClearPendingIRQ(Dmic_IRQn);
  NVIC_EnableIRQ(Dmic_IRQn);
  NVIC_ClearPendingIRQ(Dmic_Voice_Off_IRQn);
  NVIC_EnableIRQ(Dmic_Voice_Off_IRQn);
  NVIC_ClearPendingIRQ(Dmac0_Block_Done_IRQn);
  /* new datablock architecture uses buffer done DMA interrupt only
  * no need to enable block done DMA interrupt
  */
  //    NVIC_EnableIRQ(Dmac0_Block_Done_IRQn);
  NVIC_ClearPendingIRQ(Dmac0_Buffer_Done_IRQn);
  NVIC_EnableIRQ(Dmac0_Buffer_Done_IRQn);
  // clear interrupts
  INTR_CTRL->OTHER_INTR &= DMIC_VOICE_DET;
  INTR_CTRL->OTHER_INTR_EN_M4 |= DMIC_VOICE_DET;
}

static void disableDmaInt( void )
{
  NVIC_DisableIRQ(Dmac0_Block_Done_IRQn);
  NVIC_DisableIRQ(Dmac0_Buffer_Done_IRQn);
  NVIC_DisableIRQ(Dmic_IRQn);
  NVIC_DisableIRQ(Dmic_Voice_Off_IRQn);
}

//static void enableAudioNvicInterrupts()
//{
//    if(o_hal_info.lpsd_mode)
//        enableLPSDInt();
//    enableDmaInt();
//}

static void disableAudioNvicInterrupts(void)
{
    disableLPSDInt();
    disableDmaInt();
    NVIC_DisableIRQ(Ap_Pdm_Clock_On_IRQn);
    NVIC_DisableIRQ(Ap_Pdm_Clock_Off_IRQn);
}

static void config_voice_interrupts(voice_config_interrupts_e mask )
{
  ql_arch_voice_config_reg reg;
  reg.voice_config_regVal = AUD->VOICE_CONFIG;
  reg.fields.lpsd_voice_detected_mask = (int)mask;
  reg.fields.dmic_voice_detected_mask = (int)mask;
  reg.fields.dmac_blk_done_mask = (int)mask;
  reg.fields.dmac_buf_done_mask = (int)mask;
  reg.fields.ap_pdm_clk_on_mask = (int)mask;
  reg.fields.ap_pdm_clk_off_mask = (int)mask;
  AUD->VOICE_CONFIG = reg.voice_config_regVal;
}

static void voice_dmac_dst_set_addr0(uint32_t value)
{
  AUD->VOICE_DMAC_DST_ADDR0 = SRAM_ADDR_TO_DMA_ADDR(value);
}
static void voice_dmac_set_start_bit(void)
{
    ql_arch_voice_dmac_cfg_reg reg_voice_dmac_config;
    
    reg_voice_dmac_config.voice_dmac_regVal = AUD->VOICE_DMAC_CONFIG;
    reg_voice_dmac_config.fields.dmac_start = 1;
    AUD->VOICE_DMAC_CONFIG = reg_voice_dmac_config.voice_dmac_regVal;
}
static uint32_t voice_dmac_dst_set_blk_len(uint32_t value)
{
  QL_AUDIO_HAL_ENTER_CRITICAL_SECTION();
  uint32_t reg = AUD->VOICE_DMAC_LEN;
  const uint32_t kMask = 0x0000FFFF;
  reg &= ~kMask;
  reg |= ( (value<<DMAC_BLK_LEN) & kMask);
  AUD->VOICE_DMAC_LEN = reg;
  QL_AUDIO_HAL_EXIT_CRITICAL_SECTION();
  return reg;
}

static uint32_t voice_dmac_dst_set_buf_len(uint32_t value)
{
  QL_AUDIO_HAL_ENTER_CRITICAL_SECTION();
  uint32_t reg = AUD->VOICE_DMAC_LEN;
  const uint32_t kMask = 0xFFFF0000;
  reg &= ~kMask;
  reg |= ( (value<<DMAC_BUF_LEN) & kMask);
  AUD->VOICE_DMAC_LEN = reg;
  //TIM Set offset in case using dual buffer mode
  // value is length in words -- we want offset to be 1/2 value, and the offset is in bytes so 4*value/2
  AUD->VOICE_DMAC_FIFO = ((2*value) << 16);
  QL_AUDIO_HAL_EXIT_CRITICAL_SECTION();
  return reg;
}

void HAL_Audio_LPSD_Enable(bool enable)
{
    ql_arch_voice_config_reg reg;
    reg.voice_config_regVal = AUD->VOICE_CONFIG;
    if(enable){
        reg.fields.lpsd_no = 0; // LPSD enable
    } else {
        reg.fields.lpsd_no = 1; // LPSD Disable
    }
    AUD->VOICE_CONFIG = reg.voice_config_regVal;
}

static void SetVoiceConfig(bool fUsingLeftChannel, bool fUsingRightChannel)
{
  QL_AUDIO_HAL_ENTER_CRITICAL_SECTION();
  ql_arch_voice_config_reg reg;
  reg.voice_config_regVal           = AUD->VOICE_CONFIG;
  reg.fields.dmic_sel               = 0;
  reg.fields.lpsd_sel               = 0;
  reg.fields.mode_sel               = (fUsingLeftChannel && fUsingRightChannel) ? 1 : 0;    // If using both channels, must be stereo
  reg.fields.mono_chn_sel           = (fUsingRightChannel) ? 1 : 0;                         // Assume stereo ignores
  reg.fields.lpsd_mux               = (fUsingRightChannel) ? 1 : 0;                         // Says which channel to use for LPSD analysis
  reg.fields.i2s_ds_en              = 0;
  reg.fields.lpsd_use_dc_block      = 0; 
  reg.fields.i2s_pga_en             = 0;
  reg.fields.div_ap                 = 0x2;
  reg.fields.div_wd                 = 0x10;
  reg.fields.pdm_voice_scenario     = o_hal_info.pdm_voice_scenario;
  reg.fields.pdm_mic_switch_to_ap   = o_hal_info.pdm_mic_switch_to_ap;
  reg.fields.fifo_0_clear           = 1;
  reg.fields.fifo_1_clear           = 1;
  AUD->VOICE_CONFIG                 = reg.voice_config_regVal;
  QL_AUDIO_HAL_EXIT_CRITICAL_SECTION();
  
  // Set number of channels for ISR
  kchannels = (fUsingLeftChannel && fUsingRightChannel) ? 2 : 1;
}

static void setPdmCoreConfig()
{
  QL_AUDIO_HAL_ENTER_CRITICAL_SECTION();
  ql_arch_core_pdm_config_reg reg;
  reg.pdm_core_config_regVal = AUD->PDM_CORE_CONFIG;
  reg.fields.s_cycles        = 1;
  reg.fields.adchpd          = PDM2PCM_DISABLE_HPF; // 0 - enable and 1 - disable  ! 
#if PDM2PCM_DISABLE_HPF == 0 
  reg.fields.hpgain          = PDM2PCM_HPF_HPGAIN; //14;
#endif
  reg.fields.dmick_dly       = 0;
  reg.fields.div_wd_mode     = 0;
  reg.fields.soft_mute       = o_hal_info.pdm_soft_mute;
  reg.fields.div_mode        = 0;
  reg.fields.mclkdiv         = PDM2PCM_MCLKDIV_VAL;
  reg.fields.sinc_rate       = PDM2PCM_PGA_SYNC_RATE;
  reg.fields.pga_l           = o_hal_info.pdm_pga_l_value;
  reg.fields.pga_r           = o_hal_info.pdm_pga_r_value;
  reg.fields.pdmcore_en      = 1;
  AUD->PDM_CORE_CONFIG       = reg.pdm_core_config_regVal;
  QL_AUDIO_HAL_EXIT_CRITICAL_SECTION();
}

void post_first_pdm_dma_after_lpsd()
{
  if (NULL == o_hal_info.pdata_block_prev) {
     datablk_mgr_acquire(audio_isr_outq_processor.p_dbm, &o_hal_info.pdata_block_prev, 0);
     configASSERT(o_hal_info.pdata_block_prev); // probably indicates uninitialized datablock manager handle
  }
  uint8_t *p_dest = (uint8_t *)o_hal_info.pdata_block_prev + offsetof(QAI_DataBlock_t, p_data);
  voice_dmac_dst_set_addr0((int32_t)p_dest);
  //o_hal_info.pdata_block_prev->dbHeader.Tstart = xTaskGetTickCountFromISR();//g_seqNumber++;
  //Note: This is not called by any ISR
  o_hal_info.pdata_block_prev->dbHeader.Tstart = xTaskGetTickCount();

}
static void SetVoiceDmacConfig(bool fUsingLeftChannel, bool fUsingRightChannel, bool fUsingDualBuffers)
{
  QL_AUDIO_HAL_ENTER_CRITICAL_SECTION();
  ql_arch_voice_dmac_cfg_reg reg;
  reg.voice_dmac_regVal = AUD->VOICE_DMAC_CONFIG;
  reg.fields.dmac_start = 0;
  reg.fields.dmac_stop = 0;
  reg.fields.ahb_rdy = 0;
  reg.fields.ahb_burst_length = 1; // 1=4words, 2=8words(=32bytes=1msec)
  reg.fields.ping_pong_mode = 0;
//TIM
  reg.fields.stereo_dual_buf_mode = fUsingDualBuffers ? 1 : 0;
  reg.fields.voice_dmac_burst_spd = 0;
  AUD->VOICE_DMAC_CONFIG = reg.voice_dmac_regVal;

  uint32_t  ksamples = audio_isr_outq_processor.p_dbm->maxDataElements;
  voice_dmac_dst_set_blk_len(ksamples/2);   // Sample is 1/2 word
  voice_dmac_dst_set_buf_len(ksamples/2);   // sample is 1/2 word
  

  /* Acquire an audio buffer */
  /* setup the DMA start address for next buffer */
  post_first_pdm_dma_after_lpsd();
  QL_AUDIO_HAL_EXIT_CRITICAL_SECTION();
}

/**
* Starts DMA by performing the following sequence of operations:
* 	1. Set DMA Enable bit to 1
* 	2. Set DMA Start bit to 1
* 	3. Set AHB Ready to 1
*/
void HAL_Audio_StartDMA( void )
{
  QL_AUDIO_HAL_ENTER_CRITICAL_SECTION();

  S3x_Clk_Enable(S3X_AUDIO_DMA_CLK);

  ql_arch_voice_config_reg reg;
  reg.voice_config_regVal = AUD->VOICE_CONFIG;
  reg.fields.dmac_blk_done_mask = 0;
  reg.fields.dmac_buf_done_mask = 0;
  AUD->VOICE_CONFIG = reg.voice_config_regVal;

  //  post_first_pdm_dma_after_lpsd();
  ql_arch_voice_dmac_cfg_reg reg_dmac_config;
  reg_dmac_config.voice_dmac_regVal = AUD->VOICE_DMAC_CONFIG;
  reg_dmac_config.fields.dmac_en = 1;
  reg_dmac_config.fields.dmac_stop = 0;
  reg_dmac_config.fields.ahb_rdy = 1;
  // Tim: we need to ensure that the dmac is enabled and ahb is ready *before* starting DMA
  //        unpredictable errors happen if we set all three at the same time
  AUD->VOICE_DMAC_CONFIG = reg_dmac_config.voice_dmac_regVal;
  reg_dmac_config.fields.dmac_start = 1;
  AUD->VOICE_DMAC_CONFIG = reg_dmac_config.voice_dmac_regVal;

#if AUDIO_LED_TEST
    LedYellowOn();
#endif

  QL_AUDIO_HAL_EXIT_CRITICAL_SECTION();
}

/**
* Stops DMA by performing the following sequence of operations:
* 	1. Set AHB Ready to 0
* 	2. Set DMA Stop bit to 1
*
* Note: AD1_SW_RESET is also performed here to switch FIFO from B to A.
*/
void HAL_Audio_StopDMA( bool fUsingLeftChannel, bool fUsingRightChannel )
{

  QL_AUDIO_HAL_ENTER_CRITICAL_SECTION();
   
  ql_arch_voice_config_reg reg;
  reg.voice_config_regVal = AUD->VOICE_CONFIG;
  reg.fields.dmac_blk_done_mask = 1;
  reg.fields.dmac_buf_done_mask = 1;
  reg.fields.fifo_0_clear = 1;
  reg.fields.fifo_1_clear = 1;
  AUD->VOICE_CONFIG = reg.voice_config_regVal;

  ql_arch_voice_dmac_cfg_reg reg_voice_dmac_config;
  reg_voice_dmac_config.voice_dmac_regVal = AUD->VOICE_DMAC_CONFIG;
  reg_voice_dmac_config.fields.dmac_stop = 1;
  reg_voice_dmac_config.fields.ahb_rdy = 0;
  reg_voice_dmac_config.fields.dmac_en = 0;
  AUD->VOICE_DMAC_CONFIG = reg_voice_dmac_config.voice_dmac_regVal;

  //switch to 8K FIFO and reset DMA

   uint32_t mask = (fUsingRightChannel << 2) | (fUsingLeftChannel << 1);
  CRU->AUDIO_MISC_SW_RST = CRU->AUDIO_MISC_SW_RST | mask; //0x7; //reset DMA blk reset | AD2 | AD1 | AD0
  CRU->AUDIO_MISC_SW_RST = CRU->AUDIO_MISC_SW_RST & (uint32_t)(~(mask));

  S3x_Clk_Disable(S3X_AUDIO_DMA_CLK);

#if AUDIO_LED_TEST
    LedYellowOff();
#endif
  QL_AUDIO_HAL_EXIT_CRITICAL_SECTION();
}

static void pdmConnectorInit()
{
  // Disable interrupts at NVIC and INTR_CTRL layers
  disableAudioNvicInterrupts();
}

static void pdmConnectorStart(bool fUsingLeftChannel, bool fUsingRightChannel, bool fUsingDualBuffers) {    
    S3x_Clk_Enable(S3X_AUDIO_APB);                          // Enable register clocks
    PMU->GEN_PURPOSE_0 |= (AUDIO_SRAM_HW_DS_CFG);
    SetVoiceConfig(fUsingLeftChannel, fUsingRightChannel);  // Set Voice config register
    setPdmCoreConfig();                                     // Set PDM Core config register    
    SetVoiceDmacConfig(fUsingLeftChannel, fUsingRightChannel, fUsingDualBuffers);   // set dmac config
}

static void pdmConnectorStop(bool fUsingLeftCHannel, bool fUsingRightChannel) {
  //set_lpsd_state(1);
  HAL_Audio_StopDMA(fUsingLeftCHannel, fUsingRightChannel);
}

//-------------------------------------------------------------------------------------------
void i2sConnectorInit() {
  hangForever("i2sConnectorConnectorInit");
}

void i2sConnectorStart() {
  hangForever("i2sConnectorConnectorStart");
}

void i2sConnectorStop() {
  hangForever("i2sConnectorConnectorStop");
}

//-------------------------------------------------------------------------------------------
void pdmVoiceIqConnectorInit() {
  hangForever("pdmVoiceIqConnectorInit");
}

void pdmVoiceIqConnectorStart() {
  hangForever("pdmVoiceIqConnectorStart");
}

void pdmVoiceIqConnectorStop() {
  hangForever("pdmVoiceIqConnectorStop");
}

//-------------------------------------------------------------------------------------------
struct connector gPdmConnector = {
  .initialized = false,
  .started = false,
  .init = pdmConnectorInit,
  .start = pdmConnectorStart,
  .stop = pdmConnectorStop
};

#ifdef CLEANUP
struct connector gI2sConnector = {
  .initialized = false,
  .started = false,
  .init = i2sConnectorInit,
  .start = i2sConnectorStart,
  .stop = i2sConnectorStop
};

struct connector gPdmVoiceIqConnector = {
  .initialized = false,
  .started = false,
  .init = pdmVoiceIqConnectorInit,
  .start = pdmVoiceIqConnectorStart,
  .stop = pdmVoiceIqConnectorStop
};
#endif /* CLEANUP */

/*
* Returns a handle for audio core by taking in DigitalMic type.
* All subsequent (exposed) API calls use this handle to trigger right methods.
*/
audio_handle_t HAL_Audio_GetHandle(DigitalMic_t micType)
{
  switch(micType) {
#ifdef CLEANUP
  case DigitalMic_I2S:
    return (void *)&gI2sConnector;
  case DigitalMic_PDM_VoiceIQ:
    return (void *)&gPdmVoiceIqConnector;
#endif
  case DigitalMic_PDM:
    return (void *)&gPdmConnector;
  default:
    //assert("Invalid micType" && 0);
    return NULL;
  }
}

/*
* Initialize audio core. Run Once at powerup.
* 	Turns on audio core
* 	Sets all registers of audio core and other components such as CRU
* 	Turns on interrupts at NVIC and M4 level, but audio core level interrupts are turned off
*/
void HAL_Audio_Init(audio_handle_t handle, HAL_Audio_Event_Notifier_t *pevent_notifier, bool fUsingLeftChannel, bool fUsingRightChannel)
{
  struct connector *pConnector = (struct connector *)handle;
  assert(handle != NULL);

  if(pevent_notifier)
      pEventNotifier = pevent_notifier;

  // init Pdm mic
  pConnector->init();
  pConnector->initialized = true;

  // stop with C30 and C31 clocks off
  disableAudioClocks(fUsingLeftChannel, fUsingRightChannel);
}

/**
* Starts audio core. Run Everytime Voice system starts.
* 	Enable the audio interrupts at audio core to stop audio capture
*/
void HAL_Audio_Start(audio_handle_t handle, uint16_t ksamplesToDrop, bool fDisableLPSD, struct HAL_Audio_Config* pconfig)
{
      ql_arch_voice_config_reg reg;
  //assert(handle != NULL);
  struct connector *pConnector = (struct connector *)handle;
  assert(handle != NULL);
  assert(pConnector->initialized == true);

  //enableAudioClocks(pconfig->fUsingLeftChannel, pconfig->fUsingRightChannel);
  //TIM enableAudioNvicInterrupts();
  //enableDmaInt();
  //ignore_lpsd(fIgnoreLPSD);

  reg.voice_config_regVal = AUD->VOICE_CONFIG;
  if(fDisableLPSD){
    o_hal_info.lpsd_mode = 0;
    reg.fields.lpsd_no = 1; // LPSD disable
  }
  else {
    o_hal_info.lpsd_mode = 1;
    reg.fields.lpsd_no = 0; // LPSD enable
  }
  AUD->VOICE_CONFIG = reg.voice_config_regVal;

  // start Pdm mic
  //TIM Refactor pConnector->start(pconfig->fUsingLeftChannel, pconfig->fUsingRightChannel, pconfig->fUsingDualBuffers);
  pdmConnectorStart(pconfig->fUsingLeftChannel, pconfig->fUsingRightChannel, pconfig->fUsingDualBuffers);
  pConnector->started = true;
//  dbg_queue_monitor_print(SOURCE_LOCATION);
  
  //TIM
  enableAudioClocks(pconfig->fUsingLeftChannel, pconfig->fUsingRightChannel);
  
  NVIC_SetPriority (Dmac0_Block_Done_IRQn, 6);
  NVIC_SetPriority (Dmac0_Buffer_Done_IRQn, 6);
  
  

  config_voice_interrupts(e_voice_config_interrupts_unmask_all);
  //TIM
  enableDmaInt();
  o_hal_info.ksamplesToDrop = ksamplesToDrop;
}

/*
* Stops audio core.  Run Everytime Voice system stops.
* 	Disable the audio interrupts at audio core to stop audio capture
*/
void HAL_Audio_Stop(audio_handle_t handle, struct HAL_Audio_Config* pconfig)
{
    struct connector *pConnector = (struct connector *)handle;
    assert(handle != NULL);
    //assert(pConnector->started == true);
    if( pConnector->started == true ) {
        // stop Pdm mic
        //pConnector->stop(pconfig->fUsingLeftChannel, pconfig->fUsingRightChannel);
        HAL_Audio_StopDMA(pconfig->fUsingLeftChannel, pconfig->fUsingRightChannel);
        pConnector->started = false;
        config_voice_interrupts(e_voice_config_interrupts_mask_all);
//        dbg_queue_monitor_print(SOURCE_LOCATION);
    }
    disableAudioClocks(pconfig->fUsingLeftChannel, pconfig->fUsingRightChannel);
    S3x_Clk_Disable(S3X_AUDIO_APB);
    disableAudioNvicInterrupts();
}


//-------------------------------------------------------------------------------------------
// ISRs start here
void HAL_Audio_ISR_LpsdOn(void)
{
  // clear interrupt
  INTR_CTRL->OTHER_INTR &= LPSD_VOICE_DET;
  NVIC_ClearPendingIRQ(Lpsd_IRQn);

#if AUDIO_LED_TEST
    LedBlueOff();
    LedGreenOn();
#endif

  // As event notifier is calling from ISR context, event handler must be light weight.
  if(pEventNotifier)
      pEventNotifier(HAL_Audio_Event_LPSD_ON, NULL);
}


void HAL_Audio_ISR_LpsdOff(void)
{
  // clear interrupt
  //INTR_CTRL->OTHER_INTR |= LPSD_VOICE_DET;
  NVIC_ClearPendingIRQ(Lpsd_Voice_Off_IRQn);

#if AUDIO_LED_TEST
    LedBlueOn();
    LedGreenOff();
#endif
  // As event notifier is calling from ISR context, event handler must be light weight.
  if(pEventNotifier)
      pEventNotifier(HAL_Audio_Event_LPSD_OFF, NULL);
}

// Why are these here?  And not part of event handler?
void handle_led_for_lpsd_on(void)
{
    // clear interrupt
    INTR_CTRL->OTHER_INTR &= LPSD_VOICE_DET;
    NVIC_ClearPendingIRQ(Lpsd_IRQn);
#if AUDIO_LED_TEST
    LedBlueOff();
    LedGreenOn();
#endif
}

void handle_led_for_lpsd_off(void)
{
    NVIC_ClearPendingIRQ(Lpsd_Voice_Off_IRQn);
#if AUDIO_LED_TEST
    LedBlueOn();
    LedGreenOff();
#endif
}

void handle_led_for_audio_stop(void)
{
#if AUDIO_LED_TEST  
    LedBlueOff();
    LedGreenOff();
#endif
    return;
}

void onDmicOn(void)
{


}


void onDmicOff(void)
{
}


void onDmac0BlockDone(void)
{
  NVIC_ClearPendingIRQ(Dmac0_Block_Done_IRQn);
  if(pEventNotifier)
      pEventNotifier(HAL_Audio_Event_DMA_BLOCK_DONE, NULL);
}

void onDmac0BufferDone(void)
{

  // Reset DMA to start form the starting address
 //TIM  Now this looks like a really bad idea
    //voice_dmac_set_start_bit();
  
  NVIC_ClearPendingIRQ(Dmac0_Buffer_Done_IRQn);
  
  if(pEventNotifier)
    pEventNotifier(HAL_Audio_Event_DMA_BUFFER_DONE, NULL);

}

void audio_isr_onDmac0BufferDone(void)
{
    QAI_DataBlock_t*    pdata_block = NULL;
    bool                fGotNewBlock = false;

    /* Acquire an audio buffer */
    int ret = datablk_mgr_acquireFromISR(audio_isr_outq_processor.p_dbm, &pdata_block);
    //configASSERT(ret == 0);

    if (pdata_block)
    {
        fGotNewBlock = true;
    }
    else
    {
        //configASSERT(0);
        // send error message through notifier, if registered with a notifier 
        if (audio_isr_outq_processor.p_event_notifier)
        (*audio_isr_outq_processor.p_event_notifier)(audio_isr_outq_processor.in_pid, AUDIO_ISR_EVENT_NO_BUFFER, NULL, 0);

        /* repeat the last one as is */
        pdata_block = o_hal_info.pdata_block_prev;
        pdata_block->dbHeader.Tstart = xTaskGetTickCountFromISR();
        pdata_block->dbHeader.numDropCount++;
        o_hal_info.num_drop_count++;
    }
   
    uint8_t *p_dest = (uint8_t *)pdata_block + offsetof(QAI_DataBlock_t, p_data);
    /* setup the DMA start address for next buffer */
    voice_dmac_dst_set_addr0((int32_t)p_dest);
    NVIC_ClearPendingIRQ(Dmac0_Buffer_Done_IRQn);
    voice_dmac_set_start_bit();

    if (fGotNewBlock)
    {
        o_hal_info.pdata_block_prev->dbHeader.Tend = xTaskGetTickCountFromISR(); //o_hal_info.dma_isr_count++; //xTaskGetTickCountFromISR();
        o_hal_info.pdata_block_prev->dbHeader.numDataChannels = kchannels;
        if (o_hal_info.ksamplesToDrop) {
            int isample;
            uint16_t* psample = (uint16_t*)o_hal_info.pdata_block_prev->p_data;
            for (isample = 0; isample < o_hal_info.ksamplesToDrop && isample < 240; isample++) {
                *psample++ = 0;
            }
            o_hal_info.ksamplesToDrop -= isample;
        }
        datablk_mgr_WriteDataBufferToQueuesFromISR(&audio_isr_outq_processor, o_hal_info.pdata_block_prev);
        o_hal_info.pdata_block_prev = pdata_block;
    }
    if(pEventNotifier)
        pEventNotifier(HAL_Audio_Event_DMA_BUFFER_DONE, NULL);
}

////////////// Releasing of last data block acquired before audio stop.
void release_data_block_prev(void)
{
  if (o_hal_info.pdata_block_prev)
  {
    // if the block is acquired and not yet posted, increase the usecount and then release the block
    if(o_hal_info.pdata_block_prev->dbHeader.numUseCount == 0)
    {  
        datablk_mgr_usecount_increment(o_hal_info.pdata_block_prev,1);
    }
    datablk_mgr_release_generic(o_hal_info.pdata_block_prev);
    o_hal_info.pdata_block_prev = NULL;
  }
}
///////////////


void taskENTER_CRITICAL_todo()
{

}

void taskEXIT_CRITICAL_todo()
{

}

int  display_num_drop_count(void)
{
    dbg_str_int("num_drop_count = ", o_hal_info.num_drop_count);
    return o_hal_info.num_drop_count;
}

void reset_num_drop_count(void)
{
    o_hal_info.num_drop_count = 0;
}

// TIM
void QL_Audio_StartUp(HAL_Audio_Event_Notifier_t* Event_Notifier_AudioApp, struct HAL_Audio_Config* pconfig) {
  

  //S3x_Register_Qos_Node(S3X_LPSD);

  // startup Audio system in low power
  //TIM audioHandle = HAL_Audio_GetHandle(DigitalMic_PDM);
  //TIM HAL_Audio_Init(audioHandle, Event_Notifier_AudioApp);
  gPdmConnector.initialized = true;
  pEventNotifier = Event_Notifier_AudioApp;
  disableAudioNvicInterrupts();
  disableAudioClocks(pconfig->fUsingLeftChannel, pconfig->fUsingRightChannel);
}

#endif          /* ENABLE_VOICE_SOLUTION */
