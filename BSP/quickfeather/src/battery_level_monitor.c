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
 *    File   : batt_level_task.c
 *    Purpose: Monitor battery level, indicate if below some threshold
 *
 *=========================================================*/

#include "Fw_global_config.h"

#include "eoss3_dev.h"
#include "s3x_clock_hal.h"
#include "eoss3_hal_adc.h"
#include "eoss3_hal_gpio.h"

#include "battery_level_monitor.h"
#include "FreeRTOS.h"
#include "task.h"
#include "RtosTask.h"

int mCurrentBatteryLevel = 0; ///< Current battery level in mV units

/*------------------------------------------------------------------
 * Add the following pin configuration to the pincfg_table.c to
 * enable ADC0_EN GPIO signal to enable and read the battery level
 *
   { // setup ADC0_EN to enable battery level measurement
    .ucPin = PAD_26,
    .ucFunc = PAD26_FUNC_SEL_GPIO_1,
    .ucCtrl = PAD_CTRL_SRC_A0,
    .ucMode = PAD_MODE_OUTPUT_EN,
    .ucPull = PAD_NOPULL,
    .ucDrv = PAD_DRV_STRENGTH_4MA,
    .ucSpeed = PAD_SLEW_RATE_SLOW,
    .ucSmtTrg = PAD_SMT_TRIG_DIS,
  }
 *
 *------------------------------------------------------------------*/

/* Return the battery voltage level in mV units
 */
int getCurrentBatteryLevel(void)
{
	return mCurrentBatteryLevel;
}

/* Start a new ADC conversion and update the current battery level
 */
void updateCurrentBatteryLevel(void)
{
	uint16_t iCurrentBatteryLevel = 0;    ///< 12-bit integer from ADC conversion unit
	float    fCurrentBatteryLevel = 0.f;  ///< Voltage level in range [0, 1.4]V

	// Enable ADC0_EN signal
	HAL_GPIO_Write(GPIO_1, 0);
	HAL_GPIO_Write(GPIO_1, 1);

	HAL_ADC_StartConversion(); // start ADC conversion
	vTaskDelay(25);            // Conversion takes about 25ms
	HAL_ADC_GetData(&iCurrentBatteryLevel);  // get the ADC reading

	// Convert to Voltage level ( adc_reading / 4096) * 1.4 * 3
	fCurrentBatteryLevel = 3.f * 1.4f * iCurrentBatteryLevel / 4096.f ;

	// Disable ADC0_EN signal
	HAL_GPIO_Write(GPIO_1, 0);
	mCurrentBatteryLevel = (int)(fCurrentBatteryLevel * 1000);
	return ;
}

void battery_level_monitor_task(void *pParameter)
{
	int mvolts;
	HAL_ADC_Init(ADC_CHANNEL_0, 1); // Enable battery measurement
    while (1)
    {
    	updateCurrentBatteryLevel();
    	mvolts = getCurrentBatteryLevel();
    	if (mvolts < BATTERY_LEVEL_WARNING_THRESHOLD)
    	{
    		// turn on red LED for 50ms
    	    HAL_GPIO_Write(6, 1);
    	    vTaskDelay(BATTERY_LEVEL_LED_BLINK_PERIOD);
    	    HAL_GPIO_Write(6, 0);
    	}
    	vTaskDelay(BATTERY_LEVEL_MONITOR_INTERVAL);
    }
}

xTaskHandle xBatteryLevelMonitorTask;
void start_battery_level_monitor_task(void)
{
    xTaskCreate ( battery_level_monitor_task, "batery_level_monitor", BATTERY_LEVEL_MONITOR_TASK_STACKSIZE, NULL, (UBaseType_t)(PRIORITY_NORMAL), &xBatteryLevelMonitorTask);
    configASSERT( xBatteryLevelMonitorTask );
}
