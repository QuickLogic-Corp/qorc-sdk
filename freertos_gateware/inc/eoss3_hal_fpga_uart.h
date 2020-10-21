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

#ifndef __EOSS3_HAL_FB_UART_H_
#define __EOSS3_HAL_FB_UART_H_
/*! \file eoss3_hal_fpga_uart.h
 *
 *  \brief This file contains API declaration for UART read/write
 *         implmented in fpga.
 */
#include <stdint.h>
#include <stddef.h>
#include <eoss3_dev.h>
#include <eoss3_hal_def.h>
#include "eoss3_hal_uart.h"

#define FB_UART0_BASE         (FPGA_PERIPH_BASE+0x1000)
#define FB_UART1_BASE         (FPGA_PERIPH_BASE+0x2000)

#define FB_UART0              ((FPGA_UART_TypeDef *)FB_UART0_BASE)
#define FB_UART1              ((FPGA_UART_TypeDef *)FB_UART1_BASE)
//
// Interrupt Enable Register
//

#define IER_ERXRDY      0x01
#define IER_ETXRDY      0x02
#define IER_ERLS        0x04
#define IER_EMSC        0x08

//
// Interrupt Identification Register
//

#define IIR_IMASK       0x0F
#define IIR_NOPEND      0x01
#define IIR_MLSC        0x00
#define IIR_TXRDY       0x02
#define IIR_RXRDY       0x04
#define IIR_RLS         0x06
#define IIR_RXTOUT      0x0C
#define IIR_FIFO_MASK   0xC0    // Set if FIFOs are enabled

//
// FIFO Control Register
//

#define FCR_ENABLE      0x01
#define FCR_RCV_RST     0x02
#define FCR_XMT_RST     0x04
#define FCR_DMA_MODE    0x08
#define FCR_TRIGGER_1   0x00
#define FCR_TRIGGER_4   0x40
#define FCR_TRIGGER_8   0x80
#define FCR_TRIGGER_14  0xC0

//
// Line Control Register
//

#define LCR_DLAB        0x80
#define LCR_SBREAK      0x40
#define LCR_PZERO       0x30
#define LCR_PONE        0x20
#define LCR_PEVEN       0x10
#define LCR_PODD        0x00
#define LCR_PENAB       0x08
#define LCR_STOPB       0x04
#define LCR_8BITS       0x03
#define LCR_7BITS       0x02
#define LCR_6BITS       0x01
#define LCR_5BITS       0x00

//
// Modem Control Register
//


#define MCR_FLOW        0x20
#define MCR_LOOPBACK    0x10
#define MCR_OUT2        0x08
#define MCR_OUT1        0x04
#define MCR_RTS         0x02
#define MCR_DTR         0x01

//
// Line Status Register
//

#define LSR_RCV_FIFO    0x80
#define LSR_THRE        0x40
#define LSR_TXRDY       0x20
#define LSR_BI          0x10
#define LSR_FE          0x08
#define LSR_PE          0x04
#define LSR_OE          0x02
#define LSR_RXRDY       0x01
#define LSR_RCV_MASK    0x1F

//
// Modem Status Register
//

#define MSR_DCD         0x80
#define MSR_RI          0x40
#define MSR_DSR         0x20
#define MSR_CTS         0x10
#define MSR_DDCD        0x08
#define MSR_TERI        0x04
#define MSR_DDSR        0x02
#define MSR_DCTS        0x01

/*! \fn vvoid HAL_FB_UART_Init(UartHandler *ptrObj)
 *  \brief Initialize UART implemented in FPGA
 *
 *  \param ptrObj       Pointer to UART handler structure
 *  \return status
 */
//HAL_StatusTypeDef HAL_FB_UART_Init(UartHandler *pxObj);
void HAL_FB_UART_Init(int uartid, const UartHandler *pxObj);

/*! \fn void HAL_FB_UART_Stop(void)
 *  \brief Stop pending RX/TX.
 *
 */
void HAL_FB_UART_Stop(int uartid);


/*! \fn void HAL_FB_UART_Tx(int c)
 *  \brief Send byte over UART.
 *
 *  \param c    Byte to transmit over UART
 */
void HAL_FB_UART_Tx(int uartid, int c);

/*! \fn int HAL_FB_UART_Rx(void)
 *  \brief Send byte over UART implemented in FPGA.
 *
 *  \return Byte read from UART
 */
int HAL_FB_UART_Rx(int uartid);

int fb_uart_read(int uartid, ptrdiff_t buf, size_t len);

int HAL_FB_UART_dataavailable(int uartid);
int HAL_FB_UART_RxBuf(int uartid, uint8_t *b, const int l);
#endif /* !__EOSS3_HAL_FB_UART_H_ */
