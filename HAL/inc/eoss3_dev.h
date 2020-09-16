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
 *    File   : eoss3_dev.h
 *    Purpose: This file contains device mapping for
 *             interrupt vector, peripherals register mapping 
 *                                                          
 *=========================================================*/

#ifndef __EOSS3_DEV_H
#define __EOSS3_DEV_H

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

#if !defined( _EnD_Of_Fw_global_config_h )
#error "Include Fw_global_config.h first"
#endif
     
#include <stdint.h>

/// @cond EOSS3_DEV_MACROS
/**
  * @brief Configuration of the Cortex-M4 Processor and Core Peripherals
  */
#define __CM4_REV                 0x0001  /*!< Core revision r0p1                            */
#define __MPU_PRESENT             1       /*!< Tamar provides an MPU                     */
#define __NVIC_PRIO_BITS          3       /*!< Tamar uses 3 Bits for the Priority Levels */
#define __Vendor_SysTickConfig    1       /*!< Set to 1 if different SysTick Config is used  */
#define __FPU_PRESENT             1       /*!< FPU present                                   */

/*
 * Interrupt Number Definition
 */

typedef enum
{
/******  Cortex-M4 Processor Exceptions Numbers ****************************************************************/
  NonMaskableInt_IRQn         = -14,    /*!< 2 Non Maskable Interrupt                                          */
  MemoryManagement_IRQn       = -12,    /*!< 4 Cortex-M4 Memory Management Interrupt                           */
  BusFault_IRQn               = -11,    /*!< 5 Cortex-M4 Bus Fault Interrupt                                   */
  UsageFault_IRQn             = -10,    /*!< 6 Cortex-M4 Usage Fault Interrupt                                 */
  SVCall_IRQn                 = -5,     /*!< 11 Cortex-M4 SV Call Interrupt                                    */
  DebugMonitor_IRQn           = -4,     /*!< 12 Cortex-M4 Debug Monitor Interrupt                              */
  PendSV_IRQn                 = -2,     /*!< 14 Cortex-M4 Pend SV Interrupt                                    */
  SysTick_IRQn                = -1,     /*!< 15 Cortex-M4 System Tick Interrupt                                */
/******  Specific Interrupt Numbers **********************************************************************/
  SwInt2_IRQn           	  = 0,
  SwInt1_IRQn  		          = 1,
  Reserved1_IRQn	          = 2,
  Ffe0Msg_IRQn                = 3,
  FbMsg_IRQn          	      = 4,
  Gpio_IRQn                   = 5,
  SramSleep_IRQn              = 6,
  Uart_IRQn                   = 7,
  Timer_IRQn                  = 8,
  CpuWdtInt_IRQn              = 9,
  CpuWdtRst_IRQn              = 10,
  BusTimeout_IRQn             = 11,
  Fpu_IRQn                    = 12,
  Pkfb_IRQn                   = 13,
  Reserved_I2s_IRQn           = 14,
  Reserved_Audio_IRQn         = 15,
  SpiMs_IRQn                  = 16,
  CfgDma_IRQn                 = 17,
  PmuTimer_IRQn               = 18,
  AdcDone_IRQn                = 19,
  RtcAlarm_IRQn               = 20,
  ResetInt_IRQn               = 21,
  Ffe0_IRQn                   = 22,
  FfeWdt_IRQn                 = 23,
  ApBoot_IRQn                 = 24,
  Ldo30_pg_IRQn               = 25,
  Ldo50_pg_IRQn               = 26,
  Sram_to_IRQn                = 27,
  Lpsd_IRQn					  = 28,
  Dmic_IRQn					  = 29,
  Reserved2_IRQn  			  = 30,
  Sdma_Done1_IRQn  			  = 31,
  Sdma_Done2_IRQn  			  = 32,
  Sdma_Done3_IRQn  			  = 33,
  Sdma_Done4_IRQn  			  = 34,
  Sdma_Done5_IRQn  			  = 35,
  Sdma_Done6_IRQn  			  = 36,
  Sdma_Done7_IRQn  			  = 37,
  Sdma_Done8_IRQn  			  = 38,
  Sdma_Done9_IRQn  			  = 39,
  Sdma_Done10_IRQn  		  = 40,
  Sdma_Done11_IRQn  		  = 41,
  Ap_Pdm_Clock_On_IRQn 		  = 42,
  Ap_Pdm_Clock_Off_IRQn       = 43,
  Dmac0_Block_Done_IRQn 	  = 44,
  Dmac0_Buffer_Done_IRQn      = 45,
  Dmac1_Block_Done_IRQn       = 46,
  Dmac1_Buffer_Done_IRQn      = 47,
  Sdma_Done0_IRQn  		  	  = 48,
  Sdma_Err_IRQn  		      = 49,
  I2SSlv_M4_IRQn  		      = 50,
  Lpsd_Voice_Off_IRQn		  = 51,
  Dmic_Voice_Off_IRQn		  = 52

} IRQn_Type;
#ifdef __FPU_USED
#undef __FPU_USED
#endif
//new core_cm4 from CMSIS_5 has it defined
#include <core_cm4.h>             /* Cortex-M4 processor and core peripherals */

/*
 * Peripheral_registers_structures
 */

/*
 * Uart
 */
typedef struct
{
	__IO uint32_t UART_DR;		/* Data Address: 0x00 */
	__IO uint32_t UART_RSR;         /* Receive Status Register: 0x04 */
	__IO uint32_t reserved1[4];		/* Reserved: 0x08 - 0x14 */
	__IO uint32_t UART_TFR;		/* Flag Register 0x18 */
	__IO uint32_t reserved2[1];         /* Reserved 0x1C */
	__IO uint32_t UART_ILPR;        /* IrDA Low Power Counter Register 0x20 */
	__IO uint32_t UART_IBRD;        /* Integer Baud Rate Register 0x24 */
	__IO uint32_t UART_FBRD;        /* Fractional Baud Rate Register 0x28 */
	__IO uint32_t UART_LCR_H;       /* Line Control Register 0x2C */
	__IO uint32_t UART_CR;          /* Control Register 0x30 */
	__IO uint32_t UART_IFLS;        /* Interrupt FIFO Level Select Register 0x34 */
	__IO uint32_t UART_IMSC;        /* Interrupt Mask Set/Clear Register 0x38 */
	__IO uint32_t UART_RIS;         /* Raw Interrupt Status Register 0x3C */
	__IO uint32_t UART_MIS;         /* Masked Interrupt Status Register 0x40 */
	__IO uint32_t UART_ICR;         /* Interrupt Clear Register 0x44 */
	__IO uint32_t reserved4[14];
	__IO uint32_t UART_TCR;       /* Test Control Register 0x48 */
	__IO uint32_t UART_ITIP;
	__IO uint32_t UART_ITOP;
	__IO uint32_t UART_TDR;
	__IO uint32_t reserved3[980];       /* Reserved 0x4c - 0cFDC */
	__IO uint32_t UART_PeriphID0;   /* Peri. ID0 Register (0x11)                  Address offset: 0xFE0 */
	__IO uint32_t UART_PeriphID1;   /* Peri. ID1 Register (0x10)                  Address offset: 0xFE4 */
	__IO uint32_t UART_PeriphID2;   /* Peri. ID2 Register                         Address offset: 0xFE8 */
	__IO uint32_t UART_PeriphID3;   /* Peri. ID3 Register (0x00)                  Address offset: 0xFEC */
	__IO uint32_t UART_CellID0;     /* Cell ID0 Register  (0x0D)                  Address offset: 0xFF0 */
	__IO uint32_t UART_CellID1;     /* Cell ID1 Register  (0xF0)                  Address offset: 0xFF4 */
	__IO uint32_t UART_CellID2;     /* Cell ID2 Register  (0x05)                  Address offset: 0xFF8 */
	__IO uint32_t UART_CellID3;     /* Cell ID3 Register  (0xB1)                  Address offset: 0xFFC */
} UART_TypeDef;

/*
 * Pkfb
 */
typedef struct
{
  __IO uint32_t PKFB_FIFOCTRL;              /* PktFIFO Control Register 0x00 */
  __IO uint32_t PKFB_FIFOSRAMCTRL0;         /* SRAM Test Control Register 0x04 */
  __IO uint32_t PKFB_FIFOSRAMCTRL1;         /* SRAM Test Control Register,              Address offset: 0x08 */
  __IO uint32_t PKFB_FIFOSTATUS;            /* FIFO Status Register,                    Address offset: 0x0C */
  __IO uint32_t PKFB_PF0PUSHCTRL;           /* FIFO 0 Push Control,                     Address offset: 0x10 */
  __IO uint32_t PKFB_PF0POPCTRL;            /* FIFO 0 Pop Control,                      Address offset: 0x14 */
  __IO uint32_t PKFB_PF0CNT;                /* FIFO 0 Count,                            Address offset: 0x18 */
  __IO uint32_t PKFB_PF0DATA;               /* FIFO 0 Push/Pop Data Register,           Address offset: 0x1C */
  __IO uint32_t PKFB_PF1PUSHCTRL;           /* FIFO 1 Push Control,                     Address offset: 0x20 */
  __IO uint32_t PKFB_PF1POPCTRL;            /* FIFO 1 Pop Control,                      Address offset: 0x24 */
  __IO uint32_t PKFB_PF1CNT;                /* FIFO 1 Count,                            Address offset: 0x28 */
  __IO uint32_t PKFB_PF1DATA;               /* FIFO 1 Push/Pop Data Register,           Address offset: 0x2C */
  __IO uint32_t PKFB_PF2PUSHCTRL;           /* FIFO 2 Push Control,                     Address offset: 0x30 */
  __IO uint32_t PKFB_PF2POPCTRL;            /* FIFO 2 Pop Control,                      Address offset: 0x34 */
  __IO uint32_t PKFB_PF2CNT;                /* FIFO 2 Count,                            Address offset: 0x38 */
  __IO uint32_t PKFB_PF2DATA;               /* FIFO 2 Push/Pop Data Register,           Address offset: 0x3C */
  __IO uint32_t PKFB_PF8KPUSHCTRL;          /* FIFO 8K Push Control,                    Address offset: 0x40 */
  __IO uint32_t PKFB_PF8KPOPCTRL;           /* FIFO 8K Pop Control,                     Address offset: 0x44 */
  __IO uint32_t PKFB_PF8KCNT;               /* FIFO 8K Count,                           Address offset: 0x48 */
  __IO uint32_t PKFB_PF8KDATA;              /* FIFO 8K Push/Pop Data Register,          Address offset: 0x4C */
  __IO uint32_t PKFB_FIFO_COLL_INTR;        /* FIFO Collision Interrupt Register,       Address offset: 0x50 */
  __IO uint32_t PKFB_FIFO_COLL_INTR_EN;     /* FIFO Collision Interrupt Enable Register Address offset: 0x54 */
} PKFB_TypeDef;

typedef struct
{
	__IO uint32_t CLK_CTRL_A_0;			/* For Clock 1 & 10 0x00 */
	__IO uint32_t CLK_CTRL_A_1;			/* 0x04 */
	__IO uint32_t CLK_CTRL_B_0;			/* For Clock 2 0x08 */
	__IO uint32_t reserved;				/* 0x0C */
	__IO uint32_t CLK_CTRL_C_0;			/* For Clock 8 0x10 */
	__IO uint32_t CLK_CTRL_D_0;			/* For Clock 11  0x14 */
	__IO uint32_t CLK_CTRL_E_0;			/* For Clock 12  0x18 */
	__IO uint32_t reserved1;
	__IO uint32_t CLK_CTRL_F_0;			/* For Clock 16	0x20 */
	__IO uint32_t CLK_CTRL_F_1;			/* 0x24 */
	__IO uint32_t CLK_CTRL_G_0;			/* For Clock 18 0x28 */
	__IO uint32_t CLK_CTRL_H_0;			/* For Clock 19 0x2C */
	__IO uint32_t reserved2;
	__IO uint32_t CLK_CTRL_I_0;			/* 0x34 */
	__IO uint32_t CLK_CTRL_I_1;			/* 0x38 */
	__IO uint32_t reverved3;			/* 0x3C */
	__IO uint32_t C01_CLK_GATE;			/* 0x40 */
	__IO uint32_t C02_CLK_GATE;			/* 0x44 */
	__IO uint32_t C08_X4_CLK_GATE;		/* 0x48 */
	__IO uint32_t C08_X1_CLK_GATE;		/* 0x4C */
	__IO uint32_t C10_FCLK_GATE;			/* 0x50 */
	__IO uint32_t C11_CLK_GATE;			/* 0x54 */
	__IO uint32_t C12_CLK_GATE;			/* 0x58 */
	__IO uint32_t CS_CLK_GATE;			/* 0x5C */
	__IO uint32_t CU_CLK_GATE;			/* 0x60 */
	__IO uint32_t C16_CLK_GATE;			/* 0x64 */
	__IO uint32_t reversed4;			/* 0x68 */
	__IO uint32_t C19_CLK_GATE;			/* 0x6C */
	__IO uint32_t C21_CLK_GATE;			/* 0x70 */
	__IO uint32_t reversed5[3];
	__IO uint32_t PF_SW_RESET;			/* 0x80 */
	__IO uint32_t FFE_SW_RESET;			/* 0x84 */
	__IO uint32_t FB_SW_RESET;			/* 0x88 */
	__IO uint32_t A1_SW_RESET;			/* 0x8C */
	__IO uint32_t AUDIO_MISC_SW_RST;	/* 0x90 */
	__IO uint32_t FB_MISC_SW_RST_CTL;	/* 0x94 */
	__IO uint32_t reversed6[26];
	__IO uint32_t CLK_CTRL_PMU;			/* 0x100 */
	__IO uint32_t CRU_GENERAL;		    /* 0x104 */
	__IO uint32_t CRU_DEBUG;			/* 0x108 */
	__IO uint32_t reversed7[1];
	__IO uint32_t C01_CLK_DIV;			/* 0x110 */
	__IO uint32_t C09_CLK_DIV;			/* 0x114 */
	__IO uint32_t C31_CLK_DIV;			/* 0x118 */
	__IO uint32_t C09_CLK_GATE;			/* 0x11C */
	__IO uint32_t C30_31_CLK_GATE;		/* 0x120 */
	__IO uint32_t CLK_DIVIDER_CLK_GATING;	/* 0x124 */
	__IO uint32_t reserved8[2];
	__IO uint32_t CLK_SWITCH_FOR_B;
	__IO uint32_t CLK_SWITCH_FOR_C;
	__IO uint32_t CLK_SWITCH_FOR_D;
	__IO uint32_t CLK_SWITCH_FOR_H;
	__IO uint32_t CLK_SWITCH_FOR_J;
	__IO uint32_t CLK_SWITCH_FOR_G;
} CRU_TypeDef;

typedef struct
{
	__IO uint32_t MISC_POR_0;               /* 0x000 */
	__IO uint32_t MISC_POR_1;               /* 0x004 */
	__IO uint32_t MISC_POR_2;               /* 0x008 */
	__IO uint32_t MISC_POR_3;               /* 0x00C */
	__IO uint32_t RST_CTRL_0;               /* 0x010 */
	__IO uint32_t RST_CTRL_1;               /* 0x014 */
	__IO uint32_t CHIP_STA_0;               /* 0x018 */
	__IO uint32_t CHIP_STA_1;               /* 0x01C */
	__IO uint32_t WIC_CTRL;                 /* 0x020 */
	__IO uint32_t WIC_STATUS;               /* 0x024 */
	__IO uint32_t reserved[2];
	__IO uint32_t PWR_DWN_SCH;              /* 0x030 */
	__IO uint32_t reserved1[3];
	__IO uint32_t PWR_OFF_OSC;              /* 0x040 */
	__IO uint32_t EXT_WAKING_UP_SRC;        /* 0x044 */
	__IO uint32_t reserved2[10];
	__IO uint32_t SDMA_STATUS;              /* 0x070 */
	__IO uint32_t SDMA_POWER_MODE_CFG;      /* 0x074 */
	__IO uint32_t SDMA_PD_SRC_MASK_N;       /* 0x078 */
	__IO uint32_t SDMA_WU_SRC_MASK_N;       /* 0x07C */
	__IO uint32_t M4_STATUS;                /* 0x080 */
	__IO uint32_t M4_PWR_MODE_CFG;          /* 0x084 */
	__IO uint32_t M4_PD_SRC_MASK_N;         /* 0x088 */
	__IO uint32_t M4_WU_SRC_MASK_N;         /* 0x08C */
	__IO uint32_t FFE_STATUS;               /* 0x090 */
	__IO uint32_t FFE_PWR_MODE_CFG;         /* 0x094 */
	__IO uint32_t FFE_PD_SRC_MASK_N;        /* 0x098 */
	__IO uint32_t FFE_WU_SRC_MASK_N;        /* 0x09C */
	__IO uint32_t FB_STATUS;                /* 0x0A0 */
	__IO uint32_t FB_PWR_MODE_CFG;          /* 0x0A4 */
	__IO uint32_t FB_PD_SRC_MASK_N;         /* 0x0A8 */
	__IO uint32_t FB_WU_SRC_MASK_N;         /* 0x0AC */
	__IO uint32_t PF_STATUS;                /* 0x0B0 */
	__IO uint32_t PF_PWR_MODE_CFG;          /* 0x0B4 */
	__IO uint32_t PF_PD_SRC_MASK_N;         /* 0x0B8 */
	__IO uint32_t PF_WU_SRC_MASK_N;         /* 0x0BC */
	__IO uint32_t M4S0_SRAM_STATUS;         /* 0x0C0 */
	__IO uint32_t M4S0_PWR_MODE_CFG;        /* 0x0C4 */
	__IO uint32_t M4S0_PD_SRC_MASK_N;       /* 0x0C8 */
	__IO uint32_t M4S0_WU_SRC_MASK_N;       /* 0x0CC */
	__IO uint32_t A1_STATUS; 		        /* 0x0D0 */
	__IO uint32_t A1_PWR_MODE_CFG;          /* 0x0D4 */
	__IO uint32_t A1_PD_SRC_MASK_N;         /* 0x0D8 */
	__IO uint32_t A1_WU_SRC_MASK_N;         /* 0x0DC */
	__IO uint32_t MISC_STATUS;              /* 0x0E0 */
	__IO uint32_t AUDIO_STATUS;             /* 0x0E4 */
	__IO uint32_t M4_SRAM_STATUS;            /* 0x0E8 */
	__IO uint32_t AUDIO_WU_SRC_MASK_N;      /* 0x0EC */
	__IO uint32_t reserved3[4];
	__IO uint32_t M4_MEM_CTRL_0;            /* 0x100 */
	__IO uint32_t M4_MEM_CTRL_1;            /* 0x104 */
	__IO uint32_t PF_MEM_CTRL_0;            /* 0x108 */
	__IO uint32_t PF_MEM_CTRL_1;            /* 0x10C */
	__IO uint32_t FFE_MEM_CTRL_0;           /* 0x110 */
	__IO uint32_t FFE_MEM_CTRL_1;           /* 0x114 */
	__IO uint32_t AUDIO_MEM_CTRL_0;         /* 0x118 */
	__IO uint32_t AUDIO_MEM_CTRL_1;			/* 0x11C */
	__IO uint32_t M4_MEM_CFG;				/* 0x120 */
	__IO uint32_t PF_MEM_CFG;				/* 0x124 */
	__IO uint32_t FFE_MEM_CFG;				/* 0x128 */
	__IO uint32_t AUDIO_MEM_CFG;			/* 0x12C */
	__IO uint32_t M4_MEM_CTRL_PWR_0;	/* 0x130 */
	__IO uint32_t M4_MEM_CTRL_PWR_1;	/* 0x134 */
	__IO uint32_t M4_MEM_CTRL_PWR_2;	/* 0x138 */
	__IO uint32_t reserved4[1];
	__IO uint32_t SDMA_MEM_CTRL_0;		/* 0x140 */
	__IO uint32_t SDMA_MEM_CTRL_1;		/* 0x144 */
	__IO uint32_t reserved5[14];
	__IO uint32_t MEM_PWR_DOWN_CTRL;	/* 0x180 */
	__IO uint32_t PMU_TIMER_CFG_0;		/* 0x184 */
	__IO uint32_t PMU_TIMER_CFG_1;		/* 0x188 */
	__IO uint32_t PDWU_TIMER_CFG;		/* 0x18C */
	__IO uint32_t reserved6[28];
	__IO uint32_t FFE_FB_PF_SW_PD;		/* 0x200 */
	__IO uint32_t M4_SRAM_SW_PD;		/* 0x204 */
	__IO uint32_t MISC_SW_PD;               /* 0x208 */
	__IO uint32_t AUDIO_SW_PD;              /* 0x20C */
	__IO uint32_t FFE_FB_PF_SW_WU;          /* 0x210 */
	__IO uint32_t M4_SRAM_SW_WU;             /* 0x214 */
	__IO uint32_t MISC_SW_WU;               /* 0x218 */
	__IO uint32_t AUD_SRAM_SW_WU;		/* 0x21C */
	__IO uint32_t PMU_STM_PRIORITY;         /* 0x220 */
	__IO uint32_t reserved7[3];
	__IO uint32_t M4SRAM_SSW_LPMF;			/* 0x230 */
	__IO uint32_t M4SRAM_SSW_LPMH_MASK_N;	/* 0x234 */
	__IO uint32_t reserved8[106];
	__IO uint32_t EFUSE_BITS;    	      /* 0x3E0 */
	__IO uint32_t reserved9[1];
	__IO uint32_t FBVLPMinWidth;          /* 0x3E8 */
	__IO uint32_t APRebootStatus;          /* 0x3EC */
	__IO uint32_t GEN_PURPOSE_0;            /* 0x3F0 */
	__IO uint32_t FB_ISOLATION;            /* 0x3F4 */
	__IO uint32_t GEN_PURPOSE_1;			/* 0x3F8 */

} PMU_TypeDef;

typedef struct
{
	__IO uint32_t ADDR;			/* 0x000 */
	__IO uint32_t WDATA;			/* 0x004 */
	__IO uint32_t CSR;				/* 0x008 */
	__IO uint32_t RDATA;			/* 0x00C */
	__IO uint32_t reserved0[1];		/* 0x010 */
	__IO uint32_t SRAM_TEST_REG1;		/* 0x014 */
	__IO uint32_t SRAM_TEST_REG2;		/* 0x018 */
	__IO uint32_t reserved1[1];		/* 0x01C */
	__IO uint32_t FFE_CSR;			/* 0x020 */
	__IO uint32_t reserved2[5];		/* 0x024 */
	__IO uint32_t FFE_DBG_COMBINED;		/* 0x038 */
	__IO uint32_t reserved3[49];		/* 0x03C */
	__IO uint32_t CMD;			/* 0x100 */
	__IO uint32_t reserved4[1];		/* 0x104 */
	__IO uint32_t INTERRUPT;		/* 0x108 */
	__IO uint32_t INTERRUPT_EN;		/* 0x10C */
	__IO uint32_t STATUS;			/* 0x110 */
	__IO uint32_t MAILBOX_TO_FFE0;		/* 0x114 */
	__IO uint32_t reserved5[2];		/* 0x118 */
	__IO uint32_t SM_RUNTIME_ADDR;		/* 0x120 */
	__IO uint32_t SM0_RUNTIME_ADDR_CTRL;	/* 0x124 */
	__IO uint32_t SM1_RUNTIME_ADDR_CTRL;	/* 0x128 */
	__IO uint32_t SM0_RUNTIME_ADDR_CUR;	/* 0x12C */
	__IO uint32_t SM1_RUNTIME_ADDR_CUR;	/* 0x130 */
	__IO uint32_t reserved6[3];		/* 0x134 */
	__IO uint32_t SM0_DEBUG_SEL;		/* 0x140 */
	__IO uint32_t SM1_DEBUG_SEL;		/* 0x144 */
	__IO uint32_t FFE_DEBUG_SEL;		/* 0x148 */
	__IO uint32_t reserved7[1];		/* 0x14C */
	__IO uint32_t FFE0_BREAK_POINT_CFG;	/* 0x150 */
	__IO uint32_t FFE0_BREAK_POINT_CONT;	/* 0x154 */
	__IO uint32_t FFE0_BREAK_POINT_STAT;	/* 0x158 */
	__IO uint32_t reserved8[1];		/* 0x15C */
	__IO uint32_t FFE0_BP_XPC_0;		/* 0x160 */
	__IO uint32_t FFE0_BP_XPC_1;		/* 0x164 */
	__IO uint32_t FFE0_BP_XPC_2;		/* 0x168 */
	__IO uint32_t FFE0_BP_XPC_3;		/* 0x16C */
} EXT_REGS_FFE_TypeDef;

typedef struct
{
	__IO uint32_t DBG_MON;			/* 0x000 */
	__IO uint32_t SUBSYS_DBG_MON_SEL;	/* 0x004 */
	__IO uint32_t A0_DBG_MON_SEL;		/* 0x008 */
	__IO uint32_t A0_PMU_DBG_MON_SEL;	/* 0x00C */
	__IO uint32_t reserved1[60];		/* 0x010 */
	__IO uint32_t IO_INPUT;			/* 0x100 */
	__IO uint32_t IO_OUTPUT;		/* 0x104 */
	__IO uint32_t reserved2[2];		/* 0x108 */
	__IO uint32_t SW_MB_1;			/* 0x110 */
	__IO uint32_t SW_MB_2;			/* 0x114 */
	__IO uint32_t reserved3[58];		/* 0x118 */
	__IO uint32_t PAD_SEL18;		/* 0x200 */
	__IO uint32_t reserved4[3];		/* 0x204 */
	__IO uint32_t CONFIG_MEM128_AON;	/* 0x210 */
	__IO uint32_t reserved5[63];		/* 0x214 */
	__IO uint32_t LOCK_KEY_CTRL;		/* 0x310 */
	__IO uint32_t reserved6[58];		/* 0x314 */
	__IO uint32_t FB_DEVICE_ID;		/* 0x3FC */
} MISC_CTRL_BASE_TypeDef;

/*
 * IO MUX
 */
typedef struct
{
    __IO uint32_t PAD_0_CTRL;               /* PAD 0 Control 0x000 */
    __IO uint32_t PAD_1_CTRL;               /* PAD 1 Control 0x004 */
    __IO uint32_t PAD_2_CTRL;               /* PAD 2 Control 0x008 */
    __IO uint32_t PAD_3_CTRL;               /* PAD 3 Control 0x00C */
    __IO uint32_t PAD_4_CTRL;               /* PAD 4 Control 0x010 */
    __IO uint32_t PAD_5_CTRL;               /* PAD 5 Control 0x014 */
    __IO uint32_t PAD_6_CTRL;               /* PAD 6 Control 0x018 */
    __IO uint32_t PAD_7_CTRL;               /* PAD 7 Control 0x01C */
    __IO uint32_t PAD_8_CTRL;               /* PAD 8 Control 0x020 */
    __IO uint32_t PAD_9_CTRL;               /* PAD 9 Control 0x024 */
    __IO uint32_t PAD_10_CTRL;              /* PAD 10 Control 0x028 */
    __IO uint32_t PAD_11_CTRL;              /* PAD 11 Control 0x02C */
    __IO uint32_t PAD_12_CTRL;              /* PAD 12 Control 0x030 */
    __IO uint32_t PAD_13_CTRL;              /* PAD 13 Control 0x034 */
    __IO uint32_t PAD_14_CTRL;              /* PAD 14 Control 0x038 */
    __IO uint32_t PAD_15_CTRL;              /* PAD 15 Control 0x03C */
    __IO uint32_t PAD_16_CTRL;              /* PAD 16 Control 0x040 */
    __IO uint32_t PAD_17_CTRL;              /* PAD 17 Control 0x044 */
    __IO uint32_t PAD_18_CTRL;              /* PAD 18 Control 0x048 */
    __IO uint32_t PAD_19_CTRL;              /* PAD 19 Control 0x04C */
    __IO uint32_t PAD_20_CTRL;              /* PAD 20 Control 0x050 */
    __IO uint32_t PAD_21_CTRL;              /* PAD 21 Control 0x054 */
    __IO uint32_t PAD_22_CTRL;              /* PAD 22 Control 0x058 */
    __IO uint32_t PAD_23_CTRL;              /* PAD 23 Control 0x05C */
    __IO uint32_t PAD_24_CTRL;              /* PAD 24 Control 0x060 */
    __IO uint32_t PAD_25_CTRL;              /* PAD 25 Control 0x064 */
    __IO uint32_t PAD_26_CTRL;              /* PAD 26 Control 0x068 */
    __IO uint32_t PAD_27_CTRL;              /* PAD 27 Control 0x06C */
    __IO uint32_t PAD_28_CTRL;              /* PAD 28 Control 0x070 */
    __IO uint32_t PAD_29_CTRL;              /* PAD 29 Control 0x074 */
    __IO uint32_t PAD_30_CTRL;              /* PAD 30 Control 0x078 */
    __IO uint32_t PAD_31_CTRL;              /* PAD 31 Control 0x07C */
    __IO uint32_t PAD_32_CTRL;              /* PAD 32 Control 0x080 */
    __IO uint32_t PAD_33_CTRL;              /* PAD 33 Control 0x084 */
    __IO uint32_t PAD_34_CTRL;              /* PAD 34 Control 0x088 */
    __IO uint32_t PAD_35_CTRL;              /* PAD 35 Control 0x08C */
    __IO uint32_t PAD_36_CTRL;              /* PAD 36 Control 0x090 */
    __IO uint32_t PAD_37_CTRL;              /* PAD 37 Control 0x094 */
    __IO uint32_t PAD_38_CTRL;              /* PAD 38 Control 0x098 */
    __IO uint32_t PAD_39_CTRL;              /* PAD 39 Control 0x09C */
    __IO uint32_t PAD_40_CTRL;              /* PAD 40 Control 0x0A0 */
    __IO uint32_t PAD_41_CTRL;              /* PAD 41 Control 0x0A4 */
    __IO uint32_t PAD_42_CTRL;              /* PAD 42 Control 0x0A8 */
    __IO uint32_t PAD_43_CTRL;              /* PAD 43 Control 0x0AC */
    __IO uint32_t PAD_44_CTRL;              /* PAD 44 Control 0x0B0 */
    __IO uint32_t PAD_45_CTRL;              /* PAD 45 Control 0x0B4 */
    __IO uint32_t reserved1[18];
    __IO uint32_t SDA0_SEL_REG;                 /* Address offset: 0x100 */
    __IO uint32_t SDA1_SEL_REG;                 /* Address offset: 0x104 */
    __IO uint32_t SDA2_SEL_REG;                 /* Address offset: 0x108 */
    __IO uint32_t SCL0_SEL_REG;                 /* Address offset: 0x10C */
    __IO uint32_t SCL1_SEL_REG;                 /* Address offset: 0x110 */
    __IO uint32_t SCL2_SEL_REG;                 /* Address offset: 0x114 */
    __IO uint32_t SPIs_CLK_SEL;             /* Address offset: 0x118 */
    __IO uint32_t SPIs_SSn_SEL;             /* Address offset: 0x11C */
    __IO uint32_t SPIs_MOSI_SEL;            /* Address offset: 0x120 */
    __IO uint32_t SPIm_MISO_SEL;            /* Address offset: 0x124 */
    __IO uint32_t PDM_DATA_SELE;             /* Address offset: 0x128 */
    __IO uint32_t I2S_DATA_SELECT;             /* Address offset: 0x12C */
    __IO uint32_t reserved2[1];
    __IO uint32_t UART_rxd_SEL;             /* Address offset: 0x134 */
    __IO uint32_t IrDA_Sirin_SEL;           /* Address offset: 0x138 */
    __IO uint32_t S_INTR_0_SEL_REG;             /* Address offset: 0x13C */
    __IO uint32_t S_INTR_1_SEL_REG;             /* Address offset: 0x140 */
    __IO uint32_t S_INTR_2_SEL;             /* Address offset: 0x144 */
    __IO uint32_t S_INTR_3_SEL;             /* Address offset: 0x148 */
    __IO uint32_t S_INTR_4_SEL;             /* Address offset: 0x14C */
    __IO uint32_t S_INTR_5_SEL;             /* Address offset: 0x150 */
    __IO uint32_t S_INTR_6_SEL;             /* Address offset: 0x154 */
    __IO uint32_t S_INTR_7_SEL;             /* Address offset: 0x158 */
    __IO uint32_t NUARTCTS_SEL;             /* Address offset: 0x15C */
    __IO uint32_t IO_REG_SEL;               /* Address offset: 0x160 */
    __IO uint32_t reserved3[3];
    __IO uint32_t SW_CLK_SEL;               /* Address offset: 0x170 */
    __IO uint32_t SW_IO_SEL;                /* Address offset: 0x174 */
    __IO uint32_t reserved4[2];
    __IO uint32_t FBIO_SEL_1;                 /* Address offset: 0x180 */
    __IO uint32_t FBIO_SEL_2;                 /* Address offset: 0x184 */
    __IO uint32_t reserved5[2];
    __IO uint32_t SPI_SENSOR_MISO_SEL;	/* Address offset: 0x190 */
    __IO uint32_t SPI_SENSOR_MOSI_SEL;	/* Address offset: 0x194 */
    __IO uint32_t reserved6[2];
    __IO uint32_t I2S_WD_CLKIN_SEL;	/* Address offset: 0x1A0 */
    __IO uint32_t I2S_CLKIN_SEL;	/* Address offset: 0x1A4 */
    __IO uint32_t PDM_STAT_IN_SEL;	/* Address offset: 0x1A8 */
    __IO uint32_t PDM_CLKIN_SEL;	/* Address offset: 0x1AC */
} IO_MUX_TypeDef;

typedef struct
{
	__IO uint32_t SPT_CFG;
	__IO uint32_t SLEEP_MODE;
	__IO uint32_t ERR_CMP_40M;
	__IO uint32_t ERR_CMP_1S_0;
	__IO uint32_t ERR_CMP_1S_1;
	__IO uint32_t ERR_CMP_1S_2;
	__IO uint32_t ERR_CMP_1S_3;
	__IO uint32_t ERR_CMP_RTC_0;
	__IO uint32_t ERR_CMP_RTC_1;
	__IO uint32_t ERR_CMP_RTC_2;
	__IO uint32_t ERR_CMP_RTC_3;
	__IO uint32_t UPDATE_TMR_VAL;
	__IO uint32_t SPARE_BITS;
	__IO uint32_t TIMER_VAL;
	__IO uint32_t EVENT_CNT_VAL;
	__IO uint32_t MS_CNT_VAL;

}SPT_REGS_TypeDef;

typedef struct
{
    __IO uint32_t GPIO_INTR;                /* Address offset: 0x00 */
    __IO uint32_t GPIO_INTR_RAW;            /* Address offset: 0x04 */
    __IO uint32_t GPIO_INTR_TYPE;           /* Address offset: 0x08 */
    __IO uint32_t GPIO_INTR_POL;            /* Address offset: 0x0C */
    __IO uint32_t GPIO_INTR_EN_AP;          /* Address offset: 0x10 */
    __IO uint32_t GPIO_INTR_EN_M4;          /* Address offset: 0x14 */
    __IO uint32_t GPIO_INTR_EN_FFE0;        /* Address offset: 0x18 */
    __IO uint32_t GPIO_INTR_EN_FFE1;        /* Address offset: 0x1C */
    __IO uint32_t reserved1[4];
    __IO uint32_t OTHER_INTR;               /* Address offset: 0x30 */
    __IO uint32_t OTHER_INTR_EN_AP;         /* Address offset: 0x34 */
    __IO uint32_t OTHER_INTR_EN_M4;         /* Address offset: 0x38 */
    __IO uint32_t reserved2[1];
    __IO uint32_t SOFTWARE_INTR_1;          /* Address offset: 0x40 */
    __IO uint32_t SOFTWARE_INTR_1_EN_AP;    /* Address offset: 0x44 */
    __IO uint32_t SOFTWARE_INTR_1_EN_M4;    /* Address offset: 0x48 */
    __IO uint32_t reserved3[1];
    __IO uint32_t SOFTWARE_INTR_2;          /* Address offset: 0x50 */
    __IO uint32_t SOFTWARE_INTR_2_EN_AP;    /* Address offset: 0x54 */
    __IO uint32_t SOFTWARE_INTR_2_EN_M4;    /* Address offset: 0x58 */
    __IO uint32_t reserved4[1];
    __IO uint32_t FFE_INTR;                 /* Address offset: 0x60 */
    __IO uint32_t FFE_INTR_EN_AP;           /* Address offset: 0x64 */
    __IO uint32_t FFE_INTR_EN_M4;           /* Address offset: 0x68 */
    __IO uint32_t reserved5[1];
    __IO uint32_t FFE1_FB_INTR;             /* Address offset: 0x70 */
    __IO uint32_t FFE1_FB_INTR_EN_AP;       /* Address offset: 0x74 */
    __IO uint32_t FFE1_FB_INTR_EN_M4;       /* Address offset: 0x78 */
    __IO uint32_t reserved6[1];
    __IO uint32_t FB_INTR;                  /* Address offset: 0x80 */
    __IO uint32_t FB_INTR_RAW;              /* Address offset: 0x84 */
    __IO uint32_t FB_INTR_TYPE;             /* Address offset: 0x88 */
    __IO uint32_t FB_INTR_POL;              /* Address offset: 0x8C */
    __IO uint32_t FB_INTR_EN_AP;            /* Address offset: 0x90 */
    __IO uint32_t FB_INTR_EN_M4;            /* Address offset: 0x94 */
    __IO uint32_t reserved7[2];
    __IO uint32_t M4_MEM_AON_INTR;	/* Address offset: 0xA0 */
    __IO uint32_t M4_MEM_AON_INTR_EN;	/* Address offset: 0xA4 */
} INTR_CTRL_TypeDef;

typedef struct
{
    __IO uint32_t CTRLR0;                   /* Control Register #0,                               Address offset: 0x00 */
    __IO uint32_t CTRLR1;                   /* Control Register #1,                               Address offset: 0x04 */
    __IO uint32_t SSIENR;                   /* SSI Enable Register,                               Address offset: 0x08 */
    __IO uint32_t reserved1;
    __IO uint32_t SER;                      /* Slave Enable Register,                             Address offset: 0x10 */
    __IO uint32_t BAUDR;                    /* Baud Rate Register,                                Address offset: 0x14 */
    __IO uint32_t TXFTLR;                   /* Tx FIFO Threshold Register,                        Address offset: 0x18 */
    __IO uint32_t RXFTLR;                   /* Rx FIFO Threshold Register,                        Address offset: 0x1C */
    __IO uint32_t TXFLR;                    /* Tx FIFO Level Register,                            Address offset: 0x20 */
    __IO uint32_t RXFLR;                    /* Rx FIFO Level Register,                            Address offset: 0x24 */
    __IO uint32_t SR;                       /* Status Register,                                   Address offset: 0x28 */
    __IO uint32_t IMR;                      /* Interrupt Mask Register,                           Address offset: 0x2C */
    __IO uint32_t ISR;                      /* Interrupt Status Register,                         Address offset: 0x30 */
    __IO uint32_t RISR;                     /* Interrupt Raw Status Register,                     Address offset: 0x34 */
    __IO uint32_t TXOICR;                   /* Tx FIFO Overflow Interrupt Clear Register,         Address offset: 0x38 */
    __IO uint32_t RXOICR;                   /* Rx FIFO Overflow Interrupt Clear Register,         Address offset: 0x3C */
    __IO uint32_t RXUICR;                   /* Rx FIFO Underflow Interrupt Clear Register,        Address offset: 0x40 */
    __IO uint32_t MSTICR;                   /* Multi-Master Contention Interrupt Clear Register,  Address offset: 0x44 */
    __IO uint32_t ICR;                      /* Interrupt Clear Register,                          Address offset: 0x48 */
    __IO uint32_t reserved2[3];
    __IO uint32_t IDR;
    __IO uint32_t SSI_COMP_VERSION;
    __IO uint32_t DR0;                      /* Data Register,                                     Address offset: 0x60 */
} SPI_TypeDef;


typedef struct
{
    __IO uint32_t IER;			/* I2S Enable Register 0x00 */
    __IO uint32_t reserved1[1];
    __IO uint32_t ITER;			/* Tx block enable 0x008 */
    __IO uint32_t reserved2[3];
    __IO uint32_t TXFFR;		/* Tx block fifo reset reg 0x018 */
    __IO uint32_t reserved3[1];
    __IO uint32_t LTHR0;		/* Left tx holding reg 0x020 */
    __IO uint32_t RTHR0;		/* Right tx holding reg 0x020 */
    __IO uint32_t reserved4[1];
    __IO uint32_t TER0;			/* Tx enable reg 0x02C */
    __IO uint32_t reserved5[1];
    __IO uint32_t TCR0;			/* Tx config reg 0x034 */
    __IO uint32_t ISR0;			/* Intr status reg 0x038 */
    __IO uint32_t IMR0;			/* Intr mask reg 0x03C */
    __IO uint32_t reserved6[1];
    __IO uint32_t TOR0;			/* Tx overrun reg 0x044 */
    __IO uint32_t reserved7[1];
    __IO uint32_t TFCR0;		/* Tx fifo config reg 0x04C */
    __IO uint32_t reserved8[1];
    __IO uint32_t TFF0;			/* Tx fifo flush reg 0x054 */
    __IO uint32_t reserved9[92];
    __IO uint32_t TXDMA;		/* Tx block dma reg 0x1C8 */
    __IO uint32_t RTXDMA;		/* Reset Tx block dma reg 0x1CC */
    __IO uint32_t reserved10[9];
    __IO uint32_t I2S_COMP_PARAM_1;	/* Component parm reg 0x1F4 */
    __IO uint32_t I2S_COMP_VERSION;	/* Component version reg 0x1F8 */
    __IO uint32_t I2S_COMP_TYPE;	/* Component type reg 0x1FC */
    __IO uint32_t resereved11[126];
    __IO uint32_t I2S_STEREO_EN;	/* Stereo enable reg 0x3F8 */
} I2S_TypeDef;

// Naveen - Aud Block definition according to tamar2 register spec available @ ${SVN_URL}/svn/Platform/Tamar2/Design/ASSP/doc/tamar2_regs.xlsx
typedef struct {
	__IO uint32_t VOICE_CONFIG;				// 0x000
	__IO uint32_t LPSD_CONFIG;					// 0x004
	__IO uint32_t VOICE_DMAC_CONFIG;			// 0x008
	__IO uint32_t VOICE_DMAC_LEN;				// 0x00C
	__IO uint32_t VOICE_DMAC_FIFO;				// 0x010
	__IO uint32_t VOICE_DMAC_DST_ADDR0;		// 0x014
	__IO uint32_t VOICE_DMAC_DST_ADDR1;		// 0x018
	__IO uint32_t PDM_CORE_CONFIG;				// 0x01C
	__IO uint32_t VOICE_STATUS;				// 0x020
	__IO uint32_t I2S_CONFIG;					// 0x024

} AUD_TypeDef;


typedef struct
{
	__IO uint32_t DMA_CTRL;		/* DMA Control Register 0x00 */
	__IO uint32_t DMA_DEST_ADDR;	/* DMA Destination Address 0x04 */
	__IO uint32_t DMA_XFER_CNT;	/* DMA Transfer Count 0x08 */
	__IO uint32_t CFG_FLASH_HEADER;	/* CFG FLASH Header 0x0C */
	__IO uint32_t DMA_INTR;		/* DMA Interrupt Register 0x10 */
	__IO uint32_t DMA_INTR_MASK;	/* DMA Interrupt Mask Register 0x14 */
} DMA_SPI_MS_TypeDef;


/*
 * System DMA
 */
typedef struct
{
    __IO uint32_t SRC_DATA_END_PTR_CH0;      /* Ptr to the end address of the source of Ch0    Address offset: 0x00 */
    __IO uint32_t DST_DATA_END_PTR_CH0;      /* Ptr to the end address of the dest of Ch0      Address offset: 0x04 */
    __IO uint32_t CH_CFG_CH0;                /* Configuration of Ch0                           Address offset: 0x08 */
    __IO uint32_t reserved0[1];
    __IO uint32_t SRC_DATA_END_PTR_CH1;      /* Ptr to the end address of the source of Ch1    Address offset: 0x10 */
    __IO uint32_t DST_DATA_END_PTR_CH1;      /* Ptr to the end address of the dest of Ch1      Address offset: 0x14 */
    __IO uint32_t CH_CFG_CH1;                /* Configuration of Ch1                           Address offset: 0x18 */
    __IO uint32_t reserved1[1];
    __IO uint32_t SRC_DATA_END_PTR_CH2;      /* Ptr to the end address of the source of Ch2    Address offset: 0x20 */
    __IO uint32_t DST_DATA_END_PTR_CH2;      /* Ptr to the end address of the dest of Ch2      Address offset: 0x24 */
    __IO uint32_t CH_CFG_CH2;                /* Configuration of Ch2                           Address offset: 0x28 */
    __IO uint32_t reserved2[1];
    __IO uint32_t SRC_DATA_END_PTR_CH3;      /* Ptr to the end address of the source of Ch3    Address offset: 0x30 */
    __IO uint32_t DST_DATA_END_PTR_CH3;      /* Ptr to the end address of the dest of Ch3      Address offset: 0x34 */
    __IO uint32_t CH_CFG_CH3;                /* Configuration of Ch3                           Address offset: 0x38 */
    __IO uint32_t reserved3[1];
    __IO uint32_t SRC_DATA_END_PTR_CH4;      /* Ptr to the end address of the source of Ch4    Address offset: 0x40 */
    __IO uint32_t DST_DATA_END_PTR_CH4;      /* Ptr to the end address of the dest of Ch4      Address offset: 0x44 */
    __IO uint32_t CH_CFG_CH4;                /* Configuration of Ch4                           Address offset: 0x48 */
    __IO uint32_t reserved4[1];
    __IO uint32_t SRC_DATA_END_PTR_CH5;      /* Ptr to the end address of the source of Ch5    Address offset: 0x50 */
    __IO uint32_t DST_DATA_END_PTR_CH5;      /* Ptr to the end address of the dest of Ch5      Address offset: 0x54 */
    __IO uint32_t CH_CFG_CH5;                /* Configuration of Ch5                           Address offset: 0x58 */
    __IO uint32_t reserved5[1];
    __IO uint32_t SRC_DATA_END_PTR_CH6;      /* Ptr to the end address of the source of Ch6    Address offset: 0x60 */
    __IO uint32_t DST_DATA_END_PTR_CH6;      /* Ptr to the end address of the dest of Ch6      Address offset: 0x64 */
    __IO uint32_t CH_CFG_CH6;                /* Configuration of Ch6                           Address offset: 0x68 */
    __IO uint32_t reserved6[1];
    __IO uint32_t SRC_DATA_END_PTR_CH7;      /* Ptr to the end address of the source of Ch7    Address offset: 0x70 */
    __IO uint32_t DST_DATA_END_PTR_CH7;      /* Ptr to the end address of the dest of Ch7      Address offset: 0x74 */
    __IO uint32_t CH_CFG_CH7;                /* Configuration of Ch7                           Address offset: 0x78 */
    __IO uint32_t reserved7[1];
    __IO uint32_t SRC_DATA_END_PTR_CH8;      /* Ptr to the end address of the source of Ch8    Address offset: 0x80 */
    __IO uint32_t DST_DATA_END_PTR_CH8;      /* Ptr to the end address of the dest of Ch8      Address offset: 0x84 */
    __IO uint32_t CH_CFG_CH8;                /* Configuration of Ch8                           Address offset: 0x88 */
    __IO uint32_t reserved8[1];
    __IO uint32_t SRC_DATA_END_PTR_CH9;      /* Ptr to the end address of the source of Ch9    Address offset: 0x90 */
    __IO uint32_t DST_DATA_END_PTR_CH9;      /* Ptr to the end address of the dest of Ch9      Address offset: 0x94 */
    __IO uint32_t CH_CFG_CH9;                /* Configuration of Ch9                           Address offset: 0x98 */
    __IO uint32_t reserved9[1];
    __IO uint32_t SRC_DATA_END_PTR_CH10;     /* Ptr to the end address of the source of Ch10   Address offset: 0xA0 */
    __IO uint32_t DST_DATA_END_PTR_CH10;     /* Ptr to the end address of the dest of Ch10     Address offset: 0xA4 */
    __IO uint32_t CH_CFG_CH10;               /* Configuration of Ch10                          Address offset: 0xA8 */
    __IO uint32_t reserved10[1];
    __IO uint32_t SRC_DATA_END_PTR_CH11;     /* Ptr to the end address of the source of Ch11   Address offset: 0xB0 */
    __IO uint32_t DST_DATA_END_PTR_CH11;     /* Ptr to the end address of the dest of Ch11     Address offset: 0xB4 */
    __IO uint32_t CH_CFG_CH11;               /* Configuration of Ch11                          Address offset: 0xB8 */
    __IO uint32_t reserved11[1];
    __IO uint32_t SRC_DATA_END_PTR_CH12;     /* Ptr to the end address of the source of Ch12   Address offset: 0xC0 */
    __IO uint32_t DST_DATA_END_PTR_CH12;     /* Ptr to the end address of the dest of Ch12     Address offset: 0xC4 */
    __IO uint32_t CH_CFG_CH12;               /* Configuration of Ch12                          Address offset: 0xC8 */
    __IO uint32_t reserved12[1];
    __IO uint32_t SRC_DATA_END_PTR_CH13;     /* Ptr to the end address of the source of Ch13   Address offset: 0xD0 */
    __IO uint32_t DST_DATA_END_PTR_CH13;     /* Ptr to the end address of the dest of Ch13     Address offset: 0xD4 */
    __IO uint32_t CH_CFG_CH13;               /* Configuration of Ch13                          Address offset: 0xD8 */
    __IO uint32_t reserved13[1];
    __IO uint32_t SRC_DATA_END_PTR_CH14;     /* Ptr to the end address of the source of Ch14   Address offset: 0xE0 */
    __IO uint32_t DST_DATA_END_PTR_CH14;     /* Ptr to the end address of the dest of Ch14     Address offset: 0xE4 */
    __IO uint32_t CH_CFG_CH14;               /* Configuration of Ch14                          Address offset: 0xE8 */
    __IO uint32_t reserved14[1];
    __IO uint32_t SRC_DATA_END_PTR_CH15;     /* Ptr to the end address of the source of Ch15   Address offset: 0xF0 */
    __IO uint32_t DST_DATA_END_PTR_CH15;     /* Ptr to the end address of the dest of Ch15     Address offset: 0xF4 */
    __IO uint32_t CH_CFG_CH15;               /* Configuration of Ch15                          Address offset: 0xF8 */
    __IO uint32_t reserved15[1];
    __IO uint32_t ALT_SRC_DATA_END_PTR_CH0;
    __IO uint32_t ALT_DST_DATA_END_PTR_CH0;
    __IO uint32_t ALT_CHN_CFG_CH0;
    __IO uint32_t resereved16[1];
    __IO uint32_t ALT_SRC_DATA_END_PTR_CH1;
    __IO uint32_t ALT_DST_DATA_END_PTR_CH1;
    __IO uint32_t ALT_CHN_CFG_CH1;
    __IO uint32_t resereved17[1];
    __IO uint32_t ALT_SRC_DATA_END_PTR_CH2;
    __IO uint32_t ALT_DST_DATA_END_PTR_CH2;
    __IO uint32_t ALT_CHN_CFG_CH2;
    __IO uint32_t resereved18[1];
    __IO uint32_t ALT_SRC_DATA_END_PTR_CH3;
	__IO uint32_t ALT_DST_DATA_END_PTR_CH3;
	__IO uint32_t ALT_CHN_CFG_CH3;
	__IO uint32_t resereved19[1];
	__IO uint32_t ALT_SRC_DATA_END_PTR_CH4;
	__IO uint32_t ALT_DST_DATA_END_PTR_CH4;
	__IO uint32_t ALT_CHN_CFG_CH4;
	__IO uint32_t resereved20[1];
	__IO uint32_t ALT_SRC_DATA_END_PTR_CH5;
	__IO uint32_t ALT_DST_DATA_END_PTR_CH5;
	__IO uint32_t ALT_CHN_CFG_CH5;
	__IO uint32_t resereved21[1];
	__IO uint32_t ALT_SRC_DATA_END_PTR_CH6;
	__IO uint32_t ALT_DST_DATA_END_PTR_CH6;
	__IO uint32_t ALT_CHN_CFG_CH6;
	__IO uint32_t resereved22[1];
	__IO uint32_t ALT_SRC_DATA_END_PTR_CH7;
	__IO uint32_t ALT_DST_DATA_END_PTR_CH7;
	__IO uint32_t ALT_CHN_CFG_CH7;
	__IO uint32_t resereved23[1];
	__IO uint32_t ALT_SRC_DATA_END_PTR_CH8;
	__IO uint32_t ALT_DST_DATA_END_PTR_CH8;
	__IO uint32_t ALT_CHN_CFG_CH8;
	__IO uint32_t resereved24[1];
	__IO uint32_t ALT_SRC_DATA_END_PTR_CH9;
	__IO uint32_t ALT_DST_DATA_END_PTR_CH9;
	__IO uint32_t ALT_CHN_CFG_CH9;
	__IO uint32_t resereved25[1];
	__IO uint32_t ALT_SRC_DATA_END_PTR_CH10;
	__IO uint32_t ALT_DST_DATA_END_PTR_CH10;
	__IO uint32_t ALT_CHN_CFG_CH10;
	__IO uint32_t resereved26[1];
	__IO uint32_t ALT_SRC_DATA_END_PTR_CH11;
	__IO uint32_t ALT_DST_DATA_END_PTR_CH11;
	__IO uint32_t ALT_CHN_CFG_CH11;
	__IO uint32_t resereved27[1];
	__IO uint32_t ALT_SRC_DATA_END_PTR_CH12;
	__IO uint32_t ALT_DST_DATA_END_PTR_CH12;
	__IO uint32_t ALT_CHN_CFG_CH12;
	__IO uint32_t resereved28[1];
	__IO uint32_t ALT_SRC_DATA_END_PTR_CH13;
	__IO uint32_t ALT_DST_DATA_END_PTR_CH13;
	__IO uint32_t ALT_CHN_CFG_CH13;
	__IO uint32_t resereved29[1];
	__IO uint32_t ALT_SRC_DATA_END_PTR_CH14;
	__IO uint32_t ALT_DST_DATA_END_PTR_CH14;
	__IO uint32_t ALT_CHN_CFG_CH14;
	__IO uint32_t resereved30[1];
	__IO uint32_t ALT_SRC_DATA_END_PTR_CH15;
	__IO uint32_t ALT_DST_DATA_END_PTR_CH15;

}SDMA_SRAM_TypeDef;

typedef struct
{
    __IO uint32_t SRC_DATA_END_PTR;      /* Ptr to the end address of the source */
    __IO uint32_t DST_DATA_END_PTR;      /* Ptr to the end address of the dest   */
    __IO uint32_t CH_CFG;                /* Configuration of Ch0                 */
    uint32_t reserved0;
}SDMA_SRAM_ENTRY_TypeDef;

typedef struct
{
    __IO uint32_t DMA_REQ;                   /* Dma Request                                    Address offset: 0x00 */
    __IO uint32_t DMA_WAITONREQ_REG;
    __IO uint32_t DMA_ACTIVE_REG;
    __IO uint32_t SDMA_PWRDN_CNT;
    __IO uint32_t SDMA_SRAM_CTRL;
}SDMA_BRIDGE_TypeDef;

typedef struct
{
    __IO uint32_t DMA_STATUS;                /* Dma status                                     Address offset: 0x00 */
    __IO uint32_t DMA_CFG;                   /* Dma configuration                              Address offset: 0x04 */
    __IO uint32_t CTRL_BASE_PTR;             /* Control base pointer                           Address offset: 0x08 */
    __IO uint32_t ALT_CTRL_BASE_PTR;         /* Alt. Control base pointer                      Address offset: 0x0C */
    __IO uint32_t DMA_WAITONREQ_STATUS;      /* Channel wait on req status                     Address offset: 0x10 */
    __IO uint32_t CHNL_SW_REQ;               /* Channel switch req                             Address offset: 0x14 */
    __IO uint32_t CHNL_USEBURST_SET;         /* Channel burst set                              Address offset: 0x18 */
    __IO uint32_t CHNL_USEBURST_CLR;         /* Channel burst clear                            Address offset: 0x1C */
    __IO uint32_t CHNL_REQ_MASK_SET;         /* Channel req mask set                           Address offset: 0x20 */
    __IO uint32_t CHNL_REQ_MASK_CLR;         /* Channel req mask clear                         Address offset: 0x24 */
    __IO uint32_t CHNL_ENABLE_SET;           /* Channel enable set                             Address offset: 0x28 */
    __IO uint32_t CHNL_ENABLE_CLR;           /* Channel enable clear                           Address offset: 0x2C */
    __IO uint32_t CHNL_PRI_ALT_SET;          /* Channel primary alt set                        Address offset: 0x30 */
    __IO uint32_t CHNL_PRI_ALT_CLR;          /* Channel primary alt clear                      Address offset: 0x34 */
    __IO uint32_t CHNL_PRIORITY_SET;         /* Channel priority set                           Address offset: 0x38 */
    __IO uint32_t CHNL_PRIORITY_CLR;         /* Channel priority clear                         Address offset: 0x3C */
    __IO uint32_t reserved0[3];
    __IO uint32_t ERR_CLR;                   /* Clear error                                    Address offset: 0x4C */
    __IO uint32_t reserved1[995];
    __IO uint32_t PERIPH_ID_4;               /* Peripheral identification 4                    Address offset: 0xFD0 */
    __IO uint32_t reserved2[3];
    __IO uint32_t PERIPH_ID_0;               /* Peripheral identification 0                    Address offset: 0xFE0 */
    __IO uint32_t PERIPH_ID_1;               /* Peripheral identification 1                    Address offset: 0xFE4 */
    __IO uint32_t PERIPH_ID_2;               /* Peripheral identification 2                    Address offset: 0xFE8 */
    __IO uint32_t PERIPH_ID_3;               /* Peripheral identification 3                    Address offset: 0xFEC */
    __IO uint32_t PCELL_ID_0;                /* PrimeCell  identification 0                    Address offset: 0xFF0 */
    __IO uint32_t PCELL_ID_1;                /* PrimeCell  identification 1                    Address offset: 0xFF4 */
    __IO uint32_t PCELL_ID_2;                /* PrimeCell  identification 2                    Address offset: 0xFF8 */
    __IO uint32_t PCELL_ID_3;                /* PrimeCell  identification 3                    Address offset: 0xFFC */

} SDMA_TypeDef;

// i2s slave typedef
typedef struct
{
    __IO uint32_t IER;		                // i2s enable register - offset 0x0
    __IO uint32_t reserved0;					// hole - offset: 0x04
    __IO uint32_t ITER;						// i2s transmitter block enable register - offset:0x8
    __IO uint32_t reserved1[3];					// holes - offset: 0x0C
    __IO uint32_t TXFFR;					// transmitter block fifo reset register - offset: 0x18
    __IO uint32_t reserved2;				// offset: 0x1c
    __IO uint32_t LTHR0;					// left transmit holding register - offset: 0x20
    __IO uint32_t RTHR0;					// right transmit holding register - offset: 0x24
    __IO uint32_t reserved3;					// hole - offset: 0x28
    __IO uint32_t TER0;						// transmit enable register - offset: 0x2C
    __IO uint32_t reserved4;					// hole - offset: 0x30
    __IO uint32_t TCR0;						// transmit configuration register - offset: 0x34
    __IO uint32_t ISR0;						// interrupt status register - offset: 0x38
    __IO uint32_t IMR0;						// interrupt mask register - offset: 0x3C
    __IO uint32_t reserved5;					// hole - offset: 0x40
    __IO uint32_t TOR0;						// transmit overrun register - offset: 0x44
    __IO uint32_t reserved6;					// hole - offset: 0x48
    __IO uint32_t TFCR0;					// transmit fifo configuration register - offset: 0x4C
    __IO uint32_t reserved7;					// hole - offset 0x50
    __IO uint32_t TFFO;						// transmit fifo flush - offset: 0x54
    __IO uint32_t reserved8[92];				// holes - offset: 0x58
    __IO uint32_t TXDMA;					// transmit block dma register - offset: 0x1C8
    __IO uint32_t RTXDMA;					// reset transmit block dma register - offset: 0x1CC
    __IO uint32_t reserved9[9];					// holes - offset: 0x1D0
    __IO uint32_t I2S_COMP_PARAM_1;			// component parameter register 1 - offset: 0x1F4
    __IO uint32_t I2S_COMP_VERSION;			// offset: 0x1F8
	__IO uint32_t I2S_COMP_TYPE;			// offset: 0x1FC
	__IO uint32_t hole9[126];				// holes - offset 0x200
	__IO uint32_t I2S_STEREO_EN;			// i2s stereo enable register - offset: 0x3F8
} I2S_SLAVE_TypeDef;

/*
 * External Interrupt/Event Controller
 */

typedef struct
{
  __IO uint32_t IMR;    /*!< EXTI Interrupt mask register,            Address offset: 0x00 */
  __IO uint32_t EMR;    /*!< EXTI Event mask register,                Address offset: 0x04 */
  __IO uint32_t RTSR;   /*!< EXTI Rising trigger selection register,  Address offset: 0x08 */
  __IO uint32_t FTSR;   /*!< EXTI Falling trigger selection register, Address offset: 0x0C */
  __IO uint32_t SWIER;  /*!< EXTI Software interrupt event register,  Address offset: 0x10 */
  __IO uint32_t PR;     /*!< EXTI Pending register,                   Address offset: 0x14 */
} EXTI_TypeDef;

typedef struct
{
	__IO uint32_t reserved;			/* Address offset: 0x00 */
	__IO uint32_t RTC_CTRL_1;		/* Address offset: 0x04 */
	__IO uint32_t RTC_CTRL_2;		/* Address offset: 0x08 */
	__IO uint32_t RTC_CTRL_3;		/* Address offset: 0x0C */
	__IO uint32_t RTC_CTRL_4;		/* Address offset: 0x10 */
	__IO uint32_t RTC_CTRL_5;		/* Address offset: 0x14 */
	__IO uint32_t RTC_CTRL_6;		/* Address offset: 0x18 */
	__IO uint32_t RTC_CTRL_7;		/* Address offset: 0x1C */
	__IO uint32_t RTC_STA_0;		/* Address offset: 0x20 */
	__IO uint32_t RTC_STA_1;		/* Address offset: 0x24 */
	__IO uint32_t reserved1[22];
	__IO uint32_t OSC_CTRL_0;		/* Address offset: 0x80 */
	__IO uint32_t OSC_CTRL_1;		/* Address offset: 0x84 */
	__IO uint32_t OSC_CTRL_2;		/* Address offset: 0x88 */
	__IO uint32_t OSC_CTRL_3;		/* Address offset: 0x8C */
	__IO uint32_t OSC_CTRL_4;		/* Address offset: 0x90 */
	__IO uint32_t OSC_CTRL_5;		/* Address offset: 0x94 */
	__IO uint32_t OSC_CTRL_6;		/* Address offset: 0x98 */
	__IO uint32_t OSC_CTRL_7;		/* Address offset: 0x9C */
	__IO uint32_t OSC_STA_0;		/* Address offset: 0xA0 */
	__IO uint32_t OSC_STA_1;		/* Address offset: 0xA4 */
	__IO uint32_t reserved2[22];
	__IO uint32_t APC_CTRL_0;		/* Address offset: 0x100 */
	__IO uint32_t APC_CTRL_1;		/* Address offset: 0x104 */
	__IO uint32_t APC_CTRL_2;		/* Address offset: 0x108 */
	__IO uint32_t APC_CTRL_3;		/* Address offset: 0x10C */
	__IO uint32_t APC_CTRL_4;		/* Address offset: 0x110 */
	__IO uint32_t APC_CTRL_5;		/* Address offset: 0x114 */
	__IO uint32_t APC_CTRL_6;		/* Address offset: 0x118 */
	__IO uint32_t APC_CTRL_7;		/* Address offset: 0x11C */
	__IO uint32_t APC_STA_0;		/* Address offset: 0x120 */
	__IO uint32_t APC_STA_1;		/* Address offset: 0x124 */
	__IO uint32_t reserved3[22];
	__IO uint32_t RING_OSC;			/* Address offset: 0x180 */
	__IO uint32_t reserved4[31];
	__IO uint32_t LD0_30_CTRL_0;
	__IO uint32_t LD0_30_CTRL_1;
	__IO uint32_t reserved5[2];
	__IO uint32_t LD0_50_CTRL_0;
	__IO uint32_t LD0_50_CTRL_1;
}AIP_Typedef;

/*
 * Watchdog timer
 */
typedef struct
{
  __IO uint32_t WDOGLOAD;                    /* WDOG Load Register                                     Address offset: 0x00 */
  __IO uint32_t WDOGVALUE;                   /* WDOG Current Value Register                            Address offset: 0x04 */
  __IO uint32_t WDOGCONTROL;                 /* WDOG Control Register                                  Address offset: 0x08 */
  __IO uint32_t WDOGINTCLR;                  /* WDOG Clear Int Register                                Address offset: 0x0C */
  __IO uint32_t WDOGRIS;                     /* WDOG Raw Int Status Register                           Address offset: 0x10 */
  __IO uint32_t WDOGMIS;                     /* WDOG Masked Int Status Register                        Address offset: 0x14 */
  __IO uint32_t reserved1[762];
  __IO uint32_t WDOGLOCK;                    /* WDOG Disable Write Register                            Address offset: 0xC00 */
  __IO uint32_t reserved2[191];
  __IO uint32_t WDOGITCR;                    /* WDOG Enable Integration Test Register                  Address offset: 0xF00 */
  __IO uint32_t WDOGITOP;                    /* WDOG Integration Test Output Register                  Address offset: 0xF00 */
  uint32_t reserved3[50];
  __IO uint32_t WDOGPERIPHID4;               /* WDOG Peripheral ID Register 4                          Address offset: 0xFD0 */
  __IO uint32_t WDOGPERIPHID5;               /* WDOG Peripheral ID Register 5                          Address offset: 0xFD4 */
  __IO uint32_t WDOGPERIPHID6;               /* WDOG Peripheral ID Register 6                          Address offset: 0xFD8 */
  __IO uint32_t WDOGPERIPHID7;               /* WDOG Peripheral ID Register 7                          Address offset: 0xFDC */
  __IO uint32_t WDOGPERIPHID0;               /* WDOG Peripheral ID Register 0                          Address offset: 0xFE0 */
  __IO uint32_t WDOGPERIPHID1;               /* WDOG Peripheral ID Register 1                          Address offset: 0xFE4 */
  __IO uint32_t WDOGPERIPHID2;               /* WDOG Peripheral ID Register 2                          Address offset: 0xFE8 */
  __IO uint32_t WDOGPERIPHID3;               /* WDOG Peripheral ID Register 3                          Address offset: 0xFEC */
  __IO uint32_t WDOGPCELLID0;                /* WDOG Component ID Register 0                           Address offset: 0xFF0 */
  __IO uint32_t WDOGPCELLID1;                /* WDOG Component ID Register 1                           Address offset: 0xFF4 */
  __IO uint32_t WDOGPCELLID2;                /* WDOG Component ID Register 2                           Address offset: 0xFF8 */
  __IO uint32_t WDOGPCELLID3;                /* WDOG Component ID Register 3                           Address offset: 0xFFC */
} WDT_TypeDef;

/*
 * CWM Batching Type Define
 */

 typedef struct
{
    __IO uint8_t TAG; /* Data Address: 0x00 */
    __IO uint8_t WRITE_POS;
    __IO uint8_t HDR_RESERVED0[2];
    __IO uint8_t READ_POS;
    __IO uint8_t HDR_RESERVED1[3];
    __IO uint8_t HDR_RESERVED2[120];
    __IO uint8_t DATA0[128];		/* Data Address: 0x80 */
    __IO uint8_t DATA1[128];		/* Data Address: 0x100 */
    __IO uint8_t DATA2[128];		/* Data Address: 0x180 */
    __IO uint8_t DATA3[128];		/* Data Address: 0x200 */
    __IO uint8_t DATA4[128];		/* Data Address: 0x280 */
    __IO uint8_t DATA5[128];		/* Data Address: 0x300 */
    __IO uint8_t DATA6[128];		/* Data Address: 0x380 */
} BATCHING_TypeDef;

typedef struct
{
    __IO uint8_t DATA[16]; /* Data Address: 0x00 */
} RECV_DATA_TypeDef;


typedef struct
{
	__IO uint32_t CTRL;
	__IO uint32_t VALUE;
	__IO uint32_t RELOAD;
	__IO uint32_t INTSTATUS_INTCLEAR;
	__IO uint32_t reserved[1008];
	__IO uint32_t PID4;
	__IO uint32_t PID5;
	__IO uint32_t PID6;
	__IO uint32_t PID7;
	__IO uint32_t PID0;
	__IO uint32_t PID1;
	__IO uint32_t PID2;
	__IO uint32_t PID3;
	__IO uint32_t CID0;
	__IO uint32_t CID1;
	__IO uint32_t CID2;
	__IO uint32_t CID3;
}TIMER_TypeDef;

/*
 * FPGA GPIO
 */
typedef struct
{
  	__IO uint32_t ID_VALUE;
	__IO uint32_t REV_LEVEL;
	__IO uint32_t GPIO_INPUT;
	__IO uint32_t GPIO_OUTPUT;
	__IO uint32_t GPIO_DIR_CTRL;
}FPGA_GPIO_TypeDef;

typedef struct
{
	__IO uint32_t UART_DR_DLLSB;    /* Receive/Transmit/DL_LSB: 0x00 */
	__IO uint32_t UART_IER_DLMSB;   /* Interrupt Enable Register/DL_MSB: 0x04 */
	__IO uint32_t UART_IIR_FCR;     /* Interrupt Identification Register, FIFO Control Register 0x08 */
	__IO uint32_t UART_LCR;         /* Line Control Register 0xC */
	__IO uint32_t UART_MCR;         /* Modem Control Register 0x10 */
	__IO uint32_t UART_LSR;         /* Line Status Register 0x14 */
	__IO uint32_t UART_MSR;         /* Modem Status Register 0x18 */
	__IO uint32_t UART_SR;          /* Scratch Register 0x1C */
} FPGA_UART_TypeDef;

/**
  * @brief Peripheral_memory_map
  */
#define SRAM1_BASE            ((uint32_t)0x20000000) /*!< SRAM1(128 KB) base address */
#define SRAM2_BASE            ((uint32_t)0x20020000) /*!< SRAM2(128 KB) base address */
#define SRAM3_BASE            ((uint32_t)0x20040000) /*!< SRAM3(128 KB) base address */

/* Legacy defines */
#define SRAM_BASE             SRAM1_BASE

/*!< Peripheral memory map */
#define PERIPH_BASE           (0x40000000)
#define AON_PERIPH_BASE       (PERIPH_BASE)
#define APB0_PERIPH_BASE      (PERIPH_BASE + 0x00010000)
#define FPGA_PERIPH_BASE      (PERIPH_BASE + 0x00020000)
#define FFE_PERIPH_BASE       (PERIPH_BASE + 0x00040000)

/* Bit bang memory map */
#define BB_BASE		      (0x42000000)
#define AON_BB_BASE	      (BB_BASE)
#define APB0_BB_BASE          (BB_BASE + 0x00200000)
#define FPGA_BB_BASE          (BB_BASE + 0x00400000)
#define FFE_BB_BASE           (BB_BASE + 0x00800000)

/* AON peripherals */
#define PKFB_BASE             (AON_PERIPH_BASE+0x2000)
#define CRU_BASE              (AON_PERIPH_BASE+0x4000)
#define PMU_BASE              (AON_PERIPH_BASE+0x4400)
#define INTR_CTRL_BASE        (AON_PERIPH_BASE+0x4800)
#define IO_MUX_BASE           (AON_PERIPH_BASE+0x4C00)
#define MISC_CTRL_BASE        (AON_PERIPH_BASE+0x5000)
#define AIP_BASE              (AON_PERIPH_BASE+0x5400)
#define JTM_BASE              (AON_PERIPH_BASE+0x5A00)
#define SPT_BASE	      (AON_PERIPH_BASE+0x5C00)
#define A1_REG_BASE           (AON_PERIPH_BASE+0x6000)
#define SPI_MS_BASE           (AON_PERIPH_BASE+0x7000)
#define DMA_SPI_MS_BASE       (AON_PERIPH_BASE+0x7400)
#define I2S_BASE              (AON_PERIPH_BASE+0x8000)
#define I2S_SLAVE_BASE        (AON_PERIPH_BASE+0xB000)
#define SDMA_BASE             (AON_PERIPH_BASE+0xC000)
#define SDMA_BRIDGE_BASE      (AON_PERIPH_BASE+0xD000)
#define SDMA_SRAM_BASE        (AON_PERIPH_BASE+0xF000)

/* APB0 peripherals */
#define UART_BASE	      (APB0_PERIPH_BASE)
#define WDT1_BASE             (APB0_PERIPH_BASE+0x2000)
#define TIMER1_BASE           (APB0_PERIPH_BASE+0x3000)
#define PIF_CTRL_BASE         (APB0_PERIPH_BASE+0x4000)
#define AUD_BASE              (APB0_PERIPH_BASE+0x5000)
#define RAM_FIFO0_BASE        (APB0_PERIPH_BASE+0x8000)
#define RAM_FIFO1_BASE        (APB0_PERIPH_BASE+0x9000)
#define RAM_FIFO2_BASE        (APB0_PERIPH_BASE+0xA000)
#define RAM_FIFO3_BASE        (APB0_PERIPH_BASE+0xB000)

/* FFE peripherals */
#define DM0_BASE	      (FFE_PERIPH_BASE)
#define SM0_BASE	      (FFE_PERIPH_BASE+0x04000)
#define SM1_BASE	      (FFE_PERIPH_BASE+0x08000)
#define EXT_REGS_FFE_BASE     (FFE_PERIPH_BASE+0x0A000)
#define DM_FFE1_BASE	      (FFE_PERIPH_BASE+0x0C000)
#define CM_BASE	              (FFE_PERIPH_BASE+0x10000)

/* M4 Internal Registers */

/* Debug MCU registers base address */
#define DBGMCU_BASE           ((uint32_t )0xE0042000)

#define FB_GPIO_BASE	       FPGA_PERIPH_BASE
#define FB_UART_BASE          (FPGA_PERIPH_BASE+0x1000)

/* IR register definitions */
#define FB_IR_DEVID_REG					(FPGA_PERIPH_BASE+(0x800)+(0x50<<2))	//	(FPGA_PERIPH_BASE+(0x50<<2))

/*
 * Peripheral_declaration
 */

#define UART			((UART_TypeDef *)UART_BASE)
#define PKFB			((PKFB_TypeDef *)PKFB_BASE)
#define IO_MUX			((IO_MUX_TypeDef *)IO_MUX_BASE)
#define INTR_CTRL		((INTR_CTRL_TypeDef *)INTR_CTRL_BASE)
#define SPI_MS			((SPI_TypeDef *) SPI_MS_BASE)
#define I2S				((I2S_TypeDef *) I2S_BASE)
#define AUD				((AUD_TypeDef *) AUD_BASE)
#define CRU				((CRU_TypeDef *) CRU_BASE)
#define PMU				((PMU_TypeDef *) PMU_BASE)
#define MISC			((MISC_CTRL_BASE_TypeDef *)MISC_CTRL_BASE)
#define MISC_CTRL		((MISC_CTRL_BASE_TypeDef *)MISC_CTRL_BASE)
#define WDT				((WDT_TypeDef *) WDT1_BASE)
#define DMA_SPI_MS		((DMA_SPI_MS_TypeDef *) DMA_SPI_MS_BASE)
#define I2S_SLAVE		((I2S_SLAVE_TypeDef *)I2S_SLAVE_BASE)
#define SDMA			((SDMA_TypeDef *) SDMA_BASE)
#define SDMA_BRIDGE    	((SDMA_BRIDGE_TypeDef *) SDMA_BRIDGE_BASE)
#define SDMA_SRAM_TAB	((SDMA_SRAM_ENTRY_TypeDef *) SDMA_SRAM_BASE)
#define EXT_REGS_FFE	((EXT_REGS_FFE_TypeDef *)EXT_REGS_FFE_BASE)
#define AIP				((AIP_Typedef*)AIP_BASE)
#define SPT				((SPT_REGS_TypeDef *)SPT_BASE)
#define FB_GPIO		((FPGA_GPIO_TypeDef *)FB_GPIO_BASE)
#define FB_UART         ((FPGA_UART_TypeDef *)FB_UART_BASE)
#define FB_I2C          ((FPGA_I2C_TypeDef *)FB_I2C_BASE)
#define FB_HRM		((FPGA_HRM_TypeDef *)FB_HRM_BASE)
#define TIMER		((TIMER_TypeDef *)TIMER1_BASE)
#define SHM_QL_BASE                   (0x2007C000)

/******************************************************************************
 * 				FFE                                           *
 ******************************************************************************/
#define FFE_CMD_RUN_FFE0_ONCE		((uint32_t) (0x00000001))
#define FFE_CMD_RUN_FFE1		((uint32_t) (0x00000002))
#define FFE_CMD_RUN_SM0_ONCE		((uint32_t) (0x00000004))
#define FFE_CMD_RUN_SM1_ONCE		((uint32_t) (0x00000008))

#define FFE_INTR_SM_MULT_WR_INTR	((uint32_t) (0x00000001))
#define FFE_INTR_FFE0_OVERRUN		((uint32_t) (0x00000002))
#define FFE_INTR_FFE1_SM1_OVERRUN	((uint32_t) (0x00000004))
#define FFE_INTR_FFE1_SM0_OVERRUN	((uint32_t) (0x00000008))
#define FFE_INTR_FFE0_SM1_OVERRUN	((uint32_t) (0x00000010))
#define FFE_INTR_FFE0_SM0_OVERRUN	((uint32_t) (0x00000020))
#define FFE_INTR_I2C_MS_1_ERROR		((uint32_t) (0x00000040))
#define FFE_INTR_I2C_MS_0_ERROR		((uint32_t) (0x00000080))
#define FFE_INTR_CM0_LP_INTR		((uint32_t) (0x00000100))
#define FFE_INTR_DM0_LP_INTR		((uint32_t) (0x00000200))
#define FFE_INTR_DM1_LP_INTR		((uint32_t) (0x00000400))
#define FFE_INTR_SM0_LP_INTR		((uint32_t) (0x00000800))
#define FFE_INTR_SM1_LP_INTR		((uint32_t) (0x00001000))
#define FFE_INTR_FFE0_BP_MATCH_INTR	((uint32_t) (0x00002000))
#define FFE_INTR_FFE1_OVERRUN		((uint32_t) (0x00004000))
#define FFE_INTR_PKFB_OVF_INTR		((uint32_t) (0x00008000))

#define FFE_SMO_BUSY			((uint32_t) (0x00000001))
#define FFE_SM1_BUSY			((uint32_t) (0x00000002))
#define FFE_FFEO_BUSY			((uint32_t) (0x00000004))
#define FFE_FFE1_BUSY			((uint32_t) (0x00000008))
#define FFE_FFEO_BG_FLAG		((uint32_t) (0x00000010))
#define FFE_FFE0_FG_FLAG		((uint32_t) (0x00000020))
/******************************************************************************
 * 				MISC                                          *
 ******************************************************************************/
#define MISC_RUN_FFE_CNT		((uint32_t) (0x00000001))
#define MISC_FFE_START_PERIOD_LATCH_EN	((uint32_t) (0x00000002))
#define MISC_PMU_START_PERIOD_LATCH_EN	((uint32_t) (0x00000004))
#define MISC_FFE_KICKOFF_MODE		((uint32_t) (0x00000008))
#define MISC_FFE_SPT_EN 		((uint32_t) (0x00000010))
#define MISC_FFE_SLEEP_MODE		((uint32_t) (0x00000020))
#define MISC_RUN_FFE_CONT_ASYNC		((uint32_t) (0x00000100))
#define MISC_FFE_KICKOFF_MODE_ASYNC	((uint32_t) (0x00000800))
#define MISC_FFE_SPT_EN_ASYNC		((uint32_t) (0x00001000))
#define MISC_FFE_SLEEP_MODE_ASYNC	((uint32_t) (0x00002000))

/******************************************************************************
 *                                   UART                                     *
 ******************************************************************************/

/* Bit definition for data register */
#define UART_DR_OVERRUN_ERR				  	  ((uint32_t) (1 << 11))
#define UART_DR_BREAK_ERR                     ((uint32_t) (1 << 10))
#define UART_DR_PARITY_ERR                    ((uint32_t) (1 << 9))
#define UART_DR_FRAMING_ERR                   ((uint32_t) (1 << 8))
#define UART_DR_DATA                          ((uint32_t) (0xFF))

/* Bit definition for rx status register */
#define UART_RSR_OVERRUN_ERR                  ((uint32_t) (1 << 3))
#define UART_RSR_BREAK_ERR                    ((uint32_t) (1 << 2))
#define UART_RSR_PARITY_ERR                   ((uint32_t) (1 << 1))
#define UART_RSR_FRAMING_ERR                  ((uint32_t) (1 << 0))

/* Bit definition for flag register */
#define UART_TFR_RING_INDICATOR               ((uint32_t) (1 << 8))
#define UART_TFR_TX_FIFO_EMPTY                ((uint32_t) (1 << 7))
#define UART_TFR_RX_FIFO_FULL                 ((uint32_t) (1 << 6))
#define UART_TFR_TX_FIFO_FULL                 ((uint32_t) (1 << 5))
#define UART_TFR_RX_FIFO_EMPTY                ((uint32_t) (1 << 4))
#define UART_TFR_BUSY                         ((uint32_t) (1 << 3))
#define UART_TFR_DATA_CARRY_DETECT            ((uint32_t) (1 << 2))
#define UART_TFR_DATA_SET_RDY                 ((uint32_t) (1 << 1))
#define UART_TFR_CLEAR_TO_SEND                ((uint32_t) (1 << 0))

/* Bit definition for IrDA low power counter register */
#define UART_ILPR_LOW_POWER_DIVISOR           ((uint32_t) (0xFF))

/* Bit definition for integer baud rate register */
#define UART_IBRD_BAUD_INT_DIVISOR            ((uint32_t) (0xFFFF))


/* Bit definition for fractional baud rate register */
#define UART_IBRD_BAUD_FRACT_DIVISOR          ((uint32_t) (0x1F))

/* Bit definition for line control register */
#define UART_LCR_STICK_PARITY_SELECT          ((uint32_t) (0x80))
#define UART_LCR_WLEN_8_BITS                  ((uint32_t) (0x60))
#define UART_LCR_WLEN_7_BITS                  ((uint32_t) (0x40))
#define UART_LCR_WLEN_6_BITS                  ((uint32_t) (0x20))
#define UART_LCR_WLEN_5_BITS                  ((uint32_t) (0x00))
#define UART_LCR_ENABLE_FIFO                  ((uint32_t) (0x10))
#define UART_LCR_TWO_STOP_BITS                ((uint32_t) (0x08))
#define UART_LCR_EVEN_PARITY                  ((uint32_t) (0x04))
#define UART_LCR_ODD_PARITY                   ((uint32_t) (0x00))
#define UART_LCR_PARITY_ENABLE                ((uint32_t) (0x02))
#define UART_LCR_SEND_BREAK                   ((uint32_t) (0x01))

/* Bit definition for control register */
#define UART_CR_CTS_ENABLE	                  ((uint32_t) (0x8000))
#define UART_CR_RTS_ENABLE	                  ((uint32_t) (0x4000))
#define UART_CR_OUT2	                      ((uint32_t) (0x2000))
#define UART_CR_OUT1	                      ((uint32_t) (0x1000))
#define UART_CR_RTS	                          ((uint32_t) (0x0800))
#define UART_CR_DTR  	                      ((uint32_t) (0x0400))
#define UART_CR_RX_ENABLE                     ((uint32_t) (0x0200))
#define UART_CR_TX_ENABLE                     ((uint32_t) (0x0100))
#define UART_CR_LOOPBACK_ENABLE               ((uint32_t) (0x0080))
#define UART_CR_SIR_LOWPOWER                  ((uint32_t) (0x0004))
#define UART_CR_SIR_ENABLE                    ((uint32_t) (0x0002))
#define UART_CR_UART_ENABLE                   ((uint32_t) (0x0001))

/* Bit definition for interrupt FIFO level select register */
#define UART_IFLS_RX_1_8_FULL                 ((uint32_t) (0x0000))
#define UART_IFLS_RX_1_4_FULL                 ((uint32_t) (0x0008))
#define UART_IFLS_RX_1_2_FULL                 ((uint32_t) (0x0010))
#define UART_IFLS_RX_3_4_FULL                 ((uint32_t) (0x0018))
#define UART_IFLS_RX_7_8_FULL                 ((uint32_t) (0x0020))
#define UART_IFLS_TX_1_8_FULL                 ((uint32_t) (0x0000))
#define UART_IFLS_TX_1_4_FULL                 ((uint32_t) (0x0001))
#define UART_IFLS_TX_1_2_FULL                 ((uint32_t) (0x0002))
#define UART_IFLS_TX_3_4_FULL                 ((uint32_t) (0x0003))
#define UART_IFLS_TX_7_8_FULL                 ((uint32_t) (0x0004))

/* Bit definition for interrupt mask set/clear register */
#define UART_IMSC_OVERRUN_ERR                 ((uint32_t) (0x0400))
#define UART_IMSC_BREAK_ERR                   ((uint32_t) (0x0200))
#define UART_IMSC_PARITY_ERR                  ((uint32_t) (0x0100))
#define UART_IMSC_FRAMING_ERR                 ((uint32_t) (0x0080))
#define UART_IMSC_RX_TIMEOUT                  ((uint32_t) (0x0040))
#define UART_IMSC_TX                          ((uint32_t) (0x0020))
#define UART_IMSC_RX                          ((uint32_t) (0x0010))
#define UART_IMSC_DSR                         ((uint32_t) (0x0008))
#define UART_IMSC_DCD                         ((uint32_t) (0x0004))
#define UART_IMSC_CTS                         ((uint32_t) (0x0002))
#define UART_IMSC_RI                          ((uint32_t) (0x0001))

/* Bit definition for raw interrupt status register */
#define UART_RIS_OVERRUN_ERR                  ((uint32_t) (0x0400))
#define UART_RIS_BREAK_ERR                    ((uint32_t) (0x0200))
#define UART_RIS_PARITY_ERR                   ((uint32_t) (0x0100))
#define UART_RIS_FRAMING_ERR                  ((uint32_t) (0x0080))
#define UART_RIS_RX_TIMEOUT                   ((uint32_t) (0x0040))
#define UART_RIS_TX                           ((uint32_t) (0x0020))
#define UART_RIS_RX                           ((uint32_t) (0x0010))
#define UART_RIS_DSR                          ((uint32_t) (0x0008))
#define UART_RIS_DCD                          ((uint32_t) (0x0004))
#define UART_RIS_CTS                          ((uint32_t) (0x0002))
#define UART_RIS_RI                           ((uint32_t) (0x0001))

/* Bit definition for masked interrupt status register */
#define UART_MIS_OVERRUN_ERR                  ((uint32_t) (0x0400))
#define UART_MIS_BREAK_ERR                    ((uint32_t) (0x0200))
#define UART_MIS_PARITY_ERR                   ((uint32_t) (0x0100))
#define UART_MIS_FRAMING_ERR                  ((uint32_t) (0x0080))
#define UART_MIS_RX_TIMEOUT                   ((uint32_t) (0x0040))
#define UART_MIS_TX                           ((uint32_t) (0x0020))
#define UART_MIS_RX                           ((uint32_t) (0x0010))
#define UART_MIS_DSR                          ((uint32_t) (0x0008))
#define UART_MIS_DCD                          ((uint32_t) (0x0004))
#define UART_MIS_CTS                          ((uint32_t) (0x0002))
#define UART_MIS_RI                           ((uint32_t) (0x0001))

/* Bit definition for interrupt clear register */
#define UART_IC_OVERRUN_ERR                  ((uint32_t) (0x0400))
#define UART_IC_BREAK_ERR                    ((uint32_t) (0x0200))
#define UART_IC_PARITY_ERR                   ((uint32_t) (0x0100))
#define UART_IC_FRAMING_ERR                  ((uint32_t) (0x0080))
#define UART_IC_RX_TIMEOUT                   ((uint32_t) (0x0040))
#define UART_IC_TX                           ((uint32_t) (0x0020))
#define UART_IC_RX                           ((uint32_t) (0x0010))
#define UART_IC_DSR                          ((uint32_t) (0x0008))
#define UART_IC_DCD                          ((uint32_t) (0x0004))
#define UART_IC_CTS                          ((uint32_t) (0x0002))
#define UART_IC_RI                           ((uint32_t) (0x0001))

/* Bit definition for dma control register */
#define UART_DMACR_ON_ERR                    ((uint32_t) (0x0004))
#define UART_DMACR_TX_DMA_ENABLE             ((uint32_t) (0x0002))
#define UART_DMACR_RX_DMA_ENABLE             ((uint32_t) (0x0001))

/******************************************************************************
 *                                   WDT                                      *
 ******************************************************************************/
#define WDOG_CTRL_RESEN                  ((uint32_t) (0x0002))
#define WDOG_CTRL_INTEN                  ((uint32_t) (0x0001))

/******************************************************************************
 *                                   DMA                                      *
 ******************************************************************************/
#define	DMA_CTRL_START_BIT		 ((uint32_t) (0x00000001))
#define DMA_CTRL_STOP_BIT		 ((uint32_t) (0x00000002))
#define DMA_CTRL_AHB_SEL_BIT		 ((uint32_t) (0x00000004))

#define DMA_INTR_HERROR			 ((uint32_t) (0x00000001))
#define DMA_INTR_RX_DATA_AVAIL		 ((uint32_t) (0x00000002))
#define DMA_INTR_AHB_BRIDGE_FIFO_OFL	 ((uint32_t) (0x00000004))

#define DMA_HERROR_INTR_MSK		 ((uint32_t) (0x00000001))
#define DMA_RX_DATA_AVAIL_INTR_MSK	 ((uint32_t) (0x00000002))
#define DMA_AHB_FIFO_OFL_INTR_MSK	 ((uint32_t) (0x00000004))

/******************************************************************************
 *                                   PKFB                                     *
 ******************************************************************************/

/* Bit definition for FIFO control register */
#define FIFO_CTRL_PF_ENABLE                 ((uint32_t) (0x00000001))
#define FIFO_CTRL_PF_PUSH_MUX_FFE           ((uint32_t) (0x00000002))
#define FIFO_CTRL_PF_PUSH_MUX_M4            ((uint32_t) (0x00000000))
#define FIFO_CTRL_PF_POP_MUX_AP             ((uint32_t) (0x00000004))
#define FIFO_CTRL_PF_POP_MUX_M4             ((uint32_t) (0x00000000))
#define FIFO_CTRL_PF_PUSH_INT_MUX_AP        ((uint32_t) (0x00000008))
#define FIFO_CTRL_PF_PUSH_INT_MUX_M4        ((uint32_t) (0x00000000))
#define FIFO_CTRL_PF_POP_INT_MUX_AP         ((uint32_t) (0x00000010))
#define FIFO_CTRL_PF_POP_INT_MUX_M4         ((uint32_t) (0x00000000))
#define FIFO_CTRL_PF_FFE_SEL_FFE1           ((uint32_t) (0x00000020))
#define FIFO_CTRL_PF_FFE_SEL_FFE0           ((uint32_t) (0x00000000))

#define FIFO_CTRL_PF0_SHIFT                  (0)
#define FIFO_CTRL_PF1_SHIFT					 (8)
#define FIFO_CTRL_PF2_SHIFT                  (16)
#define FIFO_CTRL_PF8K_SHIFT                 (24)

#define FIFO_SRAM_CTRL_PF_TEST1A_ENABLE           ((uint32_t) (0x00000000))
#define FIFO_SRAM_CTRL_PF_TEST1A_DISABLE          ((uint32_t) (0x00000001))
#define FIFO_SRAM_CTRL_PF_RMEA_ENABLE             ((uint32_t) (0x00000000))
#define FIFO_SRAM_CTRL_PF_RMEA_DISABLE            ((uint32_t) (0x00000002))
#define FIFO_SRAM_CTRL_PF_RMA_MASK                ((uint32_t) (0x0000003C))
#define FIFO_SRAM_CTRL_PF_TEST1B_ENABLE           ((uint32_t) (0x00000000))
#define FIFO_SRAM_CTRL_PF_TEST1B_DISABLE          ((uint32_t) (0x00000100))
#define FIFO_SRAM_CTRL_PF_RMEB_ENABLE             ((uint32_t) (0x00000000))
#define FIFO_SRAM_CTRL_PF_RMEB_DISABLE            ((uint32_t) (0x00000200))
#define FIFO_SRAM_CTRL_PF_RMB_MASK                ((uint32_t) (0x00003C00))

#define FIFO_SRAM_CTRL0_PF0_SHIFT 			(0)
#define FIFO_SRAM_CTRL0_PF1_SHIFT           (16)
#define FIFO_SRAM_CTRL1_PF2_SHIFT           (0)
#define FIFO_SRAM_CTRL1_PF8K_SHIFT          (16)

#define FIFO_STATUS_PF_SRAM_SLEEP_ACTIVE            ((uint32_t) (0x00000000))
#define FIFO_STATUS_PF_SRAM_SLEEP_LIGHT             ((uint32_t) (0x00000001))
#define FIFO_STATUS_PF_SRAM_SLEEP_DEEP              ((uint32_t) (0x00000002))
#define FIFO_STATUS_PF_SRAM_SLEEP_SHUTDOWN          ((uint32_t) (0x00000003))
#define FIFO_STATUS_PF_PUSH_INT_OVER                ((uint32_t) (0x00000004))
#define FIFO_STATUS_PF_PUSH_INT_THRESH              ((uint32_t) (0x00000008))
#define FIFO_STATUS_PF_PUSH_INT_SLEEP               ((uint32_t) (0x00000010))
#define FIFO_STATUS_PF_POP_INT_UNDER                ((uint32_t) (0x00000020))
#define FIFO_STATUS_PF_POP_INT_THRESH               ((uint32_t) (0x00000040))
#define FIFO_STATUS_PF_POP_INT_SLEEP                ((uint32_t) (0x00000080))

#define FIFO_STATUS_PF0_SHIFT				(0)
#define FIFO_STATUS_PF1_SHIFT				(8)
#define FIFO_STATUS_PF2_SHIFT				(16)
#define FIFO_STATUS_PF8K_SHIFT				(24)

#define PF_PUSH_CTRL_SLEEP_EN_ACTIVE                 ((uint32_t) (0x00000001))
#define PF_PUSH_CTRL_SLEEP_TYPE_SD                   ((uint32_t) (0x00000002))
#define PF_PUSH_CTRL_SLEEP_TYPE_DS                   ((uint32_t) (0x00000000))
#define PF_PUSH_CTRL_INT_EN_MASK                     ((uint32_t) (0x00000000))
#define PF_PUSH_CTRL_INT_EN_OVER                     ((uint32_t) (0x00000004))
#define PF_PUSH_CTRL_INT_EN_THRES                    ((uint32_t) (0x00000008))
#define PF_PUSH_CTRL_INT_EN_PUSH_ON_SLEEP            ((uint32_t) (0x00000010))
#define PF_PUSH_CTRL_PUSH_THRESH_MASK                ((uint32_t) (0x01FF0000))

#define PF_POP_CTRL_SLEEP_EN_ACTIVE                  ((uint32_t) (0x00000001))
#define PF_POP_CTRL_SLEEP_TYPE_SD                    ((uint32_t) (0x00000002))
#define PF_POP_CTRL_SLEEP_TYPE_DS                    ((uint32_t) (0x00000000))
#define PF_POP_CTRL_INT_EN_MASK                      ((uint32_t) (0x00000000))
#define PF_POP_CTRL_INT_EN_UNDER                     ((uint32_t) (0x00000004))
#define PF_POP_CTRL_INT_EN_THRES                     ((uint32_t) (0x00000008))
#define PF_POP_CTRL_INT_EN_POP_ON_SLEEP              ((uint32_t) (0x00000010))
#define PF_POP_CTRL_PUSH_THRESH_MASK                 ((uint32_t) (0x01FF0000))

#define PF_CNT_POP_CNT_MASK                          ((uint32_t) (0x000001FF))
#define PF_CNT_POP_EMPTY_MASK                        ((uint32_t) (0x00008000))
#define PF_CNT_PUSH_CNT_MASK                         ((uint32_t) (0x01FF0000))
#define PF_CNT_PUSH_EMPTY_MASK                       ((uint32_t) (0x80000000))

#define PF8K_DATA_REG_PUSH_EOP                         ((uint32_t) (0x00020000))

#define FIFO_COLL_INTR_PF_COLL_INTR                   ((uint32_t) (0x00000001))

#define FIFO_COLL_INTR_PF0_SHIFT		(0)
#define FIFO_COLL_INTR_PF1_SHIFT		(1)
#define FIFO_COLL_INTR_PF2_SHIFT		(2)
#define FIFO_COLL_INTR_PF8K_SHIFT		(3)

#define FIFO_COLL_INTR_EN_PF_COLL_INTR                ((uint32_t) (0x00000001))

#define FIFO_COLL_INTR_EN_PF0_SHIFT		(0)
#define FIFO_COLL_INTR_EN_PF1_SHIFT		(1)
#define FIFO_COLL_INTR_EN_PF2_SHIFT		(2)
#define FIFO_COLL_INTR_EN_PF8K_SHIFT	(3)

/******************************************************************************
 *                                   IO_MUX                                   *
 ******************************************************************************/

/* Common bit definition for all PAD control registers */
/*
#define PAD_CTRL_SEL_AO_REG			((uint32_t) (0x00000000))
#define PAD_CTRL_SEL_OTHER			((uint32_t) (0x00000008))
#define PAD_CTRL_SEL_FPGA			((uint32_t) (0x00000010))
#define PAD_OEN_DISABLE                             ((uint32_t) (0x00000020))
#define PAD_OEN_NORMAL                              ((uint32_t) (0x00000000))
#define PAD_P_Z                                     ((uint32_t) (0x00000000))
#define PAD_P_PULLUP                                ((uint32_t) (0x00000040))
#define PAD_P_PULLDOWN                              ((uint32_t) (0x00000080))
#define PAD_P_KEEPER                                ((uint32_t) (0x000000C0))
#define PAD_E_2MA                                   ((uint32_t) (0x00000000))
#define PAD_E_4MA                                   ((uint32_t) (0x00000100))
#define PAD_E_8MA                                   ((uint32_t) (0x00000200))
#define PAD_E_12MA                                  ((uint32_t) (0x00000300))
#define PAD_SR_FAST                                 ((uint32_t) (0x00000400))
#define PAD_SR_SLOW                                 ((uint32_t) (0x00000000))
#define PAD_REN_ENABLE                              ((uint32_t) (0x00000800))
#define PAD_REN_DISABLE                             ((uint32_t) (0x00000000))
#define PAD_SMT_ENABLE                              ((uint32_t) (0x00001000))
#define PAD_SMT_DISABLE                             ((uint32_t) (0x00000000))
*/

#define PAD_E_12MA                                  ((uint32_t) (0x00000300))
#define PAD_OEN_NORMAL                              ((uint32_t) (0x00000000))

#define SDA0_SEL_PAD1                               ((uint32_t) (0x00000001))
#define SDA1_SEL_PAD10                              ((uint32_t) (0x00000001))
#define SDA1_SEL_PAD32                              ((uint32_t) (0x00000002))
#define SCL1_SEL_PAD33                              ((uint32_t) (0x00000002))
#define SDA2_SEL_PAD34                              ((uint32_t) (0x00000001))
#define SCL0_SEL_PAD0                               ((uint32_t) (0x00000000))
#define SCL0_SEL_0                                  ((uint32_t) (0x00000001))
#define SCL1_SEL_0                                  ((uint32_t) (0x00000000))
#define SCL1_SEL_PAD9                               ((uint32_t) (0x00000001))
#define SCL2_SEL_0                                  ((uint32_t) (0x00000000))
#define SCL2_SEL_PAD33                              ((uint32_t) (0x00000001))

#define SPIS_CLK_SEL_PAD17                          ((uint32_t) (0x00000000))
#define SPIS_CLK_SEL_0                              ((uint32_t) (0x00000001))

#define SPIS_SSN_SEL_PAD20                          ((uint32_t) (0x00000000))
#define SPIS_SSN_SEL_0                              ((uint32_t) (0x00000001))

#define SPIS_MOSI_SEL_PAD19                         ((uint32_t) (0x00000000))
#define SPIS_MOSI_SEL_0                             ((uint32_t) (0x00000001))

#define SPIS_MISO_SEL_PAD23                         ((uint32_t) (0x00000000))
#define SPIS_MISO_SEL_0                             ((uint32_t) (0x00000001))

#define PDM_DATA_SEL_0                              ((uint32_t) (0x00000000))
#define PDM_DATA_SEL_PAD7                           ((uint32_t) (0x00000001))

#define I2S_DATA_SEL_0                              ((uint32_t) (0x00000000))
#define I2S_DATA_SEL_PAD7                           ((uint32_t) (0x00000001))

#define UART_RXD_SEL_0                              ((uint32_t) (0x00000000))
#define UART_RXD_SEL_PAD16                          ((uint32_t) (0x00000002))
#define UART_RXD_SEL_PAD45                          ((uint32_t) (0x00000004))

#define IRDA_SIRIN_SEL_0                            ((uint32_t) (0x00000000))
#define IRDA_SIRIN_SEL_PAD6                         ((uint32_t) (0x00000001))
#define IRDA_SIRIN_SEL_PAD11                        ((uint32_t) (0x00000002))
#define IRDA_SIRIN_SEL_PAD13                        ((uint32_t) (0x00000003))
#define IRDA_SIRIN_SEL_PAD26                        ((uint32_t) (0x00000004))
#define IRDA_SIRIN_SEL_PAD28                        ((uint32_t) (0x00000005))
#define IRDA_SIRIN_SEL_PAD31                        ((uint32_t) (0x00000006))
#define IRDA_SIRIN_SEL_PAD33                        ((uint32_t) (0x00000007))

#define S_INTR_0_SEL_0                              ((uint32_t) (0x00000000))
#define S_INTR_0_SEL_PAD2                           ((uint32_t) (0x00000001))

#define S_INTR_1_SEL_0                              ((uint32_t) (0x00000000))
#define S_INTR_1_SEL_PAD4                           ((uint32_t) (0x00000001))
#define S_INTR_1_SEL_PAD11                          ((uint32_t) (0x00000002))
#define S_INTR_1_SEL_PAD15                          ((uint32_t) (0x00000003))
#define S_INTR_1_SEL_PAD26                          ((uint32_t) (0x00000004))
#define S_INTR_1_SEL_PAD32                          ((uint32_t) (0x00000005))

#define S_INTR_2_SEL_0                              ((uint32_t) (0x00000000))
#define S_INTR_2_SEL_PAD5                           ((uint32_t) (0x00000001))
#define S_INTR_2_SEL_PAD12                          ((uint32_t) (0x00000002))
#define S_INTR_2_SEL_PAD16                          ((uint32_t) (0x00000003))
#define S_INTR_2_SEL_PAD27                          ((uint32_t) (0x00000004))
#define S_INTR_2_SEL_PAD33                          ((uint32_t) (0x00000005))

#define S_INTR_3_SEL_0                              ((uint32_t) (0x00000000))
#define S_INTR_3_SEL_PAD6                           ((uint32_t) (0x00000001))
#define S_INTR_3_SEL_PAD13                          ((uint32_t) (0x00000002))
#define S_INTR_3_SEL_PAD21                          ((uint32_t) (0x00000003))
#define S_INTR_3_SEL_PAD28                          ((uint32_t) (0x00000004))
#define S_INTR_3_SEL_PAD34                          ((uint32_t) (0x00000005))

#define S_INTR_4_SEL_0                              ((uint32_t) (0x00000000))
#define S_INTR_4_SEL_PAD7                           ((uint32_t) (0x00000001))
#define S_INTR_4_SEL_PAD14                          ((uint32_t) (0x00000002))
#define S_INTR_4_SEL_PAD22                          ((uint32_t) (0x00000003))
#define S_INTR_4_SEL_PAD35                          ((uint32_t) (0x00000004))

#define S_INTR_5_SEL_0                              ((uint32_t) (0x00000000))
#define S_INTR_5_SEL_PAD8                           ((uint32_t) (0x00000001))
#define S_INTR_5_SEL_PAD23                          ((uint32_t) (0x00000002))
#define S_INTR_5_SEL_PAD29                          ((uint32_t) (0x00000003))

#define S_INTR_6_SEL_0                              ((uint32_t) (0x00000000))
#define S_INTR_6_SEL_PAD9                           ((uint32_t) (0x00000001))
#define S_INTR_6_SEL_PAD24                          ((uint32_t) (0x00000002))
#define S_INTR_6_SEL_PAD30                          ((uint32_t) (0x00000003))

#define S_INTR_7_SEL_0                              ((uint32_t) (0x00000000))
#define S_INTR_7_SEL_PAD10                          ((uint32_t) (0x00000001))
#define S_INTR_7_SEL_PAD25                          ((uint32_t) (0x00000002))
#define S_INTR_7_SEL_PAD31                          ((uint32_t) (0x00000003))

#define NUARTCTS_SEL_0                              ((uint32_t) (0x00000000))
#define NUARTCTS_SEL_PAD21                          ((uint32_t) (0x00000001))

#define IO_REG_SEL_PAD7                             ((uint32_t) (0x00000000))
#define IO_REG_SEL_PAD11                            ((uint32_t) (0x00000001))
#define IO_REG_SEL_PAD10                            ((uint32_t) (0x00000000))
#define IO_REG_SEL_PAD12                            ((uint32_t) (0x00000002))
#define IO_REG_SEL_PAD15                            ((uint32_t) (0x00000000))
#define IO_REG_SEL_PAD13                            ((uint32_t) (0x00000004))
#define IO_REG_SEL_PAD16                            ((uint32_t) (0x00000000))
#define IO_REG_SEL_PAD14                            ((uint32_t) (0x00000008))
#define IO_REG_SEL_PAD22                            ((uint32_t) (0x00000000))
#define IO_REG_SEL_PAD29                            ((uint32_t) (0x00000010))
#define IO_REG_SEL_PAD23                            ((uint32_t) (0x00000000))
#define IO_REG_SEL_PAD30                            ((uint32_t) (0x00000020))
#define IO_REG_SEL_PAD25                            ((uint32_t) (0x00000000))
#define IO_REG_SEL_PAD31                            ((uint32_t) (0x00000040))
#define IO_REG_SEL_PAD26                            ((uint32_t) (0x00000000))
#define IO_REG_SEL_PAD32                            ((uint32_t) (0x00000080))

#define SW_CLK_SEL_PAD8                             ((uint32_t) (0x00000000))
#define SW_CLK_SEL_0                                ((uint32_t) (0x00000001))

#define SW_IO_SEL_PAD8                              ((uint32_t) (0x00000000))
#define SW_IO_SEL_0                                 ((uint32_t) (0x00000001))

#define FBIO_SEL_0                                  ((uint32_t) (0x00000000))
#define FBIO_SEL_PAD                                ((uint32_t) (0x00000001))

#define FBIO_SEL_25_PAD17                           ((uint32_t) (0x00000000))
#define FBIO_SEL_25_PAD34                           ((uint32_t) (0x00000001))
#define FBIO_SEL_27_PAD18                           ((uint32_t) (0x00000000))
#define FBIO_SEL_27_PAD26                           ((uint32_t) (0x00000002))
#define FBIO_SEL_29_PAD19                           ((uint32_t) (0x00000000))
#define FBIO_SEL_29_PAD27                           ((uint32_t) (0x00000004))
#define FBIO_SEL_31_PAD20                           ((uint32_t) (0x00000000))
#define FBIO_SEL_31_PAD35                           ((uint32_t) (0x00000008))

/******************************************************************************
 *                               INTR_CTRL                                    *
 ******************************************************************************/
#define GPIO_7_INTR                                 ((uint32_t) (0x00000080))
#define GPIO_6_INTR                                 ((uint32_t) (0x00000040))
#define GPIO_5_INTR                                 ((uint32_t) (0x00000020))
#define GPIO_4_INTR                                 ((uint32_t) (0x00000010))
#define GPIO_3_INTR                                 ((uint32_t) (0x00000008))
#define GPIO_2_INTR                                 ((uint32_t) (0x00000004))
#define GPIO_1_INTR                                 ((uint32_t) (0x00000002))
#define GPIO_0_INTR                                 ((uint32_t) (0x00000001))

#define GPIO_7_INTR_RAW                             ((uint32_t) (0x00000080))
#define GPIO_6_INTR_RAW                             ((uint32_t) (0x00000040))
#define GPIO_5_INTR_RAW                             ((uint32_t) (0x00000020))
#define GPIO_4_INTR_RAW                             ((uint32_t) (0x00000010))
#define GPIO_3_INTR_RAW                             ((uint32_t) (0x00000008))
#define GPIO_2_INTR_RAW                             ((uint32_t) (0x00000004))
#define GPIO_1_INTR_RAW                             ((uint32_t) (0x00000002))
#define GPIO_0_INTR_RAW                             ((uint32_t) (0x00000001))

#define GPIO_7_INTR_TYPE_EDGE                       ((uint32_t) (0x00000080))
#define GPIO_7_INTR_TYPE_LEVEL                      ((uint32_t) (0x00000000))
#define GPIO_6_INTR_TYPE_EDGE                       ((uint32_t) (0x00000040))
#define GPIO_6_INTR_TYPE_LEVEL                      ((uint32_t) (0x00000000))
#define GPIO_5_INTR_TYPE_EDGE                       ((uint32_t) (0x00000020))
#define GPIO_5_INTR_TYPE_LEVEL                      ((uint32_t) (0x00000000))
#define GPIO_4_INTR_TYPE_EDGE                       ((uint32_t) (0x00000010))
#define GPIO_4_INTR_TYPE_LEVEL                      ((uint32_t) (0x00000000))
#define GPIO_3_INTR_TYPE_EDGE                       ((uint32_t) (0x00000008))
#define GPIO_3_INTR_TYPE_LEVEL                      ((uint32_t) (0x00000000))
#define GPIO_2_INTR_TYPE_EDGE                       ((uint32_t) (0x00000004))
#define GPIO_2_INTR_TYPE_LEVEL                      ((uint32_t) (0x00000000))
#define GPIO_1_INTR_TYPE_EDGE                       ((uint32_t) (0x00000002))
#define GPIO_1_INTR_TYPE_LEVEL                      ((uint32_t) (0x00000000))
#define GPIO_0_INTR_TYPE_EDGE                       ((uint32_t) (0x00000001))
#define GPIO_0_INTR_TYPE_LEVEL                      ((uint32_t) (0x00000000))

#define GPIO_7_INTR_POL_HI_RISE                     ((uint32_t) (0x00000080))
#define GPIO_7_INTR_POL_LO_FALL                     ((uint32_t) (0x00000000))
#define GPIO_6_INTR_POL_HI_RISE                     ((uint32_t) (0x00000040))
#define GPIO_6_INTR_POL_LO_FALL                     ((uint32_t) (0x00000000))
#define GPIO_5_INTR_POL_HI_RISE                     ((uint32_t) (0x00000020))
#define GPIO_5_INTR_POL_LO_FALL                     ((uint32_t) (0x00000000))
#define GPIO_4_INTR_POL_HI_RISE                     ((uint32_t) (0x00000010))
#define GPIO_4_INTR_POL_LO_FALL                     ((uint32_t) (0x00000000))
#define GPIO_3_INTR_POL_HI_RISE                     ((uint32_t) (0x00000008))
#define GPIO_3_INTR_POL_LO_FALL                     ((uint32_t) (0x00000000))
#define GPIO_2_INTR_POL_HI_RISE                     ((uint32_t) (0x00000004))
#define GPIO_2_INTR_POL_LO_FALL                     ((uint32_t) (0x00000000))
#define GPIO_1_INTR_POL_HI_RISE                     ((uint32_t) (0x00000002))
#define GPIO_1_INTR_POL_LO_FALL                     ((uint32_t) (0x00000000))
#define GPIO_0_INTR_POL_HI_RISE                     ((uint32_t) (0x00000001))
#define GPIO_0_INTR_POL_LO_FALL                     ((uint32_t) (0x00000000))

#define GPIO_7_INTR_EN_AP_ENABLE                    ((uint32_t) (0x00000080))
#define GPIO_6_INTR_EN_AP_ENABLE                    ((uint32_t) (0x00000040))
#define GPIO_5_INTR_EN_AP_ENABLE                    ((uint32_t) (0x00000020))
#define GPIO_4_INTR_EN_AP_ENABLE                    ((uint32_t) (0x00000010))
#define GPIO_3_INTR_EN_AP_ENABLE                    ((uint32_t) (0x00000008))
#define GPIO_2_INTR_EN_AP_ENABLE                    ((uint32_t) (0x00000004))
#define GPIO_1_INTR_EN_AP_ENABLE                    ((uint32_t) (0x00000002))
#define GPIO_0_INTR_EN_AP_ENABLE                    ((uint32_t) (0x00000001))

#define GPIO_7_INTR_EN_M4_ENABLE                    ((uint32_t) (0x00000080))
#define GPIO_6_INTR_EN_M4_ENABLE                    ((uint32_t) (0x00000040))
#define GPIO_5_INTR_EN_M4_ENABLE                    ((uint32_t) (0x00000020))
#define GPIO_4_INTR_EN_M4_ENABLE                    ((uint32_t) (0x00000010))
#define GPIO_3_INTR_EN_M4_ENABLE                    ((uint32_t) (0x00000008))
#define GPIO_2_INTR_EN_M4_ENABLE                    ((uint32_t) (0x00000004))
#define GPIO_1_INTR_EN_M4_ENABLE                    ((uint32_t) (0x00000002))
#define GPIO_0_INTR_EN_M4_ENABLE                    ((uint32_t) (0x00000001))

#define GPIO_7_INTR_EN_FFE0_ENABLE                  ((uint32_t) (0x00000080))
#define GPIO_6_INTR_EN_FFE0_ENABLE                  ((uint32_t) (0x00000040))
#define GPIO_5_INTR_EN_FFE0_ENABLE                  ((uint32_t) (0x00000020))
#define GPIO_4_INTR_EN_FFE0_ENABLE                  ((uint32_t) (0x00000010))
#define GPIO_3_INTR_EN_FFE0_ENABLE                  ((uint32_t) (0x00000008))
#define GPIO_2_INTR_EN_FFE0_ENABLE                  ((uint32_t) (0x00000004))
#define GPIO_1_INTR_EN_FFE0_ENABLE                  ((uint32_t) (0x00000002))
#define GPIO_0_INTR_EN_FFE0_ENABLE                  ((uint32_t) (0x00000001))

#define GPIO_7_INTR_EN_FFE1_ENABLE                  ((uint32_t) (0x00000080))
#define GPIO_6_INTR_EN_FFE1_ENABLE                  ((uint32_t) (0x00000040))
#define GPIO_5_INTR_EN_FFE1_ENABLE                  ((uint32_t) (0x00000020))
#define GPIO_4_INTR_EN_FFE1_ENABLE                  ((uint32_t) (0x00000010))
#define GPIO_3_INTR_EN_FFE1_ENABLE                  ((uint32_t) (0x00000008))
#define GPIO_2_INTR_EN_FFE1_ENABLE                  ((uint32_t) (0x00000004))
#define GPIO_1_INTR_EN_FFE1_ENABLE                  ((uint32_t) (0x00000002))
#define GPIO_0_INTR_EN_FFE1_ENABLE                  ((uint32_t) (0x00000001))

// naveen - s3b
#define DMIC_VOICE_DET								((uint32_t)1<<23)
#define LPSD_VOICE_DET								((uint32_t)1<<22)
#define APBOOT_EN_DETECT							((uint32_t) (0x00040000))
#define WDT_FFE_DETECT			   				    ((uint32_t) (0x00020000))
#define FFE0_INTR_OTHERS_DETECT			   		    ((uint32_t) (0x00010000))
#define RST_INTR_DETECT                             ((uint32_t) (0x00008000))
#define RTC_INTR_DETECT                             ((uint32_t) (0x00004000))
#define ADC_INTR_DETECT                             ((uint32_t) (0x00002000))
#define PMU_TMR_INTR_DETECT                         ((uint32_t) (0x00001000))
#define CFG_DMA_DONE_DETECT                         ((uint32_t) (0x00000800))
#define SPI_MS_INTR_DETECT                          ((uint32_t) (0x00000400))
#define AUD_INTR_DETECT                             ((uint32_t) (0x00000200))
#define I2S_INTR_DETECT                             ((uint32_t) (0x00000100))

#define PKFB_INTR_DETECT                            ((uint32_t) (0x00000080))
#define FPU_INTR_DETECT                             ((uint32_t) (0x00000040))
#define TIMEOUT_INTR_DETECT                         ((uint32_t) (0x00000020))
#define WDOG_RST_DETECT                             ((uint32_t) (0x00000010))
#define WDOG_INTR_DETECT                            ((uint32_t) (0x00000008))
#define TIMER_INTR_DETECT                           ((uint32_t) (0x00000004))
#define UART_INTR_DETECT                            ((uint32_t) (0x00000002))
#define M4_SRAM_INTR_DETECT                         ((uint32_t) (0x00000001))

#define DMIC_VOICE_INTR                             ((uint32_t) (0x00800000))
#define LPSD_VOICE_INTR                             ((uint32_t) (0x00400000))
#define SRAM_128KB_INTR                             ((uint32_t) (0x00200000))
#define LDO50_PG_INTR                               ((uint32_t) (0x00100000))
#define LDO30_PG_INTR                               ((uint32_t) (0x00080000))

#define APBOOT_EN_DETECT	((uint32_t) (0x00040000))
#define WDT_FFE_DETECT		((uint32_t) (0x00020000))
#define FFE0_INTR_OTHERS_DETECT	((uint32_t) (0x00010000))

#define DMIC_VOICE_EN_AP        ((uint32_t) (0x00800000))
#define LPSD_VOICE_EN_AP        ((uint32_t) (0x00400000))
#define SRAM_128KB_EN_AP        ((uint32_t) (0x00200000))
#define LDO50_PG_EN_AP          ((uint32_t) (0x00100000))
#define LDO30_PG_EN_AP          ((uint32_t) (0x00080000))
#define APBOOT_EN_AP   		((uint32_t) (0x00040000))
#define WDT_FFE_EN_AP 		((uint32_t) (0x00020000))
#define FFE0_INTR_OTHERS_EN_AP 	((uint32_t) (0x00010000))
#define RST_INTR_EN_AP          ((uint32_t) (0x00008000))
#define RTC_INTR_EN_AP          ((uint32_t) (0x00004000))
#define ADC_INTR_EN_AP          ((uint32_t) (0x00002000))
#define PMU_TMR_INTR_EN_AP      ((uint32_t) (0x00001000))
#define CFG_DMA_DONE_EN_AP      ((uint32_t) (0x00000800))
#define SPI_MS_INTR_EN_AP       ((uint32_t) (0x00000400))
#define PKFB_INTR_EN_AP         ((uint32_t) (0x00000080))
#define FPU_INTR_EN_AP          ((uint32_t) (0x00000040))
#define TIMEOUT_INTR_EN_AP      ((uint32_t) (0x00000020))
#define WDOG_RST_EN_AP          ((uint32_t) (0x00000010))
#define WDOG_INTR_EN_AP         ((uint32_t) (0x00000008))
#define TIMER_INTR_EN_AP        ((uint32_t) (0x00000004))
#define UART_INTR_EN_AP         ((uint32_t) (0x00000002))
#define M4_SRAM_INTR_EN_AP      ((uint32_t) (0x00000001))

#define DMIC_VOICE_EN_M4        ((uint32_t) (0x00800000))
#define LPSD_VOICE_EN_M4        ((uint32_t) (0x00400000))
#define SRAM_128KB_EN_M4        ((uint32_t) (0x00200000))
#define LDO50_PG_EN_M4          ((uint32_t) (0x00100000))
#define LDO30_PG_EN_M4          ((uint32_t) (0x00080000))
#define APBOOT_EN_M4   		((uint32_t) (0x00040000))
#define WDT_FFE_EN_M4 		((uint32_t) (0x00020000))
#define FFE0_INTR_OTHERS_EN_M4 	((uint32_t) (0x00010000))
#define RST_INTR_EN_M4          ((uint32_t) (0x00008000))
#define RTC_INTR_EN_M4          ((uint32_t) (0x00004000))
#define ADC_INTR_EN_M4          ((uint32_t) (0x00002000))
#define PMU_TMR_INTR_EN_M4      ((uint32_t) (0x00001000))
#define CFG_DMA_DONE_EN_M4      ((uint32_t) (0x00000800))
#define SPI_MS_INTR_EN_M4       ((uint32_t) (0x00000400))
#define PKFB_INTR_EN_M4         ((uint32_t) (0x00000080))
#define FPU_INTR_EN_M4          ((uint32_t) (0x00000040))
#define TIMEOUT_INTR_EN_M4      ((uint32_t) (0x00000020))
#define WDOG_RST_EN_M4          ((uint32_t) (0x00000010))
#define WDOG_INTR_EN_M4         ((uint32_t) (0x00000008))
#define TIMER_INTR_EN_M4        ((uint32_t) (0x00000004))
#define UART_INTR_EN_M4         ((uint32_t) (0x00000002))
#define M4_SRAM_INTR_EN_M4      ((uint32_t) (0x00000001))

#define SW_INTR_1               ((uint32_t) (0x00000001))
#define SW_INTR_1_EN_AP         ((uint32_t) (0x00000001))
#define SW_INTR_1_EN_M4         ((uint32_t) (0x00000001))
#define SW_INTR_2               ((uint32_t) (0x00000001))
#define SW_INTR_2_EN_AP         ((uint32_t) (0x00000001))
#define SW_INTR_2_EN_M4         ((uint32_t) (0x00000001))

#define FFE0_7_INTR_DETECT	((uint32_t)0x00000080)
#define FFE0_6_INTR_DETECT	((uint32_t) (0x00000040))
#define FFE0_5_INTR_DETECT	((uint32_t) (0x00000020))
#define FFE0_4_INTR_DETECT	((uint32_t) (0x00000010))
#define FFE0_3_INTR_DETECT	((uint32_t) (0x00000008))
#define FFE0_2_INTR_DETECT	((uint32_t) (0x00000004))
#define FFE0_1_INTR_DETECT	((uint32_t) (0x00000002))
#define FFE0_0_INTR_DETECT	((uint32_t) (0x00000001))

#define FFE0_7_INTR_EN_AP 	((uint32_t) (0x00000080))
#define FFE0_6_INTR_EN_AP 	((uint32_t) (0x00000040))
#define FFE0_5_INTR_EN_AP 	((uint32_t) (0x00000020))
#define FFE0_4_INTR_EN_AP 	((uint32_t) (0x00000010))
#define FFE0_3_INTR_EN_AP 	((uint32_t) (0x00000008))
#define FFE0_2_INTR_EN_AP 	((uint32_t) (0x00000004))
#define FFE0_1_INTR_EN_AP 	((uint32_t) (0x00000002))
#define FFE0_0_INTR_EN_AP 	((uint32_t) (0x00000001))

#define FFE0_7_INTR_EN_M4 	((uint32_t) (0x00000080))
#define FFE0_6_INTR_EN_M4 	((uint32_t) (0x00000040))
#define FFE0_5_INTR_EN_M4 	((uint32_t) (0x00000020))
#define FFE0_4_INTR_EN_M4  	((uint32_t) (0x00000010))
#define FFE0_3_INTR_EN_M4  	((uint32_t) (0x00000008))
#define FFE0_2_INTR_EN_M4  	((uint32_t) (0x00000004))
#define FFE0_1_INTR_EN_M4  	((uint32_t) (0x00000002))
#define FFE0_0_INTR_EN_M4  	((uint32_t) (0x00000001))

#define FFE1_7_INTR_DETECT	((uint32_t) (0x00000080))
#define FFE1_6_INTR_DETECT	((uint32_t) (0x00000040))
#define FFE1_5_INTR_DETECT	((uint32_t) (0x00000020))
#define FFE1_4_INTR_DETECT	((uint32_t) (0x00000010))
#define FFE1_3_INTR_DETECT	((uint32_t) (0x00000008))
#define FFE1_2_INTR_DETECT	((uint32_t) (0x00000004))
#define FFE1_1_INTR_DETECT	((uint32_t) (0x00000002))
#define FFE1_0_INTR_DETECT	((uint32_t) (0x00000001))

#define FFE1_7_INTR_EN_AP 	((uint32_t) (0x00000080))
#define FFE1_6_INTR_EN_AP 	((uint32_t) (0x00000040))
#define FFE1_5_INTR_EN_AP 	((uint32_t) (0x00000020))
#define FFE1_4_INTR_EN_AP 	((uint32_t) (0x00000010))
#define FFE1_3_INTR_EN_AP 	((uint32_t) (0x00000008))
#define FFE1_2_INTR_EN_AP 	((uint32_t) (0x00000004))
#define FFE1_1_INTR_EN_AP 	((uint32_t) (0x00000002))
#define FFE1_0_INTR_EN_AP 	((uint32_t) (0x00000001))

#define FFE1_7_INTR_EN_M4 	((uint32_t) (0x00000080))
#define FFE1_6_INTR_EN_M4 	((uint32_t) (0x00000040))
#define FFE1_5_INTR_EN_M4 	((uint32_t) (0x00000020))
#define FFE1_4_INTR_EN_M4  	((uint32_t) (0x00000010))
#define FFE1_3_INTR_EN_M4  	((uint32_t) (0x00000008))
#define FFE1_2_INTR_EN_M4  	((uint32_t) (0x00000004))
#define FFE1_1_INTR_EN_M4  	((uint32_t) (0x00000002))
#define FFE1_0_INTR_EN_M4  	((uint32_t) (0x00000001))

#define FB_3_INTR_DETECT	((uint32_t) (0x00000008))
#define FB_2_INTR_DETECT	((uint32_t) (0x00000004))
#define FB_1_INTR_DETECT	((uint32_t) (0x00000002))
#define FB_0_INTR_DETECT	((uint32_t) (0x00000001))

#define FB_3_INTR_RAW   	((uint32_t) (0x00000008))
#define FB_2_INTR_RAW   	((uint32_t) (0x00000004))
#define FB_1_INTR_RAW   	((uint32_t) (0x00000002))
#define FB_0_INTR_RAW   	((uint32_t) (0x00000001))

#define FB_3_INTR_TYPE_LEVEL 	((uint32_t) (0x00000000))
#define FB_3_INTR_TYPE_EDGE 	((uint32_t) (0x00000008))
#define FB_2_INTR_TYPE_LEVEL    ((uint32_t) (0x00000000))
#define FB_2_INTR_TYPE_EDGE     ((uint32_t) (0x00000004))
#define FB_1_INTR_TYPE_LEVEL	((uint32_t) (0x00000000))
#define FB_1_INTR_TYPE_EDGE	((uint32_t) (0x00000002))
#define FB_0_INTR_TYPE_LEVEL	((uint32_t) (0x00000000))
#define FB_0_INTR_TYPE_EDGE	((uint32_t) (0x00000001))

#define FB_3_INTR_POL_LO_FALL 	((uint32_t) (0x00000000))
#define FB_3_INTR_POL_HI_RISE 	((uint32_t) (0x00000008))
#define FB_2_INTR_POL_LO_FALL 	((uint32_t) (0x00000000))
#define FB_2_INTR_POL_HI_RISE 	((uint32_t) (0x00000004))
#define FB_1_INTR_POL_LO_FALL 	((uint32_t) (0x00000000))
#define FB_1_INTR_POL_HI_RISE 	((uint32_t) (0x00000002))
#define FB_0_INTR_POL_LO_FALL 	((uint32_t) (0x00000000))
#define FB_0_INTR_POL_HI_RISE 	((uint32_t) (0x00000001))

#define FB_3_INTR_EN_AP 	((uint32_t) (0x00000008))
#define FB_2_INTR_EN_AP 	((uint32_t) (0x00000004))
#define FB_1_INTR_EN_AP 	((uint32_t) (0x00000002))
#define FB_0_INTR_EN_AP 	((uint32_t) (0x00000001))

#define FB_3_INTR_EN_M4 	((uint32_t) (0x00000008))
#define FB_2_INTR_EN_M4 	((uint32_t) (0x00000004))
#define FB_1_INTR_EN_M4 	((uint32_t) (0x00000002))
#define FB_0_INTR_EN_M4 	((uint32_t) (0x00000001))

/******************************************************************************
 *                                 SPI_MS                                     *
 ******************************************************************************/

#define CTRLR0_DFS_32_4_BIT                         ((uint32_t) (0x00030000))
#define CTRLR0_DFS_32_5_BIT                         ((uint32_t) (0x00040000))
#define CTRLR0_DFS_32_6_BIT                         ((uint32_t) (0x00050000))
#define CTRLR0_DFS_32_7_BIT                         ((uint32_t) (0x00060000))
#define CTRLR0_DFS_32_8_BIT                         ((uint32_t) (0x00070000))
#define CTRLR0_DFS_32_9_BIT                         ((uint32_t) (0x00080000))
#define CTRLR0_DFS_32_10_BIT                        ((uint32_t) (0x00090000))
#define CTRLR0_DFS_32_11_BIT                        ((uint32_t) (0x000A0000))
#define CTRLR0_DFS_32_12_BIT                        ((uint32_t) (0x000B0000))
#define CTRLR0_DFS_32_13_BIT                        ((uint32_t) (0x000C0000))
#define CTRLR0_DFS_32_14_BIT                        ((uint32_t) (0x000D0000))
#define CTRLR0_DFS_32_15_BIT                        ((uint32_t) (0x000E0000))
#define CTRLR0_DFS_32_16_BIT                        ((uint32_t) (0x000F0000))
#define CTRLR0_CFS_1_BIT                            ((uint32_t) (0x00000000))
#define CTRLR0_CFS_2_BIT                            ((uint32_t) (0x00001000))
#define CTRLR0_CFS_3_BIT                            ((uint32_t) (0x00002000))
#define CTRLR0_CFS_4_BIT                            ((uint32_t) (0x00003000))
#define CTRLR0_CFS_5_BIT                            ((uint32_t) (0x00004000))
#define CTRLR0_CFS_6_BIT                            ((uint32_t) (0x00005000))
#define CTRLR0_CFS_7_BIT                            ((uint32_t) (0x00006000))
#define CTRLR0_CFS_8_BIT                            ((uint32_t) (0x00007000))
#define CTRLR0_CFS_9_BIT                            ((uint32_t) (0x00008000))
#define CTRLR0_CFS_10_BIT                           ((uint32_t) (0x00009000))
#define CTRLR0_CFS_11_BIT                           ((uint32_t) (0x0000A000))
#define CTRLR0_CFS_12_BIT                           ((uint32_t) (0x0000B000))
#define CTRLR0_CFS_13_BIT                           ((uint32_t) (0x0000C000))
#define CTRLR0_CFS_14_BIT                           ((uint32_t) (0x0000D000))
#define CTRLR0_CFS_15_BIT                           ((uint32_t) (0x0000E000))
#define CTRLR0_CFS_16_BIT                           ((uint32_t) (0x0000F000))
#define CTRLR0_SRL_TEST_MODE                        ((uint32_t) (0x00000800))
#define CTRLR0_SRL_NORMAL_MODE                      ((uint32_t) (0x00000000))
#define CTRLR0_SLV_OE                               ((uint32_t) (0x00000400))
#define CTRLR0_TMOD_TX_RX                           ((uint32_t) (0x00000000))
#define CTRLR0_TMOD_TX                              ((uint32_t) (0x00000100))
#define CTRLR0_TMOD_RX                              ((uint32_t) (0x00000200))
#define CTRLR0_TMOD_EEPROM                          ((uint32_t) (0x00000300))
#define CTRLR0_SCPOL_LO                             ((uint32_t) (0x00000000))
#define CTRLR0_SCPOL_HI                             ((uint32_t) (0x00000080))
#define CTRLR0_SCPH_MID                             ((uint32_t) (0x00000000))
#define CTRLR0_SCPH_START                           ((uint32_t) (0x00000040))
#define CTRLR0_DFS_4_BIT                            ((uint32_t) (0x00000003))
#define CTRLR0_DFS_5_BIT                            ((uint32_t) (0x00000004))
#define CTRLR0_DFS_6_BIT                            ((uint32_t) (0x00000005))
#define CTRLR0_DFS_7_BIT                            ((uint32_t) (0x00000006))
#define CTRLR0_DFS_8_BIT                            ((uint32_t) (0x00000007))
#define CTRLR0_DFS_9_BIT                            ((uint32_t) (0x00000008))
#define CTRLR0_DFS_10_BIT                           ((uint32_t) (0x00000009))
#define CTRLR0_DFS_11_BIT                           ((uint32_t) (0x0000000A))
#define CTRLR0_DFS_12_BIT                           ((uint32_t) (0x0000000B))
#define CTRLR0_DFS_13_BIT                           ((uint32_t) (0x0000000C))
#define CTRLR0_DFS_14_BIT                           ((uint32_t) (0x0000000D))
#define CTRLR0_DFS_15_BIT                           ((uint32_t) (0x0000000E))
#define CTRLR0_DFS_16_BIT                           ((uint32_t) (0x0000000F))

#define SSIENR_SSI_DISABLE                          ((uint32_t) (0x00000000))
#define SSIENR_SSI_EN                               ((uint32_t) (0x00000001))

#define SER_SS_0_N_SELECTED                         ((uint32_t) (0x00000001))
#define SER_SS_1_N_SELECTED                         ((uint32_t) (0x00000002))
#define SER_SS_2_N_SELECTED                         ((uint32_t) (0x00000004))

#define SR_DCOL_TX_DATA_ERR                         ((uint32_t) (0x00000040))
#define SR_RFF                                      ((uint32_t) (0x00000010))
#define SR_RFNE                                     ((uint32_t) (0x00000008))
#define SR_TFE                                      ((uint32_t) (0x00000004))
#define SR_TFNF                                     ((uint32_t) (0x00000002))
#define SR_BUSY                                     ((uint32_t) (0x00000001))

#define IMR_MSTIM_UNMASK                            ((uint32_t) (0x00000020))
#define IMR_RXFIM_UNMASK                            ((uint32_t) (0x00000010))
#define IMR_RXFOIM_UNMASK                           ((uint32_t) (0x00000008))
#define IMR_RXUIM_UNMASK                            ((uint32_t) (0x00000004))
#define IMR_TXOIM_UNMASK                            ((uint32_t) (0x00000002))
#define IMR_TXEIM_UNMASK                            ((uint32_t) (0x00000001))

#define ISR_MSTIM_ACTIVE                            ((uint32_t) (0x00000020))
#define ISR_RXFIM_ACTIVE                            ((uint32_t) (0x00000010))
#define ISR_RXFOIM_ACTIVE                           ((uint32_t) (0x00000008))
#define ISR_RXUIM_ACTIVE                            ((uint32_t) (0x00000004))
#define ISR_TXOIM_ACTIVE                            ((uint32_t) (0x00000002))
#define ISR_TXEIM_ACTIVE                            ((uint32_t) (0x00000001))

#define RISR_MSTIM_ACTIVE                           ((uint32_t) (0x00000020))
#define RISR_RXFIM_ACTIVE                           ((uint32_t) (0x00000010))
#define RISR_RXFOIM_ACTIVE                          ((uint32_t) (0x00000008))
#define RISR_RXUIM_ACTIVE                           ((uint32_t) (0x00000004))
#define RISR_TXOIM_ACTIVE                           ((uint32_t) (0x00000002))
#define RISR_TXEIM_ACTIVE                           ((uint32_t) (0x00000002))

/******************************************************************************
 *                                   I2S                                      *
 ******************************************************************************/
#define IER_IEN                                     ((uint32_t) (0x00000001))

#define IRER_RXEN                                   ((uint32_t) (0x00000001))

#define CER_CLKEN                                   ((uint32_t) (0x00000001))

#define CCR_WSS_16_CYCLES                           ((uint32_t) (0x00000000))
#define CCR_WSS_24_CYCLES                           ((uint32_t) (0x00000008))
#define CCR_WSS_32_CYCLES                           ((uint32_t) (0x00000010))
#define CCR_SCLKG_NO_GATING                         ((uint32_t) (0x00000000))
#define CCR_SCLKG_12_GATING                         ((uint32_t) (0x00000001))
#define CCR_SCLKG_16_GATING                         ((uint32_t) (0x00000002))
#define CCR_SCLKG_20_GATING                         ((uint32_t) (0x00000003))
#define CCR_SCLKG_24_GATING                         ((uint32_t) (0x00000004))

#define RXFFR_RESET                                 ((uint32_t) (0x00000001))

#define RER0_RX_CHEN0_ENABLE                        ((uint32_t) (0x00000001))
#define RER0_RX_CHEN0_DISABLE                       ((uint32_t) (0x00000000))

#define RCR0_WLEN_IGNORE                            ((uint32_t) (0x00000000))
#define RCR0_WLEN_12BIT                             ((uint32_t) (0x00000001))
#define RCR0_WLEN_16BIT                             ((uint32_t) (0x00000002))

#define ISR0_RXFO_VALID                             ((uint32_t) (0x00000002))
#define ISR0_RXDA_VALID                             ((uint32_t) (0x00000001))

#define IMR0_RXFOM                                  ((uint32_t) (0x00000002))
#define IMR0_RXDAM                                  ((uint32_t) (0x00000001))

#define RFCR0_RXCHDT_1BIT                           ((uint32_t) (0x00000000))
#define RFCR0_RXCHDT_2BIT                           ((uint32_t) (0x00000001))
#define RFCR0_RXCHDT_3BIT                           ((uint32_t) (0x00000002))
#define RFCR0_RXCHDT_4BIT                           ((uint32_t) (0x00000003))
#define RFCR0_RXCHDT_5BIT                           ((uint32_t) (0x00000004))
#define RFCR0_RXCHDT_6BIT                           ((uint32_t) (0x00000005))
#define RFCR0_RXCHDT_7BIT                           ((uint32_t) (0x00000006))
#define RFCR0_RXCHDT_8BIT                           ((uint32_t) (0x00000007))

#define RFF0_RXCHFR_RESET                           ((uint32_t) (0x00000001))

#define AUD_P_CTRL_DMA_MUX_SEL                      ((uint32_t) (0x00000001))
#define AUD_P_CTRL_MONO_SEL_MONO                    ((uint32_t) (0x00000002))
#define AUD_P_CTRL_MONO_SEL_STEREO                  ((uint32_t) (0x00000000))
#define AUD_P_CTRL_RIGHT_SEL_RIGHT                  ((uint32_t) (0x00000004))
#define AUD_P_CTRL_RIGHT_SEL_LEFT                   ((uint32_t) (0x00000000))
#define AUD_P_CTRL_I2S_SEL_I2S                      ((uint32_t) (0x00000008))
#define AUD_P_CTRL_I2S_SEL_PDM                      ((uint32_t) (0x00000000))

#define AUD_P_INTR_FIFO_OVERFLOW                    ((uint32_t) (0x00000001))
#define AUD_P_INTR_RX_OVERRUN                       ((uint32_t) (0x00000002))
#define AUD_P_INTR_RX_DATA_AVALIABLE                ((uint32_t) (0x00000004))

#define AUD_P_INTR_MASK_FIFO_OVERFLOW_MASK          ((uint32_t) (0x00000001))
#define AUD_P_INTR_MASK_RX_OVERRUN_MASK             ((uint32_t) (0x00000002))
#define AUD_P_INTR_MASK_RX_DATA_AVALIABLE_MASK      ((uint32_t) (0x00000004))

/******************************************************************************
 *                                   AUD                                      *
 ******************************************************************************/
#define AUD_H_CTRL_DMA_START                        ((uint32_t) (0x00000001))
#define AUD_H_CTRL_DMA_STOP                         ((uint32_t) (0x00000002))
#define AUD_H_CTRL_LS_ENABLE                        ((uint32_t) (0x00000004))
#define AUD_H_CTRL_POWER_REQ_DISABLE                ((uint32_t) (0x00000008))
#define AUD_H_CTRL_AHB_ADDR_CYC                     ((uint32_t) (0x00000010))
#define AUD_H_CTRL_AHB_ADDR_XFR                     ((uint32_t) (0x00000020))
#define AUD_H_CTRL_AHB_DATA_CYC                     ((uint32_t) (0x00000040))
#define AUD_H_CTRL_AHB_DATA_XFR                     ((uint32_t) (0x00000080))

#define IDLE_CYC_PERIOD_MASK                        ((uint32_t) (0x0000001F))
#define AHB_IDLE_PERIOD_MASK                        ((uint32_t) (0x0000001F))
#define DMA_BLOCK_SIZE_MASK                         ((uint32_t) (0x0003FFFC))
#define DMA_WRAP_SIZE_MASK                          ((uint32_t) (0x0003FFFC))
#define DMA_XFER_CNT_MASK                           ((uint32_t) (0x0003FFFC))
#define DMA_START_ADDR_MASK                         ((uint32_t) (0xFFFFFFFC))
#define DMA_DEST_ADDR_MASK                          ((uint32_t) (0xFFFFFFFC))
#define FIFO_SYNC_THRESHOLD_LVL_MASK                ((uint32_t) (0x00003FFC))

#define AHB_FIFO_SYNC_TEST_RM_MASK                  ((uint32_t) (0x00000003))
#define AHB_FIFO_SYNC_TEST_RM_RSV_MASK              ((uint32_t) (0x0000000C))
#define AHB_FIFO_SYNC_TEST_RME_MASK                 ((uint32_t) (0x00000010))
#define AHB_FIFO_SYNC_TEST_1_MASK                   ((uint32_t) (0x00000020))

#define AHB_H_INTR_AUD_POWER_ON_REQ                 ((uint32_t) (0x00000001))
#define AHB_H_INTR_AUD_POWER_OFF_REQ                ((uint32_t) (0x00000002))
#define AHB_H_INTR_AUD_BLOCK                        ((uint32_t) (0x00000004))
#define AHB_H_INTR_AUD_WRAP                         ((uint32_t) (0x00000008))
#define AHB_H_INTR_FIFO_SYNC_UNDERFLOW              ((uint32_t) (0x00000010))
#define AHB_H_INTR_FIFO_SYNC_OVERFLOW               ((uint32_t) (0x00000020))
#define AHB_H_INTR_FIFO_ASYNC_UNDERFLOW             ((uint32_t) (0x00000040))
#define AHB_H_INTR_DEEP_SLEEP_ERROR                 ((uint32_t) (0x00000080))
#define AHB_H_INTR_HERROR                           ((uint32_t) (0x00000100))

#define AHB_H_INTR_MASK_AUD_POWER_ON_REQ            ((uint32_t) (0x00000001))
#define AHB_H_INTR_MASK_AUD_POWER_OFF_REQ           ((uint32_t) (0x00000002))
#define AHB_H_INTR_MASK_AUD_BLOCK                   ((uint32_t) (0x00000004))
#define AHB_H_INTR_MASK_AUD_WRAP                    ((uint32_t) (0x00000008))
#define AHB_H_INTR_MASK_FIFO_SYNC_UNDERFLOW         ((uint32_t) (0x00000010))
#define AHB_H_INTR_MASK_FIFO_SYNC_OVERFLOW          ((uint32_t) (0x00000020))
#define AHB_H_INTR_MASK_FIFO_ASYNC_UNDERFLOW        ((uint32_t) (0x00000040))
#define AHB_H_INTR_MASK_DEEP_SLEEP_ERROR            ((uint32_t) (0x00000080))
#define AHB_H_INTR_MASK_HERROR                      ((uint32_t) (0x00000100))

#define DMA_WRAP_SIZE_MASK_MASK                     ((uint32_t) (0x0003FFFC))
#define DS_WAKEUP_DLY_MASK                          ((uint32_t) (0x0000000F))


/******************************************************************************
 *                                   PMU                                      *
 ******************************************************************************/
#define PMU_WIC_CONTROL_ENABLE                      ((uint32_t) (0x00000001))

#define PMU_MISC_SW_WU_A1_WU                        ((uint32_t) (0x00000040))
#define PMU_MISC_SW_WU_I2S_WU                       ((uint32_t) (0x00000020))
#define PMU_MISC_SW_WU_RES_WU                       ((uint32_t) (0x00000010))
#define PMU_MISC_SW_WU_DBG_WU                       ((uint32_t) (0x00000008))
#define PMU_MISC_SW_WU_EFUSE_WU                     ((uint32_t) (0x00000004))
#define PMU_MISC_SW_WU_TM_WU                        ((uint32_t) (0x00000002))
#define PMU_MISC_SW_WU_SDMA_WU                      ((uint32_t) (0x00000001))

#define PMU_FFE_PD_SRC_MASK_N						((uint32_t) (0x00000001))
#define PMU_FFE_WU_SRC_MASK_N						((uint32_t) (0x00000001))

#define PMU_FB_PD_SRC_MASK_N						((uint32_t) (0x00000001))
#define PMU_FB_WU_SRC_SEN_GPIO0_INT					((uint32_t) (0x00000001))
#define PMU_FB_WU_SRC_SEN_GPIO1_INT					((uint32_t) (0x00000002))
#define PMU_FB_WU_SRC_SEN_GPIO2_INT					((uint32_t) (0x00000004))
#define PMU_FB_WU_SRC_SEN_GPIO3_INT					((uint32_t) (0x00000008))
#define PMU_FB_WU_SRC_SEN_GPIO4_INT					((uint32_t) (0x00000010))
#define PMU_FB_WU_SRC_SEN_GPIO5_INT					((uint32_t) (0x00000020))
#define PMU_FB_WU_SRC_SEN_GPIO6_INT					((uint32_t) (0x00000040))
#define PMU_FB_WU_SRC_SEN_GPIO7_INT					((uint32_t) (0x00000080))
#define PMU_FB_WU_SRC_FFE_TIMER						((uint32_t) (0x00000100))

#define PMU_FFE_FB_PF_SW_WU_PF_WU                   ((uint32_t) (0x00000004))
#define PMU_FFE_FB_PF_SW_WU_FB_WU                   ((uint32_t) (0x00000002))
#define PMU_FFE_FB_PF_SW_WU_FFE_WU                  ((uint32_t) (0x00000001))

#define PMU_M4S0_EVENT_PD                  	    	((uint32_t) (0x00000001))
#define PMU_M4S0_EVENT_WU                  	    	((uint32_t) (0x00000001))

#define PMU_M4S0_SW_PD                  			((uint32_t) (0x00000001))
#define PMU_M4S1_SW_PD                  			((uint32_t) (0x00000002))
#define PMU_M4S2_SW_PD                  		((uint32_t) (0x00000004))
#define PMU_M4S3_SW_PD                  		((uint32_t) (0x00000008))
#define PMU_M4S4_SW_PD                  		((uint32_t) (0x00000010))
#define PMU_M4S5_SW_PD                  		((uint32_t) (0x00000020))
#define PMU_M4S6_SW_PD                  		((uint32_t) (0x00000040))
#define PMU_M4S7_SW_PD                  		((uint32_t) (0x00000080))
#define PMU_M4S8_SW_PD                  		((uint32_t) (0x00000100))
#define PMU_M4S9_SW_PD                  		((uint32_t) (0x00000200))
#define PMU_M4S10_SW_PD                  		((uint32_t) (0x00000400))
#define PMU_M4S11_SW_PD                  		((uint32_t) (0x00000800))
#define PMU_M4S12_SW_PD                  		((uint32_t) (0x00001000))
#define PMU_M4S13_SW_PD                  		((uint32_t) (0x00002000))
#define PMU_M4S14_SW_PD                  		((uint32_t) (0x00004000))
#define PMU_M4S15_SW_PD                  		((uint32_t) (0x00008000))

#define PMU_M4S0_SW_WU                  		((uint32_t) (0x00000001))
#define PMU_M4S1_SW_WU                  		((uint32_t) (0x00000002))
#define PMU_M4S2_SW_WU                  		((uint32_t) (0x00000004))
#define PMU_M4S3_SW_WU                  		((uint32_t) (0x00000008))
#define PMU_M4S4_SW_WU                  		((uint32_t) (0x00000010))
#define PMU_M4S5_SW_WU                  		((uint32_t) (0x00000020))
#define PMU_M4S6_SW_WU                  		((uint32_t) (0x00000040))
#define PMU_M4S7_SW_WU                  		((uint32_t) (0x00000080))
#define PMU_M4S8_SW_WU                  		((uint32_t) (0x00000100))
#define PMU_M4S9_SW_WU                  		((uint32_t) (0x00000200))
#define PMU_M4S10_SW_WU                  		((uint32_t) (0x00000400))
#define PMU_M4S11_SW_WU                  		((uint32_t) (0x00000800))
#define PMU_M4S12_SW_WU                  		((uint32_t) (0x00001000))
#define PMU_M4S13_SW_WU                  		((uint32_t) (0x00002000))
#define PMU_M4S14_SW_WU                  		((uint32_t) (0x00004000))
#define PMU_M4S15_SW_WU                  		((uint32_t) (0x00008000))

#define PMU_AUDIO0_EVENT_WU                 		((uint32_t) (0x00000001))
#define PMU_AUDIO1_EVENT_WU                 		((uint32_t) (0x00000002))
#define PMU_AUDIO2_EVENT_WU                 		((uint32_t) (0x00000004))
#define PMU_AUDIO3_EVENT_WU                 		((uint32_t) (0x00000008))
#define PMU_AUDIO4_EVENT_WU                 		((uint32_t) (0x00000010))
#define PMU_AUDIO5_EVENT_WU                 		((uint32_t) (0x00000020))


#define PMU_M4_STATUS_WU                			((uint32_t) (0x00000001))
#define PMU_M4_STATUS_PD               				((uint32_t) (0x00000004))

#define PMU_M4_SHUTDOWN_MODE           				((uint32_t) (0x00000002))
#define PMU_M4S0_POWER_MODE            				((uint32_t) (0x00000002))

#define PMU_M4SRAM_LPMF								((uint32_t) (0x00000002))
#define PMU_M4SRAM_LPMH             				((uint32_t) (0x00000002))

#define PWR_DWN_SCH_M4S0_PD              			((uint32_t) (0x00000080))
#define PWR_DWN_SCH_AUDIO_PD              			((uint32_t) (0x00000040))
#define PWR_DWN_SCH_SRAM_PD              			((uint32_t) (0x00000020))
#define PWR_DWN_SCH_FFEFB_PD              			((uint32_t) (0x00000010))
#define PWR_DWN_SCH_M4M4S0_WU              			((uint32_t) (0x00000008))
#define PWR_DWN_SCH_AUDIO_WU              			((uint32_t) (0x00000004))
#define PWR_DWN_SCH_SRAM_WU              			((uint32_t) (0x00000002))
#define PWR_DWN_SCH_FFEFB_WU              			((uint32_t) (0x00000001))

#define PWR_SDMA_SRAM_PD_CFG              			((uint32_t) (0x00040000))

#define PWR_AUDIO_SRAM_CFG              			((uint32_t) (0x00000100))

#define PWR_AUDIO_SRAM_LC_DS_R2              		((uint32_t) (0x00000004))
#define PWR_AUDIO_SRAM_LC_DS_R1              		((uint32_t) (0x00000002))
#define PWR_AUDIO_SRAM_LC_DS_R0              		((uint32_t) (0x00000001))

#define M4_SRAM0_PD_WU				(uint32_t)(0x1)
#define M4_SRAM1_PD_WU				(uint32_t)(0x2)
#define M4_SRAM2_PD_WU				(uint32_t)(0x4)
#define M4_SRAM3_PD_WU				(uint32_t)(0x8)
#define M4_SRAM4_PD_WU				(uint32_t)(0x10)
#define M4_SRAM5_PD_WU				(uint32_t)(0x20)
#define M4_SRAM6_PD_WU				(uint32_t)(0x40)
#define M4_SRAM7_PD_WU				(uint32_t)(0x80)
#define M4_SRAM8_PD_WU				(uint32_t)(0x100)
#define M4_SRAM9_PD_WU				(uint32_t)(0x200)
#define M4_SRAM10_PD_WU				(uint32_t)(0x400)
#define M4_SRAM11_PD_WU				(uint32_t)(0x800)
#define M4_SRAM12_PD_WU				(uint32_t)(0x1000)
#define M4_SRAM13_PD_WU				(uint32_t)(0x2000)
#define M4_SRAM14_PD_WU				(uint32_t)(0x4000)
#define M4_SRAM15_PD_WU				(uint32_t)(0x8000)


#define SDMA_SW_PD_WU				(uint32_t)(0x1)
#define EFUSE_SW_PD_WU				(uint32_t)(0x2)
#define I2S_SW_PD_WU				(uint32_t)(0x20)
#define A1_SW_PD_WU					(uint32_t)(0x40)

#define AUDIO_AD0_PD_WU				(uint32_t)(0x1)
#define AUDIO_AD1_PD_WU				(uint32_t)(0x2)
#define AUDIO_AD2_PD_WU				(uint32_t)(0x4)
#define AUDIO_AD3_PD_WU				(uint32_t)(0x8)
#define AUDIO_AD4_PD_WU				(uint32_t)(0x10)
#define AUDIO_AD5_PD_WU				(uint32_t)(0x20)


/******************************************************************************
 *                                  MISC                                      *
 ******************************************************************************/
#define MISC_LOCK_KEY                               ((uint32_t)(0x1ACCE551))

/******************************************************************************
 *                                SDMA_SRAM                                   *
 ******************************************************************************/
#define SDMA_SRAM_DST_INC_BYTE                      ((uint32_t) (0x00000000))
#define SDMA_SRAM_DST_INC_HWORD                     ((uint32_t) (0x40000000))
#define SDMA_SRAM_DST_INC_WORD                      ((uint32_t) (0x80000000))
#define SDMA_SRAM_DST_SZ_BYTE                       ((uint32_t) (0x00000000))
#define SDMA_SRAM_DST_SZ_HWORD                      ((uint32_t) (0x10000000))
#define SDMA_SRAM_DST_SZ_WORD                       ((uint32_t) (0x20000000))

#define SDMA_SRAM_SRC_INC_BYTE                      ((uint32_t) (0x00000000))
#define SDMA_SRAM_SRC_INC_HWORD                     ((uint32_t) (0x04000000))
#define SDMA_SRAM_SRC_INC_WORD                      ((uint32_t) (0x08000000))
#define SDMA_SRAM_SRC_SZ_BYTE                       ((uint32_t) (0x00000000))
#define SDMA_SRAM_SRC_SZ_HWORD                      ((uint32_t) (0x01000000))
#define SDMA_SRAM_SRC_SZ_WORD                       ((uint32_t) (0x02000000))

#define SDMA_SRAM_CH_CFG_DST_CACHABLE               ((uint32_t) (0x00800000))
#define SDMA_SRAM_CH_CFG_DST_BUFFABLE               ((uint32_t) (0x00400000))
#define SDMA_SRAM_CH_CFG_DST_PRIVILEGED             ((uint32_t) (0x00200000))

#define SDMA_SRAM_CH_CFG_SRC_CACHABLE               ((uint32_t) (0x00100000))
#define SDMA_SRAM_CH_CFG_SRC_BUFFABLE               ((uint32_t) (0x00080000))
#define SDMA_SRAM_CH_CFG_SRC_PRIVILEGED             ((uint32_t) (0x00040000))

#define SDMA_SRAM_R_POWER_0                         ((uint32_t) (0x00000000))
#define SDMA_SRAM_R_POWER_2                         ((uint32_t) (0x00004000))
#define SDMA_SRAM_R_POWER_4                         ((uint32_t) (0x00008000))
#define SDMA_SRAM_R_POWER_8                         ((uint32_t) (0x0000C000))
#define SDMA_SRAM_R_POWER_16                        ((uint32_t) (0x00010000))
#define SDMA_SRAM_R_POWER_32                        ((uint32_t) (0x00014000))
#define SDMA_SRAM_R_POWER_64                        ((uint32_t) (0x00018000))
#define SDMA_SRAM_R_POWER_128                       ((uint32_t) (0x0001C000))
#define SDMA_SRAM_R_POWER_256                       ((uint32_t) (0x00020000))
#define SDMA_SRAM_R_POWER_512                       ((uint32_t) (0x00024000))
#define SDMA_SRAM_R_POWER_NO                        ((uint32_t) (0x0003C000))

#define SDMA_SRAM_N_MINUS_1_SHIFT                   ((uint32_t) 4)
#define SDMA_SRAM_NEXT_USEBURST                     ((uint32_t) (0x00000008))

#define SDMA_SRAM_CYCLE_CTRL_STOP                   ((uint32_t) (0x00000000))
#define SDMA_SRAM_CYCLE_CTRL_BASIC                  ((uint32_t) (0x00000001))
#define SDMA_SRAM_CYCLE_CTRL_AUTO_REQUEST           ((uint32_t) (0x00000002))
#define SDMA_SRAM_CYCLE_CTRL_PING_PONG              ((uint32_t) (0x00000003))
#define SDMA_SRAM_CYCLE_CTRL_PRI_SCATTER            ((uint32_t) (0x00000004))
#define SDMA_SRAM_CYCLE_CTRL_ALT_SCATTER            ((uint32_t) (0x00000005))
#define SDMA_SRAM_CYCLE_CTRL_PERI_SCATTER           ((uint32_t) (0x00000006))
#define SDMA_SRAM_CYCLE_CTRL_ALT_PERI_SCATTER       ((uint32_t) (0x00000007))

#define SDMA_SRAM_CH0                        (0)
#define SDMA_SRAM_CH1                        (1)
#define SDMA_SRAM_CH2                        (2)
#define SDMA_SRAM_CH3                        (3)
#define SDMA_SRAM_CH4                        (4)
#define SDMA_SRAM_CH5                        (5)
#define SDMA_SRAM_CH6                        (6)
#define SDMA_SRAM_CH7                        (7)
#define SDMA_SRAM_CH8                        (8)
#define SDMA_SRAM_CH9                        (9)
#define SDMA_SRAM_CH10                       (10)
#define SDMA_SRAM_CH11                       (11)
#define SDMA_SRAM_CH12                       (12)
#define SDMA_SRAM_CH13                       (13)
#define SDMA_SRAM_CH14                       (14)
#define SDMA_SRAM_CH15                       (15)

/******************************************************************************
 *                              SDMA_BRIDGE                                   *
 ******************************************************************************/
#define SDMA_BRIDGE_SINGLE_REQ_SHIFT                ((uint32_t) 27)
#define SDMA_BRIDGE_BURST_REQ_SHIFT                 ((uint32_t) 0)
#define SDMA_BRIDGE_CH1_SELECT                      ((uint32_t) (1 << 0))
#define SDMA_BRIDGE_CH2_SELECT                      ((uint32_t) (1 << 1))
#define SDMA_BRIDGE_CH3_SELECT                      ((uint32_t) (1 << 2))
#define SDMA_BRIDGE_CH4_SELECT                      ((uint32_t) (1 << 3))
#define SDMA_BRIDGE_CH5_SELECT                      ((uint32_t) (1 << 4))
#define SDMA_BRIDGE_CH6_SELECT                      ((uint32_t) (1 << 5))
#define SDMA_BRIDGE_CH7_SELECT                      ((uint32_t) (1 << 6))
#define SDMA_BRIDGE_CH8_SELECT                      ((uint32_t) (1 << 7))
#define SDMA_BRIDGE_CH9_SELECT                      ((uint32_t) (1 << 8))
#define SDMA_BRIDGE_CH10_SELECT                     ((uint32_t) (1 << 9))
#define SDMA_BRIDGE_CH11_SELECT                     ((uint32_t) (1 << 10))

/******************************************************************************
 *                                  SDMA                                      *
 ******************************************************************************/
#define SDMA_DMA_STATUS_TEST_STATUS_SHIFT           ((uint32_t) 28)
#define SDMA_DMA_STATUS_CHNLS_MINUS_1_SHIFT         ((uint32_t) 16)
#define SDMA_DMA_STATUS_STATE_MASK                  ((uint32_t) 0x000000F0)
#define SDMA_DMA_STATUS_IDLE_STATE                  ((uint32_t) 0x00000000)
#define SDMA_DMA_STATUS_RD_CH_CTRL_DATA_STATE       ((uint32_t) 0x00000010)
#define SDMA_DMA_STATUS_RD_SRC_DATA_PTR_STATE       ((uint32_t) 0x00000020)
#define SDMA_DMA_STATUS_RD_DEST_DATA_PTR_STATE      ((uint32_t) 0x00000030)
#define SDMA_DMA_STATUS_RD_SRC_DATA_STATE           ((uint32_t) 0x00000040)
#define SDMA_DMA_STATUS_WR_DEST_DATA_STATE          ((uint32_t) 0x00000050)
#define SDMA_DMA_STATUS_WAIT_DMA_STATE              ((uint32_t) 0x00000060)
#define SDMA_DMA_STATUS_WR_CH_CTRL_DATA_STATE       ((uint32_t) 0x00000070)
#define SDMA_DMA_STATUS_STALLED_STATE               ((uint32_t) 0x00000080)
#define SDMA_DMA_STATUS_DONE_STATE                  ((uint32_t) 0x00000090)
#define SDMA_DMA_STATUS_SCATTER_GATHER_STATE        ((uint32_t) 0x000000A0)
#define SDMA_DMA_STATUS_MASTER_ENABLE               ((uint32_t) 0x00000001)

#define SDMA_DMA_CFG_HPROT3_SET                     ((uint32_t) 0x00000080)
#define SDMA_DMA_CFG_HPROT2_SET                     ((uint32_t) 0x00000040)
#define SDMA_DMA_CFG_HPROT1_SET                     ((uint32_t) 0x00000020)
#define SDMA_DMA_CFG_MASTER_ENABLE                  ((uint32_t) 0x00000001)

#define SDMA_DMA_CH0_SELECT                         ((uint32_t) (1 << 0))
#define SDMA_DMA_CH1_SELECT                         ((uint32_t) (1 << 1))
#define SDMA_DMA_CH2_SELECT                         ((uint32_t) (1 << 2))
#define SDMA_DMA_CH3_SELECT                         ((uint32_t) (1 << 3))
#define SDMA_DMA_CH4_SELECT                         ((uint32_t) (1 << 4))
#define SDMA_DMA_CH5_SELECT                         ((uint32_t) (1 << 5))
#define SDMA_DMA_CH6_SELECT                         ((uint32_t) (1 << 6))
#define SDMA_DMA_CH7_SELECT                         ((uint32_t) (1 << 7))
#define SDMA_DMA_CH8_SELECT                         ((uint32_t) (1 << 8))
#define SDMA_DMA_CH9_SELECT                         ((uint32_t) (1 << 9))
#define SDMA_DMA_CH10_SELECT                        ((uint32_t) (1 << 10))
#define SDMA_DMA_CH11_SELECT                        ((uint32_t) (1 << 11))
#define SDMA_DMA_CH12_SELECT                        ((uint32_t) (1 << 12))
#define SDMA_DMA_CH13_SELECT                        ((uint32_t) (1 << 13))
#define SDMA_DMA_CH14_SELECT                        ((uint32_t) (1 << 14))
#define SDMA_DMA_CH15_SELECT                        ((uint32_t) (1 << 15))

/******************************************************************************
 *                                  AIP                                       *
 ******************************************************************************/
#define AIP_OSC_CTRL_EN                 	((uint32_t) (0x00000001))
#define AIP_OSC_CTRL_FRE_SEL              	((uint32_t) (0x00000002))

/******************************************************************************
 * 				    CRU                                       *
 ******************************************************************************/
//#define CLK_CTRL_CLK_DIVIDER_ENABLE		((uint32_t) (0x00000200))

#define CLK_CTRL_A_CLK_SOURCE_SEL_HSPEED         ((uint32_t) (0x00000000))
#define CLK_CTRL_A_CLK_SOURCE_SEL_32KHZ             ((uint32_t) (0x00000001))
#define CLK_CTRL_B_CLK_SOURCE_SEL_HSPEED            ((uint32_t) (0x00000000))
#define CLK_CTRL_B_CLK_SOURCE_SEL_32KHZ             ((uint32_t) (0x00000001))
#define CLK_CTRL_E_CLK_SOURCE_SEL_HSPEED            ((uint32_t) (0x00000000))
#define CLK_CTRL_E_CLK_SOURCE_SEL_32KHZ             ((uint32_t) (0x00000001))
#define CLK_CTRL_F_CLK_SOURCE_SEL_HSPEED            ((uint32_t) (0x00000000))
#define CLK_CTRL_F_CLK_SOURCE_SEL_32KHZ             ((uint32_t) (0x00000001))
#define CLK_CTRL_H_CLK_SOURCE_SEL_HSPEED            ((uint32_t) (0x00000000))
#define CLK_CTRL_H_CLK_SOURCE_SEL_32KHZ             ((uint32_t) (0x00000001))
#define CLK_CTRL_I_CLK_SOURCE_SEL_HSPEED            ((uint32_t) (0x00000000))
#define CLK_CTRL_I_CLK_SOURCE_SEL_32KHZ             ((uint32_t) (0x00000001))

#define HSPEED_CLK_SOURCE_SEL_FROM_OSC              ((uint32_t) (0x00000000))
#define HSPEED_CLK_SOURCE_SEL_FROM_PAD              ((uint32_t) (0x00000001))

#define C01_CLK_GATE_PATH_0_OFF                     ((uint32_t) (0x00000000))
#define C01_CLK_GATE_PATH_0_ON                      ((uint32_t) (0x00000001))
#define C01_CLK_GATE_PATH_1_OFF                     ((uint32_t) (0x00000000))
#define C01_CLK_GATE_PATH_1_ON                      ((uint32_t) (0x00000002))
#define C01_CLK_GATE_PATH_2_OFF                     ((uint32_t) (0x00000000))
#define C01_CLK_GATE_PATH_2_ON                      ((uint32_t) (0x00000004))
#define C01_CLK_GATE_PATH_3_OFF                     ((uint32_t) (0x00000000))
#define C01_CLK_GATE_PATH_3_ON                      ((uint32_t) (0x00000008))
#define C01_CLK_GATE_PATH_4_OFF                     ((uint32_t) (0x00000000))
#define C01_CLK_GATE_PATH_4_ON                      ((uint32_t) (0x00000010))
#define C01_CLK_GATE_PATH_5_OFF                     ((uint32_t) (0x00000000))
#define C01_CLK_GATE_PATH_5_ON                      ((uint32_t) (0x00000020))

#define C02_CLK_GATE_PATH_0_OFF                     ((uint32_t) (0x00000000))
#define C02_CLK_GATE_PATH_0_ON                      ((uint32_t) (0x00000001))
#define C02_CLK_GATE_PATH_1_OFF                     ((uint32_t) (0x00000000))
#define C02_CLK_GATE_PATH_1_ON                      ((uint32_t) (0x00000002))
#define C02_CLK_GATE_PATH_2_OFF                     ((uint32_t) (0x00000000))
#define C02_CLK_GATE_PATH_2_ON                      ((uint32_t) (0x00000004))
#define C02_CLK_GATE_PATH_3_OFF                     ((uint32_t) (0x00000000))
#define C02_CLK_GATE_PATH_3_ON                      ((uint32_t) (0x00000008))
#define C02_CLK_GATE_PATH_4_OFF                     ((uint32_t) (0x00000000))
#define C02_CLK_GATE_PATH_4_ON                      ((uint32_t) (0x00000010))
#define C02_CLK_GATE_PATH_5_OFF                     ((uint32_t) (0x00000000))
#define C02_CLK_GATE_PATH_5_ON                      ((uint32_t) (0x00000020))

#define C08_X4_CLK_GATE_PATH_0_OFF                  ((uint32_t) (0x00000000))
#define C08_X4_CLK_GATE_PATH_0_ON                   ((uint32_t) (0x00000001))

#define C08_X1_CLK_GATE_PATH_0_OFF                  ((uint32_t) (0x00000000))
#define C08_X1_CLK_GATE_PATH_0_ON                   ((uint32_t) (0x00000001))
#define C08_X1_CLK_GATE_PATH_1_OFF                  ((uint32_t) (0x00000000))
#define C08_X1_CLK_GATE_PATH_1_ON                   ((uint32_t) (0x00000002))
#define C08_X1_CLK_GATE_PATH_2_OFF                  ((uint32_t) (0x00000000))
#define C08_X1_CLK_GATE_PATH_2_ON                   ((uint32_t) (0x00000004))
#define C08_X1_CLK_GATE_PATH_3_OFF                  ((uint32_t) (0x00000000))
#define C08_X1_CLK_GATE_PATH_3_ON                   ((uint32_t) (0x00000008))
#define C08_X1_CLK_GATE_PATH_4_OFF                  ((uint32_t) (0x00000000))
#define C08_X1_CLK_GATE_PATH_4_ON                   ((uint32_t) (0x00000010))
#define C08_X1_CLK_GATE_PATH_5_OFF                  ((uint32_t) (0x00000000))
#define C08_X1_CLK_GATE_PATH_5_ON                   ((uint32_t) (0x00000020))
#define C08_X1_CLK_GATE_PATH_6_OFF                  ((uint32_t) (0x00000000))
#define C08_X1_CLK_GATE_PATH_6_ON                   ((uint32_t) (0x00000040))

#define C10_FCLK_GATE_PATH_0_OFF                    ((uint32_t) (0x00000000))
#define C10_FCLK_GATE_PATH_0_ON                     ((uint32_t) (0x00000001))
#define C10_FCLK_GATE_PATH_1_OFF                    ((uint32_t) (0x00000000))
#define C10_FCLK_GATE_PATH_1_ON                     ((uint32_t) (0x00000002))
#define C10_FCLK_GATE_PATH_2_OFF                    ((uint32_t) (0x00000000))
#define C10_FCLK_GATE_PATH_2_ON                     ((uint32_t) (0x00000004))
#define C10_FCLK_GATE_PATH_3_OFF                    ((uint32_t) (0x00000000))
#define C10_FCLK_GATE_PATH_3_ON                     ((uint32_t) (0x00000008))
#define C10_FCLK_GATE_PATH_4_OFF                    ((uint32_t) (0x00000000))
#define C10_FCLK_GATE_PATH_4_ON                     ((uint32_t) (0x00000010))
#define C10_FCLK_GATE_PATH_5_OFF                    ((uint32_t) (0x00000000))
#define C10_FCLK_GATE_PATH_5_ON                     ((uint32_t) (0x00000020))
#define C10_FCLK_GATE_PATH_6_OFF                    ((uint32_t) (0x00000000))
#define C10_FCLK_GATE_PATH_6_ON                     ((uint32_t) (0x00000040))

#define C11_CLK_GATE_PATH_0_OFF                     ((uint32_t) (0x00000000))
#define C11_CLK_GATE_PATH_0_ON                      ((uint32_t) (0x00000001))
#define C11_CLK_GATE_PATH_1_OFF                     ((uint32_t) (0x00000000))
#define C11_CLK_GATE_PATH_1_ON                      ((uint32_t) (0x00000002))
#define C11_CLK_GATE_PATH_2_OFF                     ((uint32_t) (0x00000000))
#define C11_CLK_GATE_PATH_2_ON                      ((uint32_t) (0x00000004))
#define C11_CLK_GATE_PATH_3_OFF                     ((uint32_t) (0x00000000))
#define C11_CLK_GATE_PATH_3_ON                      ((uint32_t) (0x00000008))
#define C11_CLK_GATE_PATH_4_OFF                     ((uint32_t) (0x00000000))
#define C11_CLK_GATE_PATH_4_ON                      ((uint32_t) (0x00000010))
#define C11_CLK_GATE_PATH_5_OFF                     ((uint32_t) (0x00000000))
#define C11_CLK_GATE_PATH_5_ON                      ((uint32_t) (0x00000020))

#define C12_CLK_GATE_PATH_0_OFF                     ((uint32_t) (0x00000000))
#define C12_CLK_GATE_PATH_0_ON                      ((uint32_t) (0x00000001))
#define C12_CLK_GATE_PATH_1_OFF                     ((uint32_t) (0x00000000))
#define C12_CLK_GATE_PATH_1_ON                      ((uint32_t) (0x00000002))

#define CS_CLK_GATE_PATH_0_OFF                      ((uint32_t) (0x00000000))
#define CS_CLK_GATE_PATH_0_ON                       ((uint32_t) (0x00000001))

#define CU_CLK_GATE_PATH_0_OFF                      ((uint32_t) (0x00000000))
#define CU_CLK_GATE_PATH_0_ON                       ((uint32_t) (0x00000001))
#define CU_CLK_GATE_PATH_1_OFF                      ((uint32_t) (0x00000000))
#define CU_CLK_GATE_PATH_1_ON                       ((uint32_t) (0x00000002))

#define C16_CLK_GATE_PATH_0_OFF                     ((uint32_t) (0x00000000))
#define C16_CLK_GATE_PATH_0_ON                      ((uint32_t) (0x00000001))
#define C18_CLK_GATE_PATH_0_OFF                     ((uint32_t) (0x00000000))
#define C18_CLK_GATE_PATH_0_ON                      ((uint32_t) (0x00000001))
#define C19_CLK_GATE_PATH_0_OFF                     ((uint32_t) (0x00000000))
#define C19_CLK_GATE_PATH_0_ON                      ((uint32_t) (0x00000001))
#define C21_CLK_GATE_PATH_0_OFF                     ((uint32_t) (0x00000000))
#define C21_CLK_GATE_PATH_0_ON                      ((uint32_t) (0x00000001))

#define C22_CLK_CTRL_GATE_CTRL_OFF                  ((uint32_t) (0x00000000))
#define C22_CLK_CTRL_GATE_CTRL_ON                   ((uint32_t) (0x00000001))
#define C22_CLK_CTRL_SOURCE_SEL_I2S                 ((uint32_t) (0x00000000))
#define C22_CLK_CTRL_SOURCE_SEL_PDM                 ((uint32_t) (0x00000002))

#define C30C31_CLK_CTRL_SOURCE_PDM_LEFT             ((uint32_t) (0x00000001))
#define C30C31_CLK_CTRL_SOURCE_PDM_RIGHT            ((uint32_t) (0x00000002))

/// @endcond EOSS3_DEV_MACROS

#define REBOOT_CAUSE_HARDFAULT	(0x1)
#define REBOOT_CAUSE_FLASHING	(0x2)
#define REBOOT_CAUSE_SOFTFAULT	(0x3)
#define REBOOT_CAUSE		(0xF)
#define REBOOT_STATUS_REG	(PMU->MISC_POR_3)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __EOSS3_DEV_H */

