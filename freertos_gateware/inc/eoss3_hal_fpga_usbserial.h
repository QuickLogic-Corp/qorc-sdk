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

#ifndef __EOSS3_HAL_USBSERIAL_H_
#define __EOSS3_HAL_USBSERIAL_H_

#include <stdbool.h>

#include <FreeRTOS.h>
#include <queue.h>

#include <eoss3_dev.h>
#include <eoss3_hal_def.h>
#include "s3x_clock.h"


#define USBSERIAL_TX_FIFOSIZE (512)
#define USBSERIAL_RX_FIFOSIZE (512)

#define USBSERIAL_RX_BUFSIZE (256)

#define FPGA_USBSERIAL_ID_VALUE         0xA5BD
#define FPGA_USBSERIAL_REV_NUM          0x0100

/* USB-Serial FIFO status values */
#define FPGA_USBSERIAL_TX_FIFO_FULL          (0x00)    // 0000 Full
#define FPGA_USBSERIAL_TX_FIFO_EMPTY         (0x01)    // 0001 Empty
#define FPGA_USBSERIAL_TX_FIFO_GT_HALF       (0x02)    // 0010 Room for more than 1/2
#define FPGA_USBSERIAL_TX_FIFO_GT_QUARTER    (0x03)    // 0011 Room for more than 1/4
#define FPGA_USBSERIAL_TX_FIFO_LT_QUARTER    (0x04)    // 0100 Room for less than 1/4
#define FPGA_USBSERIAL_TX_FIFO_32_TO_63      (0x0A)    // 1010 Room for 32 to 63
#define FPGA_USBSERIAL_TX_FIFO_16_TO_31      (0x0B)    // 1011 Room for 16 to 31
#define FPGA_USBSERIAL_TX_FIFO_8_TO_15       (0x0C)    // 1100 Room for 8 to 15
#define FPGA_USBSERIAL_TX_FIFO_4_TO_7        (0x0D)    // 1101 Room for 4 to 7
#define FPGA_USBSERIAL_TX_FIFO_GE_2          (0x0E)    // 1110 Room for atleast 2
#define FPGA_USBSERIAL_TX_FIFO_GE_1          (0x0F)    // 1111 Room for atleast 1

typedef struct fpga_usbserial_regs {
    uint32_t    device_id;
    uint32_t    rev_num;
    uint16_t    scratch_reg;
    uint16_t    reserved1;
    uint32_t    clock_select;
    uint32_t    usbpid;
    uint32_t    reserved2[11];
    unsigned    u2m_fifo_flags : 4;
    unsigned    reserved3 :28;
    unsigned    rdata : 8;
    unsigned    reserved4 : 24;
    uint32_t    reserved5[14];
    unsigned    m2u_fifo_flags : 4;
    unsigned    reserved6 :28;
    unsigned    wdata : 8;
    unsigned    reserved7 : 24;
    uint32_t    reserved8[14];
    unsigned    u2m_fifo_int_en : 1;
    unsigned    reserved9 : 31;
} fpga_usbserial_regs_t;

void        HAL_usbserial_init(bool fUseInterrupt);
uint32_t    HAL_usbserial_ipid(void);
uint32_t    HAL_usbserial_dataavailable(void);
int         HAL_usbserial_getc(void);
void        HAL_usbserial_putc(char c);
void        HAL_usbserial_txbuf(const uint8_t *buf, size_t len);
int         HAL_usbserial_rxwait(int msecs);
int         HAL_usbserial_tx_is_fifo_full(void);
int         HAL_usbserial_tx_is_fifo_empty(void);
void        HAL_usbserial_init2(bool fUseInterrupt, bool fUse72MHz, uint32_t usbpid);
int         HAL_usbserial_tx_is_fifo_half_empty(void);
int         HAL_usbserial_tx_get_fifo_status(void);
int         HAL_usbserial_tx_get_fifo_space_available(void);
void HAL_usbserial_i2s_init(bool fUseInterrupt, bool fUse72MHz, uint32_t usbpid,uint32_t usb_ipid );
#endif // EOSS3_HAL_USBSERIAL_H_
