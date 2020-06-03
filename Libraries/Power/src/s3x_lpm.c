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
 *    File   : s3x_lpm.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"
#include <string.h>
#include "stdbool.h"
#include "eoss3_hal_gpio.h"
#include "s3x_lpm.h"
#include "s3x_pi.h"
#include "s3x_dfs.h"
#include "s3x_cpuload.h"
#include "dbg_uart.h"
#define POST_OFFSET 700
#define DWT_MAX_CNT 0xFFFFFFFF
#define MIN_ALLOW_TICK 3
#if (CONST_FREQ == 0)
volatile static uint32_t *DWTCYCCNT = (volatile uint32_t *)0xE0001004;
#endif
lpm_node *lpm_hnd = NULL;

void default_sram_in_lpm(void);
#ifdef GCC_MAKE
extern void s3x_sram_in_lpm	(void)__attribute__ ((weak, alias("default_sram_in_lpm")));
#else
extern void s3x_sram_in_lpm (void);
#pragma weak 	s3x_sram_in_lpm	 = default_sram_in_lpm
#endif


static void  S3x_add_lpm_cb_node (lpm_node *node)
{
    if (!lpm_hnd)
    {
        lpm_hnd = node;
    }
    else
    {
        lpm_node *temp = lpm_hnd;
        while (temp->next){
            temp =  temp->next;
        }
        temp->next = node;
    }

}
void S3x_Register_Lpm_Cb(Dev_Lpm_Cb lpm_cb, char *name)
{
    lpm_node *node;
    if (lpm_cb)
    {
         node = pvPortMalloc(sizeof(lpm_node));
         if (node != NULL)
         {
             node->pm_cb = lpm_cb;
             node->next = NULL;
             strncpy(node->name, name, sizeof(node->name));
             S3x_add_lpm_cb_node(node);
          }
    }

}

///* PMU resolution is ~2ms */
//static uint16_t pmutickCurrent;
//void PMU_Set_Timer(UINT16_t pmu_tick)
//{
//    pmutickCurrent = (pmu_tick & 0x3FF);
//    PMU->PMU_TIMER_CFG_0 = (UINT32_t )(pmu_tick & 0x3FF);
//    PMU->PMU_TIMER_CFG_1 = 0x1;
//    INTR_CTRL->OTHER_INTR_EN_M4 |= PMU_TMR_INTR_DETECT;
//    NVIC_ClearPendingIRQ(PmuTimer_IRQn);
//    //NVIC_SetPriority(PmuTimer_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
//    NVIC_EnableIRQ(PmuTimer_IRQn);
//}
//
//void PMU_Timer_Handler(void)
//{
//    static bool f = false;
//     if (f) {
//        f = false;
//        HAL_GPIO_Write(GPIO_5, 0);
//    } else {
//        f = true;
//        HAL_GPIO_Write(GPIO_5, 1);
//    }
//    INTR_CTRL->OTHER_INTR_EN_M4 &= ~(PMU_TMR_INTR_DETECT);
//    INTR_CTRL->OTHER_INTR &= PMU_TMR_INTR_DETECT;
//    //NVIC_ClearPendingIRQ(PmuTimer_IRQn);
//    PMU_Set_Timer(pmutickCurrent);
//    s3x_dfs_Tick_Handler();
//}

//void PMU_Disable_Timer(void)
//{
//    INTR_CTRL->OTHER_INTR_EN_M4 &= ~(PMU_TMR_INTR_DETECT);
//    PMU->PMU_TIMER_CFG_1 = 0x0;
//    PMU->PMU_TIMER_CFG_0 = 0x0;
//    NVIC_DisableIRQ(PmuTimer_IRQn);
// }

void s3x_setup_systick_interrupt(uint32_t ticks, uint32_t offset)
{
    uint32_t MaxTicks, OneTickCount, loadValue;

    OneTickCount = ( configCPU_CLOCK_HZ / configTICK_RATE_HZ );
    MaxTicks = portMAX_24_BIT_NUMBER / OneTickCount;
    if (ticks > MaxTicks)
      ticks = MaxTicks;
    loadValue = (OneTickCount * ticks);
    if (offset && (offset < loadValue))
    {
        loadValue -= offset;
    }
    portNVIC_SYSTICK_CTRL_REG &= ~(portNVIC_SYSTICK_ENABLE_BIT
                                | portNVIC_SYSTICK_INT_BIT);
    portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;
    portNVIC_SYSTICK_LOAD_REG = loadValue;
    portNVIC_SYSTICK_CTRL_REG |= (portNVIC_SYSTICK_ENABLE_BIT
                            | portNVIC_SYSTICK_INT_BIT);
}

void prvStopTickInterruptTimer(void)
{
    uint32_t ulSysTickCTRL;
    ulSysTickCTRL = portNVIC_SYSTICK_CTRL_REG;
    portNVIC_SYSTICK_CTRL_REG = (ulSysTickCTRL &
                (~(portNVIC_SYSTICK_ENABLE_BIT | portNVIC_SYSTICK_INT_BIT)));
}

void prvStartTickInterruptTimer(uint32_t offset)
{
   s3x_setup_systick_interrupt(1, offset);
}

void CPU_Tick_Handler(void)
{
    xPortSysTickHandler();
    s3x_setup_systick_interrupt(1, 0);
}
/*
 * Setup the systick timer to generate the tick interrupts at the required
 * frequency.
 */
void vPortSetupTimerInterrupt( void )
{
    prvStartTickInterruptTimer(0);
}

#if configUSE_TICKLESS_IDLE == 1

int S3x_Pre_Lpm (TickType_t IdleTime )
{
    int ret = 0;
    lpm_node *node  = lpm_hnd;
    while (node) {
        ret = node->pm_cb(ENTER_LPM);
        if (ret)
        {
            //QL_LOG_ERR_150K("LPM failed due to %s\n", node->name);
            break;
        }
        node = node->next;
    }

    return ret;
}
void S3x_Post_Lpm (TickType_t IdleTime)
{
    lpm_node *node  = lpm_hnd;
    while (node) {
        node->pm_cb(EXIT_LPM);
        node = node->next;
    }
}

/*
 * Murthy went thru the code and identified an
 * issue that is fixed.  We believe that was
 * the defect. We need to see if it happens again.
 *
 * There is a VERY HARD to capture assert 
 * that sometimes occurs when the DCL starts/stops 
 * recording to SD card, see around line 451
 *
 * This is temporary debug code so that
 * we can capture the sequence of events and code-flow
 * decisions that lead up to the assert occuring.
 *
 * If the assert occurs - you can look at this
 * trace array and see values of variables at
 * various points in time
 *
 * The "lineno" tells you the flow and decisions
 * made during the call to the sleep code.
 */
struct sleep_trace_entry {
    int lineno;
    uint64_t value;
#define TRACE_LEN 64
} trace_history[ TRACE_LEN ];
static int trace_idx;

static void trace_this( int lineno, uint32_t value )
{
    if( trace_idx >= TRACE_LEN ){
        return;
    }
    trace_history[trace_idx].lineno = lineno;
    trace_history[trace_idx].value = value;
    trace_idx++;
}

void S3x_Lpm_Handler( TickType_t xExpectedIdleTime )
{
    uint32_t ulReloadValue,ulSysTickCTRL;
    uint32_t cpu_rate, OneTick, xMaxTicks, PrevOneTick;
#if (CONST_FREQ == 0)
    uint32_t CycleCompensation;
    uint64_t start_cnt, end_cnt;
#endif
    uint64_t temp1, temp2;
    uint32_t div_reg, TickComp = 0, offset = 0;
    uint32_t ulCompleteTickPeriods;
    TickType_t xModifiableIdleTime;
    uint32_t usCompensation = 0;
    uint8_t restore = 0;
    bool    fNeedToRestoreDFS = false;

    /* RESET the trace. */
    trace_idx = 0;
    trace_this( __LINE__, xExpectedIdleTime );

 if( xExpectedIdleTime <= MIN_ALLOW_TICK )
    {
        return;
    }
    /* Stop the timer that is generating the tick interrupt. */
    prvStopTickInterruptTimer();
 #if (CONST_FREQ == 0)
    start_cnt = *DWTCYCCNT;
#endif
    PrevOneTick = portNVIC_SYSTICK_LOAD_REG;   // # counts per tick for us is MHz/1000 since ticks are 1ms
    trace_this( __LINE__, PrevOneTick );
    offset = PrevOneTick - portNVIC_SYSTICK_CURRENT_VALUE_REG;  // offset is number of clocks from start of tick to now
    trace_this( __LINE__, offset );
    __disable_interrupt();

    /* Ensure it is still ok to enter the sleep mode. */
    if( eTaskConfirmSleepModeStatus() == eAbortSleep )
    {
        /* A task has been moved out of the Blocked state since this macro was
        executed, or a context siwth is being held pending.  Do not enter a
        sleep state.  Restart the tick and exit the critical section. */
        trace_this(__LINE__,1);
         prvStartTickInterruptTimer(offset);
        __enable_interrupt();
    }
    else if (configPRE_SLEEP_PROCESSING( xExpectedIdleTime ))
    {
        trace_this(__LINE__,2);
         prvStartTickInterruptTimer(offset);
        __enable_interrupt();
    }
    else
    {
#if (CONST_FREQ == 0)// Define this macro to disable QoS, dfs and lpm.
        if (!(S3x_Is_Dfs_En()))
        {
        trace_this(__LINE__,3);

            cpu_rate = s3x_clkd_get_init_rate(S3X_M4_S0_S3_CLK);
            if (cpu_rate > 0)
            {
                trace_this(__LINE__,4);
                _S3x_Clk_Set_Rate(S3X_M4_S0_S3_CLK, HSOSC_4MHZ);
                S3x_Clkd_Change_Hsosc(HSOSC_36MHZ);
                restore = 1;
            } else {
                trace_this(__LINE__,5);
            }
        }
        else
        {
                trace_this(__LINE__,6);
            /* Stop dfs */
           
            end_cnt = (*DWTCYCCNT);
            if (end_cnt > start_cnt)
                CycleCompensation = (end_cnt - start_cnt);
            else
                CycleCompensation = (DWT_MAX_CNT - start_cnt) + end_cnt;
            trace_this(__LINE__,CycleCompensation);
            CycleCompensation += offset;
            offset = 0;
            usCompensation += (CycleCompensation * 1000) / PrevOneTick;
            if (DFS_fPolicyHasDeepSleep()) {
                s3x_suspend_dfs();
                DFS_SwitchToSleepPolicy();
                fNeedToRestoreDFS = true;
            }
        }
#endif
        usCompensation += POST_OFFSET;
        trace_this(__LINE__,usCompensation);
        if (usCompensation > 1000)
        {
            TickComp = (usCompensation / 1000);
            usCompensation -=  (TickComp * 1000);
        }
        OneTick = ( configCPU_CLOCK_HZ / configTICK_RATE_HZ );
        trace_this(__LINE__,OneTick);
        div_reg = CRU_RVAL(0);
        CRU_WVAL(0, 0);

        xMaxTicks = portMAX_24_BIT_NUMBER / OneTick;
        trace_this(__LINE__,xMaxTicks);
        trace_this(__LINE__,xExpectedIdleTime);

        /* Make sure the SysTick reload value does not overflow the counter. */
        if( xExpectedIdleTime > xMaxTicks )
        {
            xExpectedIdleTime = xMaxTicks;
        }

        /* Calculate the reload value required to wait xExpectedIdleTime
        tick periods.  -1 is used because this code will execute part way
        through one of the tick periods. */
        //ulReloadValue = OneTick * ( xExpectedIdleTime - 1);
        ulReloadValue = OneTick * ( xExpectedIdleTime - TickComp);
        trace_this(__LINE__,ulReloadValue);
        ulReloadValue -=POST_OFFSET;
        trace_this(__LINE__,ulReloadValue);
        if (ulReloadValue < OneTick)
        {
            trace_this(__LINE__,0);
            //ulCompleteTickPeriods = 1 + TickComp;
            ulCompleteTickPeriods = 1;
            goto exit;
        }


        trace_this(__LINE__,0);
        portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;
        /* Set the new reload value. */
        portNVIC_SYSTICK_LOAD_REG = ulReloadValue;
        CRU_WVAL(0, div_reg);
        /* Clear the SysTick count flag and set the count value back to
        zero. */
        /* Restart SysTick. */
        portNVIC_SYSTICK_CTRL_REG |= (portNVIC_SYSTICK_ENABLE_BIT
                                    | portNVIC_SYSTICK_INT_BIT);

        /* Sleep until something happens.  configPRE_SLEEP_PROCESSING() can
        set its parameter to 0 to indicate that its implementation contains
        its own wait for interrupt or wait for event instruction, and so wfi
        should not be executed again.  However, the original expected idle
        time variable must remain unmodified, so a copy is taken. */
        xModifiableIdleTime = xExpectedIdleTime;
        if( xModifiableIdleTime > 0 )
        {
	        trace_this(__LINE__,0);
#if defined(HOST_VOICE) || defined(HOST_SENSOR)
             __DSB();
             __ISB();
            s3x_sram_in_lpm();
#endif
            __DSB();
            __ISB();
            __WFI();
            __ISB();
        }
        trace_this(__LINE__,0);

        /* Stop SysTick.  Again, the time the SysTick is stopped for is
        accounted for as best it can be, but using the tickless mode will
        inevitably result in some tiny drift of the time maintained by the
        kernel with respect to calendar time. */
        ulSysTickCTRL = portNVIC_SYSTICK_CTRL_REG;
        portNVIC_SYSTICK_CTRL_REG = ( ulSysTickCTRL & ~portNVIC_SYSTICK_ENABLE_BIT );
        /* temporary bumpup to exit faster */
        //CRU_WVAL(0, 0);
        //CRU_WVAL(0x110, 0x10);
        //CRU_WVAL(0x114, 0x10);

        if( ( ulSysTickCTRL & portNVIC_SYSTICK_COUNT_FLAG_BIT ) != 0 )
        {
            /* The tick interrupt handler will already have pended the tick
            processing in the kernel.  As the pending tick will be
            processed as soon as this function exits, the tick value
            maintained by the tick is stepped forward by one less than the
            time spent waiting. */
            ulCompleteTickPeriods = xExpectedIdleTime - 1;
            trace_this(__LINE__,ulCompleteTickPeriods);
        }
        else
        {
            /* Something other than the tick interrupt ended the sleep.
            Work out how long the sleep lasted rounded to complete tick
            periods (not the ulReload value which accounted for part
            ticks). */
            portNVIC_SYSTICK_CTRL_REG = ( ulSysTickCTRL & ~portNVIC_SYSTICK_ENABLE_BIT );

            /* How many complete tick periods passed while the processor
            was waiting? */
            //temp1 = xExpectedIdleTime - (1 + TickComp);
            temp1 = xExpectedIdleTime - (TickComp);
            trace_this( __LINE__, temp1 );
            temp2 = OneTick;
            temp1 = temp1 * temp2;
            offset = 0;
            trace_this( __LINE__, portNVIC_SYSTICK_CURRENT_VALUE_REG );
            if (temp1 < portNVIC_SYSTICK_CURRENT_VALUE_REG)
            {
                ulCompleteTickPeriods = xExpectedIdleTime - 1;
                trace_this(__LINE__,ulCompleteTickPeriods);
            }
            else
            {
                temp2 = temp1 - portNVIC_SYSTICK_CURRENT_VALUE_REG;
                trace_this( __LINE__, temp2 );
                trace_this( __LINE__, OneTick );
                ulCompleteTickPeriods = temp2 / OneTick;
                trace_this( __LINE__, ulCompleteTickPeriods );
                if (!ulCompleteTickPeriods)
                {
                trace_this(__LINE__,ulCompleteTickPeriods);
                    ulCompleteTickPeriods = TickComp;
                    offset  = OneTick - temp2;
                    offset += usCompensation;
                    if (offset > OneTick )
                    {
                        ulCompleteTickPeriods++;
                    }
                }
                else if (ulCompleteTickPeriods < xExpectedIdleTime)
                {
                trace_this(__LINE__,ulCompleteTickPeriods);
                    ulCompleteTickPeriods += TickComp;
                    if (ulCompleteTickPeriods < xExpectedIdleTime)
                    {
                        offset  = ((ulCompleteTickPeriods + 1) * OneTick) - temp2;
                        if (offset > (OneTick / 2) )
                        {
                            ulCompleteTickPeriods++;

                        }
                    }
                }
                trace_this(__LINE__,ulCompleteTickPeriods);

        }


        }

exit:
        if (fNeedToRestoreDFS)
        {
            trace_this(__LINE__,0);
            S3x_Set_Dfs_st(DFS_RES);
            DFS_RestoreFromSleep();
            //S3x_Dfs_Handler();
        }
        else if (restore)
            _S3x_Clk_Set_Rate(S3X_M4_S0_S3_CLK, cpu_rate);

        configPOST_SLEEP_PROCESSING( xExpectedIdleTime );
        DFS_updatesleepticks(ulCompleteTickPeriods);

      /* Re-enable interrupts - see comments above __disable_interrupt()
        call above. */
        __enable_interrupt();

        portENTER_CRITICAL();
        {
            trace_this(__LINE__,ulCompleteTickPeriods);
            trace_this(__LINE__,xExpectedIdleTime);
            /* This assert *SOMETIMES* occurs it is hard to capture */
            /* TIM 2020-04-04 Believe we have fixed this */
            // configASSERT( !(ulCompleteTickPeriods > xExpectedIdleTime));
            vTaskStepTick( ulCompleteTickPeriods );
            prvStartTickInterruptTimer(0);

        }
        portEXIT_CRITICAL();
        
        // Monitor unexpected situtation
            if (ulCompleteTickPeriods > xExpectedIdleTime)
            {
              dbg_str_int_noln("Warning LPM Ticks "__FILE__, __LINE__);
              dbg_str_int_noln(" Completed", ulCompleteTickPeriods);
              dbg_str_int(" > Expected", xExpectedIdleTime);
            }
 
    }
}

void default_sram_in_lpm(void)
{
    return;
}

#endif /* #if configUSE_TICKLESS_IDLE */
