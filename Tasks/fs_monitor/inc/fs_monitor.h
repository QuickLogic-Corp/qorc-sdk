/** @file fs_monitor.h */

/*==========================================================
 * Copyright 2021 QuickLogic Corporation
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

#ifndef FS_MONITOR_H
#define FS_MONITOR_H

#include "Fw_global_config.h"

// Default low threshold value of available free space as a percentage of total available space
// Override in Fw_global_config.h to set a different value
#ifndef FS_MONITOR_LOW_DISK_SPACE_THRESHOLD
#define FS_MONITOR_LOW_DISK_SPACE_THRESHOLD   (5)
#endif

// Default monitoring interval in seconds
// Override in Fw_global_config.h to set a different value
#ifndef FS_MONITOR_INTERVAL
#define FS_MONITOR_INTERVAL                   (60)
#endif

// Default stack size for file system monitor task
// Override in Fw_global_config.h to set a different value
#ifndef FS_MONITOR_TASK_STACKSIZE
#define FS_MONITOR_TASK_STACKSIZE             (256)
#endif

/** Callback function: Invoked when available free disk space falls below the
 * low disk space threshold. The default callback function outputs a message
 * to the UART. Application may override this default function to define the
 * the desired behavior
 */
extern void riff_low_disk_space(void);

/** Start file system monitor task.
 * Application should invoke this to start the file system monitor
 */
extern void start_fs_monitor_task(void);

#endif /* FS_MONITOR_H */
