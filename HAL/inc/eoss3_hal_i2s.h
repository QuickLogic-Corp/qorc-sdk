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

#ifndef __EOSS3_HAL_I2S_H_
#define __EOSS3_HAL_I2S_H_
/*!	\file eoss3_hal_i2s.h
 *
 *  \brief This file contains API declaration for I2S Peripheral(s)
 * 
 */
#include <stdint.h>

/**
 * @brief return value definitions
 */
#define HAL_I2S_RET_VAL          (0x0)
#define HAL_I2S_RET_VAL_ERROR    (0x20)

#define HAL_I2S_SUCCESS          (HAL_I2S_RET_VAL + 0x0)

#define HAL_I2S_ERROR            (HAL_I2S_RET_VAL_ERROR + 0x1)
#define HAL_I2S_TX_ERROR         (HAL_I2S_RET_VAL_ERROR + 0x2)
#define HAL_I2S_RX_ERROR         (HAL_I2S_RET_VAL_ERROR + 0x3)
#define HAL_I2S_TIME_OUT_ERROR   (HAL_I2S_RET_VAL_ERROR + 0x4)
#define HAL_I2S_BAD_PARAMETER    (HAL_I2S_RET_VAL_ERROR + 0x5)
#define HAL_I2S_INVALID_STATE    (HAL_I2S_RET_VAL_ERROR + 0x6)


/**
 * @brief I2S Stereo/Mono selection.
 */
#define I2S_CHANNELS_STEREO     0x0     /* Stereo channel */
#define I2S_CHANNELS_MONO       0x1     /* Mono channel */

/**
 * @brief I2S Mono Left/ Mono Right selection.
 */
#define I2S_CHANNEL_MONO_LEFT   0x0     /* Mono Left channel */
#define I2S_CHANNEL_MONO_RIGHT  0x1     /* Mono Right channel */

/**
 * @brief I2S Master/Slave in use details
 */
#define I2S_MASTER_ASSP_RX      0x0     /* I2S master in use with Rx */
#define I2S_SLAVE_ASSP_TX       0x1     /* I2S slave in use with Tx */
#define I2S_SLAVE_FABRIC_RX     0x2     /* I2S Slave with Rx on Fabric */
#define I2S_MASTER_SLAVE_MAX    0x3     /* Max number of i2s instances */


/**
 * @brief I2S driver structure
 */
typedef struct
{
    uint8_t sdma_used; 		/* SDMA used, if not will use normal mode of Tx/Rx */
    uint8_t i2s_wd_clk;     /* I2S left /right sync clock */
      

    uint8_t ch_sel;    /* Channel select (Stereo/Mono) */
    uint8_t mono_sel;     /* Mono Left/Right Selection */
} I2S_Config_t;

/**
 * @brief HAL_I2S_Cb_Handler_t I2S driver call back. Callback gets called after completion of Tx/Rx.
 *
 * @param[in] i2s_id_sel I2S ID which is a identifier for the I2S.
 *
 * @param[in]  p_data_received Pointer to the buffer with received data,
 *                             or NULL if the handler is for Tx only.
 * @param[out] p_data_to_send  Pointer to the buffer with data sent
 *                             ,or NULL if the handler is for Rx only.
 * @param[in]  buffer_size  Buffer size in bytes, Length of data received and/or sent.
 *                              This value is always equal to half the size of 
 *                        the buffers set by the call ql_i2s_data_tx_rx_start function.
 *                             Since it uses ping pong buffer mechanism.
 *                             
 */
typedef void (* HAL_I2S_Cb_Handler_t)(uint8_t i2s_id_sel, uint32_t const * p_data_received,
                                  uint32_t * p_data_to_send,
                                  uint16_t   buffer_size);

/**
 * @brief HAL_I2S_Init Function for initializing the I2S driver.
 *
 * @param[in] i2s_id_sel I2S ID which is a identifier for the I2S. 
 *
 * @param[in] p_i2s_cfg Pointer to the structure with initial configuration.
 *
 * @param[in] handler  callback for getting a callback once done with Rx/Tx.
 *
 * @retval HAL_I2S_SUCCESS in case of success, HAL_I2S_ERROR in case of
 *         failure.
 */
uint32_t HAL_I2S_Init (uint8_t i2s_id_sel, I2S_Config_t *p_i2s_cfg, HAL_I2S_Cb_Handler_t handler);

/**
 * @brief HAL_I2S_TX_RX_Buffer Function for starting the Tx/Rx over I2S
 *
 * @param[in] i2s_id_sel I2S ID which is a identifier for the I2S. 
 *
 * @param[in] p_rx_buffer  receive buffer in case of RX otherwise NULL
 *
 * @param[in] p_tx_buffer  transmit buffer in case of RX otherwise NULL
 *
 * @param[in] buffer_size  Tx/Rx buffer size in Bytes (ping pong buffer mechanism)
 *
 * @retval  HAL_I2S_SUCCESS in case of success, else other error value defined
 *          above.
 */
uint32_t HAL_I2S_TX_RX_Buffer (uint8_t i2s_id_sel, uint32_t * p_rx_buffer,
                               uint32_t * p_tx_buffer,
                               uint16_t   buffer_size);
/**
 * @brief HAL_I2S_Stop Function for stopping the ongoing Rx/Tx
 *
 * @param[in] i2s_id_sel I2S ID which is a identifier for the I2S 
 *
 * @retval  HAL_I2S_SUCCESS on success else HAL_I2S_ERROR.
 */
uint32_t HAL_I2S_Stop (uint8_t i2s_id_sel);

/**
 * @brief HAL_I2S_Uninit Function to undo the init
 *
 * @param[in] i2s_id_sel I2S ID which is a identifier for the I2S 
 *
 * @retval  HAL_I2S_SUCCESS on success else HAL_I2S_ERROR.
 */
uint32_t HAL_I2S_Uninit(uint8_t i2s_id_sel);

/**
 * @brief function pointers declaration to register for each i2s driver
 */
typedef uint32_t (* i2s_init)(I2S_Config_t *p_i2s_cfg,
                       HAL_I2S_Cb_Handler_t handler);

typedef uint32_t (* i2s_buffer)(uint32_t * p_rx_buffer,
                            uint32_t * p_tx_buffer,
                            uint16_t   buffer_size);

typedef void (* i2s_stop)(void);
typedef void (* i2s_uninit)(void);

/**
 * @brief function pointers for each i2s driver
 */
typedef struct
{
    uint8_t i2s_state; /* to maintain state of each i2s */

    i2s_init initfn;
    i2s_buffer bufferfn;
    i2s_stop stopfn;
    i2s_uninit un_initfn;
} I2S_Drv_t; 

/**
 * @brief HAL_I2S_register_driver i2s slave/master drivers should register
 * their function using this function
 * @param[in] i2s_id_sel I2S ID which is a identifier for the I2S
 *
 * @param[in] i2s_drv_fn function pointers to be passed here from i2s drivers
 *
 * @retval  HAL_I2S_SUCCESS on success else HAL_I2S_ERROR.
 */
   
uint32_t HAL_I2S_Register_Driver(uint8_t i2s_id_sel, I2S_Drv_t i2s_drv_fn);
#endif /* !__EOSS3_HAL_I2S_H_ */
