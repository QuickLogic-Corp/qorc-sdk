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
 *    File   : s3x_dfs.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __S3X_DFS_H
#define __S3X_DFS_H

#include "stdbool.h"
#include <test_types.h>
#include <s3x_err_base.h>

/*** Debug ***/
#define     K_DBGTRACE_DFS  200


/* The systick is a 24-bit counter. */
#define portMAX_24_BIT_NUMBER               ( 0xffffffUL )
/* Constants required to manipulate the core.  Registers first... */
#define portNVIC_SYSTICK_CTRL_REG           ( * ( ( volatile uint32_t * ) 0xe000e010 ) )
#define portNVIC_SYSTICK_LOAD_REG           ( * ( ( volatile uint32_t * ) 0xe000e014 ) )
#define portNVIC_SYSTICK_CURRENT_VALUE_REG  ( * ( ( volatile uint32_t * ) 0xe000e018 ) )
#define portNVIC_SYSHPR3_REG                ( * ( ( volatile uint32_t * ) 0xe000ed20 ) )
/* ...then bits in the registers. */
#define portNVIC_SYSTICK_INT_BIT            ( 1UL << 1UL )
#define portNVIC_SYSTICK_ENABLE_BIT         ( 1UL << 0UL )
#define portNVIC_SYSTICK_COUNT_FLAG_BIT     ( 1UL << 16UL )
#define portNVIC_PENDSVCLEAR_BIT            ( 1UL << 27UL )
#define portNVIC_PEND_SYSTICK_CLEAR_BIT     ( 1UL << 25UL )


/* timer hook for dfs */
void S3x_Tick_Hook(void);

typedef enum {
    DFS_DIS = 0,
    DFS_EN,
    DFS_SUS,
	DFS_RES,
} S3x_DFS_ST;


typedef enum {
    STEP_UP = 0,
    STEP_DOWN,
    LOWEST_VAL,
    MAX_VAL,
} S3x_Policy_level;
#define KPOLICY_CLK_DOMAINS 4
#define KCRUVAL             20
typedef struct {
    UINT8_t     clk_domain[KPOLICY_CLK_DOMAINS];
    UINT32_t    rate[KPOLICY_CLK_DOMAINS];
    uint32_t    minHSOSC;
    UINT16_t    step_width;
    uint16_t    cpuload_downthreshold;  // Use next lower policy if cpu load is less than this value
    uint16_t    cpuload_upthreshold;    // Use next higher policy if cpu load is greater than this value
    uint8_t     policySleep;            // Set this if the sleep policy is not the current policy
} S3x_Policy_Node;

extern S3x_Policy_Node dfs_node[];
extern uint8_t policyInitial;          // Initial policy

void S3x_Dfs_Handler(void);

int S3x_Set_Policy(UINT8_t index, UINT32_t src, uint32_t* cru000val);

uint8_t S3x_Is_Dfs_En(void);

void S3x_Set_Dfs_st(S3x_DFS_ST st);

uint8_t DFS_Get_Curr_Policy(void);

uint32_t s3x_get_policy_max_rate (uint8_t domain_id);

void PMU_Disable_Timer(void);

int S3x_Set_Lowest_Policy(void);

uint32_t s3x_get_policy_max_rate (uint8_t domain_id);

void S3x_Setup_Next_Trigger(UINT16_t width);

void s3x_suspend_dfs(void);

void s3x_dfs_Tick_Handler(void);

void  s3x_set_min_allowed_policy (uint8_t min_policy);
void        DFS_AlignPolicy(uint8_t policy);
void        DFS_RestoreFromSleep(void);
void        DFS_SwitchToSleepPolicy(void);                  // Switch to the DFS policy defined for sleep
bool        DFS_fPolicyHasDeepSleep(void);                  // The current DFS policy has an alternate policy for deep sleep
void        DFS_Enable(void);
void        s3x_disable_dfs(void);
void        DFS_StartTimer(void);
uint16_t    DFS_cpuload(void);
void        DFS_updatesleepticks(uint32_t ticks);
void        DFS_resetsleepticks(void);
uint32_t    DFS_FreqCPUSleep(void);
void        DFS_Initialize(void);

#endif      /* __S3X_CLOCK_H  */
