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

/* Misc functions that need replacing */
#include "Fw_global_config.h"
#include "ql_controlTask.h"
#include "fsm.h"

int*               PTT_Switch_FSMConfigData;
enum process_state PTT_Switch_FSMAction(enum process_action pa, void* pv){
    switch(pa) {
    case PACTION_CONFIG:
        return PSTATE_STOPPED;
    }
    configASSERT(0);
    return(PSTATE_UNKNOWN);
};


int*                Mute_Switch_FSMConfigData;
enum process_state Mute_Switch_FSMAction(enum process_action pa, void* pv){
    switch(pa) {
    case PACTION_CONFIG:
        return PSTATE_STOPPED;
    }
    configASSERT(0);
    return(PSTATE_UNKNOWN);
};

//enum process_state vm1010_Config(void* pv){};
//int                vm1010_ConfigArg;
//enum process_state VesperMic_Stop(int x){};
//enum process_state VesperMic_Start(int x){};
//enum process_state vm1010_SetState(int x){return(PSTATE_UNCONFIG);};

//enum process_state vm1010Timer_Config(void* pv){};
//int                vm1010Timer_ConfigArg;
//enum process_state WOS_Timer_Stop(int x){};
//enum process_state WOS_Timer_Start(int x){};
//enum process_state vm1010Timer_SetState(int x){return(PSTATE_UNCONFIG);};

//enum process_state AudioHW_Config(void* pv){};
//int                AudioHW_ConfigArg;
//enum process_state AudioHW_Stop(int x){};
//enum process_state AudioHW_Start(int x

//enum process_state LPSD_Config(void* pv){};
//int                LPSD_ConfigArg;
//enum process_state LPSD_Stop(int x){};
//enum process_state LPSD_Start(int x){};

//enum process_state CircularBuffer_Config(void* pv){};
//int                CircularBuffer_ConfigArg;
//enum process_state CircularBuffer_Stop(int x){};
//enum process_state CircularBuffer_Start(int x){};
enum process_state CircularBuffer_SetState(int x){return(PSTATE_UNCONFIG);};

//enum process_state VR_Config(void* pv){};
int                VR_ConfigArg;
//enum process_state VR_Stop(int x){};
//enum process_state VR_Start(int x){};
enum process_state VR_SetState(int x){return(PSTATE_UNCONFIG);};

//enum process_state Host_Config(void* pv){};
//int                Host_ConfigArg;
//enum process_state Host_Stop(int x){};
//enum process_state Host_Start(int x){};
enum process_state HIF_SetState(int x){return(PSTATE_UNCONFIG);};

enum process_state D2H_SetState(int x){return(PSTATE_UNCONFIG);};