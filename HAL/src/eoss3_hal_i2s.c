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

/*!	\file eoss3_hal_i2s.c
 *
 *  \brief This file contains API implemenation for I2S Peripheral(s)
 * 
 */
#include "Fw_global_config.h"

/* Standard includes. */
#include <stdio.h>
#include <string.h>

#include "Fw_global_config.h"
#include "eoss3_hal_i2s.h"
#include "eoss3_hal_i2s_drv.h"
   
#include "sec_debug.h"

#include <FreeRTOS.h>
#include <semphr.h>
#include "task.h"

I2S_Drv_t i2s_instances[I2S_MASTER_SLAVE_MAX];

/**
 * @brief Driver state.
 */
#define I2S_STATE_UNINITIALIZED     0x0  /* Uninitialized. */
#define I2S_STATE_INITIALIZED       0x1  /* Initialized but powered off. */
#define I2S_STATE_STOP              0x2  /* Initialized, but stopped */
#define I2S_STATE_START             0x3  /* START condition, ongoing transaction */

#define I2S_VALID_ID(id) (id < I2S_MASTER_SLAVE_MAX)? HAL_I2S_SUCCESS:HAL_I2S_ERROR

/**
 * @brief HAL_I2S_Init Function for initializing the I2S driver.
 *
 */
uint32_t HAL_I2S_Init (uint8_t i2s_id_sel, I2S_Config_t *p_i2s_cfg,
                       HAL_I2S_Cb_Handler_t handler)
{
    uint32_t ret_val = HAL_I2S_SUCCESS;

    configASSERT(handler);

    if(I2S_VALID_ID(i2s_id_sel))
    {
        printf("[I2S] %s Invalid i2s id %d\n", __func__, i2s_id_sel);
        return HAL_I2S_BAD_PARAMETER;
    }
    
    if(p_i2s_cfg == NULL)
    {
        printf("[I2S] %s Invalid i2s config %d\n", __func__, i2s_id_sel);
        return HAL_I2S_BAD_PARAMETER;
    }
    
    if(i2s_instances[i2s_id_sel].i2s_state == I2S_STATE_INITIALIZED)
    {
        printf("[I2S] %s Invalid i2s state for i2s id %d\n", __func__, i2s_id_sel);
        return HAL_I2S_INVALID_STATE;
    }
    
    if(i2s_instances[i2s_id_sel].initfn != NULL)
        ret_val = i2s_instances[i2s_id_sel].initfn(p_i2s_cfg, handler);
    else
    {
        printf("[I2S] %s initfn() not registered %d\n", __func__, i2s_id_sel);
        return HAL_I2S_ERROR;
    }

    if (ret_val != HAL_I2S_SUCCESS)
    {
        printf("[I2S] %s initfn() failed %d\n", __func__, i2s_id_sel);
        return ret_val;
    }
    
    i2s_instances[i2s_id_sel].i2s_state = I2S_STATE_INITIALIZED;
    
    return ret_val;
}

/**
 * @brief HAL_I2S_TX_RX_Buffer Function for starting the Tx/Rx over I2S
 *
 */
uint32_t HAL_I2S_TX_RX_Buffer (uint8_t i2s_id_sel,
                            uint32_t * p_rx_buffer,
                            uint32_t * p_tx_buffer,
                            uint16_t   buffer_size)
{
    uint32_t ret_val = HAL_I2S_SUCCESS;
    
    if(I2S_VALID_ID(i2s_id_sel) || (!buffer_size))
    {
        printf("[I2S] %s Invalid i2s id %d\n", __func__, i2s_id_sel);
        return HAL_I2S_BAD_PARAMETER;
    }
        
    if(i2s_instances[i2s_id_sel].i2s_state == I2S_STATE_UNINITIALIZED)
    {
        printf("[I2S] %s Invalid i2s state for i2s id %d\n", __func__, i2s_id_sel);
        return HAL_I2S_INVALID_STATE;
    }
    
    if(i2s_instances[i2s_id_sel].bufferfn != NULL)
        ret_val = i2s_instances[i2s_id_sel].bufferfn(p_rx_buffer, p_tx_buffer, buffer_size);
    else
    {
        printf("[I2S] %s initfn() not registered %d\n", __func__, i2s_id_sel);
        return HAL_I2S_ERROR;
    }
    
    if (ret_val != HAL_I2S_SUCCESS)
    {
        printf("[I2S] %s bufferfn() failed %d\n", __func__, i2s_id_sel);
        return ret_val;
    }
    
    i2s_instances[i2s_id_sel].i2s_state = I2S_STATE_START;

    return ret_val;
}

/**
 * @brief HAL_I2S_Stop Function for stopping the ongoing Rx/Tx
 *
 */
uint32_t HAL_I2S_Stop (uint8_t i2s_id_sel)
{
    uint32_t ret_val = HAL_I2S_SUCCESS;

    if(I2S_VALID_ID(i2s_id_sel))
    {
        printf("[I2S] %s Invalid i2s id %d\n", __func__, i2s_id_sel);
        return HAL_I2S_BAD_PARAMETER;
    }

    if(i2s_instances[i2s_id_sel].i2s_state != I2S_STATE_START) 
    {
        printf("[I2S] %s Invalid i2s state for i2s id %d\n", __func__, i2s_id_sel);
        return HAL_I2S_INVALID_STATE;
    }

    if(i2s_instances[i2s_id_sel].stopfn != NULL)
        i2s_instances[i2s_id_sel].stopfn();
    else
    {
        printf("[I2S] %s stopfn() not registered %d\n", __func__, i2s_id_sel);
        return HAL_I2S_ERROR;
    }
    i2s_instances[i2s_id_sel].i2s_state = I2S_STATE_STOP;
    
    return ret_val;
}

/**
 * @brief HAL_I2S_Uninit Function to undo the init
 *
 */
uint32_t HAL_I2S_Uninit(uint8_t i2s_id_sel)
{
    uint32_t ret_val = HAL_I2S_SUCCESS;
    
    if(I2S_VALID_ID(i2s_id_sel))
    {
        printf("[I2S] %s Invalid i2s id %d\n", __func__, i2s_id_sel);
        return HAL_I2S_BAD_PARAMETER;
    }
    
    if((i2s_instances[i2s_id_sel].i2s_state != I2S_STATE_INITIALIZED) ||
       (i2s_instances[i2s_id_sel].i2s_state != I2S_STATE_STOP))
    {
        printf("[I2S] %s Invalid i2s state for i2s id %d\n", __func__, i2s_id_sel);
        return HAL_I2S_INVALID_STATE;
    }

    if(i2s_instances[i2s_id_sel].un_initfn != NULL)
        i2s_instances[i2s_id_sel].un_initfn();
    else
    {
        printf("[I2S] %s un_initfn() not registered %d\n", __func__, i2s_id_sel);
        return HAL_I2S_ERROR;
    }
    i2s_instances[i2s_id_sel].i2s_state = I2S_STATE_UNINITIALIZED;

    return ret_val;
}

/**
 * @brief HAL_I2S_register_driver i2s slave/master drivers should register
 * their function using this function
 */
uint32_t HAL_I2S_Register_Driver(uint8_t i2s_id_sel, I2S_Drv_t i2s_drv_fn)
{
    uint32_t ret_val = HAL_I2S_SUCCESS;
    
    if(I2S_VALID_ID(i2s_id_sel))
    {
        printf("[I2S] %s Invalid i2s id %d\n", __func__, i2s_id_sel);
        return HAL_I2S_BAD_PARAMETER;
    }
        
    i2s_instances[i2s_id_sel].initfn = i2s_drv_fn.initfn;
    i2s_instances[i2s_id_sel].bufferfn = i2s_drv_fn.bufferfn;
    i2s_instances[i2s_id_sel].stopfn = i2s_drv_fn.stopfn;
    i2s_instances[i2s_id_sel].un_initfn = i2s_drv_fn.un_initfn;

    printf("[I2S] %d has registered\n", i2s_id_sel);

    return ret_val;
}
