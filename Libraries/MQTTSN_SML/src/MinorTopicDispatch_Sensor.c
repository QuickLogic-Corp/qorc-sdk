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
*    File   : MinorTopicDispatch_Sensor.c
*    Purpose: Sensor minor topic dispatch
*                                                          
*=========================================================*/
#include "Fw_global_config.h"
#include <stdio.h>
#include <string.h>
#include "dcl_commands.h"
#include "iop_messages.h"
#include "stdarg.h"
#include "dbg_uart.h"
#include "ql_time.h"
#include "DataCollection.h"
#include "Sensor_Attributes.h"
#include "sensor_config.h"
#include "Mqttsn_Topics.h"
#include "micro_tick64.h"

/*
 * This file is the "dispatch" for all SENSOR commands.
 * The MAJOR item SENSOR has already be dispatched.
 * Here, we are handling the MINOR sensor commands.
 */
#if S3AI_FIRMWARE_IS_COLLECTION
/* return list of supported sensors */
static void get_list(Mqttsn_IOMsgData_t *pIoMsgData)
{
    //reply with a Publish payload
    int result = 0;
    Mqttsn_MsgData_t *pOutMsgData = &pIoMsgData->outgoingResponse;

    result = Mqttsn_AllocPublishPayload(MQTTSN_BUFFER_SMALL, &pOutMsgData->pMsgPayldBuf,
                                &pOutMsgData->allocLen);

    if(result)
    {
        uint8_t *pPayld = pOutMsgData->pMsgPayldBuf;

        pOutMsgData->topicId = TOPIC_SENSOR_LIST_RSP;

        // TODO Until we have generic sensor list struct where we will consolidate
        // all sensor related information, we return list of sensors supported for
        // chilkat (hard coded). New list will include sensor_live_t beside others.
        // We will iterate through the list with is_sensor_available() and add sensors.
#if IMU_DRIVERS
        pOutMsgData->payldLen = Mqttsn_BuffWr_u32(&pPayld, SENSOR_ENG_VALUE_ACCEL_GYRO);
        pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, SENSOR_ENG_VALUE_ACCEL);
        pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, SENSOR_ENG_VALUE_GYRO);
        /* mag sensor is not enabled yet */
        //pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, SENSOR_ENG_VALUE_MAGNETOMETER);
#endif

#if AUDIO_DRIVER
        pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, SENSOR_AUDIO);
#endif

#if LTC1859_DRIVER
        pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, SENSOR_ADC_LTC_1859_MAYHEW);
#endif
#if AD7476_FPGA_DRIVER
        pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, SENSOR_ADC_AD7476);
#endif
    }
}

/* dispatch entry */
struct sensor_cmd_dispatch_entry {
    uint32_t value;
    void (*handler)(Mqttsn_IOMsgData_t *);
    uint32_t expPayldLength;
    uint8_t isVariableLen;
};

static void wrap_sensor_clear(Mqttsn_IOMsgData_t *pIoMsgData)
{
    sensor_clear( &datacollection_sensor_status );
    sensors_all_startstop(0);
}

static void wrap_sensor_add( Mqttsn_IOMsgData_t *pIoMsgData )
{
    uint8_t *pInBuff = pIoMsgData->incomingMsg.pMsgPayldBuf;

    sensor_config_msg.sensor_common.sensor_id = Mqttsn_BuffRead_u32(&pInBuff);
    sensor_config_msg.sensor_common.rate_hz = Mqttsn_BuffRead_u32(&pInBuff);

    memcpy((void *)&sensor_config_msg.unpacked, pInBuff, pIoMsgData->incomingMsg.msgLen-8);

    if (sensor_config_msg.sensor_common.sensor_id == SENSOR_ENG_VALUE_ACCEL)
    {
        // This is to supress gyro-range (sensor2_rage) check complain when we don't have setting for it.
        sensor_config_msg.unpacked.imu_config.sensor2_range = 0;
    }

    sensor_add( &datacollection_sensor_status  );
}

#define CONFIG_LENGTH_MIN  1    //IMU has 2 byte config value

extern void configure_all_sensors(Mqttsn_IOMsgData_t *pIoMsgData);
/* sensor commands we support - in a table */
static struct sensor_cmd_dispatch_entry const scde_table[] = {
    { .value = TOPIC_SENSOR_LIST_REQ, get_list, .expPayldLength = 0, .isVariableLen = 0 },
    { .value = TOPIC_SENSOR_CLEAR, wrap_sensor_clear, .expPayldLength = 0, .isVariableLen = 0 },
    { .value = TOPIC_SENSOR_ADD, wrap_sensor_add, .expPayldLength = 8+CONFIG_LENGTH_MIN, .isVariableLen = 1 },
    { .value = TOPIC_SENSOR_DONE, configure_all_sensors, .expPayldLength = 0, .isVariableLen = 0 },
    /* terminate */
    { .value = -1, .handler = NULL }
};


/* process the SENSOR major command
 * really all we do is dispatch the sub-command.
 */
void run_sensor_cmd(Mqttsn_IOMsgData_t *pIoMsgData )
{
    Mqttsn_MsgData_t *pInMsg = &pIoMsgData->incomingMsg;
    
    const struct sensor_cmd_dispatch_entry *pSCDE;
    
    //if its a PUBACK, we have no use for it here just return
    if(pInMsg->msgType == MQTTSN_PUBACK)
        return;    
    
    sensor_config_msg.msg_type = GET_TOPIC_MINOR(pInMsg->topicId);
    
    for( pSCDE = scde_table; pSCDE->handler != NULL ; pSCDE++ )
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
    dbg_str("err-sensor-subcmd\n");
    set_sys_error( SYS_ERR_ENOTSUP,pInMsg->topicId );
}

#endif
