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
 *    File   : ql_audio.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __QL_CONTROLTASK_H_
#define __QL_CONTROLTASK_H_

#if !defined( _EnD_Of_Fw_global_config_h )
#error "Include Fw_global_config.h first"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include <queue.h>
#include "common.h"
#include "ql_controlDefines.h"
#include "cli.h"


#define CONTROL_QUEUE_LENGTH  (20) 

struct xCQ_Packet
{
  enum control_event    ceEvent;
};

void    ControlEventSend(enum control_event ce);
void    ControlEventSendFromISR(enum control_event ce);
void    addPktToControlQueueFromISR( struct xCQ_Packet *pPacket );
void    addPktToControlQueue(struct xCQ_Packet *pxMsg);

void    DoAction(int, enum process_action);
int     FindCurrentState(const char*);
void    UpdateCurrentState(void);

void    cli_fsm_event(const struct cli_cmd_entry *pEntry);


#endif //__QL_CONTROLTASK_H_
