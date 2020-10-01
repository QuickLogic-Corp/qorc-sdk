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
*    File   : ql_controlTask.c
*    Purpose:
*
*=========================================================*/


/*****************Prototype******************************
*   struct xCQ_Packet CQpacket;
* 
*    CQpacket.ceEvent = CEVENT_XXXXXX;
*    addPktToControlQueue(&CQpacket);
*********************************************************/
#include "Fw_global_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ql_controlTask.h"
#include "qlsh_commands.h"
#include "eoss3_dev.h"
#include "eoss3_hal_audio.h"
#include "eoss3_hal_leds.h"
#include "s3x_clock_hal.h"
#include "s3x_clock.h"
#include "RtosTask.h"
#include <math.h>
#include "qlsh_commands.h"

#include "ql_util.h"
#include "cli.h"

#include "timers.h"


// Include fsm data structures, which are a 'c' file

#include "fsm_tables.h"
xTaskHandle     xHandleTaskControl;
QueueHandle_t   xHandleQueueControl;
int             xQueueControlOverflow;

int             irowPreviousState;


/* LOCAL STATIC VARIABLES */

void ProcessControlEvent(enum control_event ceEvent) {
	int		irowEvent;
	int		iprocess;
	bool	fValid = false;

#if DEBUG_QL_CONTROLTASK == 1
    dbg_str_str("Sys State", afsmrow[irowPreviousState].psSysStateName);
    dbg_str_str("Event", apsFromCE[ceEvent]);
#endif

	// Now find Event arc associated with the current state
	for (irowEvent = irowPreviousState + 1; !afsmrow[irowEvent].fIsState && irowEvent != KSTATESACTUAL; irowEvent++) {
      //printf("irowPreviousState = %d, irowEvent = %d, event = %s\n", irowPreviousState, irowEvent, apsFromCE[afsmrow[irowEvent].ceEvent]);
		if (afsmrow[irowEvent].ceEvent == ceEvent) {
			// Need to check if all guards are valid
			fValid = true;
			for (iprocess = 0; iprocess != KPROCESSACTUAL; iprocess++) {
              if (afsmrow[irowEvent].aStateOrGuard[iprocess] == 0) continue;
              if (currentState.aStateOrGuard[iprocess] != afsmrow[irowEvent].aStateOrGuard[iprocess]) {
                  fValid = false;
                  //dbg_str_int("Guard failed", irowEvent);
              }
			}
		}
		if (fValid) break;
	}
                                                                                          
	if (afsmrow[irowEvent].fIsState) {
        dbg_str_str_nonl("Event", apsFromCE[ceEvent]);
        dbg_str_str(" not found for", afsmrow[irowPreviousState].psSysStateName);
        #if (CONTROLTASK_IGNORE_UNHANDLED_EVENT == 0)
        assert(0);
        #endif
        dbg_str("Ignored\n");
	}
                                                                                          
	// Scan thru finding actions
	for (iprocess = 0; iprocess != KPROCESSACTUAL; iprocess++) {
        //printf("row%d, name=%s, paction = %x\n", irowEvent, apsFromCE[afsmrow[irowEvent].ceEvent], afsmrow[irowEvent].aActions[iprocess]);
		if (afsmrow[irowEvent].aActions[iprocess] != 0) { // Got work to do
			DoAction(iprocess, (enum process_action)afsmrow[irowEvent].aActions[iprocess]);
		}
	}
    irowPreviousState = FindCurrentState(apsFromCE[ceEvent]);
    #if DEBUG_QL_SYSTEMSTATE == 1
    dbg_str_str(afsmrow[irowPreviousState].psSysStateName, ""); 
    #endif //DEBUG_QL_SYSTEMSTATE
}

void    DoAction(int iprocess, enum process_action pa) {
    #if DEBUG_QL_CONTROLTASK == 1
    printf("Process %15s->%s\n", apsFromProcess[iprocess], apsFromPA[pa]);
    #endif //DEBUG_QL_CONTROLTASK
    currentState.aStateOrGuard[iprocess] = apaction[iprocess].pFSMAction(pa, apaction[iprocess].pFSMConfigData);
}

int FindCurrentState(const char* pzEvent) {
    int		irow;
	int		iprocess;
	bool	fMatch;

	for (irow = 0; irow != KSTATESACTUAL; irow++) {
        if (!afsmrow[irow].fIsState) continue;
		fMatch = true;
		for (iprocess = 0; iprocess != KPROCESSACTUAL; iprocess++) {
			if ((afsmrow[irow].aStateOrGuard[iprocess] != PSTATE_DONT_CARE) && (currentState.aStateOrGuard[iprocess] != PSTATE_DONT_CARE)) {
				if (afsmrow[irow].aStateOrGuard[iprocess] != currentState.aStateOrGuard[iprocess]) {
					fMatch = false;
					break;
				}
			}
		}
		if (fMatch) break;
	}
	if (irow == KSTATESACTUAL) {
		printf("Error: Can't find CurrentState in GSM table (preceeding event = '%s')\n", pzEvent);
        for (iprocess = 0; iprocess != KPROCESSACTUAL; iprocess++) {
			printf("irow=%d, iprocess=%-15s, CS=%s\n", irow, apsFromProcess[iprocess], apsFromPS[currentState.aStateOrGuard[iprocess]]);
		}
        assert(0);
	}
	currentState.psSysStateName = afsmrow[irow].psSysStateName;
	return(irow);
}

/* Control  task */
void controlTaskHandler(void *pParameter)
{
    struct xCQ_Packet CQpacket;
    BaseType_t         xResult = pdFAIL;

    irowPreviousState = FindCurrentState("startup");  // Initialize previous state pointer to be the initial state

    vTaskDelay(100/portTICK_PERIOD_MS);    // Wait 100ms before running

    // Push event that will trigger configuration onto queue
    CQpacket.ceEvent = CEVENT_CONFIG;
    addPktToControlQueue(&CQpacket);

    // Push event that will start the system
    CQpacket.ceEvent = CEVENT_START;
    addPktToControlQueue(&CQpacket);

    /* Wait for events */
    while(1)
    {
        /* QL read message from Message queue */
        xResult = xQueueReceive( xHandleQueueControl, &( CQpacket ), portMAX_DELAY);

        if( xResult == pdPASS ) {
            // Process all messages/data regardless of source
            ProcessControlEvent( CQpacket.ceEvent );
        }
    } /* QL end of while loop */
}


signed portBASE_TYPE StartControlTask( void)                                  // to remove warnings      uxPriority not used in the function
{
  static UINT8_t ucParameterToPass;
  /* Create queue for AP Task */
  xHandleQueueControl = xQueueCreate( CONTROL_QUEUE_LENGTH, sizeof(struct xQ_Packet) );
  if(xHandleQueueControl == 0)
  {
    return pdFAIL;
  }
  vQueueAddToRegistry( xHandleQueueControl, "Control_Q" );
  /* Create AP Task */
  xTaskCreate (controlTaskHandler, "ControlTaskHandler", STACK_SIZE_ALLOC(STACK_SIZE_TASK_CONTROL), &ucParameterToPass, PRIORITY_TASK_CONTROL, &xHandleTaskControl);
  configASSERT( xHandleTaskControl );
  return pdPASS;
}

void addPktToControlQueueFromISR( struct xCQ_Packet *pPacket )
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if( xQueueIsQueueFullFromISR( xHandleQueueControl ) ){
    xQueueControlOverflow++;
    return;
  }
  xQueueSendFromISR( xHandleQueueControl, ( void * )pPacket, &xHigherPriorityTaskWoken );

  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void addPktToControlQueue( struct xCQ_Packet *pPacket )
{
    if( !(uxQueueSpacesAvailable( xHandleQueueControl )) ){
        xQueueControlOverflow++;
        return;
    }
    xQueueSend( xHandleQueueControl, ( void * )pPacket, 10 );
}

void ControlEventSend(enum control_event ce) {
    struct xCQ_Packet CQpacket;

    CQpacket.ceEvent = ce;
    addPktToControlQueue(&CQpacket);
}

void ControlEventSendFromISR(enum control_event ce) {
    struct xCQ_Packet CQpacket;

    CQpacket.ceEvent = ce;
    addPktToControlQueueFromISR(&CQpacket);
}

void cli_fsm_event(const struct cli_cmd_entry *pEntry)
{
    ControlEventSend((enum control_event)pEntry->cookie);
}
