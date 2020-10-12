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

#ifndef __QL_VOICE_HEARABLE_PROTOCOL_H
#define __QL_VOICE_HEARABLE_PROTOCOL_H

#include "ql_uart.h"

#define SM_VOICE_TRIG 119

/**
 * @brief return value definitions
 */
#define VOICE_RET_VAL          (0x0)
#define VOICE_RET_VAL_ERROR    (0x20)

#define VOICE_SUCCESS          		(VOICE_RET_VAL + 0x0)

#define VOICE_ERROR            		(VOICE_RET_VAL_ERROR + 0x1)
#define VOICE_BAD_PARAMETER    		(VOICE_RET_VAL_ERROR + 0x2)
#define VOICE_DEVICE_NOT_READY 		(VOICE_RET_VAL_ERROR + 0x3)
#define VOICE_DEVICE_INVALID_CMD	(VOICE_RET_VAL_ERROR + 0x4)
#define VOICE_UART_CMD_TX_ERROR 	(VOICE_RET_VAL_ERROR + 0x5)
#define VOICE_UART_RESPEVT_TX_ERROR (VOICE_RET_VAL_ERROR + 0x6)
#define VOICE_UART_TIME_OUT_ERROR	(VOICE_RET_VAL_ERROR + 0x7)

#define VR_ON  1
#define VR_OFF 0

#define AUDIO_STREAMING_ON  1
#define AUDIO_STREAMING_OFF 0

#if 1 // Syama: Brought in from ql_apTask.h
#define SHIFT_1_BYTE	8
#define SHIFT_2_BYTES	16
#define SHIFT_3_BYTES	24

#define MESG_TYPE_MASK_BITS_LOC	(1)
#define CATEGORY_ID_MASK_BITS_LOC	(2)
#define COMMAND_MASK_BITS_LOC	(4)

#define MSG_RESPONSE    0
#define MSG_NOTIFY      1
#define MSG_TYPE_SHORT	0
#define MSG_TYPE_LONG	1

#define MASK_1_BIT	(0x1)
#define MASK_2_BIT	(0x3)
#define MASK_3_BIT	(0x7)
#define MASK_4_BIT	(0xF)
#define MASK_8_BIT	(0xFF)
   
#define QLU_VOICE_PKT_LEN (4)

#endif //Syama

#if 1 //Syama: Brought in from qlsh_commands.h
/*! \enum eVOICE_EVT_VOICE2CT
    \brief These events are sent to Control Task from Audio Trigger task.

   All events are part of standard Queue packet format (command)
 */
typedef enum eVOICE_EVT_VOICE2AP
{
	eVOICE_EVT_VA_READY = 1,          /*On boot EOSS3 ready */
	eVOICE_EVT_VA_ACK,                /*Event in response to AP command if result is OK */
	eVOICE_EVT_VA_ERR,                /*Event in response to AP command if result is not OK */
	eVOICE_EVT_VA_LPSDT,              /*LPSD Trigger happened */
	eVOICE_EVT_VA_KPT,                /*Key Phrase Trigger happened */
	eVOICE_EVT_VA_AUDIO_DATA,         /*EOS3 put in low power state */
	eVOICE_EVT_VA_INFO_RESP_1,        /*EOS3 put in WFH state */
	eVOICE_EVT_VA_INFO_RESP_2,        /*EOS3 put in WFH state */
	eVOICE_EVT_VA_MODEL_UPDT_READY,   /*EOS3 put in WFH state */
#ifdef KPDCMD
	eVOICE_EVT_VA_KPT_CMD,		/*	CMD after KPD triggered	*/
#endif
} Voice_Evt;

#endif //Syama

/*Voice Events Supported for Hearable*/
typedef enum {
	RESP_SUCCESS = 1,
	RESP_FAILURE = 2,
} VoiceResps;

/*Voice Events Supported for Hearable*/
typedef enum {
	EVT_READY = 1,
	EVT_PHRASE_DET = 11,
	EVT_ERROR = 12,
} VoiceEvts;

/*Voice Commands Supported for Hearable*/
typedef enum {
	CFG_VR = 25,
	CFG_AUDIO_STREAMING = 26,
} VoiceCmds;

/*Voice Event/Response Handlers*/
typedef struct {
	void (*VoiceResp)(VoiceResps respId, uint8_t data1, uint8_t data2);
	void (*VoiceEvt)(VoiceEvts evtId, uint8_t data1, uint8_t data2);
} VoiceHandlers;

/*Voice Protocol APIs*/
uint32_t Voice_Init(void);
uint32_t Voice_Register_Handlers(VoiceHandlers *voiceHandlers);
bool Voice_Device_Ready_Status (void);
uint32_t Voice_Send_RespEvt_to_BT(uint32_t pkt);
uint32_t Voice_Send_DataPkt_to_BT(uint8_t *pkt_buf, uint8_t data_len);
QLUART_Status Voice_Qluart_Cb(uint8_t*data,uint16_t len,void* cookie,uint8_t pkt_type);
#endif
