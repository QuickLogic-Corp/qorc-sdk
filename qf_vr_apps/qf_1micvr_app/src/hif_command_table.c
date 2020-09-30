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

#include "s3_host_proto_defs.h"
#include "host_interface.h"
#include "ql_controlTask.h"
#include "d2h_protocol.h"
#include "RtosTask.h"

///////////// Debug Trace ////////////////////////////////
#include "dbgtrace.h"
#define             K_DBGTRACE_HIF  200
extern dbgtrace_t   adbgtraceHIF[K_DBGTRACE_HIF];
extern int          idbgtraceHIF;

void hifcb_cmd_host_ready_to_receive(void *pdata, int len);
void hifcb_cmd_host_process_off(void *pdata, int len);
void hifcb_cmd_host_process_on(void *pdata, int len);
void hifcb_cmd_host_mute_off(void *pdata, int len);
void hifcb_cmd_host_mute_on(void *pdata, int len);
void hifcb_cmd_host_threshold(void *pdata, int len);
void hifcb_cmd_host_force_kpd(void *pdata, int len);
void hifcb_cmd_host_ping(void *pdata, int len);
void hifcb_cmd_host_get_moninfo(void *pdata, int len);

hif_command_table_t hif_command_table[] =
{
  { CMD_HOST_READY_TO_RECEIVE, hifcb_cmd_host_ready_to_receive },
  { CMD_HOST_PROCESS_OFF,      hifcb_cmd_host_process_off      },
  { CMD_HOST_STOP_STREAMING,   hifcb_cmd_host_process_off      },
  { CMD_HOST_PROCESS_ON,       hifcb_cmd_host_process_on       },
  { CMD_HOST_MUTE_OFF,         hifcb_cmd_host_mute_off         },
  { CMD_HOST_MUTE_ON,          hifcb_cmd_host_mute_on          },
  { CMD_HOST_THRESHOLD,        hifcb_cmd_host_threshold        },
  { CMD_HOST_FORCE_KPD,        hifcb_cmd_host_process_on       },
  { CMD_HOST_PING,             hifcb_cmd_host_ping             },
  { CMD_HOST_GET_MONINFO,      hifcb_cmd_host_get_moninfo      },
  { 0,                         NULL                            } // Indicates end of table
};
int hif_command_table_size = sizeof(hif_command_table)/sizeof(hif_command_table[0]);
void hifcb_cmd_host_ready_to_receive(void *pdata, int len)
{
    struct xCQ_Packet CQpacket;
     CQpacket.ceEvent = CEVENT_HOST_READY_TO_RECEIVE;
        //Release semaphore to unblock sending opus chunks to host. TODO!!
        if (xSemaphoreGive(g_host_ready_lock) != pdTRUE)
        {
            dbgtracePrint(adbgtraceHIF, K_DBGTRACE_HIF, idbgtraceHIF);
            dbg_fatal_error("[Host Interface] : Error : unable to release lock to g_host_ready_lock\n");
        }
     addPktToControlQueue(&CQpacket);
}

void hifcb_cmd_host_process_off(void *pdata, int len)
{
    struct xCQ_Packet CQpacket;
     CQpacket.ceEvent = CEVENT_HOST_PROCESS_OFF;
     addPktToControlQueue(&CQpacket);
}

void hifcb_cmd_host_process_on(void *pdata, int len)
{
    struct xCQ_Packet CQpacket;
     CQpacket.ceEvent = CEVENT_HOST_PROCESS_ON;
     addPktToControlQueue(&CQpacket);
}

void hifcb_cmd_host_mute_off(void *pdata, int len)
{
    struct xCQ_Packet CQpacket;
     CQpacket.ceEvent = CEVENT_HOST_MUTE_OFF;
     addPktToControlQueue(&CQpacket);
}

void hifcb_cmd_host_mute_on(void *pdata, int len)
{
    struct xCQ_Packet CQpacket;
     CQpacket.ceEvent = CEVENT_HOST_MUTE_ON;
     addPktToControlQueue(&CQpacket);
}

void hifcb_cmd_host_threshold(void *pdata, int len)
{
     //roku_i2c_host_threshold_interface((uint8_t *)pdata);
}

void hifcb_cmd_host_force_kpd(void *pdata, int len)
{
     dbg_str("Received CMD_HOST_FORCE_KPD\n");
     //force_kpd();
}

void hifcb_cmd_host_ping(void *pdata, int len)
{
     dbg_str("Received CMD_HOST_PING\n");
}

void hifcb_cmd_host_get_moninfo(void *pdata, int len)
{
    dbg_str("Received CMD_HOST_GET_MONINFO\n");
     struct xQ_Packet packet={0};
     packet.ucCommand = MESSAGE_GET_MONINFO;
     packet.ucSrc = HOSTIF_TASK_MESSAGE;
     xQueueSend( xHandleQueueHostIf, ( void * )&packet, 0 );
}
