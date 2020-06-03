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

/*!	\file eoss3_hal_wb.h
 *
 *  Created on: March 11, 2016
 *      Author: Rajkumar Thiagarajan
 * 
 *  \brief This file contains macros and APIs
 *         for wishbone master inside FFE subsystem. 
 */

#ifndef HAL_INC_EOSS3_HAL_WB_H_
#define HAL_INC_EOSS3_HAL_WB_H_

#include <common.h>
#include <stdint.h>
#include <stddef.h>

#include "test_types.h"
#include "eoss3_hal_def.h"

/*!
 * \brief WB_ADDR register definition
 */
#define WB_ADDR_SPI0_SLAVE_SEL		((UINT8_t)(0x2 << BYTE_IDX_6))
#define WB_ADDR_I2C1_SLAVE_SEL		((UINT8_t)(0x1 << BYTE_IDX_6))
#define WB_ADDR_I2C0_SLAVE_SEL		((UINT8_t)(0x0 << BYTE_IDX_6))

/*!
 *\brief Wishbone Control and status register definition
 */
#define WB_CSR_SPI0MUX_SEL_WBMASTER		((UINT8_t)(1 << BYTE_IDX_7))
#define WB_CSR_SPI0MUX_SEL_SM1                  ((UINT8_t)(0 << BYTE_IDX_7))        
#define WB_CSR_I2C1MUX_SEL_WBMASTER             ((UINT8_t)(1 << BYTE_IDX_6))
#define WB_CSR_I2C1MUX_SEL_SM1			((UINT8_t)(0 << BYTE_IDX_6))
#define WB_CSR_I2C0MUX_SEL_WBMASTER             ((UINT8_t)(1 << BYTE_IDX_5))
#define WB_CSR_I2C0MUX_SEL_SM0			((UINT8_t)(0 << BYTE_IDX_5))

#define WB_CSR_MASTER_WR_EN			((UINT8_t)(1 << BYTE_IDX_1))
#define WB_CSR_MASTER_START			((UINT8_t)(1 << BYTE_IDX_0))
#define WB_CSR_BUSY				((UINT8_t)(1 << BYTE_IDX_3))
#define WB_CSR_OVFL				((UINT8_t)(1 << BYTE_IDX_4))

/*!
 * \fn		HAL_StatusTypeDef HAL_WB_Transmit(UINT8_t ucOffset, UINT8_t ucVal, UINT8_t ucSlaveSel)
 * \brief 	Function to send data over Wishbone interface
 * \param	ucOffset        --- Wishbone register offset
 * \param       ucVal           --- Data
 * \param       ucSlaveSel      --- Slave Select (I2C1 or I2C0 or SPI)
 * \return      HAL status
 */
HAL_StatusTypeDef HAL_WB_Transmit(UINT8_t ucOffset, UINT8_t ucVal, UINT8_t ucSlaveSel);
/*!
 * \fn		HAL_StatusTypeDef HAL_WB_Receive(UINT8_t ucOffset, UINT8_t *buf, UINT8_t ucSlaveSel)
 * \brief 	Function to read data over Wishbone interface
 * \param	ucOffset        --- Wishbone register offset
 * \param       ucVal           --- Data
 * \param       ucSlaveSel      --- Slave Select (I2C1 or I2C0 or SPI)
 * \return      HAL status
 */
HAL_StatusTypeDef HAL_WB_Receive(UINT8_t ucOffset, UINT8_t *buf, UINT8_t ucSlaveSel);
/*!
 * \fn		HAL_StatusTypeDef HAL_WB_Init(UINT8_t ucSlaveSel)
 * \brief 	Function to initialize Wishbone interface
 * \param       ucSlaveSel      --- Slave Select (I2C1 or I2C0 or SPI)
 * \return      HAL status
 */
HAL_StatusTypeDef HAL_WB_Init(UINT8_t ucSlaveSel);
/*!
 * \fn		HAL_StatusTypeDef HAL_WB_DeInit(UINT8_t ucSlaveSel)
 * \brief 	Function to De-initialize Wishbone interface
 * \param       ucSlaveSel      --- Slave Select (I2C1 or I2C0 or SPI)
 * \return      HAL status
 */
HAL_StatusTypeDef HAL_WB_DeInit(UINT8_t ucSlaveSel);

#endif /* HAL_INC_EOSS3_HAL_WB_H_ */
