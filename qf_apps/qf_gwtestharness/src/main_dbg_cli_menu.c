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

#if FEATURE_CLI_DEBUG_INTERFACE


//-------------------------------------------------
//
//   GW test harness
//
//-------------------------------------------------

//------------- Pointer to registers ---------------------//
volatile uint8_t* pgwregs = (volatile uint8_t*)(FPGA_PERIPH_BASE);

//------------- Local functions -------------------
static void readid(const struct cli_cmd_entry *pEntry);
static void readreg(const struct cli_cmd_entry *pEntry);
static void writereg(const struct cli_cmd_entry *pEntry);

const struct cli_cmd_entry qf_gwtest[] =
{
    CLI_CMD_SIMPLE( "readid", readid, "-- read the gateware id reister (@0x00)" ),
    CLI_CMD_SIMPLE( "readreg", readreg, " 0xaddr -- read register at specified address"),
    CLI_CMD_SIMPLE( "writereg", writereg, "0xaddr 0xvalue -- write val to specified address"),

    CLI_CMD_TERMINATE()
};

static void readid(const struct cli_cmd_entry *pEntry)
{
    uint32_t            uxAddress;
    uint32_t            uxValue;
    volatile uint8_t*   pgwreg;
    
    (void)pEntry;
    // Add functionality here
    pgwreg = pgwregs + 0x00;
    uxValue = *((volatile uint32_t*)pgwreg);
    dbg_str_hex32("id register =", uxValue);
    return;
}

static void readreg(const struct cli_cmd_entry *pEntry)
{
    uint32_t            uxAddress;
    uint32_t            uxValue;
    volatile uint8_t*   pgwreg;
    
    (void)pEntry;
    // Add functionality here
    CLI_uint32_getshow( "address", &uxAddress );
    pgwreg = pgwregs + uxAddress;
    uxValue = *((volatile uint32_t*)pgwreg);
    dbg_str_hex32("register at", uxAddress);
    dbg_str_hex32("value      ", uxValue);
    return;
}

static void writereg(const struct cli_cmd_entry *pEntry)
{
    uint32_t            uxAddress;
    uint32_t            uxValue;
    volatile uint8_t*   pgwreg;
    
    (void)pEntry;
    // Add functionality here
    CLI_uint32_getshow( "address", &uxAddress );
    CLI_uint32_getshow( "value", &uxValue );
    dbg_str_hex32("uxValue", uxValue);
    pgwreg = pgwregs + uxAddress;
    *((volatile uint32_t*)pgwreg) = uxValue;
    dbg_str_hex32("register at    ", uxAddress);
    dbg_str_hex32("register set to", uxValue);
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
	CLI_CMD_SUBMENU( "gwtest", qf_gwtest, "General test harness for gateware" ),
    CLI_CMD_SUBMENU( "diag", qf_diagnostic, "QuickFeather diagnostic commands" ),
    CLI_CMD_TERMINATE()
};

#endif
