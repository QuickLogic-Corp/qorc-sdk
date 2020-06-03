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

/*
 *    File   : vectorCM4F_iar.c
 *    Purpose: This file contains vector table used for startup/initialization 
 *             code.
 *
*/


#pragma language=extended
#pragma segment="CSTACK"

/* Dummy handler used as default weak alias for other handler.*/
void Default_Handler(void);
extern void Reset_Handler(void);
extern void dbg_fatal_error_int(const char *s, int v );

typedef void( *intfunc )( void );
typedef union { void (*int_func)(void); void * __ptr; int __val;} intvec_elem;

extern void    NMI_Handler	 (void);
extern void    HardFault_Handler	 (void);
extern void    MemManage_Handler	 (void);
extern void    BusFault_Handler	 (void);
extern void    UsageFault_Handler	 (void);
extern void    SVC_Handler	 (void);
extern void    DebugMon_Handler	 (void);
extern void    PendSV_Handler	 (void);
extern void    SysTick_Handler	 (void);
extern void    SwInt2_Handler	 (void);
extern void    SwInt1_Handler	 (void);
extern void    Ffe0Msg_Handler	 (void);
extern void    FbMsg_Handler	 (void);
extern void    SensorGpio_Handler	 (void);
extern void    M4SramSleep_Handler	 (void);
extern void    Uart_Handler	 (void);
extern void    Timer_Handler	 (void);
extern void    CpuWdtInt_Handler	 (void);
extern void    CpuWdtRst_Handler	 (void);
extern void    BusTimeout_Handler	 (void);
extern void    Fpu_Handler	 (void);
extern void    Pkfb_Handler	 (void);
extern void    I2s_Handler	 (void);
extern void    Audio_Handler	 (void);
extern void    SpiMs_Handler	 (void);
extern void    CfgDma_Handler	 (void);
extern void    PmuTimer_Handler	 (void);
extern void    AdcDone_Handler	 (void);
extern void    RtcAlarm_Handler	 (void);
extern void    ResetInt_Handler	 (void);
extern void    Ffe0_Handler	 (void);
extern void    FfeWdt_Handler	 (void);
extern void    ApBoot_Handler	 (void);
extern void    LDO30_PG_Handler	 (void);
extern void    LDO50_PG_Handler	 (void);
extern void    SRAM_128KB_Handler	 (void);
extern void    LPSD_Voice_Det_Handler	 (void);
extern void    DMIC_Voice_Det_Handler	 (void);
extern void    Sdma1Done_Handler	 (void);
extern void    Sdma2Done_Handler	 (void);
extern void    Sdma3Done_Handler	 (void);
extern void    Sdma4Done_Handler	 (void);
extern void    Sdma5Done_Handler	 (void);
extern void    Sdma6Done_Handler	 (void);
extern void    Sdma7Done_Handler	 (void);
extern void    Sdma8Done_Handler	 (void);
extern void    Sdma9Done_Handler	 (void);
extern void    Sdma10Done_Handler	 (void);
extern void    Sdma11Done_Handler	 (void);
extern void    ApPDMClkOn_Handler	 (void);
extern void    ApPDMClkOff_Handler	 (void);
extern void    Dmac0BlkDone_Handler	 (void);
extern void    Dmac0BufDone_Handler	 (void);
extern void    Dmac1BlkDone_Handler	 (void);
extern void    Dmac1BufDone_Handler	 (void);
extern void    Sdma0Done_Handler	 (void);
extern void    SdmaErr_Handler	 (void);
extern void    I2S_SlvM4TxOr_Handler	 (void);
extern void    lpsdVoiceOffHandler	 (void);
extern void    dmicVoiceOffHandler	 (void);



#pragma weak 	NMI_Handler	 = Default_Handler
#pragma weak 	HardFault_Handler	 = Default_Handler
#pragma weak 	MemManage_Handler	 = Default_Handler
#pragma weak 	BusFault_Handler	 = Default_Handler
#pragma weak 	UsageFault_Handler	 = Default_Handler
#pragma weak 	SVC_Handler	 = Default_Handler
#pragma weak 	DebugMon_Handler	 = Default_Handler
#pragma weak 	PendSV_Handler	 = Default_Handler
#pragma weak 	SysTick_Handler	 = Default_Handler
#pragma weak 	SwInt2_Handler	 = Default_Handler
#pragma weak 	SwInt1_Handler	 = Default_Handler
#pragma weak 	Ffe0Msg_Handler	 = Default_Handler
#pragma weak 	FbMsg_Handler	 = Default_Handler
#pragma weak 	SensorGpio_Handler	 = Default_Handler
#pragma weak 	M4SramSleep_Handler	 = Default_Handler
#pragma weak 	Uart_Handler	 = Default_Handler
#pragma weak 	Timer_Handler	 = Default_Handler
#pragma weak 	CpuWdtInt_Handler	 = Default_Handler
#pragma weak 	CpuWdtRst_Handler	 = Default_Handler
#pragma weak 	BusTimeout_Handler	 = Default_Handler
#pragma weak 	Fpu_Handler	 = Default_Handler
#pragma weak 	Pkfb_Handler	 = Default_Handler
#pragma weak 	I2s_Handler	 = Default_Handler
#pragma weak 	Audio_Handler	 = Default_Handler
#pragma weak 	SpiMs_Handler	 = Default_Handler
#pragma weak 	CfgDma_Handler	 = Default_Handler
#pragma weak 	PmuTimer_Handler	 = Default_Handler
#pragma weak 	AdcDone_Handler	 = Default_Handler
#pragma weak 	RtcAlarm_Handler	 = Default_Handler
#pragma weak 	ResetInt_Handler	 = Default_Handler
#pragma weak 	Ffe0_Handler	 = Default_Handler
#pragma weak 	FfeWdt_Handler	 = Default_Handler
#pragma weak 	ApBoot_Handler	 = Default_Handler
#pragma weak 	LDO30_PG_Handler	 = Default_Handler
#pragma weak 	LDO50_PG_Handler	 = Default_Handler
#pragma weak 	SRAM_128KB_Handler	 = Default_Handler
#pragma weak 	LPSD_Voice_Det_Handler	 = Default_Handler
#pragma weak 	DMIC_Voice_Det_Handler	 = Default_Handler
#pragma weak 	Sdma1Done_Handler	 = Default_Handler
#pragma weak 	Sdma2Done_Handler	 = Default_Handler
#pragma weak 	Sdma3Done_Handler	 = Default_Handler
#pragma weak 	Sdma4Done_Handler	 = Default_Handler
#pragma weak 	Sdma5Done_Handler	 = Default_Handler
#pragma weak 	Sdma6Done_Handler	 = Default_Handler
#pragma weak 	Sdma7Done_Handler	 = Default_Handler
#pragma weak 	Sdma8Done_Handler	 = Default_Handler
#pragma weak 	Sdma9Done_Handler	 = Default_Handler
#pragma weak 	Sdma10Done_Handler	 = Default_Handler
#pragma weak 	Sdma11Done_Handler	 = Default_Handler
#pragma weak 	ApPDMClkOn_Handler	 = Default_Handler
#pragma weak 	ApPDMClkOff_Handler	 = Default_Handler
#pragma weak 	Dmac0BlkDone_Handler	 = Default_Handler
#pragma weak 	Dmac0BufDone_Handler	 = Default_Handler
#pragma weak 	Dmac1BlkDone_Handler	 = Default_Handler
#pragma weak 	Dmac1BufDone_Handler	 = Default_Handler
#pragma weak 	Sdma0Done_Handler	 = Default_Handler
#pragma weak 	SdmaErr_Handler	 = Default_Handler
#pragma weak 	I2S_SlvM4TxOr_Handler	 = Default_Handler
#pragma weak 	lpsdVoiceOffHandler	 = Default_Handler
#pragma weak 	dmicVoiceOffHandler	 = Default_Handler


#pragma location = ".intvec"

const intvec_elem __vector_table[] =
{
    { .__ptr = __sfe( "CSTACK" ) },             /*  0x0 :Main SP */
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

void Default_Handler (void)
{
    dbg_fatal_error_int("sperious-irq", __LINE__);
}
