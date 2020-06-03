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
 * tamar_hal_spi.h
 *
 *  Created on: Feb 23, 2015
 *      Author: Rajkumar Thiagarajan
 */

#ifndef TAMAR_HAL_SPI_H_
#define TAMAR_HAL_SPI_H_

#include <common.h>
#include <stdint.h>
#include <stddef.h>

#include "eoss3_hal_def.h"
#include "Alg_types.h"

#define assert_param(expr)  	((expr) ? (void)0 : assert_failed((UINT8_t*)__FILE__,__LINE__))


/*!
 * HAL SPI State structure definition
 */
typedef enum
{
	HAL_SPI_STATE_RESET=0,		/*! SPI not yet initialized or disabled */
	HAL_SPI_STATE_READY,		/*! SPI initialized and ready for use 	*/
	HAL_SPI_STATE_TX_BUSY,		/*! SPI Data transmission process is ongoing  */
	HAL_SPI_STATE_RX_BUSY,		/*! SPI Reception process is ongoing */
	HAL_SPI_STATE_TX_RX_BUSY,
	HAL_SPI_STATE_ERROR
}HAL_SPI_StateTypeDef;

typedef enum
{
	SPI_MODE_0 = 0,
	SPI_MODE_1,
	SPI_MODE_2,
	SPI_MODE_3,
	SPI_MODE_INVALID,
}SpiMode;

typedef enum {
	CMD_WithResponse = 0,
	CMD_NoResponse,
	READ_CMD,
	PROGRAM_CMD,
}FlashCmdType;


// Return Message
typedef enum {
    FlashOperationSuccess=1,
    FlashWriteRegFailed,
    FlashTimeOut,
    FlashIsBusy,
    FlashQuadNotEnable,
    FlashAddressInvalid,
    FlashInvalidParams,
    FlashCmdFailed,
    FlashMatchFound,
    FlashNotFound,
}ReturnMsg;


                                                                                        /*! Sensor SPI Master PAD selection.
											If ucSPI0PadSel = 0 then, SPI_MOSI--> PAD6, SPI_MISO--> PAD8, SPI_CLK--> PAD10, SPI_SSN1--> PAD9,
																	  SPI_SSN2--> PAD2, SPI_SSN3--> PAD4, SPI_SSN4--> PAD5, SPI_SSN5--> PAD7,
																	  SPI_SSN6--> PAD11, SPI_SSN7--> PAD12, SPI_SSN8--> PAD13

											If ucSPI0PadSel = 1 then, SPI_MOSI--> PAD28, SPI_MISO--> PAD29, SPI_CLK--> PAD31, SPI_SSN1--> PAD30,
																	  SPI_SSN2--> PAD36, SPI_SSN3--> PAD4, SPI_SSN4--> PAD26, SPI_SSN5--> PAD27,
																	  SPI_SSN6--> PAD33, SPI_SSN7--> PAD35, SPI_SSN8--> PAD37
											*/


/*!
 * \brief SPI Configuration Structure definition
 */
typedef struct
{
	UINT8_t  		ucSPIInf;			/*! SPI Interface : ucSPIInf = 0 (4-wire), ucSPIInf = 1 (3-wire) interface */
	SpiMode	 		ucSPIMode;			/*! SPI transfer mode, mode 0 - 3 */
	FlashCmdType	ucCmdType;			/*! SPI Flash command type */
	UINT8_t			ucSSn;				/*! SPI slave select pin */
	UINT32_t		ucFreq;				/*! SPI Communication Frequency */
	UINT32_t 		ulDataSize;			/*! Specifies the SPI data size.*/
	UINT32_t 		ulCLKPolarity;		/*! Specifies the serial clock steady state.*/
	UINT32_t 		ulCLKPhase;			/*! Specifies the clock active edge for the bit capture. */
	UINT32_t 		ulFirstBit;			/*! Specifies whether data transfers start from MSB or LSB bit. */
    uint8_t         fRawMode;           /*! Tim: turns off wierd behavior */
}SPI_InitTypeDef;


/*!
 * \brief SPI Handle structure definition
 */
typedef struct __SPI_HandleTypeDef
{
	SPI_InitTypeDef 			Init;			/*! SPI Communication Parameters */
	UINT8_t 					ucSPIx;			/*! SPI Master index for base address*/
	UINT8_t 					*pTxBuffer;		/*! Pointer to SPI Tx transfer Buffer */
	UINT8_t 					*pRxBuffer;		/*! Pointer to SPI Rx transfer Buffer */
	UINT32_t 					usTxXferCount;	/*! SPI Tx total transfer count */
	UINT32_t 					usRxXferCount;	/*! SPI Rx total transfer count */
	UINT32_t 					usTxXferSize;	/*! SPI Tx transfer size for each transfer*/
	UINT32_t 					usRxXferSize;	/*! SPI Rx transfer size */
	HAL_SPI_StateTypeDef  		State;        	/*! SPI communication state */
	void 						(*RxISR)(struct __SPI_HandleTypeDef *spi);	/*! function pointer on Rx ISR */
	void 						(*TxISR)(struct __SPI_HandleTypeDef *spi);	/*! function pointer on Tx ISR */
	void 						(*HAL_SPI_TxRxComplCallback)(void);	/*! function pointer on Tx/Rx complete Callback */
}SPI_HandleTypeDef;


/* SPI Transaction size bit definition */
#define SPI_DATASIZE_8BIT			((UINT8_t)0x7)
#define SPI_DATASIZE_7BIT			((UINT8_t)0x6)
#define SPI_DATASIZE_6BIT			((UINT8_t)0x5)
#define SPI_DATASIZE_5BIT			((UINT8_t)0x4)
#define SPI_DATASIZE_4BIT			((UINT8_t)0x3)
#define SPI_DATASIZE_3BIT			((UINT8_t)0x2)
#define SPI_DATASIZE_2BIT			((UINT8_t)0x1)
#define SPI_DATASIZE_1BIT			((UINT8_t)0x0)
#define IS_SPI_DATASIZE(DATASIZE)	((DATASIZE == SPI_DATASIZE_8BIT) || (DATASIZE == SPI_DATASIZE_7BIT) || \
									 (DATASIZE == SPI_DATASIZE_6BIT) || (DATASIZE == SPI_DATASIZE_5BIT) || \
									 (DATASIZE == SPI_DATASIZE_4BIT) || (DATASIZE == SPI_DATASIZE_3BIT) || \
									 (DATASIZE == SPI_DATASIZE_2BIT) || (DATASIZE == SPI_DATASIZE_1BIT))


/* SPI Clock Polarity */
#define SPI_POLARITY_LOW			((UINT8_t)0x0)
#define SPI_POLARITY_HIGH			((UINT8_t)0x1)
#define IS_SPI_CPOL(CPOL)			((CPOL == SPI_POLARITY_LOW) || (CPOL == SPI_POLARITY_HIGH))

/* SPI Clock Phase */
#define SPI_PHASE_1EDGE				((UINT8_t)0x0)
#define SPI_PHASE_2EDGE				((UINT8_t)0x1)
#define IS_SPI_CPHA(CPHA)			((CPHA == SPI_PHASE_1EDGE) || (CPHA == SPI_PHASE_2EDGE))

/* SPI_MSB_LSB_transmission */
#define SPI_FIRSTBIT_MSB			((UINT8_t)0x0)
#define SPI_FIRSTBIT_LSB			((UINT8_t)0x1)
#define IS_SPI_FIRST_BIT(BIT)		((BIT == SPI_FIRSTBIT_MSB) || (BIT == SPI_FIRSTBIT_LSB))

/* SPI 3-wire configuration */
#define SPI_BIDIR_MODE_DIS			((UINT8_t)0x0)
#define SPI_BIDIR_MODE_EN			((UINT8_t)0x1)
#define IS_SPI_BIDIR_MODE(MODE)		((MODE == SPI_BIDIR_MODE_DIS) || (MODE == SPI_BIDIR_MODE_EN))

#define SPI_3_WIRE_MODE				(0x1)
#define SPI_4_WIRE_MODE				(0x0)

/* SPI Interrupt/Status register bit definition */
#define SPI_XFER_IN_PROGRESS		((UINT8_t)0x4)
#define SPI_WRITE_XFER_DONE			((UINT8_t)0x2)
#define SPI_READ_XFER_DONE			((UINT8_t)0x1)

/* SPI Slave select register bit definition */
#define SPI_SLAVE_1_SELECT			((UINT8_t)0x1 << BYTE_IDX_0)
#define SPI_SLAVE_2_SELECT			((UINT8_t)0x1 << BYTE_IDX_1)
#define SPI_SLAVE_3_SELECT			((UINT8_t)0x1 << BYTE_IDX_2)
#define SPI_SLAVE_4_SELECT			((UINT8_t)0x1 << BYTE_IDX_3)
#define SPI_SLAVE_5_SELECT			((UINT8_t)0x1 << BYTE_IDX_4)
#define SPI_SLAVE_6_SELECT			((UINT8_t)0x1 << BYTE_IDX_5)
#define SPI_SLAVE_7_SELECT			((UINT8_t)0x1 << BYTE_IDX_6)
#define SPI_SLAVE_8_SELECT			((UINT8_t)0x1 << BYTE_IDX_7)

#define IS_SPI_SSN_VALID(SS)		((SS == SPI_SLAVE_1_SELECT) || (SS == SPI_SLAVE_2_SELECT) || (SS == SPI_SLAVE_3_SELECT) || \
									 (SS == SPI_SLAVE_4_SELECT) || (SS == SPI_SLAVE_5_SELECT) || (SS == SPI_SLAVE_6_SELECT) || \
									 (SS == SPI_SLAVE_7_SELECT)	||	(SS == SPI_SLAVE_8_SELECT))


#define IS_SPIx_VALID(n)			((n == SPI0_MASTER_SEL) || (n == SPI1_MASTER_SEL))

/*! \def SPI0_MASTER_SEL
    \brief A macro to select Sensor SPI Master controller in FFE subsystem
*/
#define SPI0_MASTER_SEL				0

/*! \def SPI1_MASTER_SEL
    \brief A macro to select SPI Master controller connected to SPI Flash
*/
#define SPI1_MASTER_SEL				1

/*! \def I2C_0_SELECT
    \brief A macro to select I2C_0 as slave to be accessed by WB master
*/
#define I2C_0_SELECT				((UINT8_t)0x0)

/*! \def I2C_1_SELECT
    \brief A macro to select I2C_1 as slave to be accessed by WB master
*/
#define I2C_1_SELECT				((UINT8_t)0x40)

/*! \def SPI_0_SELECT
    \brief A macro to select SPI_0 as slave to be accessed by WB master
*/
#define SPI_0_SELECT				((UINT8_t)0x80)

/*! \def SPI0_MUX_SEL_WB_MASTER
    \brief A macro to define SPI_0 wishbone control mux select
*/
#define SPI0_MUX_SEL_WB_MASTER		((UINT8_t)0x80)

/*!
 * \brief The following macros define the Sensor SPI Master register offsets
 */
#define SPI0_BR_LSB_REG				0x0
#define SPI0_BR_MSB_REG				0x1
#define SPI0_CFG_REG				0x2
#define SPI0_TX_RX_REG				0x3
#define SPI0_CMD_STS_REG			0x4
#define SPI0_SS_REG				0x5
#define SPI0_CLK_CTRL_REG			0x6
#define SPI0_ADD_CLK_REG			0x7

#define SPI_BAUDRATE_625KHZ			((UINT32_t)625000)
#define SPI_BAUDRATE_1MHZ			((UINT32_t)1000000)
#define SPI_BAUDRATE_2_5MHZ			((UINT32_t)2500000)
#define SPI_BAUDRATE_5MHZ			((UINT32_t)5000000)
#define SPI_BAUDRATE_10MHZ			((UINT32_t)10000000)
#define SPI_BAUDRATE_15MHZ                      ((UINT32_t)15000000)
#define SPI_BAUDRATE_20MHZ                      ((UINT32_t)20000000)
   

/*!
 * \brief SPI command register (offset 0x4) bit definition
 */
#define SPI_CMD_START				((UINT8_t)(1 << BYTE_IDX_0))
#define SPI_CMD_STOP				((UINT8_t)(1 << BYTE_IDX_1))
#define SPI_CMD_WRITE				((UINT8_t)(1 << BYTE_IDX_2))
#define SPI_CMD_READ				((UINT8_t)(1 << BYTE_IDX_3))
#define SPI_CMD_IACK				((UINT8_t)(1 << BYTE_IDX_7))

/*!
 * \brief SPI Interrupt/Status register bit definition
 */
#define SPI_STAT_TIP				((UINT8_t) (1 << BYTE_IDX_2))
#define SPI_INTR_IW				((UINT8_t) (1 << BYTE_IDX_1))
#define SPI_INTR_IR				((UINT8_t) (1 << BYTE_IDX_0))


/*!
 * \brief SPI configuration register (offset 0x2) bit definition
 */
#define SPI_SYSTEM_EN				((UINT8_t)(1 << BYTE_IDX_7))
#define SPI_INTR_EN				((UINT8_t)(1 << BYTE_IDX_6))

#define SPI_MS_INTR_EN				((UINT32_t)0x40000)

#define SPI1_XFER_LEN_MAX			260

/********************** Exported Functions ********************/
HAL_SPI_StateTypeDef HAL_SPI_GetState(SPI_HandleTypeDef *hspi);
void HAL_SPI_IRQHandler(void);
HAL_StatusTypeDef HAL_SPI_Init(SPI_HandleTypeDef  *hspi);

HAL_StatusTypeDef HAL_SPI_DeInit(SPI_HandleTypeDef  *hspi);

HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef  *hspi, UINT8_t *pData, UINT32_t ulTxLen);

HAL_StatusTypeDef HAL_SPI_TransmitReceive(SPI_HandleTypeDef *hspi, UINT8_t *pTxData, const UINT32_t usTxSize,UINT8_t *pRxData, const UINT32_t usRxSize,void (*HAL_SPI_TxRxComplCallback)(void));

HAL_StatusTypeDef HAL_SPI_Transmit_IT(SPI_HandleTypeDef *hspi, UINT8_t *pData, UINT32_t ulTxLen, void (*HAL_SPI_Callback)(void));

HAL_StatusTypeDef HAL_SPI_TransmitReceive_IT(SPI_HandleTypeDef *hspi, UINT8_t *pTxData, UINT32_t usTxSize, UINT8_t *pRxData,UINT32_t usRxSize,
											void (*HAL_SPI_TxRxComplCallback)(void));

//void SPI_DMA_Complete(SPI_HandleTypeDef  *hspi);
void SPI_DMA_Complete(void);



#endif /* TAMAR_HAL_SPI_H_ */
