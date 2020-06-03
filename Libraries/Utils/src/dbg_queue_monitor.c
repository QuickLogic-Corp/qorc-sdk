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
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "stdio.h"
#include "dbg_uart.h"
#include "dbg_queue_monitor.h"

// Monitors Queue and Heap sizes
s3_mon_info_t  s3_mon_info = 
{
  .num_drop_count = 0,
  .heap_size = 0,
  .dqmArraySize = 0
};

//#define        DQM_SIZE_MAX     (64)
//QueueHandle_t  dbg_queue_monitor_array[DQM_SIZE_MAX];
//int            dqmArraySize = 0;

/* Add Queue handle to the monitoring array 
 * @param[in] q handle to FreeRTOS queue to add to monitoring array   
 */
void dbg_queue_monitor_add(QueueHandle_t q)
{
  s3_mon_info.dbg_queue_monitor_array[s3_mon_info.dqmArraySize] = q;
  s3_mon_info.dqmArraySize++;
}

/* print current length of each queue registered
 * using dbg_queue_monitor_add() function
 *
 * @param[in] message Additional message to be printed
 *
 * @returns None
 */
void dbg_queue_monitor_print(char *message)
{
  QueueHandle_t q;
  dbg_str(message);
  dbg_str_int("heap", xPortGetFreeHeapSize());
  for (int k = 0; k < s3_mon_info.dqmArraySize; k++) {
    q = s3_mon_info.dbg_queue_monitor_array[k];
    dbg_str_hex32("Q-handle", (uint32_t)q);
    dbg_str_hex32("Q-Len", uxQueueMessagesWaiting(q));
  }
}

/* Update current length of each queue registered
 * using dbg_queue_monitor_add() function
 *
 * @returns None
 */
void dbg_queue_monitor_update(void)
{
  QueueHandle_t q;
  for (int k = 0; k < s3_mon_info.dqmArraySize; k++) {
    q = s3_mon_info.dbg_queue_monitor_array[k];
    s3_mon_info.dbg_queue_monitor_value[k] = uxQueueMessagesWaiting(q);
  }
}
