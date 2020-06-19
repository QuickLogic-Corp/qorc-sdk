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

#ifndef __MQTTSN_TOPICS_H
#define __MQTTSN_TOPICS_H

#include "MQTTSNPacket.h"

#define MAX_TOPIC_ID                0xffff

#define TOPIC_MAJOR_SYSTEM      0
#define TOPIC_MAJOR_LIVE        1
#define TOPIC_MAJOR_RESULT      2
#define TOPIC_MAJOR_RECOG       3
#define TOPIC_MAJOR_STORAGE     4
#define TOPIC_MAJOR_COLLECT     5
#define TOPIC_MAJOR_SENSOR      6
#define TOPIC_MAJOR_MAX         7
#define TOPIC_ID_UNASSIGNED     0xffff

#define MAKE_TOPIC_ID(MAJOR, MINOR) ((((MAJOR)&0xff)<<8)|((MINOR)&0xff))    
#define GET_TOPIC_MAJOR(TOPICID) (((TOPICID)>>8)&0xff)
#define GET_TOPIC_MINOR(TOPICID) ((TOPICID)&0xff)

#define TOPIC_SYS_WILL_STATUS           MAKE_TOPIC_ID(TOPIC_MAJOR_SYSTEM, 1)
#define TOPIC_SYS_ALL_STOP              MAKE_TOPIC_ID(TOPIC_MAJOR_SYSTEM, 2)
#define TOPIC_SYS_DEVICE_UUIDS_REQ      MAKE_TOPIC_ID(TOPIC_MAJOR_SYSTEM, 3)
#define TOPIC_SYS_DEVICE_UUIDS_RSP      MAKE_TOPIC_ID(TOPIC_MAJOR_SYSTEM, 4)
#define TOPIC_SYS_VERSION_REQ           MAKE_TOPIC_ID(TOPIC_MAJOR_SYSTEM, 5)
#define TOPIC_SYS_VERSION_RSP           MAKE_TOPIC_ID(TOPIC_MAJOR_SYSTEM, 6)
#define TOPIC_SYS_COMPDATETIME_REQ      MAKE_TOPIC_ID(TOPIC_MAJOR_SYSTEM, 7)
#define TOPIC_SYS_COMPDATETIME_RSP      MAKE_TOPIC_ID(TOPIC_MAJOR_SYSTEM, 8)
#define TOPIC_SYS_UNIXTIME_SET          MAKE_TOPIC_ID(TOPIC_MAJOR_SYSTEM, 9)
#define TOPIC_SYS_STATUS_REQ            MAKE_TOPIC_ID(TOPIC_MAJOR_SYSTEM, 10)
#define TOPIC_SYS_STATUS_RSP            MAKE_TOPIC_ID(TOPIC_MAJOR_SYSTEM, 12)
#define TOPIC_SYS_STATUS_CLR            MAKE_TOPIC_ID(TOPIC_MAJOR_SYSTEM, 13)
#define TOPIC_SYS_ERROR                 MAKE_TOPIC_ID(TOPIC_MAJOR_SYSTEM, 255)

#define TOPIC_LIVE_START                MAKE_TOPIC_ID(TOPIC_MAJOR_LIVE, 0)
#define TOPIC_LIVE_STOP                 MAKE_TOPIC_ID(TOPIC_MAJOR_LIVE, 1)
#define TOPIC_LIVE_RAW_DATA             MAKE_TOPIC_ID(TOPIC_MAJOR_LIVE, 2)
#define TOPIC_LIVE_SENSOR_LIST_REQ      MAKE_TOPIC_ID(TOPIC_MAJOR_LIVE, 3)
#define TOPIC_LIVE_SENSOR_LIST_RSP      MAKE_TOPIC_ID(TOPIC_MAJOR_LIVE, 4)
#define TOPIC_LIVE_SET_RATE_REQ         MAKE_TOPIC_ID(TOPIC_MAJOR_LIVE, 5)
#define TOPIC_LIVE_SET_RATE_RSP         MAKE_TOPIC_ID(TOPIC_MAJOR_LIVE, 6)

#define TOPIC_RESULT_CLASS_START       MAKE_TOPIC_ID(TOPIC_MAJOR_RESULT, 0)
#define TOPIC_RESULT_CLASS_STOP        MAKE_TOPIC_ID(TOPIC_MAJOR_RESULT, 1)
#define TOPIC_RESULT_CLASS_DATA        MAKE_TOPIC_ID(TOPIC_MAJOR_RESULT, 2)
#define TOPIC_RESULT_CLASS_SET_RATE    MAKE_TOPIC_ID(TOPIC_MAJOR_RESULT, 3)

#define TOPIC_RECOG_MODEL_UUID_REQ      MAKE_TOPIC_ID(TOPIC_MAJOR_RECOG, 0)
#define TOPIC_RECOG_MODEL_UUID_RSP      MAKE_TOPIC_ID(TOPIC_MAJOR_RECOG, 1)
#define TOPIC_RECOG_START               MAKE_TOPIC_ID(TOPIC_MAJOR_RECOG, 2)
#define TOPIC_RECOG_STOP                MAKE_TOPIC_ID(TOPIC_MAJOR_RECOG, 3)

#define TOPIC_STORAGE_DIR_REQ          MAKE_TOPIC_ID(TOPIC_MAJOR_STORAGE, 0)
#define TOPIC_STORAGE_DIR_RSP          MAKE_TOPIC_ID(TOPIC_MAJOR_STORAGE, 1)
#define TOPIC_STORAGE_SPACE_REQ        MAKE_TOPIC_ID(TOPIC_MAJOR_STORAGE, 2)
#define TOPIC_STORAGE_SPACE_RSP        MAKE_TOPIC_ID(TOPIC_MAJOR_STORAGE, 3)
#define TOPIC_STORAGE_DEL              MAKE_TOPIC_ID(TOPIC_MAJOR_STORAGE, 4)
#define TOPIC_STORAGE_GET_START        MAKE_TOPIC_ID(TOPIC_MAJOR_STORAGE, 5)
#define TOPIC_STORAGE_GET_DATA_REQ     MAKE_TOPIC_ID(TOPIC_MAJOR_STORAGE, 6)
#define TOPIC_STORAGE_GET_DATA_RSP     MAKE_TOPIC_ID(TOPIC_MAJOR_STORAGE, 7)
#define TOPIC_STORAGE_GET_STOP         MAKE_TOPIC_ID(TOPIC_MAJOR_STORAGE, 8)
#define TOPIC_STORAGE_PUT_START        MAKE_TOPIC_ID(TOPIC_MAJOR_STORAGE, 9)
#define TOPIC_STORAGE_PUT_DATA_REQ     MAKE_TOPIC_ID(TOPIC_MAJOR_STORAGE, 10)
#define TOPIC_STORAGE_PUT_STOP         MAKE_TOPIC_ID(TOPIC_MAJOR_STORAGE, 11)

#define TOPIC_COLLECT_PREFIX_SET       MAKE_TOPIC_ID(TOPIC_MAJOR_COLLECT, 0)
#define TOPIC_COLLECT_START            MAKE_TOPIC_ID(TOPIC_MAJOR_COLLECT, 1)
#define TOPIC_COLLECT_STOP             MAKE_TOPIC_ID(TOPIC_MAJOR_COLLECT, 2)

#define TOPIC_SENSOR_LIST_REQ       MAKE_TOPIC_ID(TOPIC_MAJOR_SENSOR, 0)
#define TOPIC_SENSOR_LIST_RSP       MAKE_TOPIC_ID(TOPIC_MAJOR_SENSOR, 1)
#define TOPIC_SENSOR_CLEAR          MAKE_TOPIC_ID(TOPIC_MAJOR_SENSOR, 2)
#define TOPIC_SENSOR_ADD            MAKE_TOPIC_ID(TOPIC_MAJOR_SENSOR, 3)
#define TOPIC_SENSOR_DONE           MAKE_TOPIC_ID(TOPIC_MAJOR_SENSOR, 4)


/********************************OLD****************************************/
#define TOPIC_SENSOR_SELECT_BY_ID       MAKE_TOPIC_ID(TOPIC_MAJOR_SENSOR, 0)
#define TOPIC_SENSOR_START              MAKE_TOPIC_ID(TOPIC_MAJOR_SENSOR, 1)
#define TOPIC_SENSOR_STOP               MAKE_TOPIC_ID(TOPIC_MAJOR_SENSOR, 2)


/*Info needed for each topic*/
typedef struct{
    char *tpcString;
    MQTTSN_topicid assgndID;
    uint16_t topicValue;
    uint8_t status;
    int8_t  qosLevel;
    uint8_t isRegistration; //0-subscribe to the topic, 1-register to the topic
    uint8_t retainFlag;
}Mqttsn_TopicInfo_t;

/*MQTTSN QoS levels */
typedef enum QoS
{   QOS_MINUS1 = -1,
    QOS0,
    QOS1,
    QOS2 
}Mqttsn_QosLevels_t;

extern Mqttsn_TopicInfo_t Topics[];
Mqttsn_TopicInfo_t * Mqttsn_GetTopicInfo(uint16_t topicId);
Mqttsn_TopicInfo_t * Mqttsn_GetInternalTopicInfo(uint16_t topicId);
void Mqttsn_UpdateTopicSubStatus(uint16_t topicId, uint8_t status, int8_t qosLevel);
void Mqttsn_UpdateTopicUnSubStatus(uint16_t topicId);
void Mqttsn_UnsubscribeAllTopics(void);
void Mqttsn_FakeSubscriptions(void);
#endif
