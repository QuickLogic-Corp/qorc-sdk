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
 *    File   : eoss3_hal_adc.c
 *    Purpose: 
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"
#include "eoss3_dev.h"
#include "eoss3_hal_adc.h"
#include "s3x_clock_hal.h"
//#include "eoss3_hal_rcc.h"
//#include "eoss3_hal_clock.h"

#define getreg32(a)          				(*(volatile uint32_t *)(a))
#define putreg32(v,a)        				(*(volatile uint32_t *)(a) = (v))
#define JTM_OUT				 		JTM_BASE + 0x000
#define JTM_STATUS			 		JTM_BASE + 0x004
#define JTM_CONTROL			 		JTM_BASE + 0x008


#define JTM_CONTROL_SOC_BIT            		   	0 // Start Of Conversion bit
#define JTM_CONTROL_CHANNEL_SEL_BIT            	        1
#define JTM_CONTROL_MEAS_EN_BIT     			2
#define JTM_STATUS_EOC_BIT            		   	0 // End Of Conversion bit
#define JTM_OUT_12_BIT_MASK                             ((1<<12)-1)


#define delayCycles(_x_)	do {							\
					volatile unsigned int _delayCycleCount = _x_;	\
					while (_delayCycleCount--);			\
				} while(0)


HAL_StatusTypeDef HAL_ADC_Init(ADC_channel channelNum,uint8_t enableBattMeasure)
{

    S3x_Clk_Enable(S3X_ADC_CLK);

	// set the channelNum & battery enable
	putreg32(0, JTM_CONTROL);
	putreg32(( (channelNum << JTM_CONTROL_CHANNEL_SEL_BIT) |
		  	(enableBattMeasure << JTM_CONTROL_MEAS_EN_BIT) ),JTM_CONTROL);
	return HAL_OK;
}

void HAL_ADC_StartConversion(void )
{
	uint32_t temp;
	temp = getreg32(JTM_CONTROL);
	putreg32((temp |(1<< JTM_CONTROL_SOC_BIT)),JTM_CONTROL);
#if 1
    while(getreg32(JTM_STATUS)); //Wait for it to go low. It takes 3 JTM clk cycles to go low
#else
    delayCycles(130); //Wait atleast 3 JTM clk cycles for status to go low, even at 80MHz M4 clk, JTM at 1MHz
#endif
}

HAL_StatusTypeDef HAL_ADC_GetData( uint16_t *pData)
{
	uint32_t temp;

	// Check if conversion is done
	while(!(getreg32(JTM_STATUS)));
	// read data
	delayCycles(50);
    *pData = getreg32(JTM_OUT) & JTM_OUT_12_BIT_MASK;

	//	disable SOC
	temp = getreg32(JTM_CONTROL);
	putreg32((temp & ~(1<< JTM_CONTROL_SOC_BIT)),JTM_CONTROL);

	return HAL_OK;
}



