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
 *    File   : eoss3_hal_uart.h
 *    Purpose: This file contains macros, structures and APIs for
 *             UART read/write 
 *                                                          
 *=========================================================*/

#ifndef __TAMAR_HAL_UART_H_
#define __TAMAR_HAL_UART_H_

#include "Fw_global_config.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "eoss3_hal_pad_config.h"

/*!	\enum UartBaudRateType
	\brief Uart baudrate
*/
typedef enum
{
	BAUD_2400 = 0,
	BAUD_4800,
	BAUD_9600,
	BAUD_19200,
	BAUD_38400,
	BAUD_57600,
	BAUD_115200,
	BAUD_230400,
	BAUD_460800,
	BAUD_921600,
	BAUD_INVALID,
} UartBaudRateType;

/*!	\enum UartWordLenType
	\brief Uart wordlength to transfer/receive
*/
typedef enum
{
	WORDLEN_8B = 0,
	WORDLEN_7B,
	WORDLEN_6B,
	WORDLEN_5B,
	WORDLEN_INVALID = -1,
} UartWordLenType;

/*!	\enum UartStopBitsType
	\brief Uart stopbits selection
*/
typedef enum
{
	STOPBITS_1 = 0,
	STOPBITS_2,
	STOPBIT_INVALID = -1,
} UartStopBitsType;

/*!	\enum UartParityType
	\brief Uart parity type
*/
typedef enum
{
	PARITY_NONE = 0,
	PARITY_EVEN,
	PARITY_ODD,
} UartParityType;

/*!	\enum UartModeType
	\brief Uart operation mode
*/
typedef enum
{
	DISABLE_MODE = 0,
	TX_MODE,
	RX_MODE,
	TX_RX_MODE
} UartModeType;

/*!	\enum UartHwFlowCtrl
	\brief Uart H/W flow control selection
*/
typedef enum
{
	HW_FLOW_CTRL_DISABLE = 0,
	HW_FLOW_CTRL_ENABLE,
} UartHwFlowCtrl;

/*!	\enum UartInterruptMode
	\brief Uart interrupt mode selection
*/
typedef enum
{
	UART_INTR_DISABLE = 0,
	UART_INTR_ENABLE
} UartInterruptMode;

/*!	\enum CpuClk
	\brief CPU clock for UART configuration
*/
typedef enum
{
	CLK_2_33 = 0,
	CLK_77_76,
	CLK_19,
	CLK_40,
	CLK_2_2,
	CLK_10,
} CpuClk;

#define UART_ID_DISABLED     0   // /dev/null */
#define UART_ID_HW           1   // the hard UART on the S3
#define UART_ID_SEMIHOST     2   // Write debug data to semihost
#define UART_ID_FPGA         3   // second uart if part of FPGA
#define UART_ID_BUFFER       4   // Write data to buffer
#define UART_ID_SEMBUF       5   // Write data to semihost and buffer
#define UART_ID_USBSERIAL    6   // Write data to USB serial port
#define UART_ID_FPGA_UART1   7   // FPGA UART 1

#if !defined(UART_ID_CONSOLE)   /* most often, the UART_HW */
#define UART_ID_CONSOLE UART_ID_USBSERIAL
#endif

/*! \struct UartHandler eoss3_hal_uart.h "inc/eoss3_hal_uart.h"
 * 	\brief UART configuration structure filled in by user.
 */
typedef struct
{
    CpuClk                 cpuClk;		/*!< CPU clock rate */
    UartBaudRateType       baud;		/*!< Uart baud rate */
    UartWordLenType        wl;			/*!< Uart word length */
    UartStopBitsType       stop;		/*!< Uart stop bit */
    UartParityType         parity;		/*!< Uart parity bit */
    UartModeType           mode;		/*!< Uart operation mode */
    UartHwFlowCtrl         hwCtrl;		/*!< Uart H/W Flow control */
    UartInterruptMode      intrMode;		/*!< Uart interrupt mode */
    uint32_t               lcr_h_value;
    uint32_t               cr_value;
    uint32_t               ibrd_value;
    uint32_t               fbrd_value;
    int                    lpm_enabled;
	bool					fBaremetal;
}UartHandler;


void uart_pm_update( int uartid, int is_wakeup );

void uart_init(int uartid, PadConfig* ppadConfigTx, PadConfig* ppadConfigRx, const UartHandler *pConfig );

/* newbaudrate can be:  BAUD_9600, or 9600 */
void uart_new_baudrate( int uartid, uint32_t newbaudrate );

void uart_isr_handler(int uartid);

/**
 * @brief Send byte over UART.
 *
 *  @param uartid   which uart
 *  @param c		Byte to transmit over UART
 *
 *  Raw does not perform lf -> cr/lf mapping.
 *  Where as "non-raw" does perform lf->cr/lf mapping
 */
void uart_tx_raw(int uartid, int c);

/**
 * @brief Send byte over UART.
 *
 *  @param uartid   which uart
 *  @param c		Byte to transmit over UART
 *
 *  Raw does not perform lf -> cr/lf mapping.
 *  Where as "non-raw" does perform lf->cr/lf mapping
 */
void uart_tx(int uartid, int c);

/**
 * @brief Send buffer over UART.
 *
 *  @param uartid   which uart
 *  @param data     bytes to send
 *  @param len      count of bytes.
 *
 *  Raw does not perform lf -> cr/lf mapping.
 *  Where as "non-raw" does perform lf->cr/lf mapping
 */

void uart_tx_raw_buf(int uartid, const uint8_t *data,size_t len);

/**
 * @brief Send buffer over UART.
 *
 *  @param uartid   which uart
 *  @param data     bytes to send
 *  @param len      count of bytes.
 *
 *  Raw does not perform lf -> cr/lf mapping.
 *  Where as "non-raw" does perform lf->cr/lf mapping
 */
void uart_tx_buf(int uartid, const uint8_t *data,size_t len);

/* wait till a byte is ready, or timeout */
int uart_rx_wait( int uartid, int timeout );
/**
 * @brief Return number of bytes available in input buffer.
 *
 * @param uartid   which uart.
 *
 */
int uart_rx_available(int uartid );

/**
 * @brief Read N bytes from uart into buffer
 *
 * @param uartid   which uart.
 *
 * This is a RAW uart api, and does not preform cr/lf mapping -> lf.
 */
int uart_rx_raw_buf( int uartid, uint8_t *puthere, size_t nbytes );

/**
 * @brief Read byte from uart, blocking does not return
 */
int uart_rx(int uartid);

/**
 * @brief Set uart lpm enable or disable
 * 
 * @param 1 = enable, 0 = disable
 */
void uart_set_lpm_state(int uart_id, int lpm_en);

int uart_tx_is_fifo_full(int uart_id);
int uart_tx_is_fifo_empty(int uartid);

extern int uart_tx_is_fifo_half_empty(int uartid);
extern int uart_tx_get_fifo_status(int uartid);
extern int uart_tx_get_fifo_space_available(int uartid);

/*
 * @}
 */
#endif /* !__TAMAR_HAL_UART_H_ */
