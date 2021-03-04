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
* eoss3_hal_audio.h
*
*/

#ifndef __AUDIO_CORE_H_
#define __AUDIO_CORE_H_
#include <stdbool.h>

#include "Fw_global_config.h"
#include "eoss3_dev.h"
#include "datablk_mgr.h"

#ifndef PDM2DEC_FACT
#define PDM2DEC_FACT 32
#endif

//PDM Core Config Registers Setting
#define PDM_MODE_SEL            0       // 0=mono, 1=stereo
#define PDM_MONO_CHN_SEL        0       // 0=left, 1=right if in mono mode
//#define DEFAULT_PDM_PGA_L_VAL   8
//#define DEFAULT_PDM_PGA_R_VAL   8
#define DEFAULT_SOFT_MUTE       0

#if     PDM2DEC_FACT==64

#define  PDM2PCM_PGA_SYNC_RATE          (64)
#define  PDM2PCM_PGA_GAIN_VAL           (6)
#define  PDM2PCM_MCLKDIV_VAL            (0)
#define  PDM2PCM_CLK_C30                (HSOSC_2MHZ)
#define  PDM2PCM_CLK_C31                (HSOSC_512KHZ)

#elif   PDM2DEC_FACT==48

#define  PDM2PCM_PGA_SYNC_RATE          (48)
#define  PDM2PCM_PGA_GAIN_VAL           (8)
#define  PDM2PCM_MCLKDIV_VAL            (1)
#define  PDM2PCM_CLK_C30                (HSOSC_3MHZ)
#define  PDM2PCM_CLK_C31                (HSOSC_768KHZ)

#elif   PDM2DEC_FACT==32

#define  PDM2PCM_PGA_SYNC_RATE          (32)
#define  PDM2PCM_PGA_GAIN_VAL           (16)
#define  PDM2PCM_MCLKDIV_VAL            (1)
#define  PDM2PCM_CLK_C30                (HSOSC_2MHZ)
#define  PDM2PCM_CLK_C31                (HSOSC_512KHZ)

#elif   PDM2DEC_FACT==24
 
#define  PDM2PCM_PGA_SYNC_RATE          (24)
#define  PDM2PCM_PGA_GAIN_VAL           (8+8)  // < == not sure about gain
#define  PDM2PCM_MCLKDIV_VAL            (3)
#define  PDM2PCM_CLK_C30                (HSOSC_3MHZ)  // <= ¾ = 768KHz
#define  PDM2PCM_CLK_C31                (HSOSC_768KHZ)

#else
#error "PDM2DEC_FACT is not defined "
#endif

#define PDM2PCM_DISABLE_HPF           (0)  
#if PDM2PCM_DISABLE_HPF == 0
#define PDM2PCM_HPF_HPGAIN            (5)
#endif
//PDM Voice Config Registers Setting
enum VOICE_CONFIG_SCENARIO {
  PDM_VOICE_SCENARIO1 = 0,
  PDM_VOICE_SCENARIO2 = 1,
  PDM_VOICE_SCENARIO3_MODE1 = 2,
  PDM_VOICE_SCENARIO3_MODE2 = 3,
  PDM_VOICE_SCENARIO3_MODE3 = 4
};

/** @brief Enumerators for audio errors */
typedef enum {
  AUDIO_ISR_EVENT_NO_BUFFER, ///< ISR could not obtain new buffer from FreeQ
} audio_event_types_t;

#define NO_SWITCH 0
#define SWITCH_TO_AP 1

#define DEFAULT_PDM_VOICE_SCENARIO PDM_VOICE_SCENARIO1
#define DEFAULT_PDM_MIC_SWITCH_TO_AP NO_SWITCH

#define AUDIO_ISR_MESSAGE  ( 0x0ff & ('A' + 'I' + 'S' + 'R') )
#define AUDIO_TASK_MESSAGE  ( 0x0ff & ('T' + 'A' + 'S' + 'K') )

//TIM
struct HAL_Audio_Config {
    bool    fUsingLeftChannel;
    bool    fUsingRightChannel;
    bool    fUsingDualBuffers;
};

struct connector {
  bool initialized;
  bool started;
  void (*init)();
  void (*start)(bool, bool, bool);
  void (*stop)(bool, bool);
};

typedef void* audio_handle_t;

/*
* DigitalMic type
*/
typedef enum DigitalMic {
  DigitalMic_I2S,
  DigitalMic_PDM,
  DigitalMic_PDM_VoiceIQ
} DigitalMic_t;

/*
* Event Notifier types
*/
typedef enum event_type {
  HAL_Audio_Event_Start,
  HAL_Audio_Event_LPSD_ON,
  HAL_Audio_Event_LPSD_OFF,
  HAL_Audio_Event_DMA_BLOCK_DONE,
  HAL_Audio_Event_DMA_BUFFER_DONE,
  HAL_Audio_Event_End
} HAL_Audio_Event_type_t;

extern struct connector gPdmConnector;

/*
* Get a handle for audio core by passing digital mic type.
* Use this handle in all future calls.
*/
audio_handle_t HAL_Audio_GetHandle(DigitalMic_t micType);

/*
* Event Notifier prototype
* Application can define this function and pass to HAL_Audio_Init API to get event callbacks.
*/
typedef void HAL_Audio_Event_Notifier_t(HAL_Audio_Event_type_t event_type, void *p_event_data);

/*
* Initialize audio core.
* 	Turns on audio core
* 	Sets all registers of audio core and other components such as CRU
* 	Turns on interrupts at NVIC and M4 level, but audio core level interrupts are turned off
*/
void HAL_Audio_Init(audio_handle_t handle, HAL_Audio_Event_Notifier_t *pevent_notifier, bool, bool);

/**
* Starts audio core.
* 	Enable the audio interrupts at audio core to stop audio capture
*/
void HAL_Audio_Start(audio_handle_t handle, uint16_t ksamplesToDrop, bool fIgnoreLPSD, struct HAL_Audio_Config*);

/*
* Stops audio core.
* 	Disable the audio interrupts at audio core to stop audio capture
*/
void HAL_Audio_Stop(audio_handle_t handle, struct HAL_Audio_Config*);

/**
* Starts Audio DMA
*/
void HAL_Audio_StartDMA( void );

/**
* Stops Audio DMA
*/
void HAL_Audio_StopDMA( bool fUsingLeftChannel, bool fUsingRightChannel );

void release_data_block_prev(void);

/**
* Enables LPSD
*/
void    HAL_Audio_LPSD_Enable(bool fEnable);
void    HAL_Audio_LPSD_Int(bool fEnable);

// Audio Related isrs
void HAL_Audio_ISR_LpsdOn();
void HAL_Audio_ISR_LpsdOff();
void onDmicOn();
void onDmicOff();
void onDmac0BlockDone();
void onDmac0BufferDone();
void handle_led_for_lpsd_on(void);
void handle_led_for_lpsd_off(void);
void handle_led_for_audio_stop(void);
/** User application should define and instantiate the following structure */
extern outQ_processor_t audio_isr_outq_processor ;

int  display_num_drop_count(void);
void reset_num_drop_count(void);

//TIM
void QL_Audio_StartUp(HAL_Audio_Event_Notifier_t* Event_Notifier_AudioApp, struct HAL_Audio_Config*);
void enableAudioClocks(bool fUsingLeftChannel, bool fUsingRightChannel);
void disableAudioClocks(bool fUsingLeftChannel, bool fUsingRightChannel);

#endif /* __AUDIO_CORE_H_ */

