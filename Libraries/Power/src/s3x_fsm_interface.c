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
#include "FreeRTOS.h"
#include "task.h"
#include "s3x_dfs.h"
#include "fsm.h"

void    DFS_Start(uint32_t nodeMin);
/*********************************************************************************
 *
 *  DFS-FSM interface
 *
 ********************************************************************************/
int                 DFS_FSMConfigData;
enum process_state  DFS_pstate;
       
enum process_state DFS_FSMAction(enum process_action pa, void* pv) {    
    switch(pa) {
    case PACTION_CONFIG:
        DFS_Start(0);
        DFS_pstate = PSTATE_NODE0;
        break;
        
    case PACTION_MIN_NODE0:
        DFS_Start(0);
        DFS_pstate = PSTATE_NODE0;
        break;
        
    case PACTION_MIN_NODE1:
        DFS_Start(1);
        DFS_pstate = PSTATE_NODE1;
        break;
        
    case PACTION_MIN_NODE2:
        DFS_Start(2);
        DFS_pstate = PSTATE_NODE2;
        break;
        
    case PACTION_MIN_NODE3:
        DFS_Start(3);
        DFS_pstate = PSTATE_NODE3;
        break;
        
    case PACTION_MIN_NODE4:
        DFS_Start(4);
        DFS_pstate = PSTATE_NODE4;
        break;
        
    case PACTION_MIN_NODE5:
        DFS_Start(5);
        DFS_pstate = PSTATE_NODE5;
        break;
        
    case PACTION_MIN_NODE6:
        DFS_Start(6);
        DFS_pstate = PSTATE_NODE6;
        break;
        
    case PACTION_MIN_NODE7:
        DFS_Start(7);
        DFS_pstate = PSTATE_NODE7;
        break;
        
    case PACTION_STOP:
        s3x_disable_dfs();
        DFS_pstate = PSTATE_STOPPED;
        break;
        
    default:
        configASSERT(0);
        break;
    }
    return(DFS_pstate);
}

void DFS_Start(uint32_t policyMin) {
    // Make sure dfs is active and running at the designated node
    DFS_Enable();
    s3x_set_min_allowed_policy(policyMin);
    DFS_AlignPolicy(policyMin);
}

/********************************************************************************/