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

static void userbutton(const struct cli_cmd_entry *pEntry)
{
    uint8_t ucVal;
    return;
}

const struct cli_cmd_entry qf_diagnostic[] =
{
    CLI_CMD_SIMPLE( "red", toggleredled, "toggle red led" ),
    CLI_CMD_SIMPLE( "green", togglegreenled, "toggle green led" ),
    CLI_CMD_SIMPLE( "blue", toggleblueled, "toggle blue led" ),
    CLI_CMD_TERMINATE()
};

const struct cli_cmd_entry my_main_menu[] = {
    CLI_CMD_SUBMENU( "diag", qf_diagnostic, "Qomu diagnostic commands" ),
    CLI_CMD_TERMINATE()
};

#endif
