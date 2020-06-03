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

/*! \file eoss3_hal_fpga_sdma_api.h
 *
 *  \brief This file contains SDMA wrapper APIs to use by FPGA IP drivers
 *         for data transfer to and from FPGA IP on S3.
 *         These APIs can be used by only FPGA IP applications.
 */

#ifndef HAL_EOSS3_HAL_FPGA_SDMA_API_H_
#define HAL_EOSS3_HAL_FPGA_SDMA_API_H_

#include "eoss3_dev.h"
#include "eoss3_hal_def.h"

/* Datasize unit of SDMA transfers. */
typedef enum
{
  DATASIZE_BYTE  = 8,
  DATASIZE_HWORD = 16,
  DATASIZE_WORD  = 32,
  DTASIZE_INVALID
} DATA_SIZE;

/* SDMA channel configuration to use in HAL_FSDMA_GetChannel API. */
typedef struct __FSDMA_Channel_Config
{
  DATA_SIZE   data_size;
  uint32_t    r_power;
}ch_cfg_t;

/* Initilazation of FPGA SDMA HAL driver. */
HAL_StatusTypeDef HAL_FSDMA_Init(void);

/*
 * API to get a channel handle to make transfer and Receive data from FPGA IP.
 *   On channel resource allocation, handle will be returned. Applciation should use
 *   this handle for next transfer/Receive APIs.
 * If transfer completes between FPGA and SRAM, callback will be triggered.
 * If callback parameter is NULL, Transfer and receive APIs are synchronous.
 */
void* HAL_FSDMA_GetChannel(void (*pCallback)(void*), ch_cfg_t *cfg);

/*
 * API to release channel resource.
 *
 */
HAL_StatusTypeDef HAL_FSDMA_ReleaseChannel(void *handle);

/*
 * Transfer data of length bytes from SRAM source to FPGA IP.
 * Assuming, Destination in FPGA IP is fixed Fifo Data register
 * length is transfer size in bytes.
 */
HAL_StatusTypeDef HAL_FSDMA_Send(void *handle, void *srcptr, uint32_t length);

/*
 * Transfer data of length bytes from FPGA IP to SRAM destination address.
 * Assuming, Source in FPGA IP is fixed Fifo Data register.
 * length is transfer size in bytes.
 */
HAL_StatusTypeDef HAL_FSDMA_Receive(void *handle, void *dstptr, uint32_t length);


#endif //HAL_EOSS3_HAL_FPGA_SDMA_API_H_
