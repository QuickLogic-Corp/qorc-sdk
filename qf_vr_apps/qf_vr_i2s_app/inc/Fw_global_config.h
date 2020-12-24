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

#define FEATURE_CLI_DEBUG_INTERFACE  0
#define FEATURE_CLI_FILESYSTEM       0

#define FEATURE_D2HPROTOCOL_DEVICE (1)

#define FEATURE_FLL_I2S_DEVICE (1) 

#if (FEATURE_FLL_I2S_DEVICE == 1)
//#define FLL_I2S_LOCAL_CLK  (1*1024*1000) //for 16K sample rate = 2*32*16K = 1024000
#endif    

/* Select the filesystem API to use */
#define USE_FREERTOS_FAT         0  ///< Set this to 1 to use FreeRTOS FAT filesystem (Merced default)
#define USE_FATFS                0  ///< Set this to 1 to use FATFs filesystem

#define uartHandlerUpdate(id,x)

// Options for debug output -- use to set DEBUG_UART below
// #define UART_ID_DISABLED  0 /* /dev/null */
// #define UART_ID_HW        1 /* the hard UART on the S3 */
// #define UART_ID_SEMIHOST  2
// #define UART_ID_FPGA      3 /* second uart if part of FPGA */
// #define UART_ID_BUFFER    4 // Write data to buffer
// #define UART_ID_SEMBUF    5 // Write datat to semihost and buffer
#define DEBUG_UART UART_ID_BUFFER   // Write debug data to buffer (bufferprinttask can be used to print)
     
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

#define ENABLE_LOAD_FROM_FLASH (0)

/** Enable Host mode for Voice application, This standalone S3 mode */

#define HOST_VOICE  1


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
#define SW_ENABLE_AUDIO 1
/* enable the FFE or not, see SW_ENABLE_FFE only 1 should exist */
#define FFE_DRIVERS	0 // 1

/* do or do not perform dynamic frequency scaling */
#define CONST_FREQ (0)
/* used to enable CPU load calculation for DFS */
#define CPU_LOAD_CALC_ENABLE 0

/* enable the LTC1859 driver */
#define LTC1859_DRIVER  0 // 1

/* enable the AUDIO driver */
#define AUDIO_DRIVER    1    // Set 1 to enable audio sampling


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
#define ENABLE_INTERNAL_LDO   0  // set to 0 for power measurement

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

#define EN_STEREO_DUAL_BUF 1 //0= use stereo data interleaved

#define ENABLE_I2S_TX_SLAVE (1) //Audio data is streamed on I2S interface
#if (ENABLE_I2S_TX_SLAVE == 1 )
#define ENABLE_I2S_AND_SPI_DATA_TRANSMIT   (1) //Audio data is streamed on SPI interface also
#endif
//These are for I2S stream options used by the I2S Task
#define ENABLE_I2S_STEREO_TRANSMIT 0 //0 = mono channel transmit, 1 =stereo
#define ENABLE_I2S_48K_TRANSMIT    0 //0 = 16K samples, 1 = 48K 
#define I2S_DMA_BUFFER_COUNT      (4)//must be >=2 for continuous I2S stream


#define QL_XPORT_INCLUDE_HEADER (0) //(1)

#define ENABLE_RAW_TX_SPI   (1)
#define ENABLE_OPUS_ENCODER (0)

//#define ENABLE_HOST_IF (1)

//#if (ENABLE_I2S_TX_SLAVE == 1)
//#undef ENABLE_HOST_IF
//#endif


//#if (ENABLE_HOST_IF == 1)
//#define VM1010_MIC_BOARD    (0)
//#define ENABLE_OPUS_TX (1)

//#if (ENABLE_OPUS_TX == 0)
//#define ENABLE_RAW_TX_SPI   (1)
//#define ENABLE_OPUS_ENCODER (0)
//#else
//#define ENABLE_RAW_TX_SPI   (1)
//#define ENABLE_OPUS_ENCODER (0)
//#endif

//#endif  // ENABLE_HOST_IF


/* Fs = Audio sampling rate */
#define AUDIO_SAMPLING_RATE             (16000)
#define AUDIO_BLOCK_SIZE_IN_SAMPLES     (240)
#define AUDIO_BLOCK_SIZE_IN_MS          (15)
#if QL_XPORT_INCLUDE_HEADER == 1
#define QL_XPORT_BLCOK_HDR_SZ           (8)
#else
#define QL_XPORT_BLCOK_HDR_SZ           (0)
#endif


//#if ENABLE_RAW_TX_SPI == 1

#define TX_SPI_AUDIO_BLOCK_COUNT   (8)

/* examlpe ~100ms = 6 blocks*AUDIO_BLOCK_SIZE_IN_SAMPLES = 1440  pcm samples mono */
/* OR       120ms = 8 blocks*AUDIO_BLOCK_SIZE_IN_SAMPLES = 1920  pcm samples mono */
/* 960 samples = 4 block of 15 mS each = 60mSec and 960 block sz pcm samples mono */
/* 1200 samples = 5 block of 15 mS each = 75mSec and 1200 block sz pcm samples mono */
#define TX_SPI_AUDIO_RAW_BLOCK_SZ (TX_SPI_AUDIO_BLOCK_COUNT*(AUDIO_BLOCK_SIZE_IN_SAMPLES + QL_XPORT_BLCOK_HDR_SZ))

/* 50ms Timer - to check enough raw pcm samples available */
#define TX_SPI_AUDIO_RAW_MSG_PERIOD (40)
/* chalil - guess - shall be little lesser than time for TX_SPI_AUDIO_RAW_BLOCK_SZ ?,
   todo -test with more than one values {50, 90, 100} */
//#define TX_SPI_AUDIO_RAW_MSG_PERIOD (AUDIO_BLOCK_SIZE_IN_MS*(TX_SPI_AUDIO_BLOCK_COUNT-2))

// assume all the blocks are 15ms.
// for example, if this value is 10, a maximum of 150ms is held for instant streaming
// Note: set it to at least 2 times the buffer required
#define MAX_AUDIO_RAW_STREAM_QUEUE_SIZE  (TX_SPI_AUDIO_BLOCK_COUNT*8)

//#endif

#define AUDIO_LED_TEST  0 // Valid only in Chandalar BSP specific apps
#define DFS_LED_TEST    1 // Used to track DFS states (N0 = none, N1 = green, N2 = blue, N3 = orange, N4 = all
#define D2H_LED_TEST    0 // Blue means D2H line is high

#define MIN_AUDIO_SAVE_BLOCKS_SPARE_SIZE (30)
#define PRE_TEXT_BEFORE_KEY_PHRASE       (0)
/*Set this flag to 1 to enabled forced streaming enable/disable command through CLI.
Setting it to 1 will remove uart in lpm feature (since uart cli will be now used to send the start/stop cmd.)
(Will increase power consumption)*/
#define FORCED_STREAMING_ENABLED (0)

/********************/

/* this should always be the last #define in this file */
/* it insures that we have completely processed this entire file */
#define _EnD_Of_Fw_global_config_h  1

#endif
