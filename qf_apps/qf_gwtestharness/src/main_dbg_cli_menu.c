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
#include "s3x_clock_hal.h"

#if FEATURE_CLI_DEBUG_INTERFACE


//-------------------------------------------------
//
//   GW test harness
//
//-------------------------------------------------

//------------- Pointer to registers ---------------------//
volatile uint8_t* pgwregs = (volatile uint8_t*)(FPGA_PERIPH_BASE);

//------------- Local functions --------------------------//
static void readid(const struct cli_cmd_entry *pEntry);
static void readreg(const struct cli_cmd_entry *pEntry);
static void writereg(const struct cli_cmd_entry *pEntry);
static void readregrel(const struct cli_cmd_entry *pEntry);
static void writeregrel(const struct cli_cmd_entry *pEntry);

static void setbaseaddr(const struct cli_cmd_entry *pEntry);
static void readregx(const struct cli_cmd_entry *pEntry);
static void writeregx(const struct cli_cmd_entry *pEntry);

static void setclk16(const struct cli_cmd_entry *pEntry);
static void setclk21(const struct cli_cmd_entry *pEntry);

static void null(const struct cli_cmd_entry *pEntry);   // Dummy to enable adding breaks to help

//------------- Local variables --------------------------//
uint32_t	uxBaseaddr = 0x00;    // Base address for current GW

const struct cli_cmd_entry qf_gwtest[] =
{
    CLI_CMD_SIMPLE( "readid", readid,           "read the gateware id reister (@0x00)" ),
    CLI_CMD_SIMPLE( "readreg", readreg,         "0xaddr -- read register at specified address"),
    CLI_CMD_SIMPLE( "rr", readreg,              "0xaddr -- read register at specified address"),
    CLI_CMD_SIMPLE( "writereg", writereg,       "0xaddr 0xvalue -- write val to register at address"),
    CLI_CMD_SIMPLE( "wr", writereg,             "0xaddr 0xvalue -- write val to regsiter at address"),
    CLI_CMD_SIMPLE( "clk16", setclk16,          "value -- set clk16 to specified frequency"),
    CLI_CMD_SIMPLE( "clk21", setclk21,          "value -- set clk21 to specified frequency"),

    CLI_CMD_SIMPLE ( " ", null,            " "),
    CLI_CMD_SIMPLE ( "------", null,            "all commands after this use the baseaddr to modify the address---"),
    CLI_CMD_SIMPLE ( " ", null,            " "),
    
    CLI_CMD_SIMPLE( "baseaddr", setbaseaddr,    "0xaddr -- set base address for GW"),CLI_CMD_SIMPLE( "readregrel", readregrel,   "0xaddr -- read register at address + baseaddr"),
    
    CLI_CMD_SIMPLE( "rrr", readregrel,          "0xaddr -- read register at address + baseaddr"),
    CLI_CMD_SIMPLE( "writeregrel", writeregrel, "0xaddr 0xvalue -- write val to register at address + baseaddr"),
    CLI_CMD_SIMPLE( "wrr", writeregrel,         "0xaddr 0xvalue -- write val to register at address + baseaddr"),

    CLI_CMD_WITH_ARG( "rr0", readregx, 0x00,    "read register at offset 00 from base addr"),
    CLI_CMD_WITH_ARG( "rr4", readregx, 0x04,    "read register at offset 04 from base addr"),
    CLI_CMD_WITH_ARG( "rr8", readregx, 0x08,    "read register at offset 08 from base addr"),
    CLI_CMD_WITH_ARG( "rrc", readregx, 0x0c,    "read register at offset 0C from base addr"),
    CLI_CMD_WITH_ARG( "rr10", readregx, 0x10,   "read register at offset 10 from base addr"),
    CLI_CMD_WITH_ARG( "rr14", readregx, 0x14,   "read register at offset 14 from base addr"),
    CLI_CMD_WITH_ARG( "rr18", readregx, 0x18,   "read register at offset 18 from base addr"),
    CLI_CMD_WITH_ARG( "rr1c", readregx, 0x1c,   "read register at offset 1C from base addr"),
    
    CLI_CMD_WITH_ARG( "wr0", writeregx, 0x00,   "value -- write value to register at offset 00 from base addr"),
    CLI_CMD_WITH_ARG( "wr4", writeregx, 0x04,   "value -- write value to register at offset 04 from base addr"),
    CLI_CMD_WITH_ARG( "wr8", writeregx, 0x08,   "value -- write value to register at offset 08 from base addr"),
    CLI_CMD_WITH_ARG( "wrc", writeregx, 0x0c,   "value -- write value to register at offset 0C from base addr"),
	

    CLI_CMD_TERMINATE()
};

static void null(const struct cli_cmd_entry *pEntry)
{
    return;
}

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

static void readregrel(const struct cli_cmd_entry *pEntry)
{
    uint32_t            uxAddress;
    uint32_t            uxValue;
    volatile uint8_t*   pgwreg;
    
    (void)pEntry;
    // Add functionality here
    CLI_uint32_getshow( "address", &uxAddress );
    pgwreg = pgwregs + uxBaseaddr + uxAddress;
    uxValue = *((volatile uint32_t*)pgwreg);
    dbg_str_hex32("register at    ", uxBaseaddr + uxAddress);
    dbg_str_hex32("value      ", uxValue);
    return;
}

static void writeregrel(const struct cli_cmd_entry *pEntry)
{
    uint32_t            uxAddress;
    uint32_t            uxValue;
    volatile uint8_t*   pgwreg;
    
    (void)pEntry;
    // Add functionality here
    CLI_uint32_getshow( "address", &uxAddress );
    CLI_uint32_getshow( "value", &uxValue );
    pgwreg = pgwregs + uxBaseaddr + uxAddress;
    *((volatile uint32_t*)pgwreg) = uxValue;
    dbg_str_hex32("register at    ", uxBaseaddr + uxAddress);
    dbg_str_hex32("register set to", uxValue);
    return;
}

static void setbaseaddr(const struct cli_cmd_entry *pEntry)
{
    (void)pEntry;
    // Add functionality here
    CLI_uint32_getshow( "baseaddr", &uxBaseaddr );
    dbg_str_hex32("baseaddr      ", uxBaseaddr);
    return;
}


static void readregx(const struct cli_cmd_entry *pEntry)
{
    volatile uint8_t*   pgwreg;
    uint32_t            uxAddr;
    uint32_t            uxValue;
    
    uxAddr = pEntry->cookie;
    // Add functionality here
    pgwreg = pgwregs + uxBaseaddr + uxAddr;
    uxValue = *((volatile uint32_t*)pgwreg);
    dbg_str_hex32("register at", uxBaseaddr + uxAddr);
    dbg_str_hex32("value      ", uxValue);
    return;
}

static void writeregx(const struct cli_cmd_entry *pEntry)
{
    uint32_t            uxAddr;
    uint32_t            uxValue;
    volatile uint8_t*   pgwreg;
    
    uxAddr = pEntry->cookie;
    // Add functionality here
    CLI_uint32_getshow( "value", &uxValue );
    pgwreg = pgwregs + uxBaseaddr + uxAddr;
    *((volatile uint32_t*)pgwreg) = uxValue;
    dbg_str_hex32("register at    ", uxBaseaddr + uxAddr);
    dbg_str_hex32("register set to", uxValue);
    return;
}

static void setclk16(const struct cli_cmd_entry *pEntry)
{
    uint32_t            uxValue;
    volatile uint8_t*   pgwreg;
    
    (void)pEntry;
    // Add functionality here
    CLI_uint32_getshow( "freq", &uxValue );
    dbg_str_int("freq", uxValue);
	S3x_Clk_Set_Rate(S3X_FB_16_CLK, uxValue);
    return;
}

static void setclk21(const struct cli_cmd_entry *pEntry)
{
    uint32_t            uxValue;
    volatile uint8_t*   pgwreg;
    
    (void)pEntry;
    // Add functionality here
    CLI_uint32_getshow( "freq", &uxValue );
    dbg_str_int("freq", (int32_t)uxValue);
    
	S3x_Clk_Set_Rate(S3X_FB_21_CLK, uxValue);
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
