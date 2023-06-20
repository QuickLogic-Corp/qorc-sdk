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
//#include "qlsh_commands.h"

#if FEATURE_CLI_DEBUG_INTERFACE

static void togglegreenled(const struct cli_cmd_entry *pEntry)
{
    static bool fLit = false;
    uint32_t temp;
    (void)pEntry;
    fLit = !fLit;
    // Disable IO24 output; red LED
    temp = (*( volatile unsigned int *) (0x40004c60));
    (*( volatile unsigned int *) (0x40004c60)) = temp | 0x20;
    // Enable IO6 output; green LED
    temp = (*( volatile unsigned int *) (0x40004c18));
    (*( volatile unsigned int *) (0x40004c18)) = temp & 0xffffffdf;
    HAL_GPIO_Write(0, fLit);
    return;
}

static void toggleredled(const struct cli_cmd_entry *pEntry)
{
    static bool fLit = false;
    uint32_t temp;
    (void)pEntry;
    fLit = !fLit;
    // Disable IO6 output; green LED
    temp = (*( volatile unsigned int *) (0x40004c18));
    (*( volatile unsigned int *) (0x40004c18)) = temp | 0x20;
    // Enable IO24 output; red LED
    temp = (*( volatile unsigned int *) (0x40004c60));
    (*( volatile unsigned int *) (0x40004c60)) = temp & 0xffffffdf;
    HAL_GPIO_Write(0, fLit);
    return;
}

static void toggleblueled(const struct cli_cmd_entry *pEntry)
{
    static bool fLit = false;
    (void)pEntry;
    fLit = !fLit;
    HAL_GPIO_Write(3, fLit);
    return;
}

static void toggletp1(const struct cli_cmd_entry *pEntry)
{
    uint32_t temp = 0;
    (void)pEntry;
    // read FPGA SCRATCH register to determine current setting values for TPs
    temp = (*( volatile unsigned int *) (0x40020008));
    if( (temp & 0x1000) == 0x1000 )
    {
    	(*( volatile unsigned int *) (0x40020008)) = (temp & 0xeeff) | 0x0100;
    	dbg_str( "set touchpad1 to 0\n");
    }
    else
    {
    	(*( volatile unsigned int *) (0x40020008)) = (temp & 0xeeff) | 0x1100;
    	dbg_str( "set touchpad1 to 1\n");
    }

    return;
}

static void toggletp2(const struct cli_cmd_entry *pEntry)
{
    uint32_t temp = 0;
    (void)pEntry;
    // read FPGA SCRATCH register to determine current setting values for TPs
    temp = (*( volatile unsigned int *) (0x40020008));
    if( (temp & 0x2000) == 0x2000 )
    {
    	(*( volatile unsigned int *) (0x40020008)) = (temp & 0xddff) | 0x0200;
    	dbg_str( "set touchpad2 to 0\n");
    }
    else
    {
    	(*( volatile unsigned int *) (0x40020008)) = (temp & 0xddff) | 0x2200;
    	dbg_str( "set touchpad2 to 1\n");
    }

    return;
}

static void toggletp3(const struct cli_cmd_entry *pEntry)
{
    uint32_t temp = 0;
    (void)pEntry;
    // read FPGA SCRATCH register to determine current setting values for TPs
    temp = (*( volatile unsigned int *) (0x40020008));
    if( (temp & 0x4000) == 0x4000 )
    {
    	(*( volatile unsigned int *) (0x40020008)) = (temp & 0xbbff) | 0x0400;
    	dbg_str( "set touchpad3 to 0\n");
    }
    else
    {
    	(*( volatile unsigned int *) (0x40020008)) = (temp & 0xbbff) | 0x4400;
    	dbg_str( "set touchpad3 to 1\n");
    }

    return;
}

static void toggletp4(const struct cli_cmd_entry *pEntry)
{
    uint32_t temp = 0;
    (void)pEntry;
    // read FPGA SCRATCH register to determine current setting values for TPs
    temp = (*( volatile unsigned int *) (0x40020008));
    if( (temp & 0x8000) == 0x8000 )
    {
    	(*( volatile unsigned int *) (0x40020008)) = (temp & 0x77ff) | 0x0800;
    	dbg_str( "set touchpad4 to 0\n");
    }
    else
    {
    	(*( volatile unsigned int *) (0x40020008)) = (temp & 0x77ff) | 0x8800;
    	dbg_str( "set touchpad4 to 1\n");
    }

    return;
}

static void setalltplow(const struct cli_cmd_entry *pEntry)
{
    uint32_t temp = 0;
    (void)pEntry;
    // read FPGA SCRATCH register to determine current setting values for TPs
    temp = (*( volatile unsigned int *) (0x40020008));
    (*( volatile unsigned int *) (0x40020008)) = (temp & 0x00ff) | 0x0F00;
    dbg_str( "set all touchpads to 0\n");

    return;
}

static void readtps(const struct cli_cmd_entry *pEntry)
{
    uint32_t temp = 0;
    (void)pEntry;
    // read FPGA SCRATCH register to determine current setting values for TPs
    temp = (*( volatile unsigned int *) (0x40020008));
    (*( volatile unsigned int *) (0x40020008)) = (temp & 0x00ff);
    temp = (*( volatile unsigned int *) (0x40020008));
    dbg_str( "disable touchpads output!\n");
    dbg_str_hex32( "touchpads value: ", temp);
    dbg_str( "\n");

    return;
}

// Function to reset the HW and execute boot-from-flash
static void hwreset(const struct cli_cmd_entry *pEntry)
{
	dbg_str( "perform system HW reset\n");
	NVIC_SystemReset();
    return;
}

const struct cli_cmd_entry qf_diagnostic[] =
{
    CLI_CMD_SIMPLE( "red", toggleredled, "toggle red led" ),
    CLI_CMD_SIMPLE( "green", togglegreenled, "toggle green led" ),
    CLI_CMD_SIMPLE( "blue", toggleblueled, "toggle blue led" ),
	CLI_CMD_SIMPLE( "tp1", toggletp1, "toggle touch pad 1" ),
	CLI_CMD_SIMPLE( "tp2", toggletp2, "toggle touch pad 2" ),
	CLI_CMD_SIMPLE( "tp3", toggletp3, "toggle touch pad 3" ),
	CLI_CMD_SIMPLE( "tp4", toggletp4, "toggle touch pad 4" ),
	CLI_CMD_SIMPLE( "setalltp0", setalltplow, "set all touch pads to 0" ),
	CLI_CMD_SIMPLE( "readtps", readtps, "display touch value" ),
	CLI_CMD_SIMPLE( "systemreset", hwreset, "reset system" ),
    CLI_CMD_TERMINATE()
};

const struct cli_cmd_entry my_main_menu[] = {
    CLI_CMD_SUBMENU( "diag", qf_diagnostic, "Qomu diagnostic commands" ),
    CLI_CMD_TERMINATE()
};

#endif
