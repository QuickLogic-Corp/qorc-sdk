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
 *    File   : Fw_global_config.h
 *    Purpose:
 *
 *=========================================================*/

#ifndef FW_GLOBAL_CONFIG_H_INCLUDED     /* Avoid multiple inclusion             */
#define FW_GLOBAL_CONFIG_H_INCLUDED

#include <stdint.h>

#if   (__IAR_SYSTEMS_ICC__)
#define PLACE_IN_SECTION(x) @x
#elif (__GNUC__)
#define PLACE_IN_SECTION(x) __attribute__((section(x)))
#else
#error "Unknown compiler"
#endif

#define ENABLE_VOICE_SOLUTION   1

#define FEATURE_CLI_DEBUG_INTERFACE  1
#define FEATURE_CLI_FILESYSTEM       0

#define S3AI_FIRMWARE_MODE_RECOGNITION   ('R')
#define S3AI_FIRMWARE_MODE_COLLECTION    ('C')
#define S3AI_FIRMWARE_MODE_none           0

/* Select the filesystem API to use */
#define USE_FREERTOS_FAT         1  ///< Set this to 1 to use FreeRTOS FAT filesystem (Merced default)
#define USE_FATFS                0  ///< Set this to 1 to use FATFs filesystem

/* use this flag to check the data block header */
/* can be used only iff the QL_XPORT_INCLUDE_HEADER is defined for Device */
#define TST_HEADER_VERIFY (1)


#if !defined(S3AI_FIRMWARE_MODE)
     /* allow for commandline define for automated builds on Linux */
/* There is not enough RAM to do both - collection & recognition, choose 1 */
//#define S3AI_FIRMWARE_MODE      S3AI_FIRMWARE_MODE_COLLECTION
#define S3AI_FIRMWARE_MODE   S3AI_FIRMWARE_MODE_RECOGNITION
// #define S3AI_FIRMWARE_MODE    S3AI_FIRMWARE_MODE_none
#endif


#define S3AI_FIRMWARE_IS_COLLECTION   (S3AI_FIRMWARE_MODE==S3AI_FIRMWARE_MODE_COLLECTION)
#define S3AI_FIRMWARE_IS_RECOGNITION  (S3AI_FIRMWARE_MODE==S3AI_FIRMWARE_MODE_RECOGNITION)
/* future may have other modes? */

#if S3AI_FIRMWARE_IS_COLLECTION
#define DATA_CAPTURE_BUFFER_SIZE_K_BYTES   20
#else
/*
 * In this case, Data collection is disabled
 *
 * Thus #define is "funny lookign" it is 3 english words.
 * If this macro is actually used, those 3 words will cause
 * a syntax error and code will not compile, that is the intent.
 */
#define DATA_CAPTURE_BUFFER_SIZE_K_BYTES  not enabled here
#endif


#if ( S3AI_FIRMWARE_IS_COLLECTION + S3AI_FIRMWARE_IS_RECOGNITION ) > 1
#error "S3AI does not have enough memory to support both at the same time"
#endif


#define uartHandlerUpdate(id,x)

#define FEATURE_FPGA_UART   0       // FPGA UART not present
#define FEATURE_USBSERIAL   0       // USBSERIAL port is present
#define USB_UART_CHECK_FIFO_CONNECT  FEATURE_USBSERIAL    // 1 for USB-serial

// Options for debug output -- use to set DEBUG_UART below
// #define UART_ID_DISABLED     0   // /dev/null */
// #define UART_ID_HW           1   // the hard UART on the S3
// #define UART_ID_SEMIHOST     2   // Write debug data to semihost
// #define UART_ID_FPGA         3   // second uart if part of FPGA
// #define UART_ID_BUFFER       4   // Write data to buffer
// #define UART_ID_SEMBUF       5   // Write data to semihost and buffer
// #define UART_ID_USBSERIAL    6   // Write data to USB serial port

#if (FEATURE_USBSERIAL == 1)
#define DEBUG_UART  UART_ID_HW  // Write data to USB serial port
#define UART_ID_CONSOLE UART_ID_HW
#define UART_ID_MQTTSN  UART_ID_USBSERIAL   
#else
#define DEBUG_UART  UART_ID_BUFFER  // Write data to USB serial port
#define UART_ID_CONSOLE UART_ID_DISABLED
#define UART_ID_MQTTSN  UART_ID_HW
#endif

#define USE_SEMIHOSTING     0       // 1 => use semihosting, 0 => use UART_ID_HW

#define SIZEOF_DBGBUFFER    2048    // Number of characters in circular debug buffer


#define DBG_flags_default 0 //  (DBG_FLAG_ble_cmd + DBG_FLAG_sensor_rate+DBG_FLAG_datasave_debug)
#define DBG_FLAGS_ENABLE 1
#if !DBG_FLAGS_ENABLE
#define DBG_flags 0
#else
extern uint32_t DBG_flags;
#endif

#define DBG_FLAG_recog_result   (0x00000001)
#define DBG_FLAG_q_drop         (0x00000002)
#define DBG_FLAG_ble            (0x00000004)
#define DBG_FLAG_ble_cmd        (0x00000008)
#define DBG_FLAG_ble_background (0x00000010)
#define DBG_FLAG_datasave_debug (0x00000020)
#define DBG_FLAG_ble_details    (0x00000040)
#define DBG_FLAG_data_collect   (0x00000080)
#define DBG_FLAG_sensor_rate    (0x00000100)
#define DBG_FLAG_ffe            (0x00000100)
#define DBG_FLAG_adc_task       (0x00000200)

extern const char *SOFTWARE_VERSION_STR;

#define UUID_TOTAL_BYTES     16
extern uint8_t DeviceClassUUID[UUID_TOTAL_BYTES]; 

extern int FPGA_FFE_LOADED;

#define ENABLE_PRINTF 1

/** Enable Host mode for Voice application, This standalone S3 mode */

#define HOST_VOICE  0


/** Define COMPANION_SENSOR macro to configure system in CO-PROCESSOR mode for double tap application */
//#define COMPANION_SENSOR

/** Define HOST_SENSOR macro to configure system in HOST mode  */
#define HOST_SENSOR 1

#if !defined(ENABLE_PRINTF)
#include <stdio.h> /* we require printf() to be defined first */
#define printf(x, ...) 	/* Uart is disabled in CO-PROCESSOR_VOICE for power numbers */
#endif

#define QL_LOG_INFO_150K(X,...)   printf(X,##__VA_ARGS__)
#define	QL_LOG_DBG_150K(X,...)	  printf(X,##__VA_ARGS__)
#define	QL_LOG_ERR_150K(X,...)	  printf(X,##__VA_ARGS__)
#define	QL_LOG_WARN_150K(X,...)	  printf(X,##__VA_ARGS__)
#define	QL_LOG_TEST_150K(X,...)	  printf(X,##__VA_ARGS__)


#define IMU_M4_DRIVERS     1  ///< enable IMU sensors (Accel, Gyro, ...)
                              ///< using M4 driver to probe and collect data from
                              ///< the sensors

#define IMU_FFE_DRIVERS    0  ///< option to enable IMU sensor data collection
                              ///< using onchip FFE (availble on EOS-S3 only)

#define IMU_DRIVERS        (IMU_M4_DRIVERS || IMU_FFE_DRIVERS)

#define USE_IMU_FIFO_MODE   (1)    ///< Use FIFO mode to read from Accelerometer device

/* enable via sw the FFE or disable it, TODO: Make this real instead of a hack */
#define SW_ENABLE_FFE   1
/* enable via sw the AUDIO or disable it, TODO: Make this real instead of a hack */
#define SW_ENABLE_AUDIO 0
/* enable the FFE or not, see SW_ENABLE_FFE only 1 should exist */
#define FFE_DRIVERS	(IMU_FFE_DRIVERS) // 1
//
///* do or do not perform dynamic frequency scaling */
#define CONST_FREQ (1)

// Enable ADC FPGA Driver
#define AD7476_FPGA_DRIVER   1

#if (FEATURE_USBSERIAL == 1) && (AD7476_FPGA_DRIVER == 1)
#error "FEATURE_USBSERIAL and AD7476_FPGA_DRIVER are both enabled, Please select only of these FPGA IP features"
#endif

///* enable the LTC1859 driver */
//#define LTC1859_DRIVER  0 // 1
//
///* enable the AUDIO driver */
#define AUDIO_DRIVER    1    // Set 1 to enable audio sampling

#define PDM2DEC_FACT    48   // Use 1.5MHz PDM Clock

//
///* enable LPSD mode of AUDIO IP*/
//#define ENABLE_LPSD    0 //Set to 1 enable, 0 to disable LPSD
//
///* if 0 load from SPI, if 1 load FFE/FPGA from SD card */
//#define LOAD_FROM_SD    1

#define EOSS3_ASSERT( x )

/* FIXME: Future, make this configurable based on filesystem?
 * OR - maybe 4K is a really good number and we should stick with 4K
 */
#define RIFF_BLOCK_SIZE (4*1024) /* we save data in 4K blocks */
// The first 0x30 bytes of a data block is the data block header.
#define RIFF_DATA_BLOCK_PAYLOAD_SIZE  (RIFF_BLOCK_SIZE - 0x30)

/* Define this flag to Enable Internal LDO. If undefined, internal LDO will be disabled.*/
//#define ENABLE_INTERNAL_LDO   1  // set to 0 for power measurement

/* max frequency is 1600hz, we give a bit more just in case */
/* report rate is 10mSec, or 100 times per second */
#define SENSIML_FFE_MAX_DATARATE        1600
#define SENSIML_FFE_REPORT_RATE          100
#define SENSIML_FFE_MAX_BATCH_DATA_SZ ((SENSIML_FFE_MAX_DATARATE/ SENSIML_FFE_REPORT_RATE)+2)
#define SENSIML_FFE_MIN_BATCH_DATA_SZ (6)

/* select one of the following for Audio PDM  block */
//#define PDM_PAD_28_29 1
//#define PDM_PAD_8_10  0
//#define VOICE_AP_BYPASS_MODE 0

#define SET_LPSD_THRESH 0

#define VOICE_CONF_ENABLE_I2S_MIC 0

/* if mono define one of these */
#define PDM_MIC_CHANNELS 1
#define PDM_MIC_LEFT_CH  1
#define PDM_MIC_RIGHT_CH 0

#define EN_STEREO_DUAL_BUF 0

//#define AEC_ENABLED 1

//#define AUDIO_LED_TEST 1 //Valid only in Chandalar BSP specific apps

#define MQTTSN_OVER_UART    1 //current implementation is over UART
//#define MQTTSN_OVER_BLE     (RECOG_VIA_BLE) //currently use BLE once Recogonition

#if (MQTTSN_OVER_UART == 1)
#define USE_FPGA_UART       0
//#define FEATURE_FPGA_UART   1
//#define MQTTSN_UART         UART_ID_FPGA //UART_ID_HW  //Use debug UART for now
#endif

#define DEFAULT_STORAGE_LOCATION    FREERTOS_NONE_MOUNTED // FREERTOS_SPI_FLASH

#if 1 //sensorTile has only SD card //QAI_CHILKAT
#define NUM_SUPPORTED_PATHS 1 
#else
#define NUM_SUPPORTED_PATHS 2
#endif //QAI_CHILKAT

/* stringize value */
#define VALUE2STRING(value)    TOSTRING(value)
#define TOSTRING(value)        #value
#define SOURCE_LOCATION        __FILE__":"VALUE2STRING(__LINE__)

/********************/

/* this should always be the last #define in this file */
/* it insures that we have completely processed this entire file */
#define _EnD_Of_Fw_global_config_h  1

#endif
