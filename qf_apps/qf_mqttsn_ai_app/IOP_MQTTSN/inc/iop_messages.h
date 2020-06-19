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
 *    File   : iop_messages.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __IOP_MESSAGES_H__
#define __IOP_MESSAGES_H__

#if defined(__GNUC__)
#define __packed
#endif

#include <stdint.h>
#include "ble_collection_defs.h"
#include "ble_pme_defs.h"

#ifdef BLE_STACK_SUPPORT_REQD
#include "app_timer.h"
#endif
#include "Mqttsn_MessageHandler.h"
     
#if  0 // since defined in iop_msg_cmd_t  RECOG_VIA_BLE

//Old IOP messages
#define IOP_MSG_RECO_CLASSIFICATION_START   40
#define IOP_MSG_RECO_CLASSIFICATION_STOP    41
#define IOP_MSG_RECO_CLASSIFICATION_DATA    42 
    
#define IOP_MSG_RECO_FEATURES_START         43
#define IOP_MSG_RECO_FEATURES_STOP          44
#define IOP_MSG_RECO_FEATURE_DATA           45

#endif     


struct iop_raw_spi_message {
    uint8_t len;
    uint8_t cmd; /* one of iop_msg_cmd_t */
    uint8_t payload[126];
};

/*
* IMU data is in this form
*/
struct iop_xyz_data {
    uint16_t x, y, z;
};

struct iop_imu_data {
    /* this dummy value is here for alignment
    * and is special cased & removed later */
    uint8_t dummy_value;
    uint8_t n_values;
    struct iop_xyz_data motion[ 12 ];
};

struct iop_sensor_config {
    uint8_t sensor_cmd;
    uint8_t payload[19];
};


__packed struct iop_status {
    /* number of bytes written to usb/sdcard */
    uint32_t bytes_saved;
    
    /* bit values for status bit s*/
#define COLLECT_ACTIVE          0x0001 /* device is storing data to filesystem */
#define LIVESTREAM_ACTIVE       0x0002 /* sending IMU data to the BLE */
#define IOP_STATUS_BIT_reco     0x0008 /* recognition is enabled */
#define IOP_STATUS_BIT_reco_f   0x0010 /* recongition with features are enabled */
#define LIVESTREAM_OVERRUN      0x0080 /* BLE cannot keep up with the data, overrun error */
#define COLLECT_OVERRUN         0x0100 /* SD card cannot keep up, overr runrun error occured */
#define ANY_ERROR  0x0200 /* set if ANY error occured */
    uint32_t bit_flags;
    
    /* packet counters, always increasing - DCL can use this to monitor health of BLE connection */
    uint16_t rx_count;
    uint16_t tx_count;
    
    /* overrun counters, see the "oe" status bits above */
    uint16_t live_oe_count;
    uint16_t collect_oe_count;
    
    /* sticky error number, only saves first, until an ERROR clear occurs */
    uint8_t sticky_error_code;
    
    /* monotonic increasing error counter,
     * DCL can use this to determine if errors occured
     * and if more then 1 error occured */
    uint8_t error_count;
};

//TODO [Justin] : Change the name to result_class_type
enum iop_class_type {
    CLASS_RESULT_ONLY = 1,    // type (uint8_t) + current time (uint64_t) + model(uint16_t) index + class_result(uint16_t)
    CLASS_RESULT_FV   = 2,    // _ONLY + number of feature vectors (uint8_t) + vectors(number of VF, each VF is 1 byte)
    /* New type must be added before this line. Update CLASS_RESULT_MAX to be the last type. */
    CLASS_RESULT_MAX  = CLASS_RESULT_FV       //The device is busy and cannot do this    
};

/*
 * Some replies are simple, we use this structure for generic replies.
 * Example:  VERSIONSTRING reply uses this, as do a few others.
 */
struct iop_misc_reply {
    /* the command replying to */
    uint8_t cmd;
    union {
        /* data is a string or bytes */
        char    cbuf[100];
        uint8_t as_u8[100];
    } u;
};



typedef enum{
    SYS_ERR_NO_ERROR = 0,
    SYS_ERR_EPERM = 1,       //permission denied (ie: read only)
    SYS_ERR_ENOENT = 2,      //file not found
    SYS_ERR_EIO = 5,         //IO Error (the sdcard disappeared)
    SYS_ERR_EBUSY = 16,       //The device is busy and cannot do this    
    SYS_ERR_ENODEV = 19,      //That storage area does not exist, or that sensor does not exist.
    SYS_ERR_EINVAL = 22,      //something is invalid in the request (preferred over ERANGE)
    SYS_ERR_EFBIG = 27,       //file is too big
    SYS_ERR_EROFS = 30,       //read only file system    
    SYS_ERR_ETIMEDOUT = 60,  //Timeout
    SYS_ERR_ESTALE = 70,      //There is no transfer going on (invalid transaction id)
    SYS_ERR_ENOSYS = 78,     //Feature not implemented
    SYS_ERR_ENOMEDIUM = 85,    //No medium found
    SYS_ERR_ENOTSUP = 91      //The request is not supported
}sys_error_code_t;


/*
 * This is the task variables used by the IOP (ble command processing)
 */
#define IOP_MAX_BYTES 128 
struct iop_globals {
    struct iop_raw_spi_message  cmd_from_host;
    struct iop_raw_spi_message  rsp_to_host;
    struct iop_status           cur_status;
    
    int data_busy;  
    union iop_data_msg {
        uint8_t as_u8[IOP_MAX_BYTES];
        char    as_string[IOP_MAX_BYTES];
        uint32_t as_u32[ IOP_MAX_BYTES / 4 ];
        uint16_t as_u16[ IOP_MAX_BYTES / 2 ]; 
        struct iop_imu_data   imu_data;
	ble_pme_result_t pme_results;
	ble_pme_result_w_fv_t pme_fv_results;
    } u_data;
    

    union iop_aligned_msg {
        uint8_t as_u8[IOP_MAX_BYTES];
        char    as_string[IOP_MAX_BYTES];
        uint32_t as_u32[ IOP_MAX_BYTES / 4 ];
        uint16_t as_u16[ IOP_MAX_BYTES / 2 ]; 
        struct iop_misc_reply misc;
    } u_cmd, u_rsp;

    uint8_t class_type;
    uint8_t class_rate;
    uint8_t class_count;
};

extern struct iop_globals iop_globals;

/**
* @brief set this status error code.
*/
void set_sys_error( sys_error_code_t error_code, uint32_t more_info );

void iop_send_pme_results(ble_pme_result_w_fv_t * results);

void send_recognition_results(ble_pme_result_w_fv_t * results);

void iop_msg_parse(struct iop_raw_spi_message *pRawMsg);

//void app_datastorage_set_flags( uint32_t flags );
void run_system_cmd(Mqttsn_IOMsgData_t *pIoMsgData );
void run_livestream_cmd(Mqttsn_IOMsgData_t *pIoMsgData );
void run_recognition_cmd(Mqttsn_IOMsgData_t *pIoMsgData );
void run_storage_cmd(Mqttsn_IOMsgData_t *pIoMsgData );
void run_collect_cmd(Mqttsn_IOMsgData_t *pIoMsgData );
void run_sensor_cmd(Mqttsn_IOMsgData_t *pIoMsgData );
void PopulateRejectMsg(Mqttsn_IOMsgData_t *pIoMsgData, sys_error_code_t error);
void do_all_stop(Mqttsn_IOMsgData_t *pIoMsgData);
void Live_AllStop(void);
void Recogntion_AllStop(void);
void Collect_AllStop(void);
void Storage_AllStop(void);

uint32_t CheckPayloadValidity(Mqttsn_MsgData_t *pInputMsg, uint32_t expPayldLen, uint8_t isVariableLen);

int send_data_start(void);
void send_data_end(void);

void MajorTopicDispatch(Mqttsn_IOMsgData_t *pIoMsgData);
/**
* @brief concatinate a series of strings as a string reply
*
* usage:  iop_reply_string( "your", "name", "here", NULL );
*
* Results in: "your name here" 
*/
void iop_reply_string(uint8_t *pBuf,  const char *cp, ... );
void my_ble_send( int cmd, int len, const void *data );
// To get rid of SensorTIle errors
typedef enum{
    /* bidirectional packet type */
    IOP_MSG_GET_VERSION       = 0, 
    IOP_MSG_GET_COMPDATETIME  = 1,
    
    /* host asks device for model GUID */
    IOP_MSG_GET_MODEL_GUID   = 2,
    IOP_MSG_GET_STATUS       = 3,
    /* byte 0 is msg, byte 1 is the cmd the reply was for */
    IOP_MSG_MISC_REPLY = 4, /* device -> host */
    
    /* clear error status to zero */
    IOP_MSG_CLR_STATUS       = 5,
    
    /* 4 .. 10 unused */
    
    /* sending IMU data to the host */
    IOP_MSG_IMU_DATA_START      = 10, /* host ->device */
    IOP_MSG_IMU_DATA_STOP       = 11, /* host ->device */
    /* msg type for imu data to host */
    IOP_MSG_IMU_DATA            = 12, /* device -> host */
    
    /* select any sensor */
    IOP_MSG_SENSOR_SELECT_BY_ID = 20, /* host -> device, with sensor id, and channel */
    /* turn sensor on and off */
    IOP_MSG_SENSOR_START        = 21, /* host -> device */
    IOP_MSG_SENSOR_STOP         = 22, /* host -> device */
    /* msg type for selected sensor to host */
    IOP_MSG_SENSOR_DATA         = 23, /* device->host */
    /* 22 .. 29 unused */
    
	IOP_MSG_STORAGE_FILENAME = 30, /* host->device */
	IOP_MSG_STORAGE_CONFIG   = 31, /* host->device */
    IOP_MSG_STORAGE_START    = 32,
    IOP_MSG_STORAGE_STOP     = 33,
    
    IOP_MSG_RECO_CLASSIFICATION_START = 40,
    IOP_MSG_RECO_CLASSIFICATION_STOP = 41,
    IOP_MSG_RECO_CLASSIFICATION_DATA = 42, /* device -> host */
    
	IOP_MSG_RECO_FEATURES_START = 43,
	IOP_MSG_RECO_FEATURES_STOP = 44,
    IOP_MSG_RECO_FEATURE_DATA = 45, /* device -> host */
    
	IOP_MSG_RECO_SEGMENT_START = 46,
	IOP_MSG_RECO_SEGMENT_STOP = 47,
    IOP_MSG_RECO_SEGMENT_DATA = 48, /* device -> host */
    /* 38 39 not used */
    
    /* see dcl_commands */
    IOP_MSG_SENSOR_CONFIG = 50,
    IOP_MSG_SENSOR_REPLY  = 51, /* device -> host */

}iop_msg_cmd_t;

#if 0
enum iop_error_codes {
    
    /* all is well */
    IOP_ERR_OK                  = 0,
    
    /* an unknown command was sent */
    IOP_ERR_UNKNOWN_MAJOR_CMD       = 1,
    IOP_ERR_UNKNOWN_MINOR_CMD       = 2,
    
    /* no such sensor */
    IOP_ERR_UNKNOWN_SENSOR      = 3,
    
    /* sample rate is not supported */
    IOP_ERR_ILLEGAL_RATE        = 4,
    
    /* a parameter to this command/sensor is not supported */
    IOP_ERR_INVALID_PARAMETER   = 5,
    
    /* Combination of sensors, rates, and modes not supported */
    IOP_ERR_INVALID_CONFIG      = 6,
    
    /* something is wrong with the file system storage */
    IOP_ERR_FILESYSTEM          = 7,
    
};
#endif

#endif //__IOP_MESSAGES_H__


