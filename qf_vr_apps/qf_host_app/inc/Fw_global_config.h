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
 *
 *    File   : Fw_global_config.h
 *    Purpose:
 *
 *=========================================================*/

#ifndef FW_GLOBAL_CONFIG_H_INCLUDED     /* Avoid multiple inclusion             */
#define FW_GLOBAL_CONFIG_H_INCLUDED

#include <stdint.h>
#define DEBUG_QL_SYSTEMSTATE 1
#define CONTROLTASK_IGNORE_UNHANDLED_EVENT  1       // Set to 1 to ignore unhandled events (prints warning), or set to 0 to configassert on unhandled event

#define ENABLE_VOICE_SOLUTION   1
#define PDM2DEC_FACT  48

#define FEATURE_CLI_DEBUG_INTERFACE  1
#define FEATURE_CLI_FILESYSTEM       0

#define FEATURE_USBSERIAL   (1)       // USBSERIAL port is present
#define FEATURE_I2S_MASTER_CLKS   (0) // Generates I2S Master clks

#define FEATURE_D2HPROTOCOL_HOST (1)

#define FEATURE_1WIRE_PROTOCOL_HOST (0) //1 = 1-Pin protocol, 0 = 4-pin protocol
#define FEATURE_OPUS_ENCODER        (0)

/* Device firmware image selection */
#define DEVICE_FIRMWARE_IMAGE_VR_RAW_APP    (1)
#define DEVICE_FIRMWARE_IMAGE_VR_OPUS_APP   (2)
#define DEVICE_FIRMWARE_IMAGE_VR_I2S_APP    (3)
#define DEVICE_FIRMWARE_IMAGE_VR_1WIRE_RAW_APP    (4)

// Select raw streaming app or opus streaming app as the device firmware image
#define DEVICE_FIRMWARE_IMAGE               (DEVICE_FIRMWARE_IMAGE_VR_RAW_APP)
//#define DEVICE_FIRMWARE_IMAGE             (DEVICE_FIRMWARE_IMAGE_VR_OPUS_APP)
//#define DEVICE_FIRMWARE_IMAGE             (DEVICE_FIRMWARE_IMAGE_VR_I2S_APP)
//#define DEVICE_FIRMWARE_IMAGE             (DEVICE_FIRMWARE_IMAGE_VR_1WIRE_RAW_APP)

#if (DEVICE_FIRMWARE_IMAGE == DEVICE_FIRMWARE_IMAGE_VR_1WIRE_RAW_APP)
#undef  FEATURE_1WIRE_PROTOCOL_HOST
#define FEATURE_1WIRE_PROTOCOL_HOST (1) //1 = 1-Pin protocol, 0 = 4-pin protocol
#endif

#if (DEVICE_FIRMWARE_IMAGE == DEVICE_FIRMWARE_IMAGE_VR_OPUS_APP)
#undef  FEATURE_OPUS_ENCODER
#define FEATURE_OPUS_ENCODER    (1)
#endif

#if (DEVICE_FIRMWARE_IMAGE == DEVICE_FIRMWARE_IMAGE_VR_I2S_APP)
#undef  FEATURE_I2S_MASTER_CLKS
#define FEATURE_I2S_MASTER_CLKS   (1) // Generates I2S Master clks
#endif

/* Select the filesystem API to use */
#define USE_FREERTOS_FAT         0  ///< Set this to 1 to use FreeRTOS FAT filesystem (Merced default)
#define USE_FATFS                0  ///< Set this to 1 to use FATFs filesystem

/* use this flag to check the data block header */
/* can be used only iff the QL_XPORT_INCLUDE_HEADER is defined for Device */
#define TST_HEADER_VERIFY (0)


#define uartHandlerUpdate(id,x)

// Options for debug output -- use to set DEBUG_UART below
// #define UART_ID_DISABLED  0 /* /dev/null */
// #define UART_ID_HW        1 /* the hard UART on the S3 */
// #define UART_ID_SEMIHOST  2  // Write debug data to semihost
// #define UART_ID_FPGA      3 /* second uart if part of FPGA */
// #define UART_ID_BUFFER    4 // Write data to buffer
// #define UART_ID_SEMBUF    5 // Write datat to semihost and buffer
// #define UART_ID_USBSERIAL    6   // Write data to USB serial port
#define DEBUG_UART UART_ID_HW   // Write debug data to buffer (bufferprinttask can be used to print)
#define UART_ID_CONSOLE    UART_ID_HW
#define USE_SEMIHOSTING     0       // 1 => use semihosting, 0 => use UART_ID_HW

#define SIZEOF_DBGBUFFER    2048    // Number of characters in circular debug buffer

#define DBG_flags_default 0 //  (DBG_FLAG_ble_cmd + DBG_FLAG_sensor_rate+DBG_FLAG_datasave_debug)
#define DBG_FLAGS_ENABLE 1
#if !DBG_FLAGS_ENABLE
#define DBG_flags 0
#else
extern uint32_t DBG_flags;
#endif


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


/* enable via sw the FFE or disable it, TODO: Make this real instead of a hack */
#define SW_ENABLE_FFE   0
/* enable via sw the AUDIO or disable it, TODO: Make this real instead of a hack */
#define SW_ENABLE_AUDIO 0
/* enable the FFE or not, see SW_ENABLE_FFE only 1 should exist */
#define FFE_DRIVERS	0 // 1

/* do or do not perform dynamic frequency scaling */
#define CONST_FREQ (1)
/* used to enable CPU load calculation for DFS */
#define CPU_LOAD_CALC_ENABLE 0

/* enable the LTC1859 driver */
#define LTC1859_DRIVER  0 // 1

/* enable the AUDIO driver */
#define AUDIO_DRIVER    0    // Set 1 to enable audio sampling


/* if 0 load from SPI, if 1 load FFE/FPGA from SD card */
#define LOAD_FROM_SD    1

#define EOSS3_ASSERT( x )




//#define 	USE_FPGA_UART

#if 0
#define ASSP_UARTTx uarttx
#define ASSP_UARTRx uartrx
#define assp_uartHandlerUpdate uartHandlerUpdate
#define assp_uartInit uartInit
#define ASSP_uart_read uart_read
#define ASSP_fillRxBuf fillRxBuf
#define ASSP_getRxBuf getRxBuf
#define ASSP_getRxBufSize getRxBufSize
#endif

/* Define this flag to Enable Internal LDO. If undefined, internal LDO will be disabled.*/
#define ENABLE_INTERNAL_LDO   1  // set to 0 for power measurement

/* select one of the following for Audio PDM  block */
#define PDM_PAD_28_29 1
#define PDM_PAD_8_10  0
#define VOICE_AP_BYPASS_MODE 0

#define SET_LPSD_THRESH 0
#define KSAMPLES_TO_DROP    10 //400 

/* define one of these */
#define PDM_MIC_CHANNELS (1)
#if (PDM_MIC_CHANNELS == 2)
    #define PDM_MIC_STEREO (1)
#endif
#define VOICE_CONF_ENABLE_I2S_MIC 0

/* if mono define one of these */
#define PDM_MIC_LEFT_CH 1
#define PDM_MIC_RIGHT_CH 0

#define EN_STEREO_DUAL_BUF 0

//#define AEC_ENABLED 1

#define AUDIO_LED_TEST 1 //Valid only in Chandalar BSP specific apps

/* stringize value */
#define VALUE2STRING(value)    TOSTRING(value)
#define TOSTRING(value)        #value
#define SOURCE_LOCATION        __FILE__":"VALUE2STRING(__LINE__)

/********************/

/* this should always be the last #define in this file */
/* it insures that we have completely processed this entire file */
#define _EnD_Of_Fw_global_config_h  1

#endif
