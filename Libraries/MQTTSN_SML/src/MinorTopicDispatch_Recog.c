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
*    File   : MinorTopicDispatch_Recog.c
*    Purpose: Recognition minor topic dispatch
*                                                          
*=========================================================*/
#include "Fw_global_config.h"
#include <string.h>
#include "iop_messages.h"
#include "Mqttsn_MessageHandler.h"
#include "Mqttsn_Topics.h"
#include "Recognition.h"
#include "dbg_uart.h"

#if S3AI_FIRMWARE_IS_RECOGNITION

#define TOPIC_MODEL_GUID_LENGTH 16

/* dispatch entry */
struct recog_cmd_dispatch_entry {
    uint32_t value;
    void (*handler)(Mqttsn_IOMsgData_t *);
    uint32_t expPayldLength;
    uint8_t isVariableLen;
};

/* handle MODEL GUID command */
void do_get_model_guid(Mqttsn_IOMsgData_t *pIoMsgData)
{
    //No error to send back
    
    //reply with Publish payload
    int result = 0;
    Mqttsn_MsgData_t *pOutMsgData = &pIoMsgData->outgoingResponse;
    
    result = Mqttsn_AllocPublishPayload(MQTTSN_BUFFER_SMALL, &pOutMsgData->pMsgPayldBuf,
                                &pOutMsgData->allocLen);
    
    if(result)
    {
        const uint8_t *pGuid;
        uint8_t *pPayld = pOutMsgData->pMsgPayldBuf;
        
        pOutMsgData->topicId = TOPIC_RECOG_MODEL_UUID_RSP;
        
#if 0//S3AI_FIRMWARE_IS_RECOGNITION
        /* we only send the first GUID */
        pGuid = sml_get_model_uuid_ptr(0);
        
#else
        /* all zeros is no-guid */
        static const uint8_t dummy_guid[TOPIC_MODEL_GUID_LENGTH] = { 0 };
        pGuid = dummy_guid;
#endif
        
        memcpy((void *)(&pPayld), (const void *)pGuid, 
               TOPIC_MODEL_GUID_LENGTH);
        pOutMsgData->payldLen = TOPIC_MODEL_GUID_LENGTH;
    }
}

void do_recog_start(Mqttsn_IOMsgData_t *pIoMsgData)
{
    recognition_startstop( RECOG_CMD_RECOGNIZE_START );
}

void do_recog_stop(Mqttsn_IOMsgData_t *pIoMsgData)
{
    recognition_startstop( RECOG_CMD_RECOGNIZE_STOP );
}

void Recogntion_AllStop(void)
{
    recognition_startstop( RECOG_CMD_RECOGNIZE_STOP );
}

/* recognition commands  */
static struct recog_cmd_dispatch_entry const rcde_table[] = {
    { .value = TOPIC_RECOG_MODEL_UUID_REQ, do_get_model_guid, .expPayldLength = 0, .isVariableLen = 0 },
    { .value = TOPIC_RECOG_START, do_recog_start, .expPayldLength = 0, .isVariableLen = 0 },
    { .value = TOPIC_RECOG_STOP, do_recog_stop, .expPayldLength = 0, .isVariableLen = 0 },
    /* terminate */
    { .value = -1, .handler = NULL }
};


// process the Recognition minor command
 void run_recognition_cmd(Mqttsn_IOMsgData_t *pIoMsgData )
{
    Mqttsn_MsgData_t *pInMsg = &pIoMsgData->incomingMsg;
    
    const struct recog_cmd_dispatch_entry *pRCDE;
    
    //if its a PUBACK, we have no use for it here just return
    if(pInMsg->msgType == MQTTSN_PUBACK)
        return;    
    
    for( pRCDE = rcde_table; pRCDE->handler != NULL ; pRCDE++ )
    {
        if( pRCDE->value == pInMsg->topicId)
        {
            uint32_t payldErr = CheckPayloadValidity(&pIoMsgData->incomingMsg, 
                                                     pRCDE->expPayldLength, 
                                                     pRCDE->isVariableLen);
            
            if(!payldErr)
            {            
                (*(pRCDE->handler))(pIoMsgData);
                return;
            }
            else
            {
                //there is a payload error, send an error message back
                PopulateRejectMsg(pIoMsgData, SYS_ERR_EINVAL);
            }            
        }
    }
    
    /* unknown */
    dbg_str("err-sensor-subcmd\n");
    set_sys_error( SYS_ERR_ENOTSUP, pInMsg->topicId );
}


#endif //#if S3AI_FIRMWARE_IS_RECOGNITION
