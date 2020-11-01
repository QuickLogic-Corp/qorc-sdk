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
 *    File   : main_dbg_cli_menu.c
 *    Purpose:
 *
 *=========================================================*/

#include "Fw_global_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"

#include "dbg_uart.h"
#include "ql_hostTask.h"
#include "h2d_protocol.h"
#include "qlsh_commands.h"

#if FEATURE_CLI_DEBUG_INTERFACE

#if 1 //DEBUG_H2D_PROTOCOL


static void GenerateInterruptToHost(const struct cli_cmd_entry *pEntry)
{
    (void)pEntry;
    // Add functionality here
    return;
}

static void GenerateInterruptToS3(const struct cli_cmd_entry *pEntry)
{
    (void)pEntry;
        // Add functionality here
    generate_interrupt_to_device();
    
    return;
}

static void ClearInterruptToS3(const struct cli_cmd_entry *pEntry)
{
    (void)pEntry;
        // Add functionality here    
    clear_interrupt_to_device();
    
    return;
}

static void ReadDataFromS3(const struct cli_cmd_entry *pEntry)
{
    (void)pEntry;
        // Add functionality here
    dbg_str("sending HOST_CMD_READ_DATA_FROM_S3 cmd to host task. \n");
    struct xQ_Packet hostMsg;
    hostMsg.ucCommand = HOST_CMD_READ_DATA_FROM_S3;
    addPktToQueue_Host(&hostMsg, CTXT_TASK);
    
    return;
}
static void WriteDataToS3(const struct cli_cmd_entry *pEntry)
{
    (void)pEntry;
        // Add functionality here
    dbg_str("sent HOST_CMD_WRTIE_DATA_TO_S3 cmd to host task. \n");
    struct xQ_Packet hostMsg;
    hostMsg.ucCommand = HOST_CMD_WRTIE_DATA_TO_S3;
    addPktToQueue_Host(&hostMsg, CTXT_TASK);
    
    return;
}

static void send_cmd(const struct cli_cmd_entry *pEntry)
{
        // Add functionality here
    dbg_str_int("CMD_HOST", pEntry->cookie);
    struct xQ_Packet hostMsg;
    hostMsg.ucCommand = HOST_SEND_CMD_TO_DEVICE;
    hostMsg.ucData[0] = pEntry->cookie;
    addPktToQueue_Host(&hostMsg, CTXT_TASK);
    
    return;
}
#endif

#if 1
extern int opus_test_en;
static void opus_test_on(const struct cli_cmd_entry *pEntry)
{
    (void)pEntry;
        // Add functionality here
    dbg_str("opus test on \n");
    opus_test_en = 1;
    
    return;
}

static void opus_test_off(const struct cli_cmd_entry *pEntry)
{
    (void)pEntry;
        // Add functionality here
    dbg_str("opus test off \n");
    opus_test_en = 0;
    
    return;
}
#endif
extern void display_rx_buf_addr_size(void);

static void rx_buf_addr(const struct cli_cmd_entry *pEntry)
{
    (void)pEntry;
        // Add functionality here
    display_rx_buf_addr_size();
    return;
}
uint8_t ch_number = 0;
static void set_rx_channel(const struct cli_cmd_entry *pEntry)
{
    
    (void)pEntry;
        // Add functionality here
    CLI_uint8_getshow( "ch_number", &ch_number );
    host_set_rx_channel(ch_number);
    return;
}

const struct cli_cmd_entry host_cmd[] =
{
    CLI_CMD_SIMPLE( "intr_s3", GenerateInterruptToS3, "to generate software interrupt to slave s3" ),
    CLI_CMD_SIMPLE( "clear_intr_s3", ClearInterruptToS3, "to generate software interrupt to slave s3" ),
    CLI_CMD_SIMPLE( "intr_host", GenerateInterruptToHost, "to simulate the scenario when AP is interrupted by Slave S3" ),
    CLI_CMD_SIMPLE( "read_data", ReadDataFromS3, "To read data from slave S3 over SPI " ),
    CLI_CMD_SIMPLE( "write_data", WriteDataToS3, "To write data from slave S3 over SPI " ),
    CLI_CMD_WITH_ARG( "dy2rcv", send_cmd, CMD_HOST_READY_TO_RECEIVE, "to send CMD_HOST_READY_TO_RECEIVE to the device" ),
    CLI_CMD_WITH_ARG( "off", send_cmd, CMD_HOST_PROCESS_OFF, "to send CMD_HOST_PROCESS_OFF to the device" ),
    CLI_CMD_WITH_ARG( "on", send_cmd, CMD_HOST_PROCESS_ON, "to send CMD_HOST_PROCESS_ON to the device" ),
    CLI_CMD_WITH_ARG( "unmute", send_cmd, CMD_HOST_MUTE_OFF, "to send CMD_HOST_MUTE_OFF to the device" ),
    CLI_CMD_WITH_ARG( "mute", send_cmd, CMD_HOST_MUTE_ON, "to send CMD_HOST_MUTE_ON to the device" ),
    CLI_CMD_WITH_ARG( "get_moninfo", send_cmd, CMD_HOST_GET_MONINFO, "Get current heap size, queue size... from device" ),
    CLI_CMD_TERMINATE()
};

const struct cli_cmd_entry my_main_menu[] = {
    CLI_CMD_SIMPLE( "opus_test_on", opus_test_on, "to enable the opus test. First mem dump after KPD availbale " ),
    CLI_CMD_SIMPLE( "opus_test_off", opus_test_off, "to disable the opus test mode. " ), 
    CLI_CMD_SIMPLE( "rx_buf_addr", rx_buf_addr, "function to get rx buffer address and size for memory dump. " ),
    CLI_CMD_SIMPLE( "set_rx_channel", set_rx_channel, "function to set the rx channel. 1 - for OPUS, 10 - for RAW PCM. " ),
    CLI_CMD_SUBMENU( "host_cmd", host_cmd, "ql smart remote host side menu" ),
    CLI_CMD_TERMINATE()
};

#endif