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

#ifndef FW_GLOBAL_CONFIG_H_INCLUDED   
#define FW_GLOBAL_CONFIG_H_INCLUDED

#include <stdint.h>
#include "app_config.h"
#include "sec_debug.h"


/*######################## INTERFACE OUTPUT OPTIONS  ################################*/



/***************   CONFIGURE HARDWARE OUTPUT    *****************/

#define FEATURE_FPGA_UART   0       // Set to 1 to enable the FPGA UART port if present
#define FEATURE_USBSERIAL   1       // Set to 1 to enable the USBSERIAL port if present
#define USE_SEMIHOSTING     0       // Set to 1 to enable the semihosting port if present



/***************   UART ROUTES SETTINGS        *****************/

// TODO: CHECK THAT ONLY ONE IS ENABLED

// #define UART_ID_DISABLED     0   // /dev/null */
// #define UART_ID_HW           1   // the hard UART on the S3
// #define UART_ID_SEMIHOST     2   // Write debug data to semihost
// #define UART_ID_FPGA         3   // second uart if part of FPGA
// #define UART_ID_BUFFER       4   // Write data to internal buffer
// #define UART_ID_SEMBUF       5   // Write data to semihost and buffer
// #define UART_ID_USBSERIAL    6   // Write data to USB serial port
     
#define DEBUG_UART  (UART_ID_BUFFER)  // Set the output of debug messages
#define UART_ID_APP  (UART_ID_USBSERIAL)       // Set the output for application messages



/*######################## ADVANCED SETTINGS  ################################*/

#define SIZEOF_DBGBUFFER    512    // Number of characters in circular debug buffer

#define ENABLE_VOICE_SOLUTION   1
#define PDM2DEC_FACT  32

#define FEATURE_CLI_DEBUG_INTERFACE  1
#define FEATURE_CLI_FILESYSTEM       0

/* The following macros select the filesystem used in the QLFS Library */
/* Select the filesystem API to use */
#define USE_FREERTOS_FAT         0  ///< Set this to 1 to use FreeRTOS FAT filesystem (Merced default)
#define USE_FATFS                1  ///< Set this to 1 to use FATFs filesystem

/* Select one of these APIs for use in the RIFF Library */
/* Make sure config-GCC.mk is properly configured based on these selections */
#define USE_QLFS_APIS            0 ///< Set this to 1 to use QLFS APIs for the RIFF Library
#define USE_FATFS_APIS           USE_FATFS ///< Set this to 1 to use FATFS filesystem APIs for the RIFF Library
#define USE_FREERTOS_FAT_APIS    USE_FREERTOS_FAT ///< Set this to 1 to use FREERTOS filesystem APIs for the RIFF Library
#if (USE_QLFS_APIS == 1)
#undef  USE_FATFS_APIS
#undef  USE_FREERTOS_FAT_APIS
#define USE_FATFS_APIS           0
#define USE_FREERTOS_FAT_APIS    0
#endif

/* use this flag to check the data block header */
/* can be used only iff the QL_XPORT_INCLUDE_HEADER is defined for Device */
#define TST_HEADER_VERIFY (1)

/* future may have other modes? */

#define uartHandlerUpdate(id,x)


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

#define DEFAULT_STORAGE_LOCATION    FREERTOS_SPI_SD
/* Entire filename, volume + path + filename */
#define QLFS_MAX_ABS_PATHLEN (200+1)
/* no component of the filename can be larger then this */
#define QLFS_MAX_FILENAME_COMPONENT 50

/* Select the maximum file size for storing the sensor data */
#define RIFF_FILE_SIZE_MAX   (1024*4*25)  // 100KB

// Select one of the file naming generation when RIFF_FILE_SIZE_MAX is defined
#define RIFF_AUTO_SEQUENCE_FILENAMES        (0)  // Set to 1 to use sequential count appended to filename
#define RIFF_TIMESTAMP_SEQUENCE_FILENAMES   (1)  // Set to 1 to use timestamp appended to filename
#define USE_DCL_FILENAME_ONLY               (0)
#define SSI_RECOGNITION_RIFF_ID     (0x12345678) // ID stored in SD card datafile for recognition results

#define UUID_TOTAL_BYTES     16
extern uint8_t DeviceClassUUID[UUID_TOTAL_BYTES];

#if 1 //sensorTile has only SD card //QAI_CHILKAT
#define NUM_SUPPORTED_PATHS 1
#else
#define NUM_SUPPORTED_PATHS 2
#endif //QAI_CHILKAT

extern const char *SOFTWARE_VERSION_STR;

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

#define IMU_M4_DRIVERS     0  ///< enable IMU sensors (Accel, Gyro, ...)
                              ///< using M4 driver to probe and collect data from
                              ///< the sensors

#define IMU_FFE_DRIVERS    0  ///< option to enable IMU sensor data collection
                              ///< using onchip FFE (availble on EOS-S3 only)

#define IMU_DRIVERS        (IMU_M4_DRIVERS || IMU_FFE_DRIVERS)

#define USE_IMU_FIFO_MODE   (0)    ///< Use FIFO mode to read from Accelerometer device


///* enable via sw the FFE or disable it, TODO: Make this real instead of a hack */
//#define SW_ENABLE_FFE   0
///* enable via sw the AUDIO or disable it, TODO: Make this real instead of a hack */
//#define SW_ENABLE_AUDIO 0
///* enable the FFE or not, see SW_ENABLE_FFE only 1 should exist */
//#define FFE_DRIVERS	0 // 1
//
///* do or do not perform dynamic frequency scaling */
#define CONST_FREQ (1)
//
///* enable the LTC1859 driver */
//#define LTC1859_DRIVER  0 // 1
//
///* enable the AUDIO driver */
#define AUDIO_DRIVER    1    // Set 1 to enable audio sampling
//
///* enable LPSD mode of AUDIO IP*/
//#define ENABLE_LPSD    0 //Set to 1 enable, 0 to disable LPSD
//
///* if 0 load from SPI, if 1 load FFE/FPGA from SD card */
//#define LOAD_FROM_SD    1

#define EOSS3_ASSERT( x )

/* Define this flag to Enable Internal LDO. If undefined, internal LDO will be disabled.*/
//#define ENABLE_INTERNAL_LDO   1  // set to 0 for power measurement

/* select one of the following for Audio PDM  block */
#define PDM_PAD_28_29 1
//#define PDM_PAD_8_10  0
//#define VOICE_AP_BYPASS_MODE 0

#define SET_LPSD_THRESH 0

/* define one of these */
#define PDM_MIC_MONO 1
#define PDM_MIC_STEREO 0
#define VOICE_CONF_ENABLE_I2S_MIC 0

/* if mono define one of these */
#define PDM_MIC_LEFT_CH 1
#define PDM_MIC_RIGHT_CH 0

#define EN_STEREO_DUAL_BUF 0

#define PDM_MIC_CHANNELS (1)

//#define AEC_ENABLED 1

//#define AUDIO_LED_TEST 1 //Valid only in Chandalar BSP specific apps

/* stringize value */
#define VALUE2STRING(value)    TOSTRING(value)
#define TOSTRING(value)        #value
#define SOURCE_LOCATION        __FILE__ ":" VALUE2STRING(__LINE__)

/********************/

/* this should always be the last #define in this file */
/* it insures that we have completely processed this entire file */
#define _EnD_Of_Fw_global_config_h  1



#endif
