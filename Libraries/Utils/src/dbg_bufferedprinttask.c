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
 *    File   : dbg_bufferedprint.c
 *    Purpose: print message buffers at low priority. 
 * $Revision: 4224$
 * $Date: 2011-02-22$
 *=========================================================*/

/** @file cli_platform.c */


#include "Fw_global_config.h"

#include "FreeRTOS.h"
#include "task.h"
#include "RtosTask.h"

#include <eoss3_hal_uart.h>
#include "dbg_uart.h"

#define DBG_TASK_STACKSIZE  256

xTaskHandle xHandleDbgbufferedprint;
extern uint8_t      acDbgBuffer[SIZEOF_DBGBUFFER];
extern uint8_t*     pcDbgBuffer;
extern uint8_t*     pcDbgBufferLim;

static uint8_t*     pcDbgBufferPrev = acDbgBuffer;

void dbg_bufferedprinttask( void *pParameter )
{
    (void)(pParameter);
    while (1) {
        #if USE_SEMIHOSTING == 1    // Defined in Fw_global_config.h
            if (pcDbgBuffer > pcDbgBufferPrev) {        // Normal case: characters have been added
                _semihost_write((char*)pcDbgBufferPrev, pcDbgBuffer - pcDbgBufferPrev);
                pcDbgBufferPrev = pcDbgBuffer;
            } else if (pcDbgBufferPrev > pcDbgBuffer) { // Wrap-around has happened
                _semihost_write((char*)pcDbgBufferPrev, pcDbgBufferLim - pcDbgBufferPrev); // Write tail of the buffer
                pcDbgBufferPrev = acDbgBuffer;                                      // Wrap pointer
                if (pcDbgBuffer > pcDbgBufferPrev) {    // If more characters at start of buffer write them
                    _semihost_write((char*)pcDbgBufferPrev, pcDbgBuffer - pcDbgBufferPrev);
                    pcDbgBufferPrev = pcDbgBuffer;
                } 
            }
        #else
            while (pcDbgBufferPrev != pcDbgBuffer) {
                uart_tx_raw(UART_ID_HW, *pcDbgBufferPrev++);
                if (pcDbgBufferPrev == pcDbgBufferLim) {
                    pcDbgBufferPrev = acDbgBuffer;
                }
            }
        #endif
        vTaskDelay(100);    // Give other low priority tasks a chance to run
    }
}



void dbg_startbufferedprinttask(UBaseType_t priority)
{
    xTaskCreate ( dbg_bufferedprinttask, "dbgbufferedprint", DBG_TASK_STACKSIZE, NULL, (UBaseType_t)(PRIORITY_LOWER), &xHandleDbgbufferedprint);
    configASSERT( xHandleDbgbufferedprint );
}
