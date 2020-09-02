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
*    File   : MinorTopicDispatch_Livestream.c
*    Purpose: Livestream minor topic dispatch
*                                                          
*=========================================================*/
#include "Fw_global_config.h"
#include <string.h>
#include "dcl_commands.h"
#include "DataCollection.h"
#include "iop_messages.h"
#include "Mqttsn_MessageHandler.h"
#include "Mqttsn_Topics.h"
#include "dbg_uart.h"
#include "sensor_live.h"
#if LTC1859_DRIVER
#include "ql_adcTask.h"
#endif
#include "Recognition.h"
#include "micro_tick64.h"

int ble_motion_drop;

/* (Id,live,min,size,seq,cur,alloc,count,reload,msg) */
Sensor_live_t sensor_live_list[] = {
    SENSOR_LIVE_INIT(SENSOR_ENG_VALUE_ACCEL,0,1,6,0,0,0,0,0),
    SENSOR_LIVE_INIT(SENSOR_ENG_VALUE_GYRO,0,1,6,0,0,0,0,0),
    SENSOR_LIVE_INIT(SENSOR_ENG_VALUE_ACCEL_GYRO,0,1,12,0,0,0,0,0),
    //FIXME sample sizes of following sensors need to be douple checked.
    SENSOR_LIVE_INIT(SENSOR_ENG_VALUE_MAGNETOMETER,0,1,2,0,0,0,0,0),
    SENSOR_LIVE_INIT(SENSOR_AUDIO,0,1,2,0,0,0,0,0),
    SENSOR_LIVE_INIT(SENSOR_ADC_LTC_1859_MAYHEW,0,1,2,0,0,0,0,0),
    SENSOR_LIVE_INIT(SENSOR_ADC_LTC_1859_B,0,1,2,0,0,0,0,0),
    SENSOR_LIVE_INIT(SENSOR_ADC_AD7476,0,1,2,0,0,0,0,0),
    /* reserved for new sensor */
    // If new sensor needs to be added, add it before this line.
    // Modify SENSOR_NUM_MAX to include new sensor.
    // The last line should always be SENSOR_RESERVED
    SENSOR_LIVE_INIT(SENSOR_RESERVED,0,1,1,0,0,0,0,0)
};

/* dispatch entry */
struct livestream_cmd_dispatch_entry {
    uint32_t value;
    void (*handler)(Mqttsn_IOMsgData_t *);
    uint32_t expPayldLength;
    uint8_t isVariableLen;
};

/**
 * @brief find the sensor which matches the id
 * @param[IN] find_id : the sensor id in searching
 *
 * @return the sensor, otherwise NULL;
 **/
Sensor_live_t *sensor_live_get_by_id(uint32_t find_id)
{
    for (int i=0; i<SENSOR_NUM_MAX; i++)
    {
        if (SENSOR_LIVE_GET_ID(&sensor_live_list[i]) == find_id)
            return &sensor_live_list[i];
        else if (SENSOR_LIVE_GET_ID(&sensor_live_list[i]) == SENSOR_RESERVED)
            return NULL;
    }
    return NULL;
}

/**
 * @brief check if any sensor is active of live streaming
 * @return: FALSE/TUE - no sensor is active / at least one sensor is active
 **/
uint8_t sensor_live_any_enabled(void)
{
    for (int i=0; i<SENSOR_NUM_MAX; i++)
    {
        if (SENSOR_LIVE_GET_STATE(&sensor_live_list[i]))
            return 1;
       else if (SENSOR_LIVE_GET_ID(&sensor_live_list[i]) == SENSOR_RESERVED)
            return 0;
    }
    return 0;
}

/**
 * @brief Check if a imu sensor is active live streaming.
 * @param[IN] sensor_id : sensor to be checked.
 *
 * return : TRUE/FALSE - active/no-active
 **/
bool is_sensor_live_active(uint32_t sensor_id)
{
    bool yes = FALSE;
    uint32_t id;

    for (int i=0; i<SENSOR_NUM_MAX; i++)
    {
        id = SENSOR_LIVE_GET_ID(&sensor_live_list[i]);
        if (id == sensor_id)
        {
            if (sensor_live_list[i].live)
            {
                yes = TRUE;
                break;
            }
        }
    }
    return yes;
}

/**
 * @brief Set default values for sensor live streaming struct
 *        disable all supported sensors for live streaming.
 */
void sensor_live_set_default(void)
{
    Sensor_live_t *s = &sensor_live_list[0];

    do {
        /* sample size */
        switch (s->sensor_id)
        {
            case SENSOR_ENG_VALUE_ACCEL:
            case SENSOR_ENG_VALUE_GYRO:
                SENSOR_LIVE_SET_SIZE(s,6);
                break;
            case SENSOR_ENG_VALUE_ACCEL_GYRO:
                SENSOR_LIVE_SET_SIZE(s,12);
                break;
            //TODO need to revisit default sample sizes for sensors below
            case SENSOR_ENG_VALUE_MAGNETOMETER:
            case SENSOR_AUDIO:
            case SENSOR_ADC_LTC_1859_MAYHEW:
            case SENSOR_ADC_AD7476:
            case SENSOR_ADC_LTC_1859_B:
                SENSOR_LIVE_SET_SIZE(s,2);
                break;
        }

        /* common */
        SENSOR_LIVE_DISABLE(s);
        SENSOR_LIVE_SET_MIN(s,1);
        SENSOR_LIVE_CLR_SEQ(s);
        SENSOR_LIVE_SET_CUR(s,0);
        SENSOR_LIVE_SET_COUNT(s,0);
        SENSOR_LIVE_SET_RELOAD(s,0);
	//FIXME zero msg should only be done if allocated == 0
	//otherwise do it in ble_send() 
        if (!SENSOR_LIVE_GET_ALLOC(s))
           memset((void *)&s->msg, 0, sizeof(Mqttsn_MsgData_t));

        s++;
    } while (s->sensor_id != SENSOR_RESERVED);

    iop_globals.cur_status.bit_flags &= ~LIVESTREAM_ACTIVE;
}

/**
 * @brief Disable all supported sensors for live streaming,
 *        without change sensor's configurations.
 */
void sensor_live_stop_only(void)
{
    Sensor_live_t *s = &sensor_live_list[0];

    do {
        /* common */
        SENSOR_LIVE_DISABLE(s);
        //SENSOR_LIVE_CLR_SEQ(s);
        SENSOR_LIVE_SET_CUR(s,0);
        // zero msg should only be done if allocated == 0
        // otherwise do it in ble_send() 
        if (!SENSOR_LIVE_GET_ALLOC(s))
           memset((void *)&s->msg, 0, sizeof(Mqttsn_MsgData_t));

        s++;
    } while (s->sensor_id != SENSOR_RESERVED);

    iop_globals.cur_status.bit_flags &= ~LIVESTREAM_ACTIVE;
}

/**
 * @brief see sensor_live_set_default()
 **/
void sensor_live_stop_all(void)
{
    sensor_live_set_default();
}

/**
 * @brief receive from MqttsnApp_InMsgQ (inqueued by cli or fpga-uart) and enable/disable live streaming.
 * @param[IN] pIoMsgData : Pointer to IO packet buffer. Payload has information for each sensor listed.
 *
 * TOPIC_LIVE_START payload format:
 *     count (1 byte)       : number of sensors will be started
 *     sensorID 1 (4 bytes) : sensor id 1
 *     samples    (1 byte)  : minimal number of samples from sensor ID 1
 *     ......
 *     sensorID N (4 bytes) : sensor id N
 *     samples    (1 byte)  : minimal number of samples from sensor ID N
 **/
void do_livestream_start(Mqttsn_IOMsgData_t *pIoMsgData)
{
    bool error = FALSE;
    uint8_t count;
    uint32_t sensor_id;
    Sensor_live_t *sensor;
    uint8_t *pInBuff = pIoMsgData->incomingMsg.pMsgPayldBuf;

    count = Mqttsn_BuffRead_u8(&pInBuff);
    if (count == 0)
    {
        printf("Sensor Rate Req: empty input sensor !\n");
        return;
    } else if (count>SENSOR_NUM_MAX)
    {
        // Strip off sensors which are not defined
        count = SENSOR_NUM_MAX;
        printf("The number of sensors exceeds allowed %d !\n",SENSOR_NUM_MAX);
    }

    do {
        count--;
        sensor_id = Mqttsn_BuffRead_u32(&pInBuff);
        sensor = sensor_live_get_by_id(sensor_id);

        if (sensor)
        {
            if ( !is_sensor_active(sensor_id, IMU_DATA_COLLECT) )
            {
                error = TRUE;
                break;
            }
            // Turn on this sensor's live state
            SENSOR_LIVE_ENABLE(sensor);

            // read & set samples
            SENSOR_LIVE_SET_MIN(sensor,Mqttsn_BuffRead_u8(&pInBuff));
        }
        else {
            // read & flush cmd,min for this sensor id
            sensor_id = Mqttsn_BuffRead_u8(&pInBuff);
            printf("sensore Id: 0x%X is invalid !\n", sensor_id);
            error = TRUE;
            break;
        }
    } while (count !=0);

    if (!error)
    {
        // The bit LIVESTREAM_ACTIVE is globel, each individual sensor has its own state
        if ( sensor_live_any_enabled() )
        {
            iop_globals.cur_status.bit_flags |= LIVESTREAM_ACTIVE;
        } else {
            iop_globals.cur_status.bit_flags &= ~LIVESTREAM_ACTIVE;
        }
    } else {
        sensor_live_stop_all();
        configASSERT(0);
    }
}

/**
 * @brief Disable live streaming for all sensors.
 */
void do_livestream_stop(Mqttsn_IOMsgData_t *pIoMsgData)
{
    sensor_live_stop_only();
}

/**
 * @brief receive from MqttsnApp_IOMsgQ (inqueued by cli or fpga-uart),
 *        device will RSP list of active virtual sensors w/ sampling rate
 * @param[in] pIoMsgData : pointer to IO packet buffer.
 *
 * TOPIC_LIVE_SENSOR_LIST_RSP payload format:
 *     n_sensor (1 byte)       : number of active virtual sensors (active means sensor is added/configured for sampling).
 *     sensorID 1 (4 bytes)    : sensor id 1
 *     sampling rate (4 byte)  : sampling rate of sensor ID 1
 *     ......
 *     sensorID N (4 bytes)    : sensor id N
 *     sampling rate (4 byte)  : sampling rate of sensor ID N
 **/
void do_livestream_sensor_list_req(Mqttsn_IOMsgData_t *pIoMsgData)
{
    uint8_t sensor_m = 0;
    uint32_t result = 0;
    IMU_VIRTUAL_SENSOR_t vs = sensor_get_virtual_sensor();

    Mqttsn_MsgData_t *pOutMsgData = &pIoMsgData->outgoingResponse;

    result = Mqttsn_AllocPublishPayload(MQTTSN_BUFFER_SMALL, &pOutMsgData->pMsgPayldBuf,
                                &pOutMsgData->allocLen);
    if(result)
    {
        uint32_t id, id1, rate_hz, rate1_hz;
        uint8_t *pPayld = pOutMsgData->pMsgPayldBuf;

        pOutMsgData->topicId = TOPIC_LIVE_SENSOR_LIST_RSP;

        // Check imu sensors
        if ( is_sensor_active(SENSOR_ENG_VALUE_ACCEL_GYRO, IMU_DATA_COLLECT) )
        {
            if (vs == IMU_V_SENSOR_A_G_COM)
            {
                sensor_m = 3;
                id = SENSOR_ENG_VALUE_ACCEL_GYRO;
                rate_hz = sensor_get_sampling_rate(id); // in ACCEL_GYRO accel & gyro should have the same rate.
            } else { // IMU_V_SENSOR_A_G_SEP
                sensor_m = 4;
                id = SENSOR_ENG_VALUE_ACCEL;
                rate_hz = sensor_get_sampling_rate(id);
                id1 = SENSOR_ENG_VALUE_GYRO;
                rate1_hz = sensor_get_sampling_rate(id1);
            }
        } else if ( is_sensor_active(SENSOR_ENG_VALUE_ACCEL, IMU_DATA_COLLECT) ) {
            sensor_m = 1;
            id = SENSOR_ENG_VALUE_ACCEL;
            rate_hz = sensor_get_sampling_rate(id);
        } else if ( is_sensor_active(SENSOR_ENG_VALUE_GYRO, IMU_DATA_COLLECT) ) {
            sensor_m = 2;
            id = SENSOR_ENG_VALUE_GYRO;
            rate_hz = sensor_get_sampling_rate(id);
        }

        pOutMsgData->payldLen = 0;

        // Write Id and sampling rate for the active imu sensor

        if ( sensor_m )
        {
            pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, id);
            pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, rate_hz);

            if ( sensor_m ==4 ) {
                pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, id1);
                pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, rate1_hz);
            }
        }

        if ( is_sensor_active(SENSOR_ENG_VALUE_MAGNETOMETER, IMU_DATA_COLLECT) )
        {
            pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, SENSOR_ENG_VALUE_MAGNETOMETER);
            pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, sensor_get_sampling_rate(SENSOR_ENG_VALUE_MAGNETOMETER));
        }

#if AUDIO_DRIVER
        if ( is_sensor_active(SENSOR_AUDIO, IMU_DATA_COLLECT) )
        {
            pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, SENSOR_AUDIO);
	    //rate for audio is set in hard_code_audio() as 16000
            pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, 16000);
        }
#endif
#if LTC1859_DRIVER
        if ( is_sensor_active(SENSOR_ADC_LTC_1859_MAYHEW, IMU_DATA_COLLECT) )
        {
            pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, SENSOR_ADC_LTC_1859_MAYHEW);
            pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, sensor_ltc1859_get_rate());
        }
#endif
#if AD7476_FPGA_DRIVER
        if ( is_sensor_active(SENSOR_ADC_AD7476, IMU_DATA_COLLECT) )
        {
            pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, SENSOR_ADC_AD7476);
            pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, 1000000);
        }
#endif
    }
}

/**
 * @brief receive from MqttsnApp_InMsgQ (inqueued by cli or fpga-uart) and set streaming rate for active sensors.
 * @param[IN] pIoMsgData : pointer to IO packet buffer.
 * @note : Rate for the pair of REQ/RSP is defined as count down number based on sensor's sampling rate.
 *         For example, if rate = 0, no count down, every sensor generated samples are streamed. if rate = 1
 *             every other samples of generated samples are streamed.
 *
 * TOPIC_LIVE_SET_RATE_REQ/RSP payload format:
 *     count (1 byte)       : number of active sensors. It is recommended that host get active sensors list
 *                            before set rate. This also implies, sensors given by the command are ACTIVE.
 *     sensorID 1 (4 bytes) : sensor id 1
 *     rate (4 byte)        : streaming rate of sensor ID 1
 *     ......
 *     sensorID N (4 bytes) : sensor id N
 *     rate (4 byte)        : streaming rate of sensor ID N
 **/
void do_livestream_set_rate_req(Mqttsn_IOMsgData_t *pIoMsgData)
{
    bool error = FALSE;
    uint8_t sensor_n,n,sensor_c=0;
    uint32_t sensor_id;
    Sensor_live_t *sensor;
    uint8_t *pInBuff = pIoMsgData->incomingMsg.pMsgPayldBuf;
    Mqttsn_MsgData_t *pOutMsgData = &pIoMsgData->outgoingResponse;
    uint8_t *pPayld;

    if ( !(Mqttsn_AllocPublishPayload(MQTTSN_BUFFER_SMALL, &pOutMsgData->pMsgPayldBuf,
                                &pOutMsgData->allocLen)) )
    {
        return;
    }

    pPayld = pOutMsgData->pMsgPayldBuf;

    pOutMsgData->topicId = TOPIC_LIVE_SET_RATE_RSP;

    sensor_n = Mqttsn_BuffRead_u8(&pInBuff);

    if (sensor_n == 0)
    {
        printf("Sensor Rate Req: empty input sensor !\n");
        return;
    } else if (sensor_n > SENSOR_NUM_MAX) {
        // Strip off sensors which are not defined
        sensor_n = SENSOR_NUM_MAX;
        //FIXME use right printf function
        printf("Sensor Rate Req: number of sensors exceeds allowed %d !\n",SENSOR_NUM_MAX);
    }

    // Write number of active sensors
    //pOutMsgData->payldLen = Mqttsn_BuffWr_u8(&pPayld, sensor_n);

    n = sensor_n;
    do {
        n--;
        sensor_id = Mqttsn_BuffRead_u32(&pInBuff);
        sensor = sensor_live_get_by_id(sensor_id);

        if (sensor)
        {
            if ( !is_sensor_active(sensor_id, IMU_DATA_COLLECT) )
            {
                error = TRUE;
                break;
            }
            // read & set sensor's streaming rate
            sensor->rate_reload = Mqttsn_BuffRead_u32(&pInBuff);
            // initialize rate_count
            sensor->rate_count = sensor->rate_reload;

            // write sensor_id to output buffer
            pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, sensor_id);
            // write rate to output buffer
            pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, sensor->rate_reload);
            sensor_c++;
        }
        else {
            printf("Sensor Rate Req, sensore Id: 0x%X is invalid !\n",sensor_id);
            //read & flush rate for this sensor
            Mqttsn_BuffRead_u32(&pInBuff);
            error = TRUE;
            break;
        }
    } while (n !=0);

    if (!error)
    {
        if (sensor_n != sensor_c)
        {
            // The actual number of valid sensors(sensor_c) is less than claimed(sensor_n)
            // we need to adjust number of sensors in payload, which is the 1st byte in payload.
            pPayld = pOutMsgData->pMsgPayldBuf;
            Mqttsn_BuffWr_u8(&pPayld, sensor_c);
        }
    } else {
	//TODO: It is kind of debug stage, will add assert(0); 
	//Late when we ready for customer release, probably will use sticky or send system erro to host.
        sensor_live_stop_all();
        configASSERT(0);
    }
}

#if S3AI_FIRMWARE_IS_RECOGNITION

void send_recognition_results(ble_pme_result_w_fv_t * results)
{
    int result;
    Mqttsn_MsgData_t msgData;
    Mqttsn_TopicInfo_t *pTpcInfo;
    uint16_t topicId;
    
    if( !(iop_globals.cur_status.bit_flags & (IOP_STATUS_BIT_reco|IOP_STATUS_BIT_reco_f)) ){
        return;
    }

    if( send_data_start() < 0 ){
        return;
    }
    
    // Check Count donw
    if (iop_globals.class_count)
    {
        iop_globals.class_count--;
        send_data_end();
        return;
    } else {
        iop_globals.class_count = iop_globals.class_rate;
        topicId = TOPIC_RESULT_CLASS_DATA;
    }
    
#if RECOG_VIA_BLE  
    if(iop_globals.cur_status.bit_flags & IOP_STATUS_BIT_reco_f)
    {
        my_ble_send( IOP_MSG_RECO_FEATURE_DATA,
                    sizeof(ble_pme_result_w_fv_t),
                    (void *)results);
    }
    else
    {
        my_ble_send( IOP_MSG_RECO_CLASSIFICATION_DATA,
                    sizeof(ble_pme_result_t), 
                    (void *)(results) );
    }

#endif   //#if RECOG_VIA_BLE       
   
    pTpcInfo = Mqttsn_GetTopicInfo(topicId);
    
    if((pTpcInfo)&&(pTpcInfo->status))
    {
        uint64_t class_result_time;
        Mqttsn_TxPublishErrorCodes_t publishErr;
        result = Mqttsn_AllocPublishPayload(MQTTSN_BUFFER_SMALL, &msgData.pMsgPayldBuf,
                                      &msgData.allocLen);
        if(result)
        {
            uint8_t *pPayld = msgData.pMsgPayldBuf;
            msgData.payldLen = 0;
            
            msgData.topicId = topicId;

            class_result_time = xTaskGet_uSecCount();

            msgData.payldLen += Mqttsn_BuffWr_u8(&pPayld, iop_globals.class_type);
            msgData.payldLen += Mqttsn_BuffWr_u64(&pPayld, class_result_time);

            msgData.payldLen += Mqttsn_BuffWr_u16(&pPayld, results->context);
            msgData.payldLen += Mqttsn_BuffWr_u16(&pPayld, results->classification);
            
            if(iop_globals.cur_status.bit_flags & IOP_STATUS_BIT_reco_f)
            {
                msgData.payldLen += Mqttsn_BuffWr_u8(&pPayld, results->fv_len);
                
                memcpy((void *)(pPayld), (const void *)results->feature_vector, 
                       sizeof(results->feature_vector));
                
                msgData.payldLen += sizeof(results->feature_vector);
            }
            
            publishErr = Mqttsn_SendPublish(&msgData, pTpcInfo);
        } else {
            configASSERT(0);
        }
    }
    else
    {
        //dbg_str("Not subscribed to TOPIC_RESULT_CLASS_DATA yet\n");
    }
    
    send_data_end();
}

// TOPIC_LIVE_CLASSIFICATION_START
void do_live_classification_start(Mqttsn_IOMsgData_t *pIoMsgData)
{
    //uint8_t type;
    uint8_t *pInBuff = pIoMsgData->incomingMsg.pMsgPayldBuf;

    iop_globals.class_type = Mqttsn_BuffRead_u8(&pInBuff);

    // Clear recognition flags
    iop_globals.cur_status.bit_flags &= ~(IOP_STATUS_BIT_reco | IOP_STATUS_BIT_reco_f);

    switch (iop_globals.class_type)
    {
        case CLASS_RESULT_ONLY:
            iop_globals.cur_status.bit_flags |= IOP_STATUS_BIT_reco;
            recognition_startstop( RECOG_CMD_RECOGNIZE_START );
            break;
        case CLASS_RESULT_FV:
            iop_globals.cur_status.bit_flags |= IOP_STATUS_BIT_reco_f;
            recognition_startstop(RECOG_CMD_RECOGNIZE_START_W_FV );
            break;
        default:
            break;
    }
}

// TOPIC_LIVE_CLASSIFICATION_STOP
void do_live_classification_stop(Mqttsn_IOMsgData_t *pIoMsgData)
{
    // Stop classifications result, vector feature
    iop_globals.cur_status.bit_flags &= ~(IOP_STATUS_BIT_reco | IOP_STATUS_BIT_reco_f);
    iop_globals.class_count = iop_globals.class_rate;
    recognition_startstop( RECOG_CMD_RECOGNIZE_STOP );
}

// TOPIC_LIVE_CLASSIFICATION_SET_RATE
void do_live_classification_set_rate(Mqttsn_IOMsgData_t *pIoMsgData)
{
    uint8_t *pInBuff = pIoMsgData->incomingMsg.pMsgPayldBuf;

    iop_globals.class_rate = Mqttsn_BuffRead_u8(&pInBuff);
    iop_globals.class_count = iop_globals.class_rate;
}
#endif
void Live_AllStop(void)
{
    //Stop all live data streaming
    iop_globals.cur_status.bit_flags &= ~IOP_STATUS_BIT_reco;
    iop_globals.cur_status.bit_flags &= ~IOP_STATUS_BIT_reco_f;
    iop_globals.cur_status.bit_flags &= ~LIVESTREAM_ACTIVE;
}


/* livestream commands we support - in a table */
static struct livestream_cmd_dispatch_entry const lcde_table[] = {
    { .value = TOPIC_LIVE_START, do_livestream_start, .expPayldLength = 6, .isVariableLen = 1 }, // 6=1(n_sensor)+4(sensor_id)+1(mim_samples)
    { .value = TOPIC_LIVE_STOP, do_livestream_stop, .expPayldLength = 0, .isVariableLen = 0 },
    { .value = TOPIC_LIVE_SENSOR_LIST_REQ, do_livestream_sensor_list_req, .expPayldLength = 0, .isVariableLen = 0 },
    { .value = TOPIC_LIVE_SET_RATE_REQ, do_livestream_set_rate_req, .expPayldLength = 9, .isVariableLen = 1 }, // 9=1(n_sensor)+4(sensor_id)+4(rate)
#if S3AI_FIRMWARE_IS_RECOGNITION    
    { .value = TOPIC_RESULT_CLASS_START, do_live_classification_start, .expPayldLength = 1, .isVariableLen = 0 },
    { .value = TOPIC_RESULT_CLASS_STOP, do_live_classification_stop, .expPayldLength = 0, .isVariableLen = 0 },
    { .value = TOPIC_RESULT_CLASS_SET_RATE, do_live_classification_set_rate, .expPayldLength = 1, .isVariableLen = 0 },
#endif    
    /* terminate */
    { .value = -1, .handler = NULL }
};


// Livestream minor topic dispatch
void run_livestream_cmd(Mqttsn_IOMsgData_t *pIoMsgData )
{
    Mqttsn_MsgData_t *pInMsg = &pIoMsgData->incomingMsg;
    
    const struct livestream_cmd_dispatch_entry *pLCDE;
    
    //if its a PUBACK, we have no use for it here just return
    if(pInMsg->msgType == MQTTSN_PUBACK)
        return;    
    
    for( pLCDE = lcde_table; pLCDE->handler != NULL ; pLCDE++ )
    {
        if( pLCDE->value == pInMsg->topicId )
        {
            uint32_t payldErr = CheckPayloadValidity(&pIoMsgData->incomingMsg, 
                                                     pLCDE->expPayldLength, 
                                                     pLCDE->isVariableLen);
            
            if(!payldErr)
            {            
                (*(pLCDE->handler))(pIoMsgData);
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
