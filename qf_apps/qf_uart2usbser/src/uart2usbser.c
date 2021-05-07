/** @file ssi_task.c */

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

#include "Fw_global_config.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "eoss3_hal_uart.h"
#include "dbg_uart.h"
#include "RtosTask.h"

#define STACK_SIZE_TASK_UARTTX (256)
#define STACK_SIZE_TASK_UARTRX (256)
#define PRIORITY_TASK_UARTTX (PRIORITY_NORMAL)
#define PRIORITY_TASK_UARTRX (PRIORITY_NORMAL)
#define UART_BUF_SIZE (256)

xTaskHandle xHandleTaskUartTx;
xTaskHandle xHandleTaskUartRx;

char uart_rxbuf[UART_BUF_SIZE];
char uart_txbuf[UART_BUF_SIZE];
char usbser_rxbuf[UART_BUF_SIZE];
char usbser_txbuf[UART_BUF_SIZE];

/* Receive bytes from UART and transmit to the USB-Serial */
void uartRxTaskHandler(void* pParameter)
{
	int rx_avail;
    while (1)
    {
        rx_avail = uart_rx_available(UART_ID_HW);
        while (rx_avail > UART_BUF_SIZE)
        {
            uart_rx_raw_buf(UART_ID_HW, uart_rxbuf, UART_BUF_SIZE);
            uart_tx_raw_buf(UART_ID_USBSERIAL, uart_rxbuf, UART_BUF_SIZE);
            rx_avail -= UART_BUF_SIZE;
        }
        if (rx_avail > 0)
        {
			uart_rx_raw_buf(UART_ID_HW, uart_rxbuf, rx_avail);
			uart_tx_raw_buf(UART_ID_USBSERIAL, uart_rxbuf, rx_avail);
        }
        vTaskDelay(0);
    }
}

/* Receive bytes from USB-Serial and transmit to the UART */
void uartTxTaskHandler(void* pParameter)
{
	int rx_avail;
	int count = 0;
    while (1)
    {
    	count = 0;
    	while (count < UART_BUF_SIZE)
    	{
            rx_avail = uart_rx_available(UART_ID_USBSERIAL);
            if (rx_avail == 0)
            	break;
            uart_rx_raw_buf(UART_ID_USBSERIAL, &usbser_rxbuf[count], 1);
            count++;
    	}
    	if (count > 0)
           uart_tx_raw_buf(UART_ID_HW, usbser_rxbuf, count);
        vTaskDelay(0);
    }
}

signed portBASE_TYPE StartUARTtoUSBSerialConverterTask(
    void)  // to remove warnings      uxPriority not used in the function
{
    static uint8_t ucParameterToPass;
    /* Create UART Tx Task */
    xTaskCreate(uartTxTaskHandler,
                "uartTxHandler",
                STACK_SIZE_ALLOC(STACK_SIZE_TASK_UARTTX),
                &ucParameterToPass,
                PRIORITY_TASK_UARTTX,
                &xHandleTaskUartTx);
    configASSERT(xHandleTaskUartTx);

    /* Create UART Rx Task */
    xTaskCreate(uartRxTaskHandler,
                "uartRxHandler",
                STACK_SIZE_ALLOC(STACK_SIZE_TASK_UARTRX),
                &ucParameterToPass,
                PRIORITY_TASK_UARTRX,
                &xHandleTaskUartRx);
    configASSERT(xHandleTaskUartRx);

    return pdPASS;
}
