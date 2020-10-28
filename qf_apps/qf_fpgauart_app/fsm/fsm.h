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

/* Defines for FSM Control */
#ifndef FSM_H_
#define FSM_H_
/* States */
enum process_state {
	PSTATE_UNCONFIG = 0,
	PSTATE_UNKNOWN,
	PSTATE_DONT_CARE,
	PSTATE_STOPPING,
	PSTATE_OUTQING,
	PSTATE_LPSD_ON,
	PSTATE_LPSD_OFF,
	PSTATE_STREAMING,
	PSTATE_SAVING,
	PSTATE_STARTED,
	PSTATE_WOS,
	PSTATE_NORMAL,
	PSTATE_STOPPED,
	PSTATE_NODE3,
	PSTATE_NODE2,
	PSTATE_NODE1,
	PSTATE_NODE0,
};

/* Events */
enum control_event {
	CEVENT_CONFIG = 0,
	CEVENT_START,
	CEVENT_HIF_EOT,
	CEVENT_HOST_MUTE_ON,
	CEVENT_HOST_MUTE_OFF,
	CEVENT_HOST_PROCESS_ON,
	CEVENT_HOST_PROCESS_OFF,
	CEVENT_HOST_READY_TO_RECEIVE,
	CEVENT_VR_TRIGGER,
	CEVENT_LPSD_OFF,
	CEVENT_LPSD_ON,
	CEVENT_WOS_TIMER,
	CEVENT_WOS,
};

/* Actions */
enum process_action {
	PACTION_NULL = 0,
	PACTION_BEGIN_STOPPING,
	PACTION_FINISH_STOPPING,
	PACTION_START_OUTQ,
	PACTION_STATE_ON,
	PACTION_STATE_OFF,
	PACTION_START_ON,
	PACTION_START_OFF,
	PACTION_START_STREAMING,
	PACTION_START_SAVING,
	PACTION_START,
	PACTION_START_WOS,
	PACTION_START_NORMAL,
	PACTION_STOP,
	PACTION_MIN_NODE3,
	PACTION_MIN_NODE2,
	PACTION_MIN_NODE1,
	PACTION_MIN_NODE0,
	PACTION_CONFIG,
};

#endif //FSM_H_
