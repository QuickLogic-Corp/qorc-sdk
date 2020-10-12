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
 *    File   : ql_stackwatermark.c
 *    Purpose: This file has stack monitor utilities 
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"

/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "timers.h"
#include "task.h"

//uxTaskGetStackHighWaterMark function is available only
//if it is included in FreeRTOSConfig.h 
#if (INCLUDE_uxTaskGetStackHighWaterMark == 1)

#define MAX_WATERMARK_CHECK_PERIOD_MS  (1000) //1000ms
#ifndef MAX_STACK_MONITOR_COUNT
//define a fixed value if User has not defined it
#define MAX_STACK_MONITOR_COUNT   (100) //= 100 sec
#endif

static TaskHandle_t *stack_task_array = NULL;
static int task_array_size = 0;
static int watermak_assert_on = 0;
static TimerHandle_t stack_watermark_timer = NULL;
static int stack_timer_state = 0;
static int stack_timer_count = 0; 
/* This function can be used directly by passing the task handles in an array 
   When assert_on is set, if the High Water Mark becomes 0, then it asserts.
   If assert_on is not set, the High Water Mark values (in words) are printed.
*/
void check_stack_watermarks(TaskHandle_t *task_array, int array_size, int assert_on)
{
  if(array_size <= 0)
    return;
  
  printf("\nStackHighWaterMarks (in words, 0 is stack overflow): ");
  for(int i=0; i < array_size; i++)  {
    UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(*task_array++);
    if(assert_on) {
      configASSERT(uxHighWaterMark);
    } else {
      printf(" %d ", uxHighWaterMark);
    }
  }
  printf("\n");

  return;
}
/* By default the Timer based monitor works only for fixed amount of time.
   But User can call this function any time to disable it, if the timer
   value is defined outside this module.
*/
void disable_stack_watermark_timer(void)
{
  stack_timer_state = 0;
}
/* Periodically check the task stack high water marks. If assert is enabled do 
   assert for the first task in the array that falls to zero available stack 
*/
static void stack_watermark_timer_callback(TimerHandle_t xTimer)
{
  check_stack_watermarks(stack_task_array, task_array_size, watermak_assert_on);
  stack_timer_count++;
  if(stack_timer_count > MAX_STACK_MONITOR_COUNT)
  {
    disable_stack_watermark_timer();
  }
    
  if(stack_timer_state == 0)
  {
    xTimerStop(stack_watermark_timer, pdMS_TO_TICKS(0));
    printf("\n\nStopped Stack Monitor after %d seconds \n", stack_timer_count);
  }

  return;
}
/* This functions sets up a timer to check the stack high water marks periodically.
   This function can be used directly by passing the task handles in an array 
   When assert_on is set, if the High Water Mark becomes 0, then it asserts.
   If assert_on is not set, the High Water Mark values (in words) are printed.
*/
TimerHandle_t set_stack_watermark_timer(TaskHandle_t *task_array, int array_size, int assert_on)
{
  stack_task_array = task_array;
  task_array_size = array_size;
  watermak_assert_on = assert_on;
  
  stack_watermark_timer = xTimerCreate("stack_watermark_timer",
                                       pdMS_TO_TICKS(MAX_WATERMARK_CHECK_PERIOD_MS),
                                       pdTRUE,
                                       (void*)0,
                                       stack_watermark_timer_callback);
  if(stack_watermark_timer != NULL)
  {
    xTimerStart(stack_watermark_timer, pdMS_TO_TICKS(0));
    stack_timer_state = 1;
    stack_timer_count = 0;
  }

  return stack_watermark_timer;
}


#define MAX_NUM_TASKS  (20)
static TaskHandle_t task_watermark_array[MAX_NUM_TASKS];
/* This function is simplest way to monitor the stack high water marks.
   If the timer_on is set, then it will check period else prints the
   current water mark values for all the tasks in the sytem and returns.
   When assert_on is set, if the High Water Mark becomes 0, then it asserts.
   If assert_on is not set, the High Water Mark values (in words) are printed.
*/
void set_task_stack_watermark_monitor(int timer_on, int assert_on)
{
  TaskStatus_t tasks_state[MAX_NUM_TASKS]; //this goes onto stack. so should be smaller than the calling task stack
  
  vTaskDelay(10);
  /* first get the total number of tasks created/running */
  int n_tasks = uxTaskGetNumberOfTasks();
  if(n_tasks > MAX_NUM_TASKS)
    n_tasks = MAX_NUM_TASKS; 
  
  /* print the remaining heap size */
  int remaining_heap = xPortGetFreeHeapSize();
  int used_heap = configTOTAL_HEAP_SIZE - remaining_heap; 
  printf("\n\n Max Heap Size = %d, Used Heap is %d, remaining = %d \n", configTOTAL_HEAP_SIZE, used_heap, remaining_heap);
  
  /* then get raw status information for each task */
  n_tasks = uxTaskGetSystemState( tasks_state, n_tasks, NULL);
  printf("\n\n     TaskName         TaskID  State Priority  StackHighWaterMark\n");
  for(int i=0; i < n_tasks; i++)  {
    TaskStatus_t *t = &tasks_state[i];
    task_watermark_array[i] = t->xHandle;
    printf("%18s, %5d, %5d, %5d,    %5d \n", t->pcTaskName, t->xTaskNumber, t->eCurrentState,
                                t->uxCurrentPriority, t->usStackHighWaterMark);
  }
#if 1 //1=print the state interpretation info
  /* from the enumerated state eTaskState defined in task.h */
  printf("\nThe Task states are - \n");
  printf("   0 = eRunning\n");
  printf("   1 = eReady\n");
  printf("   2 = eBlocked\n");
  printf("   3 = eSuspended\n");
  printf("   4 = eDeleted\n");
  printf("   5 = eInvalid\n");
#endif 
  
  /* setup a timer based check for the stack water marks */
  if (timer_on)
    set_stack_watermark_timer(task_watermark_array, n_tasks, assert_on); //if no assert, but prints 
/* else
    check_stack_watermarks(task_watermark_array, n_tasks, assert_on); */ 
  
  return;
}

#endif /* INCLUDE_uxTaskGetStackHighWaterMark */