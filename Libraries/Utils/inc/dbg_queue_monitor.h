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

#ifndef __DBGQUEUEMONITOR_H__
#define __DBGQUEUEMONITOR_H__

#include "Fw_global_config.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "queue.h"

/* stringize value */
#define VALUE2STRING(value)    TOSTRING(value)
#define TOSTRING(value)        #value

/* Return source code filename and line number in the format
   <FILENAME>:<LINE_NUMBER>
*/
#define SOURCE_LOCATION        __FILE__":"VALUE2STRING(__LINE__)

#define        DQM_SIZE_MAX     (64)

// Monitors Queue and Heap sizes
typedef struct st_s3_mon_info {
  int           num_drop_count;
  int           heap_size;
  int           dqmArraySize;
  int           dbg_queue_monitor_value[DQM_SIZE_MAX];
  QueueHandle_t dbg_queue_monitor_array[DQM_SIZE_MAX];
} s3_mon_info_t;
extern s3_mon_info_t s3_mon_info;

extern void dbg_queue_monitor_add(QueueHandle_t q);
extern void dbg_queue_monitor_print(char *message);
extern void dbg_queue_monitor_update(void);

#endif /* __DBGQUEUEMONITOR_H__ */