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
*    File   : MinorTopicDispatch_System.c
*    Purpose: System minor topic dispatch
*                                                          
*=========================================================*/
#include "Fw_global_config.h"
#include <stdio.h>
#include "RtosTask.h"
#include "iop_messages.h"
#include "ble_collection_defs.h"
#include "dcl_commands.h"
#include "Mqttsn_MessageHandler.h"
#include "Mqttsn_Topics.h"
#include "dbg_uart.h"
#include <string.h>
#include "micro_tick64.h"
#include "ql_time.h"

#define TOPIC_COMPDATE_LENGTH   20
#define TOPIC_VERSION_LENGTH    20
#define TOPIC_UUIDS_LENGTH      32

/* dispatch entry */
struct system_cmd_dispatch_entry {
    uint32_t value;
    void (*handler)(Mqttsn_IOMsgData_t *);
    uint32_t expPayldLength;
    uint8_t isVariableLen;
};

void do_all_stop(Mqttsn_IOMsgData_t *pIoMsgData)
{
    //We have disconnected from the network, stop everything
    Live_AllStop();
    
#if S3AI_FIRMWARE_IS_RECOGNITION    
    Recogntion_AllStop();
#endif //#if S3AI_FIRMWARE_IS_RECOGNITION   
    
#if S3AI_FIRMWARE_IS_COLLECTION    
    Collect_AllStop();
#endif //S3AI_FIRMWARE_IS_COLLECTION    
    
    Storage_AllStop();
}

void do_uuids_req(Mqttsn_IOMsgData_t *pIoMsgData)
{
    //No error to send back
    
    //reply with a Publish payload
    int result = 0;
    Mqttsn_MsgData_t *pOutMsgData = &pIoMsgData->outgoingResponse;
    
    result = Mqttsn_AllocPublishPayload(MQTTSN_BUFFER_SMALL, &pOutMsgData->pMsgPayldBuf,
                                        &pOutMsgData->allocLen);
    
    if(result)
    {
        pOutMsgData->topicId = TOPIC_SYS_DEVICE_UUIDS_RSP;
        pOutMsgData->payldLen = TOPIC_UUIDS_LENGTH;
        
        memcpy((void *)pOutMsgData->pMsgPayldBuf, DeviceClassUUID, UUID_TOTAL_BYTES);
        
        //DUNIQUE_UUID left at 0s for now
    }
    else
    {
        printf("Error: do_uuids_req - buffer not allocated");
    }
}

/* handle get sw version command */
void do_get_version(Mqttsn_IOMsgData_t *pIoMsgData)
{
    //No error to send back
    
    //reply with a Publish payload
    int result = 0;
    Mqttsn_MsgData_t *pOutMsgData = &pIoMsgData->outgoingResponse;
    
    result = Mqttsn_AllocPublishPayload(MQTTSN_BUFFER_SMALL, &pOutMsgData->pMsgPayldBuf,
                                &pOutMsgData->allocLen);
    
    if(result)
    {
        uint8_t *pBuf = pOutMsgData->pMsgPayldBuf;
        
        pOutMsgData->topicId = TOPIC_SYS_VERSION_RSP;
        pOutMsgData->payldLen = strlen(SOFTWARE_VERSION_STR);
        
        iop_reply_string(pBuf, SOFTWARE_VERSION_STR, NULL );
    }
    else
    {
        printf("Error: do_get_version - buffer not allocated");
    }
    
}


/* handle COMPILER date/time stamp command */
void do_get_compdatetime(Mqttsn_IOMsgData_t *pIoMsgData)
{
    //No error to send back
    
    //reply with a Publish payload
    int result = 0;
    Mqttsn_MsgData_t *pOutMsgData = &pIoMsgData->outgoingResponse;
    
    result = Mqttsn_AllocPublishPayload(MQTTSN_BUFFER_SMALL, &pOutMsgData->pMsgPayldBuf,
                                &pOutMsgData->allocLen);
    
    if(result)
    {
        uint8_t *pPayld = pOutMsgData->pMsgPayldBuf;
        uint32_t buffLen = 0;
        
        pOutMsgData->topicId = TOPIC_SYS_COMPDATETIME_RSP;
        
        strcpy((char *)&pPayld[buffLen], __DATE__);
        buffLen += strlen(__DATE__);
        
        pPayld[buffLen++] = ' ';
        
        strcpy((char *)&pPayld[buffLen], __TIME__);
        buffLen += strlen(__TIME__);
        
        pOutMsgData->payldLen = buffLen;
        
    }
    else
    {
        printf("Error: do_get_version - buffer not allocated");
    }
    
}


void do_set_unixtime(Mqttsn_IOMsgData_t *pIoMsgData)
{
    uint64_t v;
    uint32_t unixTime = Mqttsn_BuffRead_u32(&pIoMsgData->incomingMsg.pMsgPayldBuf);
    ql_sys_settime( unixTime );
    v = unixTime;
    v = v * 1000000;
    xTaskSet_uSecCount(v);
}

void do_get_status(Mqttsn_IOMsgData_t *pIoMsgData)
{
    //No error to send back
    
    //reply with a Publish payload
    int result = 0;
    Mqttsn_MsgData_t *pOutMsgData = &pIoMsgData->outgoingResponse;
    
    result = Mqttsn_AllocPublishPayload(MQTTSN_BUFFER_SMALL, &pOutMsgData->pMsgPayldBuf,
                                &pOutMsgData->allocLen);
    
    if(result)
    {
        uint8_t *pPayld = pOutMsgData->pMsgPayldBuf;
        pOutMsgData->payldLen = 0;
        
        pOutMsgData->topicId = TOPIC_SYS_STATUS_RSP;
        
        pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, iop_globals.cur_status.bytes_saved);
        pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, iop_globals.cur_status.bit_flags);
        pOutMsgData->payldLen += Mqttsn_BuffWr_u16(&pPayld, iop_globals.cur_status.rx_count);
        pOutMsgData->payldLen += Mqttsn_BuffWr_u16(&pPayld, iop_globals.cur_status.tx_count);
        pOutMsgData->payldLen += Mqttsn_BuffWr_u16(&pPayld, iop_globals.cur_status.live_oe_count);
        pOutMsgData->payldLen += Mqttsn_BuffWr_u16(&pPayld, iop_globals.cur_status.collect_oe_count);
        pOutMsgData->payldLen += Mqttsn_BuffWr_u8(&pPayld, iop_globals.cur_status.sticky_error_code);
        pOutMsgData->payldLen += Mqttsn_BuffWr_u8(&pPayld, iop_globals.cur_status.error_count);
        
    }
}

void do_clr_status(Mqttsn_IOMsgData_t *pIoMsgData)
{
    memset( &(iop_globals.cur_status), 0, sizeof(iop_globals.cur_status) );
}


/* sensor commands we support - in a table */
static struct system_cmd_dispatch_entry const sys_cmd_table[] = {
    { .value = TOPIC_SYS_ALL_STOP, do_all_stop, .expPayldLength = 0, .isVariableLen = 0 },
    { .value = TOPIC_SYS_DEVICE_UUIDS_REQ, do_uuids_req, .expPayldLength = 0, .isVariableLen = 0 },
    { .value = TOPIC_SYS_VERSION_REQ, do_get_version, .expPayldLength = 0, .isVariableLen =  0 },
    { .value = TOPIC_SYS_COMPDATETIME_REQ, do_get_compdatetime, .expPayldLength = 0, .isVariableLen = 0  },
    { .value = TOPIC_SYS_UNIXTIME_SET, do_set_unixtime, .expPayldLength = 4, .isVariableLen =  0 },
    { .value = TOPIC_SYS_STATUS_REQ, do_get_status, .expPayldLength = 0, .isVariableLen =  0 },
    { .value = TOPIC_SYS_STATUS_CLR, do_clr_status, .expPayldLength = 0, .isVariableLen =  0 },
    /* terminate */
    { .value = -1, .handler = NULL }
};

//  Dispatch the minor topic
 void run_system_cmd(Mqttsn_IOMsgData_t *pIoMsgData )
{
    Mqttsn_MsgData_t *pInMsg = &pIoMsgData->incomingMsg;
    
    const struct system_cmd_dispatch_entry *pSCDE;
    
    //if its a PUBACK, we have no use for it here just return
    if(pInMsg->msgType == MQTTSN_PUBACK)
        return;
    
    for( pSCDE = sys_cmd_table; pSCDE->handler != NULL ; pSCDE++ )
    {
        if( pSCDE->value == pInMsg->topicId )
        {
            uint32_t payldErr = CheckPayloadValidity(&pIoMsgData->incomingMsg, 
                                                     pSCDE->expPayldLength, 
                                                     pSCDE->isVariableLen);
            if(!payldErr)
            {            
                (*(pSCDE->handler))(pIoMsgData);
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
    dbg_str("err-system-subcmd\n");
    set_sys_error( SYS_ERR_ENOTSUP, pInMsg->topicId);
    
}
