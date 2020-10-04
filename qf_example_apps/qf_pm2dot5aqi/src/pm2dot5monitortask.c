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
 *   File   : pm2dot5monitortask.c
 *   Purpose: Contains functionality to monitor PM2.5 concentartion using PMS5003 sensor
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"

#include "FreeRTOS.h"
#include "RtosTask.h"
#include "timers.h"

#include "stdbool.h"

#include "dbg_uart.h"
#include "eoss3_hal_gpio.h"
#include "eoss3_hal_uart.h"

/*==========================================================
 *                                                          
 *   External variables
 *                                                          
 *=========================================================*/
bool    fShowPM2dot5 = false;       // When set, report PM2.5 values to USB

/*==========================================================
 *                                                          
 *   Local functions
 *                                                          
 *=========================================================*/
void monitorTaskHandler(void * parameter);

/*==========================================================
 *                                                          
 *   Local variables
 *                                                          
 *=========================================================*/
xTaskHandle xHandleMonitorTask;
#define SWAPBYTES(x)    ((((x) & 0xff) << 8) | (((x) >> 8) & 0xff))
struct aqi {
    int16_t     pmval;
    int16_t     aqival;
    uint8_t        red;
    uint8_t        green;
    uint8_t        blue;
} aqitable[] = {
    {12, 50,        0, 1, 0},   // Green = Healthy
    {35, 100,       0, 0, 1},   // Blue = Moderate
    {55, 150,       1, 0, 0},   // Red  = Unhealthy for sensitive
    {150, 200,      1, 0, 1},   // Purple = Unhealthy
    {250, 300,      1, 0, 1},
    {500, 500,      1, 0, 1},
    {10000, 10000,  1, 0, 1}
};

/*==========================================================
 *                                                          
 *   Functions
 *                                                          
 *=========================================================*/

/* Setup Task Handler for Monitor Task */
signed portBASE_TYPE StartMonitorTask( void)
{
    static uint8_t ucParameterToPass;
    
    /* Create Monitor Task */
    xTaskCreate ( monitorTaskHandler, "MonitorTaskHandler", STACK_SIZE_ALLOC(configMINIMAL_STACK_SIZE),  &ucParameterToPass, PRIORITY_NORMAL, &xHandleMonitorTask);
    configASSERT( xHandleMonitorTask );
    
    return pdPASS;
}

void monitorTaskHandler(void * parameter) {
    int         i;
    bool        fActive = true;
    struct packet {
        uint8_t     start_char1;
        uint8_t     start_char2;
        uint16_t    frame_length;
        uint16_t    adata[13];
        uint16_t    data_check;
    } packet;
    HAL_GPIO_Write(0, 1);       // Tell device to go into normal operating mode (SET)
    HAL_GPIO_Write(7, 0);                   // Put device in RESET
    while(uart_rx_available(UART_ID_HW)) {  // Flush UART
        uart_rx(UART_ID_HW);
    }
    HAL_GPIO_Write(7, 1);                   // Bring device out of RESET
    
    while(1) {
        uart_rx_raw_buf(UART_ID_HW, (uint8_t*)&packet, sizeof(packet));
        packet.frame_length = SWAPBYTES(packet.frame_length);
        for (int i = 0; i != 13; i++) {
            packet.adata[i] = SWAPBYTES(packet.adata[i]);
        }
        packet.data_check = SWAPBYTES(packet.data_check);
        if (packet.start_char1 != 0x42 || packet.start_char2 != 0x4d) {
            dbg_str("Start characters did not match");
            HAL_GPIO_Write(7, 0);                   // Put device in RESET
            while(uart_rx_available(UART_ID_HW)) {  // Flush UART
                uart_rx(UART_ID_HW);
            }
            HAL_GPIO_Write(7, 1);                   // Bring device out of RESET
        }
        if (fShowPM2dot5) {
            dbg_str_int("PM2.5", packet.adata[5-1]);
        }
        // Set led to appropriate color
        for (i = 0; i != sizeof(aqitable)/sizeof(aqitable[0])-1; i++) {
            if (packet.adata[5-1] < aqitable[i].pmval) break;
        }
        if (fActive) {
            HAL_GPIO_Write(6, aqitable[i].red);
            HAL_GPIO_Write(5, aqitable[i].green);
            HAL_GPIO_Write(4, aqitable[i].blue);
            fActive = false;
        } else {
            HAL_GPIO_Write(6, 0);
            HAL_GPIO_Write(5, 0);
            HAL_GPIO_Write(4, 0);
            fActive = true;
        }
    }
    return;
}