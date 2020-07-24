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
//#include "Recognition.h"
#include "dbg_uart.h"
#include "DataCollection.h"

#if S3AI_FIRMWARE_IS_COLLECTION

#define MIN_PREFIX_LENGTH 1

/* dispatch entry */
struct collect_cmd_dispatch_entry {
    uint32_t value;
    void (*handler)(Mqttsn_IOMsgData_t *);
    uint32_t expPayldLength;
    uint8_t isVariableLen;    
};

void app_datastorage_set_filename(const char *pFileName)
{
    /* this is the prefix for the filenames we create */
    strncpy( file_save_config.cur_filename_template,
             pFileName,
             sizeof(file_save_config.cur_filename_template)-1 );
    /* force terminate */
    file_save_config.cur_filename_template[sizeof(file_save_config.cur_filename_template)-1]=0;
    
}


void do_set_collect_prefix( Mqttsn_IOMsgData_t *pIoMsgData )
{
    uint8_t *pStorageLocation = "/default/";
    uint32_t storageLocLen = strlen((const char *)pStorageLocation);
    uint8_t *pPrefix = pIoMsgData->incomingMsg.pMsgPayldBuf;
    uint8_t *pFilename = NULL;
    
    if((strncmp("/", (const char *)pPrefix, 1) == 0))
    {
        if(strncmp((const char *)pStorageLocation, (const char *)pPrefix, storageLocLen) == 0)
        {
            pFilename =  pPrefix + storageLocLen;
        }
        else
        {
            //anything other than /default/ is not supported
            printf("anything other than /default/ is not supported\n");
        }
    }
    else
    {
        pFilename =  pPrefix;
    }
    
    if(pFilename != NULL)
    {
        app_datastorage_set_filename((char const *)pFilename);
    }

}

void do_collect_stop(Mqttsn_IOMsgData_t *pIoMsgData)
{
    app_datastorage_start_stop(0, NULL, 0 );
}

void do_collect_start(Mqttsn_IOMsgData_t *pIoMsgData)
{
    /* guids are 16 bytes */
    app_datastorage_start_stop(1, pIoMsgData->incomingMsg.pMsgPayldBuf, GUID_BYTES_MAX);
   
}

void Collect_AllStop(void)
{
    iop_globals.cur_status.bit_flags &= ~COLLECT_ACTIVE;
}

/* collect commands  */
static struct collect_cmd_dispatch_entry const ccde_table[] = {
    { .value = TOPIC_COLLECT_PREFIX_SET, do_set_collect_prefix, .expPayldLength = MIN_PREFIX_LENGTH, .isVariableLen = 1 },
    { .value = TOPIC_COLLECT_START, do_collect_start, .expPayldLength = 16, .isVariableLen = 0 },
    { .value = TOPIC_COLLECT_STOP, do_collect_stop, .expPayldLength = 0, .isVariableLen = 0 },
    /* terminate */
    { .value = -1, .handler = NULL }
};

// process the Recognition minor command
 void run_collect_cmd(Mqttsn_IOMsgData_t *pIoMsgData )
{
    Mqttsn_MsgData_t *pInMsg = &pIoMsgData->incomingMsg;
    
    //if its a PUBACK, we have no use for it here just return
    if(pInMsg->msgType == MQTTSN_PUBACK)
        return;
        
    const struct collect_cmd_dispatch_entry *pCCDE;
    
    for( pCCDE = ccde_table; pCCDE->handler != NULL ; pCCDE++ )
    {
        if( pCCDE->value == pInMsg->topicId)
        {
            uint32_t payldErr = CheckPayloadValidity(&pIoMsgData->incomingMsg, 
                                                     pCCDE->expPayldLength, 
                                                     pCCDE->isVariableLen);
            
            if(!payldErr)
            {            
                (*(pCCDE->handler))(pIoMsgData);
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

#endif //S3AI_FIRMWARE_IS_COLLECTION
