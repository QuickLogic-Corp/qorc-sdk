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

#ifndef __EOSS3_HAL_I2S_DRV_H_
#define __EOSS3_HAL_I2S_DRV_H_
/*!	\file eoss3_hal_i2s_drv.h
 *
 *  \brief This file contains some of the common API's
 * or structures which need not be exposed to the i2s driver user
 * 
 */
#include <stdint.h>

/* Register function for Master Rx */
uint32_t HAL_I2S_Master_Assp_Register(void);

/* Register function for Slave Tx */
uint32_t HAL_I2S_Slave_Assp_Register(void);

/* Register function for Slave Rx */
uint32_t HAL_I2S_Slave_FB_Register(void);
/* I2S Slave Tx  SDMA 0 done handler*/
void HAL_I2S_SLAVE_SDMA_Assp_Done(void) ;

/* I2S Master Rx Block Done handler */
void HAL_I2S_Master_Assp_BlkDone_Hndlr(void);

/* I2S Master Rx Buffer Done handler */
void HAL_I2S_Master_Assp_BufDone_Hndlr(void);
#endif /* __EOSS3_HAL_I2S_DRV_H_ */
