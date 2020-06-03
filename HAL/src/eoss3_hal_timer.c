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
 *    File   : eoss3_hal_timer.c
 *    Purpose: This file contains HAL API for M4 peripheral time
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include <test_types.h>
#include "eoss3_dev.h"
//#include "eoss3_hal_rcc.h"
#include "eoss3_hal_pad_config.h"
#include "eoss3_hal_timer.h"

#include "eoss3_hal_gpio.h"
#include "eoss3_hal_pad_config.h"
#include "s3x_clock_hal.h"
#include "s3x_clock.h"

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "RtosTask.h"

#define MHZ_1 (1000000)
#define MAX_TIMERS	(10)

char gTimerOn = 0;

struct HAL_Timer_t {
	uint32_t USecs;
	int32_t ReloadTicks, Ticks;
	uint32_t Flags;
	uint32_t InUse;
	uint32_t CallbackPending;
	void (*TimerCallback)(void *);
	void* Cookie;
};

enum TimerEventType {TIMER_EVENT_EXPIRED, TIMER_EVENT_START, TIMER_EVENT_STOP, TIMER_EVENT_THREAD_STOP};

struct TimerMsg {
	enum TimerEventType Event;
	struct HAL_Timer_t *timer;
	SemaphoreHandle_t sync;
};

static struct HAL_Timer_t HAL_Timer_List[MAX_TIMERS];
static HAL_Timer_t TimerToExpire;
static SemaphoreHandle_t lock;
static TaskHandle_t USecsTimerTaskHandle;
static QueueHandle_t TimerQueue = NULL;

enum TimerState{TIMER_UNUSED = 0, TIMER_INIT, TIMER_ACTIVE, TIMER_EXPIRED};

#define USECS_TIMER_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE )
#define TIMER_QUEUE_LENGTH	(MAX_TIMERS + 1)
#define err(...) do { printf("[HAL TIMER] [ERR] [%s] : ", __FUNCTION__); printf(__VA_ARGS__); } while(0)
#ifdef DEBUG
#define dbg(...) do { printf("[HAL TIMER] [DBG] [%s] : ", __FUNCTION__); printf(__VA_ARGS__); } while(0)
#else
#define dbg(...)
#endif

#define USEC_SCALING (15)

static inline void disable_timer(void)
{
	TIMER->CTRL &= ~(TIMER_INT_ENABLE | TIMER_ENABLE);
}

static inline void enable_timer(void)
{
	TIMER->CTRL |= (TIMER_INT_ENABLE | TIMER_ENABLE);
}

static inline void __enable_timer_irq(void)
{
	NVIC_SetPriority(Timer_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	NVIC_EnableIRQ(Timer_IRQn);
}


static inline void start_timer(int ticks)
{
	TIMER->RELOAD = ticks;
	TIMER->INTSTATUS_INTCLEAR = 0x1;
	INTR_CTRL->OTHER_INTR_EN_M4 |= TIMER_INTR_EN_M4;
	NVIC_ClearPendingIRQ(Timer_IRQn);
	__enable_timer_irq();
	enable_timer();
}

HAL_StatusTypeDef HAL_Timer_Init(void)
{
	S3x_Clk_Enable(S3X_M4_PRPHRL_CLK);
	return HAL_OK;
}

HAL_StatusTypeDef HAL_Delay_Init(void)
	{
	volatile uint32_t v, d = 10;

	CoreDebug->DEMCR &= ~0x01000000;
	CoreDebug->DEMCR |=  0x01000000;

	DWT->CTRL &= ~0x00000001;
	DWT->CTRL |=  0x00000001;

	DWT->CYCCNT = 0;
	v = DWT->CYCCNT;
	while(d--); /* do something here to increase the counter */
	if (DWT->CYCCNT - v)
		return HAL_OK;
	else
		return HAL_ERROR;
	}

void HAL_DelayUSec(uint32_t usecs)
{
	uint32_t start = DWT->CYCCNT;
	usecs *= (S3x_Clkd_Get_Cpu_Rate()/1000000);
	while ((DWT->CYCCNT - start) < usecs);
}

/*
 * reduce the ticks from every Active timer & if ticks is zero, mark it as expired.
 * also keep track of timer with lowest tick value (timer which has to be expired first) &
 * return the same
 * TBD : Change the name of this function
	 */
#define TIMER_PROCESSING_OVERHEAD		10
static HAL_Timer_t reduce_timer_ticks(uint32_t ticks)
	{
	HAL_Timer_t timer_with_lowest_ticks_to_expire = NULL;
	int i;
	for (i = 0 ; i < MAX_TIMERS ; i++) {
		if (HAL_Timer_List[i].InUse == TIMER_ACTIVE) {
			HAL_Timer_List[i].Ticks -= (ticks + TIMER_PROCESSING_OVERHEAD);
			if (HAL_Timer_List[i].Ticks <= 0) { // expire timers
				// For one shot
				if (HAL_Timer_List[i].Flags & HAL_TIMER_ONESHOT){
					HAL_Timer_List[i].Ticks = 0;
					HAL_Timer_List[i].CallbackPending = 1;
					HAL_Timer_List[i].InUse = TIMER_EXPIRED;
				}
				else {
					// periodic timer
					HAL_Timer_List[i].CallbackPending = 1;
					HAL_Timer_List[i].Ticks = HAL_Timer_List[i].ReloadTicks;
				}
			}

			// Now set for next timeout
			if ( HAL_Timer_List[i].Ticks ) {
				if (timer_with_lowest_ticks_to_expire) {	//TBD revisit to optimize
						if (timer_with_lowest_ticks_to_expire->Ticks > HAL_Timer_List[i].Ticks)
							timer_with_lowest_ticks_to_expire = &HAL_Timer_List[i];
					} else
						timer_with_lowest_ticks_to_expire = &HAL_Timer_List[i];
			}
		}
	}
	return timer_with_lowest_ticks_to_expire;
}

static void TriggerExpiredTimerCallbacks(void)
{
	int i;
	for (i = 0 ; i < MAX_TIMERS ; i++) {
		if (HAL_Timer_List[i].CallbackPending) {
			dbg("HAL_Timer_List[%d] Expired. Issuing Callback\n", i);
			if (HAL_Timer_List[i].TimerCallback)
				HAL_Timer_List[i].TimerCallback(HAL_Timer_List[i].Cookie);
	else
				err("HAL_Timer_List[%d].TimerCallback not set\n", i);
			HAL_Timer_List[i].CallbackPending = 0;
		}
	}
}

static void TimerStart(HAL_Timer_t TimerHandle)
	{
	uint32_t TicksRemaining;
	if (!TimerToExpire) {
		TimerToExpire = TimerHandle;
		TimerHandle->InUse = TIMER_ACTIVE;
		start_timer(TimerToExpire->Ticks);
	} else {
		/* if the remaining time to expire the current timer is greater than
		 * the new timer, ie the new timer will expire 1st, then load the
		 * Reload register with the new value, substract the current tick
		 * value from all the other existing timers, then restart the timer
		 */
		dbg("TOE - TIMER->VALUE = %x, TIMER->RELOAD = %x, TimerHandle->Ticks = %x\n", TIMER->VALUE, TIMER->RELOAD, TimerHandle->Ticks);
		// crtitical section
		__disable_interrupt();
		TicksRemaining = TIMER->VALUE;
		__enable_interrupt();
		// -- RIH
		if (TicksRemaining > TimerHandle->Ticks) {
			disable_timer();
			reduce_timer_ticks(TimerToExpire->Ticks - TicksRemaining);
			TimerToExpire = TimerHandle;
			start_timer(TimerToExpire->Ticks);
		} else {
			TimerHandle->Ticks += (TimerToExpire->Ticks - TicksRemaining);
		}
		TimerHandle->InUse = TIMER_ACTIVE;
	}
}

static void USecsTimerTask(void *a)
{
	uint32_t TimerThreadStop = 0, TicksRemaining = 0;
	BaseType_t ret;
	struct TimerMsg Msg;

	dbg("TimerTask Running\n");
	while(!TimerThreadStop) {
		ret = xQueueReceive(TimerQueue, &Msg, portMAX_DELAY);
		if( ret != pdPASS ) {
			err("Unable to receive Msg\n");
		}
		switch(Msg.Event)
	{
		case TIMER_EVENT_EXPIRED :
			if (TimerToExpire)
				TimerToExpire = reduce_timer_ticks(TimerToExpire->Ticks);
			else
				err("TimerToExpire not set\n");
			if (TimerToExpire)
				start_timer(TimerToExpire->Ticks);
			TriggerExpiredTimerCallbacks();
			break;
		case TIMER_EVENT_START :
			TimerStart(Msg.timer);
			xSemaphoreGive(Msg.sync);
			break;
		case TIMER_EVENT_STOP :
			Msg.timer->InUse = TIMER_EXPIRED;
			if (Msg.timer == TimerToExpire) {
				__disable_interrupt();
				TicksRemaining = TIMER->VALUE;
				__enable_interrupt();
				disable_timer();
				TimerToExpire = reduce_timer_ticks(TimerToExpire->Ticks - TicksRemaining);
				if (TimerToExpire)
					start_timer(TimerToExpire->Ticks);
			}
			xSemaphoreGive(Msg.sync);
			break;
		case TIMER_EVENT_THREAD_STOP :
			TimerThreadStop = 1;
			break;
		default:
			err("Unknown Event Received %x\n", Msg.Event);
			break;
		}
	}
}

void HAL_Init_Timer_Task(void)
{
	TimerQueue = xQueueCreate(15 ,sizeof(struct TimerMsg));
	configASSERT(TimerQueue);
    vQueueAddToRegistry(TimerQueue, "Hal_Timer" );
	xTaskCreate(USecsTimerTask, "USecs TimerTask", USECS_TIMER_TASK_STACK_SIZE, NULL, PRIORITY_HAL_TIMER, &USecsTimerTaskHandle);
	configASSERT(USecsTimerTaskHandle);
}

void HAL_Timer_ISR(void)
{
//	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	struct TimerMsg Msg;
	TIMER->CTRL &= ~(TIMER_INT_ENABLE | TIMER_ENABLE); /* emulate one shot timer */
	TIMER->INTSTATUS_INTCLEAR = 0x1;

	Msg.Event = TIMER_EVENT_EXPIRED;
	if (!TimerToExpire)
		err("TimerToExpire is not set in ISR\n");
//	xHigherPriorityTaskWoken  = pdFALSE;
	if (xQueueSendFromISR(TimerQueue, &Msg, &xHigherPriorityTaskWoken) != pdPASS) {
		err("Sending Event %x Failed\n", TIMER_EVENT_EXPIRED);
	}
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

HAL_StatusTypeDef HAL_TimerCreate(HAL_Timer_t *TimerHandle, uint32_t USecs,
				  uint32_t Flags, HAL_TimerCallback_t CallBack,
				  void *Cookie)
{
	static unsigned int init_done;
	int i;
	struct HAL_Timer_t *timer = NULL;
	UINT32_t ulDomainClk = 0;

	if (!USecs || !CallBack || !Flags || !TimerHandle) {
		err("Invalid Agruments to %s\n", __FUNCTION__);
		return HAL_ERROR;
	}
	if (!USecsTimerTaskHandle) {
		err("USecs Timer Task not started\n");
		return HAL_ERROR;
	}
	if (!init_done) {	//TBD can remove this check since semaphore is having its own check
		if (!lock) {
			lock = xSemaphoreCreateMutex();
			if (lock == NULL) {
				err("Creating Mutex failed\n");
				return HAL_ERROR;
			}
		}
		HAL_Timer_Init();
		init_done = 1;
	}
	/* this lock is to mutually exclude running of this routine in parallel */
	if (xSemaphoreTake(lock, portMAX_DELAY) != pdTRUE) {
		err("Unable to take lock\n");
		return HAL_ERROR;
	}
	for (i = 0 ; i < MAX_TIMERS ; i++) {
		if (HAL_Timer_List[i].InUse == TIMER_UNUSED) {
			timer = &HAL_Timer_List[i];
			timer->InUse = TIMER_INIT;
			break;
		}
	}
	xSemaphoreGive(lock);
	if (!timer) {
		err("unable to find a free timer handle\n");
		return HAL_ERROR;
	}
	timer->Cookie = Cookie;
	timer->TimerCallback = CallBack;
	timer->USecs = USecs;
	timer->Flags = Flags;
	ulDomainClk = S3x_Clk_Get_Rate(S3X_M4_PRPHRL_CLK);
//	if (USecs > USEC_SCALING)
//		USecs -= USEC_SCALING
	timer->Ticks = (UINT32_t)(((float)ulDomainClk/MHZ_1) * (USecs));
	timer->ReloadTicks = timer->Ticks;
	*TimerHandle = timer;
	return HAL_OK;
}

static HAL_StatusTypeDef SendEventToTimerThread(HAL_Timer_t TimerHandle, enum TimerEventType Event)
{
	struct TimerMsg Msg;
	Msg.Event = Event;
	Msg.timer = TimerHandle;
	Msg.sync = xSemaphoreCreateBinary();
	if (Msg.sync == NULL) {
		err("Creating Msg Sync Mutex failed \n");
		return HAL_ERROR;
	}
	if (xQueueSend(TimerQueue, &Msg, portMAX_DELAY) != pdPASS) {
		err("Sending Event %x Failed\n", Event);
		vSemaphoreDelete(Msg.sync);
		return HAL_ERROR;
	}
	if(xSemaphoreTake(Msg.sync, portMAX_DELAY) != pdTRUE)
	{
		err("Taking the sync semaphore failed for Event %x\n", Event);
		vSemaphoreDelete(Msg.sync);
		return HAL_ERROR;
	}
	vSemaphoreDelete(Msg.sync);
	return HAL_OK;
}

HAL_StatusTypeDef HAL_TimerStart(HAL_Timer_t TimerHandle)
{
	if (!TimerHandle) {
		err("Invalid Arguments to %s\n", __FUNCTION__);
		return HAL_ERROR;
	}
	if (TimerHandle->InUse != TIMER_INIT) {
		    err("Invalid Timer State = %d\n", TimerHandle->InUse);
		    return HAL_ERROR;
	}
	return SendEventToTimerThread(TimerHandle, TIMER_EVENT_START);
}

HAL_StatusTypeDef HAL_TimerStop(HAL_Timer_t TimerHandle)
{
	if (!TimerHandle) {
		err("Invalid Arguments to %s\n", __FUNCTION__);
		return HAL_ERROR;
	}

	if (TimerHandle->InUse != TIMER_ACTIVE) {
		    err("Invalid Timer State = %d\n", TimerHandle->InUse);
		    return HAL_ERROR;
	}
	return SendEventToTimerThread(TimerHandle, TIMER_EVENT_STOP);
}

HAL_StatusTypeDef HAL_TimerDelete(HAL_Timer_t TimerHandle)
{
	if (!TimerHandle) {
		err("Invalid Arguments to %s\n", __FUNCTION__);
		return HAL_ERROR;
	}
	if (TimerHandle->InUse != TIMER_EXPIRED && TimerHandle->InUse != TIMER_INIT) {
		    err("Invalid Timer State = %d\n", TimerHandle->InUse);
		    return HAL_ERROR;
	}
	TimerHandle->InUse = TIMER_UNUSED;
	return HAL_OK;
}


HAL_StatusTypeDef HAL_Timer_Enable( )
{
    gTimerOn = 1;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_Timer_Disable( )
{
    gTimerOn = 0;
    TIMER->RELOAD = 0;
    TIMER->VALUE = 0;
    return HAL_OK;
}
