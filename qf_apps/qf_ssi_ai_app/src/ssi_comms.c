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
#include "RtosTask.h"
#include "ssi_comms.h"

#define STACK_SIZE_TASK_SSI (256)
#define PRIORITY_TASK_SSI (PRIORITY_NORMAL)
#define SSI_RXBUF_SIZE (32)

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
