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
 *    File   : d2h_protocol.h
 *    Purpose: header file for s3 device to host communication protocol 
 *                                                          
 *=========================================================*/

#ifndef __H2D_PROTOCOL_H_
#define __H2D_PROTOCOL_H_

#if !defined( _EnD_Of_Fw_global_config_h )
#error "Include Fw_global_config.h first"
#endif

#include "stdlib.h"
#include "stdio.h"

#define DEBUG_D2H_PROTOCOL          (0)     // define this to 1 to enable debug prints/functions

//Note: This is by default 4-pin protocol 1 interrupt pin and 1 ack pin each from Device and Host
//But when there is a requirement to use only  2 pins, one pin from each of Device and Host
//there is a inherent limitation in the protocol. Since same pin is used for both interrupt generation
//and ack generation, there will be times a misinterpretation occurs which will lead to missing
//the interrupts. So, 2-pin protocol is obsoleted. Instead 1-wire protocol is used
#if (FEATURE_1WIRE_PROTOCOL_DEVICE == 1)
#define USE_4PIN_D2H_PROTOCOL       (0) //0=use 1-wire protocol
#else
#define USE_4PIN_D2H_PROTOCOL       (1) //1=use 4 pins, 2 for interrupt and 2 for ack, 0=use 1-wire 
#endif
/* These addr should be in sync with icf/ld file*/
#define D2H_READ_ADDR                  (0x7C800)  // this is where host writes and slave reads (H2D_WRITE_ADDR on host side)
#define D2H_WRITE_ADDR                   (0x7C400)  // this is whrer host reads and slave writes (H2D_READ_ADDR on host side)
#define DATA_READ_WRITE_SIZE            (128)

#define MAX_NUM_CHANNEL             (64)
#define MAX_DATA_LEN_IN_BYTES       (6)
#define H2D_PACKET_SIZE_IN_BYTES    (8)

//this is total buffer shared between Host and Device
#if (FEATURE_1WIRE_PROTOCOL_DEVICE == 1)
#define D2H_SPI_TX_TOTAL_SIZE_BYTES  (2*1024)
#else
#define D2H_SPI_TX_TOTAL_SIZE_BYTES  (1024)
#endif
#define D2H_SPI_RX_TOTAL_SIZE_BYTES  (1024)

//First 8 bytes are aside for D2H protocol packet. Rest is used for Data
#define D2H_SPI_TX_BUF_SIZE_BYTES  (D2H_SPI_TX_TOTAL_SIZE_BYTES - H2D_PACKET_SIZE_IN_BYTES )
#define D2H_SPI_RX_BUF_SIZE_BYTES  (D2H_SPI_RX_TOTAL_SIZE_BYTES - H2D_PACKET_SIZE_IN_BYTES )

#define D2HRX_MSGQ_WAIT_TIME	portMAX_DELAY

#define D2HRX_QUEUE_LENGTH  10  //msg queue size for h2dRxTaskHandler

/*msg types for h2d rx task */

#define D2HRX_MSG_ACK_RCVD   (0x41)
#define D2HRX_MSG_EVT_RCVD   (0x42)
#define D2HRX_MSG_INTR_RCVD  (0x43)
#define D2HRX_MSG_WAIT_FOR_ACK  (0x44)

typedef struct {
	uint8_t	H2D_gpio;	/* For Host to Device interrupt generation (QL_INT) */
	uint8_t	D2H_gpio;	/* For Device to Host interrupt (AP_INT) */

    //by default it is 4-pin protocol
	uint8_t	H2D_ack;	/* For Host to Device Ack (is an interrupt to Device) */
	uint8_t	D2H_ack;	/* For Device to Host interrupt (is an interrupt to Host) */

} D2H_Platform_Info;

/* Structure to be used by user to send info for transmitting command to device */
typedef struct {
    uint8_t seq;                // seq number of command. Increases after each cmd is sent
    uint8_t channel;            // channel number 
    uint8_t cmd;                //  command id
    uint8_t data[6];            // data to be sent in the packet
} D2H_Pkt_Info;


/* structre returned by the rx callback */
typedef struct {
    uint8_t data_read_req;          // flag to convey id second data read is req by host
    uint16_t len;                   // lenoth of data to be read
    uint32_t addr;                  // address (device memory) from where data is to be read
} Rx_Cb_Ret;

/*rx callback prototype*/
typedef Rx_Cb_Ret (*D2H_Callback)(void * cookie, D2H_Pkt_Info rx_pkt_info);
//typedef Rx_Cb_Ret (*H2D_Callback)(H2D_Cmd_Info rx_cmd_info, uint8_t data_buf_ready);

/*tx complete callback prototype*/
typedef void (*D2H_Tx_Done_Callback)(void *cookie, D2H_Pkt_Info tx_pkt_info);

#define D2H_ERR_BASE    (13)
#define D2H_STATUS_OK   (0)
#define D2H_ERROR       ((D2H_ERR_BASE << 16) | 1)

/* tx api*/
int d2h_transmit_cmd(D2H_Pkt_Info *d2h_evt_info);
#if (USE_4PIN_D2H_PROTOCOL == 0)
/* SW Int1 from host config */
extern void configure_host_interrupt(void);
#endif

/*register rx callback api*/
int d2h_register_callback(D2H_Callback rx_cb, uint8_t ch_num, void * cookie_rx, D2H_Tx_Done_Callback tx_done_cb, void *cookie_tx);

/* init api */
int d2h_protocol_init(D2H_Platform_Info * d2h_platform_info);

void h2d_config_intr(void *pv);

int h2d_transmit_lock_acquire(void);
int h2d_transmit_lock_release(void);


#endif //__H2D_PROTOCOL_H_