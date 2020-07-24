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
*    File   : Mqttsn_Topics.c
*    Purpose: MQTTSN Topics handling
*                                                          
*=========================================================*/
#include "Fw_global_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RtosTask.h"
#include "Mqttsn_Topics.h"

//Global Qos level for all topics
#define DEFAULT_QOS_LEVEL       QOS1 
#define DEFAULT_RETAIN_FLAG     0
#define WILL_MSG_RETAIN_FLAG    1

Mqttsn_TopicInfo_t Topics[] = 
{
    {   
        .topicValue = TOPIC_SYS_WILL_STATUS,
        .tpcString = "sensiml/sys/will/status",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 1,
        .retainFlag = WILL_MSG_RETAIN_FLAG
    },
    /*********************************SYSTEM************************************/
    {   
        .topicValue = TOPIC_SYS_ALL_STOP,
        .tpcString = "sensiml/sys/all/stop",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = WILL_MSG_RETAIN_FLAG
    },   
    {   
        .topicValue = TOPIC_SYS_VERSION_REQ,
        .tpcString = "sensiml/sys/version/req",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_SYS_VERSION_RSP, 
        .tpcString = "sensiml/sys/version/rsp",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL, 
        .isRegistration = 1,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_SYS_COMPDATETIME_REQ,
        .tpcString = "sensiml/sys/compdatetime/req",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_SYS_COMPDATETIME_RSP,
        .tpcString = "sensiml/sys/compdatetime/rsp",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 1,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_SYS_UNIXTIME_SET,
        .tpcString = "sensiml/sys/unixtime/set",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_SYS_STATUS_REQ,
        .tpcString = "sensiml/sys/status/req",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_SYS_STATUS_RSP,
        .tpcString = "sensiml/sys/status/rsp",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 1,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_SYS_STATUS_CLR,
        .tpcString = "sensiml/sys/status/clr",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_SYS_ERROR,
        .tpcString = "sensiml/sys/error",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL},
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 1,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    /*********************************LIVE************************************/
    {
        .topicValue = TOPIC_LIVE_START,
        .tpcString = "sensiml/live/start",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL},
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_LIVE_STOP,
        .tpcString = "sensiml/live/stop",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL},
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {
        .topicValue = TOPIC_LIVE_RAW_DATA,
        .tpcString = "sensiml/live/raw/data",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL},
        .qosLevel = QOS0,
        .isRegistration = 1,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {
        .topicValue = TOPIC_LIVE_SENSOR_LIST_REQ,
        .tpcString = "sensiml/live/sensor/list/req",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL},
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {
        .topicValue = TOPIC_LIVE_SENSOR_LIST_RSP,
        .tpcString = "sensiml/live/sensor/list/rsp",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL},
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 1,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {
        .topicValue = TOPIC_LIVE_SET_RATE_REQ,
        .tpcString = "sensiml/live/set/rate/req",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL},
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {
        .topicValue = TOPIC_LIVE_SET_RATE_RSP,
        .tpcString = "sensiml/live/set/rate/rsp",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL},
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 1,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },

    /*********************************RESULT**************************************/
    {   
        .topicValue = TOPIC_RESULT_CLASS_START,
        .tpcString = "sensiml/result/class/start",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_RESULT_CLASS_STOP,
        .tpcString = "sensiml/result/class/stop",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_RESULT_CLASS_DATA,
        .tpcString = "sensiml/result/class/data",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 1,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {
        .topicValue = TOPIC_RESULT_CLASS_SET_RATE,
        .tpcString = "sensiml/result/class/set/rate",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },

    /***************************RECOGNITION***********************************/
    {
        .topicValue = TOPIC_RECOG_MODEL_UUID_REQ,
        .tpcString = "sensiml/recog/model/uuid/req",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_RECOG_MODEL_UUID_RSP,
        .tpcString = "sensiml/recog/model/uuid/rsp",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 1,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_RECOG_START,
        .tpcString = "sensiml/recog/start",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_RECOG_STOP,
        .tpcString = "sensiml/recog/stop",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    
    /***************************STORAGE***********************************/
    {   
        .topicValue = TOPIC_STORAGE_DIR_REQ,
        .tpcString = "sensiml/storage/dir/req",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_STORAGE_DIR_RSP,
        .tpcString = "sensiml/storage/dir/rsp",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 1,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_STORAGE_SPACE_REQ,
        .tpcString = "sensiml/storage/space/req",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_STORAGE_SPACE_RSP,
        .tpcString = "sensiml/storage/space/rsp",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 1,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_STORAGE_DEL,
        .tpcString = "sensiml/storage/del",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_STORAGE_GET_START,
        .tpcString = "sensiml/storage/get/start",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_STORAGE_GET_DATA_REQ,
        .tpcString = "sensiml/storage/get/data/req",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_STORAGE_GET_DATA_RSP,
        .tpcString = "sensiml/storage/get/data/rsp",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 1,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_STORAGE_GET_STOP,
        .tpcString = "sensiml/storage/get/stop",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_STORAGE_PUT_START,
        .tpcString = "sensiml/storage/put/start",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_STORAGE_PUT_DATA_REQ,
        .tpcString = "sensiml/storage/put/data/req",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_STORAGE_PUT_STOP,
        .tpcString = "sensiml/storage/put/stop",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    /***************************COLLECT***********************************/
    {   
        .topicValue = TOPIC_COLLECT_PREFIX_SET,
        .tpcString = "sensiml/collect/prefix/set",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_COLLECT_START,
        .tpcString = "sensiml/collect/start",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_COLLECT_STOP,
        .tpcString = "sensiml/collect/stop",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    /***************************SENSOR***********************************/
    {   
        .topicValue = TOPIC_SENSOR_LIST_REQ,
        .tpcString = "sensiml/sensor/list/req",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_SENSOR_LIST_RSP,
        .tpcString = "sensiml/sensor/list/rsp",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 1,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_SENSOR_CLEAR,
        .tpcString = "sensiml/sensor/clr",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
        {   
        .topicValue = TOPIC_SENSOR_ADD,
        .tpcString = "sensiml/sensor/add",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_SENSOR_DONE,
        .tpcString = "sensiml/sensor/done",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },
    {   
        .topicValue = TOPIC_SYS_DEVICE_UUIDS_REQ,
        .tpcString = "sensiml/sys/device/uuids/req",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 0,
        .retainFlag = DEFAULT_RETAIN_FLAG
    },    
    {   
        .topicValue = TOPIC_SYS_DEVICE_UUIDS_RSP,
        .tpcString = "sensiml/sys/device/uuids/rsp",
        .assgndID = { .data.id = TOPIC_ID_UNASSIGNED, .type = MQTTSN_TOPIC_TYPE_NORMAL}, 
        .qosLevel = DEFAULT_QOS_LEVEL,
        .isRegistration = 1,
        .retainFlag = DEFAULT_RETAIN_FLAG        
    },
    {
        .topicValue = MAX_TOPIC_ID,
    }
        
};

/*returns the topic info pointer from the Topics table*/
Mqttsn_TopicInfo_t * Mqttsn_GetTopicInfo(uint16_t topicId)
{
    uint32_t index;
    
    for(index = 0; Topics[index].topicValue != MAX_TOPIC_ID; index++)
    {
        if(Topics[index].topicValue == topicId)
        {
            return (&Topics[index]);
        }
    }
    
    //no valid topic info found
    return NULL;
    
}

/*returns the topic info pointer from the Topics table*/
Mqttsn_TopicInfo_t * Mqttsn_GetInternalTopicInfo(uint16_t topicId)
{
    uint32_t index;
    
    for(index = 0; Topics[index].topicValue != MAX_TOPIC_ID; index++)
    {
        if(Topics[index].assgndID.data.id == topicId)
        {
            return (&Topics[index]);
        }
    }
    
    //no valid topic info found
    return NULL;
    
}

/*Update the subscription status from GW*/
void Mqttsn_UpdateTopicSubStatus(uint16_t topicId, uint8_t status, int8_t qosLevel)
{
    Mqttsn_TopicInfo_t *pTopic;
    
    pTopic = Mqttsn_GetTopicInfo(topicId);
    
    if(pTopic != NULL)
    {
        pTopic->status = status;
        
        if(pTopic->isRegistration == 0) //update qos level only for subscriptions
        {
            pTopic->qosLevel = qosLevel;  
        }
    }
    else
    {
        //If topic not found print error and return
        printf("Error: Mqttsn_UpdateTopicSubStatus - topic id %d not found\n", topicId);
    }
}

/*update the topic we unsubscribed from*/
void Mqttsn_UpdateTopicUnSubStatus(uint16_t topicId)
{
    Mqttsn_TopicInfo_t *pTopic;
    
    pTopic = Mqttsn_GetTopicInfo(topicId);
        
    if(pTopic != NULL)
    {
        //there is no return code here
        //The fact that we received an unsub ack means unsub has been accepted    
        pTopic->status = 0;
        pTopic->assgndID.data.id = TOPIC_ID_UNASSIGNED;
    }
    else
    {
        //If topic not found print error and return
        printf("Error: Mqttsn_UpdateTopicUnSubStatus - topic id %d not found\n", topicId);
    }
}

void Mqttsn_UnsubscribeAllTopics(void)
{
    uint32_t index; 
    
    for(index = 0; Topics[index].topicValue != MAX_TOPIC_ID; index++)
    {
        Topics[index].status = 0;
        Topics[index].assgndID.data.id = TOPIC_ID_UNASSIGNED;
    }
}

void Mqttsn_FakeSubscriptions(void)
{
    uint32_t index; 
    
    for(index = 0; Topics[index].topicValue != MAX_TOPIC_ID; index++)
    {
        Topics[index].status = 1;
        Topics[index].assgndID.data.id = index+1;
    }
}

