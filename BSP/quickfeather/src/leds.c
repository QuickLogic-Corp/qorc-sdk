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
 * leds.c
 *
 *  Created on: Aug 2, 2016
 *      Author: saxe
 */

#include "Fw_global_config.h"
#include <stdio.h>
#include "eoss3_dev.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "leds.h"

static TimerHandle_t  OLedTimerId = NULL;
static TimerHandle_t  YLedTimerId = NULL;

void LedGreenOn(void) {
// Turn on green led
	IO_MUX->PAD_22_CTRL = 0x103;
	MISC_CTRL->IO_OUTPUT |= (0x40);
}

void LedGreenOff(void) {
// Turn off green led
	IO_MUX->PAD_22_CTRL = 0x103;
	MISC_CTRL->IO_OUTPUT &= (~0x40);
}

void LedBlueOn(void) {
#ifndef FPGA_DIS_PAD26_CFG
	// Turn on blue led
	IO_MUX->PAD_26_CTRL = 0x103;
#endif
	MISC_CTRL->IO_OUTPUT |= (0x02);
}

void LedBlueOff(void){
#ifndef FPGA_DIS_PAD26_CFG
	// Turn off blue led
	IO_MUX->PAD_26_CTRL = 0x103;
#endif
	MISC_CTRL->IO_OUTPUT &= (~0x02);
}

void LedOrangeOn(void) {
	IO_MUX->PAD_18_CTRL = 0x103;
	MISC_CTRL->IO_OUTPUT |= 0x10;
}

void LedOrangeOff(void) {
	IO_MUX->PAD_18_CTRL = 0x103;
	MISC_CTRL->IO_OUTPUT &= (~0x10);
}

// timer callback for orange led timer to turn off led
static void OLedTimerCB( TimerHandle_t Timer ) {
    configASSERT( Timer );
    LedOrangeOff();

    if( xTimerDelete( OLedTimerId, 0 ) != pdPASS )
        printf("Fail to delete an 1/2s timer!!! \r\n");
    else
        OLedTimerId = NULL;

}

// Do not call from an ISR unless modify it to use ISR specific Timer APIs
void LedOrangeOn_AutoOff(void) {

    if( OLedTimerId == NULL ) {
        LedOrangeOn();

        // create orange led 200 ticks one-shot timer to turn off the led
        OLedTimerId = xTimerCreate("OLedTimer", 200, pdFALSE, ( void * ) 0, OLedTimerCB);
        if( OLedTimerId == NULL )
            printf("Fail to create timer!!! \r\n");
        else
            if( xTimerStart( OLedTimerId, 0 ) != pdPASS )
                printf("Fail to start orange led timer!!! \r\n");
    }
    else { // timer exists and running, i.e led is already On
        LedOrangeOff();
        vTaskDelay( configTICK_RATE_HZ/10 );//200msec of dark time
        LedOrangeOn();

        if( xTimerReset( OLedTimerId, 0 ) != pdPASS )
            printf("Fail to reset orange led timer!!! \r\n");
    }

}

void LedYellowOn(void) {
	IO_MUX->PAD_21_CTRL = 0x103;
	MISC_CTRL->IO_OUTPUT |= 0x20;
}

void LedYellowOff(void) {
	IO_MUX->PAD_21_CTRL = 0x103;
	MISC_CTRL->IO_OUTPUT &= (~0x20);
}


static void YLedTimerCB( TimerHandle_t Timer )
{
    configASSERT( Timer );
    LedYellowOn();

    if( xTimerDelete( YLedTimerId, 0 ) != pdPASS )
        printf("Fail to delete an 1/2s timer!!! \r\n");
    else
        YLedTimerId = NULL;

}

void LedYellowBlink(void) {
	LedYellowOff();
    // create yellow led 100 ticks one-shot timer to turn off the led
    YLedTimerId = xTimerCreate("YLedTimer", 100, pdFALSE, ( void * ) 0, YLedTimerCB);
    if( YLedTimerId == NULL )
        printf("Fail to create timer!!! \r\n");
    else
        if( xTimerStart( YLedTimerId, 0 ) != pdPASS )
            printf("Fail to start orange led timer!!! \r\n");

}

void LedGreenBlink(void)
{
  static int i = 0;
  i++;
  if( i%20 == 0 ) {
    LedGreenOn();
  }
  else if( i%5 == 0 ) {
    LedGreenOff();
  }
}

void turnOnLeds(uint8_t on) {
#if (FPGA_DIS_PAD26_CFG == 0)
	IO_MUX->PAD_26_CTRL = 0x103;
#endif
	IO_MUX->PAD_22_CTRL = 0x103;
	IO_MUX->PAD_18_CTRL = 0x103;
	IO_MUX->PAD_21_CTRL = 0x103;
	if(on)
		MISC_CTRL->IO_OUTPUT = (0x72);
	else
		MISC_CTRL->IO_OUTPUT = (0x0);
}
