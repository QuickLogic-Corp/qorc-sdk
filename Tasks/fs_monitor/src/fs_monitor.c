/** @file fs_monitor.c */

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

#include "Fw_global_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "RtosTask.h"
#include "dbg_uart.h"
#include "fs_monitor.h"

#if (USE_FATFS_APIS == 1)
#include "ff.h"
#else
#error "fs_monitor: Available only when using FatFs Library"
#endif

xTaskHandle xHandleFSMonitor;

/* Application project may override this default callback function to define
 * the desired application behavior when available free disk space falls
 * below the specified threshold
 */
__attribute__ ((weak)) void riff_low_disk_space(void)
{
	dbg_str_int_noln("FS-MONITOR", FS_MONITOR_LOW_DISK_SPACE_THRESHOLD);
	dbg_str_str_nonl("% disk space remaining", "free up space to continue collecting data");
	dbg_str("\n");
}

/* File system monitor task
 * Monitor available free disk space on the default mount volume.
 * Invoke the callback function when this falls below specified threshold
 */
void fs_monitor_task(void *pParameter)
{
	while (1)
	{
		vTaskDelay(1000 * pdMS_TO_TICKS(FS_MONITOR_INTERVAL));
        DWORD nclust, nsect, totsect, iperc;
        FATFS *pfs ;
        if (f_getfree("", &nclust, &pfs) == 0)
        {
        	totsect = (pfs->n_fatent - 2) * pfs->csize;
        	nsect = nclust * pfs->csize;
        	iperc = (nsect * 100L) / totsect ;
        	//dbg_str_int_noln("Free space: ", nsect/2);
        	//dbg_str_int_noln("  Total space: ", totsect/2);
        	//dbg_str_int_noln(" ", iperc);
        	//dbg_str("% disk space available\n");
        	if ( iperc <= FS_MONITOR_LOW_DISK_SPACE_THRESHOLD)
        	{
        		riff_low_disk_space();
        	}
        }
	}
}

/* Start the file system monitor task using normal priority and specified stack size
 * Application project should call this function to include the file system monitor.
 * The file system should be mounted to use this function
 */
void start_fs_monitor_task(void)
{
    xTaskCreate ( fs_monitor_task, "sdcard_disk_space_monitor", 4 * FS_MONITOR_TASK_STACKSIZE, NULL, (UBaseType_t)(PRIORITY_NORMAL), &xHandleFSMonitor);
    configASSERT( xHandleFSMonitor );
}
