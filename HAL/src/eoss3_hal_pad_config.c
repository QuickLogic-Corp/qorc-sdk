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
 *    File   : eoss3_hal_pad_config.c
 *    Purpose: This file contains macros, structures and APIs to
 *             configure pads 
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include <eoss3_hal_pad_config.h>
#include <stdint.h>
#include <stdio.h>

/// @cond <section name>
#define  OUTPUT_EN		((uint8_t)0x00)   /* Output Enable */
#define  INPUT_DIS		((uint8_t)0x00)   /* Input Disable */
#define  OUTPUT_DRV_DIS		((uint8_t)0x01)   /* Output Disable */

#define EXT_REGS_MAX	44
#define EXT_REGS_PADS_VARS_MAX	8
#define IO_INPUT_MAX	8
#define IO_MUX_MAX  2

#define DEFAULT_VAL_0   0
#define DEFAULT_VAL_1   1
#define DEFAULT_VAL_N1   -1

/*Extended Pad config register offset starts from */
/*! \var int8_t  SEL_EXT_PAD_REGS
    \brief This array contains all the possible pad selection for given register.
    These registers are extention to PAD config registers [0 to 45]. First value 
    in an array is default value and rest indexes need to be written for given pad.
    Offset of extended register starts from 0x100. This offset is embedded in each 
    pad function macro.
*/

const int8_t  SEL_EXT_PAD_REGS[EXT_REGS_MAX][EXT_REGS_PADS_VARS_MAX] = {
      {DEFAULT_VAL_1, PAD_1}, //100
      {DEFAULT_VAL_1, PAD_15,PAD_32,PAD_44}, //104
      {DEFAULT_VAL_1, PAD_41}, //108
      {DEFAULT_VAL_1, PAD_0}, //10C
      {DEFAULT_VAL_1, PAD_14, PAD_33, PAD_45}, //110
      {DEFAULT_VAL_1, PAD_40}, //114
      {PAD_16, PAD_0}, //118 
      {PAD_20, PAD_0}, //11C 
      {PAD_19, PAD_0}, //120
      {PAD_36, PAD_0}, //124 
      {DEFAULT_VAL_0, PAD_10, PAD_28}, //128
      {DEFAULT_VAL_0, PAD_10, PAD_28}, //12C
      {DEFAULT_VAL_N1}, //Dummy 130
      {DEFAULT_VAL_0, PAD_14,PAD_16,PAD_25,PAD_45}, //GAP 134
      {DEFAULT_VAL_0, PAD_6,PAD_15,PAD_21,PAD_24,PAD_28,PAD_40,PAD_44}, //138
      {DEFAULT_VAL_0, PAD_3}, //13C
      {DEFAULT_VAL_0, PAD_2,PAD_6,PAD_18,PAD_24,PAD_35,PAD_36}, //140
      {DEFAULT_VAL_0, PAD_4,PAD_8,PAD_21,PAD_25,PAD_37,PAD_38}, //144
      {DEFAULT_VAL_0, PAD_5,PAD_9,PAD_22,PAD_28,PAD_39,PAD_40}, //148
      {DEFAULT_VAL_0, PAD_7,PAD_10,PAD_26,PAD_29,PAD_44}, //14C
      {DEFAULT_VAL_0, PAD_11,PAD_14,PAD_27,PAD_30,PAD_45}, //150
      {DEFAULT_VAL_0, PAD_12,PAD_15,PAD_31,PAD_32,PAD_41}, //154
      {DEFAULT_VAL_0, PAD_13,PAD_23,PAD_33,PAD_34,PAD_42}, //158
      {DEFAULT_VAL_1, PAD_17,PAD_22}, //15C
      {DEFAULT_VAL_0, 0}, //160 //IO_REG_SEL, handled seperately using an arry IO_INPUT_PAD_SEL
      {DEFAULT_VAL_N1, 0}, //Dummy 164
      {DEFAULT_VAL_N1, 0}, //Dummy 168
      {DEFAULT_VAL_N1, 0}, //Dummy 16C
      {DEFAULT_VAL_1, 0}, //170 Debugger pad selection happed using bootstap pin
      {DEFAULT_VAL_1, 0}, //174 Debugger pad selection happed using bootstap pin
      {DEFAULT_VAL_N1, 0}, //Dummy 178
      {DEFAULT_VAL_N1, 0}, //Dummy 17C
      {DEFAULT_VAL_1, 0}, //180 FBIO : Special handling for each bit
      {DEFAULT_VAL_1, PAD_32}, //184 FBIO : Special handling for each bit (we can subtract 
      {DEFAULT_VAL_0, 0}, //Dummy 188
      {DEFAULT_VAL_0, 0}, //Dummy 18C								
      {DEFAULT_VAL_0, PAD_8,PAD_29}, //190
      {DEFAULT_VAL_0, PAD_6,PAD_28}, //194
      {DEFAULT_VAL_N1, 0}, //Dummy 198
      {DEFAULT_VAL_N1, 0}, //Dummy 19C
      {DEFAULT_VAL_0, PAD_23}, //1A0
      {DEFAULT_VAL_0, PAD_31}, //1A4
      {DEFAULT_VAL_0, PAD_9,PAD_30}, //0x1A8
      {DEFAULT_VAL_0, PAD_38}, //0x1AC			
      };

/*! \var uint8_t IO_INPUT_PAD_SEL
    \brief This array contains pad selection for GPIO input configuration written in 
    IO_REG_SEL register.
*/
const uint8_t IO_INPUT_PAD_SEL[IO_INPUT_MAX][IO_MUX_MAX] =
      {{PAD_6,PAD_24}, {PAD_9,PAD_26}, {PAD_11,PAD_28}, {PAD_14,PAD_30},
      {PAD_18,PAD_31},{PAD_21,PAD_36},{PAD_22,PAD_38},{PAD_23,PAD_45}};

/// @endcond


/*
 * Variable to tell which pads have been configured to enable detecting reconfiguration
 * Bit = 0 => Not configured
 * Bit = 1 => Is configured
 *
*/
uint64_t    fPadIsConfig = 0;

void HAL_PAD_Config(PadConfig *pxPadInit)
{
	PadConfigReg xPadRegVal;
	uint32_t *pExtRegAddr;
	uint32_t uiExtRegAddr;
	uint8_t ucCount, ucIdx;
	int8_t ucIsValid=false;
    
    /* Check to see if already configured */
    configASSERT( !(fPadIsConfig & ((uint64_t)1 << pxPadInit->ucPin)) );
    fPadIsConfig |= ((uint64_t)1 << pxPadInit->ucPin);
        
	*((uint32_t *)(&xPadRegVal)) = 0x00;

	xPadRegVal.bCtrl = pxPadInit->ucCtrl;

	xPadRegVal.bFunc = pxPadInit->ucFunc & 0x03; /*Since we have been using only two bits to define function.*/

	xPadRegVal.bOPull = pxPadInit->ucPull;
	xPadRegVal.bODrv = pxPadInit->ucDrv;
	xPadRegVal.bSpeed = pxPadInit->ucSpeed;
	xPadRegVal.bSmtTrg = pxPadInit->ucSmtTrg;
	
	ucIsValid = false;
    /* In case of output mode selection */
    if((pxPadInit->ucMode == PAD_MODE_OUTPUT_EN))
    {
    	ucIsValid = true;
    	xPadRegVal.bOEn = OUTPUT_EN;
	xPadRegVal.bIEn = INPUT_DIS;
    }
    else if((pxPadInit->ucMode == PAD_MODE_INPUT_EN))
    {
    	ucIsValid = true;
    	xPadRegVal.bOEn = OUTPUT_DRV_DIS;
	xPadRegVal.bIEn = pxPadInit->ucMode;
    }

    if(ucIsValid)
    {
    	pExtRegAddr = (uint32_t *)IO_MUX;
    	pExtRegAddr += pxPadInit->ucPin;
    	*pExtRegAddr = *(uint32_t *)&xPadRegVal;
    }

    /*Check if it needs any additional register need to be configured or Pad selection to be performed*/
    if(uiExtRegAddr = (pxPadInit->ucFunc >> EXT_REG_OFFSET_SHIFT))
    {
    	/*Check if special handling needed.*/
    	if((uiExtRegAddr == FBIO_SEL_1) ||  (uiExtRegAddr == FBIO_SEL_2))
    	{
    		if(uiExtRegAddr == FBIO_SEL_2)
    		{
    			uiExtRegAddr = uiExtRegAddr | IO_MUX_BASE;
    			pExtRegAddr = (uint32_t *)uiExtRegAddr;
    			*pExtRegAddr |= 1 << (pxPadInit->ucPin - 32);
    		}
    		else
    		{
    			uiExtRegAddr |= IO_MUX_BASE;
    			pExtRegAddr = (uint32_t *)uiExtRegAddr;
    			*pExtRegAddr |= 1 << pxPadInit->ucPin;
    		}
    	}
    	else if(uiExtRegAddr == IO_REG_SEL) /*GPIO Input Configuration*/
    	{
    		ucIsValid = false;
    		/*GPIO Input configuration*/
    		if((pxPadInit->ucMode == PAD_MODE_INPUT_EN))
    		{
    			for(ucIdx=0; ucIdx<IO_INPUT_MAX; ucIdx++)
    			{
    				for(ucCount=0;ucCount<IO_MUX_MAX;ucCount++)
    				{
    					if(IO_INPUT_PAD_SEL[ucIdx][ucCount] == pxPadInit->ucPin)
    					{
    						/*Found requested pad in Input array*/
    						ucIsValid = true;
    						uiExtRegAddr |= IO_MUX_BASE;
    						pExtRegAddr = (uint32_t *)uiExtRegAddr;
    						*pExtRegAddr |= ucCount << ucIdx;
    						break;
    					}
    				}
    			}
    		}
    	}
    	else
    	{
    		ucIsValid = false;

			/*Check if it is not pointing to dummy address*/
			if(SEL_EXT_PAD_REGS[(uiExtRegAddr - EXT_REG_OFFSET_BASE)/4][0] ==  -1)
			{
				ucIsValid = false;
			}
			else
			{
                /*Start searching pad from index 1 as 0th is default value*/
				for(ucCount = 1; ucCount < EXT_REGS_PADS_VARS_MAX; ucCount++)
				{
					/*Compare pin number in the extended register array finding index in that two dimensional array*/
					if(SEL_EXT_PAD_REGS[(uiExtRegAddr - EXT_REG_OFFSET_BASE)/4][ucCount] ==  pxPadInit->ucPin)
					{
						uiExtRegAddr |= IO_MUX_BASE;
						pExtRegAddr = (uint32_t *)uiExtRegAddr;
						*pExtRegAddr = ucCount;
						break;
					}
				}
			}
    	}
    }
}

void HAL_PAD_DeConfig(PadConfig *pxPadInit)
{
	//Write Padconfig register to default values
	//Write Ext pad config register to def value.
}
void HAL_PAD_Reconfig(PadConfig *pxPadInit)
{
  //first remove the config bit and call config
  fPadIsConfig &= ~((uint64_t)1 << pxPadInit->ucPin);
  HAL_PAD_Config(pxPadInit);
}


void configure_s3_pads(PadConfig *p_table, int nitems)
{
  for (int k = 0; k < nitems; k++)
  {
    HAL_PAD_Config(&p_table[k]);
  }
}

