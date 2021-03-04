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
 *    File   : ql_audio.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __QL_AUDIO_H_
#define __QL_AUDIO_H_

#if !defined( _EnD_Of_Fw_global_config_h )
#error "Include Fw_global_config.h first"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include <queue.h>
#include "common.h"
#include "eoss3_hal_audio.h"
#include "test_types.h"
#include "ql_controlDefines.h"

extern enum process_state LPSD_State;
#ifndef KSAMPLES_TO_DROP
#define KSAMPLES_TO_DROP    100         // Number of samples at start of audio to replace with zeros in order to suppress mic transient
#endif

#define SM_VOICE_TRIG 119

#define SVSCORE_MAX             8192
#define DEFAULT_FAFR_VAL          0
#define GRAM_DELAY              0          // 0: use grammar delay, otherwise specify delay
#define SDET_TYPE               SDET_NONE  // SDET_NONE or SDET_LPSD

#define SVSCORE_THRESH_DEFAULT_PERCENT 35
#define SV_THRESHOLD                  (8192*SVSCORE_THRESH_DEFAULT_PERCENT/100)

#define COMMAND_DELAY   0    // use grammar
#define COMMAND_TOKENS  900 //2000
#define COMMAND_TIMEOUT 0   // 3 seconds

// to use grammar EPQ settings, set both of the following to 0.
// otherwise, you can disable EPQ regardless of grammar settings,
// or enable EPQ regardless of grammar settings with a specified floating point EPQ Min SNR
// threshold which is converted to a u16 value in the code
#define EPQ_OVERRIDE_GRAMMAR_DISABLE   0
#define EPQ_OVERRIDE_GRAMMAR_ENABLE    0
#if EPQ_OVERRIDE_GRAMMAR_ENABLE
# define EPQ_MIN_SNR_FLOAT  (-5.0)
#endif


#define N_BRICKS_FOR_AP ( 4 )

#include "ql_util.h"

#define AUDIO_QUEUE_LENGTH  (DMA_NUMBER_OF_BUFFERS -1) 


#ifdef SINGLE_KPD_MODE

#define NO_OF_KPDS		(1)

#else

#define NO_OF_KPDS		(2)

#endif

#define		CMD_TIMEOUT_ID		(0xFF)


#define CMD_NONE		(1<<0)		// KPD has cmd phase
#define	CMD_INTERNAL	(1<<1)		//command is present internally
#define	CMD_EXTERNAL	(1<<2)		//command is not present internally
#define CMD_BOTH		(1<<3)

//KPT_CMD_INFO not used.


/*! \struct ModelUpdateInfo ql_audioTriggerTask.h "inc/ql_audioTriggerTask.h"
 *  \brief Structure that embeds the necessary model pointers where models needs to be loaded
 */
typedef struct
{
	uint16_t *netPtr;
	uint16_t *gramPtr;
	uint32_t netSize;
	uint32_t gramSize;

#ifdef CMD_ENROLL_SUPPORT

	uint16_t *cmd_netPtr;
	uint16_t *cmd_gramPtr;
	uint32_t cmd_netSize;
	uint32_t cmd_gramSize;

#endif

} ModelUpdateInfo;


typedef struct
{
    // kpd_cnt not used
    ModelUpdateInfo  ModelInfo;         /* Address of Model Files data */
    int16_t         *ptrAudioBuffStart; /* Address of Audio DMA buffer */
    UINT8_t          AudioBuffLen;      /* Length of Audio DMA Buff <= 256 */
    UINT8_t          sys_mode;          /* current mode of operation */
    UINT8_t          sys_state;         /* current state */
    // last AP (apps processor) not used 
    UINT32_t         numBricksIn;       /* number of bricks rx via dma (mic) */
    UINT32_t         numBricksOut;      /* number of bricks tx out during aud streaming */

// not used: KPDCMD
} VoiceSysInfo;


typedef struct
{
       unsigned int    ts;                                                                         /*!< Timestamp of audio event */
       unsigned short  eventID;                                                                    /*!< Event ID (sensorID) for audio event */
       unsigned short   dummy;                                                                        /*!< dummy to make structure 32 bit aligned */
       unsigned int   eventValue;                                                                 /*!< Event data information */
}Audio_Event_output;

enum AudioMode
{
	START_UP = 0,
	LPSD,
	CONTINUOUS
};

enum AudioState
{
	POWER_DN = 0,
	LOW_POWER,
	WAIT_FOR_LPSD,
	ACTIVE
};

void audioTriggerTask(void *pParameter);
extern struct connector gPdmConnector;
short QL_Audio_Config(unsigned char ucCfgCmd);
void QL_Audio_Voice_Stop(void);
short QL_Audio_VoiceWaitForTrigger(void);
void Event_Notifier_AudioApp(HAL_Audio_Event_type_t event_type, void *p_event_data);
// Audio task status apis
// audio tasks checks not required in new scheme.
/* add struct decl for prototype */
/* Rename generic: "updateDMAbuffer" to audio specific name */
struct xQ_Packet;
extern void updateAudioDMABuffer(struct xQ_Packet* packet);

/* prototype to header file. */
extern void addPktToQueueFromISR_Audio( struct xQ_Packet *pPacket );
void addPktToQueueFromTask_Audio(struct xQ_Packet *pxMsg);

#include "dma_buffer.h"

void AudioProcessBrick16( const int16_t *pBuffer, size_t nsamples );

//TIM
enum process_state    LPSD_Stop(int);
enum process_state    LPSD_Start(int);

#endif //__QL_AUDIO_H_
