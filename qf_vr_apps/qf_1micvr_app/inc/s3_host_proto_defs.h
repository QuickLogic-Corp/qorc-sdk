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

#ifndef S3_HOST_PROTO_DEFS_H_
#define S3_HOST_PROTO_DEFS_H_


typedef enum HIF_MESSAGE_
{
  MESSAGE_VM_WAKEUP             = 0x01,
  MESSAGE_AUDIO_OPUS_CHUNK_DONE = 0x02,
  MESSAGE_KPT_DETECTED          = 0x03, 
  MESSAGE_STOP_AUDIO            = 0x04,
  MESSAGE_START_AUDIO           = 0x05,
  MESSAGE_AUDIO_OPUS_BLOCK_READY= 0x06,
  MESSAGE_AUDIO_RAW_BLOCK_READY = 0x07,
  MESSAGE_AUDIO_PCM_CHUNK_DONE  = 0x08,
  MESSAGE_MUTE_START            = 0x09,
  MESSAGE_MUTE_STOP             = 0x0A,
  MESSAGE_PTT_START             = 0x0B,
  MESSAGE_PTT_STOP              = 0x0C,
  MESSAGE_STOP_TX,
  MESSAGE_GET_MONINFO,
  MESSAGE_STREAM_KPT_DETECTED, //while streaming another KP detected
  MESSAGE_NOP                               // Does nothing (used for side effect of monitoring D2H status */
}HIF_MESSAGE;

// maximum of 64 different events could be sent to the host
// using the D2H protocol
typedef enum HIF_EVT_
{
  EVT_KP_DETECTED     = 0x10,
  EVT_OPUS_PKT_READY  = 0x11,
  EVT_OPUS_PKT_END    = 0x12,
  EVT_THRESHOLD       = 0x13,
  EVT_PING            = 0x14,
  EVT_STREAM_KP_DETECTED   = 0x15, //while streaming another KP detected
  EVT_RAW_PKT_READY   = 0x21,
  EVT_RAW_PKT_END     = 0x22,
  EVT_MUTE_START      = 0x30,
  EVT_MUTE_STOP       = 0x31,
  EVT_PTT_START       = 0x32,
  EVT_PTT_STOP        = 0x33,
  EVT_EOT,
  EVT_GET_MONINFO,
  EVT_COMMAND_ACK     = 0x3F   // Maximum of 6-bit field in the packet
}HIF_EVT;


// maximum of 64 different events could be received from the host
// using the D2H protocol
enum HIF_CMD
{
  CMD_HOST_READY_TO_RECEIVE = 0x1,
  CMD_HOST_PROCESS_OFF      = 0x2,
  CMD_HOST_STOP_STREAMING   = 0x3,
  CMD_HOST_THRESHOLD        = 0x4,
  CMD_HOST_FORCE_KPD        = 0x5,
  CMD_HOST_PING             = 0x6,
  CMD_HOST_PROCESS_ON,
  CMD_HOST_MUTE_OFF,
  CMD_HOST_MUTE_ON,
  CMD_HOST_GET_MONINFO
};

#endif /* S3_HOST_PROTO_DEFS_H */
