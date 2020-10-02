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
 *-  Copyright Notice  -------------------------------------
 *                                                          
 *    Licensed Materials - Property of QuickLogic Corp.     
 *    Copyright (C) 2019 QuickLogic Corporation             
 *    All rights reserved                                   
 *    Use, duplication, or disclosure restricted            
 *                                                          
 *    File   : s3x_clock.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __S3X_CLOCK_H
#define __S3X_CLOCK_H

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <stdbool.h>

#include <test_types.h>
#include <s3x_err_base.h>
#include <s3x_clock_hal.h>
#include <s3x_pwrcfg.h>

/* TODO Add Clk Tree  */

#define OSC_CLK_LOCKED()                            (AIP->OSC_STA_0 & 0x1)
#define OSC_SET_FREQ_INC(FREQ)                      (AIP->OSC_CTRL_1 = ((FREQ/32768)-3) & 0xFFF)
#define OSC_GET_FREQ_INC()                          (((AIP->OSC_CTRL_1 & 0xFFF)+3)*32768)

#define MAX_CHILD 4
#define SRC_CLK_DIV_OFF 0x124
#define CRU_RVAL(x) (*((int *)((int)CRU + x)))
#define CRU_WVAL(x, y) (*(int *)((int )CRU + x) = y)
#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a):(b))
#endif
#define INVALID_INDEX 0xFF
#define GATE_EN 1
#define GATE_DIS 0
#ifdef DEF_CRU_DBG

#define MUX_RVAL(x) (*((int *)((int)IO_MUX + x)))
#define MUX_WVAL(x, y) (*(int *)((int )IO_MUX + x) = y)

#define PAD_12_OFF 0x30
#define PAD_32_OFF 0x80
#define PAD_41_OFF 0xA4
#define PAD_13_OFF 0x34
#define PAD_33_OFF 0x84
#define PAD_42_OFF 0xAC

#endif

#ifndef SIZEOF_ARRAY
#define SIZEOF_ARRAY(array) (sizeof((array))/sizeof((array)[0]))
#endif

#define LOCK_KEY  ( 1U << 0U)
#define HW_GATED  ( 1U << 1U)
#define KEEP_ON  ( 1U << 2U)
#define KEY_VALUE 0x1ACCE551UL

//These are optimized freqs for power management
#define F_1MHZ  (1000*1000)
#define F_2MHZ   (2*F_1MHZ)
#define F_3MHZ   (3*F_1MHZ)
#define F_4MHZ   (4*F_1MHZ)
#define F_5MHZ   (5*F_1MHZ)
#define F_6MHZ   (6*F_1MHZ)
#define F_8MHZ   (8*F_1MHZ)
#define F_9MHZ   (9*F_1MHZ)
#define F_10MHZ  (10*F_1MHZ)
#define F_12MHZ  (12*F_1MHZ)
#define F_15MHZ  (15*F_1MHZ)
#define F_16MHZ  (16*F_1MHZ)
#define F_18MHZ  (18*F_1MHZ)
#define F_20MHZ  (20*F_1MHZ)
#define F_21MHZ  (21*F_1MHZ)
#define F_24MHZ  (24*F_1MHZ)
#define F_27MHZ  (27*F_1MHZ)
#define F_30MHZ  (30*F_1MHZ)
#define F_32MHZ  (32*F_1MHZ)
#define F_35MHZ  (35*F_1MHZ)
#define F_36MHZ  (36*F_1MHZ)
#define F_40MHZ  (40*F_1MHZ)
#define F_45MHZ  (45*F_1MHZ)
#define F_48MHZ  (48*F_1MHZ)
#define F_54MHZ  (54*F_1MHZ)
#define F_60MHZ  (60*F_1MHZ)
#define F_64MHZ  (64*F_1MHZ)
#define F_70MHZ  (70*F_1MHZ)
#define F_72MHZ  (72*F_1MHZ)
#define F_80MHZ  (80*F_1MHZ)
#define F_768KHZ (F_3MHZ/4)
#define F_1p5MHZ (F_3MHZ/2)
#define F_7p5MHZ (F_15MHZ/2)
#define F_512KHZ (F_1MHZ/2)
#define F_256KHZ (F_1MHZ/4)

//These are optimized freqs for power management
#define FREQ_1MHZ 1024000
#define FREQ_2MHZ   (2*FREQ_1MHZ)
#define FREQ_3MHZ   (3*FREQ_1MHZ)
#define FREQ_4MHZ   (4*FREQ_1MHZ)
#define FREQ_5MHZ   (5*FREQ_1MHZ)
#define FREQ_6MHZ   (6*FREQ_1MHZ)
#define FREQ_8MHZ   (8*FREQ_1MHZ)
#define FREQ_9MHZ   (9*FREQ_1MHZ)
#define FREQ_10MHZ  (10*FREQ_1MHZ)
#define FREQ_12MHZ  (12*FREQ_1MHZ)
#define FREQ_15MHZ  (15*FREQ_1MHZ)
#define FREQ_16MHZ  (16*FREQ_1MHZ)
#define FREQ_18MHZ  (18*FREQ_1MHZ)
#define FREQ_20MHZ  (20*FREQ_1MHZ)
#define FREQ_21MHZ  (21*FREQ_1MHZ)
#define FREQ_24MHZ  (24*FREQ_1MHZ)
#define FREQ_27MHZ  (27*FREQ_1MHZ)
#define FREQ_30MHZ  (30*FREQ_1MHZ)
#define FREQ_32MHZ  (32*FREQ_1MHZ)
#define FREQ_35MHZ  (35*FREQ_1MHZ)
#define FREQ_36MHZ  (36*FREQ_1MHZ)
#define FREQ_40MHZ  (40*FREQ_1MHZ)
#define FREQ_45MHZ  (45*FREQ_1MHZ)
#define FREQ_48MHZ  (48*FREQ_1MHZ)
#define FREQ_54MHZ  (54*FREQ_1MHZ)
#define FREQ_60MHZ  (60*FREQ_1MHZ)
#define FREQ_64MHZ  (64*FREQ_1MHZ)
#define FREQ_70MHZ  (70*FREQ_1MHZ)
#define FREQ_72MHZ  (72*FREQ_1MHZ)
#define FREQ_80MHZ  (80*FREQ_1MHZ)
#define FREQ_768KHZ (FREQ_3MHZ/4)
#define FREQ_1p5MHZ (FREQ_3MHZ/2)
#define FREQ_7p5MHZ (FREQ_15MHZ/2)
#define FREQ_512KHZ (FREQ_1MHZ/2)
#define FREQ_256KHZ (FREQ_1MHZ/4)

//These are optimized freqs for power management
#define HSOSC_1MHZ 1024000
#define HSOSC_2MHZ   (2*HSOSC_1MHZ)
#define HSOSC_3MHZ   (3*HSOSC_1MHZ)
#define HSOSC_4MHZ   (4*HSOSC_1MHZ)
#define HSOSC_5MHZ   (5*HSOSC_1MHZ)
#define HSOSC_6MHZ   (6*HSOSC_1MHZ)
#define HSOSC_8MHZ   (8*HSOSC_1MHZ)
#define HSOSC_9MHZ   (9*HSOSC_1MHZ)
#define HSOSC_10MHZ  (10*HSOSC_1MHZ)
#define HSOSC_12MHZ  (12*HSOSC_1MHZ)
#define HSOSC_15MHZ  (15*HSOSC_1MHZ)
#define HSOSC_16MHZ  (16*HSOSC_1MHZ)
#define HSOSC_18MHZ  (18*HSOSC_1MHZ)
#define HSOSC_20MHZ  (20*HSOSC_1MHZ)
#define HSOSC_21MHZ  (21*HSOSC_1MHZ)
#define HSOSC_24MHZ  (24*HSOSC_1MHZ)
#define HSOSC_27MHZ  (27*HSOSC_1MHZ)
#define HSOSC_30MHZ  (30*HSOSC_1MHZ)
#define HSOSC_32MHZ  (32*HSOSC_1MHZ)
#define HSOSC_35MHZ  (35*HSOSC_1MHZ)
#define HSOSC_36MHZ  (36*HSOSC_1MHZ)
#define HSOSC_40MHZ  (40*HSOSC_1MHZ)
#define HSOSC_45MHZ  (45*HSOSC_1MHZ)
#define HSOSC_48MHZ  (48*HSOSC_1MHZ)
#define HSOSC_54MHZ  (54*HSOSC_1MHZ)
#define HSOSC_60MHZ  (60*HSOSC_1MHZ)
#define HSOSC_64MHZ  (64*HSOSC_1MHZ)
#define HSOSC_70MHZ  (70*HSOSC_1MHZ)
#define HSOSC_72MHZ  (72*HSOSC_1MHZ)
#define HSOSC_80MHZ  (80*HSOSC_1MHZ)
//#define HSOSC_72MHZ  (73728000)

#define HSOSC_768KHZ (HSOSC_3MHZ/4)
#define HSOSC_1p5MHZ (HSOSC_3MHZ/2)
#define HSOSC_7p5MHZ (HSOSC_15MHZ/2)
#define HSOSC_512KHZ (HSOSC_1MHZ/2)
#define HSOSC_256KHZ (HSOSC_1MHZ/4)

#define SCALE_DOWN 0U
#define SCALE_UP 1U

typedef struct
{
    UINT8_t en;
    UINT8_t init;
    UINT32_t rate;
    UINT32_t qos_rate;
} Hsosc_Param;

/* Source Down Clk domain info*/
typedef struct
{
    UINT8_t cnt;
    UINT8_t sd_id[MAX_CHILD];
} Sd_Clkd;

typedef struct
{
    UINT8_t src_domain;
    UINT32_t src_rate;
} Src_Clkd;

/*Sync clk info
for Sync down clock (e.g. C01, C09) this contain source domain(clock domain)
for source domain (e.g. C10) is contain id of sync down clk (Sd_Clkd sd_clk)*/
typedef union
{
    Src_Clkd sclk;
    Sd_Clkd sd_clk;
} Sync_Clk_Info;

typedef struct
{
  UINT32_t irate;
  UINT16_t imask;
  UINT8_t  en;
} Init_State;

typedef struct
{
    UINT16_t div_off;
    UINT16_t div_max;
    /*SD clock div disable will disable clock, for src clock
    it will be pass through no divide */
    UINT8_t div_en_shift;
    UINT16_t src_sel_off;
    UINT16_t gate_off;
    UINT16_t gate_mask;

    UINT16_t src_div_shift;
} Cru_Ctrl;

struct _Qos_node;

/* S3 clock domain data type */
typedef struct
{
    char name[6];
    /* Ref Clk : Src clk : Sync Down Clk */
    UINT8_t type : 4;
    /* HSOSC or RTC */
    UINT8_t ref_src : 4;
    /*Additional Ctrl */
    UINT8_t flags;
    /* domain_id */
    UINT8_t clkd_id;
    UINT32_t curr_rate;
    UINT32_t src_rate;
    /* stores the expected rate to be set for SD_CLK when the divider value reached beyond div_max value*/
    UINT32_t expected_rate;
    /* Current Policy max*/
    UINT32_t max_rate;
    /* Design max val*/
    UINT32_t def_max_rate;
    /* Div val */
    UINT16_t div_val;
    /* SW latch value */
    UINT16_t gate_val;
    /* Source Div latch val */
    UINT8_t src_div;
    /* sync down clocks or Source domain info */
    Sync_Clk_Info sync_clk;
    /* ctrl reg */
    Cru_Ctrl cru_ctrl;
    Init_State init_state;
    //void *qos;
    struct _Qos_node *qos;
    SemaphoreHandle_t clkd_sem;
    char clkd_sem_name[10];
} S3x_ClkD;

typedef struct _Qos_node
{
    QOS_REQ_TYPE active_req;
    UINT8_t req_cnt;
    UINT32_t rate[MAX_QOS_REQ];
    S3x_ClkD *clkd;
    struct _Qos_node *next;
} Qos_node;

/* Reg/Val pair */
typedef struct cruval {
    bool        fValid;
    uint32_t    regoffset;
    uint32_t    value;
} s3x_cruval_t;


/* Clock Domain Types */
typedef enum {
    REF_CLK = 0, /* RTC CLK or HSC CLK */
    SD_CLK, /* Sync Down Clock Domain e.g. C01, C09 */
    FD_CLK, /* Fix Divide Clk X1*/
    SRC_CLK
} CLK_TYPE;

typedef enum {
    HS_OSC = 0, /*HS OSC CLK */
    RTC, /* 32 || 16 KHz RTC */
} REF_CLK_TYPE;

typedef enum {
    CLK_OFF = 0,
    CLK_ON,
    CLK_DEF,
} CLK_STATE;

/* Clock Domain */
typedef enum {
    CLK_C02,
    CLK_C08X4,
    CLK_C08X1,
    CLK_C30,
    CLK_C31,
    CLK_C11,
    CLK_C16,
    CLK_C19,
    CLK_C21,
    CLK_C10,                    // Derived from HSOSC, max 80MHz: M4, M4AHB, M4SRAM, C09
    CLK_C01,                    // Derived from C10, max 10MHz: AHB switch, CfgDMA, I2S slave APB, SDMA AHB2APB, SDMA SRAM, FFE AHB, PKT FIFO AHB
    CLK_C09,                    // Derived from C10, max 10MHz: Voice APB, CFG_CTL APB 
    CLK_DOMAIN_MAX,
} CLK_DOMAIN_ID;


/* Source Divider of Clock Domain */
typedef enum {
    CLK_DIV_J,
    CLK_DIV_I,
    CLK_DIV_H,
    CLK_DIV_G,
    CLK_DIV_F,
    RESERVED,
    CLK_DIV_D,
    CLK_DIV_C,
    CLK_DIV_B,
    CLK_DIV_A
} CLK_DIV_MASK;

/*! \enum C01ClkGate
    \brief Options for gating clocks for C01 clock domain
*/
typedef enum
{
    C01_CLKGATE_A0 = 0x1,
    C01_CLKGATE_SDMA_SRAM = 0x2,
    C01_CLKGATE_PK_FIFO = 0x4,
    C01_CLKGATE_FFE = 0x8,
    C01_CLKGATE_APB2AHB_CFGDMA = 0x10,
    C01_CLKGATE_I2S = 0x20,
    C01_CLKGATE_SDMA = 0x40,
    C01_CLKGATE_EFUSE = 0x80,
    C01_CLKGATE_DBG_CTRL = 0x100,
    C01_CLKGATE_SPT = 0x200,
} C01_ClkGate;

/*! \enum C02ClkGate
    \brief Options for gating clocks for C02 clock domain
*/
typedef enum
{
    C02_CLKGATE_A1 = 0x1,
    C02_CLKGATE_FB = 0x2,
    C02_CLKGATE_EFUSE = 0x4,
} C02_ClkGate;

/*! \enum C08X4ClkGate
    \brief Options for gating clocks for C08X4 clock domain
*/
typedef enum
{
    C08X4_CLKGATE_FFE_X4CLK = 0x1,
} C08X4_ClkGate;

/*! \enum C08X1ClkGate
    \brief Options for gating clocks for C08X1 clock domain
*/
typedef enum
{
    C08X1_CLKGATE_FFE_X1CLK = 0x1,
    C08X1_CLKGATE_FB = 0x2,
    C08X1_CLKGATE_A0 = 0x4,
    C08X1_CLKGATE_ASYNC_PF0 = 0x8,
} C08X1_ClkGate;

/*! \enum C09ClkGate
    \brief Options for gating clocks for C09 clock domain
*/
typedef enum
{
    C09_CLKGATE_AUDIO_APB = 0x1,
    C09_CLKGATE_PIF = 0x2,
    C09_CLKGATE_FB = 0x4,
} C09_ClkGate;

/*! \enum C10ClkGate
    \brief Options for gating clocks for C10 clock domain
*/
typedef enum
{
    C10_CLKGATE_M4_BM_TB = 0x1,
    C10_CLKGATE_M4_SR0_SR3 = 0x2,
    C10_CLKGATE_M4_SR4_SR7 = 0x4,
    C10_CLKGATE_M4_SR8_SR11 = 0x8,
    C10_CLKGATE_M4_SR12_SR15 = 0x10,
    C10_CLKGATE_AUDIO_DMA = 0x20,
    C10_CLKGATE_SYNC_A0_AHB = 0x40,
} C10_ClkGate;

/*! \enum C11ClkGate
    \brief Options for gating clocks for C11 clock domain
*/
typedef enum
{
    C11_CLKGATE_M4_PERIPHERAL = 0x1,
} C11_ClkGate;

/*! \enum C12ClkGate
    \brief Options for gating clocks for C12 clock domain
*/
typedef enum
{
    C12_CLKRESERVED = 0,
} C12_ClkGate;

/*! \enum CSClkGate
    \brief Options for gating clocks for CS clock domain
*/
typedef enum
{
    CS_CLKGATE_SWD = 0x1,
} CS_ClkGate;

/*! \enum CUClkGate
    \brief Options for gating clocks for CU clock domain
*/
typedef enum
{
    CU_CLKRESERVED = 0,
} CU_ClkGate;

/*!     \enum C16ClkGate
     \brief Options for gating clocks for C16 clock domain
*/
typedef enum
{
    C16_CLKGATE_FB = 0x1,
} C16_ClkGate;

/*!     \enum C19ClkGate
     \brief Options for gating clocks for C19 clock domain
*/
typedef enum
{
    C19_CLKGATE_ADC = 0x1,
} C19_ClkGate;

/*! \enum C21ClkGate
    \brief Options for gating clocks for C21 clock domain
*/
typedef enum
{
    C21_CLKGATE_FB = 0x1,
} C21_ClkGate;

/*! \enum C3031ClkGate
    \brief Options for gating clocks for C30 and C31 clock domain
*/
typedef enum
{
    C30_CLKGATE_PDM_LEFT = 0x1,
    C30_CLKGATE_PDM_RIGHT = 0x2,
#ifdef PDM_MIC_STEREO
    C30_CLKGATE_PDM_STEREO = 0x3,
#endif
    C30_CLKGATE_I2S_MASTER = 0x4,
    C31_CLKGATE_LPSD = 0x8,
} C3031_ClkGate;

/*
CRU Debug Sel
*/
typedef enum
{
    SEL_NODE,
    SEL_C00,
    SEL_C01,
    SEL_C02,
    SEL_C8X4,
    SEL_C8X1,
    SEL_C09,
    SEL_C10,
    SEL_C11,
    SEL_CS,
    SEL_C16,
    SEL_C19,
    SEL_C20_C32,
    SEL_C21,
    SEL_C23,
    SEL_C30_C31,
    SEL_CRU_MAX,
} CRU_DBG_SEL;

#ifdef DEF_CRU_DBG
/*
DBG PAD
*/
typedef enum
{
    DPAD_12,
    DPAD_32,
    DPAD_41,
    DPAD_13,
    DPAD_33,
    DPAD_42,
    DPAD_MAX
} DMON_PAD;

#endif

#define CPU_CYCLE_PER_TICK (S3x_Clkd_Get_Cpu_Rate() / configTICK_RATE_HZ)

#define GET_QOS_INDEX(x)  (31 - __CLZ(x))

__INLINE UINT8_t countbits(UINT32_t b)
{
    UINT32_t count;

    for (count = 0; b != 0; count++) {
        b &= b - 1; // this clears the LSB-most set bit
    }
    return (count);
}
static int S3x_Get_Clk_Div_Val(int src_clk_id, int out_clk_val);

/* Set Divider value of all the Child Clock */
static int S3x_Update_SD_Clk(int clk_id);

/* Should update Src_clk divider if required and the divider of all child  with
 New Src clk value*/
static int S3x_Scaleup_Src_Clk_From(int req_clk);

static int S3x_Get_Clk_Src(int clk_id);

static int S3x_Set_Clk_Src(int clk_id);

void S3x_Clk_Init();

int _S3x_Clk_Enable(UINT32_t id);
int _S3x_Clk_Disable(UINT32_t id);
int _S3x_Clk_Get_Status(UINT32_t id);
int _S3x_Clk_Get_Rate(UINT32_t id);
int _S3x_Clk_Set_Rate(UINT32_t id, UINT32_t rate);
int _S3x_Clk_Get_Usecnt(UINT32_t id);
//int _S3x_Clk_Set_Rate(UINT32_t id, UINT32_t rate_min, UINT32_t rate_max);

static void  s3x_clkd_write_div(S3x_ClkD *clkd, UINT16_t div, UINT32_t rate, uint32_t* pcruval);

static UINT16_t s3x_clkd_read_div(S3x_ClkD *clkd);

UINT32_t s3x_clkd_get_src_rate(S3x_ClkD *clkd);

static int s3x_clkd_cal_div(S3x_ClkD *clkd, UINT32_t rate, UINT32_t src);

static int s3x_clkd_srate(S3x_ClkD *clkd, UINT32_t rate);

S3x_ClkD* S3x_Id_To_Domain(uint8_t id);

int  _s3x_clkd_srate(S3x_ClkD *clkd, UINT32_t rate);

UINT32_t S3x_Clkd_Get_Cpu_Rate(void);

UINT32_t S3x_Clkd_Get_Hsosc_Rate(void);

int S3x_Clkd_Change_Hsosc(UINT32_t rate);

int s3x_clkd_get_init_rate(uint8_t id);

void S3x_pwrcfg_init(void);

int  S3x_register_clkd_qnode(S3x_ClkD *clkd);

UINT32_t s3x_clkd_set_HSOSC_qos_rate(UINT32_t rate);

int S3x_set_clkd_qos_req(S3x_ClkD *clkd, QOS_REQ_TYPE req, UINT32_t val);

int S3x_get_clkd_qos_req(S3x_ClkD *clkd, QOS_REQ_TYPE req);

UINT32_t S3x_get_qos_rate(QOS_REQ_TYPE req);

int _S3x_register_qos_node(UINT32_t id);

int _S3x_set_qos_req(UINT32_t id, QOS_REQ_TYPE req, UINT32_t val);

int _S3x_get_qos_req(UINT32_t id, QOS_REQ_TYPE req);

UINT32_t S3x_Clkd_Get_Cpu_Rate(void);

UINT32_t s3x_clkd_get_active_HSOSC_qos(void);

int _S3x_clkd_set_cpu_qos_rate(S3x_ClkD *clkd, UINT32_t rate);

static int  s3x_clkd_grate(S3x_ClkD *clkd);

int s3x_update_clk_rate(S3x_ClkD *clkd, UINT32_t rate, UINT32_t src_rate, uint32_t* pcruVal);
#endif      /* __S3X_CLOCK_H  */
