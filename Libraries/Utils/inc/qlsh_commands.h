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
*    File   : qlsh_commands.h
*    Purpose: 
*                                                          
*=========================================================*/

#ifndef INC_QLSH_COMMANDS_H
#define INC_QLSH_COMMANDS_H

#if !defined( _EnD_Of_Fw_global_config_h )
#error "Include Fw_global_config.h first"
#endif
/*!	\file qlsh_commands.h
\brief Command definitions
*/

/*	Include the generic headers required for the FreeRTOS port being used */
#include <stddef.h>
#include <stdint.h>

/*!	\enum eERROR_CODES
\brief These values are for individual Tasks of sensorHub system
*/
enum eERROR_CODES
{
  eQL_SUCCESS = 0,
  eQL_ERR_MSG_SEND = 1
};



enum audio_cmds {
  eCMD_AUDIO_DATA_READY = 1,
  eCMD_AUDIO_LPSDT = 2,
  eCMD_AUDIO_DMIC = 3,
  eCMD_DATA_AV_VOICE_CFG_LPSD_ONLY = 4,     /*LPSD ON/OFF Trigger*/
  eCMD_DATA_AV_VOICE_CFG_CONTINUOUS = 5,         /*LPSD mode OFF and Continous DMA */

  eCMD_AUDIO_START = 6,
  eCMD_AUDIO_STOP
};


/*! \def MAX_QUEUE_PACKET_DATA_LEN
\brief A macro that holds maximum data length of xQ_Packet data arguments.
*/
#define MAX_QUEUE_PACKET_DATA_LEN		6

/*! \struct xQ_Packet qlsh_commands.h "inc/qlsh_commands.h"
* 	\brief packet format shared between sensorhub tasks.
* 	
* 	All sensorhub tasks will have similar queue packet format
* 	as defined below
* 	\code
* 	 =====================================================
* 	|  D5  |  D4  |  D3  |  D2  |  D1  |  D0  | Cmd | Src |
* 	|======|======|======|======|======|======|=====|=====|
* 	|      |      |      |      |      |      |     |     |
* 	 =====================================================
* 	 Src   - Source task of packet creator
* 	 Cmd   - command to process
* 	 D0-D5 - Arguments based on commands
* 	 \endcode
*/ 
struct xQ_Packet
{
  uint8_t ucSrc;								/*!< source of packet */
  uint8_t ucCommand;							/*!< command to process */
  uint8_t ucData[MAX_QUEUE_PACKET_DATA_LEN];	/*!< arguments of command */
};



#endif //INC_QLSH_COMMANDS_H
