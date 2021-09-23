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

#if !defined(SENSOR_CMDS_H)
#define SENSOR_CMDS_H

#include "cli.h"

extern uint8_t raw_bytes[128];

/* clear the command buffer */
extern void cli_cmd_buff_reset(void);

/* Add an 8bit value to the COMMAND */
extern void cli_cmd_buff_wr_u8( int v );

/* Add an 16bit value to the COMMAND */
extern void cli_cmd_buff_wr_u16( int v );
  
/* Add an 32bit value to the COMMAND */
extern void cli_cmd_buff_wr_u32( uint32_t v );

/* Add an 64bit value to the COMMAND */
extern void cli_cmd_buff_wr_u64( uint64_t v );

/* list of cli commands to simulate ble commands */
extern const struct cli_cmd_entry sensor_cmds[] ;

/* we have constructed the packet
 * our next step is to pretend that we have received the SPI packet.
 */
extern void do_dispatch( uint32_t numBytes );

#endif
