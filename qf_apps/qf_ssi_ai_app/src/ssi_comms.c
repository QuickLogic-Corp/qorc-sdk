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
#include "sensor_ssss.h"
#include "sensor_audio_config.h"
#include "RtosTask.h"
#include "ssi_comms.h"

#define STACK_SIZE_TASK_SSI (256)
#define PRIORITY_TASK_SSI (PRIORITY_NORMAL)
#define SSI_RXBUF_SIZE (32)

uint8_t txbufWithSync[1024];

extern void ssi_seqnum_init(uint8_t);
extern void ssi_seqnum_reset(uint8_t);
extern uint32_t ssi_seqnum_update(uint8_t);
extern uint32_t ssi_seqnum_get(uint8_t);
extern uint8_t ssi_payload_checksum_get(uint8_t *p_data, uint16_t len);

xTaskHandle xHandleTaskSSI;
bool        is_ssi_connected          = false;
const char  ssi_connect_string[]      = "connect";
int         ssi_connect_string_len    = sizeof(ssi_connect_string) - 1;
const char  ssi_disconnect_string[]   = "disconnect";
int         ssi_disconnect_string_len = sizeof(ssi_disconnect_string) - 1;

/* Control  task */
void ssiTaskHandler(void* pParameter)
{
    int  count    = 0;
    int  json_len = 0;
    int  rx_avail = 0;
    char ssi_rxbuf[SSI_RXBUF_SIZE];
    char *pposConnect, *pposDisconnect;
    assert(SSI_RXBUF_SIZE >= ssi_connect_string_len);

    memset(ssi_rxbuf, '0', sizeof(ssi_rxbuf));
    ssi_rxbuf[SSI_RXBUF_SIZE-1] = 0;

    json_len = strlen(json_string_sensor_config);
    while (1)
    {
        // Send the JSON string
        vTaskDelay(1000);
        if (is_ssi_connected == false)
        {
            uart_tx_raw_buf(UART_ID_APP, json_string_sensor_config, json_len);
        }
        while (1)
        {
            rx_avail = uart_rx_available(UART_ID_APP);
            if (rx_avail == 0)
            	break;
            memmove(ssi_rxbuf, ssi_rxbuf+1, SSI_RXBUF_SIZE-2);
            uart_rx_raw_buf(UART_ID_APP, &ssi_rxbuf[SSI_RXBUF_SIZE-2], 1);
      	    pposConnect = strstr(ssi_rxbuf, ssi_connect_string);
      	    pposDisconnect = strstr(ssi_rxbuf, ssi_disconnect_string);
            if (pposDisconnect)
            {
               count = pposDisconnect - ssi_rxbuf + ssi_disconnect_string_len;
               memset(ssi_rxbuf, '0', count);
               if (is_ssi_connected == false)
            	   break;
               is_ssi_connected = false;
#if (SSI_SENSOR_SELECT_SSSS)
               sensor_ssss_startstop(0);
#endif
#if (SSI_SENSOR_SELECT_AUDIO)
               sensor_audio_startstop(0);
#endif
               break;
            }
            if (pposConnect)
            {
               count = pposConnect - ssi_rxbuf + ssi_connect_string_len;
               memset(ssi_rxbuf, '0', count);
               if (is_ssi_connected == true)
            	   break;
               is_ssi_connected = true;
               // reset sequence number for this connection
               // default channel number is 0
               ssi_seqnum_reset(SSI_CHANNEL_DEFAULT);
#if (SSI_SENSOR_SELECT_SSSS)
               sensor_ssss_startstop(1);
#endif
#if (SSI_SENSOR_SELECT_AUDIO)
               uart_tx_raw_buf(UART_ID_APP, "AUDIOSTREAMSTART", 16);
               sensor_audio_add();
               sensor_audio_startstop(1);
#endif
               break;
            }
            else
            {
        	   //count = 0;
               continue;
            }
        }
    }
}

void ssi_publish_sensor_data(uint8_t* p_source, int ilen)
{
    if (is_ssi_connected)
    {
        uart_tx_raw_buf(UART_ID_APP, p_source, ilen);
    }
}

void ssi_publish_sensor_results(uint8_t* p_source, int ilen)
{
    if (is_ssi_connected == false)
    {
        uart_tx_raw_buf(UART_ID_APP, p_source, ilen);
    }
}

signed portBASE_TYPE StartSimpleStreamingInterfaceTask(
    void)  // to remove warnings      uxPriority not used in the function
{
    static uint8_t ucParameterToPass;
    /* Create SSI Task */
    xTaskCreate(ssiTaskHandler,
                "SSITaskHandler",
                STACK_SIZE_ALLOC(STACK_SIZE_TASK_SSI),
                &ucParameterToPass,
                PRIORITY_TASK_SSI,
                &xHandleTaskSSI);
    configASSERT(xHandleTaskSSI);
    return pdPASS;
}

static uint32_t ssi_conn_seqnum[SSI_MAX_CHANNELS] = {0};
void ssi_seqnum_init(uint8_t channel)
{
	if (channel >= SSI_MAX_CHANNELS)
		return;
    ssi_conn_seqnum[channel] = 0;
}

void ssi_seqnum_reset(uint8_t channel)
{
	if (channel >= SSI_MAX_CHANNELS)
		return;
    ssi_conn_seqnum[channel] = 0;
}

uint32_t ssi_seqnum_update(uint8_t channel)
{
	if (channel >= SSI_MAX_CHANNELS)
		return 0;
    ssi_conn_seqnum[channel]++;
    return ssi_conn_seqnum[channel];
}

uint32_t ssi_seqnum_get(uint8_t channel)
{
	if (channel >= SSI_MAX_CHANNELS)
		return 0;
    return ssi_conn_seqnum[channel];
}

uint8_t ssi_payload_checksum_get(uint8_t *p_data, uint16_t len)
{
    uint8_t crc8 = p_data[0];
    for (int k = 1; k < len; k++)
    {
        crc8 ^= p_data[k];
    }
    return crc8;
}

void ssiv2_publish_sensor_data(uint8_t channel, uint8_t* p_source, int ilen)
{
    if (is_ssi_connected == false)
       return;

    uint8_t ssiv2header[SSI_HEADER_SIZE];
    uint8_t sync = SSI_SYNC_DATA;
    uint8_t rsvd = 0;
    uint16_t u16len = (ilen + 6);
    uint32_t seqnum = ssi_seqnum_update(channel);
    uint8_t crc8 = 0;

    ssiv2header[0] = sync;
    ssiv2header[1] = (u16len >> 0) & 0xff;
    ssiv2header[2] = (u16len >> 8) & 0xff;
    ssiv2header[3] = rsvd;
    ssiv2header[4] = channel;
    ssiv2header[5] = (seqnum >> 0) & 0xff;
    ssiv2header[6] = (seqnum >> 8) & 0xff;
    ssiv2header[7] = (seqnum >> 16) & 0xff;
    ssiv2header[8] = (seqnum >> 24) & 0xff;

    // compute 8-bit checksum
    crc8 = crc8 ^ ssi_payload_checksum_get(ssiv2header+3, SSI_HEADER_SIZE-3);
    crc8 = crc8 ^ ssi_payload_checksum_get(p_source, ilen);

    // Send SSI v2 header information
    uart_tx_raw_buf(UART_ID_APP, ssiv2header, SSI_HEADER_SIZE);

    // Send channel data
    uart_tx_raw_buf(UART_ID_APP, p_source, ilen);

    // Send 8-bit checksum
    uart_tx_raw_buf(UART_ID_APP, &crc8, 1);
}
