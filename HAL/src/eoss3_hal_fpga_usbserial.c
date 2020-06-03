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

#include <stdint.h>
#include <stdio.h>

#include "Fw_global_config.h"
#include "eoss3_hal_fpga_usbserial.h"

//------------- Pointer to registers ---------------------//
fpga_usbserial_regs_t* pusbserial_regs = (fpga_usbserial_regs_t*)(FPGA_PERIPH_BASE);

//------------- Local functions -------------------------//
static void HAL_usbserial_isrinit(void);
static void HAL_usbserial_isrfunc(void);

//------------- Local varialbles ------------------------//
static QueueHandle_t    qhUSBserialRx = {NULL};
static bool             fUsingInterrupts;

void    HAL_usbserial_init(bool fUseInterrupt) {
    // Setup FPGA clocks
    S3x_Clk_Set_Rate(S3X_FB_21_CLK, 48000*1000);
    S3x_Clk_Set_Rate(S3X_FB_16_CLK, 12000*1000);
    S3x_Clk_Enable(S3X_FB_21_CLK);
	S3x_Clk_Enable(S3X_FB_16_CLK);
    
    // Confirm expected IP is loaded
    configASSERT(HAL_usbserial_ipid() == 0xA5BD);
    
    // Set to use interrupts if desired
    if (fUseInterrupt) {
        HAL_usbserial_isrinit();
    }
    fUsingInterrupts = fUseInterrupt;
}

int     HAL_usbserial_getc(void) {
    if (pusbserial_regs->u2m_fifo_flags == 0) {
        return EOF;
    } else {
        return pusbserial_regs->rdata;
    }
}

void    HAL_usbserial_putc(char c) {
    // Wait for room in Tx FIFO
    while(pusbserial_regs->m2u_fifo_flags == 0)
        ;
    pusbserial_regs->wdata = c;
}

void HAL_usbserial_txbuf(const uint8_t *buf, size_t len)
{
	int i;

	for(i=0; i<len; i++)
		HAL_usbserial_putc(buf[i]);
}

uint32_t    HAL_usbserial_ipid(void) {
    return pusbserial_regs->device_id;
}

uint32_t    HAL_usbserial_dataavailable(void) {
    return pusbserial_regs->u2m_fifo_flags;
}

static void HAL_usbserial_isrinit(void)
{
    qhUSBserialRx = xQueueCreate( USBSERIAL_RX_BUFSIZE, sizeof(uint8_t) );
    vQueueAddToRegistry( qhUSBserialRx, "USBserialRx" );

    FB_RegisterISR(FB_INTERRUPT_0, HAL_usbserial_isrfunc);
    FB_ConfigureInterrupt(FB_INTERRUPT_0, FB_INTERRUPT_TYPE_LEVEL/* FIXME */,
                  FB_INTERRUPT_POL_LEVEL_HIGH/* FIXME */,
                  FB_INTERRUPT_DEST_AP_DISBLE, FB_INTERRUPT_DEST_M4_ENABLE);
    NVIC_ClearPendingIRQ(FbMsg_IRQn);
    NVIC_EnableIRQ(FbMsg_IRQn);
    
    pusbserial_regs->u2m_fifo_int_en =0x01;     // Enable interrupts
}

static void HAL_usbserial_isrfunc(void)
{
    BaseType_t  pxHigherPriorityTaskWoken = pdFALSE;
    uint8_t     rx_byte;
    
    while(HAL_usbserial_dataavailable()) {
        rx_byte = pusbserial_regs->rdata;
        xQueueSendToBackFromISR( qhUSBserialRx, &rx_byte, &pxHigherPriorityTaskWoken );
    }
    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
}

int HAL_usbserial_rxwait(int msecs) {
    if (qhUSBserialRx) {
        /* suspend on the queue, but don't take from the queue */
        int r;
        uint8_t b;
        r = xQueuePeek( qhUSBserialRx, &b, msecs );
        if( r ){
            return (int)(b);
        } else {
            return EOF;
        }
    } else {
        for(;;){
            int tmp;
            tmp = HAL_usbserial_dataavailable();
            if( tmp ){
                return 0;
            }
            if( msecs == 0 ){
                return EOF;
            }
            msecs--;
            /* wait at most 1 msec */
            vTaskDelay( 1 );
        }
    }
}
