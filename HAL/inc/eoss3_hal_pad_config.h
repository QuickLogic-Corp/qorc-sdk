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
 *    File   : eoss3_hal_pad_config.h
 *    Purpose: This file contains macros, structures and APIs to
 *             configure pads 
 *                                                          
 *=========================================================*/

#ifndef __EOSS3_HAL_PAD_AF_H
#define __EOSS3_HAL_PAD_AF_H

#include <eoss3_hal_pads.h>

#ifdef __cplusplus
 extern "C" {
#endif

 /*PAD configuration Register bits*/
 
/*! \def PAD_CTRL_SRC_A0
    \brief Control block that controls pad, It's M4 or other controller where 
    connected device does not interfear.
*/
#define  PAD_CTRL_SRC_A0			((uint8_t)0x00)   /* Source is Firmware for GPIO */
   
/*! \def PAD_CTRL_SRC_OTHER
    \brief Control block that controls pad eg I2C controller, FFE0.
*/
#define  PAD_CTRL_SRC_OTHER			((uint8_t)0x01)   /* Source is HW controller */

/*! \def PAD_CTRL_SRC_FPGA
    \brief FPGA block controls pad.
*/
#define  PAD_CTRL_SRC_FPGA		((uint8_t)0x02)   /* Source is FPGA */

/*! \def PAD_MODE_OUTPUT_EN
    \brief Pad mode set as an output.
*/      
#define  PAD_MODE_OUTPUT_EN			((uint8_t)0x02)
   
/*! \def PAD_MODE_INPUT_EN
    \brief Pad mode set as an input.
*/      
#define  PAD_MODE_INPUT_EN			((uint8_t)0x01)

/*! \def PAD_NOPULL
    \brief Pad no pull.
*/      
#define  PAD_NOPULL        			((uint8_t)0x00)
   
/*! \def PAD_PULLUP
    \brief Pad pull up.
*/      
#define  PAD_PULLUP        			((uint8_t)0x01)

/*! \def PAD_PULLDOWN
    \brief Pad pull down.
*/      
#define  PAD_PULLDOWN      			((uint8_t)0x02)

/*! \def PAD_KEEPER
    \brief Pad keeper.
*/      
#define  PAD_KEEPER      			((uint8_t)0x03)

/*! \def PAD_DRV_STRENGTH_2MA
    \brief Pad driving current 2mA.
*/  
#define  PAD_DRV_STRENGTH_2MA     	((uint8_t)0x00)
#define  PAD_DRV_STRENGHT_2MA     	((uint8_t)0x00)

/*! \def PAD_DRV_STRENGTH_4MA
    \brief Pad driving current 4mA.
*/  
#define  PAD_DRV_STRENGTH_4MA     	((uint8_t)0x01)
#define  PAD_DRV_STRENGHT_4MA     	((uint8_t)0x01)
   
/*! \def PAD_DRV_STRENGTH_8MA
    \brief Pad driving current 8mA..
*/  
#define  PAD_DRV_STRENGTH_8MA     	((uint8_t)0x02)
#define  PAD_DRV_STRENGHT_8MA     	((uint8_t)0x02)
   
/*! \def PAD_DRV_STRENGTH_12MA
    \brief Pad driving current 12mA..
*/      
#define  PAD_DRV_STRENGTH_12MA     	((uint8_t)0x03)
#define  PAD_DRV_STRENGHT_12MA     	((uint8_t)0x03)

/*! \def PAD_SLEW_RATE_SLOW
    \brief Pad slew rate slow half of actual frequency.
*/   
#define  PAD_SLEW_RATE_SLOW     	((uint8_t)0x00)	/* Slow (half frequency) */

/*! \def PAD_SLEW_RATE_FAST
    \brief Pad slew rate fast.
*/      
#define  PAD_SLEW_RATE_FAST     	((uint8_t)0x01)

/*! \def PAD_SMT_TRIG_EN
    \brief Schmitt trigger enable for given pad.
*/      
#define  PAD_SMT_TRIG_EN     		((uint8_t)0x01)
   
/*! \def PAD_SMT_TRIG_DIS
    \brief Schmitt trigger disable for given pad.
*/      
#define  PAD_SMT_TRIG_DIS     		((uint8_t)0x00)

   
/*! \struct PadConfig
 *  \brief Pad Configuration structure that is filled up by programmer to select 
   pad function.
 */
 typedef struct
 {
	uint8_t ucPin;          /*!< Pad or Pin number starts from 0, Refer file eoss3_hal_pads.h macros PAD_0 to PAD_45*/

	uint32_t ucFunc;        /*!< Pad function selection listed in eoss3_hal_pad.h for each pad, each pad function listed with PAD0_FUNC_XXX to PAD45_FUNC_XXX*/

	uint8_t ucCtrl;         /*!< Pad controller A0 = Firmware; Other=M4 HW(Debugger, I2C) or FFE, FPGA */

	uint8_t ucMode;         /*!< Pad Output or Input function, OEN or REN*/

	uint8_t ucPull;         /*!< Pad Pull up config Z/up/down/keeper*/

	uint8_t ucDrv;          /*!< Pad Current driving strenght*/

	uint8_t ucSpeed;        /*!< Pad operating speed slow or fast*/

	uint8_t ucSmtTrg;       /*!< Pad configure for Schmitt trigger*/

 }PadConfig;

 /// @cond INTERNAL_STRUCT
 /*! \struct PadConfigReg
 *  \brief Pad Configuration register bit field to write directly in Pad config 
    register. This is for internal use, Programmer may not need to use it.
 */
 typedef struct
 {
	uint32_t bFunc:3;

	uint32_t bCtrl:2;

	uint32_t bOEn:1;

	uint32_t bOPull:2;

	uint32_t bODrv:2;

	uint32_t bSpeed:1;

	uint32_t bIEn:1;

	uint32_t bSmtTrg:1;
 }PadConfigReg;
/// @endcond

/*! \fn HAL_PAD_Config(PadConfig *pxPadInit)
    \brief Pad config function to configure pad functionality. This function
    need to be called with appropriate configuration before using it.

    \param *pxPadInit - Pad config structure data filled for given pad.
*/  
void HAL_PAD_Config(PadConfig *pxPadInit);

/*! \fn HAL_PAD_DeConfig(PadConfig *pxPadInit)
    \brief Pad config function to deconfigure of pad functionality and put that 
    pad in the default state.

    \param *pxPadInit - Pad config structure data filled for given pad.
*/
void HAL_PAD_DeConfig(PadConfig *pxPadInit);

/** Configures PADs using given configurations in `p_table`
 * @param p_table table of PadConfig structures
 * @param nitems  number of PadConfig structures to configure
 * 
 * @return None 
 */
void configure_s3_pads(PadConfig *p_table, int nitems);

/*! \fn HAL_PAD_Reconfig(PadConfig *pxPadInit)
    \brief Pad config function to re-configure pad functionality. This will 
     reset the set bits in so HAL_PAD_Config() can be called.

    \param *pxPadInit - Pad config structure data filled for given pad.
*/  
void HAL_PAD_Reconfig(PadConfig *pxPadInit);

#ifdef __cplusplus
}
#endif

#endif /* __EOSS3_HAL_PAD_AF_H */
