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
 *    File   : eoss3_hal_wdt.c
 *    Purpose: 
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#include "eoss3_dev.h"
#include "s3x_clock_hal.h"
#include "eoss3_hal_def.h"
#include "eoss3_hal_wdt.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "sec_debug.h"
#include "Fw_global_config.h"
#include "dbg_uart.h"

#define WDOGLOCK_UNLOCK		(0x1ACCE551)
#define WDOGLOCK_LOCK		(0x1)
#define WDOGLOCK_LOCKED		(0x1)

#define WDOGCONTROL_RESEN	(1 << 1)
#define WDOGCONTROL_INTEN	(1 << 0)


#define WDT_FLAG_RESET_EN	(1 << 0)
#define WDT_FLAG_DEFAULT	(0)

#define WDT_UNLOCK() do { WDT->WDOGLOCK = WDOGLOCK_UNLOCK; } while (0);
#define WDT_LOCK() do { WDT->WDOGLOCK = WDOGLOCK_LOCK; } while (0);
#define err(...) do { printf("[HAL WDT] [ERR] [%s] : ", __FUNCTION__); printf(__VA_ARGS__); } while(0)

enum wdtstate {WDT_DEINIT, WDT_INIT, WDT_START, WDT_STOP};
struct WDTHandle {
	enum wdtstate state;
	unsigned int reloadvalue, timeout, flags;
};

static struct WDTHandle WDT_Handle = {
	WDT_DEINIT, 0, 0
};


void WDT_ISR(void)
{
	static unsigned int isrcount;
	if (WDT_Handle.state != WDT_START)
		err("Actv in Inval Ste = %x\n", WDT_Handle.state);
	dbg_str_int("@@WDT_ISR@@", ++isrcount);
	if (!(WDT_Handle.flags & WDT_FLAG_RESET_EN)) {
		WDT_UNLOCK();
		WDT->WDOGINTCLR = 1;
		WDT_LOCK();
	}

	taskDISABLE_INTERRUPTS();
	save_assert_info("watchdog timeout", uxTaskGetTCBNumber(xTaskGetCurrentTaskHandle()));
	invoke_soft_fault();

}

static HAL_StatusTypeDef __HAL_WDT_Configure(struct WDTHandle *handle, unsigned int secs_timeout)
{
	unsigned int clk;
	clk = S3x_Clk_Get_Rate(S3X_M4_PRPHRL_CLK);
	handle->reloadvalue = clk * secs_timeout;
	return HAL_OK;
}

static HAL_StatusTypeDef __HAL_WDT_Init(unsigned int secs_timeout, unsigned int flag)
{
	if (WDT_Handle.state != WDT_DEINIT) {
		err("Inval Ste = %x\n", WDT_Handle.state);
		return HAL_ERROR;
	}

	WDT_Handle.flags = flag;
	__HAL_WDT_Configure(&WDT_Handle, secs_timeout);

	WDT_UNLOCK();
	WDT->WDOGCONTROL = 0;
	WDT->WDOGINTCLR = 1;
	WDT->WDOGLOAD = WDT_Handle.reloadvalue;
	WDT_LOCK();

	WDT_Handle.state = WDT_INIT;

	return HAL_OK;
}

HAL_StatusTypeDef HAL_WDT_Init(unsigned int secs_timeout)
{
	return __HAL_WDT_Init(secs_timeout, WDT_FLAG_DEFAULT);
}

HAL_StatusTypeDef HAL_WDT_Start(void)
{
	unsigned int wdgctrl = 0;
	if (WDT_Handle.state != WDT_INIT && WDT_Handle.state != WDT_STOP)
		return HAL_ERROR;

	//printf("### [WDT] start watchdog timer ###\n");

	wdgctrl = WDOGCONTROL_INTEN;
	if (WDT_Handle.flags & WDT_FLAG_RESET_EN)
		wdgctrl |= WDOGCONTROL_RESEN;

	INTR_CTRL->OTHER_INTR &= WDOG_INTR_DETECT;
	INTR_CTRL->OTHER_INTR_EN_M4 |= WDOG_INTR_EN_M4;
	NVIC_ClearPendingIRQ(CpuWdtInt_IRQn);

	NVIC_EnableIRQ(CpuWdtInt_IRQn);

	WDT_UNLOCK();
	WDT_Handle.state = WDT_START;
	WDT->WDOGCONTROL = wdgctrl;
	WDT_LOCK();
	return HAL_OK;
}

HAL_StatusTypeDef HAL_WDT_Stop(void)
{
	if (WDT_Handle.state != WDT_START)
		return HAL_ERROR;

	//printf("### [WDT] stop watchdog timer ###\n");

	WDT_Handle.state = WDT_STOP;
	WDT_UNLOCK();
	WDT->WDOGCONTROL = 0;
	WDT_LOCK();
	return HAL_OK;
}

HAL_StatusTypeDef HAL_WDT_WdtIsStartStatus(void)
{
 	if (WDT_Handle.state != WDT_START)
		return HAL_BUSY;

	return HAL_OK;
}

HAL_StatusTypeDef HAL_WDT_Configure(unsigned int secs_timeout)
{
	return __HAL_WDT_Configure(&WDT_Handle, secs_timeout);
}

HAL_StatusTypeDef HAL_WDT_Reload(void)
{
	if (WDT_Handle.state != WDT_START)
		return HAL_ERROR;

	WDT_UNLOCK();
	WDT->WDOGLOAD = WDT_Handle.reloadvalue;
	WDT_LOCK();
	return HAL_OK;
}

HAL_StatusTypeDef HAL_WDT_DeInit(void)
{
	if (WDT_Handle.state == WDT_INIT)
		HAL_WDT_Stop();
	WDT_Handle.state = WDT_DEINIT;
	return HAL_OK;
}
