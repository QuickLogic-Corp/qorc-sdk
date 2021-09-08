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

/* FEATURE for global configuration -------------------------------*/

/** Enable/Disable Uart for power management */
#define ENABLE_UART

#define FEATURE_FPGA_UART   0       // FPGA UART not present
#define FEATURE_USBSERIAL   0       // USBSERIAL port is present

// Options for debug output -- use to set DEBUG_UART below
// #define UART_ID_DISABLED  0 /* /dev/null */
// #define UART_ID_HW        1 /* the hard UART on the S3 */
// #define UART_ID_SEMIHOST  2  // Write debug data to semihost
// #define UART_ID_FPGA      3 /* second uart if part of FPGA */
// #define UART_ID_BUFFER    4 // Write data to buffer
// #define UART_ID_SEMBUF    5 // Write datat to semihost and buffer
// #define UART_ID_USBSERIAL    6   // Write data to USB serial port
#define DEBUG_UART UART_ID_BUFFER  // Write debug data to semihost

// Set the UART ID to use for bootloader
#define UART_ID_BOOTLOADER UART_ID_HW

//#define USE_SEMIHOSTING     1       // 1 => use semihosting, 0 => use UART_ID_HW

#define SIZEOF_DBGBUFFER    2048    // Number of characters in circular debug buffer

/* disable if needed */
#define FEATURE_DBG_UART   1 /* must be 0 or 1, not defined or undefined */

#define CONST_FREQ  (1)

#define EOSS3_ASSERT( x )

#define ASSP_UARTTx uarttx
#define ASSP_UARTRx uartrx
#define assp_uartHandlerUpdate uartHandlerUpdate
#define assp_uartInit uartInit
#define ASSP_uart_read uart_read
#define ASSP_fillRxBuf fillRxBuf
#define ASSP_getRxBuf getRxBuf
#define ASSP_getRxBufSize getRxBufSize


#define _EnD_Of_Fw_global_config_h

#endif  //FW_GLOBAL_CONFIG_H_INCLUDED
/********************/


