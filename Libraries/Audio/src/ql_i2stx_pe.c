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

/** @file ql_i2stx_pe.c */

#include "Fw_global_config.h"
#include <string.h>
#include "datablk_mgr.h"
#include "datablk_processor.h"
#include "process_ids.h"
#include "eoss3_hal_i2s.h"
#include "eoss3_hal_i2s_drv.h"
#include "ql_util.h"

#ifndef NO_OF_BUFINFOS
#define NO_OF_BUFINFOS 30
#endif

#define SILENCE_FRAME_DURATION 15
short silence_frame[16 * SILENCE_FRAME_DURATION]={0};
uint8_t silence_frame_sent = 0;

/* Debug variables */
int i2s_data_overflow = 0;
int i2s_data_underflow = 0;

typedef struct blocks_info 
{
  QAI_DataBlock_t *blocks[NO_OF_BUFINFOS];
  uint8_t validity[NO_OF_BUFINFOS];
  int windex;
  int rindex;
} blocks_info_t;

blocks_info_t blocksInfo;
int blockIdx_in_process;

uint8_t sdma_start = 0;
static void I2S_callback_Handler(uint8_t i2s_id_sel, uint32_t const * p_data_received, uint32_t *p_data_to_send, uint16_t buffer_size);
static void I2S_DMA_block();

static void Send_silenceFrame(void)
{
  silence_frame_sent = 1;
  HAL_I2S_TX_RX_Buffer(I2S_SLAVE_ASSP_TX, NULL,(uint32_t*)silence_frame, sizeof(silence_frame));
}
void datablk_pe_config_ql_i2stx(void *p_pe_object)
{
    HAL_I2S_Slave_Assp_Register();
    uint32_t ret_val = HAL_I2S_SUCCESS;
    I2S_Config_t p_i2s_cfg;
    p_i2s_cfg.ch_sel = I2S_CHANNELS_MONO;
    p_i2s_cfg.i2s_wd_clk = 16;
    p_i2s_cfg.mono_sel = I2S_CHANNEL_MONO_LEFT;
    p_i2s_cfg.sdma_used = 0;
    ret_val = HAL_I2S_Init(I2S_SLAVE_ASSP_TX, &p_i2s_cfg, I2S_callback_Handler);
    if(ret_val != HAL_I2S_SUCCESS)
    	QL_LOG_ERR ("Error in I2S Slave Init\n");

  return;
}

void datablk_pe_process_ql_i2stx(QAI_DataBlock_t *pIn, QAI_DataBlock_t *pOut, QAI_DataBlock_t **pRet,
                                 void (*p_event_notifier)(int pid, int event_type, void *p_event_data, int num_data_bytes))
{
  /* Save incoming data block in I2S Tx buffer queue.
     And update buffer info validity field and write index. */
  if(blocksInfo.validity[blocksInfo.windex])
  {
    //Data over flow debug variable.
    i2s_data_overflow++;
  }
  blocksInfo.blocks[blocksInfo.windex] = pIn;
  blocksInfo.validity[blocksInfo.windex] = 1;
  blocksInfo.windex++;
  if(blocksInfo.windex >= NO_OF_BUFINFOS)
      blocksInfo.windex = 0;

  if((sdma_start == 0) && (blocksInfo.windex >= (NO_OF_BUFINFOS - 5)))
  {
    sdma_start = 1;
    I2S_DMA_block();
  }

/*  if (p_event_notifier)
    (*p_event_notifier)(AUDIO_QL_I2STX_PID, 0, NULL, 0);*/
  return;
}

void I2S_callback_Handler(uint8_t i2s_id_sel, uint32_t const * p_data_received, uint32_t *p_data_to_send, uint16_t buffer_size)
{
  /*Check for last frame is either valid frame or silence frame. */
  if(0 == silence_frame_sent)
  {
    /* On receiving DMA transfer completion callback, Release data block to free Q. */
    QAI_DataBlock_t *pBlock_in_process = blocksInfo.blocks[blockIdx_in_process] ;

    blocksInfo.validity[blockIdx_in_process] = 0;
    if(pBlock_in_process->dbHeader.numUseCount > 0)
      datablk_mgr_releaseFromISR_generic(pBlock_in_process);
  }
  else
  {
    silence_frame_sent = 0;
  }

  // get next data buffer to stream over I2S Tx interface
  I2S_DMA_block();

}

void I2S_DMA_block()
{
  /*If valid buffer block is not avaialble, send silence frame. */
  if(blocksInfo.validity[blocksInfo.rindex] == 0)
  {
    //Data under flow debug variable.
    i2s_data_underflow++;

    Send_silenceFrame();
  }
  else
  {
    QAI_DataBlock_t *block = blocksInfo.blocks[blocksInfo.rindex] ;
    blockIdx_in_process = blocksInfo.rindex;

    blocksInfo.rindex++;
    if(blocksInfo.rindex >= NO_OF_BUFINFOS)
        blocksInfo.rindex = 0;

    HAL_I2S_TX_RX_Buffer(I2S_SLAVE_ASSP_TX, NULL,(uint32_t*)block->p_data, block->dbHeader.numDataElements * sizeof(short));
  }
  }

/** @} */
