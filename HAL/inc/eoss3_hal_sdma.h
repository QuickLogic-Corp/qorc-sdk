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
 *    File   : eoss3_hal_sdma.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __EOSS3_HAL_SDMA_H_
#define __EOSS3_HAL_SDMA_H_
#include "test_types.h"
#include "eoss3_hal_def.h"
typedef enum
{
    SDMA_XFER_INVALID = 0,
    SDMA_XFER_BYTE,
    SDMA_XFER_HWORD,
    SDMA_XFER_WORD
} SDMA_XFER_FORMAT;

/*!
 * @brief Supported HAL SDMA Address location width sizes
 */
typedef enum
{
    SDMA_ADDR_WIDTH_BYTE,
    SDMA_ADDR_WIDTH_HWORD,
    SDMA_ADDR_WIDTH_WORD,
    SDMA_ADDR_WIDTH_NOINC
} SDMA_ADDR_WIDTH;

/**
  * @brief SDMA Channel properties structure
  */
typedef struct
{
   SDMA_XFER_FORMAT data_size;   /* source and destination should have same data size */
   SDMA_ADDR_WIDTH  src_addr_inc;
   SDMA_ADDR_WIDTH  dest_addr_inc;
   uint32_t         rpowerCode;
} SDMA_ch_cfg_t;

HAL_StatusTypeDef HAL_SDMA_Init(void);
//32 bit transfer
HAL_StatusTypeDef HAL_SDMA_xferWord(int chan, uint32_t * srcptr, uint32_t *dstptr, int wordLen);
HAL_StatusTypeDef HAL_SDMA_xfer(int chan, uint32_t * srcptr, uint32_t *dstptr, int wordLen, SDMA_ch_cfg_t *chCfg);
HAL_StatusTypeDef HAL_SDMA_Register_FPGA_DmaChan_Comp_Notifier(void (*comp_notifier)(void*));
HAL_StatusTypeDef HAL_SDMA_Reg_CB(int channel_number, void (*triggerSdma)(int chan),  void (*comp_notifier)(int chan));


#endif /* __EOSS3_HAL_SDMA_H_ */
