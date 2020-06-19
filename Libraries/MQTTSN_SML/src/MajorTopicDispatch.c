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
*    File   : MajorTopicDispatch.c
*    Purpose: APIs to dispatch major topics
*                                                          
*=========================================================*/
#include "Fw_global_config.h"
#include "iop_messages.h"
#include <string.h>
#include <stdio.h>
#include "stdarg.h"
#include "dbg_uart.h"
#include "Mqttsn_MessageHandler.h"
#include "Mqttsn_Topics.h"

struct iop_globals iop_globals;


/* reply to host as a series of strings */
void iop_reply_string(uint8_t *pBuf,  const char *cp, ... )
{
    va_list ap;
    
    char *d;
    int n;
    int c;
    
    va_start(ap,cp);
    n = 0;
    d = (char *)pBuf;
    
    while( (n < sizeof(iop_globals.u_rsp.misc.u.cbuf)) && (cp != NULL) ){
        c = *cp++;
        if( c == 0 ){
            c = ' ';
            cp = va_arg( ap, const char * );
        }
        *d++ = c;
        n++;
    }
}


void PopulateRejectMsg(Mqttsn_IOMsgData_t *pIoMsgData, sys_error_code_t ftErr)
{
    int result = 0;
    Mqttsn_MsgData_t *pOutMsgData = &pIoMsgData->outgoingResponse;
    pOutMsgData->payldLen = 0;
    
    // get the ack and change return a REJECT
    pIoMsgData->incomingMsg.retCode = MQTTSN_NOT_SUPPORTED;
    
    //PUBLISH an error
    pOutMsgData->topicId = TOPIC_SYS_ERROR;
    result = Mqttsn_AllocPublishPayload(MQTTSN_BUFFER_SMALL, &pOutMsgData->pMsgPayldBuf,
                                  &pOutMsgData->allocLen);
    
    if(result)
    {
        uint8_t *pPayld = pOutMsgData->pMsgPayldBuf;
        Mqttsn_TopicInfo_t *pTopic = Mqttsn_GetTopicInfo(pIoMsgData->incomingMsg.topicId);

        pOutMsgData->payldLen += Mqttsn_BuffWr_u16(&pPayld, pIoMsgData->incomingMsg.pktId);
        pOutMsgData->payldLen += Mqttsn_BuffWr_u16(&pPayld, pTopic->assgndID.data.id);
        pOutMsgData->payldLen += Mqttsn_BuffWr_u16(&pPayld, pIoMsgData->incomingMsg.topicId);
        pOutMsgData->payldLen += Mqttsn_BuffWr_u8(&pPayld, ftErr);
        pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, 0);
    }
    
    //Also set the system error
    set_sys_error( ftErr, 0);
}

/* an error has occured .. update error bits */
void set_sys_error(sys_error_code_t errcode, uint32_t more_info )
{
    (void)(more_info);
    dbg_str_int("iop-error", errcode );
    if( 0 == iop_globals.cur_status.sticky_error_code ){
        iop_globals.cur_status.sticky_error_code  = errcode;
        
    }
    iop_globals.cur_status.bit_flags |= ANY_ERROR;
    iop_globals.cur_status.error_count++;
}

/* start sending data - track overrun if needed */
int send_data_start(void)
{
    if( iop_globals.data_busy ){
        iop_globals.cur_status.bit_flags |= LIVESTREAM_OVERRUN;
        iop_globals.cur_status.live_oe_count += 1;
        return -1;
    }
    
    iop_globals.data_busy  = 1;
    memset( (void *)(&iop_globals.u_data), 0, sizeof( iop_globals.u_data ) );
    
    return 0;
}

void send_data_end(void)
{
    iop_globals.data_busy  = 0;
}

uint32_t CheckPayloadValidity(Mqttsn_MsgData_t *pInputMsg, uint32_t expPayldLen, uint8_t isVariableLen)
{
    uint32_t result = 0;
    
    if(isVariableLen)
    {
        if(expPayldLen > pInputMsg->payldLen)
        {
            //send an error publish for unexpected payload length
            //FUTURE:Send a reject publish back
            result = 1;
        }
    }
    else
    {
        if(expPayldLen != pInputMsg->payldLen)
        {
            //send an error publish for unexpected payload length
            //FUTURE:Send a reject publish back            
            result = 1;
        }
    }
    
    return result;
}


typedef struct {
    uint8_t value;
    void (*handler)(Mqttsn_IOMsgData_t *);
}IopMajorTpcDisptach_t;

static IopMajorTpcDisptach_t MajorCmdsTbl[] = {
    { .value = TOPIC_MAJOR_SYSTEM,      .handler = run_system_cmd },
    { .value = TOPIC_MAJOR_LIVE,        .handler = run_livestream_cmd },
    { .value = TOPIC_MAJOR_RESULT,      .handler = run_livestream_cmd },
#if S3AI_FIRMWARE_IS_RECOGNITION    
    { .value = TOPIC_MAJOR_RECOG,       .handler = run_recognition_cmd},
#endif //#if S3AI_FIRMWARE_IS_RECOGNITION    
    { .value = TOPIC_MAJOR_STORAGE,     .handler = run_storage_cmd },
    
#if S3AI_FIRMWARE_IS_COLLECTION    
    { .value = TOPIC_MAJOR_COLLECT,     .handler = run_collect_cmd },
    { .value = TOPIC_MAJOR_SENSOR,      .handler = run_sensor_cmd },
#endif //S3AI_FIRMWARE_IS_COLLECTION
    
    { .value = 255,                     .handler = NULL }
};

void MajorTopicDispatch(Mqttsn_IOMsgData_t *pIoMsgData)
{
    const IopMajorTpcDisptach_t *pMjCmdDisp;
    Mqttsn_MsgData_t *pInMsgData = &pIoMsgData->incomingMsg;
    
    iop_globals.cur_status.rx_count += 1;
    
    if( DBG_flags & DBG_FLAG_ble_cmd )
    {
        dbg_str("iop-cmd\n");
        dbg_memdump8( 0, (void *)(&iop_globals.cmd_from_host), 64 );
    }
    
    pMjCmdDisp = &MajorCmdsTbl[0];
    
   
    while(pMjCmdDisp->handler != NULL)
    {
        if( pMjCmdDisp->value != GET_TOPIC_MAJOR(pInMsgData->topicId))
        {
            pMjCmdDisp++;
            continue;
        } 
        else
        {
            break;
        }
    }
    
    if( pMjCmdDisp->handler )
    {
        (*(pMjCmdDisp->handler))(pIoMsgData);
    }
    else
    {
        dbg_str("err-unknown-cmd\n");
        set_sys_error( SYS_ERR_ENOTSUP,iop_globals.cmd_from_host.cmd  );
    }
    
    if( DBG_flags & DBG_FLAG_ble_cmd )
    {
        dbg_putc('\n');
    
        if( iop_globals.rsp_to_host.len == 0 )
        {
            dbg_str("iop-no-rsp\n");
        }
        else
        {
            dbg_str("iop-cmd: response\n");
            dbg_memdump8( 0, &(iop_globals.rsp_to_host), 64 );
        }
    }
    
}
