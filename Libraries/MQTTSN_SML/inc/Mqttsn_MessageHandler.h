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

#ifndef __MQTTSN_MESSAGEHANDLER_H
#define __MQTTSN_MESSAGEHANDLER_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "MQTTSNPacket.h"
#include "MQTTSNConnect.h"
#include "Mqttsn_Topics.h"

/*************************************Defines**********************************/
#define MAX_GW_ADDR_LEN             30   //revisit
#define MAX_PENDING_MSGS            5
#define MQTTSN_MAX_RETRIES          3       /* Spec says between 3-5 is sufficient */
#define MQTTSN_RETRY_PERIOD         15000      /*in seconds*/
#define PENDING_Q_CHECK_INTERVAL    50
#define NUM_BUFFERS_SMALL           20
#define NUM_BUFFERS_LARGE           10
#define RADIUS_OF_MSG               ((uint8_t)3)
#define KEEP_ALIVE_TIMER            60  //in secs
#define MQTTSN_QUEUE_LENGTH         30  //msg queue size for mqttsnTaskHandler
#define MQTTSN_MSGQ_WAIT_TIME       portMAX_DELAY
#define MQTTSN_MSGQ_SEND_WAIT_TICKS 100
#define MQTTSN_MESSAGE_TYPES_MAX    30
#define MQTTSN_LEN_FIELD_MSG_SIZE   256

/*********************************Data Structures******************************/

/*Message direction*/
typedef enum{
    MSG_TX,
    MSG_RX
}MsgDirection_t;

/*MQTTSN return codes*/
typedef enum {
        MQTTSN_ACCEPTED,
        MQTTSN_REJECTED_CONGESTED,
        MQTTSN_REJECTED_INVALID_TOPIC_ID,
        MQTTSN_NOT_SUPPORTED
}MQTTSN_retCodes_t;

/*Publish internal error codes for the caller*/
typedef enum{
    MQTTSN_TX_PUBLISH_NO_ERROR,
    MQTTSN_TX_PUBLISH_NOT_CONNECTED,
    MQTTSN_TX_PUBLISH_TOPIC_NOT_SUBSCRIBED,
    MQTTSN_TX_PUBLISH_PAYLD_BUFFER_NOT_ALLOCED,
    MQTTSN_TX_PUBLISH_TX_QUEUE_FULL
}Mqttsn_TxPublishErrorCodes_t;

typedef enum{
    MQTTSN_WILL_STATUS_OFFLINE,
    MQTTSN_WILL_STATUS_ONLINE
}Mqttsn_WillStatus_t;

/*Message information, all the data needed to form an MQTTSN message*/
typedef struct{
    int8_t  qosLevel;       //qos level of the message
    uint8_t retain;         //retain flag value 
    uint8_t *pMsgBuf;       //pointer to the message buffer
    uint8_t *pMsgPayldBuf;  //pointer to the message payload
    uint16_t pktId;         //message id of the packet
    uint32_t allocLen;      //allocated length of the message
    uint32_t msgLen;        //length of the message
    uint32_t payldLen;      //length of the payload
    MsgDirection_t  msgDirec;   //msg direction : tx or rx
    MQTTSN_retCodes_t retCode;  //return code if any in the message
    uint16_t topicId;           //topic id of the publish
    enum MQTTSN_msgTypes msgType;   //MQTTSN msg type
} Mqttsn_MsgData_t;

/*Basic IO message cluster for processing each incoming message */
typedef struct{
    Mqttsn_MsgData_t incomingMsg;  //incoming message - can be request or ack
    Mqttsn_MsgData_t outgoingAck;    //any outgoing Ack
    Mqttsn_MsgData_t outgoingResponse;  //outgoing publish - response to the request
}Mqttsn_IOMsgData_t;

/*Dispatch funsction handlers for all MQTTSN messages*/
struct MessageDispatch{
    void (*txHandler)(Mqttsn_MsgData_t *);
    void (*rxHandler)(Mqttsn_IOMsgData_t *);
};

/*Any MQTTSN message needs to store the followinf fields in case a retransmission 
is required*/
typedef struct{
  uint8_t       numTries;   //num of tries so far
  uint8_t       isValid;    //marks entry validity
  uint8_t       *pMsgBuff;  //pointer to the message buffer
  uint16_t      msgId;      //Msg id
  uint16_t      topicId;    //topic id of the message if any
  enum MQTTSN_msgTypes currMsg; //message type
  intptr_t      txTimerToken;   //time of the last transmssion of this message
  uint32_t       msgLen;        //length of the message
  uint32_t       allocLen;      //allocated length of the message
}pendingMsg_t;

/*General MQTTSN information needed to maintain state, GW info, etc*/
typedef struct {
    uint8_t       isConnected;    //State info of the connection
    uint8_t       gwId;           //Gateway ID
    uint8_t       pad;            
    uint8_t       pubPending;     //flag for any pending publish ack  
    uint8_t       gwAddress[MAX_GW_ADDR_LEN];   //GW address
    uint16_t      gwAdvDuration;  //Gateway advertize duration
    uint16_t      gwAddressLen;   //Gateway address length
    uint16_t      nextMsgId;      //next message id  
    MQTTSNPacket_connectData connData;  //connection parameters
    intptr_t      pendQTimeVal;         //pending Q timer token
    intptr_t      pingTimerToken;       //ping timer token  
    pendingMsg_t  pendingMsgQueue[MAX_PENDING_MSGS];    //pending message list
}mqttsnInfo_t;

/*Types of buffers */
typedef enum{
    MQTTSN_BUFFER_SMALL = 60,   //40 bytes
    //MQTTSN_BUFFER_MEDIUM      //future
    MQTTSN_BUFFER_LARGE = 1056  //1k
}Mqttsn_BuffSz_t;

/*Mqttsn protocol data unit info*/
typedef struct{
    uint8_t *pBuf;
    uint32_t numBytes;
    uint32_t allocLen;
}MqttsnPdu_t;

/************************************APIs**************************************/

extern signed portBASE_TYPE StartRtosTaskMqttsnMsgHandler( void);
uint32_t Mqttsn_CopyMsg(Mqttsn_MsgData_t *pMsgData, uint8_t *pBuf, uint32_t numBytes);
uint32_t Mqttsn_ProcessIncmgMsg(Mqttsn_IOMsgData_t *pIoMsgData, MqttsnPdu_t *pMqttsnPdu);
uint32_t Mqttsn_AllocMsg(Mqttsn_BuffSz_t buffSz, uint8_t **pBuf, uint32_t *pBufLen);
uint32_t Mqttsn_AllocPublishPayload(Mqttsn_BuffSz_t buffSz, uint8_t **pBuf, uint32_t *pBufLen);
uint32_t Mqttsn_SendReply(Mqttsn_MsgData_t *pMsgData);
Mqttsn_TxPublishErrorCodes_t Mqttsn_SendPublish(Mqttsn_MsgData_t *pMsgData, Mqttsn_TopicInfo_t *pTopicInfo);
void Mqttsn_FreePublishPayload(uint8_t **pBuf, uint32_t allocLen);
uint32_t Mqttsn_FreeMsg(uint8_t **pBuf);
uint32_t Mqttsn_Subscribe(Mqttsn_TopicInfo_t *pTopicInfo);
uint32_t Mqttsn_Register(Mqttsn_TopicInfo_t *pTopicInfo);
uint32_t Mqttsn_SendConnectionRequest(void);
extern void SendMessage(uint8_t *pBuf, uint32_t bufLen);
uint32_t Mqttsn_BuffWr_u64(uint8_t **pBuf, uint64_t v );
uint32_t Mqttsn_BuffWr_u32(uint8_t **pBuf, uint32_t v );
uint32_t Mqttsn_BuffWr_u16(uint8_t **pBuf, uint16_t v );
uint32_t Mqttsn_BuffWr_u8(uint8_t **pBuf, uint8_t v );
uint8_t Mqttsn_BuffRead_u8(uint8_t **pBuff);
uint16_t Mqttsn_BuffRead_u16(uint8_t **pBuff);
uint32_t Mqttsn_BuffRead_u32(uint8_t **pBuff);
void Mqttsn_PublishWillStatus(Mqttsn_WillStatus_t willStatus);
uint32_t GetPayloadOffset(uint32_t msgLength);
#endif  //MQTTSN_H
