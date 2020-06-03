/* File: vectors_CM4F_gcc.c
 * Purpose: vector table file for Cortex-M4 devices.
 *          Should be used with GCC 'GNU Tools ARM Embedded'
 * Version: V1.01
 * Date: 12 June 2014
 *
 */
/* Copyright (c) 2011 - 2014 ARM LIMITED
   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ARM nor the names of its contributors may be used
     to endorse or promote products derived from this software without
     specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/

#include <stdint.h>


/*----------------------------------------------------------------------------
  Linker generated Symbols
 *----------------------------------------------------------------------------*/
extern uint32_t __StackTop;

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler Function Prototype
 *----------------------------------------------------------------------------*/
typedef void( *intfunc )( void );

typedef union { void (*int_func)(void); void * __ptr; int __val;} intvec_elem;


/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
void Default_Handler(void);                          /* Default empty handler */
extern void Reset_Handler(void);                            /* Reset Handler */


extern void dbg_fatal_error_int(const char *s, int v );

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Cortex-M4 Processor Exceptions */
extern	void	NMI_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	HardFault_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	MemManage_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	BusFault_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	UsageFault_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	SVC_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	DebugMon_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	PendSV_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	SysTick_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));

extern	void	SwInt2_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	SwInt1_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Ffe0Msg_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	FbMsg_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	SensorGpio_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	M4SramSleep_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Uart_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Timer_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	CpuWdtInt_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	CpuWdtRst_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	BusTimeout_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Fpu_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Pkfb_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	I2s_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Audio_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	SpiMs_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	CfgDma_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	PmuTimer_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	AdcDone_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	RtcAlarm_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	ResetInt_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Ffe0_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	FfeWdt_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	ApBoot_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	LDO30_PG_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	LDO50_PG_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	SRAM_128KB_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	LPSD_Voice_Det_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	DMIC_Voice_Det_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Sdma1Done_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Sdma2Done_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Sdma3Done_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Sdma4Done_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Sdma5Done_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Sdma6Done_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Sdma7Done_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Sdma8Done_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Sdma9Done_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Sdma10Done_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Sdma11Done_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	ApPDMClkOn_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	ApPDMClkOff_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Dmac0BlkDone_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Dmac0BufDone_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Dmac1BlkDone_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Dmac1BufDone_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	Sdma0Done_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	SdmaErr_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	I2S_SlvM4TxOr_Handler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	lpsdVoiceOffHandler	(void)__attribute__ ((weak, alias("Default_Handler")));
extern	void	dmicVoiceOffHandler	(void)__attribute__ ((weak, alias("Default_Handler")));



/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/
const intvec_elem __Vectors[] __attribute__ ((section(".isr_vector"))) = {
    (intfunc)&__StackTop,                       /*  0x0 :Main SP */
    Reset_Handler,                              /*  0x4 : Reset */
    NMI_Handler	,	                            /* 	0x8	NMI	 */
    HardFault_Handler	,	                    /* 	0xC	Hard Fault	 */
    MemManage_Handler	,	                    /* 	0x10	Mem Manage Fault */
    BusFault_Handler	,	                    /* 	0x14	Bus Fault	 */
    UsageFault_Handler	,	                    /* 	0x18	Usage Fault	 */
    0	,	                                    /* 	0x1C	Reserved	 */
    0	,	                                    /* 	0x20	Reserved	 */
    0	,	                                    /* 	0x24	Reserved	 */
    0	,	                                    /* 	0x28	Reserved	 */
    SVC_Handler	,	                            /* 	0x2C	SVC	 */
    DebugMon_Handler	,	                    /* 	0x30	Debug Monitor	 */
    0	,	                                    /* 	0x34	Reserved	 */
    PendSV_Handler	,	                        /* 	0x38	Pend SV	 */
    SysTick_Handler	,	                        /* 	0x3C	SysTick	 */
    SwInt2_Handler	,	                      /* 0x40	Software Interrupt 2*/
    SwInt1_Handler	,	                      /* 0x44	Software Interrupt 1*/
    0	,	                                    /* 	0x48	Reserved	 */
    Ffe0Msg_Handler	,	                        /* 	0x4C	FFE0 Message	 */
    FbMsg_Handler	,	                        /* 	0x50	Fabric Message	 */
    SensorGpio_Handler	,	                    /* 	0x54	Sensor/GPIO	 */
    M4SramSleep_Handler	,	                    /* 	0x58	M4 SRAM Sleep	 */
    Uart_Handler	,	                        /* 	0x5C	UART	 */
    Timer_Handler	,	                        /* 	0x60	TIMER	 */
    CpuWdtInt_Handler	,	                    /* 	0x64	CPU WDOG_INTR	 */
    CpuWdtRst_Handler	,	                    /* 	0x68	CPU WDOG_RST	 */
    BusTimeout_Handler	,                   	/* 	0x6C	Bus Timeout	 */
    Fpu_Handler	,	                            /* 	0x70	FPU	 */
    Pkfb_Handler	,	                        /* 	0x74	PKFB	 */
    I2s_Handler	,	                            /* 	0x78	Reserved	 */
    Audio_Handler	,	                        /* 	0x7C	Reserved	 */
    SpiMs_Handler	,	                        /* 	0x80	SPI_MS	 */
    CfgDma_Handler	,	                        /* 	0x84	CFG_DMA	 */
    PmuTimer_Handler	,	                    /* 	0x88	PMU Timer	 */
    AdcDone_Handler	,	                        /* 	0x8C	ADC Done	 */
    RtcAlarm_Handler	,	                    /* 	0x90	RTC Alarm	 */
    ResetInt_Handler	,	                    /* 	0x94	Reset Interrupt	 */
    Ffe0_Handler	,	                        /* 	0x98	FFE0 Combined	 */
    FfeWdt_Handler	,	                        /* 	0x9C	Reserved	 */
    ApBoot_Handler	,	                        /* 	0xA0	AP Boot	 */
    LDO30_PG_Handler	,	                    /* 	0xA4	LDO30 PG INTR	 */
    LDO50_PG_Handler	,	                    /* 	0xA8	LDO50 PG INTR	 */
    SRAM_128KB_Handler	,	                    /* 	0xAC	Reserved	 */
    LPSD_Voice_Det_Handler	,	                /* 	0xB0	LPSD Voice Det	 */
    DMIC_Voice_Det_Handler	,	                /* 	0xB4	DMIC Voice Det	 */
    0	,	                                    /* 	0xB8	Reserved	 */
    Sdma1Done_Handler	,	                    /* 	0xBC	SDMA_DONE	 */
    Sdma2Done_Handler	,	                    /* 	0xC0	SDMA_DONE	 */
    Sdma3Done_Handler	,	                    /* 	0xC4	SDMA_DONE	 */
    Sdma4Done_Handler	,	                    /* 	0xC8	SDMA_DONE	 */
    Sdma5Done_Handler	,	                    /* 	0xCC	SDMA_DONE	 */
    Sdma6Done_Handler	,	                    /* 	0xD0	SDMA_DONE	 */
    Sdma7Done_Handler	,	                    /* 	0xD4	SDMA_DONE	 */
    Sdma8Done_Handler	,	                    /* 	0xD8	SDMA_DONE	 */
    Sdma9Done_Handler	,	                    /* 	0xDC	SDMA_DONE	 */
    Sdma10Done_Handler	,	                    /* 	0xE0	SDMA_DONE	 */
    Sdma11Done_Handler	,	                    /* 	0xE4	SDMA_DONE	 */
    ApPDMClkOn_Handler	,	                    /* 	0xE8	AP_PDM_CLK_ON 	 */
    ApPDMClkOff_Handler	,	                    /* 	0xEC	AP_PDM_CLK_OFF 	 */
    Dmac0BlkDone_Handler	,	                /* 	0xF0	DMAC0_BLK_DONE 	 */
    Dmac0BufDone_Handler	,	                /* 	0xF4	DMAC0_BUF_DONE 	 */
    Dmac1BlkDone_Handler	,	                /* 	0xF8	DMAC1_BLK_DONE	 */
    Dmac1BufDone_Handler	,	                /* 	0xFC	DMAC1_BUF_DONE	 */
    Sdma0Done_Handler	,	                    /* 	0x100	SDMA_DONE[0]	 */
    SdmaErr_Handler	,	                        /* 	0x104	SDMA_ERR	 */
    I2S_SlvM4TxOr_Handler	,	              /* 0x108	I2SSLV_M4_tx_or_intr*/
    lpsdVoiceOffHandler	,	                    /* 	0x10C	LPSD_VOICE_OFF	 */
    dmicVoiceOffHandler	,	                    /* 	0x110	DMIC_VOICE_OFF	 */	
    0,                                          /*0x114     */
    0,                                          /*0x118     */
    0,                                          /*0x11c     */
    {.__val = (0x20021FFF)}                     /* 0x120    Flash Boot header */
};


/*----------------------------------------------------------------------------
  Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
void Default_Handler(void) {

    dbg_fatal_error_int("sperious-irq", __LINE__);
	//while(1);
}
