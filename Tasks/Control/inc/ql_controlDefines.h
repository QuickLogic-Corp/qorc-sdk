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

/* Defines associated with Control Task */

#ifndef __QL_CONTROLDEFINES_H_
#define __QL_CONTROLDEFINES_H_

#include <stdbool.h>
#include <stdio.h>
#include "fsm.h"

#define KPROCESS    15       // Number of processes in the GSM
#define KSTATES     55       // Number of entries in the GSM  State Machine (sum of states + event arcs)

struct GSMrow {
  bool  fIsState;                   // True if this row represents a state, False if is an event (arc) row
  char* psSysStateName;             // Friendly name for the system state
  char  ceEvent;                    // For state rows this is empty
                                    // For event rows this is event
  char  aStateOrGuard[KPROCESS];    // For state rows these are the process states that define the System State
                                    // For event rows these are the guards on the event -- all guards must be true for the event to lead to actions
  char  aActions[KPROCESS];         // For state rows these are empty
                                    // For event rows, these define what actions should happen for each process
};

struct ProcessActions {
  enum process_state    (*pFSMAction)(enum process_action, void *);
  void*                  pFSMConfigData;        
};
#endif // __QL_CONTROLDEFINES_H_