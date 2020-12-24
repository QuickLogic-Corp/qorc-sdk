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
volatile fpga_usbserial_regs_t* pusbserial_regs = (fpga_usbserial_regs_t*)(FPGA_PERIPH_BASE);

//------------- Local functions -------------------------//
static void HAL_usbserial_isrinit(void);
static void HAL_usbserial_isrfunc(void);

//------------- Local varialbles ------------------------//
static QueueHandle_t    qhUSBserialRx = {NULL};
static bool             fUsingInterrupts;

void    HAL_usbserial_init(bool fUseInterrupt) {
	HAL_usbserial_init2(fUseInterrupt, false, 0x6141);
}
//This can take product as USB and IP id differnt from it 
static void HAL_usbserial_init_common(bool fUseInterrupt, bool fUse72MHz, uint32_t usbpid, uint32_t usb_ipid ) {
    // Setup FPGA clocks

    // Enable 12MHz clock on C16
    S3x_Clk_Set_Rate(S3X_FB_16_CLK, 12000*1000);
    S3x_Clk_Enable(S3X_FB_16_CLK);

    // Setup the clock select register
    if (fUse72MHz) {
       pusbserial_regs->clock_select = 1;   // Write 1 to Offset 0x0C to enable 1.5 divider (72/1.5 = 48MHz)
    } else {
       pusbserial_regs->clock_select = 0;   // Write 0 to Offset 0x0C to use input clock as is
    }
    // Set the USB product ID
    pusbserial_regs->usbpid = usbpid;

    // Enable 48MHz/72MHz clock on C21
    if (fUse72MHz) {
       S3x_Clk_Set_Rate(S3X_FB_21_CLK, 72000*1000);
    } else {
       S3x_Clk_Set_Rate(S3X_FB_21_CLK, 48000*1000);
    }
    S3x_Clk_Enable(S3X_FB_21_CLK);
    
    // Confirm expected IP is loaded
    configASSERT(HAL_usbserial_ipid() == usb_ipid);
    if(usb_ipid == 0xA5BD) {
      configASSERT(pusbserial_regs->rev_num == 0x0200);
    }
    
    // Set to use interrupts if desired
    if (fUseInterrupt) {
        HAL_usbserial_isrinit();
    }
    fUsingInterrupts = fUseInterrupt;
}
//This is init (USBserial + I2S). So, the IP id is different
void HAL_usbserial_i2s_init(bool fUseInterrupt, bool fUse72MHz, uint32_t usbpid,uint32_t usb_ipid ) {
  
  HAL_usbserial_init_common(fUseInterrupt,fUse72MHz,usbpid, usb_ipid);
  return;   
}
//This is init USBserial only
void HAL_usbserial_init2(bool fUseInterrupt, bool fUse72MHz, uint32_t usbpid) {
  
  HAL_usbserial_init_common(fUseInterrupt,fUse72MHz,usbpid, 0xA5BD);
  return;   
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

/* Return 1 if FIFO available space is less than 1/4
 * (FIFO is more than 3/4 full)
 */
int HAL_usbserial_tx_is_fifo_full(void)
{
  if (pusbserial_regs->m2u_fifo_flags == FPGA_USBSERIAL_TX_FIFO_LT_QUARTER)
    return 1;
  else 
    return 0;
}

/* Return 1 if FIFO is empty
 */
int HAL_usbserial_tx_is_fifo_empty(void)
{
  if (pusbserial_regs->m2u_fifo_flags == FPGA_USBSERIAL_TX_FIFO_EMPTY)
    return 1;
  else 
    return 0;
}

/* Return 1 if FIFO is atleast half empty
 */
int HAL_usbserial_tx_is_fifo_half_empty(void)
{
  if (pusbserial_regs->m2u_fifo_flags == FPGA_USBSERIAL_TX_FIFO_GT_HALF)
    return 1;
  else 
    return 0;
}

/* Returns FIFO status value, refer the eoss3_hal_fpga_usbserial.h header file for definitions */
int HAL_usbserial_tx_get_fifo_status(void)
{
  return pusbserial_regs->m2u_fifo_flags ;
}

/* Returns FIFO status value, refer the eoss3_hal_fpga_usbserial.h header file for definitions */
int HAL_usbserial_tx_get_fifo_space_available(void)
{
  int ret = pusbserial_regs->m2u_fifo_flags ;
  switch (ret)
  {
  case FPGA_USBSERIAL_TX_FIFO_FULL:  // (0x00)    // 0000 Full
    return 0;
  case FPGA_USBSERIAL_TX_FIFO_EMPTY:   // (0x01)    // 0001 Empty
    return USBSERIAL_TX_FIFOSIZE;
  case FPGA_USBSERIAL_TX_FIFO_GT_HALF:       // (0x02)    // 0010 Room for more than 1/2
    return (USBSERIAL_TX_FIFOSIZE/2);
  case FPGA_USBSERIAL_TX_FIFO_GT_QUARTER:    // (0x03)    // 0011 Room for more than 1/4
    return (USBSERIAL_TX_FIFOSIZE/4);
  case FPGA_USBSERIAL_TX_FIFO_LT_QUARTER:    // (0x04)    // 0100 Room for less than 1/4
    return 32;
  case FPGA_USBSERIAL_TX_FIFO_32_TO_63:      // (0x0A)    // 1010 Room for 32 to 63
    return 32;
  case FPGA_USBSERIAL_TX_FIFO_16_TO_31:      // (0x0B)    // 1011 Room for 16 to 31
    return 16;
  case FPGA_USBSERIAL_TX_FIFO_8_TO_15:       // (0x0C)    // 1100 Room for 8 to 15
    return 8;
  case FPGA_USBSERIAL_TX_FIFO_4_TO_7:        // (0x0D)    // 1101 Room for 4 to 7
    return 4;
  case FPGA_USBSERIAL_TX_FIFO_GE_2:          // (0x0E)    // 1110 Room for atleast 2
    return 2;
  case FPGA_USBSERIAL_TX_FIFO_GE_1:          // (0x0F)    // 1111 Room for atleast 1
    return 1;
  default:
    return 0;
  }
}
