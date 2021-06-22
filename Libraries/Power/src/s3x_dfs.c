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
 *    File   : s3x_dfs.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "stdbool.h"
#include "eoss3_hal_gpio.h"
#include "eoss3_dev.h"
#include "s3x_clock.h"
#include "s3x_dfs.h"
#include "s3x_lpm.h"
#include "dbg_uart.h"
#if DFS_LED_TEST == 1
#include "leds.h"
#endif // DFS_LED_TEST

/*** Local macros ***/
#define CLKD_MAX_INDEX_PNODE (SIZEOF_ARRAY(dfs_node[0].clk_domain) - 1)

/*** Define local functions ***/
static void DFS_TimerCB(TimerHandle_t);
static void DFS_UpdateTimer(void);

static int S3x_Stepup_Policy();

#if (CONST_FREQ == 0)
static uint8_t s3x_dfs_enable = DFS_EN;
#else
static uint8_t s3x_dfs_enable = DFS_DIS;
#endif

static uint8_t          policyCurrent = 0xFF;
static uint8_t          policyBeforeSleep;
static uint8_t          policyDuringSleep;
static uint32_t         policyMaxrate = 0;
static uint8_t          dfs_min_policy = 0;

extern                  S3x_Policy_Node dfs_node[];
extern uint8_t          S3_dfs_max_index;
uint8_t                 stepup_clkd_index[] = {C10_IDX, C01_IDX, C09_IDX, C8X4_IDX};
uint8_t                 stepdown_clkd_index[] = {C8X4_IDX, C10_IDX, C01_IDX, C09_IDX};
static TimerHandle_t    DFS_TimerHandle = NULL;
static uint32_t         freqHsoscBeforeSleep;
static uint32_t         cruReg0x000;
static uint32_t         cruReg0x110;
static uint32_t         cruReg0x114;

static s3x_cruval_t     acruval[KPOLICY_CLK_DOMAINS];
static s3x_cruval_t     acruval2[KCRUVAL];

//---------------------  Debug ---------------------//

void DFS_DisplayCurrentPolicy(void) {
    #if DFS_LED_TEST == 1
    switch (policyCurrent) {
    case 0: LedGreenOff();   LedOrangeOff();  LedYellowOff(); break;
    case 1: LedGreenOn();    LedOrangeOff();  LedYellowOff(); break;
    case 2: LedGreenOff();   LedOrangeOn();   LedYellowOff(); break;
    case 3: LedGreenOn();    LedOrangeOn();   LedYellowOff(); break;
    case 4: LedGreenOff();   LedOrangeOff();  LedYellowOn(); break;
    case 5: LedGreenOn();    LedOrangeOff();  LedYellowOn(); break;
    case 6: LedGreenOff();   LedOrangeOn();   LedYellowOn(); break;
    default: LedGreenOn();   LedOrangeOn();   LedYellowOn(); break;
    }
    #endif // DFS_LED_TEST
}

void DFS_StartTimer(void) {
    TickType_t tickStepWidth;
    tickStepWidth = (policyCurrent == 0xFF) ? dfs_node[policyInitial].step_width : dfs_node[policyCurrent].step_width;
    if (DFS_TimerHandle == NULL) {
        DFS_TimerHandle = xTimerCreate("DFS_Timer", pdMS_TO_TICKS(tickStepWidth), pdFALSE, (void*)0, DFS_TimerCB);
        configASSERT(DFS_TimerHandle != NULL);
    }
    xTimerStart(DFS_TimerHandle, 0);
}

static void DFS_TimerCB(TimerHandle_t DFS_TimerHandle)
{
    S3x_Stepup_Policy();
    DFS_UpdateTimer();    
}

static void DFS_UpdateTimer(void) {
    TickType_t tickStepWidth;
    
    tickStepWidth = (policyCurrent == 0xFF) ? dfs_node[policyInitial].step_width : dfs_node[policyCurrent].step_width; // Update step width to current policy
    tickStepWidth = (tickStepWidth == 0) ? 1000 : tickStepWidth;
    xTimerChangePeriod( DFS_TimerHandle, tickStepWidth, 0 );
    xTimerStart(DFS_TimerHandle, 0);
    DFS_resetsleepticks();  // So CPU load matches step width
}

void s3x_suspend_dfs(void) {
    S3x_Set_Dfs_st(DFS_SUS);
}

void  s3x_set_min_allowed_policy (uint8_t min_policy)
{
    if (min_policy <= S3_dfs_max_index)
        dfs_min_policy = min_policy;
}

int S3x_Set_Policy(uint8_t policy,  uint32_t src, uint32_t* pcruval)
{
    int i, max;
    S3x_ClkD *clkd;
    int ret = -1;
    uint32_t max_rate = 0;
    uint8_t *clkd_index = (policy > policyCurrent) ? stepup_clkd_index : stepdown_clkd_index;

    if ((policy  <= S3_dfs_max_index) &&  (policyCurrent != policy))
    {
        S3x_Policy_Node dfs_policy = dfs_node[policy];
        max = SIZEOF_ARRAY(dfs_policy.clk_domain);
        for(i = 0; i < max ; i++){
            //if (i == 1) HAL_GPIO_Write(GPIO_1, 0);
            clkd = S3x_Id_To_Domain(dfs_policy.clk_domain[clkd_index[i]]);
            ret = s3x_update_clk_rate(clkd, dfs_policy.rate[clkd_index[i]], src, pcruval);
            //TIM 24 if (pcruval) CRU_WVAL(0x000, *pcruval); //TIM 24
            if (ret < 0) {
                break;
            }   

            if(clkd->expected_rate){
                if (max_rate < clkd->expected_rate)
                    max_rate = clkd->expected_rate;
            }
            else{
                if (max_rate < clkd->curr_rate)
                    max_rate = clkd->curr_rate;
            }
        }

        if (i == max){
            policyCurrent = policy;
            if (s3x_dfs_enable != DFS_SUS && s3x_dfs_enable != DFS_RES) {
                DFS_UpdateTimer();
            }
            DFS_DisplayCurrentPolicy();     // Enabled by DFS_LED_TEST == 1
            policyMaxrate = (dfs_node[policyCurrent].minHSOSC > max_rate) ? dfs_node[policyCurrent].minHSOSC : max_rate;
            ret = 0;
        }
        else if (ret < 0 )
        {
            S3x_Set_Policy(policyCurrent, src, pcruval);
            dbg_str("DFS: SetPolicy failed\n");
        }
    }
    
    return ret;
}

/* get the current policy policy */
uint8_t S3x_Get_Curr_Policy_Index(void)
{
    return policyCurrent;
}

uint32_t S3x_get_max_rate_dfs_node(uint8_t policy)
{
    uint32_t rate = 0, i;

    for (i = 0; i <= CLKD_MAX_INDEX_PNODE; i++){
        if (dfs_node[policy].rate[i] > rate)
            rate = dfs_node[policy].rate[i];
    }
    return rate;
}

static int S3x_Stepup_Policy()
{
    uint8_t policy;
    uint32_t src_rate, max_prate;

    policy = policyCurrent;
    if (policy == 0xFF) {
        policy = policyInitial;
    } else {
        uint16_t    cpuload = DFS_cpuload();
        if ((cpuload < dfs_node[policy].cpuload_downthreshold) && (policy > 0)) {
            policy --;
        }  else if ((dfs_node[policy].cpuload_upthreshold == 0) && (cpuload > 98) && (policy < S3_dfs_max_index)) { // Should make work with older pwrcfg.c where cpuload_upthreshold defaults to 0
            policy++;
        } else if ((cpuload > dfs_node[policy].cpuload_upthreshold) && (policy < S3_dfs_max_index)) {
            policy++;
        }
    }

    src_rate = S3x_Clkd_Get_Hsosc_Rate();
    max_prate = S3x_get_max_rate_dfs_node(policy);

    if (src_rate < max_prate)
     S3x_Clkd_Change_Hsosc(max_prate);
    src_rate = S3x_Clkd_Get_Hsosc_Rate();
    S3x_Set_Policy(policy, src_rate, NULL);
    return policy;
}

void DFS_AlignPolicy(uint8_t policy)
{
    uint32_t src_rate, max_prate;
    
    if (policy == 0xFF) //TIM why this?
        policy = policyInitial;

    src_rate = S3x_Clkd_Get_Hsosc_Rate();
    max_prate = S3x_get_max_rate_dfs_node(policy);

    if (src_rate < max_prate)
     S3x_Clkd_Change_Hsosc(max_prate);
    src_rate = S3x_Clkd_Get_Hsosc_Rate();
    S3x_Set_Policy(policy, src_rate, NULL);
}


void S3x_Reduce_Hsosc(uint32_t rate)
{
    uint32_t curr, diff, srate, cpu;
    //curr = S3x_Clkd_Get_Hsosc_Rate();
    
    do {
        /*Temp Bump up to speed up execution of clock divider updation routine
          ByPass C10 divider to rum M4 at Hsosc rate
          Set C01/C09 to run at half the C10 rate
          (We should not bypass them to run beyond hardware spec)*/
        cpu = CRU_RVAL(0);
        CRU_WVAL(0, 0);
//        CRU_WVAL(0x110, 0x11);
//        CRU_WVAL(0x114, 0x11);
        
        curr = S3x_Clkd_Get_Hsosc_Rate();
        diff = curr - rate;
        if ((curr <= rate) ||
                (curr <= s3x_clkd_get_active_HSOSC_qos()))
        {
            //CRU_WVAL(0, cpu);
            //CRU_WVAL(0x110, 0x11);
            //CRU_WVAL(0x114, 0x11);
            break;
        }
        else if(diff <= HSOSC_STEP_WIDTH)
        {
            S3x_Clkd_Change_Hsosc(rate);
            break;
        }
        srate = curr - HSOSC_STEP_WIDTH;
        S3x_Clkd_Change_Hsosc(srate);
    } while (srate > rate);

}

int S3x_Set_Lowest_Policy(void)
{
    uint32_t hsosc_rate;
    
    hsosc_rate = S3x_Clkd_Get_Hsosc_Rate();
    S3x_Set_Policy(dfs_min_policy, hsosc_rate, NULL);

    if (policyMaxrate < hsosc_rate)
    {
       S3x_Reduce_Hsosc(policyMaxrate);
    }
    return STATUS_OK;
}
bool    DFS_fPolicyHasDeepSleep(void) {
    return ((dfs_node[policyCurrent].policySleep != 0xFF));
}

/*
 * CPU Clock Frequency during sleep
 */
uint32_t DFS_FreqCPUSleep(void) {
    uint8_t policy;
    int     i;
    policy = (dfs_node[policyCurrent].policySleep == 0xFF) ? policyCurrent : dfs_node[policyCurrent].policySleep;
    for (i = 0; i <= CLKD_MAX_INDEX_PNODE; i++){
        if (dfs_node[policy].clk_domain[i] == CLK_C10) {
            break;
        }
    }
    return (dfs_node[policy].rate[i]);
}

/*
 * Switch to Sleep Policy
 * Used by LPM when the CPU can sleep for a longish time
 */
void DFS_SwitchToSleepPolicy(void)
{    
    uint32_t    cruReg0x000SleepVal;

    policyBeforeSleep = policyCurrent;
    policyDuringSleep = (dfs_node[policyCurrent].policySleep == 0xFF) ? policyCurrent : dfs_node[policyCurrent].policySleep;
    
    // Save current HSOSC and major clock settings as required
    freqHsoscBeforeSleep = S3x_Clkd_Get_Hsosc_Rate();
    if (policyDuringSleep == 0) {
        cruReg0x000 = CRU_RVAL(0);
        cruReg0x110 = CRU_RVAL(0x110);
        cruReg0x114 = CRU_RVAL(0x114);
    }
    
    S3x_Set_Policy(policyDuringSleep, freqHsoscBeforeSleep, &cruReg0x000SleepVal);
    CRU_WVAL(0x000, cruReg0x000SleepVal); //TIM 25
    //HAL_GPIO_Write(GPIO_1, 0);
    if (policyMaxrate < freqHsoscBeforeSleep) {
       //S3x_Reduce_Hsosc(policyMaxrate, acruval2, &icruval2);
        S3x_Reduce_Hsosc(policyMaxrate);
    }
    //HAL_GPIO_Write(GPIO_1, 1);
}

void DFS_RestoreFromSleep(void)
{
    int policy;
//TIM26
//    if (policyCurrent == 0) {
//        // Restore major clock and HSOC settings
//        CRU_WVAL(0x114, cruReg0x114);
//        CRU_WVAL(0x110, cruReg0x110);
        CRU_WVAL(0x000, cruReg0x000);
//        S3x_Clkd_Change_Hsosc(freqHsoscBeforeSleep);
//    }

    policy = (policyBeforeSleep == 0xFF) ? policyInitial : policyBeforeSleep;
    DFS_AlignPolicy(policy);
    S3x_Set_Dfs_st(DFS_EN);
    DFS_DisplayCurrentPolicy();     // Enabled by DFS_LED_TEST == 1
}


int S3x_Set_Max_Policy()
{
    uint32_t hsosc_rate;
    hsosc_rate = S3x_Clkd_Get_Hsosc_Rate();
    S3x_Set_Policy(S3_dfs_max_index, hsosc_rate, NULL);
    return STATUS_OK;
}

int s3x_policy_change(S3x_Policy_level level)
{
    int ret;
     if (level ==  STEP_UP)
         ret = S3x_Stepup_Policy();
     else if (level ==  LOWEST_VAL)
         ret = S3x_Set_Lowest_Policy();
     else if (level ==  MAX_VAL)
         ret = S3x_Set_Max_Policy();

     return ret;
}


/* handle dfs algo */
void S3x_Dfs_Handler(void)
{
    S3x_Stepup_Policy();
}

uint8_t s3x_get_index_in_dfs_node (uint8_t domain_id)
{
    uint8_t i;
    for (i = 0; i <= CLKD_MAX_INDEX_PNODE; i++){
        if (dfs_node[0].clk_domain[i] == domain_id)
            break;
    }

    return i;
}

/* this will return STATUS_OK in case domain id is not in policy list or
*  in case dfs is not activated, in case dfs is active and
* domain id is listed in policy table it will give the rate
* of current policy policy
*/
uint32_t s3x_get_policy_max_rate (uint8_t domain_id)
{
    uint8_t i;
    uint32_t ret = 0;

    if (policyCurrent != 0xFF) {
      i = s3x_get_index_in_dfs_node(domain_id);
      if (i <= CLKD_MAX_INDEX_PNODE)
          ret = dfs_node[policyCurrent].rate[i];
    }

    return ret;
}

uint8_t inline S3x_Is_Dfs_En(void)
{
    return (s3x_dfs_enable == DFS_EN ? 1 : 0);
}

void inline S3x_Set_Dfs_st(S3x_DFS_ST st)
{
        s3x_dfs_enable = st;
}

uint8_t DFS_Get_Curr_Policy(void)
{
    return policyCurrent;
}

void DFS_Initialize(void) {
    policyCurrent = policyInitial;
}

void s3x_dfs_init(void)
{
#ifdef S3X_ENABLE_DFS
    s3x_dfs_enable = DFS_EN;
#else
    s3x_dfs_enable = DFS_DIS;
#endif
}

void DFS_Enable(void) {
    s3x_dfs_enable = DFS_EN;
}

void s3x_disable_dfs(void) {
    s3x_dfs_enable = DFS_DIS;
}

void print_dfs_policies(void)
{
  printf("DFS:%d,%d,%d:", policyCurrent, policyBeforeSleep, policyDuringSleep);
  //printf(":%d,%d::", policyMaxrate, dfs_min_policy);
  printf(":%d::", s3x_dfs_enable);
}
