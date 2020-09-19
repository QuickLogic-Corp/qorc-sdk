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
 *    File   : ql_stackwatermark.h
 *    Purpose: This file has APIs for stack monitor utilities 
 *                                                          
 *=========================================================*/

#ifndef QL_STACKWATERMARK_H
#define QL_STACKWATERMAKR_H

#include "FreeRTOS.h"
#include "timers.h"

/* This function can be used directly by passing the task handles in an array 
   When assert_on is set, if the High Water Mark becomes 0, then it asserts.
   If assert_on is not set, the High Water Mark values (in words) are printed.
*/
extern void check_stack_watermarks(TaskHandle_t *task_array, int array_size, int assert_on);

/* By default the Timer based monitor works only for fixed amount of time.
   But User can call this function any time to disable it, if the timer
   value is defined outside this module.
*/
extern void disable_stack_watermark_timer(void);

/* This functions sets up a timer to check the stack high water marks periodically.
   This function can be used directly by passing the task handles in an array 
   When assert_on is set, if the High Water Mark becomes 0, then it asserts.
   If assert_on is not set, the High Water Mark values (in words) are printed.
*/
extern TimerHandle_t set_stack_watermark_timer(TaskHandle_t *task_array, int array_size, int assert_on);

/* This function is simplest way to monitor the stack high water marks.
   If the timer_on is set, then it will check period else prints the
   current water mark values for all the tasks in the sytem and returns.
   When assert_on is set, if the High Water Mark becomes 0, then it asserts.
   If assert_on is not set, the High Water Mark values (in words) are printed.
*/
extern void set_task_stack_watermark_monitor(int timer_on, int assert_on);



#endif /* QL_STACKWATERMAKR_H */
