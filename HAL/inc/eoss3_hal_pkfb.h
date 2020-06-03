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

/*!	\file eoss3_hal_pkfb.h
 *  Author: Ketan Patel <kpatel@quicklogic.com>
 *
 * Date 25th Jan, 2016
 *
 * \brief This file contains macros, structures and APIs for
          configuration, read and write to FIFOs available in
		  system.
 */

#ifndef __EOSS3_HAL_PKFB_H_
#define __EOSS3_HAL_PKFB_H_

#include "eoss3_dev.h"
#include "test_types.h"
#include "eoss3_hal_def.h"

 /******************************************************************************
  * 						    Packet FIFO                                    *
  ******************************************************************************/
/*!	\enum FIFO_Type
	\brief FIFO types available
*/
typedef enum
{
    FIFO0 = 0,
    FIFO1,
    FIFO2,
    FIFO8K,
    FIFO_INVALID = -1
}FIFO_Type;

/*!	\enum FIFO_SrcType
	\brief FIFO sources that can push data to FIFO
*/
typedef enum
{
	FIFO_SRC_M4 = 0,
	FIFO_SRC_FFE0,
	FIFO_SRC_FFE1,
	FIFO_SRC_INVALID = -1
} FIFO_SrcType;

/*!	\enum FIFO_DestType
	\brief FIFO destinations that can pop data from FIFO
*/
typedef enum
{
	FIFO_DEST_M4 = 0,
	FIFO_DEST_AP,
	FIFO_DEST_INVALID = -1,
} FIFO_DestType;

/*!	\enum FIFO_PushIntType
	\brief FIFO interrupt configuration for Push side
*/
typedef enum
{
	FIFO_PUSH_INT_OVERFLOW = 0x1,
	FIFO_PUSH_INT_THRESHOLD = 0x2,
	FIFO_PUSH_INT_SRAM_SLEEP = 0x4,
	FIFO_PUSH_INT_INVALID = -1
}FIFO_PushIntType;

/*!	\enum FIFO_PopIntType
	\brief FIFO interrupt configuration for Pop side
*/typedef enum
{
	FIFO_POP_INT_UNDERFLOW = 0x1,
	FIFO_POP_INT_THRESHOLD = 0x2,
	FIFO_POP_INT_SRAM_SLEEP = 0x4,
	FIFO_POP_INT_INVALID = -1
}FIFO_PopIntType;


/*! \def FIFO_WORD_WIDTH
    \brief A macro to define FIFO0, FIFO1 and FIFO2 WORD Width.
*/
#define FIFO_WORD_WIDTH				   4 		//4 Bytes or 32 bits

/*! \def FIFO8X_WORD_WIDTH
    \brief A macro to define 8K FIFO WORD Width.
*/
#define FIFO8X_WORD_WIDTH			   4 		//4 Bytes or 32 bits

/*! \def FIFO0_SZ
    \brief A macro to define FIFO 0 size in words.
*/
#define FIFO0_SZ                      (256)

/*! \def FIFO1_SZ
    \brief A macro to define FIFO 1 size in words.
*/
#define FIFO1_SZ                      (128)

/*! \def FIFO2_SZ
    \brief A macro to define FIFO 2 size in words.
*/
#define FIFO2_SZ                      (128)

/*! \def FIFO8K_SZ
    \brief A macro to define FIFO 8K size in words.
*/
#define FIFO8K_SZ                     (8*1024)

///@cond PKFB_MACROS
/******************************************************************************
 *                                   PKFB                                     *
 ******************************************************************************/
/* FIFO interrupt configuration bit definition */
#define	FIFO_NO_INT                   (0x00)
#define	FIFO_PUSH_OVERFLOW_INT        (0x01)
#define	FIFO_PUSH_THRESH_INT          (0x02)
#define	FIFO_PUSH_SLEEP_INT           (0x04)
#define	FIFO_POP_UNDERFLOW_INT        (0x08)
#define	FIFO_POP_THRESH_INT           (0x10)
#define	FIFO_POP_SLEEP_INT            (0x20)

/* Macros and bit definition for FIFO control register */

#define FIFO_CTRL_ENABLE                               ((uint32_t) (0x00000000))
#define FIFO_CTRL_PUSH_MUX                             ((uint32_t) (0x00000001))
#define FIFO_CTRL_POP_MUX                              ((uint32_t) (0x00000002))
#define FIFO_CTRL_PUSH_INT_MUX                         ((uint32_t) (0x00000003))
#define FIFO_CTRL_POP_INT_MUX                          ((uint32_t) (0x00000004))
#define FIFO_CTRL_FFE_SEL                              ((uint32_t) (0x00000005))

#define FIFOx_CTRL_ENABLE(FIFO_INDEX)                  (PKFB->PKFB_FIFOCTRL |= 1<<((FIFO_INDEX * 8) + FIFO_CTRL_ENABLE))
#define FIFOx_CTRL_DISABLE(FIFO_INDEX)                 (PKFB->PKFB_FIFOCTRL &= (~1<<((FIFO_INDEX * 8)+ FIFO_CTRL_ENABLE)))

#define FIFOx_CTRL_PUSH_FFE(FIFO_INDEX)                (PKFB->PKFB_FIFOCTRL |= 1<<((FIFO_INDEX * 8) + FIFO_CTRL_PUSH_MUX))
#define FIFOx_CTRL_PUSH_M4(FIFO_INDEX)                 (PKFB->PKFB_FIFOCTRL &= ~(1<<((FIFO_INDEX * 8) + FIFO_CTRL_PUSH_MUX))); (PKFB->PKFB_FIFOCTRL &= ~(1<<((FIFO_INDEX * 8) + FIFO_CTRL_POP_INT_MUX)))

#define FIFOx_CTRL_POP_AP(FIFO_INDEX)                  (PKFB->PKFB_FIFOCTRL |= 1<<((FIFO_INDEX * 8) + FIFO_CTRL_POP_MUX)); (PKFB->PKFB_FIFOCTRL |= 1<<((FIFO_INDEX * 8) + FIFO_CTRL_PUSH_INT_MUX))
#define FIFOx_CTRL_POP_M4(FIFO_INDEX)                  (PKFB->PKFB_FIFOCTRL &= ~(1<<((FIFO_INDEX * 8) + FIFO_CTRL_POP_MUX))); (PKFB->PKFB_FIFOCTRL &= ~(1<<((FIFO_INDEX * 8) + FIFO_CTRL_PUSH_INT_MUX)))

#define FIFOx_CTRL_FFE0_SEL(FIFO_INDEX)                (FIFOx_CTRL_PUSH_FFE(FIFO_INDEX)); (PKFB->PKFB_FIFOCTRL &= ~(1<<((FIFO_INDEX * 8) + FIFO_CTRL_FFE_SEL)))
#define FIFOx_CTRL_FFE1_SEL(FIFO_INDEX)                (FIFOx_CTRL_PUSH_FFE(FIFO_INDEX)); (PKFB->PKFB_FIFOCTRL |= (1<<((FIFO_INDEX * 8) + FIFO_CTRL_FFE_SEL)))

#define FIFOx_CTRL_RESET(FIFO_INDEX)                   (PKFB->PKFB_FIFOCTRL &= ~(0xFF<<(FIFO_INDEX * 8)))

/* Macros and bit definition for FIFO Status register */
#define FIFO_STATUS_SRAM_SLEEP_ON                      (0x00000000)
#define FIFO_STATUS_SRAM_SLEEP_LS                      (0x00000001)
#define FIFO_STATUS_SRAM_SLEEP_DS                      (0x00000002)
#define FIFO_STATUS_SRAM_SLEEP_SD                      (0x00000003)
#define FIFO_STATUS_PUSH_INT_OVER                      (0x00000004)
#define FIFO_STATUS_PUSH_INT_THRESH                    (0x00000008)
#define FIFO_STATUS_PUSH_INT_SLEEP                     (0x00000010)
#define FIFO_STATUS_POP_INT_UNDER                      (0x00000020)
#define FIFO_STATUS_POP_INT_THRESH                     (0x00000040)
#define FIFO_STATUS_POP_INT_SLEEP                      (0x00000080)


#define FIFOx_STATUS_SRAM_SLEEP(FIFO_INDEX)            (PKFB->PKFB_FIFOSTATUS & (FIFO_STATUS_SRAM_SLEEP_SD)<<(FIFO_INDEX * 8))

#define IS_FIFOx_STATUS_SRAM_ACTIVE(FIFO_INDEX)        (!FIFOx_STATUS_SRAM_SLEEP(FIFO_INDEX)?1:0)
#define IS_FIFOx_STATUS_SRAM_LS(FIFO_INDEX)            (FIFOx_STATUS_SRAM_SLEEP(FIFO_INDEX) == (FIFO_STATUS_SRAM_SLEEP_LS)<<(FIFO_INDEX * 8)? 1:0)
#define IS_FIFOx_STATUS_SRAM_DS(FIFO_INDEX)            (FIFOx_STATUS_SRAM_SLEEP(FIFO_INDEX) == (FIFO_STATUS_SRAM_SLEEP_DS)<<(FIFO_INDEX * 8)? 1:0)
#define IS_FIFOx_STATUS_SRAM_SD(FIFO_INDEX)            (FIFOx_STATUS_SRAM_SLEEP(FIFO_INDEX) == (FIFO_STATUS_SRAM_SLEEP_SD)<<(FIFO_INDEX * 8)? 1:0)


#define FIFOx_STATUS_CLEAR_PUSH_INT_OVER(FIFO_INDEX)   (PKFB->PKFB_FIFOSTATUS |= (FIFO_STATUS_PUSH_INT_OVER)<<(FIFO_INDEX * 8))
#define FIFOx_STATUS_READ_PUSH_INT_OVER(FIFO_INDEX)    (PKFB->PKFB_FIFOSTATUS & (FIFO_STATUS_PUSH_INT_OVER)<<(FIFO_INDEX * 8))

#define FIFOx_STATUS_CLEAR_PUSH_INT_THRESH(FIFO_INDEX) (PKFB->PKFB_FIFOSTATUS |= (FIFO_STATUS_PUSH_INT_THRESH)<<(FIFO_INDEX * 8))
#define FIFOx_STATUS_READ_PUSH_INT_THRESH(FIFO_INDEX)  (PKFB->PKFB_FIFOSTATUS & (FIFO_STATUS_PUSH_INT_THRESH)<<(FIFO_INDEX * 8))

#define FIFOx_STATUS_CLEAR_PUSH_INT_SLEEP(FIFO_INDEX)  (PKFB->PKFB_FIFOSTATUS |= (FIFO_STATUS_PUSH_INT_SLEEP)<<(FIFO_INDEX * 8))
#define FIFOx_STATUS_READ_PUSH_INT_SLEEP(FIFO_INDEX)   (PKFB->PKFB_FIFOSTATUS & (FIFO_STATUS_PUSH_INT_SLEEP)<<(FIFO_INDEX * 8))

#define FIFOx_STATUS_CLEAR_POP_INT_UNDER(FIFO_INDEX)   (PKFB->PKFB_FIFOSTATUS |= (FIFO_STATUS_POP_INT_UNDER)<<(FIFO_INDEX * 8))
#define FIFOx_STATUS_READ_POP_INT_UNDER(FIFO_INDEX)    (PKFB->PKFB_FIFOSTATUS & (FIFO_STATUS_POP_INT_UNDER)<<(FIFO_INDEX * 8))

#define FIFOx_STATUS_CLEAR_POP_INT_THRESH(FIFO_INDEX)  (PKFB->PKFB_FIFOSTATUS |= (FIFO_STATUS_POP_INT_THRESH)<<(FIFO_INDEX * 8))
#define FIFOx_STATUS_READ_POP_INT_THRESH(FIFO_INDEX)   (PKFB->PKFB_FIFOSTATUS & (FIFO_STATUS_POP_INT_THRESH)<<(FIFO_INDEX * 8))

#define FIFOx_STATUS_CLEAR_POP_INT_SLEEP(FIFO_INDEX)   (PKFB->PKFB_FIFOSTATUS |= (FIFO_STATUS_POP_INT_SLEEP)<<(FIFO_INDEX * 8))
#define FIFOx_STATUS_READ_POP_INT_SLEEP(FIFO_INDEX)    (PKFB->PKFB_FIFOSTATUS & (FIFO_STATUS_POP_INT_SLEEP)<<(FIFO_INDEX * 8))

/* Macros and bit definition for FIFO PUSH control register */
#define FIFO_SLEEP_EN                                  (0)
#define FIFO_SLEEP_TYPE                                (1)
#define FIFO_INT_OVER_UNDER                            (2)
#define FIFO_INT_THRESH                                (3)
#define FIFO_INT_SRAM_SLEEP                            (4)
#define FIFO_THRESH                                    (16)

#define FIFOx_PUSH_SLEEP_ENABLE(FIFO_INDEX)            (*(&PKFB->PKFB_PF0PUSHCTRL+ FIFO_INDEX*0x4) |= 1<<FIFO_SLEEP_EN)
#define FIFOx_PUSH_SLEEP_DISABLE(FIFO_INDEX)           (*(&PKFB->PKFB_PF0PUSHCTRL+ FIFO_INDEX*0x4) &= ~(1<<FIFO_SLEEP_EN))

//#define FIFOx_PUSH_SLEEP_DS(FIFO_INDEX)                (*(&PKFB->PKFB_PF0PUSHCTRL+ FIFO_INDEX*0x4) &= ~(1<<FIFO_SLEEP_TYPE))
//#define FIFOx_PUSH_SLEEP_SD(FIFO_INDEX)                (*(&PKFB->PKFB_PF0PUSHCTRL+ FIFO_INDEX*0x4) |= (1<<FIFO_SLEEP_TYPE))

#define FIFOx_PUSH_INT_OVER_ENABLE(FIFO_INDEX)         (*(&PKFB->PKFB_PF0PUSHCTRL+ FIFO_INDEX*0x4) |= (1<<FIFO_INT_OVER_UNDER))
#define FIFOx_PUSH_INT_OVER_DISABLE(FIFO_INDEX)        (*(&PKFB->PKFB_PF0PUSHCTRL+ FIFO_INDEX*0x4) &= ~(1<<FIFO_INT_OVER_UNDER))

#define FIFOx_PUSH_INT_THRESH_ENABLE(FIFO_INDEX)       (*(&PKFB->PKFB_PF0PUSHCTRL+ FIFO_INDEX*0x4) |= (1<<FIFO_INT_THRESH))
#define FIFOx_PUSH_INT_THRESH_DISABLE(FIFO_INDEX)      (*(&PKFB->PKFB_PF0PUSHCTRL+ FIFO_INDEX*0x4) &= ~(1<<FIFO_INT_THRESH))

#define FIFOx_PUSH_INT_SRAM_SLEEP_ENABLE(FIFO_INDEX)   (*(&PKFB->PKFB_PF0PUSHCTRL+ FIFO_INDEX*0x4) |= (1<<FIFO_INT_SRAM_SLEEP))
#define FIFOx_PUSH_INT_SRAM_SLEEP_DISABLE(FIFO_INDEX)  (*(&PKFB->PKFB_PF0PUSHCTRL+ FIFO_INDEX*0x4) &= ~(1<<FIFO_INT_SRAM_SLEEP))

#define FIFOx_PUSH_THRESHOLD(FIFO_INDEX, COUNT)        (*(&PKFB->PKFB_PF0PUSHCTRL+ FIFO_INDEX*0x4) |= (COUNT<<FIFO_THRESH))
#define FIFOx_PUSH_THRESHOLD_RESET(FIFO_INDEX)         (*(&PKFB->PKFB_PF0PUSHCTRL+ FIFO_INDEX*0x4) &= ~(0x1FF<<FIFO_THRESH))

#define FIFOx_PUSH_CTRL_RESET(FIFO_INDEX)              (*(&PKFB->PKFB_PF0PUSHCTRL+ FIFO_INDEX*0x4) = 0)

/* Macros and bit definition for FIFO POP control register */
#define FIFOx_POP_SLEEP_ENABLE(FIFO_INDEX)             (*(&PKFB->PKFB_PF0POPCTRL+ FIFO_INDEX*0x4) |= 1<<FIFO_SLEEP_EN)
#define FIFOx_POP_SLEEP_DISABLE(FIFO_INDEX)            (*(&PKFB->PKFB_PF0POPCTRL+ FIFO_INDEX*0x4) &= ~(1<<FIFO_SLEEP_EN))

#define FIFOx_POP_SLEEP_DS(FIFO_INDEX)                 (*(&PKFB->PKFB_PF0POPCTRL+ FIFO_INDEX*0x4) &= ~(1<<FIFO_SLEEP_TYPE))
#define FIFOx_POP_SLEEP_SD(FIFO_INDEX)                 (*(&PKFB->PKFB_PF0POPCTRL+ FIFO_INDEX*0x4) |= (1<<FIFO_SLEEP_TYPE))

#define FIFOx_POP_INT_OVER_ENABLE(FIFO_INDEX)          (*(&PKFB->PKFB_PF0POPCTRL+ FIFO_INDEX*0x4) |= (1<<FIFO_INT_OVER_UNDER))
#define FIFOx_POP_INT_OVER_DISABLE(FIFO_INDEX)         (*(&PKFB->PKFB_PF0POPCTRL+ FIFO_INDEX*0x4) &= ~(1<<FIFO_INT_OVER_UNDER))

#define FIFOx_POP_INT_THRESH_ENABLE(FIFO_INDEX)        (*(&PKFB->PKFB_PF0POPCTRL+ FIFO_INDEX*0x4) |= (1<<FIFO_INT_THRESH))
#define FIFOx_POP_INT_THRESH_DISABLE(FIFO_INDEX)       (*(&PKFB->PKFB_PF0POPCTRL+ FIFO_INDEX*0x4) &= ~(1<<FIFO_INT_THRESH))

#define FIFOx_POP_INT_SRAM_SLEEP_ENABLE(FIFO_INDEX)    (*(&PKFB->PKFB_PF0POPCTRL+ FIFO_INDEX*0x4) |= (1<<FIFO_INT_SRAM_SLEEP))
#define FIFOx_POP_INT_SRAM_SLEEP_DISABLE(FIFO_INDEX)   (*(&PKFB->PKFB_PF0POPCTRL+ FIFO_INDEX*0x4) &= ~(1<<FIFO_INT_SRAM_SLEEP))

#define FIFOx_POP_THRESHOLD(FIFO_INDEX, COUNT)         (*(&PKFB->PKFB_PF0POPCTRL+ FIFO_INDEX*0x4) |= (COUNT<<FIFO_THRESH))
#define FIFOx_POP_THRESHOLD_RESET(FIFO_INDEX)          (*(&PKFB->PKFB_PF0POPCTRL+ FIFO_INDEX*0x4) &= ~(0x1FF<<FIFO_THRESH))

#define FIFOx_POP_RESET(FIFO_INDEX)                    (*(&PKFB->PKFB_PF0POPCTRL+ FIFO_INDEX*0x4) = 0)

/* Macros and bit definition for FIFO count register */
#define FIFO_COUNT                                     (0x000001FF)
#define FIFO_PUSH_OFFSET                               (16)
#define FIFO_EMPTY_MASK                                (15)
#define FIFO_FULL_MASK                                 (31)


#define FIFOx_GET_POP_COUNT(FIFO_INDEX)                (*(&PKFB->PKFB_PF0CNT + FIFO_INDEX*0x4) & FIFO_COUNT)
#define FIFOx_GET_PUSH_COUNT(FIFO_INDEX)               ((*(&PKFB->PKFB_PF0CNT + FIFO_INDEX*0x4)>>FIFO_PUSH_OFFSET) & FIFO_COUNT)

#define IS_FIFO_EMPTY(FIFO_INDEX)                      (*(&PKFB->PKFB_PF0CNT + FIFO_INDEX*0x4) & (1<<FIFO_EMPTY_MASK))
#define IS_FIFO_FULL(FIFO_INDEX)                       (*(&PKFB->PKFB_PF0CNT + FIFO_INDEX*0x4) & (1<<FIFO_FULL_MASK))

/* Macros for FIFO data register */
#define FIFOx_DATA_WRITE(FIFO_INDEX, VALUE)            (*(&PKFB->PKFB_PF0DATA + FIFO_INDEX*0x4) |= VALUE)
#define FIFOx_DATA_READ(FIFO_INDEX)                    (*(&PKFB->PKFB_PF0DATA + FIFO_INDEX*0x4))

/* Add FIFO collision macros and bit definitions */

///@endcond PKFB_MACROS

/*! \struct FIFO_Config eoss3_hal_pkfb.h "inc/eoss3_hal_pkfb.h"
 * 	\brief FIFO configuration for PUSH source and POP destination.
 */
typedef struct
{
	FIFO_Type     eFifoID;                              /*!< FIFO number to configure */
	FIFO_SrcType  eSrc;                                 /*!< FIFO PUSH source */
	FIFO_DestType eDest;                                /*!< FIFO POP destination */
}FIFO_Config;

/*! \struct FIFO_IntConfig eoss3_hal_pkfb.h "inc/eoss3_hal_pkfb.h"
 * 	\brief FIFO interrupt configuration for PUSH and POP side.
 */
typedef struct
{
	FIFO_DestType    ePushIntMux;                       /*!< FIFO owner that should receive PUSH interrupts */
	FIFO_DestType    ePopIntMux;                        /*!< FIFO owner that should receive POP interrupts */
	FIFO_PushIntType ePushIntType;                      /*!< FIFO PUSH side interrupts type */
	FIFO_PushIntType ePopIntType;                       /*!< FIFO POP side interrupt types */
	UINT32_t         uiPushThresholdCount;              /*!< Threshold in words to generate PUSH side interrupt */
	UINT32_t         uiPopThresholdCount;               /*!< Threshold in workds to generate POP side interrupt */
}FIFO_IntConfig;

/* Exported functions --------------------------------------------------------*/
/*! \fn void HAL_FIFO_Enable(FIFO_Type eFifoID)
 *  \brief Enable FIFO for PUSH/POP operation.
 *
 *  \param eFifoID             FIFO ID to enable
 */
void HAL_FIFO_Enable(FIFO_Type eFifoID);


/*! \fn void HAL_FIFO_Disable(FIFO_Type eFifoID)
 *  \brief Enable FIFO for PUSH/POP operation.
 *
 *  \param eFifoID             FIFO ID to disable
 */
void HAL_FIFO_Disable(FIFO_Type eFifoID);


/*! \fn UINT32_t HAL_FIFO_Read(FIFO_Type eFifoID, INT32_t iLen, UINT32_t *puiRxBuf)
 *  \brief Read data from FIFO.
 * 
 *  \param eFifoID             FIFO ID to read from
 *  \param iLen                number of 32 bit words to read
 *  \param puiRxBuf            pointer to buffer where data has to be read
 *  \return UINT32_t            Number of 32-bit words read from FIFO
 */
UINT32_t HAL_FIFO_Read(FIFO_Type eFifoID, INT32_t iLen, UINT32_t *puiRxBuf);


/*! \fn UINT32_t HAL_FIFO8K_Read(FIFO_Type eFifoID, INT32_t iLen, UINT16_t *pusRxBuf)
 *  \brief Read data from 8K FIFO.
 * 
 *  \param eFifoID             FIFO ID of 8K FIFO
 *  \param iLen                number of 16 bit words to read
 *  \param pusRxBuf            pointer to buffer where data has to be read
 *  \return UINT32_t            Number of 16-bit words read from FIFO
 */
UINT32_t HAL_FIFO8K_Read(FIFO_Type eFifoID, INT32_t iLen, UINT16_t *pusRxBuf);


/*! \fn UINT32_t HAL_FIFO_Write(FIFO_Type eFifoID, INT32_t iLen, UINT32_t *puiTxBuf)
 *  \brief Write data to FIFO.
 * 
 *  \param eFifoID             FIFO ID to write
 *  \param iLen                number of 32 bit words to write
 *  \param puiTxBuf            pointer to buffer with data to be written to FIFO
 *  \return UINT32_t            Number of 32-bit words written to FIFO
 */
UINT32_t HAL_FIFO_Write(FIFO_Type eFifoID, INT32_t iLen, UINT32_t *puiTxBuf);


/*! \fn UINT32_t HAL_FIFO8K_Write(FIFO_Type eFifoID, INT32_t iLen, UINT16_t *pusTxBuf)
 *  \brief Write data to 8K FIFO.
 * 
 *  \param eFifoID             FIFO ID of 8K FIFO to write
 *  \param iLen                number of 16 bit words to write
 *  \param pusTxBuf            pointer to buffer with data to be written to FIFO
 *  \return UINT32_t           Number of 16-bit words written to FIFO
 */
UINT32_t HAL_FIFO8K_Write(FIFO_Type eFifoID, INT32_t iLen, UINT16_t *pusTxBuf);


/*! \fn void HAL_FIFO_PushSleepEnable(FIFO_Type eFifoID)
 *  \brief Enable sleep mode on PUSH side of FIFO.
 *
 *  \param eFifoID             FIFO ID to enable PUSH side sleep mode
 */
void HAL_FIFO_PushSleepEnable(FIFO_Type eFifoID);


/*! \fn void HAL_FIFO_PushSleepDisable(FIFO_Type eFifoID)
 *  \brief Disable sleep mode on PUSH side of FIFO.
 *
 *  \param eFifoID             FIFO ID to disable PUSH side sleep mode
 */
void HAL_FIFO_PushSleepDisable(FIFO_Type eFifoID);


/*! \fn void HAL_FIFO_PopSleepEnable(FIFO_Type eFifoID)
 *  \brief Enable sleep mode on POP side of FIFO.
 *
 *  \param eFifoID           	FIFO ID to enable POP side sleep mode
 */
void HAL_FIFO_PopSleepEnable(FIFO_Type eFifoID);


/*! \fn void HAL_FIFO_PopSleepDisable(FIFO_Type eFifoID)
 *  \brief Disable sleep mode on POP side of FIFO.
 *
 *  \param eFifoID             FIFO ID to disable POP side sleep mode
 */
void HAL_FIFO_PopSleepDisable(FIFO_Type eFifoID);


/*! \fn void HAL_FIFO_PopCount(FIFO_Type eFifoID)
 *  \brief Number of words available in FIFO to pop.
 *
 *  \param eFifoID             FIFO ID to read available word count for POP
 */
UINT32_t HAL_FIFO_PopCount(FIFO_Type eFifoID);


/*! \fn void HAL_FIFO_PushCount(FIFO_Type eFifoID)
 *  \brief Number of words available for PUSH operation in FIFO.
 *
 *  \param eFifoID             FIFO ID to read available word count for PUSH
 */
UINT32_t HAL_FIFO_PushCount(FIFO_Type eFifoID);


/*! \fn HAL_StatusTypeDef HAL_FIFO_Init(FIFO_Config xFifoConfig)
 *  \brief Initialize FIFO source and destination for PUSH and POP operation.
 *
 *  \param xFifoConfig         FIFO_Config structure filled by user to initialize FIFO
 *  \return HAL_StatusTypeDef   Status of FIFO initialization. HAL_OK or HAL_ERROR.
 */
HAL_StatusTypeDef HAL_FIFO_Init(FIFO_Config xFifoConfig);

/*! \fn HAL_StatusTypeDef HAL_FIFO_ConfigInt(FIFO_IntConfig xFifoIntConfig)
 *  \brief Initialize FIFO interrupts for different conditions. i.e. overflow, underflow
 *         threshold, sleep etc.
 *
 *  \param xFifoIntConfig      FIFO_Config structure filled by user to initialize FIFO
 *  \return HAL_StatusTypeDef   Status of FIFO interrupt configuration. HAL_OK or HAL_ERROR.
 */
HAL_StatusTypeDef HAL_FIFO_ConfigInt(FIFO_IntConfig xFifoIntConfig);


/*! \fn void HAL_FIFO_ClkInit(FIFO_Type eFifoID)
 *  \brief Initialize clock for FIFO.
 *
 *  \param eFifoID             FIFO ID to initialize clock
 */
void HAL_FIFO_ClkInit(FIFO_Type eFifoID);

/*! \fn void HAL_FIFO_ClkDeInit(FIFO_Type eFifoID)
 *  \brief Disable clock for FIFO.
 *
 *  \param eFifoID             FIFO ID to disable clock
 */
void HAL_FIFO_ClkDeInit(FIFO_Type eFifoID);

/*! \fn HAL_StatusTypeDef HAL_FIFO_PowerInit(FIFO_Type eFifoID)
 *  \brief Initialize power for FIFO.
 *
 *  \param eFifoID             FIFO ID to initialize power
 *  \return HAL_StatusTypeDef   Status of power initialization of FIFO. HAL_OK or HAL_ERROR.
 */
HAL_StatusTypeDef HAL_FIFO_PowerInit(FIFO_Type eFifoID);

#endif /* __EOSS3_HAL_PKFB_H_ */
