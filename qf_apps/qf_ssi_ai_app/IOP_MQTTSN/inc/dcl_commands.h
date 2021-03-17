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

#if !defined(DCL_COMMANDS_H)
#define DCL_COMMANDS_H

/*
* This file is *SHARED* between multiple tools.
* This file must be compileable using only "stdint.h"
*
* An edits to this file requires that all tools be updated.
* 
* Tools is a long list of things.
* Current list as of: Mar 4 2019
* ================================
* Tools =  All Embedded Proto Apps, all platforms.
* Tools =  BLE Interface from PC to Device.
* Tools =  SensiML DCL application (PC APP)
* Tools =  SensiML Android Application (Mobile App)
*/
#include <stdint.h>

typedef enum iop_sensor_config_minor_cmds {
    /**
    * disables all sensors
    *
    * Also "clears" configuration error flag.
    */
    IOP_COLLECTION_CLEAR_SENSORS = 0x00,
    
    /**
    * adds (enabling) a sensor
    * Sensor details are:
    *   32bit sensor_id
    *   32bit rate_hz
    *
    *  Followed by upto 8 bytes of 'sensor specific data'
    *
    * If configuration is invalid/not known/etc
    * then "error_number" is set to some NONZERO value (TBD)
    */
    IOP_COLLECTION_ADD_SENSOR = 0x01,
    
    /**
    * Set DateTime, Unix.
    */
    IOP_COLLECTION_SET_UNIX_DATETIME = 0x02,
    
    /**
    * Get supported sensors list.
    * Returns a 0 terminated list of supported sensors
    */
    IOP_COLLECTION_GET_SENSOR_LIST = 0x03,
    
    /**
    * Tells device all configuration is complete.
    * No payload
    *   response is 2 32bit values
    *     uint32_t error_number; // 0 means no error
    *     uint32_t extended_details; // more details if possible.
    *
    * This can be sent multiple times...
    * error is cleared by sending a clear sensor
    */
    IOP_COLLECTION_CONFIG_DONE = 0x04,
    
    /**
    * RESERVED VALUES BELOW THIS LINE.
    * These are used by the BLE implimenation
    */
    IOP_COLLECTION_RESERVED_1 = 0xF0,
    IOP_COLLECTION_RESERVED_2 = 0xF1,
    IOP_COLLECTION_RESERVED_3 = 0xF2,
    IOP_COLLECTION_RESERVED_4 = 0xF3,
    IOP_COLLECTION_RESERVED_5 = 0xF4,
    IOP_COLLECTION_RESERVED_6 = 0xF5,
    IOP_COLLECTION_RESERVED_7 = 0xF6,
    //IOP_COLLECTION_RESERVED_8 = 0xF7,
    //IOP_COLLECTION_RESERVED_9 = 0xF8,
    //IOP_COLLECTION_RESERVED_10 = 0xF9,
    //IOP_COLLECTION_RESERVED_11 = 0xFA,
    
    //taken from SensorTile Project
    DCL_COMMAND_CLEAR_ERR_BIT = 0xF7,
    DCL_COMMAND_GET_SENSOR_LIST_RSP = 0xF8,
    DCL_COMMAND_RESERVED_DCL_GET_VERSION = 0xF9,
    DCL_COMMAND_RESERVED_DCL_GET_COMP_DT = 0xFA,

    /**
    *  DCL -> SetStorage, translated in BLE
    */
    IOP_COLLECTION_RESERVED_DCL_SET_STORAGE = 0xFB,
    /**
    * DCL -> GetStatus, translated in BLE
    */
    IOP_COLLECTION_RESERVED_DCL_GET_STATUS = 0xFC,
    /**
    * DCL -> RecordDataStart, translated in BLE
    */
    IOP_COLLECTION_RESERVED_DCL_RECORD_START = 0xFD,
    /**
    * DCL -> RecordDataStop, translated in BLE
    */
    IOP_COLLECTION_RESERVED_DCL_RECORD_STOP = 0xFE,
    
    IOP_UNKNOWN_COMMAND = 0xFF,
    
    /* 8bit enum */
} sensor_config_cmd_t;

#define SENSOR_MAKE_ID_32BIT(A,B,C,D)  \
      (((A) << 24) + ((B)<<16) + ((C)<<8) + ((D)<<0))


/* About SENSOR_ENG_VALUE_IMU_agm - it is only used in "get list of supported sensors".
* and when the AGM sensors are ganged
*
* The low 8bits represents which sensor is present (set=1) or not present (clear=0)
* bits 0 - accel
* bits 1 - gyro
* bits 2 - magnetometer
*/
#define SENSOR_ENG_VALUE_IMU_agm_base           SENSOR_MAKE_ID_32BIT( 'I', 'M', 'U',  0 )
#define SENSOR_ENG_VALUE_IMU_agm_a_bit          (1)
#define SENSOR_ENG_VALUE_IMU_agm_g_bit          (2)
#define SENSOR_ENG_VALUE_IMU_agm_m_bit          (4)
#define SENSOR_ENG_VALUE_ACCEL                  SENSOR_MAKE_ID_32BIT( 'I', 'M', 'U', 'A' )
#define SENSOR_ENG_VALUE_GYRO                   SENSOR_MAKE_ID_32BIT( 'I', 'M', 'U', 'G' )
#define SENSOR_ENG_VALUE_ACCEL_GYRO             (SENSOR_ENG_VALUE_IMU_agm_base + SENSOR_ENG_VALUE_IMU_agm_a_bit + SENSOR_ENG_VALUE_IMU_agm_g_bit)
#define SENSOR_ENG_VALUE_MAGNETOMETER           SENSOR_MAKE_ID_32BIT( 'I', 'M', 'U', 'M' )
/* audio is 8 or 16 bit data, in a continous stream of sample values
 * data may be monoral, or stereo - etc (1 channel, or 2 channel)
 * and comes at some configured rate, ie: 8khz or 16khz or ... ??hz
 */
#define SENSOR_AUDIO                            SENSOR_MAKE_ID_32BIT( 'A', 'U', 'D', 'O' )

/* mayhewe board */
#define SENSOR_ADC_LTC_1859_MAYHEW              SENSOR_MAKE_ID_32BIT( 'L', 'T', 0, (1859*10)+0 )
/* some future same chip, different config */
#define SENSOR_ADC_LTC_1859_B                   SENSOR_MAKE_ID_32BIT( 'L', 'T', 0, (1859*10)+1 )
/* Analog Devices AD7476 */
#define SENSOR_ADC_AD7476                       SENSOR_MAKE_ID_32BIT( 'A','D', 0, (7476))

// This needs to be updated when new sensor is added.
#define SENSOR_NUM_MAX  9
#define SENSOR_RESERVED 0xFFFF

  /* SET TIME command */
typedef struct
{
    uint32_t  unix_time_seconds;
} sensor_datetime_t;

// first two values in all sensor add commands
typedef struct{
    uint32_t   sensor_id;
    uint32_t   rate_hz;  /* rate = 0 is disabled */
} sensor_common_t;

// IMU sensor
typedef struct{
    /**
    * Range * 10,
    *   ie: 20 = 2g, 40 = 4g, 5 = 0.5 g
    */
    uint8_t sensor1_range, sensor2_range;
} sensor_imu_t;



/**
* @brief audio sensor config
* Think of this as a microphone
*/
typedef struct {
    /* data format is an array of u16 values, no compression
    * Rate is in the rate_hz field.
    *
    * To configure mono, set mic_config[0] = 1
    * To configure stereo set both [0] and [1] to 1
    *
    * Normally only 0, 1, or 2 mics are supported.
    * Some platforms may have 3 or 4 mics
    * Rarely do we expect to see more then 4
    *
    * not all sample rates are supported.
    * not all mic configurations are supported.
    *
    * mic config = 0 is disabled.
    * mic config = 1 is "normal default" operation.
    *
    * If a MIC supports other ranges ... ie: pre-amp
    * then that is different value (not 1, and not 0)
    */
    uint8_t nbits; /* 8 or 16 */
    /* While S3 supports 4
     * the packet supports 8 (future chips) 
     */
    uint8_t mic_config[8];
} sensor_audio_t;

/**
* @brief config structure for LTC1859 chip
*
* Values are from the LTC1859 data sheet.
* see: https://www.analog.com/media/en/technical-documentation/data-sheets/185789fb.pdf
* Page: 15, "INPUT DATA WORD", bits [7:0] are used, [15:8] unused.
* Thus, bit[7] maps to the SGL/DIFF bit, and bit[0] is the sleep bit.
*/

typedef struct {
    uint8_t chnl_config[8]; /* 8bytes */
} sensor_adc_ltc1859_a_t;

typedef struct {
    //sensor_common_t common; /* 8bytes */
    uint32_t param0;
    uint32_t param1;
} sensor_adc_ad7476_t;

/**
* @brief - Get supported Sensors Response.
*/
typedef struct {
    uint32_t sensor_list[4];
} sensor_list_t;

/* a command had an error, we return an error number
 * followed by the first 18 bytes of the command
 * why 18? Because the BLE packet is limited to 20 bytes.
 * We have the ERROR reply byte
 * followed by the error number
 * And 18bytes remain in the packet
 */
typedef struct {
    uint8_t error_number;
    uint8_t first_18_bytes[18];
} sensor_error_t;


/**
* @brief BLE Sensor Config Commands
*
* All packets are 20 bytes (BLE limitation), 
* byte0 is MAJOR message type.
* For sensor config, it is always: IOP_MSG_SENSOR_CONFIG
*
* byte1 - is a MINOR message type, this code cares about this value.
*
* Rather then decorate structures with pack & unpack
* we use a few unions and byte copy the structures.
*
*/

struct sensor_config_msg {
    uint8_t msg_type; /* really: sensor_config_cmd_t */
    sensor_common_t        sensor_common;
    
    union sensor_union {
        uint8_t                as_u8[ 32 ];
        sensor_imu_t           imu_config;
        //TODO will define types for accel,gyro,mag separately.
        sensor_adc_ltc1859_a_t ltc1859_a;
        sensor_adc_ad7476_t    ad7476;
        sensor_audio_t         audio;
    } unpacked;
    
};

/* for the embedded side */
extern struct sensor_config_msg sensor_config_msg;
extern const struct sensor_config_msg datacapture_default_config[];


#endif /* #define DCL_COMMANDS_H */
