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

#if !defined(MQTTSN_CLICMDS_H)
#define MQTTSN_CLICMDS_H

#include "cli.h"

/* list of cli commands to simulate MQTTSN commands */
extern const struct cli_cmd_entry mqttsn_cmds[] ;
void CLI_ProcessDataIn(uint8_t *pData, uint32_t numBytes);

#endif