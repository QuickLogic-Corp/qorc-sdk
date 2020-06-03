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


#define USBSERIAL_RX_BUFSIZE (256)

#define FPGA_USBSERIAL_ID_VALUE         0xA5BD
#define FPGA_USBSERIAL_REV_NUM          0x0100

typedef struct fpga_usbserial_regs {
    uint32_t    device_id;
    uint32_t    rev_num;
    uint16_t    scratch_reg;
    uint16_t    reserved1;
    uint32_t    reserved2[13];
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

#endif // EOSS3_HAL_USBSERIAL_H_