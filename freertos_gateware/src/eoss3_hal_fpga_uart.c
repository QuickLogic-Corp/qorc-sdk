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
 *    File   : eoss3_hal_fpga_uart.c
 *    Purpose: 
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <eoss3_dev.h>
#include <eoss3_hal_def.h>
#include <eoss3_hal_fpga_uart.h>
#include <eoss3_hal_pad_config.h>
//#include "Alg_types.h"
#include "s3x_clock_hal.h"
#include "s3x_lpm.h"

//#include "Periph_setup.h"
#include "Fw_global_config.h"

#define USE_FPGA_UART (FEATURE_FPGA_UART)

#if  (USE_FPGA_UART == 1)

#ifdef __RTOS
#include <FreeRTOS.h>
#include <semphr.h>
#endif
#if (!USE_FPGA_UART)
#define FB_UART_LPM_EN /* Enable LPM for FPGA UART saves power */
#endif
#define RXBUF_LEN       (256)
#define CLK_ENABLE  1
#define CLK_DISABLE 0
struct RxBuf
{
    uint8_t b[RXBUF_LEN];
    uint8_t start;
    uint8_t end;
};
static struct RxBuf rxBuf;
static xSemaphoreHandle consumer_wakeup_sem;
UartHandler FBUartObj;
unsigned char uart_init_done=0;
static int FB_uart_lpm_en = 0, FB_uart_in_lpm = 0;
void FB_uart_register_lpm(void);

static void initRxBuf()
{
    rxBuf.start = rxBuf.end = 0;
}

int FB_getRxBufSize(void)
{
    if(rxBuf.start <= rxBuf.end) return (rxBuf.end - rxBuf.start);
    else return (RXBUF_LEN + rxBuf.end - rxBuf.start);
}
void FB_fillRxBuf(const uint8_t *b, const int l, BaseType_t *pxHigherPriorityTaskWoken)
{
    int wrLen = l;
    int i;

    if ((FB_getRxBufSize() + wrLen) > RXBUF_LEN)
    {
        wrLen = RXBUF_LEN - FB_getRxBufSize();
    }
    for(i=l-wrLen; i<l; i++)
    {
        rxBuf.b[rxBuf.end++] = b[i];
    }
    if (FB_getRxBufSize() > 0 && consumer_wakeup_sem) {
        xSemaphoreGiveFromISR(consumer_wakeup_sem, pxHigherPriorityTaskWoken);
    }
}

int FB_getRxBuf(uint8_t *b, const int l)
{
    int rdLen = l;
    int i;

    if (FB_getRxBufSize() == 0 && consumer_wakeup_sem) {
        xSemaphoreTake(consumer_wakeup_sem, portMAX_DELAY);
    }
    if (FB_getRxBufSize() < rdLen)
    {
        rdLen = FB_getRxBufSize();
    }
    for(i=0; i<rdLen; i++)
    {
        b[i] = rxBuf.b[rxBuf.start];
        rxBuf.start++;
    }
    return i;
}

static void FB_UART_ISR_Func(void)
{
    unsigned int iir = FB_UART->UART_IIR_FCR;
    unsigned char tmp = 0;
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
    if (!(iir & IIR_NOPEND)) {
        if (iir & IIR_RXRDY) {
            while (FB_UART->UART_LSR & LSR_RXRDY) {    //TBD : Enable fifo
                tmp = FB_UART->UART_DR_DLLSB & 0xFF;
                FB_fillRxBuf(&tmp, 1, &pxHigherPriorityTaskWoken);
                tmp = FB_UART->UART_LSR;
            }
        }
    }
    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
}

int fb_uart_read(int uartid, ptrdiff_t buf, size_t len)
{
    return FB_getRxBuf(( unsigned char *)buf, len);
}

void fb_uart_set_clk(uint8_t en)
{
    if (en)
    {
        S3x_Clk_Set_Rate(S3X_FB_16_CLK, F_24MHZ);
        S3x_Clk_Enable(S3X_FB_16_CLK);
        S3x_Clk_Enable(S3X_FB_02_CLK);
        S3x_Clk_Enable(S3X_FFE_X1_CLK);
        S3x_Clk_Enable(S3X_CLKGATE_FB);
        S3x_Clk_Enable(S3X_FB_21_CLK);
    }
    else
    {
        S3x_Clk_Disable(S3X_FB_16_CLK);
        S3x_Clk_Disable(S3X_FB_02_CLK);
        S3x_Clk_Disable(S3X_FFE_X1_CLK);
        S3x_Clk_Disable(S3X_CLKGATE_FB);
        S3x_Clk_Disable(S3X_FB_21_CLK);
    }
}

void HAL_FB_UART_Init(const UartHandler *pxObj)
{
    UINT32_t lcr_h_value = 0;
    UINT32_t cr_value = 0;
    UINT32_t dlatch_value = 0;
    //UINT32_t imsc_value = 0;
    UINT32_t clock, baud;

    UartHandler *ptrObj;
    if(pxObj)
    {
        memcpy(&FBUartObj, pxObj, sizeof(FBUartObj));
    }
    ptrObj = &FBUartObj;


    initRxBuf();
    /* Unlock write to C11 clock gate register */
//    MISC_CTRL->LOCK_KEY_CTRL = 0x1ACCE551;

/*
    clkInitParams.uiClockDomainType = CLK_DOMAIN_C16;
    clkInitParams.uiClockGate = C16_CLK_GATE_FB;
    clkInitParams.uiCLKFrequency = 0;
    HAL_RCC_EnableClk(clkInitParams);
*/

    /* Unlock write to C11 clock gate register */
    //MISC_CTRL->LOCK_KEY_CTRL = 0x1ACCE551;

    fb_uart_set_clk(CLK_ENABLE);

    clock = S3x_Clk_Get_Rate(S3X_FB_16_CLK);

    switch (pxObj->baud)
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
        //return HAL_ERROR;
        return ;
    }
        dlatch_value = (UINT32_t)((((float)clock/(baud*16))-1) + 0.5);

    switch (pxObj->wl)
    {
    case WORDLEN_8B: lcr_h_value |= LCR_8BITS; break;
    case WORDLEN_7B: lcr_h_value |= LCR_7BITS; break;
    case WORDLEN_6B: lcr_h_value |= LCR_6BITS; break;
    case WORDLEN_5B: lcr_h_value |= LCR_5BITS; break;
    default:
    case WORDLEN_INVALID:
        //return HAL_ERROR;
        return ;
    }

    switch (pxObj->stop)
    {
    case STOPBITS_1: break;
    case STOPBITS_2: lcr_h_value |= LCR_STOPB; break;
    default:
    case STOPBIT_INVALID:
        //return HAL_ERROR;
        return;
    }

    switch(pxObj->parity)
    {
    case PARITY_NONE: break;
    case PARITY_EVEN:
        lcr_h_value |= (LCR_PENAB | LCR_PEVEN);
        break;
    case PARITY_ODD:
        lcr_h_value |= (LCR_PENAB | LCR_PODD);
        break;
    }

    switch(pxObj->hwCtrl)
    {
        case HW_FLOW_CTRL_DISABLE: break;
        case HW_FLOW_CTRL_ENABLE: cr_value |= MCR_FLOW; break;
    }

    // Disable UART
    FB_UART->UART_LCR = 0;

    // Clear and disable all interrupts

    /* Program Divisor latch */
    FB_UART->UART_LCR = LCR_DLAB;
    FB_UART->UART_IER_DLMSB = (dlatch_value >> 8) & 0xFF;

    FB_UART->UART_DR_DLLSB = dlatch_value & 0xFF;

#if (0) // pad configuration setup moved to pincfg_table.c
    // Set up IO pin for TX
    if ((pxObj->mode == TX_MODE) || (pxObj->mode == TX_RX_MODE))
    {
        PadConfig xPadConf;
        xPadConf.ucPin  = 0;
        xPadConf.ucFunc = 0;
        xPadConf.ucCtrl = 0;
        xPadConf.ucMode = 0;
        xPadConf.ucPull = 0;
        xPadConf.ucDrv =  0;
        xPadConf.ucSpeed = 0;
        xPadConf.ucSmtTrg =0;
        //memset((uint8_t *)&xPadConf, 0, sizeof(PadConfig));

        xPadConf.ucPin = FPGA_UART_TX_GPIO;
        xPadConf.ucFunc = FPGA_UART_TX_FUNC;
        xPadConf.ucCtrl = PAD_CTRL_SRC_FPGA;
        xPadConf.ucMode = PAD_MODE_OUTPUT_EN;
        xPadConf.ucPull = PAD_NOPULL;
        xPadConf.ucDrv = PAD_DRV_STRENGHT_4MA;
        xPadConf.ucSpeed = PAD_SLEW_RATE_SLOW;
        xPadConf.ucSmtTrg = PAD_SMT_TRIG_DIS;

        HAL_PAD_Config(&xPadConf);
    }

    // Set up IO pin and interrupt mask for RX
    if ((pxObj->mode == RX_MODE) || (pxObj->mode == TX_RX_MODE))
    {
        PadConfig xPadConf;

        xPadConf.ucPin = FPGA_UART_RX_GPIO;
        xPadConf.ucFunc = FPGA_UART_RX_FUNC;
        xPadConf.ucCtrl = PAD_CTRL_SRC_FPGA;
        xPadConf.ucMode = PAD_MODE_INPUT_EN;
        xPadConf.ucPull = PAD_NOPULL;
        xPadConf.ucDrv = PAD_DRV_STRENGHT_4MA;
        xPadConf.ucSpeed = PAD_SLEW_RATE_SLOW;
        xPadConf.ucSmtTrg = PAD_SMT_TRIG_DIS;

        HAL_PAD_Config(&xPadConf);

    }
#endif


    //setup interrupt here
    if ((pxObj->mode == RX_MODE) || (pxObj->mode == TX_RX_MODE))
    {
    consumer_wakeup_sem = xSemaphoreCreateBinary();
    FB_RegisterISR(FB_INTERRUPT_2, FB_UART_ISR_Func);
    FB_ConfigureInterrupt(FB_INTERRUPT_2, FB_INTERRUPT_TYPE_LEVEL/* FIXME */,
                  FB_INTERRUPT_POL_LEVEL_HIGH/* FIXME */,
                  FB_INTERRUPT_DEST_AP_DISBLE, FB_INTERRUPT_DEST_M4_ENABLE);
    NVIC_ClearPendingIRQ(FbMsg_IRQn);
    NVIC_EnableIRQ(FbMsg_IRQn);
    FB_UART->UART_LCR &= ~LCR_DLAB;
    FB_UART->UART_IER_DLMSB = IER_ERXRDY ;
    }

    // Set up UART LCR and MCR
    FB_UART->UART_LCR = ptrObj->lcr_h_value = lcr_h_value;
    FB_UART->UART_MCR = ptrObj->cr_value = cr_value;

    /* Program FIFO Enable */
    FB_UART->UART_IIR_FCR = FCR_ENABLE|FCR_TRIGGER_14;

    /* flag to indicate the UART init is completed.
    This is to make sure before any UART prints called the init is completed */
    uart_init_done = 1;

    FB_uart_register_lpm();
#ifdef FB_UART_LPM_EN
    FB_uart_lpm_en = 1;
#endif
    //return HAL_OK;
}

/*! \fn void HAL_FB_UART_Stop(void)
 *  \brief Stop pending RX/TX.
 *
 */
void HAL_FB_UART_Stop()
{
    FB_UART->UART_LCR |= LCR_SBREAK;
    FB_UART->UART_LCR &= ~LCR_SBREAK;
}

/*
 * Basic read, write, put, get routines follow here.
 */
static int fb_uart_getc(void)
{
    int c;

    while (!(FB_UART->UART_LSR & LSR_RXRDY));
    c = FB_UART->UART_DR_DLLSB & 0xFF;
    return(c);
}

static void uart_putc(int c)
{
    //while ((FB_UART->UART_LSR & LSR_TXRDY));
    while (!(FB_UART->UART_LSR & LSR_THRE /*LSR_TXRDY*/));

    FB_UART->UART_DR_DLLSB = c  & 0xFF ;
}

/*! \fn void HAL_FB_UART_Tx(int c)
 *  \brief Send byte over UART.
 *
 *  \param c    Byte to transmit over UART
 */
void HAL_FB_UART_Tx(int c)
{
      uart_putc(c);
}

/*! \fn int HAL_FB_UART_Rx(void)
 *  \brief Send byte over UART implemented in FPGA.
 *
 *  \return Byte read from UART
 */
int HAL_FB_UART_Rx(void)
{
    return(fb_uart_getc());
}


int FB_uart_enter_lpm(void)
{
    if (FB_uart_lpm_en)
    {
        FB_UART->UART_LCR = 0;
        FB_uart_in_lpm = 1;
        fb_uart_set_clk(CLK_DISABLE);
    }
    return 0;
}

void FB_uart_exit_lpm(void)
{
    UartHandler *ptrObj = &FBUartObj;

    if (FB_uart_in_lpm)
    {
        fb_uart_set_clk(CLK_ENABLE);
        FB_UART->UART_LCR = ptrObj->lcr_h_value;
        FB_UART->UART_MCR = ptrObj->cr_value;
        FB_uart_in_lpm = 0;
    }
}

int FB_uart_lpm_callback( int state)
{
    int ret = 0;
    if (state == ENTER_LPM)
    {
        ret = FB_uart_enter_lpm();
    }
    else if (state == EXIT_LPM)
    {
        FB_uart_exit_lpm();
    }
    return ret;
}

void FB_uart_register_lpm(void)
{
    S3x_Register_Lpm_Cb(FB_uart_lpm_callback, "FB_UART");
}
#endif /* USE_FPGA_UART */
