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
 *    File   : mqttsn_uart.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"
#include <stdio.h>
#include <stdint.h>
//#include "usbd_cdc_interface.h"
#include "eoss3_hal_uart.h"
//#include "eoss3_hal_fpga_usbserial.h"

#if 1 //( USE_USB_UART  == 1)   

#define MAX_UART_RX_BUF_SIZE 2048
static unsigned char uart_init_done = 0;
static uint8_t uart_buffer_to_read[MAX_UART_RX_BUF_SIZE];
static int rx_rd_index, rx_wr_index;
static int rx_rd_count, rx_wr_count;

// These are functions from MQTTSN_SML that have dependency on SeneorTile
void mqttsn_uart_setup(void)
{
  uart_init_done = 1;
  rx_rd_index = 0;
  rx_wr_index = 0;
  rx_rd_count = 0;
  rx_wr_count = 0;    
  return;
}
#if 0
void fill_uart_rx_buffer(uint8_t* pBuf, uint32_t bufLen)
{
  //pBuf[bufLen] = 0;  printf("Rx:%d:%s\n", bufLen,pBuf);
  int copy_count = bufLen;
  if ((rx_wr_index + copy_count) > MAX_UART_RX_BUF_SIZE)
  {
    copy_count = MAX_UART_RX_BUF_SIZE - rx_wr_index;
    memcpy(&uart_buffer_to_read[rx_wr_index], pBuf, copy_count);
    pBuf += copy_count;
    if(rx_rd_index > rx_wr_index)
    {
      printf("UART RX buffer overflow \n");
    }
    rx_wr_index = 0;
    copy_count = bufLen - copy_count;
  }
  memcpy(&uart_buffer_to_read[rx_wr_index], pBuf, copy_count);
  rx_wr_index += copy_count;
  if(rx_wr_index >= MAX_UART_RX_BUF_SIZE)
    rx_wr_index = 0;
  rx_wr_count += bufLen;
  return;
}
#endif
void mqttsn_uart_tx(uint8_t *pBuf, uint32_t bufLen)
{
    uart_tx_raw_buf(UART_ID_MQTTSN, pBuf, bufLen);
    return;
}

int mqttsn_uart_rx_available(void)
{
  int count = uart_rx_available(UART_ID_MQTTSN);
  //if (count > 0) dbg_str_int("", count);
  return count;
}

int mqttsn_uart_rx(uint8_t *pBuf, int n)
{
  int count = uart_rx_raw_buf(UART_ID_MQTTSN, pBuf, n);
  return count;
}

/* Return 1 if FIFO is almost full
 * Return 0 otherwise
 */
int mqttsn_uart_tx_is_fifo_full()
{
  return uart_tx_is_fifo_full(UART_ID_MQTTSN);
}

/* Return 1 if FIFO is empty
 * Return 0 otherwise
 */
int mqttsn_uart_tx_is_fifo_empty()
{
  return uart_tx_is_fifo_empty(UART_ID_MQTTSN);
}

#endif

