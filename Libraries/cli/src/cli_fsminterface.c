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
 *    File   : cli_platform.c
 *    Purpose: command line interface for smart remote demo application. 
 * $Revision: 4224$
 * $Date: 2011-02-22$
 *=========================================================*/

/** @file cli_platform.c */


#include "Fw_global_config.h"

#include "cli.h"
#include "FreeRTOS.h"
#include "task.h"
#include <eoss3_hal_uart.h>
#include "RtosTask.h"
#include "fsm.h"

/*********************************************************************************
 *
 *  CLI-FSM interface
 *
 ********************************************************************************/
enum process_state  CLI_pstate;
       
enum process_state CLI_FSMAction(enum process_action pa, void* pv) { 
    struct cli_cmd_entry* pmenu = (struct cli_cmd_entry*)(*((int *) pv));
    switch(pa) {
    case PACTION_CONFIG:
        CLI_pstate = PSTATE_STOPPED;
        break;
        
    case PACTION_START:
        CLI_start_task(pmenu);
        CLI_pstate = PSTATE_STARTED;
        break;
            
    default:
        configASSERT(0);
        break;
    }
    return(CLI_pstate);
}
