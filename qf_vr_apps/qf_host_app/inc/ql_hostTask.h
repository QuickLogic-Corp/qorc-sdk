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
 *    File   : ql_hostTask.h
 *    Purpose: header file for host task for QL smart remote test application 
 *                                                          
 *=========================================================*/

#ifndef __QL_HOSTTASK_H_
#define __QL_HOSTTASK_H_

#if !defined( _EnD_Of_Fw_global_config_h )
#error "Include Fw_global_config.h first"
#endif


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "qlsh_commands.h"

#define CTXT_ISR     (1)
#define CTXT_TASK    (0)

#define HOST_QUEUE_LENGTH (10)
#define HOST_MSGQ_WAIT_TIME (portMAX_DELAY)

/* internal msgs to the host task*/
#define HOST_CMD_READ_DATA_FROM_S3      (21)
#define HOST_CMD_WRTIE_DATA_TO_S3       (22)
#define HOST_SEND_CMD_TO_DEVICE         (23)
#define HOST_LOAD_DEVICE_FW             (24)

// for debug/testing purpose only
#define CMD_DUMMY_1                     (0xF)
#define CHANNEL_DUMMY_1                     (0x5)


/* CMD and events shared between host and device
should be same on both sides.*/
//////// Moved to s3_host_proto_defs.h
#include "s3_host_proto_defs.h"
// event from device to host
//typedef enum HIF_EVT_
//{
//  EVT_KP_DETECTED     = 0x10,
//  EVT_OPUS_PKT_READY  = 0x11,
//  EVT_OPUS_PKT_END    = 0x12,
//  EVT_RAW_PKT_READY   = 0x21,
//  EVT_RAW_PKT_END     = 0x22,
//  EVT_EOT
//}HIF_EVT;
//
//
//// cmd from host to device
//#define CMD_HOST_READY_TO_RECEIVE 0x1
//#define CMD_HOST_PROCESS_OFF      0x2
//#define CMD_HOST_PROCESS_ON       0x3
//#define CMD_HOST_MUTE_OFF      0x4
//#define CMD_HOST_MUTE_ON      0x5

// channel number used for the host - device communication
// chalil - this should be read from Device when sessoin starts, as part of what ??
#define PROTOCOL_CHANNEL_NUMBER_OPUS 9
#define PROTOCOL_CHANNEL_NUMBER_RAW  10
#define PROTOCOL_CHANNEL_NUMBER_DEFAULT PROTOCOL_CHANNEL_NUMBER_RAW

int8_t host_set_rx_channel(int8_t channel);


uint32_t addPktToQueue_Host(struct xQ_Packet *pxMsg, int ctx);
signed portBASE_TYPE StartRtosTaskHost( void);

#endif  //__QL_HOSTTASK_H_