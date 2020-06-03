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
*
*/

#ifndef QL_UTIL__H
#define QL_UTIL__H

#include <assert.h>
#include <stdio.h>
#include "dbg_uart.h"
#ifdef VOICE_DBG
#define QL_LOG_INFO(X,...)  printf(X,##__VA_ARGS__)
#define	QL_LOG_DBG(X,...)	//printf(X,##__VA_ARGS__)
#define	QL_LOG_ERR(X,...)	printf(X,##__VA_ARGS__)
#define	QL_LOG_WARN(X,...)	//printf(X,##__VA_ARGS__)
#define	QL_LOG_TEST(X,...)	//printf(X,##__VA_ARGS__)
#define	VLOG_SQA(X,...)	        //printf(X,##__VA_ARGS__)

#define ql_assert(x)            assert(x)

#else
#define QL_LOG_RESULT(X,...)      printf("%s"X,"  RESULT : ",##__VA_ARGS__)
#define QL_LOG_INFO(X,...)      printf("%s"X,"  INFO : ",##__VA_ARGS__)
#define	QL_LOG_WARN(X,...)	printf("%s"X,"  WARN : ",##__VA_ARGS__)
#define	QL_LOG_DBG(X,...)	//printf(X,##__VA_ARGS__)
#define	QL_LOG_ERR(X,...)	printf("%s"X,"  ERROR : ",##__VA_ARGS__)
#define	QL_LOG_TEST(X,...)	//printf(X,##__VA_ARGS__)
#define	VLOG_SQA(X,...)	        //printf(X,##__VA_ARGS__)

#define ql_assert(x)            configASSERT(x)
#endif

static inline int32_t calc_max_and_ave(int32_t my_array[], size_t size, float *ave, char *str) {
    /* enforce the contract */
    assert(my_array && size);
    size_t i;
    float sum = 0.0f;
    int32_t max_value = my_array[0];

    for (i = 1; i < size; ++i) {
        if ( my_array[i] > max_value ) {
            max_value = my_array[i];
        }
        sum += my_array[i];
    }
    *ave = sum/size;
    QL_LOG_INFO("MIPS : %s : max= %d, average = %f\n",str, max_value, *ave );
    return max_value;
}

#define CYCLE_MEASURE_OPTION_1

#ifdef CYCLE_MEASURE_OPTION_1
#define start_timer()    *((volatile uint32_t*)0xE0001000) = 0x40000001  // Enable CYCCNT register
#define stop_timer()   *((volatile uint32_t*)0xE0001000) = 0x40000000  // Disable CYCCNT register
#define get_timer()   *((volatile uint32_t*)0xE0001004)               // Get value from CYCCNT register


#else
/* DWT (Data Watchpoint and Trace) registers, only exists on ARM Cortex with a DWT unit */
/*!< DWT Control register */
#define KIN1_DWT_CONTROL             (*((volatile uint32_t*)0xE0001000))
/*!< CYCCNTENA bit in DWT_CONTROL register */
#define KIN1_DWT_CYCCNTENA_BIT       (1UL<<0)
/*!< DWT Cycle Counter register */
#define KIN1_DWT_CYCCNT              (*((volatile uint32_t*)0xE0001004))
/*!< DEMCR: Debug Exception and Monitor Control Register */
#define KIN1_DEMCR                   (*((volatile uint32_t*)0xE000EDFC))
/*!< Trace enable bit in DEMCR register */
#define KIN1_TRCENA_BIT              (1UL<<24)

/*!< TRCENA: Enable trace and debug block DEMCR (Debug Exception and Monitor Control Register */
#define KIN1_InitCycleCounter() \
KIN1_DEMCR |= KIN1_TRCENA_BIT
/*!< Reset cycle counter */
#define KIN1_ResetCycleCounter() \
KIN1_DWT_CYCCNT = 0
/*!< Enable cycle counter */
#define KIN1_EnableCycleCounter() \
KIN1_DWT_CONTROL |= KIN1_DWT_CYCCNTENA_BIT
/*!< Disable cycle counter */
#define KIN1_DisableCycleCounter() \
KIN1_DWT_CONTROL &= ~KIN1_DWT_CYCCNTENA_BIT
/*!< Read cycle counter register */
#define KIN1_GetCycleCounter() \
KIN1_DWT_CYCCNT

#if 0
KIN1_InitCycleCounter(); /* enable DWT hardware */
KIN1_ResetCycleCounter(); /* reset cycle counter */
KIN1_EnableCycleCounter(); /* start counting */
foo();
cycles = KIN1_GetCycleCounter(); /* get cycle counter */
KIN1_DisableCycleCounter(); /* disable counting if not used any more */
#endif
#endif  /* CYCLE_MEASURE_OPTION_1 */

#endif /* QL_UTIL__H */
