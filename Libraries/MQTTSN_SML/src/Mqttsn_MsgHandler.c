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
*    File   : mqttsn.c
*    Purpose: MQTTSN APIs and message processing
*                                                          
*=========================================================*/
#include "Fw_global_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "RtosTask.h"
#include "Mqttsn_MessageHandler.h"
#include "mqttsn_comm.h"
#include "cli.h"
#include "ql_time.h"
#include "Mqttsn_App.h"
#include "MQTTSNPacket.h"

#define MQTTSN_MSGHANDLER_TRACES 0

#define MSG_PUBLISH_PAYLD_OFFSET_LT256    7 //Publish payload offset when message length is less than 256 bytes
#define MSG_PUBLISH_PAYLD_OFFSET_GT256    9 //Publish payload offset when message length is more than 256 bytes

mqttsnInfo_t mqttsnInfo;
uint8_t mqttsnBuff[NUM_BUFFERS_SMALL][MQTTSN_BUFFER_SMALL];
uint8_t mqttsnBuffLrg[NUM_BUFFERS_LARGE][MQTTSN_BUFFER_LARGE];

#define MQTTSN_SMALL_BUFF_START_ADDR    (&mqttsnBuff[0][0])
#define MQTTSN_SMALL_BUFF_END_ADDR      ((&mqttsnBuff[0][0]) + ((NUM_BUFFERS_SMALL) * (MQTTSN_BUFFER_SMALL)))

#define MQTTSN_LARGE_BUFF_START_ADDR    (&mqttsnBuffLrg[0][0])
#define MQTTSN_LARGE_BUFF_END_ADDR      ((&mqttsnBuffLrg[0][0]) + ((NUM_BUFFERS_LARGE) * (MQTTSN_BUFFER_LARGE)))

static const char *pWillStatusStrings[] = {
    "offline", "online"
};

//MQTTSN Task variables
xTaskHandle xHandleTaskMQTTSN;
QueueHandle_t MQTTSN_TxMsgQ;
QueueHandle_t MQTTSN_FreeBuffQSm;
QueueHandle_t MQTTSN_FreeBuffQLrg;

/*API to initialize most fields inside a Mqttsn_MsgData_t data structure*/
void InitMsgData(Mqttsn_MsgData_t *pMsgData, MsgDirection_t  msgDirec, MQTTSN_retCodes_t retCode,
                uint8_t retain, enum MQTTSN_msgTypes msgType, uint16_t pktId)
{
    pMsgData->msgDirec = msgDirec;
    pMsgData->retCode = retCode;
    pMsgData->retain = retain;
    pMsgData->msgType = msgType;
    pMsgData->pktId = pktId;
}

/*Returns the next message ID. Acc to the spec, its a 16-bit number*/
static uint16_t GetNextMsgId(void)
{
    taskENTER_CRITICAL();     
    uint16_t msgId = mqttsnInfo.nextMsgId;
   
    mqttsnInfo.nextMsgId += 1;
    
    if(mqttsnInfo.nextMsgId == 0)
    {
        mqttsnInfo.nextMsgId = 1;
    }
    
    taskEXIT_CRITICAL();   

    return msgId;
        
}

/*Function to reset the pending queue*/
void ResetPendingQ(void)
{
    memset((void *)&mqttsnInfo.pendingMsgQueue, 0, sizeof(mqttsnInfo.pendingMsgQueue));
}

/*Returns a pointer to a free/available entry in the pending queue*/
static pendingMsg_t *GetFreePendQEntry(void)
{
    uint32_t idx;
    pendingMsg_t *pMsg = NULL;
    
    for(idx = 0; idx < MAX_PENDING_MSGS; idx++)
    {
        if(mqttsnInfo.pendingMsgQueue[idx].isValid == false)
        {
            pMsg = (&mqttsnInfo.pendingMsgQueue[idx]);
            break;
        }
    }
    
    return pMsg;
}

/*Adds an entry to the pending queue to track for retransmission*/
static uint32_t AddPendQEntry(uint16_t msgId, uint16_t topicId, enum MQTTSN_msgTypes currMsg,
                               intptr_t txTimerToken, uint8_t *pMsgBuff, uint32_t msgLen, uint32_t allocLen)
                               
{
    uint32_t result = 0;
    pendingMsg_t *pNewEntry;
    
    //Get a free entry
    pNewEntry = GetFreePendQEntry();
    
    if(pNewEntry)
    {
        pNewEntry->isValid = true;  
        pNewEntry->msgId = msgId;
        pNewEntry->topicId = topicId;
        pNewEntry->currMsg = currMsg;
        pNewEntry->numTries = 0;
        pNewEntry->txTimerToken = txTimerToken;
        pNewEntry->pMsgBuff = pMsgBuff;
        pNewEntry->msgLen = msgLen;
        pNewEntry->allocLen = allocLen;
        result = 1;
    }
    else
    {
        //error no entry present in pending queue
        printf("error no entry present in pending queue\n");
    }
    
    return result;
}

/*Match the input pkt ID and return a pointer to an entry in the pending queue*/
static pendingMsg_t *GetPendingMsg(uint16_t pktId, enum MQTTSN_msgTypes msgType)
{
    uint32_t idx;
    bool chkMsgId = 1;
    pendingMsg_t *pEntry = NULL;
    
    //Connect and ping req do not have a msg id
    if((msgType == MQTTSN_CONNECT)||
       (msgType == MQTTSN_PINGREQ))
    {
        chkMsgId = 0;
    }
    
    for(idx = 0; idx < MAX_PENDING_MSGS; idx++)
    {
        if((mqttsnInfo.pendingMsgQueue[idx].isValid)&&
           (((chkMsgId)&&(mqttsnInfo.pendingMsgQueue[idx].msgId == pktId))||
            ((!chkMsgId)&&(mqttsnInfo.pendingMsgQueue[idx].currMsg == msgType))))
        {
            pEntry = &mqttsnInfo.pendingMsgQueue[idx];
            break;
        }
    }
    
    return pEntry;
}

/*Clears a pending queue entry. pkt ID and/or msg type is required to retrieve the entry*/
static uint32_t ClearPendingMsg(uint16_t pktId, enum MQTTSN_msgTypes msgType)
{
    uint32_t result = 0;
    pendingMsg_t *pEntry = NULL;
    
    //Get the pending entry
    pEntry = GetPendingMsg(pktId, msgType);
    
    if(pEntry)
    {
        pEntry->isValid = false;
        Mqttsn_FreeMsg(&pEntry->pMsgBuff);
        result = 1;
    }
    
    return result;
}

/* Add a message to the Tx queue*/
int AddToTxQ(Mqttsn_MsgData_t *pMsgData)
{
    int result = 1;
    
    if((pMsgData) && (pMsgData->pMsgBuf))
    {
        //Add it to the Tx Queue
        if(xQueueSend( MQTTSN_TxMsgQ, pMsgData, ( TickType_t )MQTTSN_MSGQ_SEND_WAIT_TICKS) != pdPASS )
        {
            /* Failed to post the message, even after 10 ticks. */
            result = 0;
        } 
    }
    else
    {
        result = 0;
        printf("Tried to add an unallocated message to the Tx Q");
    }
    
    return result;
}

/*Process an advertise message from the Gateway*/
static void ProcessAdvertise(Mqttsn_IOMsgData_t *pIoMsgData)
{
    uint32_t result;
    Mqttsn_MsgData_t *pMsgData = &pIoMsgData->incomingMsg;
    
    if(mqttsnInfo.isConnected == false)  //We are not connected, try to connect
    {
        result = MQTTSNDeserialize_advertise(&mqttsnInfo.gwId, &mqttsnInfo.gwAdvDuration,
                                             pMsgData->pMsgBuf, pMsgData->msgLen);
        
        if(result)
        {
            //deserialise is successful, send the next message - Search GW
            //TO DO: Add a delay before sending this message
            Mqttsn_MsgData_t *pNextMsgData = &pIoMsgData->outgoingAck;
            InitMsgData(pNextMsgData, MSG_TX, (MQTTSN_retCodes_t)0, 0, MQTTSN_SEARCHGW, 0);
            
            if(!Mqttsn_AllocMsg(MQTTSN_BUFFER_SMALL,  &pNextMsgData->pMsgBuf, &pNextMsgData->allocLen))
            {
                printf("Error: ProcessAdvertise - could not allocate next msg buffer\n");
            }
            
            AddToTxQ(pNextMsgData);
        }
        else
        {
            printf("Error: ProcessAdvertise - could not deserialise\n");
        }
        
    }    
}

/*Serilaise and send a SearchGW message*/
static void SendSearchGW(Mqttsn_MsgData_t *pMsgData)
{
    if(mqttsnInfo.isConnected == false)
    {
        if(pMsgData->pMsgBuf)
        {
            int32_t serialisedLen;
            
            serialisedLen = MQTTSNSerialize_searchgw(pMsgData->pMsgBuf, pMsgData->allocLen, RADIUS_OF_MSG);
            
            if(serialisedLen>0)
            {
                pMsgData->msgLen = serialisedLen;
                SendMessage(pMsgData->pMsgBuf, pMsgData->msgLen);
            }
            else
            {
                printf("Error: SendSearchGW - could not serialise\n");
                configASSERT(0);
            }
            
            Mqttsn_FreeMsg(&pMsgData->pMsgBuf);
        }
        else
        {
            printf("Error: SendSearchGW - buffer not allocated for nxt msg\n");
        }
    }
}

/*Process the incoming GW Info message*/
static void ProcessGwInfo(Mqttsn_IOMsgData_t *pIoMsgData)
{
    Mqttsn_MsgData_t *pMsgData = &pIoMsgData->incomingMsg;
    
    if(mqttsnInfo.isConnected == false)
    {
        uint32_t result;
        uint8_t *pGWAddr = NULL;
        uint8_t gwId;
        
        result = MQTTSNDeserialize_gwinfo(&gwId, &mqttsnInfo.gwAddressLen,
                                          &pGWAddr, pMsgData->pMsgBuf, pMsgData->msgLen);

        //Process this message only if deserialise is successful
        //and the GW id matches the GW during advertise
        //else it could be someone else is sending this message
        if((result) && (gwId == mqttsnInfo.gwId))
        {
            Mqttsn_MsgData_t *pNextMsgData = &pIoMsgData->outgoingAck;
            
            //if greater than MAX length we support we store truncated version
            //Ideally MAX should be long enough to accomodate any type of gateway
            //addresses
            if(mqttsnInfo.gwAddressLen>MAX_GW_ADDR_LEN)
                mqttsnInfo.gwAddressLen = MAX_GW_ADDR_LEN;
            
            memcpy((void *)mqttsnInfo.gwAddress, (void const *)pGWAddr, mqttsnInfo.gwAddressLen);
            
            //Send the next message CONNECT
            InitMsgData(pNextMsgData, MSG_TX, (MQTTSN_retCodes_t)0, 0, MQTTSN_CONNECT, 0);
            if(!Mqttsn_AllocMsg(MQTTSN_BUFFER_SMALL, &pNextMsgData->pMsgBuf, &pNextMsgData->allocLen))
            {
                printf("Error: ProcessGwInfo - could not allocate next msg buffer\n");
            }
            AddToTxQ(pNextMsgData);
        }
        else
        {
            printf("Error: ProcessGwInfo - could not deserialise\n");
        }
        
    }    
}

/*Serialise and send a CONNECT message*/
static void SendConnect(Mqttsn_MsgData_t *pMsgData)
{
    if(pMsgData->pMsgBuf)   //buffer should have been allocated
    {
        int32_t serialisedLen;
        
        serialisedLen = MQTTSNSerialize_connect(pMsgData->pMsgBuf, pMsgData->allocLen, &mqttsnInfo.connData);
        
        if(serialisedLen>0)
        {
            pMsgData->msgLen = serialisedLen;
            SendMessage(pMsgData->pMsgBuf, pMsgData->msgLen);

            mqttsnInfo.pendQTimeVal = ql_lw_timer_start();
        }
        else
        {
            printf("Error: SendConnect - could not serialise\n");
            configASSERT(0);
        }
        
        Mqttsn_FreeMsg(&pMsgData->pMsgBuf);
    }
    else
    {
        printf("Error: SendConnect - buffer not allocated\n");
    }
}

static void ProcessWillTopicReq(Mqttsn_IOMsgData_t *pIoMsgData)
{
    Mqttsn_MsgData_t *pNextMsgData = &pIoMsgData->outgoingAck;
    
    //Send the WILLTOPIC
    InitMsgData(pNextMsgData, MSG_TX, (MQTTSN_retCodes_t)0, 1, MQTTSN_WILLTOPIC, 0);
    
    if(!Mqttsn_AllocMsg(MQTTSN_BUFFER_SMALL, &pNextMsgData->pMsgBuf, &pNextMsgData->allocLen))
    {
        printf("Error: ProcessWillTopicReq - could not allocate next msg buffer\n");
    }
    AddToTxQ(pNextMsgData);
}
 
static void SendWillTopic(Mqttsn_MsgData_t *pMsgData)
{
    if(pMsgData->pMsgBuf)   //buffer should have been allocated
    {
        int32_t serialisedLen;
        MQTTSNString topicString;
        
        memset((void *)&topicString, 0, sizeof(topicString));
        
        Mqttsn_TopicInfo_t *pWillTopic = Mqttsn_GetTopicInfo(TOPIC_SYS_WILL_STATUS);
        
        topicString.cstring = pWillTopic->tpcString;
        
        serialisedLen = MQTTSNSerialize_willtopic(pMsgData->pMsgBuf, pMsgData->allocLen,
                                                  pWillTopic->qosLevel, pMsgData->retain,
                                                  topicString);
        
        if(serialisedLen>0)
        {
            pMsgData->msgLen = serialisedLen;
            SendMessage(pMsgData->pMsgBuf, pMsgData->msgLen);
        }
        else
        {
            printf("Error: SendWillTopic - could not serialise\n");
            configASSERT(0);
        }
        
        Mqttsn_FreeMsg(&pMsgData->pMsgBuf);
    }
    else
    {
        printf("Error: SendWillTopic - buffer not allocated\n");
    }
}

static void ProcessWillMsgReq(Mqttsn_IOMsgData_t *pIoMsgData)
{
    Mqttsn_MsgData_t *pNextMsgData = &pIoMsgData->outgoingAck;
    
    //Send the WILLMSG
    InitMsgData(pNextMsgData, MSG_TX, (MQTTSN_retCodes_t)0, 0, MQTTSN_WILLMSG, 0);
    
    if(!Mqttsn_AllocMsg(MQTTSN_BUFFER_SMALL, &pNextMsgData->pMsgBuf, &pNextMsgData->allocLen))
    {
        printf("Error: ProcessWillMsgReq - could not allocate next msg buffer\n");
    }
    
    AddToTxQ(pNextMsgData);
}

static void SendWillMsg(Mqttsn_MsgData_t *pMsgData)
{
    if(pMsgData->pMsgBuf)   //buffer should have been allocated
    {
        int32_t serialisedLen;
        MQTTSNString topicString;
        
        memset((void *)&topicString, 0, sizeof(topicString));
        
        topicString.cstring = (char *)pWillStatusStrings[MQTTSN_WILL_STATUS_OFFLINE];
        
        Mqttsn_TopicInfo_t *pWillTopic = Mqttsn_GetTopicInfo(TOPIC_SYS_WILL_STATUS);
        
        serialisedLen = MQTTSNSerialize_willmsg(pMsgData->pMsgBuf, pMsgData->allocLen,
                                                topicString);
        
        if(serialisedLen>0)
        {
            pMsgData->msgLen = serialisedLen;
            SendMessage(pMsgData->pMsgBuf, pMsgData->msgLen);
        }
        else
        {
            printf("Error: SendWillTopic - could not serialise\n");
            configASSERT(0);
        }
        
        Mqttsn_FreeMsg(&pMsgData->pMsgBuf);
    }
    else
    {
        printf("Error: SendWillTopic - buffer not allocated\n");
    }    
}


/* Desrialise and process the CONNACK from GW*/
static void ProcessConnack(Mqttsn_IOMsgData_t *pIoMsgData)
{
    int32_t connackRet;
    uint32_t result;
    Mqttsn_MsgData_t *pMsgData = &pIoMsgData->incomingMsg;
    
    result = MQTTSNDeserialize_connack(&connackRet, pMsgData->pMsgBuf, pMsgData->msgLen);
    
    if((result) && (connackRet == MQTTSN_ACCEPTED))
    {
        mqttsnInfo.isConnected = 1;
        
        //initialize ping timer token
        mqttsnInfo.pingTimerToken = ql_lw_timer_start();
        
        //Clear any pending Connect message
        ClearPendingMsg(0, MQTTSN_CONNECT);
    }
    else
    {
        if(!result)
        {
            printf("Error: ProcessConnack - could not deserialise the msg \n");
        }
        else
        {
            printf("CONNACK return code is %d", connackRet);
        }
    }    
}

/*Deserialise and process an incoming PUBLISH.*/
static void ProcessPublish(Mqttsn_IOMsgData_t *pIoMsgData)
{
    uint32_t result = 0;
    Mqttsn_MsgData_t *pMsgData = &pIoMsgData->incomingMsg;
    Mqttsn_MsgData_t *pNextMsgData = &pIoMsgData->outgoingAck;
    int32_t qos = pMsgData->qosLevel; 
    uint8_t duplicate;
    int32_t payldLen;
    MQTTSN_topicid topicId;
    Mqttsn_TopicInfo_t *pIntTopic;
    
    result = MQTTSNDeserialize_publish(&duplicate, &qos, &pMsgData->retain, &pMsgData->pktId, &topicId,
                                       &pMsgData->pMsgPayldBuf, &payldLen, pMsgData->pMsgBuf, pMsgData->msgLen);
    
    //convert external topic to internal value
    pIntTopic = Mqttsn_GetInternalTopicInfo(topicId.data.id);
    
    pMsgData->topicId = pIntTopic->topicValue;
    
    if(qos == QOS1)
    {
        //If QoS1 was selected allocate PUBACK message
        InitMsgData(pNextMsgData, MSG_TX, (MQTTSN_retCodes_t)0, 0, MQTTSN_PUBACK, pMsgData->pktId);
        pNextMsgData->topicId = pIntTopic->topicValue;    //assign internal topic to next message
        
        if(result)
        {
            pNextMsgData->retCode = MQTTSN_ACCEPTED;
            pMsgData->payldLen = (uint32_t)payldLen;
        }
        else
        {
            pNextMsgData->retCode = MQTTSN_NOT_SUPPORTED;
        }
        
        if(!Mqttsn_AllocMsg(MQTTSN_BUFFER_SMALL, &pNextMsgData->pMsgBuf, &pNextMsgData->allocLen))
        {
            printf("ProcessPublish: Next buffer allocation failed \n");
        }
            
    }
    else
    {
        //Qos 0 or -1, nothing to do here
        //QoS 2 is not supported
        pMsgData->payldLen = (uint32_t)payldLen;
    }
}

/*Serialise and send a PUBLISH message*/
static void SendPublish(Mqttsn_MsgData_t *pMsgData)
{
    pMsgData->pktId = GetNextMsgId();
    
    if(pMsgData->pMsgBuf)
    {
        int32_t serialisedLen;
        Mqttsn_TopicInfo_t *pTopic = Mqttsn_GetTopicInfo(pMsgData->topicId);
        
        
        /*pMsgData->pBuf and pTxBuff overlap in payload portion*/
        serialisedLen = MQTTSNSerialize_publish(pMsgData->pMsgBuf, pMsgData->allocLen, 0,
                                         pMsgData->qosLevel, pMsgData->retain, pMsgData->pktId,
                                         pTopic->assgndID, pMsgData->pMsgPayldBuf,
                                         pMsgData->payldLen);
        if(serialisedLen>0)
        {
            pMsgData->msgLen = serialisedLen;
            SendMessage(pMsgData->pMsgBuf, pMsgData->msgLen);
            
            //if QoS 1 we have to retransmit if PUBACK is not received within timeout period
            //so add the message to the pending queue
            if(pMsgData->qosLevel == QOS1)
            {
                AddPendQEntry(pMsgData->pktId, pMsgData->topicId, MQTTSN_PUBLISH,
                              ql_lw_timer_start(), pMsgData->pMsgBuf, pMsgData->msgLen,
                              pMsgData->allocLen);
                mqttsnInfo.pubPending = 1;
            }
            else
            {
                //Qos 0 - no ack is expected, do not add to pending Q
                //and free the publish message here
                Mqttsn_FreeMsg(&pMsgData->pMsgBuf);
            }
        }
        else
        {
            //could not serialise, release the buffer here
            Mqttsn_FreeMsg(&pMsgData->pMsgBuf);
            printf("Error: SendPublish - could not serialise\n");
            configASSERT(0);
        }
    }
    else
    {
        //error- could not allocate buffer
        printf("Error: SendPublish - could not allocate buffer\n");
    }
}

/*API to turn on the duplicate flag for retransmissions*/
void Mqttsn_SetDupFlag(uint8_t *pBuf, uint32_t msgLen)
{
    if(msgLen <= MQTTSN_LEN_FIELD_MSG_SIZE)
    {
        *(pBuf+2) |= 0x80;
    }
    else
    {
        *(pBuf+4) |= 0x80;
    }
}

/*Deserialize and process the PUBACK pkt*/
static void ProcessPubAck(Mqttsn_IOMsgData_t *pIoMsgData)
{
    uint32_t result = 0;
    Mqttsn_MsgData_t *pMsgData = &pIoMsgData->incomingMsg;
    uint16_t topicId;
    
    result = MQTTSNDeserialize_puback(&topicId, &pMsgData->pktId,
                                      ((uint8_t *)&pMsgData->retCode), pMsgData->pMsgBuf, 
                                      pMsgData->msgLen);
    
    if(result)
    {
        if(pMsgData->retCode == MQTTSN_ACCEPTED)
        {
            Mqttsn_TopicInfo_t *pTopic = Mqttsn_GetInternalTopicInfo(topicId);
            
            //ugly fix for RSMB server returning a topic id of 0
            if(topicId == 0)
            {
                pendingMsg_t *pEntry = GetPendingMsg(pMsgData->pktId, MQTTSN_PUBLISH);
                topicId = pEntry->topicId;
                pTopic = Mqttsn_GetTopicInfo(topicId);
            }
            
            if(pTopic)
            {
                //store the topic value in msgData to be used by other modules
                pMsgData->topicId = pTopic->topicValue;
                
                //if the message was accepted clear the pending queue entry
                result = ClearPendingMsg(pMsgData->pktId, MQTTSN_SUBSCRIBE);

            }
            else
            {
                configASSERT(0);    //invalid topic ID
            }
            
            mqttsnInfo.pubPending = 0;
        }
        else
        {
            //if not accepted  - resend the PUBLISH pkt
            //TO DO: review this strategy
            pendingMsg_t *pEntry = GetPendingMsg(pMsgData->pktId, MQTTSN_PUBLISH);
            Mqttsn_SetDupFlag(pEntry->pMsgBuff, pEntry->msgLen);
            
            SendMessage(pEntry->pMsgBuff, pEntry->msgLen);
        }
    }
    else
    {
        printf("Error: ProcessPubAck - failed to desrialise\n");
    }
}

/*Serialise and send the PUBACK message*/
static void SendPubAck(Mqttsn_MsgData_t *pMsgData)
{
    if(pMsgData->pMsgBuf)
    {
        Mqttsn_TopicInfo_t *pTopic = Mqttsn_GetTopicInfo(pMsgData->topicId);
        int32_t serialisedLen;

        serialisedLen = MQTTSNSerialize_puback(pMsgData->pMsgBuf, pMsgData->allocLen, 
                                                  pTopic->assgndID.data.id, pMsgData->pktId,
                                                  pMsgData->retCode);
        if(serialisedLen>0)
        {
            pMsgData->msgLen = serialisedLen;
            SendMessage(pMsgData->pMsgBuf, pMsgData->msgLen);
        }
        else
        {
            printf("SendPubAck: could not serialise PubAck\n");
            configASSERT(0);
        }
        
        //Free the Tx Pkt here. No need to add to pending queue.
        //If the Puback is not received by the GW, 
        //the packet will be published again
        Mqttsn_FreeMsg(&pMsgData->pMsgBuf);
    }
    else
    {
        printf("SendPubAck:Msg buffer not allocated\n");
    }
}

/*Serialise and send the subscribe packet*/
static void SendSubscribe(Mqttsn_MsgData_t *pMsgData)
{
    uint16_t msgId = GetNextMsgId();
    
    if(pMsgData->pMsgBuf)
    {
        Mqttsn_TopicInfo_t *pTopic = Mqttsn_GetTopicInfo(pMsgData->topicId);
        MQTTSN_topicid subsTopic;
        int32_t serialisedLen;
        
        subsTopic.type = pTopic->assgndID.type;
        subsTopic.data.long_.name = pTopic->tpcString;
        subsTopic.data.long_.len = strlen(pTopic->tpcString);        
        
        //Start subscriptions

        serialisedLen = MQTTSNSerialize_subscribe(pMsgData->pMsgBuf, pMsgData->allocLen,
                                                     0, pMsgData->qosLevel, msgId,
                                                     &subsTopic);
        
        if(serialisedLen>0)
        {
            pMsgData->msgLen = serialisedLen;
            SendMessage(pMsgData->pMsgBuf, pMsgData->msgLen);
            
            //Init the queue entry, as we have to wait for the ack from GW
            AddPendQEntry(msgId, pMsgData->topicId, MQTTSN_SUBSCRIBE,
                          ql_lw_timer_start(), pMsgData->pMsgBuf, pMsgData->msgLen,
                          pMsgData->allocLen);
        }
        else
        {
            printf("SendSubscribe: could not serialise subscribe\n");
            Mqttsn_FreeMsg(&pMsgData->pMsgBuf);
            configASSERT(0);
        }    
    }
    else
    {
        printf("SendSubscribe: buffer not allocated\n");
    }
}

/*Desrialise and process the SUBACK message*/
static void ProcessSubAck(Mqttsn_IOMsgData_t *pIoMsgData)
{
    uint32_t result = 0;
    uint16_t topicId;   //dummy variable for the function below
    uint16_t pktId;
    uint8_t retCode;
    Mqttsn_MsgData_t *pMsgData = &pIoMsgData->incomingMsg;
    
    result = MQTTSNDeserialize_suback((int32_t *)&pMsgData->qosLevel, &topicId, 
                                      &pktId, &retCode, pMsgData->pMsgBuf,
                                      (int32_t)pMsgData->msgLen);
    
    if(result)
    {
        pMsgData->retCode = (MQTTSN_retCodes_t)retCode;
        
        if(pMsgData->retCode == MQTTSN_ACCEPTED)
        {
            pendingMsg_t *pPendingEntry = GetPendingMsg(pktId, MQTTSN_SUBSCRIBE);
            
            if(pPendingEntry)
            {
                Mqttsn_TopicInfo_t *pTopic = Mqttsn_GetTopicInfo(pPendingEntry->topicId);
                
                //store the assigned topic id
                pTopic->assgndID.data.id = topicId;
                pMsgData->topicId = pTopic->topicValue;
                
                //Clear the pending message
                result = ClearPendingMsg(pktId, MQTTSN_SUBSCRIBE);
            }
        }
        else
        {
            //resend with duplicate set
            //TO DO: review this strategy
            pendingMsg_t *pEntry = GetPendingMsg(pktId, MQTTSN_SUBSCRIBE);
            Mqttsn_SetDupFlag(pEntry->pMsgBuff, pEntry->msgLen);
            
            SendMessage(pEntry->pMsgBuff, pEntry->msgLen);
            pEntry->txTimerToken = ql_lw_timer_start();
        }
    }
    else
    {
        printf("Error: ProcessSubAck - got a msg with non- existant seq #\n");
    }
}

/*Serialise and send the register message*/
static void SendRegister(Mqttsn_MsgData_t *pMsgData)
{
    MQTTSNString topicStr;
    uint16_t msgId = GetNextMsgId();
    
    if(pMsgData->pMsgBuf)
    {
        Mqttsn_TopicInfo_t *pTopic = Mqttsn_GetTopicInfo(pMsgData->topicId);
        int32_t serialisedLen;
        
        topicStr.cstring = pTopic->tpcString;
        
        serialisedLen = MQTTSNSerialize_register(pMsgData->pMsgBuf, pMsgData->allocLen,
                                                    0, msgId, 
                                                    &topicStr);
        if(serialisedLen>0)
        {
            pMsgData->msgLen = serialisedLen;
            SendMessage(pMsgData->pMsgBuf, pMsgData->msgLen);
            
            //Init the queue entry, as we have to wait for the ack from GW
            AddPendQEntry(msgId, pMsgData->topicId, MQTTSN_REGISTER,
                          ql_lw_timer_start(), pMsgData->pMsgBuf, pMsgData->msgLen,
                          pMsgData->allocLen);
        }
        else
        {
            printf("SendRegister: could not serialise register\n");
            Mqttsn_FreeMsg(&pMsgData->pMsgBuf);
            configASSERT(0);            
        }    
    }
    else
    {
        printf("SendRegister: buffer not allocated\n");
    }
}

/*Desrialise and process the REGACK message*/
static void ProcessRegAck(Mqttsn_IOMsgData_t *pIoMsgData)
{
    uint32_t result = 0;
    uint16_t pktId;
    uint8_t retCode;
    Mqttsn_MsgData_t *pMsgData = &pIoMsgData->incomingMsg;
    
    result = MQTTSNDeserialize_regack(&pMsgData->topicId, &pktId, &retCode,
                                        pMsgData->pMsgBuf, (int32_t)pMsgData->msgLen);
    
    if(result)
    {
        pMsgData->retCode = (MQTTSN_retCodes_t)retCode;
        
        if(pMsgData->retCode == MQTTSN_ACCEPTED)
        {
            pendingMsg_t *pPendingEntry = GetPendingMsg(pktId, MQTTSN_REGISTER);
            
            if(pPendingEntry)
            {
                Mqttsn_TopicInfo_t *pTopic = Mqttsn_GetTopicInfo(pPendingEntry->topicId);
                
                //store the assigned topic id
                pTopic->assgndID.data.id = pMsgData->topicId;
                pMsgData->topicId = pPendingEntry->topicId;
                result = ClearPendingMsg(pktId, MQTTSN_REGISTER);
            }
        }
        else
        {
            //resend with duplicate set
            //TO DO: review this strategy
            pendingMsg_t *pEntry = GetPendingMsg(pktId, MQTTSN_PUBLISH);
            Mqttsn_SetDupFlag(pEntry->pMsgBuff, pEntry->msgLen);
            
            SendMessage(pEntry->pMsgBuff, pEntry->msgLen);
            pEntry->txTimerToken = ql_lw_timer_start();
        }
    }
    else
    {
        printf("Error: ProcessRegAck - got a msg with non- existant seq #\n");
    }
}

/*Send an unsubscribe message*/
static void SendUnSubscribe(Mqttsn_MsgData_t *pMsgData)
{
    if(pMsgData->pMsgBuf)
    {
        int32_t serialisedLen;
        pMsgData->pktId = GetNextMsgId();
        
        Mqttsn_TopicInfo_t *pTopic = Mqttsn_GetTopicInfo(pMsgData->topicId);
        
        serialisedLen = MQTTSNSerialize_unsubscribe(pMsgData->pMsgBuf, pMsgData->allocLen, pMsgData->pktId, &pTopic->assgndID);
        
        if(serialisedLen>0)
        {
            pMsgData->msgLen = serialisedLen;
            SendMessage(pMsgData->pMsgBuf, pMsgData->msgLen);
            
            AddPendQEntry(pMsgData->pktId, pMsgData->topicId, MQTTSN_UNSUBSCRIBE,
                          ql_lw_timer_start(), pMsgData->pMsgBuf, pMsgData->msgLen,
                          pMsgData->allocLen);
        }
        else
        {
            printf("Error: ProcessUnSubAck - could not serialise \n");
            Mqttsn_FreeMsg(&pMsgData->pMsgBuf);
            configASSERT(0);            
        }
    }
    else
    {
        printf("Error: ProcessUnSubAck - could not allocate buffer\n");
    }
}

/*Process an UNSUBACK*/
static void ProcessUnSubAck(Mqttsn_IOMsgData_t *pIoMsgData)
{
    uint32_t result = 0;
    uint16_t pktId;
    Mqttsn_MsgData_t *pMsgData = &pIoMsgData->incomingMsg;
    
    result = MQTTSNDeserialize_unsuback(&pktId, pMsgData->pMsgBuf, pMsgData->msgLen);
    //FreeBuffer(MQTTSN_UNSUBACK, 0, &pMsgData->pMsgBuf);
    
    if(result)
    {
        pendingMsg_t *pPendingEntry = GetPendingMsg(pktId, MQTTSN_UNSUBSCRIBE);
        
        if(pPendingEntry)
        {
            pMsgData->topicId = pPendingEntry->topicId;
            
            ClearPendingMsg(pktId, MQTTSN_UNSUBSCRIBE);
        }
    }
    else
    {
        //resend
        pendingMsg_t *pEntry = GetPendingMsg(pktId, MQTTSN_UNSUBSCRIBE);
        Mqttsn_SetDupFlag(pEntry->pMsgBuff, pEntry->msgLen);
        
        SendMessage(pEntry->pMsgBuff, pEntry->msgLen);
    }
}

/*Process an incoming ping request*/
static void ProcessPingReq(Mqttsn_IOMsgData_t *pIoMsgData)
{
    //Send a ping response
    Mqttsn_MsgData_t *pNextMsgData = &pIoMsgData->outgoingAck;
    
    InitMsgData(pNextMsgData, MSG_TX, (MQTTSN_retCodes_t)0, 0, MQTTSN_PINGRESP, 0);

    if(!Mqttsn_AllocMsg(MQTTSN_BUFFER_SMALL, &pNextMsgData->pMsgBuf, &pNextMsgData->allocLen))
    {
        printf("Error: ProcessPingReq-could not allocate buffer for next msg\n");
    }

    AddToTxQ(pNextMsgData);

}

/*Serialise and send a ping request*/
static void SendPingReq(Mqttsn_MsgData_t *pMsgData)
{
    if(pMsgData->pMsgBuf)
    {
        int32_t serialisedLen;
        
        serialisedLen = MQTTSNSerialize_pingreq(pMsgData->pMsgBuf, pMsgData->allocLen, mqttsnInfo.connData.clientID);
        
        if(serialisedLen>0)
        {
            pMsgData->msgLen = serialisedLen;
            //*pMsgData->pMsgBuf = 2;
            SendMessage(pMsgData->pMsgBuf, pMsgData->msgLen);
            AddPendQEntry(0, 0, MQTTSN_PINGREQ, ql_lw_timer_start(), pMsgData->pMsgBuf, pMsgData->msgLen,
                          pMsgData->allocLen);
        }
        else
        {
            printf("Error: SendPingReq - could not serialise\n");
            Mqttsn_FreeMsg(&pMsgData->pMsgBuf);
            configASSERT(0);
        }
    }
    else
    {
        printf("Error: SendPingReq - buffer not allocated\n");
    }
}

/*Process an incoming ping response*/
static void ProcessPingResp(Mqttsn_IOMsgData_t *pIoMsgData)
{
    Mqttsn_MsgData_t *pMsgData = &pIoMsgData->incomingMsg;
    uint32_t result = MQTTSNDeserialize_pingresp(pMsgData->pMsgBuf, pMsgData->msgLen);
    
    if(result)
    {
        ClearPendingMsg(0, MQTTSN_PINGREQ);
    }
}

/*Send an ping response*/
static void SendPingResp(Mqttsn_MsgData_t *pMsgData)
{
    if(pMsgData->pMsgBuf)
    {
        int32_t serialisedLen;
        
        serialisedLen = MQTTSNSerialize_pingresp(pMsgData->pMsgBuf, pMsgData->allocLen);
        
        if(serialisedLen>0)
        {
            pMsgData->msgLen = serialisedLen;
            SendMessage(pMsgData->pMsgBuf, pMsgData->msgLen);
            Mqttsn_FreeMsg(&pMsgData->pMsgBuf);
        }
        else
        {
            printf("Error: SendPingResp - could not serialise\n");
            Mqttsn_FreeMsg(&pMsgData->pMsgBuf);
            configASSERT(0);
        }
    }
    else
    {
        //could not allocate buffer
        printf("Error: SendPingResp -buffer not allocated\n");
    }
}

/*Process a disconnect message*/
static void ProcessDisconnect(Mqttsn_IOMsgData_t *pIoMsgData)
{
    int32_t duration, result;
    uint32_t index;
    Mqttsn_MsgData_t *pMsgData = &pIoMsgData->incomingMsg;
    
    mqttsnInfo.nextMsgId = 1;
    mqttsnInfo.isConnected = false;  
    
    result = MQTTSNDeserialize_disconnect(&duration, pMsgData->pMsgBuf, (int32_t)pMsgData->msgLen);
    //To Do: Act on duration? reconnect after duration?
    
    if(result)
    {
        printf("Received disconnect with duration %d\n", duration);
        
        //Invalidate all the pending entries
        for(index = 0; index< MAX_PENDING_MSGS; index++)
        {
            pendingMsg_t *pEntry = &mqttsnInfo.pendingMsgQueue[index];
            pEntry->isValid = 0;
        }
    }
    else
    {
        printf("Error: ProcessDisconnect - failed to desrialise\n");
    }
}

/*Send a disconnect*/
static void SendDisconnect(Mqttsn_MsgData_t *pMsgData)
{
    uint32_t result = 0;
    mqttsnInfo.nextMsgId = 1;
    mqttsnInfo.isConnected = false;  
    
    ResetPendingQ();
    
    result = Mqttsn_AllocMsg(MQTTSN_BUFFER_SMALL, &pMsgData->pMsgBuf, &pMsgData->allocLen);
    
    if(result)
    {
        int32_t serialisedLen;
        
        serialisedLen = MQTTSNSerialize_disconnect(pMsgData->pMsgBuf, pMsgData->allocLen, -1);
        
        if(serialisedLen>0)
        {
            uint32_t index;
            
            pMsgData->msgLen = serialisedLen;
            SendMessage(pMsgData->pMsgBuf, pMsgData->msgLen);
            Mqttsn_FreeMsg(&pMsgData->pMsgBuf);
            
            //Invalidate all the pending entries
            for(index = 0; index< MAX_PENDING_MSGS; index++)
            {
                pendingMsg_t *pEntry = &mqttsnInfo.pendingMsgQueue[index];
                pEntry->isValid = 0;
            }
            
        }
        else
        {
            printf("Error: SendDisconnect - could not serialise\n");
            Mqttsn_FreeMsg(&pMsgData->pMsgBuf);
            configASSERT(0);            
        }
    }
    else
    {
        printf("Error:SendDisconnect - could not allocate buffer\n");
    } 
}


/*Periodically check the pending messages to see if timeout has happened. In which
case we have to retransmit the packet. If the number of retries exceeds a set max
we send a disconnect message to the GW as mentioned in the spec*/
static void CheckPendingMsgs()
{
    
    if( ql_lw_timer_is_expired( mqttsnInfo.pendQTimeVal, PENDING_Q_CHECK_INTERVAL ))
    {
        uint32_t index;
        mqttsnInfo.pendQTimeVal = ql_lw_timer_start();
        
        for(index = 0; index < MAX_PENDING_MSGS; index++)
        {
            //Check each pending entry
            if(ql_lw_timer_is_expired( mqttsnInfo.pendingMsgQueue[index].txTimerToken, MQTTSN_RETRY_PERIOD ))
            {
                pendingMsg_t *pEntry = &mqttsnInfo.pendingMsgQueue[index];
                
                if(pEntry->isValid == true)
                {
                    if(pEntry->numTries < MQTTSN_MAX_RETRIES)
                    {
                        //set duplicate flag only for publishes
                        if((pEntry->currMsg == MQTTSN_PUBLISH) ||
                           (pEntry->currMsg == MQTTSN_SUBSCRIBE))
                        {
                            Mqttsn_SetDupFlag(pEntry->pMsgBuff, pEntry->msgLen);
                        }
                        
                        SendMessage(pEntry->pMsgBuff, pEntry->msgLen);
                        
                        //increment the retry count
                        pEntry->numTries++;
                        pEntry->txTimerToken = ql_lw_timer_start();
                    }
                    else
                    {
                        //disconnect
                        Mqttsn_MsgData_t nextMsgData;
                        
                        if(mqttsnInfo.isConnected)
                        {
                            InitMsgData(&nextMsgData, MSG_TX, (MQTTSN_retCodes_t)0, 0, MQTTSN_DISCONNECT, 0);
                            
                            if(xQueueSend(MQTTSN_TxMsgQ, ( void * ) &nextMsgData, MQTTSN_MSGQ_SEND_WAIT_TICKS))
                            {
                                printf("MQTTSN Disconnecting, Max reties exceeded. Pending msg type: 0x%x \n", pEntry->currMsg);
                            }
                            HandleDisconnect();
                        }
                    }
                }
            }
        }
    }
    
    if((mqttsnInfo.isConnected)&&
       (ql_lw_timer_is_expired( mqttsnInfo.pingTimerToken, KEEP_ALIVE_TIMER*1000 )))
    {
        Mqttsn_MsgData_t pingMsg;
        
        InitMsgData(&pingMsg, MSG_TX, (MQTTSN_retCodes_t)0, 0, MQTTSN_PINGREQ, 0);
        
        if(!Mqttsn_AllocMsg(MQTTSN_BUFFER_SMALL, &pingMsg.pMsgBuf, &pingMsg.allocLen))
        {
            printf("Error: ProcessPingReq-could not allocate buffer for next msg\n");
        }
        
        AddToTxQ(&pingMsg);
    }
}

/* Message dispatch table for each MQTTSN message */
static const struct MessageDispatch Messages[] = {
    {.txHandler = NULL, .rxHandler = ProcessAdvertise},  /*MQTTSN_ADVERTISE*/ 
    {.txHandler = SendSearchGW, .rxHandler = NULL},      /*MQTTSN_SEARCHGW*/
    {.txHandler = NULL, .rxHandler = ProcessGwInfo},     /*MQTTSN_GWINFO*/
    {.txHandler = NULL, .rxHandler = NULL},              /*MQTTSN_RESERVED1*/
    {.txHandler = SendConnect, .rxHandler = NULL},       /*MQTTSN_CONNECT*/
    {.txHandler = NULL, .rxHandler = ProcessConnack},    /*MQTTSN_CONNACK*/ 
    {.txHandler = NULL, .rxHandler = ProcessWillTopicReq},      /*MQTTSN_WILLTOPICREQ*/
    {.txHandler = SendWillTopic, .rxHandler = NULL},      /*MQTTSN_WILLTOPIC*/
    {.txHandler = NULL, .rxHandler = ProcessWillMsgReq},      /*MQTTSN_WILLMSGREQ*/
    {.txHandler = SendWillMsg, .rxHandler = NULL},      /*MQTTSN_WILLMSG*/
    {.txHandler = SendRegister, .rxHandler = NULL},      /*MQTTSN_REGISTER*/
    {.txHandler = NULL, .rxHandler = ProcessRegAck},      /*MQTTSN_REGACK*/ 
    {.txHandler = SendPublish, .rxHandler = ProcessPublish},    /*MQTTSN_PUBLISH*/            
    {.txHandler = SendPubAck, .rxHandler = ProcessPubAck},     /*MQTTSN_PUBACK */
    {.txHandler = NULL, .rxHandler = NULL},    /*MQTTSN_PUBCOMP */ 
    {.txHandler = NULL, .rxHandler = NULL},     /*MQTTSN_PUBREC */  
    {.txHandler = NULL, .rxHandler = NULL},     /*MQTTSN_PUBREL */ 
    {.txHandler = NULL, .rxHandler = NULL},      /*MQTTSN_RESERVED2 */ 
    {.txHandler = SendSubscribe, .rxHandler = NULL},     /*MQTTSN_SUBSCRIBE */ 
    {.txHandler = NULL, .rxHandler = ProcessSubAck},     /*MQTTSN_SUBACK */ 
    {.txHandler = SendUnSubscribe, .rxHandler = NULL},   /*MQTTSN_UNSUBSCRIBE */ 
    {.txHandler = NULL, .rxHandler = ProcessUnSubAck},   /*MQTTSN_UNSUBACK */ 
    {.txHandler = SendPingReq, .rxHandler = ProcessPingReq},    /*MQTTSN_PINGREQ */ 
    {.txHandler = SendPingResp, .rxHandler = ProcessPingResp},   /*MQTTSN_PINGRESP */ 
    {.txHandler = SendDisconnect, .rxHandler = ProcessDisconnect}, /*MQTTSN_DISCONNECT */ 
    {.txHandler = NULL, .rxHandler = NULL},      /*MQTTSN_RESERVED3 */ 
    {.txHandler = NULL, .rxHandler = NULL},      /*MQTTSN_WILLTOPICUPD */ 
    {.txHandler = NULL, .rxHandler = NULL},      /*MQTTSN_WILLTOPICRESP */ 
    {.txHandler = NULL, .rxHandler = NULL},      /*MQTTSN_WILLMSGUPD */ 
    {.txHandler = NULL, .rxHandler = NULL}       /*MQTTSN_WILLMSGRESP */
};

/*Initialize any global MQTTSN task variables*/
static void mqttsn_init(void)
{
    uint32_t index;
    
    memset((void *)&mqttsnInfo, 0, sizeof(mqttsnInfo_t)); 
    mqttsnInfo.connData = (MQTTSNPacket_connectData)MQTTSNPacket_connectData_initializer;
    mqttsnInfo.connData.clientID.cstring = "QL_S3";
    mqttsnInfo.connData.duration = KEEP_ALIVE_TIMER;
    mqttsnInfo.connData.cleansession = 0x1; //Always clean session
    mqttsnInfo.connData.willFlag = 0x1;
    
    //start with 1 instead of 0
    mqttsnInfo.nextMsgId = 1;
    
    for(index = 0; index<NUM_BUFFERS_SMALL; index++)
    {
        uint32_t *pBuff = (uint32_t *)&mqttsnBuff[index][0];
        if( xQueueSend( MQTTSN_FreeBuffQSm, &pBuff, ( TickType_t ) MQTTSN_MSGQ_SEND_WAIT_TICKS) != pdPASS )
        {
            /* Failed to post the message, even after 10 ticks. */
        }
    }
    
    for(index = 0; index<NUM_BUFFERS_LARGE; index++)
    { 
        uint32_t *pBuff = (uint32_t *)&mqttsnBuffLrg[index][0];
        if( xQueueSend( MQTTSN_FreeBuffQLrg, &pBuff, ( TickType_t ) MQTTSN_MSGQ_SEND_WAIT_TICKS ) != pdPASS )
        {
            /* Failed to post the message, even after 10 ticks. */
        }
    }
}


/* Add an 8bit value to the buffer */
uint32_t Mqttsn_BuffWr_u8( uint8_t **pBuf, uint8_t v )
{
    **pBuf = (uint8_t)v;
    (*pBuf) +=1;
    
    return 1;
}

/* Add an 16bit value to the buffer */
uint32_t Mqttsn_BuffWr_u16( uint8_t **pBuf, uint16_t v )
{
    uint32_t totalLen = 0;
    
    totalLen += Mqttsn_BuffWr_u8(pBuf,  v >> 8 );
    totalLen += Mqttsn_BuffWr_u8(pBuf,  v >> 0 );
    
    return totalLen;
}
  
/* Add an 32bit value to the buffer */
uint32_t Mqttsn_BuffWr_u32(uint8_t **pBuf, uint32_t v )
{
    uint32_t totalLen = 0;
    
    totalLen += Mqttsn_BuffWr_u16(pBuf, v >> 16 );
    totalLen += Mqttsn_BuffWr_u16(pBuf, v >>  0 );
    
    return totalLen;    
}

/* Add an 32bit value to the buffer */
uint32_t Mqttsn_BuffWr_u64(uint8_t **pBuf, uint64_t v )
{
    uint32_t totalLen = 0;
    
    totalLen += Mqttsn_BuffWr_u32(pBuf, v >> 32 );
    totalLen += Mqttsn_BuffWr_u32(pBuf, v >>  0 );
    
    return totalLen;    
}

/*Read an 8 bit value from the buffer*/
uint8_t Mqttsn_BuffRead_u8(uint8_t **pBuff)
{
    uint8_t value;
    
    value = **pBuff;
    
    (*pBuff) +=1;
    
    return value;
}

/*Read a 16 bit value from the buffer*/
uint16_t Mqttsn_BuffRead_u16(uint8_t **pBuff)
{
    uint16_t value;
    
    value = (Mqttsn_BuffRead_u8(pBuff)<<8);
    value |= (Mqttsn_BuffRead_u8(pBuff)<<0);
    
    return value;
}

/*Read a 32 bit value from the buffer*/
uint32_t Mqttsn_BuffRead_u32(uint8_t **pBuff)
{
    uint32_t value;
    
    value = (Mqttsn_BuffRead_u8(pBuff)<<24);
    value |= (Mqttsn_BuffRead_u8(pBuff)<<16);
    value |= (Mqttsn_BuffRead_u8(pBuff)<<8);
    value |= (Mqttsn_BuffRead_u8(pBuff)<<0);
    
    return value;
}


/*API to add the reply to an incoming message to an outgoing queue*/
uint32_t Mqttsn_SendReply(Mqttsn_MsgData_t *pMsgData)
{
    uint32_t result = 0;
    
    if( xQueueSend( MQTTSN_TxMsgQ, pMsgData, ( TickType_t )MQTTSN_MSGQ_SEND_WAIT_TICKS ) != pdPASS )
    {
        printf("Error Mqttsn_SendReply:Failed to add to Tx Q\n");
    } 
    
    return result;
}

uint32_t GetPayloadOffset(uint32_t msgLength)
{
    configASSERT(msgLength <= MQTTSN_BUFFER_LARGE);

    if(msgLength < MQTTSN_LEN_FIELD_MSG_SIZE)
        return MSG_PUBLISH_PAYLD_OFFSET_LT256;
    else
        return MSG_PUBLISH_PAYLD_OFFSET_GT256;
}

void Mqttsn_PublishWillStatus(Mqttsn_WillStatus_t willStatus)
{
    Mqttsn_TopicInfo_t *pTpcInfo;    
    Mqttsn_MsgData_t msgData;
    
    msgData.topicId = TOPIC_SYS_WILL_STATUS;
    
    pTpcInfo = Mqttsn_GetTopicInfo(msgData.topicId);
    
    if((pTpcInfo)&&(pTpcInfo->status)&&(mqttsnInfo.isConnected))
    {
        Mqttsn_TxPublishErrorCodes_t publishErr;
        uint32_t result;
        
        result = Mqttsn_AllocPublishPayload(MQTTSN_BUFFER_SMALL, &msgData.pMsgPayldBuf,
                                            &msgData.allocLen);
        if(result)
        {
            memcpy((void *)msgData.pMsgPayldBuf, pWillStatusStrings[willStatus], 
                   strlen(pWillStatusStrings[willStatus]));

            msgData.payldLen = strlen(pWillStatusStrings[willStatus]);
            
            publishErr = Mqttsn_SendPublish(&msgData, pTpcInfo);
        }
        
    }
}

       
/*Adds the populated PUBLISH message to the outgoing message queue*/
Mqttsn_TxPublishErrorCodes_t Mqttsn_SendPublish(Mqttsn_MsgData_t *pMsgData, Mqttsn_TopicInfo_t *pTopicInfo)
{
    Mqttsn_TxPublishErrorCodes_t result = MQTTSN_TX_PUBLISH_NO_ERROR;

    //Send the PUBLISH packet only if 1. We are connected
    //2. Payload buffer has been allocated - all PUBLISH messages from device have payloads
    //3. The topic has been registered and accepted by the GW
    if((mqttsnInfo.isConnected) && (pMsgData->pMsgPayldBuf) && (pTopicInfo->status))
    {
        uint32_t payloadOffset = 0;

        payloadOffset = GetPayloadOffset(pMsgData->allocLen);

        //Get the msg buffer pointer/length based on the payload pointer/length
        pMsgData->pMsgBuf = pMsgData->pMsgPayldBuf-payloadOffset;
        pMsgData->allocLen = pMsgData->allocLen+payloadOffset;

        //Initialise topic info
        pMsgData->qosLevel = pTopicInfo->qosLevel;

        //Initialise the msg data
        InitMsgData(pMsgData, MSG_TX, (MQTTSN_retCodes_t)0, pTopicInfo->retainFlag, MQTTSN_PUBLISH, 0);

        //Add it to the Tx Queue
        if(xQueueSend( MQTTSN_TxMsgQ, pMsgData, ( TickType_t ) 10) != pdPASS )
        {
            /* Failed to post the message, even after 10 ticks. */
            result = MQTTSN_TX_PUBLISH_TX_QUEUE_FULL;
            printf("Error Mqttsn_SendPublish:Failed to add to Tx Q\n");
        } 
    }
    else
    {
        //return the error
        if(mqttsnInfo.isConnected == 0)
        {
            result = MQTTSN_TX_PUBLISH_NOT_CONNECTED;
        }
        else if(pTopicInfo->status == 0)
        {
            result = MQTTSN_TX_PUBLISH_TOPIC_NOT_SUBSCRIBED;
        }
        else
        {
            result = MQTTSN_TX_PUBLISH_PAYLD_BUFFER_NOT_ALLOCED;
        }
    }

    if(result != MQTTSN_TX_PUBLISH_NO_ERROR)
    {
        //release the buffer
        Mqttsn_FreeMsg(&pMsgData->pMsgPayldBuf);
    }

    return result;
}

/*Allocates a message payload as requested by any outside task*/
uint32_t Mqttsn_AllocPublishPayload(Mqttsn_BuffSz_t buffSz, uint8_t **pBuf, uint32_t *pBufLen)
{
    uint32_t result;
    uint32_t payloadOffset = 0;
    
    //Get the message pointer
    result = Mqttsn_AllocMsg(buffSz, pBuf, pBufLen);
    
    if(result)
    {
        payloadOffset = GetPayloadOffset(*pBufLen);
        
        //adjust the pointers to point to the payload
        *pBuf = *pBuf + payloadOffset;
        *pBufLen = *pBufLen-payloadOffset;
    }
    
    return result;
    
}

/*Allocate a message buffer based on the input length requested*/
uint32_t Mqttsn_AllocMsg(Mqttsn_BuffSz_t buffSz, uint8_t **pBuf, uint32_t *pBufLen)
{
    uint32_t result = 0;
    *pBufLen = 0;
    
    //depending on the buffer size get the buffer pointer from the appropriate queue
    if(buffSz == MQTTSN_BUFFER_LARGE)
    {
        if(xQueueReceive( MQTTSN_FreeBuffQLrg, pBuf, ( TickType_t ) 100 ))
        {
            result = 1;
            *pBufLen = MQTTSN_BUFFER_LARGE;
        }
    }
    else
    {
        if(xQueueReceive( MQTTSN_FreeBuffQSm, pBuf, ( TickType_t ) 100 )) 
        {
            result = 1;
            *pBufLen = MQTTSN_BUFFER_SMALL;
        }
    }
    
    //set the buffer to zeros
    if(*pBuf)
    {
        memset((void *)*pBuf, 0, *pBufLen);
    }

    //assert if we ran out of buffers    
    configASSERT(result);

    
    return result;
}

void Mqttsn_FreePublishPayload(uint8_t **pBuf, uint32_t allocLen)
{
    uint32_t payloadOffset = GetPayloadOffset(allocLen);
    
    //adjust the pointers to point to the header
    *pBuf = *pBuf - payloadOffset;
    
    Mqttsn_FreeMsg(pBuf);
    
}

/*Free the allocated message buffer */
uint32_t Mqttsn_FreeMsg(uint8_t **pBuf)
{
    uint32_t result = 1;
    
    configASSERT(pBuf);
    
    if(*pBuf != NULL)
    {
        if((*pBuf >= MQTTSN_LARGE_BUFF_START_ADDR) && (*pBuf < MQTTSN_LARGE_BUFF_END_ADDR))
        { 
            if( xQueueSend( MQTTSN_FreeBuffQLrg, pBuf, ( TickType_t ) 10 ) != pdPASS )
            {
                /* Failed to post the message, even after 10 ticks. */
                result = 0;
            }
        }
        else if((*pBuf >= MQTTSN_SMALL_BUFF_START_ADDR) && (*pBuf < MQTTSN_SMALL_BUFF_END_ADDR))
        {
            if( xQueueSend( MQTTSN_FreeBuffQSm, pBuf, ( TickType_t ) 10 ) != pdPASS )
            {
                /* Failed to post the message, even after 10 ticks. */
                result = 0;
            } 
        }
        else
        {
            //assert here as we have not found the pointer in the memory pools
            configASSERT(0);
        }
    }
    
    return result;
}

/*Reads the incoming message, allocates the message buffer and copies the message*/
uint32_t Mqttsn_CopyMsg(Mqttsn_MsgData_t *pMsgData, uint8_t *pBuf, uint32_t numBytes)
{
    uint32_t result = 0;
    
    pMsgData->msgDirec = MSG_RX;
    pMsgData->msgType = (enum MQTTSN_msgTypes)MQTTSNPacket_read_nb(pBuf, numBytes);
    
    //allocate the message buffer
    result = Mqttsn_AllocMsg(numBytes>MQTTSN_BUFFER_SMALL?MQTTSN_BUFFER_LARGE:MQTTSN_BUFFER_SMALL,
                       &pMsgData->pMsgBuf, &pMsgData->allocLen);
    
    if(result)
    {
        //allocation successful, copy the packet
        pMsgData->msgLen = numBytes;
        
        memcpy((void *)pMsgData->pMsgBuf, pBuf, numBytes);
    }
    
    return result;
}

/*MQTTSN subscribe API for pre-defined topics*/
uint32_t Mqttsn_Subscribe(Mqttsn_TopicInfo_t *pTopicInfo)
{
    Mqttsn_MsgData_t msgData;
    uint32_t result = 0;
    
    msgData.msgDirec = MSG_TX;
    msgData.qosLevel = pTopicInfo->qosLevel;
    msgData.msgType = MQTTSN_SUBSCRIBE;
    
    //copy the requested topic info
    msgData.topicId = pTopicInfo->topicValue;
    
    //allocate the message buffer
    result = Mqttsn_AllocMsg(MQTTSN_BUFFER_SMALL, &msgData.pMsgBuf, &msgData.allocLen);
    
    if(result)
    {
        //Add it to the Tx Queue
        if(xQueueSend( MQTTSN_TxMsgQ, &msgData, ( TickType_t ) MQTTSN_MSGQ_SEND_WAIT_TICKS ) != pdPASS )
        {
            /* Failed to post the message, even after 10 ticks. */
            result = 0;
        } 
    }
    else
    {
        printf("Could not allocate buffer for subscribe\n");
    }
    return result;
}

uint32_t Mqttsn_Register(Mqttsn_TopicInfo_t *pTopicInfo)
{
    Mqttsn_MsgData_t msgData;
    uint32_t result = 0;
    
    msgData.msgDirec = MSG_TX;
    msgData.qosLevel = pTopicInfo->qosLevel;
    msgData.msgType = MQTTSN_REGISTER;
    
    //copy the requested topic info
    msgData.topicId = pTopicInfo->topicValue;
    
    //allocate the message buffer
    result = Mqttsn_AllocMsg(MQTTSN_BUFFER_SMALL, &msgData.pMsgBuf, &msgData.allocLen);
    
    if(result)
    {
        //Add it to the Tx Queue
        if(xQueueSend( MQTTSN_TxMsgQ, &msgData, ( TickType_t ) 10 ) != pdPASS )
        {
            /* Failed to post the message, even after 10 ticks. */
            result = 0;
        } 
    }
    else
    {
        printf("Could not allocate buffer for register\n");
    }
    return result;
}


void SendMessage(uint8_t *pBuf, uint32_t bufLen)
{
    
#if MQTTSN_MSGHANDLER_TRACES        
    if(pBuf[4] != 0x10)
        CLI_hexdump( 0, pBuf, bufLen);
#endif //MQTTSN_MSGHANDLER_TRACES 
    
    mqttsn_comm_tx(pBuf, bufLen);
    
    //reinitialize ping timer token
    mqttsnInfo.pingTimerToken = ql_lw_timer_start();
    
}

/*Processes the incoming message - parses the data and calls the appropriate handler*/
uint32_t Mqttsn_ProcessIncmgMsg(Mqttsn_IOMsgData_t *pIoMsgData, MqttsnPdu_t *pMqttsnPdu)
{
    uint32_t result;
    Mqttsn_MsgData_t *pInMsg = &pIoMsgData->incomingMsg;
    
    pInMsg->msgDirec = MSG_RX;
    pInMsg->pMsgBuf = pMqttsnPdu->pBuf;
    pInMsg->msgLen = pMqttsnPdu->numBytes;
    pInMsg->allocLen = pMqttsnPdu->allocLen;
    
    pInMsg->msgType = (enum MQTTSN_msgTypes)MQTTSNPacket_read_nb(pInMsg->pMsgBuf,
                                                                 pInMsg->msgLen);

    if((pInMsg->msgType<=MQTTSN_MESSAGE_TYPES_MAX)
        &&(Messages[pInMsg->msgType].rxHandler))
    {
        Messages[pInMsg->msgType].rxHandler(pIoMsgData);
        result = 1;
    }
    else
    {
        result = 0;
        //printf("MQTTSN: Mqttsn_ProcessIncmgMsg - Unknown message\n");
    }
    
   return result;
}

/*Send a connection request*/
uint32_t Mqttsn_SendConnectionRequest(void)
{
    Mqttsn_MsgData_t msgData;
    uint32_t retCode = 0;
#if (USB_UART_CHECK_FIFO_CONNECT == 1)
    // Check if there is space in COMM tx buffer
    if (!mqttsn_comm_tx_is_fifo_empty()) {
      // FIFO is not empty, do not send CONNECT request
      return retCode;
    }
#endif
    //Send CONNECT
    InitMsgData(&msgData, MSG_TX, (MQTTSN_retCodes_t)0, 0, MQTTSN_CONNECT, 0);

    if(!Mqttsn_AllocMsg(MQTTSN_BUFFER_SMALL, &msgData.pMsgBuf, &msgData.allocLen))
    {
        printf("Error: Mqttsn_SendConnectionRequest - could not allocate msg buffer\n");
        retCode = 1;
    }
    AddToTxQ(&msgData);
    
    return retCode;
}

/*MQTTSN Task handler - Checks for pending messages
and process the TX queue to transmit a packet */
void mqttsnTaskHandler(void *pParameter)
{
    Mqttsn_MsgData_t mqttsnMsgData;
    
    wait_ffe_fpga_load();
    
    mqttsn_init();
    
    //clear the Msg Q buffer 
    memset(&mqttsnMsgData, 0, sizeof(mqttsnMsgData));

    while(1)
    {
        CheckPendingMsgs();
        
        //Peek at the head of the queue and check the msg type
        if(xQueuePeek( MQTTSN_TxMsgQ, &( mqttsnMsgData ), ( TickType_t ) PENDING_Q_CHECK_INTERVAL ) )
        {
            //if the message type is PUBLISH and we have another pending
            //we cannot transmit it yet.
            if(((mqttsnMsgData.msgType == MQTTSN_PUBLISH)&&(mqttsnInfo.pubPending == 0))||
                (mqttsnMsgData.msgType != MQTTSN_PUBLISH))
            {
                if(xQueueReceive(MQTTSN_TxMsgQ, &( mqttsnMsgData ), ( TickType_t ) PENDING_Q_CHECK_INTERVAL))
                {
                    if(Messages[mqttsnMsgData.msgType].txHandler)
                    {
                        Messages[mqttsnMsgData.msgType].txHandler(&mqttsnMsgData);
                    }
                    else
                    {
                        printf("MQTTSN: Unknown message\n");
                    }
                }
            }
        }
    } 
}

/* Setup msg queue and Task Handler for MQTTSN Task */
signed portBASE_TYPE StartRtosTaskMqttsnMsgHandler( void)
{
    static uint8_t ucParameterToPass;

    /* Create queue for Tx messages which are ready */
    MQTTSN_TxMsgQ = xQueueCreate( MQTTSN_QUEUE_LENGTH, sizeof(Mqttsn_MsgData_t) );
    vQueueAddToRegistry( MQTTSN_TxMsgQ, "MQTTSN_TX_MSG_Q" );
    configASSERT( MQTTSN_TxMsgQ );
    
    /* Create queue for all the free small message buffers */
    MQTTSN_FreeBuffQSm = xQueueCreate( NUM_BUFFERS_SMALL, sizeof(uint8_t *) );
    vQueueAddToRegistry( MQTTSN_FreeBuffQSm, "MQTTSN_FreeBuffSm_Q" );
    configASSERT( MQTTSN_FreeBuffQSm );
    
    /* Create queue for all the free large message buffers */
    MQTTSN_FreeBuffQLrg = xQueueCreate( NUM_BUFFERS_LARGE, sizeof(uint8_t *) );
    vQueueAddToRegistry( MQTTSN_FreeBuffQLrg, "MQTTSN_FreeBuffLrg_Q" );
    configASSERT( MQTTSN_FreeBuffQLrg );

    /* Create MQTTSN Task */
    xTaskCreate ( mqttsnTaskHandler, "MQTTSNTaskHandler", STACK_SIZE_ALLOC(STACK_SIZE_TASK_MQTTSN),  &ucParameterToPass, PRIORITY_TASK_MQTTSN, &xHandleTaskMQTTSN);
    configASSERT( xHandleTaskMQTTSN );
    
    return pdPASS;
}


