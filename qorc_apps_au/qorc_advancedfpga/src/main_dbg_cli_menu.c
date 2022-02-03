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
 *    File   : main.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <eoss3_hal_gpio.h>
#include "cli.h"
#include <stdbool.h>
#include "dbg_uart.h"
#include "fpga_ledctlr.h"

#if FEATURE_CLI_DEBUG_INTERFACE

#if 0 //DEBUG_H2D_PROTOCOL
#include "ql_hostTask.h"
#include "h2d_protocol.h"


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

#if 0
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

#if 0
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
#endif
//-------------------------------------------------
//
//   LED Controller CLI
//
//-------------------------------------------------

//------------- Local functions -------------------
static void set_color0(const struct cli_cmd_entry *pEntry);
static void set_color1(const struct cli_cmd_entry *pEntry);
static void set_color2(const struct cli_cmd_entry *pEntry);
static void set_color3(const struct cli_cmd_entry *pEntry);

static void set_duration0(const struct cli_cmd_entry *pEntry);
static void set_duration1(const struct cli_cmd_entry *pEntry);
static void set_duration2(const struct cli_cmd_entry *pEntry);
static void set_duration3(const struct cli_cmd_entry *pEntry);

const struct cli_cmd_entry qf_ledctlr[] =
{
    CLI_CMD_SIMPLE( "color0", set_color0, "set color for time slot 0" ),
	CLI_CMD_SIMPLE( "color1", set_color1, "set color for time slot 1" ),
	CLI_CMD_SIMPLE( "color2", set_color2, "set color for time slot 2" ),
	CLI_CMD_SIMPLE( "color3", set_color3, "set color for time slot 3" ),
	
	CLI_CMD_SIMPLE( "duration0", set_duration0, "set duration for time slot 0" ),
	CLI_CMD_SIMPLE( "duration1", set_duration1, "set duration for time slot 1" ),
	CLI_CMD_SIMPLE( "duration2", set_duration2, "set duration for time slot 2" ),
	CLI_CMD_SIMPLE( "duration3", set_duration3, "set duration for time slot 3" ),

    CLI_CMD_TERMINATE()
};

uint8_t ucColor0 = 0;
static void set_color0(const struct cli_cmd_entry *pEntry)
{
    
    (void)pEntry;
        // Add functionality here
	ucColor0 = fpga_ledctlr_getcolor(0);
    CLI_uint8_getshow( "color0", &ucColor0 );
    fpga_ledctlr_setcolor(ucColor0, 0);
    return;
}

uint8_t ucColor1 = 0;
static void set_color1(const struct cli_cmd_entry *pEntry)
{
    
    (void)pEntry;
        // Add functionality here
	ucColor1 = fpga_ledctlr_getcolor(1);
    CLI_uint8_getshow( "color1", &ucColor1 );
    fpga_ledctlr_setcolor(ucColor1, 1);
    return;
}

uint8_t ucColor2 = 0;
static void set_color2(const struct cli_cmd_entry *pEntry)
{
    
    (void)pEntry;
        // Add functionality here
	ucColor2 = fpga_ledctlr_getcolor(2);
    CLI_uint8_getshow( "color2", &ucColor2 );
    fpga_ledctlr_setcolor(ucColor2, 2);
    return;
}

uint8_t ucColor3 = 0;
static void set_color3(const struct cli_cmd_entry *pEntry)
{
    
    (void)pEntry;
        // Add functionality here
	ucColor3 = fpga_ledctlr_getcolor(3);
    CLI_uint8_getshow( "color3", &ucColor3 );
    fpga_ledctlr_setcolor(ucColor3, 3);
    return;
}

uint16_t uhDuration0 = 0;
static void set_duration0(const struct cli_cmd_entry *pEntry)
{
    
    (void)pEntry;
        // Add functionality here
	uhDuration0 = fpga_ledctlr_getduration(0);
    CLI_uint16_getshow( "duration0", &uhDuration0 );
    fpga_ledctlr_setduration(uhDuration0, 0);
    return;
}

uint16_t uhDuration1 = 0;
static void set_duration1(const struct cli_cmd_entry *pEntry)
{
    
    (void)pEntry;
        // Add functionality here
	uhDuration1 = fpga_ledctlr_getduration(1);
    CLI_uint16_getshow( "duration1", &uhDuration1 );
    fpga_ledctlr_setduration(uhDuration0, 1);
    return;
}

uint16_t uhDuration2 = 0;
static void set_duration2(const struct cli_cmd_entry *pEntry)
{
    
    (void)pEntry;
        // Add functionality here
	uhDuration2 = fpga_ledctlr_getduration(2);
    CLI_uint16_getshow( "duration2", &uhDuration2 );
    fpga_ledctlr_setduration(uhDuration2, 2);
    return;
}

uint16_t uhDuration3 = 0;
static void set_duration3(const struct cli_cmd_entry *pEntry)
{
    
    (void)pEntry;
        // Add functionality here
	uhDuration3 = fpga_ledctlr_getduration(3);
    CLI_uint16_getshow( "duration3", &uhDuration3 );
    fpga_ledctlr_setduration(uhDuration3, 3);
    return;
}


//-------------------------------------------------
//
//   Diagnostic CLI
//
//-------------------------------------------------
static void togglegreenled(const struct cli_cmd_entry *pEntry)
{
    static bool fLit = false;
    (void)pEntry;
    fLit = !fLit;
    HAL_GPIO_Write(5, fLit);
    return;
}

static void toggleredled(const struct cli_cmd_entry *pEntry)
{
    static bool fLit = false;
    (void)pEntry;
    fLit = !fLit;
    HAL_GPIO_Write(6, fLit);
    return;
}

static void toggleblueled(const struct cli_cmd_entry *pEntry)
{
    static bool fLit = false;
    (void)pEntry;
    fLit = !fLit;
    HAL_GPIO_Write(4, fLit);
    return;
}

static void userbutton(const struct cli_cmd_entry *pEntry)
{
    uint8_t ucVal;
    (void)pEntry;
 
    HAL_GPIO_Read(0, &ucVal);
    if (ucVal) {
        CLI_puts("Not pressed");
    } else {
         CLI_puts("Pressed");
    }
    return;
}

const struct cli_cmd_entry qf_diagnostic[] =
{
    CLI_CMD_SIMPLE( "red", toggleredled, "toggle red led" ),
    CLI_CMD_SIMPLE( "green", togglegreenled, "toggle green led" ),
    CLI_CMD_SIMPLE( "blue", toggleblueled, "toggle blue led" ),
    CLI_CMD_SIMPLE( "userbutton", userbutton, "show state of user button" ),
    CLI_CMD_TERMINATE()
};

const struct cli_cmd_entry my_main_menu[] = {
	CLI_CMD_SUBMENU( "ledctlr", qf_ledctlr, "FPGA led controller" ),
    CLI_CMD_SUBMENU( "diag", qf_diagnostic, "QuickFeather diagnostic commands" ),
    CLI_CMD_TERMINATE()
};

#endif
