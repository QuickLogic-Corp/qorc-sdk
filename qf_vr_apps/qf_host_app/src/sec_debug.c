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
 *    File   : sec_debug.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "eoss3_dev.h"
#include "sec_debug.h"
#include "dbg_uart.h"


#ifdef BACKUP_REGISTER_SUPPORT
const unsigned long ulRegBackUp[16] = {0x0UL};
#endif  /* BACKUP_REGISTER_SUPPORT */


#if ( configSAVE_TASK_HISTORY==1 )
#define NUM_OF_TASK_HISTORY	200
typedef struct A_TASK_HISTORY {
	TaskHandle_t* tcb;
	TickType_t tick;
} TaskHistory_t;

uint16_t idx_task_history;
TaskHistory_t task_history[NUM_OF_TASK_HISTORY];
#endif


void save_assert_info(char* file, int line)
{
	char assert_info[270];
	
    dbg_str("****ASSERT****\n");
    dbg_str_str("assert", file);
    dbg_str_int("line", line );
	sprintf(assert_info, "%s(%d)\0", strrchr(file, '\\') ? strrchr(file, '\\')+1 : file, line);
	strncpy((char*)0x20000000, assert_info, strlen(assert_info));
	//QL_LOG_INFO_150K("fault cause = %s\n", assert_info);

	REBOOT_STATUS_REG &= ~REBOOT_CAUSE;
	REBOOT_STATUS_REG |= REBOOT_CAUSE_SOFTFAULT;	/* CHANGING THIS VALUE OR REGISTER REQUIRE CORRESPONDING CHANGE IN BOOTLOADER */
}


void invoke_soft_fault(void)
{      
        //QL_LOG_INFO_150K("Soft Fault\n");
        dbg_fatal_error("SOFT FAULT\n");
	{ taskDISABLE_INTERRUPTS(); for( ;; ); }
}


#if ( configSAVE_TASK_HISTORY==1 )

void sec_save_task_history(TaskHandle_t* tcb, TickType_t tick)
{
	(++idx_task_history)==NUM_OF_TASK_HISTORY?idx_task_history=0:0;

	
	task_history[idx_task_history].tcb = tcb;
	task_history[idx_task_history].tick = tick;
}

#endif
