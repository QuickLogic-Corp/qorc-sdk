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
*    File   : ql_audio.c
*    Purpose:
*
*=========================================================*/
#include "Fw_global_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ql_audio.h"
#include "ql_controlTask.h"
#include "qlsh_commands.h"
#include "eoss3_dev.h"
//#include "eoss3_hal_rcc.h"
#include "eoss3_hal_audio.h"
#include "eoss3_hal_gpio.h"
#include "dma_buffer.h"
#include "eoss3_hal_leds.h"
#include "s3x_clock_hal.h"
#include "s3x_clock.h"
#include "s3x_dfs.h"
#include "RtosTask.h"
//#include "ql_apTask.h"
//#include "ql_uart.h"
//#include "voiceControlSim.h"
#include <math.h>
#include "qlsh_commands.h"

//user application should processing this

//#define TIMEOUT_DBG

#ifdef TIMEOUT_DBG
#include <eoss3_hal_pads.h>
#include <eoss3_hal_pad_config.h>
#include <eoss3_hal_gpio.h>
#endif

#include "timers.h"

/*
* TODO: and FIXME:  Create  "struct audio_task_vars"
* TODO: Remove various voice system stuff from here (simplify code)
* TODO: Make configurable, ie: 48khz, 16khz, 22.1khz etc.
* TODO: Make start/stop work
*/

xTaskHandle         xHandleTaskAudio;
QueueHandle_t       xHandleQueueAudio;
int                 xAudioQueueOverflow;



/* GLOBAL VARIABLES */
//The following variables are updated in efuse library
//int Sensory_SFT_En = 0;
//int Sensory_SFT_UDT = 0;
//int Sensory_EFT_UDP = 0;
//int Sensory_FT_UDT_PSCMD = 1;

/* LOCAL STATIC VARIABLES */
static struct HAL_Audio_Config  Audio_Config_Local;

/*********************************************************************************
 *
 *  AudioHW-FSM interface
 *
 ********************************************************************************/

enum process_state Audio_State = PSTATE_UNCONFIG;
struct HAL_Audio_Config  AudioHW_FSMConfigData = {
    .fUsingLeftChannel = (PDM_MIC_CHANNELS == 2) || (PDM_MIC_LEFT_CH == 1),
    .fUsingRightChannel = (PDM_MIC_CHANNELS == 2) || (PDM_MIC_RIGHT_CH == 1),
    .fUsingDualBuffers = (EN_STEREO_DUAL_BUF == 1),
};
       
enum process_state AudioHW_FSMAction(enum process_action pa, void* pv) {
    uint32_t voice_status;
    switch(pa) {
    case PACTION_CONFIG:
        Audio_Config_Local = *((struct HAL_Audio_Config*)pv); 
        QL_Audio_StartUp(Event_Notifier_AudioApp, &Audio_Config_Local);
        QL_Audio_Voice_Stop();
        Audio_State = PSTATE_STOPPED;
        break;
        
    case PACTION_START_SAVING:
        // This can be either a START or a RESTART
        // If stopped, start the audio then put in the SAVING mode (Archband calls this buffering)
        // If not stopped, then just put in SAVING mode  
        configASSERT(Audio_State != PSTATE_UNCONFIG);
        if( Audio_State == PSTATE_STOPPED ) {
            HAL_Audio_Start(&gPdmConnector, KSAMPLES_TO_DROP, false, &Audio_Config_Local);
        }
        HAL_Audio_StopDMA(Audio_Config_Local.fUsingLeftChannel, Audio_Config_Local.fUsingRightChannel);    // Resets HW CircBuffer, and sends data into it
        Audio_State = PSTATE_SAVING;
        break;
        
    case PACTION_START_STREAMING:
        // This can be either a START or a RESTART
        // If stopped, of if LPSD not triggered, start the audio then put in the STREAMING mode so that data flows to the rest of the system
        // If not stopped, then just put in STREAMING mode
        configASSERT(Audio_State != PSTATE_UNCONFIG);
        voice_status = AUD->VOICE_STATUS & (1<<17);
        if( Audio_State == PSTATE_STOPPED || (voice_status == 0)) {
            HAL_Audio_Start(&gPdmConnector, KSAMPLES_TO_DROP, true, &Audio_Config_Local);
            Audio_State = PSTATE_SAVING;
        }
        if( Audio_State != PSTATE_STOPPED ) {
            HAL_Audio_StartDMA();   // Starting DMA immedately ships data
        }
        Audio_State = PSTATE_STREAMING;
        break;
        
    case PACTION_STOP:
        configASSERT(Audio_State != PSTATE_UNCONFIG);
        HAL_Audio_Stop(&gPdmConnector, &Audio_Config_Local); // Stops the DMA, among other things
        Audio_State = PSTATE_STOPPED;
        break;
        
    default:
        Audio_State = PSTATE_UNKNOWN;
        assert(0);
    }
    return(Audio_State);
}

/*********************************************************************************
 *
 *  LPSD-FSM interface
 *
 ********************************************************************************/
enum process_state LPSD_State = PSTATE_UNCONFIG;
int                LPSD_FSMConfigData;
    
enum process_state LPSD_FSMAction(enum process_action pa, void* pv) {
    switch(pa) {
    case PACTION_CONFIG:
        configASSERT(LPSD_State == PSTATE_UNCONFIG);
       //TIM configASSERT(Audio_State != PSTATE_UNCONFIG);
        
        NVIC_SetPriority (Lpsd_IRQn, 6);
        NVIC_SetPriority (Lpsd_Voice_Off_IRQn, 6);
        LPSD_State = PSTATE_STOPPED;
        break;
        
    case PACTION_STATE_ON:              // LPSD_ON event     
        LPSD_State = PSTATE_LPSD_ON;
        break;
        
    case PACTION_START_ON:
        if (LPSD_State == PSTATE_STOPPED) {
            S3x_Clk_Enable(S3X_LPSD);
            HAL_Audio_LPSD_Enable(true); 
            HAL_Audio_LPSD_Int(true);
        }
        LPSD_State = PSTATE_LPSD_ON;
        break;
        
    case PACTION_STATE_OFF:              // LPSD_OFF event     
        LPSD_State = PSTATE_LPSD_OFF;
        break;
        
    case PACTION_START_OFF:
        if (LPSD_State == PSTATE_STOPPED) {
            S3x_Clk_Enable(S3X_LPSD);
            HAL_Audio_LPSD_Enable(true);
            HAL_Audio_LPSD_Int(true);
        }
        LPSD_State = PSTATE_LPSD_OFF;
        break;
    
    case PACTION_STOP:
        HAL_Audio_LPSD_Int(false);
        HAL_Audio_LPSD_Enable(false);
        S3x_Clk_Disable(S3X_LPSD);
        #if AUDIO_LED_TEST
        /* Reset LPSD state LEDs indication */
        LedBlueOff();
        LedGreenOff();
        #endif
        LPSD_State = PSTATE_STOPPED;
        break;
        
    default:
        configASSERT(pa);
    }
    return(LPSD_State);
}

void Ql_Audio_DMAStart(void);

void vr_rdsp_clear_static_mem();
/* Audio algorithm task */

//TIM UPDATED
void Event_Notifier_AudioApp(HAL_Audio_Event_type_t event_type, void *p_event_data)
{
  struct xCQ_Packet packet;
  switch(event_type)
  {
  case HAL_Audio_Event_LPSD_ON :
    packet.ceEvent = CEVENT_LPSD_ON;
    addPktToControlQueueFromISR(&packet);
    break;
  case HAL_Audio_Event_LPSD_OFF :
    packet.ceEvent = CEVENT_LPSD_OFF;
    addPktToControlQueueFromISR(&packet);
    break;
    // Ignore DMA stuff
  case HAL_Audio_Event_DMA_BLOCK_DONE :
  case HAL_Audio_Event_DMA_BUFFER_DONE :
    break;
  default :
    printf("Error : Unhandled Event from Audio HAL driver\n");
  }
}

void QL_Audio_Voice_Stop(void)
{
  HAL_Audio_StopDMA(Audio_Config_Local.fUsingLeftChannel, Audio_Config_Local.fUsingRightChannel);
  /*Ask EOSS3 to stop and go in power save mode*/
  HAL_Audio_Stop(&gPdmConnector, &Audio_Config_Local);
}

void QL_Audio_Voice_Start(void)
{
  //S3x_Clk_Enable(S3X_LPSD);
  HAL_Audio_Start(&gPdmConnector, 0, true, &Audio_Config_Local);
}


