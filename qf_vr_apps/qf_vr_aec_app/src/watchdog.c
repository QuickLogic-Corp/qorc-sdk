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
 *    File   : watchdog.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"
#include "eoss3_hal_wdt.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include "sec_debug.h"
//#include "test_bool.h"

#ifdef WATCHDOG_SUPPORT
SemaphoreHandle_t g_wdtMutex = NULL;
static TaskHandle_t wdt_task_handle = NULL;
bool wdt_sleep = false;
#define WDT_TIMEOUT		(20)
#define WDT_RELOAD_TIME		(10)
#define WDT_RELOAD_COUNT	(5)

static void wdt_reload_task_handler(void *a)
{
	if (HAL_OK != HAL_WDT_Init(WDT_TIMEOUT)) {
		printf("HAL_WDT_Init Failed\n");
		return;
	}

	if (HAL_OK != HAL_WDT_Start()) {
		printf("HAL_WDT_Init Failed\n");
		return;
	}

    wait_ffe_fpga_load();

	while (1) {
		vTaskDelay(((WDT_RELOAD_TIME) * 1000) / portTICK_PERIOD_MS);
		if ( HAL_OK == HAL_WDT_WdtIsStartStatus() ) {
			//printf("### [WDT] reload watchdog timer ###\n");
			HAL_WDT_Reload();
			wdt_sleep = false;
		}
		else {
			//printf("### [WDT] wait wakeup ###\n");			
			wdt_sleep = true;			
			xSemaphoreTake(g_wdtMutex, portMAX_DELAY);
		}
	}
}

void init_watchdog_reload_task(void)
{
  	if (g_wdtMutex == NULL)
	{
		g_wdtMutex = xSemaphoreCreateBinary();
		if (g_wdtMutex != NULL){
			xSemaphoreGive(g_wdtMutex);
		}
        vQueueAddToRegistry(g_wdtMutex, "WDog");
	}
	/* create watchdog reload task */
	xTaskCreate(wdt_reload_task_handler, "WDT RLD TSK", configMINIMAL_STACK_SIZE , NULL, TASK_PRI_WATCHDOG_RELOAD, &wdt_task_handle);
	configASSERT( wdt_task_handle );
}
#endif