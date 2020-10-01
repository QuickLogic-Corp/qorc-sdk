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

/** @file ql_voice_hearable_protocol.c
Voice Hearable Protocol APIs
*/
#include <stdio.h>
#include <stdbool.h>
#include "RtosTask.h"
#include "Fw_global_config.h"
#include "ql_voice_hearable_protocol.h"
#include "qlsh_commands.h"
#include "host_interface.h"
#include "dbg_uart.h"
#include "ql_controlTask.h"
#include "fsm.h"


/*Voice Packet Headers*/
#define CLIENT_VOICE	1
#define CLIENT_SENSOR	2
#define PKT_TYPE_CMD	1
#define PKT_TYPE_RESP	2
#define PKT_TYPE_EVT	3
#define PKT_SEQ_ID		0

/*Voice Modes*/
#define CFG_VR_OFF				11
#define CFG_VR_ON				12
#define CFG_AUD_STREAMING_OFF	13
#define CFG_AUD_STREAMING_ON	14

/*Voice Packet Header*/
#define CLIENT_ID_MASK_BITS_LOC	(2)			// Bit[1-0]: Client ID
#define PACKET_TYPE_MASK_BITS_LOC	(2)		// Bit[3-2]: Packet Type
#define PACKET_SEQ_ID_MASK_BITS_LOC	(4)		// Bit[7-4]: Packet Sequence ID


#define MAX_RETRY_CNT	2

/*Voice States*/
typedef enum {
	DEVICE_NOT_READY_STATE = 0,
	DEVICE_READY_STATE = 1,
	WAIT_FOR_RESPONSE_STATE = 2,
	RUNNING_STATE = 3,
	STOPPED_STATE = 4,
} VoiceStates;

/*Voice Packet Struture*/
typedef struct
{
	unsigned char clientId : 2;
	unsigned char pktType : 2;
	unsigned char pktSeqId : 4;
	unsigned char pktID;
	unsigned char data1;
	unsigned char data2;
} VoicePkt;

VoiceHandlers vHandlers;
uint8_t currentVoiceCmd;
uint8_t pktSeqId[15];
uint8_t pktIndx = 0;
uint8_t primPhrWrdId = 0;
bool deviceReady = false;
bool vrMode = false;
bool audioStreaming = false;
bool config_done = false;

/*********************************************************************************
 *
 *  UARTrx-FSM interface
 *
 ********************************************************************************/
enum process_state  UARTrx_pstate = PSTATE_UNCONFIG;

enum process_state UARTrx_FSMAction(enum process_action pa, void* pv) {
       
    switch(pa) {   
    case PACTION_CONFIG:
        configASSERT(UARTrx_pstate == PSTATE_UNCONFIG);
	if(QLUART_Init((QLUART_Platform_Info*)pv)!=QLUART_STATUS_OK)
            dbg_str("\n QLUART init failed \n");
        else {
            //dbg_str("QluartInitDone\n");
            Voice_Init();
            UARTrx_pstate = PSTATE_STOPPED;
        }
        break;
        
    case PACTION_START:
        configASSERT(UARTrx_pstate != PSTATE_UNCONFIG);
        UARTrx_pstate = PSTATE_STARTED;
        break;

    case PACTION_STOP:
        configASSERT(UARTrx_pstate != PSTATE_UNCONFIG);
        UARTrx_pstate = PSTATE_STOPPED;
        break;
        
    default:
        UARTrx_pstate = PSTATE_UNKNOWN;
        configASSERT(0);
    }
  return(UARTrx_pstate);
}


static void Prepare_Voice_Pkt_to_BT(VoicePkt *voiceRespEvtPkt, uint32_t pkt);

static void PostCommandtControlTask(struct xQ_Packet *cmdData, uint8_t pktId, uint8_t data1, uint8_t data2,  uint8_t pktType)
{
    volatile uint32_t  uiCommand;
    struct xCQ_Packet CQpacket;

    if (pktType != PKT_TYPE_CMD)
      return;
    
    switch (pktId)
    {
        case CFG_VR:
            if (data1 == VR_ON)
            {
                dbg_str("<--VR_ON\n");
                // rule out unexpected mode/state for this command
                if( config_done == true ) 
                {
                    // send ERR event back
                    dbg_str_int("\n Invalid command ,CFG_VR ON, config_done",config_done);

                    uiCommand = eVOICE_EVT_VA_ERR;
                    uiCommand |= (((UINT32_t)SM_VOICE_TRIG << SHIFT_1_BYTE)  & 0x0000FF00);
                        
                    Voice_Send_RespEvt_to_BT(uiCommand);

                    // Exit switch case and go back to the pend on Queue
                    break;
                }
                else
                {
                  CQpacket.ceEvent = CEVENT_VR_ON;
                  addPktToControlQueue(&CQpacket);

                  config_done = true;
                }
            }
            else
            {
                dbg_str("<--VR_OFF\n");
                CQpacket.ceEvent = CEVENT_VR_OFF;
                addPktToControlQueue(&CQpacket);
                config_done = false;
            }
            break;
        case CFG_AUDIO_STREAMING:
            if (data1 == AUDIO_STREAMING_ON)
            {
                if (data2 != 1)
                {
                    dbg_str("<--STREAM_ON\n");
                    CQpacket.ceEvent = CEVENT_STREAM_ON;
                }
                else
                {
#if ENABLE_OPUS_ENCODER == 0
                    dbg_str("<--STREAM_VCALL_ON\n");
                    CQpacket.ceEvent = CEVENT_STREAM_VOICE_CALL_ON;
#else
                    dbg_str("<--STREAM_VCALL_ON (Not Supported)\n");
                    dbg_str("\nSending error response..");
                    uiCommand = eVOICE_EVT_VA_ERR;
                    uiCommand |= (((UINT32_t)SM_VOICE_TRIG << SHIFT_1_BYTE)  & 0x0000FF00);

                    Voice_Send_RespEvt_to_BT(uiCommand);
#endif
                }
                addPktToControlQueue(&CQpacket);
            }
            else
            {
                dbg_str("<--STREAM_OFF\n");
                if (isStreamingOn()) 
                {
                  CQpacket.ceEvent = CEVENT_STREAM_OFF;
                  addPktToControlQueue(&CQpacket);
                  config_done = false;
                }
                else //Error
                {
                    dbg_str("\nSending error response..");
                    uiCommand = eVOICE_EVT_VA_ERR;
                    uiCommand |= (((UINT32_t)SM_VOICE_TRIG << SHIFT_1_BYTE)  & 0x0000FF00);

                    Voice_Send_RespEvt_to_BT(uiCommand);
                }
            }
            break;
        default:
            dbg_str("Invalid command sent from BT \n");
            break;
        }

	return;
}

QLUART_Status Voice_Qluart_Cb(uint8_t*data, uint16_t len,void* cookie,uint8_t pkt_type)
{
    struct xQ_Packet pktMsg;
    //dbg_str("\n Voice_Qluart_Cb \n");
#if 0
    for(int i=0;i<len;i++)
    {
        //printf("\n %d = %d \n",i,data[i]);
        dbg_str_int("\n", i);
        dbg_str_int(" = ", data[i]);
        dbg_str("\n");
    }
#endif

    if ((data[0] & MASK_2_BIT) != CLIENT_VOICE)
      return QLUART_STATUS_ERROR;
    
    /*Parse the voice packet and prepare the packet for APTask*/
    switch ((data[0] >> PACKET_TYPE_MASK_BITS_LOC) & MASK_2_BIT)
    {
        case PKT_TYPE_CMD:
            pktSeqId[pktIndx] = (data[0] >> PACKET_SEQ_ID_MASK_BITS_LOC) & MASK_4_BIT; // storing the packet header
            if (pktIndx <= 15)
                pktIndx++;
            else
                pktIndx = 0;

            /*Send the command packet to Control Task*/
            PostCommandtControlTask(&pktMsg, data[1], data[2], data[3], PKT_TYPE_CMD);

            break;

        case PKT_TYPE_RESP:
            dbg_str("Response Packet Received.. Not expected\n");
            break;
        case PKT_TYPE_EVT:
            dbg_str("Event Packet Received.. Not expected\n");
            break;
        default:
            dbg_str("Invalid Voice Packet Type \n");
            break;
    }

    return QLUART_STATUS_OK;
}


static void Prepare_Voice_Pkt_to_BT(VoicePkt *voiceRespEvtPkt, uint32_t pkt)
{
    voiceRespEvtPkt->clientId = CLIENT_VOICE;

    switch (pkt & MASK_8_BIT)
    {
        case eVOICE_EVT_VA_ACK:
            voiceRespEvtPkt->pktType = PKT_TYPE_RESP;
            voiceRespEvtPkt->pktSeqId = pktSeqId[pktIndx];
            voiceRespEvtPkt->pktID = RESP_SUCCESS;

            voiceRespEvtPkt->data1 = (pkt>>16)&0xFF;
            voiceRespEvtPkt->data2 = (pkt>>24)&0xFF;
            break;
        case eVOICE_EVT_VA_ERR:
            voiceRespEvtPkt->pktType = PKT_TYPE_RESP;
            voiceRespEvtPkt->pktSeqId = pktSeqId[pktIndx];
            voiceRespEvtPkt->pktID = RESP_FAILURE;

            voiceRespEvtPkt->data1 = (pkt>>16)&0xFF;
            voiceRespEvtPkt->data2 = (pkt>>24)&0xFF;

            break;
        case eVOICE_EVT_VA_READY:
            voiceRespEvtPkt->pktType = PKT_TYPE_EVT;
            voiceRespEvtPkt->pktSeqId = pktSeqId[pktIndx];
            voiceRespEvtPkt->pktID = EVT_READY;

            voiceRespEvtPkt->data1 = (pkt>>16)&0xFF;
            voiceRespEvtPkt->data2 = (pkt>>24)&0xFF;

            break;
        case eVOICE_EVT_VA_KPT:
            voiceRespEvtPkt->pktType = PKT_TYPE_EVT;
            voiceRespEvtPkt->pktSeqId = pktSeqId[pktIndx];
            voiceRespEvtPkt->pktID = EVT_PHRASE_DET;
            voiceRespEvtPkt->data1 = ((pkt >> SHIFT_2_BYTES) & MASK_8_BIT);   //KPT id
            voiceRespEvtPkt->data2 = 0;
            primPhrWrdId = voiceRespEvtPkt->data1;
            break;
    #ifdef KPDCMD
        case eVOICE_EVT_VA_KPT_CMD:
            voiceRespEvtPkt->pktType = PKT_TYPE_EVT;
            voiceRespEvtPkt->pktSeqId = pktSeqId[pktIndx];
            voiceRespEvtPkt->pktID = EVT_PHRASE_DET;
            voiceRespEvtPkt->data1 = primPhrWrdId;
            voiceRespEvtPkt->data2 = ((pkt >> SHIFT_3_BYTES) & MASK_8_BIT);
            break;
    #endif                        
            case eVOICE_EVT_VA_AUDIO_DATA:
            printf("\n Voice data event \n");
            break;
                
        default:
            printf("Invalid Response/Event %d",((pkt >> COMMAND_MASK_BITS_LOC) & MASK_4_BIT));
            voiceRespEvtPkt->pktType = PKT_TYPE_EVT;
            voiceRespEvtPkt->pktSeqId = pktSeqId[pktIndx];
            voiceRespEvtPkt->pktID = RESP_FAILURE;
            voiceRespEvtPkt->data1 = (pkt>>16)&0xFF;
            voiceRespEvtPkt->data2 = (pkt>>24)&0xFF;
            break;
        }
	return;
}

/*Send Voice Response/Event to BT*/
uint32_t Voice_Send_RespEvt_to_BT(uint32_t pkt)
{
    uint32_t retVal = VOICE_SUCCESS;
    /*Retry count for timeout error from QLUART*/
    uint32_t retry_cnt = 0;
    VoicePkt voiceRespEvtPkt = {0};

    Prepare_Voice_Pkt_to_BT(&voiceRespEvtPkt, pkt);

    /*Call QLUART Trasmit API to trasmit the voice packet over uart*/
    retVal = QLUART_Transmit((uint8_t *)&voiceRespEvtPkt, QLU_VOICE_PKT_LEN,CONTROL_DATA_PKT,QLUART_CLIENT_VOICE);
    if(retVal != QLUART_STATUS_OK)
    {
	if (retVal == QLUART_STATUS_ERROR_TIMEOUT)
	{
            //dbg_str_int("Uart Timeout..retrying.. Err",retVal);

            while (retVal == QLUART_STATUS_ERROR_TIMEOUT)
            {
		//printf("Retrying..retry count = %d \n", retry_cnt);
		if (retry_cnt > MAX_RETRY_CNT)
		{
                    dbg_str("Err: retry failed (RespEvt)\n");
                    retVal = VOICE_UART_TIME_OUT_ERROR;
                    break;
		}
		retVal = QLUART_Transmit((uint8_t *)&voiceRespEvtPkt, QLU_VOICE_PKT_LEN,CONTROL_DATA_PKT,QLUART_CLIENT_VOICE);
		retry_cnt++;
                //printf("retry %d\n", retry_cnt);
            }

	}
	else
	{
            printf("Uart Transmit FAIL: error cause= %d",retVal);
            retVal = VOICE_UART_RESPEVT_TX_ERROR;
	}
    }

    return retVal;
}

/*Send Voice Response/Event to BT*/
uint32_t Voice_Send_DataPkt_to_BT(uint8_t *pkt_buf, uint8_t data_len)
{
    uint32_t retVal = VOICE_SUCCESS;
    /*Retry count for timeout error from QLUART*/
    uint32_t retry_cnt = 0;
    VoicePkt *voiceDataPkt;
    uint8_t pkt_len;

#ifdef OPUS_HDR_EXCLUDE
    voiceDataPkt = (VoicePkt *)((uint8_t*)pkt_buf+OPUS_HDR_LEN);
    pkt_len = (data_len-OPUS_HDR_LEN);
#else
    voiceDataPkt = (VoicePkt *)pkt_buf;
    pkt_len = data_len;	
#endif	
	
    /*Call QLUART Trasmit API to trasmit the voice packet over uart*/
    retVal = QLUART_Transmit((uint8_t *)voiceDataPkt, pkt_len,ISO_DATA_PKT,QLUART_CLIENT_VOICE);
    if(retVal != QLUART_STATUS_OK)
    {
        if (retVal == QLUART_STATUS_ERROR_TIMEOUT)
	{
            printf("Uart Timeout..err %d retrying \n",retVal);

            while (retVal == QLUART_STATUS_ERROR_TIMEOUT)
            {
                //printf("Retrying..retry count = %d \n", retry_cnt);
		if (retry_cnt > MAX_RETRY_CNT)
		{
                    dbg_str("Err: retry failed (DataPkt)\n");
                    retVal = VOICE_UART_TIME_OUT_ERROR;
                    break;
		}
                retVal = QLUART_Transmit((uint8_t *)voiceDataPkt, pkt_len,ISO_DATA_PKT,QLUART_CLIENT_VOICE);
                retry_cnt++;
                //printf("retry %d\n", retry_cnt);
            }
	}
	else
	{
            printf("Uart Transmit FAIL: error cause= %d",retVal);
            retVal = VOICE_UART_RESPEVT_TX_ERROR;
	}
    }

    //dbg_ch('.');	//dbg, indicates OPUS data streaming
	
    return retVal;
}


/*Get the Voice Device Ready Status*/
bool Voice_Device_Ready_Status (void)
{
    return deviceReady;
}


/*Voice Initialization*/
uint32_t Voice_Init(void)
{
    int *cookie =  (int *)0x1234;
    QLUART_Register_Receive_CallBack(Voice_Qluart_Cb,cookie,QLUART_CLIENT_VOICE);

    config_done = false;
    
    return VOICE_SUCCESS;
}
