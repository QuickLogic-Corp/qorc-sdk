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
 *    File   : eoss3_hal_uart.h
 *    Purpose: 
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <eoss3_api.h>
#include <eoss3_dev.h>
#include <common.h>
//#include <eoss3_hal_rcc.h>
#include <eoss3_hal_uart.h>
#include <eoss3_hal_pad_config.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "queue.h"

#include "Fw_global_config.h"
//#include "eoss3_hal_clock.h"
#include "queue.h"
#include "s3x_clock_hal.h"
#include "s3x_lpm.h"
#ifdef QLUART_ENABLE
#include "ql_uart.h"
#endif
#if FEATURE_USBSERIAL == 1
#include "eoss3_hal_fpga_usbserial.h"
#endif // FEATURE_USBSERIAL
#if FEATURE_FPGA_UART == 1
#include "eoss3_hal_fpga_uart.h"
#endif
/*
 * Private variables
 */
#ifdef QLUART_ENABLE
#define USE_RX_QUEUE 0
#else
#define USE_RX_QUEUE 1
#endif   
#define UART_HW_RX_BUFSIZE (1* 1024)
     
struct uart_vars {
    /* queue handle for the rx queue */
#if USE_RX_QUEUE
    QueueHandle_t  rx_qh;
#else
    uint8_t buf[ UART_HW_RX_BUFSIZE];
    int     buf_oe;
    volatile int     buf_wr;
    volatile int     buf_rd;
#endif
    int rx_cnt;
    /* we are "enabled" if we have a non-zero baudrate */
    UartHandler hw_config;
    
    /* are we in low power mode */
    int in_lpm;
    
    /* have we registered with LPM yet? */
    int lpm_registered;
    
    /* how many framing errors have occured */
    int framing_err_count;
    /* how many overrun errors have occured */
    int overrun_err_count;
    /* how many parity errors have occured */
    int parity_err_count;
    /* how many break errors have occured */
    int break_err_count;
    int lpm_count;
};

static struct uart_vars uart_vars;

static int is_hw_uart_enabled(int uartid)
{
    if( uart_vars.hw_config.baud == 0 ){
        return 0;
    }

    if( uartid == UART_ID_CONSOLE ){
        return 1;
    }
    if( uartid == UART_ID_HW ){
        return 1;
    }
    return 0;
}

static void uart_isr_init(void)
{
#if USE_RX_QUEUE
    uart_vars.rx_qh = xQueueCreate( UART_HW_RX_BUFSIZE, sizeof(uint8_t) );
    vQueueAddToRegistry( uart_vars.rx_qh, "Uart_Q" );
#else
    uart_vars.buf_wr = 1;
    uart_vars.buf_rd = 0;
#endif
    INTR_CTRL->OTHER_INTR &= UART_INTR_DETECT;
    INTR_CTRL->OTHER_INTR_EN_M4 |= UART_INTR_EN_M4;
    NVIC_ClearPendingIRQ(Uart_IRQn);
    // This interrupt cannot be triggered during executing main()
    // because the value of base priority register is 0x40 at main.
    // To initialize system, this interrupt should be triggered at main.
    // So, we will set its priority just before calling vTaskStartScheduler(), not this time.
    //NVIC_SetPriority(Uart_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(Uart_IRQn);
}

static void uart_disable_irqs(void)
{
	UART->UART_IMSC = 0;
	UART->UART_RSR = 0;
	UART->UART_ILPR = 0;
	UART->UART_ICR = UART_IC_OVERRUN_ERR | UART_IC_BREAK_ERR |
		UART_IC_PARITY_ERR | UART_IC_FRAMING_ERR | UART_IC_RX_TIMEOUT |
		UART_IC_TX | UART_IC_RX | UART_IC_DSR |
		UART_IC_DCD | UART_IC_CTS | UART_IC_RI;
}

static void uart_disable(void)
{
    UART->UART_CR = 0;
    UART->UART_LCR_H = 0;
    uart_disable_irqs();
}


static int uart_lpm_callback( int state)
{
    int ret = 0;
    if (state == ENTER_LPM)
{
        if( uart_vars.hw_config.lpm_enabled){
            uart_vars.lpm_count += 1;
            uart_disable();
            uart_vars.in_lpm = 1;
            S3x_Clk_Disable(S3X_M4_PRPHRL_CLK);
}
}
    else if (state == EXIT_LPM)
{
        if( uart_vars.in_lpm ){
            uart_vars.in_lpm = 0;
            S3x_Clk_Enable(S3X_M4_PRPHRL_CLK);
            NVIC_ClearPendingIRQ(Uart_IRQn);
            UART->UART_IBRD = uart_vars.hw_config.ibrd_value;
            UART->UART_FBRD = uart_vars.hw_config.fbrd_value;
            UART->UART_LCR_H = uart_vars.hw_config.lcr_h_value;
            if (uart_vars.hw_config.intrMode == UART_INTR_ENABLE)
                UART->UART_IMSC = UART_IMSC_RX;
            UART->UART_CR = uart_vars.hw_config.cr_value | UART_CR_UART_ENABLE;    
		}
	}
    return ret;
}

static inline unsigned int compute_divider(unsigned int baud)
    {
    return(unsigned int)(baud * 16);
    }

static unsigned int compute_ibrd(unsigned int clock, unsigned int baud)
    {
    unsigned int divider = compute_divider(baud);
           
    return(clock / divider );
    }

static unsigned int compute_fbrd(unsigned int clock, unsigned int baud)
	{
    unsigned int divider = compute_divider(baud);
    float fbrd;

    fbrd = ((float)clock / divider) - (clock/divider);
    fbrd = fbrd * 64 + 0.5f;
    return (unsigned int)fbrd;
}


static void uart_baud_rate (void)
	{
    unsigned int clock, baud, fbrd_value = 0;

    clock = S3x_Clk_Get_Rate(S3X_M4_PRPHRL_CLK);
    switch (uart_vars.hw_config.baud)
	{
	case BAUD_2400:   baud = 2400;   break;
	case BAUD_4800:   baud = 4800;   break;
	case BAUD_9600:   baud = 9600;   break;
	case BAUD_19200:  baud = 19200;  break;
	case BAUD_38400:  baud = 38400;  break;
	case BAUD_57600:  baud = 57600;  break;
	case BAUD_115200: baud = 115200; break;
	case BAUD_230400: baud = 230400; break;
	case BAUD_460800: baud = 460800; break;
	case BAUD_921600: baud = 921600; break;
	default:
		return;
	}

    uart_vars.hw_config.ibrd_value = compute_ibrd(clock, baud);
	fbrd_value = compute_fbrd(clock, baud);
    uart_vars.hw_config.fbrd_value = fbrd_value ;
}


static void uart_control_registers(void)
{
    uint32_t lcr_h_value = 0;
    uint32_t cr_value = 0;

    switch (uart_vars.hw_config.wl)
	{
	case WORDLEN_8B: lcr_h_value |= UART_LCR_WLEN_8_BITS; break;
	case WORDLEN_7B: lcr_h_value |= UART_LCR_WLEN_7_BITS; break;
	case WORDLEN_6B: lcr_h_value |= UART_LCR_WLEN_6_BITS; break;
	case WORDLEN_5B: lcr_h_value |= UART_LCR_WLEN_5_BITS; break;
	default:
	case WORDLEN_INVALID: return;
	}

    switch (uart_vars.hw_config.stop)
	{
	case STOPBITS_1: break;
	case STOPBITS_2: lcr_h_value |= UART_LCR_TWO_STOP_BITS; break;
	default:
	case STOPBIT_INVALID: return;
	}

    switch(uart_vars.hw_config.parity)
	{
	case PARITY_NONE: break;
	case PARITY_EVEN:
		lcr_h_value |= (UART_LCR_EVEN_PARITY | UART_LCR_PARITY_ENABLE);
		break;
	case PARITY_ODD:
		lcr_h_value |= (UART_LCR_ODD_PARITY | UART_LCR_PARITY_ENABLE);
		break;
	}

    switch(uart_vars.hw_config.mode)
    {
        // FIXME what to do with interrupt masking?
        case DISABLE_MODE: return;
        case TX_MODE: cr_value |= UART_CR_TX_ENABLE; break;
        case RX_MODE: cr_value |= UART_CR_RX_ENABLE; break;
        case TX_RX_MODE: cr_value |= (UART_CR_TX_ENABLE | UART_CR_RX_ENABLE); break;
    }

    switch(uart_vars.hw_config.hwCtrl)
    {
        case HW_FLOW_CTRL_DISABLE: break;
        case HW_FLOW_CTRL_ENABLE: cr_value |= (UART_CR_CTS_ENABLE | UART_CR_RTS_ENABLE); break;
    }
    uart_vars.hw_config.lcr_h_value = lcr_h_value;
    uart_vars.hw_config.cr_value = cr_value;
}


static void uart_config_pad(PadConfig* ppadConfigTx, PadConfig* ppadConfigRx)
{
	// Set up IO pin for TX
    if ((uart_vars.hw_config.mode == TX_MODE) || (uart_vars.hw_config.mode == TX_RX_MODE))
	{
		HAL_PAD_Config(ppadConfigTx);
	}

    // Set up IO pin and interrupt mask for RX
    if ((uart_vars.hw_config.mode == RX_MODE) || (uart_vars.hw_config.mode == TX_RX_MODE))
    {
		HAL_PAD_Config(ppadConfigRx);
    }
}

void uart_new_baudrate( int uartid, uint32_t newbaudrate )
{
    uint32_t imsc_value;
    
    if( uartid != UART_ID_HW ){
        return;
    }
    switch( newbaudrate ){
    case BAUD_2400:
    case 2400:
        newbaudrate = BAUD_2400;
        break;
	case BAUD_4800:
    case 4800:
        newbaudrate = BAUD_4800;
        break;
	case BAUD_9600:
    case 9600:
        newbaudrate = BAUD_9600;
        break;
	case BAUD_19200:
    case 19200:
        newbaudrate = BAUD_19200;
        break;
	case BAUD_38400:
    case 38400:
        newbaudrate = BAUD_38400;
        break;
	case BAUD_57600:
    case 57600:
        newbaudrate = BAUD_57600;
        break;
	case BAUD_115200:
    case 115200:
        newbaudrate = BAUD_115200;
        break;
	case BAUD_230400:
    case 230400:
        newbaudrate = BAUD_230400;
        break;
	case BAUD_460800:
    case 460800:
        newbaudrate = BAUD_460800;
        break;
	case BAUD_921600:
    case 921600:
        newbaudrate = BAUD_921600;
        break;
    }
    uart_vars.hw_config.baud = (UartBaudRateType)newbaudrate; // Intersting technique
    uart_baud_rate();
    
    // Disable
    UART->UART_CR = 0;
    UART->UART_LCR_H = 0;
    
    imsc_value = 0;
    if(uart_vars.hw_config.intrMode == UART_INTR_ENABLE ){
        imsc_value |= UART_IMSC_RX | UART_IMSC_RX_TIMEOUT;
    } else {
        /* we don't use the isrs */
    }
    UART->UART_IBRD = uart_vars.hw_config.ibrd_value;
    UART->UART_FBRD = uart_vars.hw_config.fbrd_value;
    UART->UART_LCR_H = uart_vars.hw_config.lcr_h_value | UART_LCR_ENABLE_FIFO;
    UART->UART_IMSC = imsc_value;
    UART->UART_IFLS = (UART_IFLS_RX_1_8_FULL | UART_IFLS_TX_1_2_FULL);
    UART->UART_CR = uart_vars.hw_config.cr_value | UART_CR_UART_ENABLE;
}    

void uart_init( int uartid, PadConfig* ppadConfigTx, PadConfig* ppadConfigRx, const UartHandler *pConfig )
{
    uint32_t imsc_value;
    
#if FEATURE_FPGA_UART == 1
    if ( ( uartid == UART_ID_FPGA ) || ( uartid == UART_ID_FPGA_UART1) ){
        HAL_FB_UART_Init( uartid, pConfig );
        return;
    }
#endif
#if FEATURE_USBSERIAL == 1
    if( uartid == UART_ID_USBSERIAL ){
        HAL_usbserial_init(pConfig->intrMode == UART_INTR_ENABLE);
        return;
    }
#endif
    if( uartid != UART_ID_HW ){
        return;
    }
    uart_vars.hw_config = *pConfig;

    if (!pConfig->fBaremetal) S3x_Clk_Enable(S3X_M4_PRPHRL_CLK);		// In baremetal case, assume clock already set up
    uart_baud_rate();
    uart_control_registers();
    uart_disable();
    uart_disable_irqs();
    
    // Backwards compatibility TODO Remove when no longer necessary
    if (ppadConfigTx != NULL && ppadConfigRx != NULL)
        uart_config_pad(ppadConfigTx, ppadConfigRx);

    imsc_value = 0;
    if(!pConfig->fBaremetal && (uart_vars.hw_config.intrMode == UART_INTR_ENABLE) ){
        uart_isr_init();
            imsc_value |= UART_IMSC_RX | UART_IMSC_RX_TIMEOUT;
    } else {
        /* we don't use the isrs */
		uart_vars.hw_config.intrMode = UART_INTR_DISABLE;
        }
    UART->UART_IBRD = uart_vars.hw_config.ibrd_value;
    UART->UART_FBRD = uart_vars.hw_config.fbrd_value;
    UART->UART_LCR_H = uart_vars.hw_config.lcr_h_value | UART_LCR_ENABLE_FIFO;
    UART->UART_IMSC = imsc_value;
    UART->UART_IFLS = (UART_IFLS_RX_1_8_FULL | UART_IFLS_TX_1_2_FULL);
    UART->UART_CR = uart_vars.hw_config.cr_value | UART_CR_UART_ENABLE;
    /* only regiser with low power mode once */
    
    if( !pConfig->fBaremetal && !uart_vars.lpm_registered){
        uart_vars.lpm_registered = 1;
        S3x_Register_Lpm_Cb( uart_lpm_callback, "UART");
    }
}

void uart_isr_handler( int uartid )
{
    uint32_t r32;
    uint8_t rx_byte;
    BaseType_t awoken = pdFALSE;
    
    /* we only do RX irqs */
    if( uartid != UART_ID_HW ){
        return;
    }

    while( !(UART->UART_TFR & UART_TFR_RX_FIFO_EMPTY) ){
        r32 = UART->UART_DR;
        if( r32 & UART_DR_OVERRUN_ERR ){
            uart_vars.overrun_err_count += 1;
        }
        if( r32 & UART_DR_FRAMING_ERR ){
            uart_vars.framing_err_count += 1;
        }
        if( r32 & UART_DR_PARITY_ERR ){
            uart_vars.parity_err_count += 1;
        }
        if( r32 & UART_DR_BREAK_ERR ){
            uart_vars.break_err_count += 1;
        }
        rx_byte = r32;
        uart_vars.rx_cnt += 1;
 #if USE_RX_QUEUE
        xQueueSendToBackFromISR( uart_vars.rx_qh, &rx_byte, &awoken );
#else
        int tmp;
        tmp = uart_vars.buf_wr;
        tmp = (tmp+1) % UART_HW_RX_BUFSIZE;
        if( tmp == uart_vars.buf_rd ){
            uart_vars.buf_oe = 1;
        } else {
            uart_vars.buf[ uart_vars.buf_wr ] = rx_byte;
            uart_vars.buf_wr = tmp;
        }
#endif
#ifdef QLUART_ENABLE
	QLUART_Fill_RX_Buf(&rx_byte,1);
#endif
    }

    if( awoken ){
        taskYIELD();
    }
}

int uart_rx_raw_buf( int uartid, uint8_t *pBuf, size_t n )
{
    int x;
    
    for( x = 0 ; x < n ; x++ ){
        pBuf[x] = uart_rx(uartid);
    }
    return (int)(n);
}



int uart_rx( int uartid )
{
    int r;

#if FEATURE_FPGA_UART == 1
    if(( uartid == UART_ID_FPGA ) || (uartid == UART_ID_FPGA_UART1) ){
        uint8_t rdValue;
        HAL_FB_UART_RxBuf(uartid, &rdValue, 1);
	return rdValue;
    }
#endif
#if FEATURE_USBSERIAL == 1
    if( uartid == UART_ID_USBSERIAL ){
        return HAL_usbserial_getc();
    }
#endif

    /* if wrong uart */
    if( !is_hw_uart_enabled(uartid) ){
        /* return error */
        return -1;
    }
    while( !uart_rx_available(uartid) ){
        vTaskDelay(1);
        taskYIELD();
    }
    if( uart_vars.hw_config.intrMode == UART_INTR_ENABLE ){
        uint8_t rx_byte;
#if USE_RX_QUEUE
        xQueueReceive( uart_vars.rx_qh, &rx_byte, 0 );
#else
        int tmp;
        tmp = uart_vars.buf_rd;
        tmp = (tmp + 1) % UART_HW_RX_BUFSIZE;
        rx_byte = uart_vars.buf[ uart_vars.buf_rd ];
        uart_vars.buf_rd = tmp;
#endif
        r = rx_byte;
    } else {
        r = UART->UART_DR;
        r = r & 0x0ff;
	}
    return r;
}


volatile int uart_bad_tx_char ;

void uart_tx_raw(int uartid, int c)
	{
#if FEATURE_FPGA_UART == 1
    if( (uartid == UART_ID_FPGA) || (uartid == UART_ID_FPGA_UART1) ) {
        HAL_FB_UART_Tx( uartid, c );
        return;
	}
#endif
#if FEATURE_USBSERIAL == 1
    if( uartid == UART_ID_USBSERIAL ){
        HAL_usbserial_putc( c );
        return;
	}
#endif
    if( !is_hw_uart_enabled(uartid) ){
        return;
	}

    if( (c=='\n') || (c=='\r') || (c=='\b') || ( (c >= 0x20) && (c < 0x7f) ) ){
        /* all is well */
    } else {
        uart_bad_tx_char += 1;
        }
    
    /* if fifo is full */;
    while ((UART->UART_TFR & UART_TFR_TX_FIFO_FULL))
        ;
#if 0
    /* wait for fifo to be empty */
    while (!(UART->UART_TFR & UART_TFR_TX_FIFO_EMPTY))
        ;

    /* wait for transmit shif register to be empty */
    while (UART->UART_TFR & UART_TFR_BUSY)
        ;
#endif
    UART->UART_DR = c /* & 0xFF */;
}

void uart_tx(int uartid, int c)
	{
    if( c == '\n' ){
        uart_tx_raw(uartid,'\r');
        }
    uart_tx_raw(uartid,c);
}

void uart_tx_buf(int uartid, const uint8_t *buf, size_t len)
{
	int i;

	for(i=0; i<len; i++)
		uart_tx( uartid, buf[i]);
}

void uart_tx_raw_buf(int uartid, const uint8_t *buf, size_t len)
{
	int i;
	for(i=0; i<len; i++)
		uart_tx_raw( uartid, buf[i]);
}

int uart_rx_wait( int uartid, int msecs )
{
#if FEATURE_FPGA_UART == 1
    if( (uartid == UART_ID_FPGA) || (uartid == UART_ID_FPGA_UART1) ){
      return EOF; // fgpa_uart_rx_wait( msecs );
}
#endif

#if FEATURE_USBSERIAL == 1
    if( uartid == UART_ID_USBSERIAL ){
        return HAL_usbserial_rxwait( msecs );
}
#endif
    if( uartid == UART_ID_DISABLED ){
        return EOF;
    }

    if( !is_hw_uart_enabled(uartid) ){
        return EOF;
    }

#if USE_RX_QUEUE
    /* suspend on the queue, but don't take from the queue */
    int r;
    uint8_t b;
    r = xQueuePeek( uart_vars.rx_qh, &b, msecs );
    if( r ){
        return (int)(b);
    } else {
        return EOF;
    }
#else
    for(;;){
        int tmp;
        tmp = uart_rx_available( uartid );
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
#endif
}


int uart_rx_available( int uartid )
{
    int r;
#if FEATURE_FPGA_UART == 1
    if(( uartid == UART_ID_FPGA ) || (uartid == UART_ID_FPGA_UART1) ){
        return HAL_FB_UART_dataavailable(uartid); // fgpa_uart_rx_available();
    }
#endif
#if FEATURE_USBSERIAL == 1
    if( uartid == UART_ID_USBSERIAL ){
        return HAL_usbserial_dataavailable();
    }
#endif
    if( uartid == UART_ID_DISABLED ){
        return 0;
    }
    if( !is_hw_uart_enabled(uartid) ){
        return 0;
    }

    if( uart_vars.hw_config.intrMode == UART_INTR_ENABLE ){
#if USE_RX_QUEUE
        r = uxQueueMessagesWaiting( uart_vars.rx_qh );
#else
        int tmp;
        tmp = uart_vars.buf_wr + UART_HW_RX_BUFSIZE - (uart_vars.buf_rd + 1);
        tmp = tmp % UART_HW_RX_BUFSIZE ;
        return tmp;
#endif
    } else {
        r = !!(UART->UART_TFR & UART_TFR_RX_FIFO_EMPTY);
    }
    return r;
}

void uart_set_lpm_state(int uart_id, int lpm_en)
{
    if(uart_id == UART_ID_HW)
    {
        if (1 == lpm_en)
        {
            uart_vars.hw_config.lpm_enabled = 1;
        }
        else
        {
            uart_vars.hw_config.lpm_enabled = 0;
        }
    }
    return;
}

/* return 1 if FIFO is almost full 
 * return 0 otherwise
 * For USB-serial almost full means FIFO is more than 3/4 filled up,
 *     that is only 1/4 space is available in FIFO
 * For other UART types (UART_ID_HW) always return 0
 */
int uart_tx_is_fifo_full(int uartid)
{
#if FEATURE_USBSERIAL == 1
    if( uartid == UART_ID_USBSERIAL ){
        return HAL_usbserial_tx_is_fifo_full(  );
        ;
    }
#endif
    return 0;
}

/* return 1 if FIFO is emtpy 
 * return 0 otherwise
 * For USB-serial return whether FIFO is empty
 * For other UART types (UART_ID_HW) always return 1
 */
int uart_tx_is_fifo_empty(int uartid)
{
#if FEATURE_USBSERIAL == 1
    if( uartid == UART_ID_USBSERIAL ){
        return HAL_usbserial_tx_is_fifo_empty(  );
    }
#endif
    return 1;
}

#if FEATURE_USBSERIAL == 1
int uart_tx_is_fifo_half_empty(int uartid)
{
    if( uartid == UART_ID_USBSERIAL ){
        return HAL_usbserial_tx_is_fifo_half_empty();
    }
    return -1; // Unknown UART ID
}

int uart_tx_get_fifo_status(int uartid)
{
    if( uartid == UART_ID_USBSERIAL ){
        return HAL_usbserial_tx_get_fifo_status();
    }
    return -1; // Unknown UART ID    
}

int uart_tx_get_fifo_space_available(int uartid)
{
    if( uartid == UART_ID_USBSERIAL ){
        return HAL_usbserial_tx_get_fifo_space_available();
    }
    return -1; // Unknown UART ID    
}
#endif