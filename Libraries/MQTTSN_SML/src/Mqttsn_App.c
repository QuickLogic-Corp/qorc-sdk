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
*    File   : Mqttsn_App.c
*    Purpose: MQTTSN Application to handle imcoming messages
*             and call topic processing if necessary
*                                                          
*=========================================================*/

#include "Fw_global_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RtosTask.h"
#include "iop_messages.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "dbg_uart.h"
#include "Mqttsn_App.h"
#include "Mqttsn_Topics.h"     
#include "Mqttsn_MessageHandler.h"
#include "mqttsn_comm.h"
#include "ql_time.h"
#include "ql_fs.h"
#include "Mqttsn_CliCmds.h"

#define MQTTSN_MIN_MSG_LEN          2
#define MQTTSN_MAX_LEN_BYTES        3
#define MQTTSN_MSG_WAIT_TIME        10
#define INCOMING_MSG_QUEUE_LENGTH   3
#define MESSAGE_PROFILING           0
#define MQTTSN_CONNECTION_REQ_INTERVAL  1000

#if RECOG_VIA_BLE

typedef struct{
    uint32_t mqttsnTopicId;   //MQTTSN topic ID
    uint32_t iopMsgId;       //equivalent IOP msg id
}Mqttsn_Iop_Mapper_t;

/* Message dispatch table for each MQTTSN message */
static const Mqttsn_Iop_Mapper_t MqttsnIopMap[] = 
{
    {.mqttsnTopicId = TOPIC_RESULT_CLASS_START, .iopMsgId = IOP_MSG_RECO_CLASSIFICATION_START},
    {.mqttsnTopicId = TOPIC_RESULT_CLASS_STOP, .iopMsgId = IOP_MSG_RECO_CLASSIFICATION_STOP},
    {.mqttsnTopicId = TOPIC_RESULT_CLASS_DATA, .iopMsgId = IOP_MSG_RECO_CLASSIFICATION_DATA},
    {.mqttsnTopicId = TOPIC_RESULT_CLASS_START, .iopMsgId = IOP_MSG_RECO_FEATURES_START},
    {.mqttsnTopicId = TOPIC_RESULT_CLASS_STOP, .iopMsgId = IOP_MSG_RECO_FEATURES_STOP},
    {.mqttsnTopicId = TOPIC_RESULT_CLASS_DATA, .iopMsgId = IOP_MSG_RECO_FEATURE_DATA},
    {.mqttsnTopicId = NULL, .iopMsgId = NULL}
};

uint8_t Recog_Mqttsn_Msg_Template[7] = {0x7, 0xc, 0x0, 0x0, 0x0, 0x0, 0x0};
#endif
xTaskHandle xHandleTaskMqttsnApp;
QueueHandle_t MqttsnApp_InMsgQ;
MqttsnState_t mqttsnState = MQTTSN_STATE_DISCONNECTED;
uint32_t numTopics = 0;
uint32_t subscrIdx = 0;
uint8_t UuidReqMsg[7] = {0x7, 0xc, 0x0, 0x0, 0x0, 0x0, 0x1};
uint8_t DisconnectMsg[2] = {0x2, 0x18};
#define MAX_INVALID_LEN_MSGS 10 //max invalid messages threshold

typedef struct{
    enum MQTTSN_msgTypes msgType;   //MQTTSN msg type
    void (*msgHandler)(Mqttsn_IOMsgData_t *);
}Mqttsn_AppMsgHandler_t;

typedef enum{
    MSG_READ_INITIAL,   //buffer allocation
    MSG_READ_REM_BYTES, //read any remaining bytes
    MSG_READ_END        //end of message read, add to the Queue
}MqttsnMsgReadStates_t;

void my_ble_callback( uint8_t *pData )
{
#if RECOG_VIA_BLE    
    const Mqttsn_Iop_Mapper_t *pCmd;
   
    pCmd = &MqttsnIopMap[0];
    while(pCmd->mqttsnTopicId)
    {
        if( pCmd->iopMsgId != pData[1] )
        {
            pCmd++;
            continue;
        } else {
            break;
        }
    }
    
    if(pCmd)
    {
        uint8_t mqttsnMsg[8];
        Mqttsn_TopicInfo_t *pTopicId;
        uint32_t msgLen = 8;
        
        if(mqttsnState != MQTTSN_STATE_READY_FOR_PUBLISH)
        {
            Mqttsn_FakeSubscriptions();
            mqttsnState = MQTTSN_STATE_READY_FOR_PUBLISH;
        }
        
        pTopicId = Mqttsn_GetTopicInfo(pCmd->mqttsnTopicId);
        
        //Construct the MQTTSN message
        memcpy((void *)mqttsnMsg, (const void *)Recog_Mqttsn_Msg_Template, sizeof(Recog_Mqttsn_Msg_Template));

        if(pCmd->iopMsgId == IOP_MSG_RECO_CLASSIFICATION_START)
        {
            mqttsnMsg[7] = 1;
        }
        else if(pCmd->iopMsgId == IOP_MSG_RECO_FEATURES_START)
        {
            mqttsnMsg[7] = 2;
        }
        else if((pCmd->iopMsgId == IOP_MSG_RECO_CLASSIFICATION_STOP)||
                (pCmd->iopMsgId == IOP_MSG_RECO_FEATURES_STOP))
        {
            msgLen = 7;
        }
        
        mqttsnMsg[0] = msgLen;
        mqttsnMsg[3] = GET_TOPIC_MAJOR(pTopicId->assgndID.data.id);
        mqttsnMsg[4] = GET_TOPIC_MINOR(pTopicId->assgndID.data.id);
        
        //Add to the input queue
        CLI_ProcessDataIn(mqttsnMsg, msgLen);
    }
#endif  //#if RECOG_VIA_BLE    
}

void CLI_ProcessDataIn(uint8_t *pData, uint32_t numBytes)
{
    uint32_t result;
    MqttsnPdu_t pdu;
    
    result = Mqttsn_AllocMsg(numBytes>MQTTSN_BUFFER_SMALL?MQTTSN_BUFFER_LARGE:MQTTSN_BUFFER_SMALL,
                             &pdu.pBuf, &pdu.allocLen);
    
    if(result)
    {
        pdu.numBytes = numBytes;
        
        memcpy((void *)pdu.pBuf, pData, numBytes);
        
        //Add it to the incoming message queue
        if(xQueueSend(MqttsnApp_InMsgQ, ( void * ) &pdu, 100) != pdPASS)
        {
            printf("MQTTSN: No queue entry available\n");
        }
    }
    else
    {
        printf("CLI_ProcessDataIn: Could not allocate buffer\n");
    }
    
}


void ProcessDataIn(void)
{
    static MqttsnPdu_t pdu;
    uint32_t availableBytes;
    static MqttsnMsgReadStates_t msgReadState = MSG_READ_INITIAL;
    static uint32_t pendingBytes = 0;
    static uint8_t *pPduBuff; 
    static uint8_t msgLen[MQTTSN_MAX_LEN_BYTES] = {0, 0, 0};
    uint32_t result = 0;
    static intptr_t rxTimerToken;
    static uint32_t invalidLenMsgCnt = 0;
   
    switch(msgReadState)
    {
        case MSG_READ_INITIAL:

        memset((void *)&pdu, 0, sizeof(pdu));
        
        availableBytes = mqttsn_comm_rx_available();
    
        if(availableBytes >= MQTTSN_MIN_MSG_LEN)
        {

            mqttsn_comm_rx(msgLen, MQTTSN_MIN_MSG_LEN);

            uint32_t bytesRead = MQTTSN_MIN_MSG_LEN;
                    
            if(msgLen[0] != 0)
            {
                invalidLenMsgCnt = 0;
                
                if(msgLen[0] == 1)
                {
                    availableBytes = mqttsn_comm_rx_available();
                    
                    if(availableBytes>=1)
                    {
                        mqttsn_comm_rx(&msgLen[MQTTSN_MAX_LEN_BYTES-1], 1);
                        
                        bytesRead += 1;
                        
                        //allocate a large message buffer
                        result = Mqttsn_AllocMsg(MQTTSN_BUFFER_LARGE,
                                                 &pdu.pBuf, &pdu.allocLen);
                        if(result == 0)
                        {
                            //print message here, ASSERT on NULL buffer at the end
                            printf("Could not allocate message buffer\n");
                        }
                        
                        pdu.numBytes = (msgLen[1]<<8)|msgLen[2];
                    }
                }
                else
                {
#if MESSAGE_PROFILING                    
                     printf("Mqttsn_App -- ProcessDataIn -- Initial Read 0x%x\n", ql_lw_timer_start());
#endif  //MESSAGE_PROFILING                
                    //allocate a small message buffer
                    result = Mqttsn_AllocMsg(MQTTSN_BUFFER_SMALL,
                                             &pdu.pBuf, &pdu.allocLen);
                    
                    if(result == 0)
                    {
                        printf("Could not allocate message buffer\n");
                    }
                    
                    pdu.numBytes = msgLen[0];
                }

                memcpy((void *)pdu.pBuf, (const void *)&msgLen, bytesRead);
                
                pPduBuff = pdu.pBuf + bytesRead;
                pendingBytes = pdu.numBytes - bytesRead;
                
                rxTimerToken = ql_lw_timer_start();
                
                //if there are no more pending bytes, add the message to the queue
                //this happens when the message length is 3 bytes like in CONNACK
                if(pendingBytes == 0)
                    msgReadState = MSG_READ_END;
                else
                    msgReadState = MSG_READ_REM_BYTES;
            }
            else
            {
                invalidLenMsgCnt++;
                
                //Consecutive errors could mean we lost sync or there is noise coming in,
                //disconnect from the network.
                if((mqttsnState != MQTTSN_STATE_DISCONNECTED)&&
                   (invalidLenMsgCnt > MAX_INVALID_LEN_MSGS))
                {
                    invalidLenMsgCnt = 0;
                    printf("Invalid MQTTSN Message length field %d %d %d %d\n", msgLen[0], msgLen[1], msgLen[2], invalidLenMsgCnt);
                    //Disconnect
                    CLI_ProcessDataIn(DisconnectMsg, 2);
                }
            }

        }
        break;
        
        case MSG_READ_REM_BYTES:
        
        if(ql_lw_timer_is_expired( rxTimerToken, 200 ))
        {
            //Timer expired dump data and look for newer bytes
            Mqttsn_FreeMsg(&pdu.pBuf);
            msgReadState = MSG_READ_INITIAL;
            return;
        }
            
        availableBytes = mqttsn_comm_rx_available();
        
#if MESSAGE_PROFILING         
        printf("Mqttsn_App -- ProcessDataIn -- MSG_READ_REM_BYTES 0x%x\n", ql_lw_timer_start());
        printf("Available: %d Pending: %d\n", availableBytes, pendingBytes);
#endif //#if MESSAGE_PROFILING      
        
        if((pendingBytes) &&(availableBytes))
        {
            if(availableBytes>=pendingBytes)
            {
                mqttsn_comm_rx(pPduBuff, pendingBytes);
                
                //we received all the message bytes, get ready to read the next message
                msgReadState = MSG_READ_END;
                pendingBytes = 0;
#if MESSAGE_PROFILING                 
                printf("Read COMPLETE 0x%x\n", ql_lw_timer_start());
#endif //#if MESSAGE_PROFILING                 
            }
            else
            {
                mqttsn_comm_rx(pPduBuff, availableBytes);

                pendingBytes -= availableBytes;
                pPduBuff += availableBytes;
            }
        }
        break;
        
        case MSG_READ_END:
#if MESSAGE_PROFILING         
        printf("Mqttsn_App -- ProcessDataIn -- MSG_READ_END 0x%x\n", ql_lw_timer_start());
#endif //#if MESSAGE_PROFILING         
        
        memset(msgLen, 0, sizeof(msgLen));
        
        //Add it to the incoming message queue
        if(xQueueSend(MqttsnApp_InMsgQ, ( void * ) &pdu, 100) != pdPASS)
        {
            printf("MQTTSN: No queue entry available\n");
        }
        msgReadState = MSG_READ_INITIAL;
        
        break;
        
        default:
        break;
    }
}

void HandleDisconnect(void)
{
    mqttsnState = MQTTSN_STATE_DISCONNECTED;

    //Stop any streaming, collection, storage
    do_all_stop(NULL);
    
    //update subscription status of all topics
    Mqttsn_UnsubscribeAllTopics();
}

static void HandleDisconnectState(Mqttsn_IOMsgData_t *pIoMsg)
{
    static intptr_t lastConnectionReqTime = 0;
    
    if((lastConnectionReqTime == 0)||
       (ql_lw_timer_is_expired(lastConnectionReqTime, MQTTSN_CONNECTION_REQ_INTERVAL )))
    {
        //printf("Connection start\n");
        Mqttsn_SendConnectionRequest();
        //printf("Connection sent\n");

        lastConnectionReqTime = ql_lw_timer_start();
    }
    
    if((pIoMsg)&&
       (pIoMsg->incomingMsg.msgType == MQTTSN_CONNACK)&&
       (pIoMsg->incomingMsg.retCode == MQTTSN_ACCEPTED))
    {
        mqttsnState = MQTTSN_STATE_CONNECTED;
        
        //Start subscription and transition to subcribing status
        subscrIdx = 0;
        mqttsnState = MQTTSN_STATE_SUBSCRIBING;
        
        //mount the default storage
        QLFS_mount_as_default( DEFAULT_STORAGE_LOCATION );
    }
}

static void HandleSubscribingState(Mqttsn_IOMsgData_t *pIoMsg)
{
#if SKIP_SUBSCRIBING
    mqttsnState = MQTTSN_STATE_SUBSCR_COMPLETE;
    
    Mqttsn_FakeSubscriptions();
    
    subscrIdx = numTopics;
#else
    
    if((pIoMsg) && (pIoMsg->incomingMsg.msgType == MQTTSN_DISCONNECT))
    {
        HandleDisconnect();
        return;
    }
    
    //Send a single subscription, wait for Ack and send another
    if(subscrIdx<numTopics)
    {
        if(Topics[subscrIdx].isRegistration)
        {
#if MESSAGE_PROFILING                     
            printf("Mqttsn_App.c -- Sending Register for %d 0x%x\n", subscrIdx, ql_lw_timer_start());
#endif //#if MESSAGE_PROFILING 
            
            Mqttsn_Register(&Topics[subscrIdx]);
        }
        else
        {
#if MESSAGE_PROFILING                     
            printf("Mqttsn_App.c -- Sending Subscribe for %d 0x%x\n", subscrIdx, ql_lw_timer_start());
#endif //#if MESSAGE_PROFILING                     
            
            Mqttsn_Subscribe(&Topics[subscrIdx]);
        }
        
        mqttsnState = MQTTSN_STATE_SUBS_ACK_PENDING;
    }
    else
    {
        //Publish online status
        Mqttsn_PublishWillStatus(MQTTSN_WILL_STATUS_ONLINE);
        
        mqttsnState = MQTTSN_STATE_READY_FOR_PUBLISH;
        
        //Send an unsolicited response in order to publish the UUIDs signalling the end
        //of subscription+registration process
        Mqttsn_TopicInfo_t *pTopicId = Mqttsn_GetTopicInfo(TOPIC_SYS_DEVICE_UUIDS_REQ);
        UuidReqMsg[3] = GET_TOPIC_MAJOR(pTopicId->assgndID.data.id);
        UuidReqMsg[4] = GET_TOPIC_MINOR(pTopicId->assgndID.data.id);
        CLI_ProcessDataIn(UuidReqMsg, 7);
        
    }
#endif            

}

static void HandleSubsAckPendingState(Mqttsn_IOMsgData_t *pIoMsg)
{
    if(pIoMsg)
    {
        if((pIoMsg->incomingMsg.msgType == MQTTSN_SUBACK)||
           (pIoMsg->incomingMsg.msgType == MQTTSN_REGACK))
        {
            Mqttsn_MsgData_t *pInMsgData = &pIoMsg->incomingMsg;
            
            //update subscription status
            Mqttsn_UpdateTopicSubStatus(pInMsgData->topicId,
                                        (pInMsgData->retCode == MQTTSN_ACCEPTED)?1:0,
                                        pInMsgData->qosLevel);
            //go to next subscription
            subscrIdx++;
            mqttsnState = MQTTSN_STATE_SUBSCRIBING;     
        }
        else if(pIoMsg->incomingMsg.msgType == MQTTSN_DISCONNECT)
        {
            HandleDisconnect();
        }
    }
}

static void HandlePublishState(Mqttsn_IOMsgData_t *pIoMsg)
{
    if(pIoMsg)
    {
        if((pIoMsg->incomingMsg.msgType == MQTTSN_PUBLISH)||
           (pIoMsg->incomingMsg.msgType == MQTTSN_PUBACK))
        {
            //In case of incoming message of type PUBLISH
            //The reply will contain an Ack/Nack
            //Outgoing PUBLISh will contain the response
            
            //In case of incoming message of type PUBACK
            //The reply will be left NULL, there is no reply to an ack
            //Outgoing PUBLISH will contain the next PUBLISH, this is
            //mainly used during file transfers.
            
            //Process the topic 
            MajorTopicDispatch(pIoMsg);
            
            //Check if there is a reply to send here
            if(pIoMsg->outgoingAck.pMsgBuf)
            {
                //PUBACK should never come here
                configASSERT(pIoMsg->incomingMsg.msgType != MQTTSN_PUBACK);
                
                Mqttsn_SendReply(&pIoMsg->outgoingAck);
                
            }
            
            //Check if there is an outgoing PUBLISH to send here
            if(pIoMsg->outgoingResponse.pMsgPayldBuf)
            {
                Mqttsn_TxPublishErrorCodes_t publishErr;
                Mqttsn_MsgData_t *pOPublish = &pIoMsg->outgoingResponse;
                Mqttsn_TopicInfo_t *pTpcInfo;
                
                pTpcInfo = Mqttsn_GetTopicInfo(pOPublish->topicId);
                
                configASSERT(pTpcInfo);
                
                publishErr = Mqttsn_SendPublish(&pIoMsg->outgoingResponse, pTpcInfo);
            }    
        }
        else if(pIoMsg->incomingMsg.msgType == MQTTSN_DISCONNECT)
        {
            HandleDisconnect();
        }
    }
}

void (*Mqttsn_StateHandler[])(Mqttsn_IOMsgData_t *) = {    HandleDisconnectState,  //MQTTSN_STATE_DISCONNECTED
                                    NULL,                   //MQTTSN_STATE_CONNECTED
                                    HandleSubscribingState, //MQTTSN_STATE_SUBSCRIBING
                                    HandleSubsAckPendingState,  //MQTTSN_STATE_SUBS_ACK_PENDING
                                    NULL,                   //MQTTSN_STATE_SUBSCR_COMPLETE
                                    HandlePublishState};    //MQTTSN_STATE_READY_FOR_PUBLISH

  
void MqttsnAppTaskHandler(void *pParameter)
{
    BaseType_t qret;
    Mqttsn_IOMsgData_t ioMsgData;
    MqttsnPdu_t mqttsnPdu;
    
    mqttsn_comm_setup();
    
    
    while(1)
    {
        //Read UART to see if there are any new messages
        ProcessDataIn();
        
        if(*(Mqttsn_StateHandler[mqttsnState]))
        {
            (*(Mqttsn_StateHandler[mqttsnState]))(NULL);
        }
        
        /*read the incoming message from the message queue*/
        memset(&ioMsgData, 0, sizeof(ioMsgData));
        qret = xQueueReceive(MqttsnApp_InMsgQ, &mqttsnPdu, MQTTSN_MSG_WAIT_TIME);
        
        if(qret)
        {
            uint32_t rdSuccess;

            //Deserialise and process the incoming message    
            rdSuccess = Mqttsn_ProcessIncmgMsg(&ioMsgData, &mqttsnPdu);
            
            if(rdSuccess)
            {
                if(*(Mqttsn_StateHandler[mqttsnState]))
                {
                    (*(Mqttsn_StateHandler[mqttsnState]))(&ioMsgData);
                }
            }
            
            Mqttsn_FreeMsg(&ioMsgData.incomingMsg.pMsgBuf);
        }
    }
    
} 

/* Setup msg queue and Task Handler for IO Msg Task */
signed portBASE_TYPE StartRtosTaskMqttsnApp( void)
{
    static uint8_t ucParameterToPass;
    
    //Count the number of topics to register/subscribe to
    while(Topics[numTopics].topicValue != MAX_TOPIC_ID)
    {
        numTopics++;
    }
    
    /* Create queue for incoming messages to be processed at a later time */
    MqttsnApp_InMsgQ = xQueueCreate( INCOMING_MSG_QUEUE_LENGTH, sizeof(MqttsnPdu_t) );
    vQueueAddToRegistry( MqttsnApp_InMsgQ, "Incoming Message Q" );
    configASSERT( MqttsnApp_InMsgQ );
    
    /* Create IO Task */
    xTaskCreate ( MqttsnAppTaskHandler, "MqttsnAppTaskHandler", STACK_SIZE_ALLOC(STACK_SIZE_TASK_MQTTSN_APP),  &ucParameterToPass, PRIORITY_TASK_MQTTSN_APP, &xHandleTaskMqttsnApp);
    configASSERT( xHandleTaskMqttsnApp );
    
    return pdPASS;
}
