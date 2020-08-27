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

/*! \file eoss3_hal_fpga_adc.c
 *
 *  \brief This file contains FPGA ADC APIs implementation
 */
#include "Fw_global_config.h"

#include <stdio.h>
#include <assert.h>
#include "eoss3_hal_fpga_sdma_api.h"
#include "eoss3_hal_fpga_ad7476_reg.h"
//#include "eoss3_hal_gpio.h"
#include "eoss3_hal_fpga_adc_api.h"

#include "s3x_clock.h"
#include "eoss3_hal_def.h"
#include "dbg_uart.h"

typedef struct
{
  void *sdma_handle;
  void (*callback_rxptr)(void);
  uint32_t adc_sensor_id;
  int calback_debug_count;
  uint32_t overflow_last;
} adc_info_t;
uint32_t t_dma_start;
struct adc_timestamp_s adc_timestamps[ MAX_ADC_STAMPS ];
static  adc_info_t adc_info_state;  //To store state info

int AD7476_to_mVolts( uint32_t vdd_mvolts, uint16_t value )
{
    int result;
    result = ((int)value) * ((int)vdd_mvolts);
    result = result + (4096/2);
    result = result / 4096;
    return result;
}


void sdma_callback(void *handle)
{
  //call back to user for dma readcomplete
    if( adc_info_state.calback_debug_count < MAX_ADC_STAMPS ){
        adc_timestamps[ adc_info_state.calback_debug_count ].time_dma_callback = DWT->CYCCNT;
        if( adc_info_state.adc_sensor_id == HAL_SENSOR_ID_AD7476 ){
            //adc_timestamps[ adc_info_state.calback_debug_count ].overflow_value = FPGA_ADC->SEN1_SETTING;
        }
        adc_info_state.calback_debug_count++;
    }
  (*adc_info_state.callback_rxptr)();
  return;
}

bool HAL_ADC_FPGA_is_supported( uint32_t sensorid )
{
    uint32_t actual;
    actual = HAL_ADC_FPGA_get_sensor_id();
    return (sensorid == actual);
}
    
uint32_t HAL_ADC_FPGA_get_sensor_id(void)
{
    uint32_t a;
    uint32_t b;
    
    a = FPGA_ADC->CHIP_ID;
    b = FPGA_ADC->REV_NO;
    //dbg_str_hex32("adc-fpga-id", a );
    //dbg_str_hex32("adc-fpga-rev",b);
    
    if( (a == FPGA_CHIP_ID_AD7476) && (b == FPGA_CHIP_ID_AD7476_REV) ){
        adc_info_state.adc_sensor_id = HAL_SENSOR_ID_AD7476;
        return HAL_SENSOR_ID_AD7476;
    }
    dbg_str_hex32("unknown-fpga-id", a );
    dbg_str_hex32("unknown-fpga-rev", b);
    return 0; // unknown
}


HAL_StatusTypeDef HAL_ADC_FPGA_Init( HAL_ADC_FPGA_cfg_t *adc_cfg, void (*pcb_read)(void))
{
  ch_cfg_t cfg;
  cfg.data_size = DATASIZE_WORD;
  cfg.r_power   = 512;

  
  if( !HAL_ADC_FPGA_is_supported(adc_cfg->sensor_id) ){
      return HAL_ERROR;
  }
  
  switch(adc_cfg->sensor_id){
  case HAL_SENSOR_ID_AD7476:
      break;
  default:
      return HAL_ERROR;
      break;
  }
  
  if (NULL == pcb_read)
  {
        //dbg_str("Read call back is NULL:\n");
    return HAL_ERROR;
  }

  if( !HAL_ADC_FPGA_is_supported( adc_cfg->sensor_id ) ){
      return HAL_ERROR;
  }

  adc_info_state.callback_rxptr = pcb_read;

  //Enable FB clocks.
  S3x_Clk_Set_Rate(S3X_FB_16_CLK, F_18MHZ);
  assert( S3x_Clk_Get_Rate(S3X_FB_16_CLK) == F_18MHZ );
  S3x_Clk_Enable(S3X_FB_16_CLK);

  S3x_Clk_Set_Rate(S3X_FB_21_CLK, F_4MHZ);
  assert( S3x_Clk_Get_Rate(S3X_FB_21_CLK) == F_4MHZ );
  S3x_Clk_Enable(S3X_FB_21_CLK);

#ifndef CONST_FREQ
  S3x_Register_Qos_Node(S3X_FB_16_CLK);
  S3x_Set_Qos_Req(S3X_FB_16_CLK, MIN_HSOSC_FREQ, HSOSC_72MHZ);
#endif


  HAL_FSDMA_Init();
  //Get dma channel HAL_FSDMA_GetChannel
  adc_info_state.sdma_handle = HAL_FSDMA_GetChannel(&sdma_callback, &cfg);
  if (NULL == adc_info_state.sdma_handle)
  {
        dbg_fatal_error("ad7476-dma-null\n");
    return HAL_ERROR;
  }
    if( DBG_flags & DBG_FLAG_adc_task ){
        dbg_str_ptr("ad7476-dma", adc_info_state.sdma_handle );
    }


  switch(adc_cfg->sensor_id){
  case HAL_SENSOR_ID_AD7476:
    FPGA_ADC->SEN_ENR  = 1;
    break;
  }

  /* flush the fifo at init time */
  FPGA_ADC->FIFO_RESET = 1;
  return HAL_OK;
}

#include "datablk_mgr.h"
#include "sensor_ad7476_acquisition.h"
volatile int ad7476_dma_stop_request = 1;
void ad7476_isr_DmacDone(void)
{
    QAI_DataBlock_t  *pdata_block = NULL;
    QAI_DataBlock_t  *pdata_block_prev = pad7476_data_block_prev;
    int  gotNewBlock = 0;
    if (ad7476_dma_stop_request == 1)
    {
      ad7476_dma_stop_request = 2;
      return;
    }
    if (ad7476_dma_stop_request > 0)
    {
      return;
    }
    /* Acquire an audio buffer */
    datablk_mgr_acquireFromISR(ad7476_isr_outq_processor.p_dbm, &pdata_block);
    if (pdata_block)
    {
        gotNewBlock = 1;
    }
    else
    {
        // send error message 
        // xQueueSendFromISR( error_queue, ... )
        if (ad7476_isr_outq_processor.p_event_notifier)
          (*ad7476_isr_outq_processor.p_event_notifier)(ad7476_isr_outq_processor.in_pid, AD7476_ISR_EVENT_NO_BUFFER, NULL, 0);
        pdata_block = pdata_block_prev;
        pdata_block->dbHeader.Tstart = xTaskGetTickCountFromISR();
        pdata_block->dbHeader.numDropCount++;
    }
    uint8_t *p_dest = (uint8_t *)pdata_block->p_data;  // (uint8_t *)pdata_block + offsetof(QAI_DataBlock_t, p_data);
    uint32_t length = pdata_block->dbHeader.numDataElements * pdata_block->dbHeader.dataElementSize;
    /* setup the DMA start address for next buffer */
    // todo , write code here to setup dma for next transfer
    HAL_StatusTypeDef err;
    err = HAL_FSDMA_Receive(adc_info_state.sdma_handle, p_dest, length);

    if (gotNewBlock)
    {
        /* send the previously filled audio data to specified output Queues */     
        pdata_block_prev->dbHeader.Tend = pdata_block->dbHeader.Tstart;
        datablk_mgr_WriteDataBufferToQueuesFromISR(&ad7476_isr_outq_processor, pdata_block_prev);
        pdata_block_prev = pdata_block;
    }
}

void ad7476_start_dma(void)
{
    uint8_t *p_dest = (uint8_t *)pad7476_data_block_prev->p_data;  // (uint8_t *)pdata_block + offsetof(QAI_DataBlock_t, p_data);
    uint32_t length = pad7476_data_block_prev->dbHeader.numDataElements * pad7476_data_block_prev->dbHeader.dataElementSize;
    /* setup the DMA start address for next buffer */
    // todo , write code here to setup dma for next transfer
    HAL_StatusTypeDef err;
    err = HAL_FSDMA_Receive(adc_info_state.sdma_handle, p_dest, length);
    ad7476_dma_stop_request = 0;
    return;
}

void ad7476_stop_dma(void)
{
  if (ad7476_dma_stop_request == 0)
     ad7476_dma_stop_request = 1;
  return;
}
int HAL_FSDMA_IsTransferInProgress(void *handle);
int ad7476_IsDmaTransferInProgress()
{
  return HAL_FSDMA_IsTransferInProgress(adc_info_state.sdma_handle);
}

HAL_StatusTypeDef HAL_ADC_FPGA_Read(void *buffer, size_t n_bytes)
{
  HAL_StatusTypeDef dmaStatus;

  if (NULL == buffer)
  {
    return HAL_ERROR;
  }

    if( n_bytes & 3 ){
        return HAL_ERROR;
    }
    if( adc_info_state.sdma_handle == NULL ){
        dbg_fatal_error("ad7476-rd-no-dma\n");
    }

  dmaStatus = HAL_FSDMA_Receive(adc_info_state.sdma_handle, (void *)buffer, n_bytes);
    if (HAL_OK != dmaStatus)
  {
        //printf("dma read failed");
        return dmaStatus;
  }

  return HAL_OK;
}

void HAL_ADC_FPGA_De_Init(void)
{
  HAL_StatusTypeDef dmaStatus;
    if( adc_info_state.sdma_handle == NULL ){
        dbg_fatal_error("adc-fpga-close-error\n");
    }
    
    if( DBG_flags & DBG_FLAG_adc_task ){
        dbg_str_ptr("adc-fpga-dma-close", adc_info_state.sdma_handle );
    }
    while (ad7476_dma_stop_request != 2)
    {
      vTaskDelay(1);
    }
  dmaStatus = HAL_FSDMA_ReleaseChannel(adc_info_state.sdma_handle);
    if (HAL_OK != dmaStatus){
        dbg_fatal_error("Error in Releasing SDMA channel");
    }
    adc_info_state.sdma_handle = NULL;
  FPGA_ADC->SEN_ENR = 0;
#ifndef CONST_FREQ
     S3x_Clear_Qos_Req(S3X_FB_16_CLK, MIN_HSOSC_FREQ);
#endif

  S3x_Clk_Disable(S3X_FB_21_CLK);
  S3x_Clk_Disable(S3X_FB_16_CLK);

  return;
}
