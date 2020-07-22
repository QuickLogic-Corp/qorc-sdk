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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if (MQTTSN_OVER_UART == 1)
#include "mqttsn_uart.h"

// These are wrapper API functions from MQTTSN_SML that have dependency on Hardware
void mqttsn_comm_setup(void)
{
  mqttsn_uart_setup();
  return;
}
void mqttsn_comm_tx(uint8_t *pBuf, uint32_t bufLen)
{
  mqttsn_uart_tx(pBuf, bufLen);
  return;
}
int mqttsn_comm_rx_available(void)
{
  return mqttsn_uart_rx_available();
}

int mqttsn_comm_rx(uint8_t *pBuf, int n)
{
  return mqttsn_uart_rx(pBuf, n);
}

int mqttsn_comm_tx_is_fifo_full(void)
{
  return mqttsn_uart_tx_is_fifo_full();
}

int mqttsn_comm_tx_is_fifo_empty(void)
{
  return mqttsn_uart_tx_is_fifo_empty();
}

#endif

#if (MQTTSN_OVER_BLE == 1)
#include "ql_bleTask.h"
#include "dbg_uart.h"

/* send this packet to the BLE */
void my_ble_send( int cmd, int len, const void *data )
{
    uint8_t databuf[20];
    if( len > 20){
        dbg_fatal_error("ble-msg-overflow\n");
    }
    databuf[0] = cmd;
    memcpy( (void *)(&databuf[1]), (void *)(data), len );
    SendToBLE( SEND_BLE_IMMEDIATE, len+1, databuf );
}

#endif
