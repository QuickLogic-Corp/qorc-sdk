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
*    File   : sensor_live.h
*    Purpose: Struct, name defines for sensor's live stream
*                                                          
*=========================================================*/
#if !defined(SENSOR_LIVE_H)
#define SENSOR_LIVE_H


#define SENSOR_LIVE_INIT(SENSOR, LIVE, MIN, SIZE, SEQ, CUR, ALLOC, COUNT, RELOAD) \
{ \
    .sensor_id = (uint32_t)SENSOR,\
    .live=(uint8_t)LIVE, \
    .sample_min=(uint8_t)MIN, \
    .sample_sz=(uint8_t)SIZE, \
    .seq=SEQ, \
    .samples_cur=CUR, \
    .allocated=ALLOC, \
    .rate_count=COUNT, \
    .rate_reload=RELOAD \
}

#define SENSOR_LIVE_ENABLE(SENSOR) \
( \
    SENSOR->live=(uint8_t)1 \
)

#define SENSOR_LIVE_DISABLE(SENSOR) \
( \
    SENSOR->live=(uint8_t)0 \
)

#define SENSOR_LIVE_SET_MIN(SENSOR, MIN) \
( \
    SENSOR->sample_min=(uint8_t)MIN \
)

#define SENSOR_LIVE_SET_SIZE(SENSOR, SIZE) \
( \
    SENSOR->sample_sz=(uint8_t)SIZE \
)

#define SENSOR_LIVE_CLR_SEQ(SENSOR) \
( \
    SENSOR->seq=(uint8_t)0 \
)

#define SENSOR_LIVE_INC_SEQ(SENSOR) \
( \
    SENSOR->seq +=(uint8_t)1 \
)

#define SENSOR_LIVE_SET_CUR(SENSOR, CUR) \
( \
    SENSOR->samples_cur =(uint8_t)CUR \
)

#define SENSOR_LIVE_INC_CUR(SENSOR) \
( \
    SENSOR->samples_cur +=(uint8_t)1 \
)

#define SENSOR_LIVE_SET_COUNT(SENSOR, COUNT) \
( \
    SENSOR->rate_count =(uint32_t)COUNT \
)

#define SENSOR_LIVE_DEC_COUNT(SENSOR) \
( \
    SENSOR->rate_count -= (uint32_t)1 \
)

#define SENSOR_LIVE_SET_RELOAD(SENSOR, RELOAD) \
( \
    SENSOR->rate_reload =(uint32_t)RELOAD \
)

#define SENSOR_LIVE_SET_ALLOC(SENSOR, ALLOC) \
( \
    SENSOR->allocated =(uint8_t)ALLOC \
)

#define SENSOR_LIVE_GET_ID(SENSOR) (SENSOR)->sensor_id
#define SENSOR_LIVE_GET_STATE(SENSOR) (SENSOR)->live
#define SENSOR_LIVE_GET_MIN(SENSOR) (SENSOR)->sample_min
#define SENSOR_LIVE_GET_SIZE(SENSOR) (SENSOR)->sample_sz
#define SENSOR_LIVE_GET_SEQ(SENSOR) (SENSOR)->seq
#define SENSOR_LIVE_GET_CUR(SENSOR) (SENSOR)->samples_cur
#define SENSOR_LIVE_GET_COUNT(SENSOR) (SENSOR)->rate_count
#define SENSOR_LIVE_GET_RELOAD(SENSOR) (SENSOR)->rate_reload
#define SENSOR_LIVE_GET_ALLOC(SENSOR) (SENSOR)->allocated

typedef struct Sensor_live
{
    uint32_t   sensor_id;
    uint8_t    live;        /* state of live streaming of this sensor. 0/1=disable/enable */
    uint8_t    sample_min;  /* minial number of samples per publish. It is subjuct to size
                               of mqttsn buffer. */
    uint8_t    sample_sz;   /* sample size in bytes, it usually the bytes of each read */
    uint8_t    seq;         /* sequence number for raw data */
    uint8_t    samples_cur; /* how many samples stored in msg.pMsgBuf toward sample_min */
    uint8_t    allocated;   /* message buffer allocated for publish samples of the sensor */
    uint32_t   rate_count;  /* is initialized with rate_reload, when reach 0, the sample will be
                               picked and saved to msg. cur_samples will increased by 1. */
    uint32_t   rate_reload; /* rate as set by TOPIC_LIVE_SET_RATE. It is to initial rate_count.
                               When 0, mean every sample. */
    Mqttsn_MsgData_t msg;   /* msg.pMsgBuf point to buffer to store live raw data. */
} Sensor_live_t;

extern Sensor_live_t sensor_live_list[];
extern Sensor_live_t *sensor_live_get_by_id(uint32_t find_id);
extern uint8_t sensor_live_any_enabled(void);
extern void sensor_live_stop_all(void);
#endif // SENSOR_LIVE_H
