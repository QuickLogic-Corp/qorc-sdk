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
 *    File   : eoss3_hal_rtc.c
 *    Purpose: 
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <eoss3_dev.h>
#include <eoss3_hal_rtc.h>
#include "FreeRTOSConfig.h"
#include "Fw_global_config.h"
#include "dbg_uart.h"

typedef enum {RTC_TICKGEN = 0x01, REG_TRIM, REG_CLK, REG_ALARM, REG_0, RTC_END_OF_REG} eRTC_REG;

#define is_valid_rtc_int_reg(_x_) ((_x_) < RTC_END_OF_REG)
#define REG_0_MASK	1

// write/read_rtc_reg is specific to the IP used. read/write seq can be found in
// the datasheet
static void write_rtc_reg(eRTC_REG rtc_reg, uint32_t val)
{
	 if (!is_valid_rtc_int_reg(rtc_reg)) {
		dbg_fatal_error("Invalid register to read\n");
		return;
	}
	AIP->RTC_CTRL_3 = 0;
	AIP->RTC_CTRL_3 = rtc_reg;
	AIP->RTC_CTRL_6 = val;
	AIP->RTC_CTRL_4 = 1;
	AIP->RTC_CTRL_4 = 0;
	AIP->RTC_CTRL_3 = 0;
}

static uint32_t read_rtc_reg(eRTC_REG rtc_reg)
{
	volatile uint32_t val, val1;
	 if (!is_valid_rtc_int_reg(rtc_reg)) {
		dbg_fatal_error("Invalid register to read\n");
		return -1;
	}
	AIP->RTC_CTRL_4 = 0;
	AIP->RTC_CTRL_3 = 0;
	AIP->RTC_CTRL_3 = rtc_reg;
	val1 = AIP->RTC_STA_1;
	val = AIP->RTC_STA_1;
	AIP->RTC_CTRL_3 =  0;
	return val;
}

static inline uint32_t read_rtc_reg_clk(void)
{
	return read_rtc_reg(REG_CLK);
}

#ifdef RTC_DEBUG

static void dump_rtc_regs(void)
{
	QL_LOG_INFO_150K("RTC_CTRL_1 = %#x, RTC_CTRL_2 = %#x, RTC_CTRL_3 = %#x, RTC_CTRL_4 = %#x, "
		"RTC_CTRL_5 = %#x, RTC_CTRL_6 = %#x, RTC_CTRL_7 = %#x, RTC_STA_0 = %#x, "
		"RTC_STA_1 = %#x\n", AIP->RTC_CTRL_1, AIP->RTC_CTRL_2, AIP->RTC_CTRL_3,
		AIP->RTC_CTRL_4, AIP->RTC_CTRL_5, AIP->RTC_CTRL_6, AIP->RTC_CTRL_7,
		AIP->RTC_STA_0, AIP->RTC_STA_1);
	
}
#endif

static uint32_t rtc_alarm_interval = 1, rtc_initialized=0;
static void (*rtc_cb_func)(void);

static void inline __rtc_setup_alarm(uint32_t rtc_intr_inter)
{
     	uint32_t clk;
	write_rtc_reg(REG_0, 1);
	clk = read_rtc_reg_clk();
	write_rtc_reg(REG_ALARM, clk + rtc_alarm_interval);
	write_rtc_reg(REG_0, 0);	
}

static void inline __rtc_init(uint32_t uiInitialRTCValue)
{
	write_rtc_reg(REG_TRIM, 0);
	write_rtc_reg(RTC_TICKGEN, 31);
	write_rtc_reg(REG_CLK, uiInitialRTCValue);	//Initial RTC Val
}

static void inline eoss3_rtc_enable_irq(void)
{
	INTR_CTRL->OTHER_INTR_EN_M4 |= RTC_INTR_EN_M4;
	NVIC_ClearPendingIRQ(RtcAlarm_IRQn);
	NVIC_SetPriority(RtcAlarm_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	NVIC_EnableIRQ(RtcAlarm_IRQn);
}

void HAL_RTC_ISR(void)
{
	if (rtc_cb_func)
		rtc_cb_func();
	if (rtc_alarm_interval)
		__rtc_setup_alarm(rtc_alarm_interval);
	eoss3_rtc_enable_irq();
}

HAL_StatusTypeDef HAL_RTC_SetupAlarm(uint32_t rtc_intr_inter, void (*rtc_cb)(void))
{
	if (rtc_cb)
		rtc_cb_func = rtc_cb;
	if (rtc_intr_inter) {
		rtc_alarm_interval = rtc_intr_inter;
	}
	__rtc_setup_alarm(rtc_alarm_interval);
	eoss3_rtc_enable_irq();
	return HAL_OK;
}

HAL_StatusTypeDef HAL_RTC_Init(uint32_t uiInitialRTCValue)
{
	if (!rtc_initialized)
	{
		//QL_LOG_DBG_150K("HAL_RTC_Init\r\n");
		AIP->RTC_CTRL_2 = 1 << 2;
		__rtc_init(uiInitialRTCValue);
		rtc_initialized = 1;
	}
	return HAL_OK;
}

HAL_StatusTypeDef HAL_RTC_SetTime(uint32_t uiRTCValue)
{
	// Needs protection from interrupt handler because accessing RTC registers
	// is not a single reg read operation & everal registers needs to setup 
	// to access a particular rtc register. IRQ handler coming in between 
	// the write/read_rtc_reg calls from HAL_RTC_Set/GetTime can alter these
	// values setup in these registers, leading to incorrect value. This is a very 
	// remote corner case that can arise.
	NVIC_DisableIRQ(RtcAlarm_IRQn);
	write_rtc_reg(REG_CLK, uiRTCValue);
	NVIC_SetPriority(RtcAlarm_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	NVIC_EnableIRQ(RtcAlarm_IRQn);

	return HAL_OK;
}

HAL_StatusTypeDef HAL_RTC_GetTime(uint32_t *uiRTCValue)
{
	if (!uiRTCValue)
		return HAL_ERROR;
	NVIC_DisableIRQ(RtcAlarm_IRQn);
	*uiRTCValue = read_rtc_reg(REG_CLK);
	NVIC_SetPriority(RtcAlarm_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	NVIC_EnableIRQ(RtcAlarm_IRQn);

	return HAL_OK;	 
}
