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
#include <eoss3_hal_uart.h>
#include "cli.h"
#include <stdbool.h>
#include "dbg_uart.h"
#include "tinytester.h"

#if FEATURE_CLI_DEBUG_INTERFACE

//-------------------------------------------------
//
//   tinytester Controller CLI
//
//-------------------------------------------------

//-------------- External variables ----------------
extern tinytester_channelconfig_t atinytester_channelconfig;
extern uint32_t     kchannelconfig;
extern uint32_t     auxOutputvector;
extern uint32_t     kvectorOutput;
extern uint32_t     auxInputvector;
extern uint32_t     kvectorInput;

//------------- Local functions -------------------
static void     runtest(const struct cli_cmd_entry *pEntry);
static void     get_signature(const struct cli_cmd_entry *pEntry);
static void     set_control(const struct cli_cmd_entry *pEntry);
static void     get_status(const struct cli_cmd_entry *pEntry);
static void     toggle_control(const struct cli_cmd_entry *pEntry);
static void     set_oe(const struct cli_cmd_entry *pEntry);
static void     set_active_on_p0(const struct cli_cmd_entry *pEntry);
static void     set_active_on_p1(const struct cli_cmd_entry *pEntry);
static void     set_active_on_p2(const struct cli_cmd_entry *pEntry);
static void     set_active_on_p3(const struct cli_cmd_entry *pEntry);

static void     set_dataout(const struct cli_cmd_entry *pEntry);
static void     execute_vector(const struct cli_cmd_entry *pEntry);
static void     execute_binary_vector(const struct cli_cmd_entry *pEntry);
static void     get_datain(const struct cli_cmd_entry *pEntry);


// Run test vectors //
static void runtest(const struct cli_cmd_entry *pEntry) {
    tinytester_run(kchannelconfig, &atinytester_channelconfig, kvectorOutput, 
        &auxOutputvector, kvectorInput, &auxInputvector); 
}

static void get_signature(const struct cli_cmd_entry *pEntry)
{
    uint32_t uxSignature = 0;
    
    (void)pEntry;
        // Add functionality here
    uxSignature = tinytester_signatureis();
    dbg_str_hex32("signature", uxSignature);
    return;
}


static void set_control(const struct cli_cmd_entry *pEntry)
{
    static uint32_t uxControl = 0;
    
    (void)pEntry;
        // Add functionality here
    uxControl = tinytester_controlis();
    dbg_str_hex32("control", uxControl);
    CLI_uint32_getshow( "control_reg", &uxControl );
    tinytester_control(uxControl);
    return;
}

static void get_status(const struct cli_cmd_entry *pEntry)
{
    uint32_t uxStatus = 0;
    
    (void)pEntry;
        // Add functionality here
    uxStatus = tinytester_statusis();
    dbg_str_hex32("status", uxStatus);
    return;
}

static void toggle_control(const struct cli_cmd_entry *pEntry)
{
    (void)pEntry;
        // Add functionality here
    tinytester_control(0x1);
    tinytester_control(0x0);
    return;
}

static void set_oe(const struct cli_cmd_entry *pEntry)
{
    static uint32_t uxOe = 0;
    
    (void)pEntry;
        // Add functionality here
    uxOe = tinytester_oeis();
    dbg_str_hex32("oe", uxOe);
    CLI_uint32_getshow( "oe_reg", &uxOe );
    tinytester_oe(uxOe);
    return;
}

static void set_active_on_p0(const struct cli_cmd_entry *pEntry)
{
    static uint32_t uxActive_on_p0 = 0;
    
    (void)pEntry;
        // Add functionality here
	uxActive_on_p0 = tinytester_active_on_p0is();
    CLI_uint32_getshow( "p0_reg", &uxActive_on_p0 );
    tinytester_active_on_p0(uxActive_on_p0);
    return;
}

static void set_active_on_p1(const struct cli_cmd_entry *pEntry)
{
    static uint32_t uxActive_on_p1 = 0;
    
    (void)pEntry;
        // Add functionality here
    uxActive_on_p1 = tinytester_active_on_p1is();
    CLI_uint32_getshow( "p1_reg", &uxActive_on_p1 );
    tinytester_active_on_p1(uxActive_on_p1);
    return;
}
static void set_active_on_p2(const struct cli_cmd_entry *pEntry)
{
    static uint32_t uxActive_on_p2 = 0;
    
    (void)pEntry;
        // Add functionality here
    uxActive_on_p2 = tinytester_active_on_p2is();
    CLI_uint32_getshow( "p2_reg", &uxActive_on_p2 );
    tinytester_active_on_p2(uxActive_on_p2);
    return;
}
static void set_active_on_p3(const struct cli_cmd_entry *pEntry)
{
    static uint32_t uxActive_on_p3 = 0;
    
    (void)pEntry;
        // Add functionality here
    uxActive_on_p3 = tinytester_active_on_p3is();
    CLI_uint32_getshow( "p3_reg", &uxActive_on_p3 );
    tinytester_active_on_p3(uxActive_on_p3);
    return;
}

static void set_dataout(const struct cli_cmd_entry *pEntry)
{
    static uint32_t uxDataout = 0;
    
    (void)pEntry;
        // Add functionality here
    uxDataout = tinytester_dataoutis();
    CLI_uint32_getshow( "dataout", &uxDataout );
    tinytester_dataout(uxDataout);
    dbg_str_hex32("dataout", uxDataout);
    return;
}

static void execute_vector(const struct cli_cmd_entry *pEntry)
{
    static uint32_t uxDataout = 0;
    uint32_t        uxDatain;
    
    (void)pEntry;
        // Add functionality here
    CLI_uint32_get( "dataout", &uxDataout );
    dbg_str_hex32("do", uxDataout);
    tinytester_executevector(uxDataout, &uxDatain);
    dbg_str_hex32("di", uxDatain);
    return;
}

static void execute_binary_vector(const struct cli_cmd_entry *pEntry)
{
    static uint32_t uxDataout;
    uint32_t        uxDatain;
    
    (void)pEntry;
        // Add functionality here
    while (uart_rx(UART_ID_HW) != '\r') {
      uart_rx_raw_buf( UART_ID_HW, (uint8_t*)&uxDataout, sizeof(uxDataout) );
      //dbg_str_hex32("do", uxDataout);
      tinytester_executevector(uxDataout, &uxDatain);
      uart_tx_raw_buf( UART_ID_HW, (uint8_t *)&uxDatain, sizeof(uxDatain) );
      //dbg_str_hex32("di", uxDatain);
    }
    return;
}

static void get_datain(const struct cli_cmd_entry *pEntry)
{
    uint32_t uxDatain = 0;
    
    (void)pEntry;
        // Add functionality here
    uxDatain = tinytester_datainis();
    dbg_str_hex32("datain", uxDatain);
    return;
}

static void get_databus(const struct cli_cmd_entry *pEntry)
{
    uint32_t uxDatabus = 0;
    
    (void)pEntry;
        // Add functionality here
    uxDatabus = tinytester_databusis();
    dbg_str_hex32("datain", uxDatabus);
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
  CLI_CMD_SUBMENU( "diag", qf_diagnostic, "QuickFeather diagnostic commands" ),
  
  CLI_CMD_SIMPLE( "run", runtest, "run test" ),
  CLI_CMD_SIMPLE( "sig", get_signature, "get signature" ),
  CLI_CMD_SIMPLE( "ctl", set_control, "set control register" ),
  CLI_CMD_SIMPLE( "status", get_status, "get status" ),
  CLI_CMD_SIMPLE( "go", toggle_control, "toggle control register" ),
  CLI_CMD_SIMPLE( "oe", set_oe, "set oe register" ),
  CLI_CMD_SIMPLE( "p0", set_active_on_p0, "set set_active_on_p0" ),
  CLI_CMD_SIMPLE( "p1", set_active_on_p1, "set set_active_on_p1" ),
  CLI_CMD_SIMPLE( "p2", set_active_on_p2, "set set_active_on_p2" ),
  CLI_CMD_SIMPLE( "p3", set_active_on_p3, "set set_active_on_p3" ),

  CLI_CMD_SIMPLE( "w", set_dataout, "write dataout" ),
  CLI_CMD_SIMPLE( "x", execute_vector, "execute vector" ),
  CLI_CMD_SIMPLE( "b", execute_binary_vector, "execute binary vector" ),
  CLI_CMD_SIMPLE( "r", get_datain, "read datain" ),
  CLI_CMD_SIMPLE( "v", get_databus, "read databus" ),
  
  CLI_CMD_TERMINATE()
};

#endif
