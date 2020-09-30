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
#include "eoss3_hal_audio.h"
#include "host_interface.h"
#include "eoss3_hal_uart.h"

//#include "vm1010.h"
#include "qlsh_commands.h"
#include "ql_audio.h"
#include "ql_controlTask.h"


#if FEATURE_CLI_DEBUG_INTERFACE

#define SPI_TRANSFER_TEST       (0)
#if SPI_TRANSFER_TEST        // to test the Host - Slave tranfer over spi
#include "d2h_protocol.h"


#define SLAVE_S3_WRITE_MEM_ADDR         (0x7C800)    // this is where host writes and slave reads
#define SLAVE_S3_READ_MEM_ADDR          (0x7C400)   // this is whrer host reads and slave writes
#define DATA_READ_WRITE_SIZE            (128)


#define CMD_DUMMY_1                     (0xF)
#define CHANNEL_DUMMY_1                     (0x5)


// These addresses are fixed for communication between BLE and S3 using QL SPI
//#pragma default_variable_attributes = @ "BLE_SPI_TX_LOCATION"
uint8_t host_spi_read_buff[DATA_READ_WRITE_SIZE]; //host reads from here, slave writes to this location
//#pragma default_variable_attributes =

//#pragma default_variable_attributes = @ "BLE_SPI_RX_LOCATION"
uint8_t host_spi_write_buff[DATA_READ_WRITE_SIZE];////host writes here, slave reads from this location
//#pragma default_variable_attributes =
static void intr_host(const struct cli_cmd_entry *pEntry)
{
    (void)pEntry;
    // do something
    return;

}

static void read_data(const struct cli_cmd_entry *pEntry)
{
    (void)pEntry;
    // do something
    int i = 0;

    memcpy(&(host_spi_write_buff[0]),(void *)(SLAVE_S3_WRITE_MEM_ADDR),DATA_READ_WRITE_SIZE);

    dbg_memdump8((intptr_t)(host_spi_write_buff),(void *)(host_spi_write_buff),DATA_READ_WRITE_SIZE);

    return;

}

uint8_t pattern = 0xFA;
static void write_data(const struct cli_cmd_entry *pEntry)
{
    (void)pEntry;
    // do something
    int i = 0;
    for(i = 0; i< DATA_READ_WRITE_SIZE; ++i)
    {
        //host_spi_read_buff[i] = pattern;
        host_spi_read_buff[i] = i%16;
    }
    memcpy((void *)(SLAVE_S3_READ_MEM_ADDR),&(host_spi_read_buff[0]),DATA_READ_WRITE_SIZE);
     dbg_memdump8((intptr_t)(host_spi_read_buff),(void *)(host_spi_read_buff),DATA_READ_WRITE_SIZE);

    return;

}


static void set_write_pattern( const struct cli_cmd_entry *pEntry )
{
  (void)pEntry;
#if DBG_FLAGS_ENABLE
  CLI_uint8_getshow( "pattern", &pattern );
#endif
}
static int seq = 0;
static void send_cmd(const struct cli_cmd_entry *pEntry)
{
    (void)pEntry;
        // Add functionality here
    dbg_str("sending event to host. \n");
    D2H_Pkt_Info cmd_info = {0};
    cmd_info.channel = CHANNEL_DUMMY_1;

    seq++;
    if(0xf <= seq) {
      seq = 0;
    }

    cmd_info.seq = seq;
    cmd_info.cmd = CMD_DUMMY_1;
    cmd_info.data[0] = 0x01;
    cmd_info.data[1] = 0x11;
    cmd_info.data[2] = 0x22;
    cmd_info.data[3] = 0x33;
    cmd_info.data[4] = 0x44;
    cmd_info.data[5] = 0x55;

    if (d2h_transmit_cmd(&cmd_info)){
        dbg_str("Error returned from d2h tansmit api\n");
    }

}


Rx_Cb_Ret d2h_receive_callback(D2H_Pkt_Info rx_cmd_info)
{
    Rx_Cb_Ret ret = {0};
    dbg_str("callback invoked\n");

    // display the unpacked cmd received
    dbg_str_int("seq = ",rx_cmd_info.seq);
    dbg_str_int("channel = ",rx_cmd_info.channel);
    dbg_str_int("cmd = ",rx_cmd_info.cmd);
    dbg_str_int("data[0] = ",rx_cmd_info.data[0]);
    dbg_str_int("data[1] = ",rx_cmd_info.data[1]);
    dbg_str_int("data[2] = ",rx_cmd_info.data[2]);
    dbg_str_int("data[3] = ",rx_cmd_info.data[3]);
    dbg_str_int("data[4] = ",rx_cmd_info.data[4]);
    dbg_str_int("data[5] = ",rx_cmd_info.data[5]);


    return ret;
}
static void reg_callback( const struct cli_cmd_entry *pEntry )
{
  (void)pEntry;
  d2h_register_rx_callback((&d2h_receive_callback),CHANNEL_DUMMY_1, NULL, NULL, NULL );

}

const struct cli_cmd_entry spi_test[] =
{
    CLI_CMD_SIMPLE( "intr_host", intr_host, "cli command to generate interrupt to Host" ),
    CLI_CMD_SIMPLE( "set_write_pattern", set_write_pattern, "cli command to set the write pattern for write_data" ),
    CLI_CMD_SIMPLE( "read_data", read_data, "cli command to read buffer modified by Host" ),
    CLI_CMD_SIMPLE( "write_data", write_data, " cli command to write pattern to buffer modified by Host " ),
    CLI_CMD_SIMPLE( "reg_callback", reg_callback, " to register test callback to d2h protocol " ),
    CLI_CMD_SIMPLE( "send_cmd", send_cmd, " to send dummy cmd to host " ),
    CLI_CMD_TERMINATE()
};


#endif

static void set_dbg_flags( const struct cli_cmd_entry *pEntry )
{
  (void)pEntry;
#if DBG_FLAGS_ENABLE
  CLI_uint32_getshow( "debug-flags", &DBG_flags );
#endif
}




static void vm1010_int(const struct cli_cmd_entry *pEntry)
{
    (void)pEntry;
    struct xCQ_Packet CQpacket;
    CQpacket.ceEvent = CEVENT_VM1010;
    addPktToControlQueue(&CQpacket);
    dbg_str("\n VM1010 Event sent\n");
    return;
}

static void vrtrigger(const struct cli_cmd_entry *pEntry)
{
    (void)pEntry;
    struct xCQ_Packet CQpacket;
    CQpacket.ceEvent = CEVENT_VR_TRIGGER;
    addPktToControlQueue(&CQpacket);
    dbg_str("\n VM1010 Event sent\n");
    return;
}

static void print_regs(const struct cli_cmd_entry *pEntry) {
    print_S3_register_values();
}

#if ENABLE_OPUS_ENCODER
void set_audio_status(uint8_t val);
void opus_setTestMode(uint8_t value);
static void vr_on(const struct cli_cmd_entry *pEntry)
{
  (void)pEntry;
  set_audio_status(0);
  return;
}

static void opus_test0(const struct cli_cmd_entry *pEntry)
{
  opus_setTestMode(0);
}

static void opus_test1(const struct cli_cmd_entry *pEntry)
{
  opus_setTestMode(1);
}
#endif

void enable_VR_continuous(uint8_t val);
static void frr_test1(const struct cli_cmd_entry *pEntry)
{
  //TIMenable_VR_continuous(1);
}

static void frr_test0(const struct cli_cmd_entry *pEntry)
{
 //TIM enable_VR_continuous(0);
}

void audio_ql_vr_event_handler(int pid, int event_type, void *p_event_data, int num_data_bytes);






static void disable_host(const struct cli_cmd_entry *pEntry)
{
  dbg_str("\n Host communication disabled, standalone test in progress:\n");
  //enable_host_communication(0);
  hif_enableCommunication(0);
}

static void enable_host(const struct cli_cmd_entry *pEntry)
{
  dbg_str("\n Host communication enabled:\n");
  hif_enableCommunication(1);
}

static void enable_uart_lpm(const struct cli_cmd_entry *pEntry)
{
    (void)pEntry;
    uart_set_lpm_state(UART_ID_HW,1);
    return;
}
static void disable_uart_lpm(const struct cli_cmd_entry *pEntry)
{
    (void)pEntry;
    uart_set_lpm_state(UART_ID_HW,0);
    return;
}
#if (ENABLE_I2S_TX_SLAVE == 1)
static void i2s_aud_start(const struct cli_cmd_entry *pEntry)
{
  (void)pEntry;
    set_audio_clock_qos(1);
    
      struct xQ_Packet packet={0};
  packet.ucCommand = eCMD_AUDIO_START;
  packet.ucSrc = AUDIO_TASK_MESSAGE;
  addPktToQueueFromTask_Audio(&packet);
    return;
}
#endif
const struct cli_cmd_entry voice_test[] =
{
#if ENABLE_OPUS_ENCODER
    CLI_CMD_SIMPLE( "vr_on", vr_on, " cli command to make VR ON and OPUS OFF. OPUS will be active [and VR is auto off] only after KP detects." ),
    CLI_CMD_SIMPLE( "opus_test1", opus_test1, " cli command to enable OPUS test mode" ),
    CLI_CMD_SIMPLE( "opus_test0", opus_test0, " cli command to disable OPUS test mode" ),
    CLI_CMD_SIMPLE( "frr_test1", frr_test1, " Enable FRR test, OPUS will be disabled" ),
    CLI_CMD_SIMPLE( "frr_test0", frr_test0, " Disable FRR test, OPUS will be enabled only after KPD" ),
#endif

    CLI_CMD_TERMINATE()
};


extern const struct cli_cmd_entry fsm_menu[];   // From Control Task

const struct cli_cmd_entry my_main_menu[] = {
    CLI_CMD_SIMPLE( "vm1010", vm1010_int, "sends vm1010 event" ),
    CLI_CMD_SIMPLE( "vr", vrtrigger, "sends vr_trigger event" ),
    CLI_CMD_SIMPLE( "print_regs", print_regs, "prints most register values" ),
    CLI_CMD_SIMPLE( "disable_host", disable_host, "disables host interface communication" ),
    CLI_CMD_SIMPLE( "enable_host", enable_host, "enables host interface communication" ),
    CLI_CMD_SIMPLE( "uart_lpm_en", enable_uart_lpm, "to enable uart lpm " ),
    CLI_CMD_SIMPLE( "uart_lpm_dis", disable_uart_lpm, "to disable uart lpm" ),
    CLI_CMD_SUBMENU( "voice_test", voice_test, "ql smart remote application menu" ),
    CLI_CMD_SUBMENU( "fsm", fsm_menu, "fsm menu" ),
#if SPI_TRANSFER_TEST
    CLI_CMD_SUBMENU( "spi_test", spi_test, "ql smart remote application menu" ),
#endif
#if (ENABLE_I2S_TX_SLAVE == 1)
    CLI_CMD_SIMPLE( "aud_start", i2s_aud_start, "to start audio for i2s transfer " ),
#endif
    
    CLI_CMD_TERMINATE()
};

#endif
