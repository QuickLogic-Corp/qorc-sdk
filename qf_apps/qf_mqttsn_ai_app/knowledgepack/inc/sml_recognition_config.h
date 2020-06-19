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

#ifndef __SENSIML_SENSOR_CONFIG_H__
#define __SENSIML_SENSOR_CONFIG_H__

#include <stdint.h>
#include "dcl_commands.h"
#include "sensor_config.h"

#define SENSIML_CFG_ARRAY(...) {__VA_ARGS__}

#define SENSIML_SENSOR_IMU_MSG(SENSOR_ID, RATE, RANGE) \
{ \
    .msg_type=(uint8_t)(GET_TOPIC_MINOR(TOPIC_SENSOR_ADD)), \
    .sensor_common.sensor_id=(uint32_t)SENSOR_ID,\
    .sensor_common.rate_hz=(uint32_t)RATE, \
    .unpacked.imu_config.sensor1_range =(uint8_t)RANGE \
}

#define SENSIML_SENSOR_IMU_2_MSG(SENSOR_ID, RATE, RANGE1, RANGE2) \
{ \
    .msg_type=(uint8_t)GET_TOPIC_MINOR(TOPIC_SENSOR_ADD), \
    .sensor_common.sensor_id=(uint32_t)SENSOR_ID,\
    .sensor_common.rate_hz=(uint32_t)RATE, \
    .unpacked.imu_config.sensor1_range =(uint8_t)RANGE1, \
    .unpacked.imu_config.sensor2_range =(uint8_t)RANGE2 \
}

#define SENSIML_SENSOR_AUDIO_MSG(ID, RATE, NBITS, MIC_CFG) \
{ \
    .msg_type=(uint8_t)(GET_TOPIC_MINOR(TOPIC_SENSOR_ADD)), \
    .unpacked.audio.sensor_common.sensor_id=(uint32_t)ID,\
    .unpacked.audio.sensor_common.rate_hz=(uint32_t)RATE, \
    .unpacked.audio.nbits=(uint8_t)NBITS, \
    .unpacked.audio.mic_config = MIC_CFG \
}

#define SENSIML_SENSOR_ADC_MSG(ID, RATE, CHANNEL_CFG) \
{ \
    .msg_type=(uint8_t)(GET_TOPIC_MINOR(TOPIC_SENSOR_ADD)), \
    .unpacked.ltc1859_a.common.sensor_id=(uint32_t)ID,\
    .unpacked.ltc1859_a.common.rate_hz=(uint32_t)RATE, \
    .unpacked.ltc1859_a.chnl_config = CHANNEL_CFG \
}

#define SENSIML_SENSOR_CLEAR_MSG \
{ \
    .msg_type=(uint8_t)(GET_TOPIC_MINOR(TOPIC_SENSOR_CLEAR))\
}
#define SENSIML_SENSOR_CFG_DONE_MSG \
{ \
    .msg_type=(uint8_t)(GET_TOPIC_MINOR(TOPIC_SENSOR_DONE))\
}

#define SML_DEVICE_COMMAND_NUM_MSGS 5

const struct sensor_config_msg recognition_config[] = {
        SENSOR_CONFIG_CLEAR_MSG(),
        SENSIML_SENSOR_IMU_2_MSG(SENSOR_ENG_VALUE_ACCEL_GYRO,104,20,0),
        //SENSOR_CONFIG_IMU_MSG(SENSOR_ENG_VALUE_ACCEL,104,0x14),
        //SENSOR_CONFIG_IMU_MSG(SENSOR_ENG_VALUE_GYRO,104,0),
        //SENSOR_CONFIG_AUDIO_MSG(SENSOR_AUDIO,16000, 16, SENSOR_CONFIG_ARRAY(0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00)),
        SENSOR_CONFIG_DONE_MSG()
};

#endif //__SENSIML_SENSOR_CONFIG_H__
