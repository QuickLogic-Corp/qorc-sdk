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

#ifndef __MQTTSN_APP_H
#define __MQTTSN_APP_H

typedef enum {
    MQTTSN_STATE_DISCONNECTED,
    MQTTSN_STATE_CONNECTED,
    MQTTSN_STATE_SUBSCRIBING,
    MQTTSN_STATE_SUBS_ACK_PENDING,
    MQTTSN_STATE_SUBSCR_COMPLETE,
    MQTTSN_STATE_READY_FOR_PUBLISH,
    MQTTSN_STATE_APP_MAX
}MqttsnState_t;



void ProcessDataIn(void);
void HandleDisconnect(void);

#endif