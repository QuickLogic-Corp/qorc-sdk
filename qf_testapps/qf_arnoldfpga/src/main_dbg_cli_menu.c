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
#include "fpga_arnold_control.h"

#if FEATURE_CLI_DEBUG_INTERFACE



//-------------------------------------------------
//
//   Arnold Controller CLI
//
//-------------------------------------------------

//------------- Local functions -------------------
static void set_reset(const struct cli_cmd_entry *pEntry);
static void clr_reset(const struct cli_cmd_entry *pEntry);
static void toggle_reset(const struct cli_cmd_entry *pEntry);
static void print_control(const struct cli_cmd_entry *pEntry);
static void set_clkdiv(const struct cli_cmd_entry *pEntry);
static void print_clkdiv(const struct cli_cmd_entry *pEntry);
static void print_sig(const struct cli_cmd_entry *pEntry);
static void print_rev(const struct cli_cmd_entry *pEntry);
static void set_scratch(const struct cli_cmd_entry *pEntry);
static void print_scratch(const struct cli_cmd_entry *pEntry);

const struct cli_cmd_entry arnold_control[] =
{
	CLI_CMD_SIMPLE( "rstlo", set_reset, "assert reset (drive low)" ),
	CLI_CMD_SIMPLE( "rst", set_reset, "assert reset (drive low)" ),
	CLI_CMD_SIMPLE( "rsthi", clr_reset, "release reset (drive high)" ),
	CLI_CMD_SIMPLE( "run", clr_reset, "release reset (drive high)" ),
	CLI_CMD_SIMPLE( "t", toggle_reset, "toggle reset (drive low, then drive high)" ),
	CLI_CMD_SIMPLE( "control", print_control, "print control register" ),
	CLI_CMD_SIMPLE( "setdiv", set_clkdiv, "set clock divider)" ),
	CLI_CMD_SIMPLE( "div", print_clkdiv, "print clock divider)" ),
	CLI_CMD_SIMPLE( "sig", print_sig, "print signature" ),
	CLI_CMD_SIMPLE( "rev", print_rev, "print revision" ),
	CLI_CMD_SIMPLE( "setscratch", set_scratch, "set scratch" ),
	CLI_CMD_SIMPLE( "scratch", print_scratch, "print scratch" ),


    CLI_CMD_TERMINATE()
};

static void set_reset(const struct cli_cmd_entry *pEntry) {
	fpga_arnold_set_control(0x0);
}

static void clr_reset(const struct cli_cmd_entry *pEntry) {
	fpga_arnold_set_control(0x1);
}

static void toggle_reset(const struct cli_cmd_entry *pEntry) {
	for (int i = 0; i != 10; i++) {
		fpga_arnold_set_control(0x0);
	}
	fpga_arnold_set_control(0x1);
}

static void print_control(const struct cli_cmd_entry *pEntry) {
	uint32_t	uxcontrol = fpga_arnold_get_control();
	dbg_str_hex32("Control =", uxcontrol);
}

static void set_clkdiv(const struct cli_cmd_entry *pEntry) {
	static uint32_t uxClkdiv;
	uxClkdiv = fpga_arnold_get_clkdiv();
	CLI_uint32_getshow( "clkdiv", &uxClkdiv );
	fpga_arnold_set_clkdiv(uxClkdiv);
}

static void print_clkdiv(const struct cli_cmd_entry *pEntry) {
	uint32_t	uxclkdiv = fpga_arnold_get_clkdiv();
	dbg_str_hex32("clkdiv =", uxclkdiv);
}
static void print_sig(const struct cli_cmd_entry *pEntry) {
	uint32_t	uxsig = fpga_arnold_get_signature();
	dbg_str_hex32("Signature =", uxsig);
}

static void print_rev(const struct cli_cmd_entry *pEntry) {
	uint32_t	uxrev = fpga_arnold_get_revision();
	dbg_str_hex32("Revision =", uxrev);
}


static void set_scratch(const struct cli_cmd_entry *pEntry) {
	static uint32_t uxScratch;
	CLI_uint32_getshow( "scratch", &uxScratch );
	fpga_arnold_set_scratch(uxScratch);
}

static void print_scratch(const struct cli_cmd_entry *pEntry) {
	uint32_t	uxScratch = fpga_arnold_get_scratch();
	dbg_str_hex32("Scratch =", uxScratch);
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
	CLI_CMD_SUBMENU( "arnold", arnold_control, "Arnold controller" ),
    CLI_CMD_SUBMENU( "diag", qf_diagnostic, "QuickFeather diagnostic commands" ),
    CLI_CMD_TERMINATE()
};

#endif
