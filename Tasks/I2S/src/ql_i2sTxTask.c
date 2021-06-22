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
 *    File   : ql_i2sTask.c
 *    Purpose: I2S data block transmission with enable and disable
 *
 *=========================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "Fw_global_config.h"

#include "FreeRTOS.h"
#include "task.h"
#include "RtosTask.h"
#include "datablk_mgr.h"
#include "eoss3_hal_i2s.h"
#include "eoss3_hal_i2s_drv.h"

#if (ENABLE_I2S_48K_TRANSMIT == 1)
#include "QL_InterpBy3_Stereo.h"
#endif

#include "ql_i2sTxTask.h"


#if 0 //These are to be defined in Fw_global_config.h
#define ENABLE_I2S_STEREO_TRANSMIT 1 //0 = mono channel transmit, 1 =stereo
#define ENABLE_I2S_48K_TRANSMIT    1 //0 = 16K samples, 1 = 48K 
#define I2S_DMA_BUFFER_COUNT      (4)//must be >=2 for continuous I2S stream
#endif

#ifndef I2S_DMA_BUFFER_COUNT
#define I2S_DMA_BUFFER_COUNT      (4)//must be >=2 for continuous I2S stream
#endif

#if ENABLE_I2S_STEREO_TRANSMIT == 1
#define I2S_CHANNEL_COUNT  2    //Data Block size is only 7.5ms
#define I2S_QUEUE_LENGTH (2*30) //need 2 blocks to fill one DMA buffer
#else
#define I2S_CHANNEL_COUNT  1
#define I2S_QUEUE_LENGTH (30)
#endif

#if ENABLE_I2S_48K_TRANSMIT == 1
#define I2S_INTP_FACTOR  3     //The size of DMA buffer is 3 times after interpolation
#else
#define I2S_INTP_FACTOR  1
#endif
//Single DMA Buffer size is directly related to the number of I2S interrupts
//Intention is to minimize the number of interrupts to as low as possible
//The minimum #interrupts (=66) are for 15ms DataBlock, with mono channel at 16K rate
#if (FEATURE_I2S_MASTER_DATA == 1)
#define I2S_MIN_DATA_BLOCK_BYTES   (15*48*2) //in bytes 15ms mono, 16bit @48K sample buffer
#else
#define I2S_MIN_DATA_BLOCK_BYTES   (15*16*2) //in bytes 15ms mono @16K sample buffer
#endif

//This will be size of one DMA buffer needed to keep 66 interrupts per second 
//Note: DMA size must be multple of words
#define I2S_SINGLE_DMA_SIZE        ((I2S_MIN_DATA_BLOCK_BYTES*I2S_CHANNEL_COUNT*I2S_INTP_FACTOR)/4)


xTaskHandle xHandleTaskI2S;
QueueHandle_t I2SDataQ;
int I2S_Q_ErrCount =  0;
int I2S_ISR_ErrCount =  0;
int enable_i2s_tranmission = 0;
int start_block_count = 0;

#if (ENABLE_I2S_48K_TRANSMIT == 1)
static int16_t intp_buffer[FIR_BLK_SIZE_16K_TO_48K*3*I2S_CHANNEL_COUNT];
static int16_t *intp_output = intp_buffer;
#else
//static int16_t *intp_output = NULL;
#endif

static uint32_t i2sDMABuffers[I2S_SINGLE_DMA_SIZE*I2S_DMA_BUFFER_COUNT]; 
static uint32_t i2sSilenceBuffer[I2S_SINGLE_DMA_SIZE]; //always zeros
static uint8_t i2sBuffValids[I2S_DMA_BUFFER_COUNT];
static uint8_t i2s_buf_rd_index;
static uint8_t i2s_buf_wr_index;
static uint8_t i2s_sdma_start;

#define DBG_BUF_ENABLE (0)
#if (DBG_BUF_ENABLE == 1)
static uint32_t i2s_test_Buffer[I2S_SINGLE_DMA_SIZE]; 

static uint8_t dbg_buf[100][4];
static int dbg_buf_count = 0;

static uint8_t dbg_buf2[100][4];
static int dbg_buf_count2 = 0;

void set_dbg_buf(int buf_num)
{
  if(buf_num == 1)
  {
    if(dbg_buf_count < 100) {
      dbg_buf[dbg_buf_count][0] = i2s_buf_rd_index;
      dbg_buf[dbg_buf_count][1] = i2s_buf_wr_index;
      dbg_buf[dbg_buf_count][2] = i2sBuffValids[i2s_buf_rd_index];
      dbg_buf[dbg_buf_count][3] = i2sBuffValids[i2s_buf_wr_index];
      dbg_buf_count++;
    }
  }
  if(buf_num == 2)
  {
    if(dbg_buf_count2 < 100) {
      dbg_buf2[dbg_buf_count2][0] = i2s_buf_rd_index;
      dbg_buf2[dbg_buf_count2][1] = i2s_buf_wr_index;
      dbg_buf2[dbg_buf_count2][2] = i2sBuffValids[i2s_buf_rd_index];
      dbg_buf2[dbg_buf_count2][3] = i2sBuffValids[i2s_buf_wr_index];
      dbg_buf_count2++;
    }
  }

}
#endif
/*  Add a Data Blk to the I2S Data Queue */
int addDatablkToQueue_I2S(QAI_DataBlock_t *pIn)
{
    int ret = 0;

    ret = xQueueSendToBack( I2SDataQ, ( void * )&pIn, 0 );
    //configASSERT(ret == pdTRUE);
    if(ret != pdPASS )
      I2S_Q_ErrCount++;

    return ret ;
}
/* set one of the DMA buffer state to 1 (filled). will be set to 0 (empty) by ISR */
static void set_i2s_buf_state(uint8_t buf_index)
{
  if(buf_index <= I2S_DMA_BUFFER_COUNT)  {
    taskENTER_CRITICAL();
    i2sBuffValids[buf_index] = 1;
    taskEXIT_CRITICAL();
  }  else   {   
    configASSERT(0);
  }
  return;
}
/* called by I2S SDMA ISR */
void I2S_SDMA_callback(uint8_t i2s_id_sel, uint32_t const * p_data_received, uint32_t *p_data_to_send, uint16_t buffer_size)
{
#if (DBG_BUF_ENABLE == 1)  
set_dbg_buf(1);
#endif
  /* first set current buffer to emptied state */
  if(i2s_buf_rd_index < I2S_DMA_BUFFER_COUNT)
    i2sBuffValids[i2s_buf_rd_index++] = 0;
  if(i2s_buf_rd_index >= I2S_DMA_BUFFER_COUNT)
    i2s_buf_rd_index = 0;
  
  /* select the next buffer */
  uint32_t *src = &i2sDMABuffers[I2S_SINGLE_DMA_SIZE*i2s_buf_rd_index];
  
  /* if not a valid buffer, send silence buffer */
  if(i2sBuffValids[i2s_buf_rd_index] == 0)
  {
    src = i2sSilenceBuffer;
    I2S_ISR_ErrCount++;
  }
//src = i2s_test_Buffer;  
  HAL_I2S_TX_RX_Buffer(I2S_SLAVE_ASSP_TX, NULL, src, I2S_SINGLE_DMA_SIZE * sizeof(uint32_t));

  return;
}
#if (I2S_INTP_FACTOR == 3)
#if (ENABLE_I2S_STEREO_TRANSMIT == 1)
/*First interpolate by a factor of 3 both left and right channels.
* Then the left and right channels are copied interleaved for DMA
* Note: The copy loop uses the Stereo element count By 2 times 
*/
static int fill_i2s_DMA_buf_stereo_48K(uint8_t buf_index, int offset, QAI_DataBlock_t *pIn)
{
  int16_t *left = (int16_t *)&pIn->p_data;
  int16_t *right = left + pIn->dbHeader.numDataElements/2;
  int16_t *left_48K_out = intp_output;
  int16_t *right_48K_out = intp_output + (FIR_BLK_SIZE_16K_TO_48K*3);
  
  configASSERT(intp_output);
#if 0 //just for debug fill const data
  int16_t *left1 = (int16_t *)&pIn->p_data;
  int16_t *right1 = left + pIn->dbHeader.numDataElements/2;

  /* copy the data as interleaved */
  for(int i =0; i < (pIn->dbHeader.numDataElements/2); i++)
  {
    *left1++ = 0x2000; //first left
    *right1++ = -0x2000;
  }
#endif
  
  /* get the interpolated left and right channel output */
  convert_16_to_48_stereo(left, left_48K_out, right, right_48K_out);
  
  left = left_48K_out;
  right = right_48K_out;
  int16_t *stereo = (int16_t *)&i2sDMABuffers[I2S_SINGLE_DMA_SIZE*buf_index + (offset/4)];
  
  /* copy the data as interleaved */
  for(int i =0; i < (3*pIn->dbHeader.numDataElements/2); i++)
  {
    *stereo++ = *left++; //first left
    *stereo++ = *right++;
  }
  
  int bytes_copied = 3*pIn->dbHeader.numDataElements*pIn->dbHeader.dataElementSize;
  configASSERT(bytes_copied == (I2S_SINGLE_DMA_SIZE* sizeof(uint32_t)/I2S_CHANNEL_COUNT));
  
  return bytes_copied;
}
#else
/* Interpolate by a factor of 3 one channel and copy both left and right channels.
* Note: The copy uses the Data Block element count times 
*/
static int fill_i2s_DMA_buf_mono_48K(uint8_t buf_index, int offset, QAI_DataBlock_t *pIn)
{
  int16_t *left = (int16_t *)&pIn->p_data;
  int16_t *left_48K_out = intp_output;
  
  configASSERT(intp_output);
    
  /* get the interpolated left channel output */
  convert_16_to_48_left(left, left_48K_out);

  uint8_t *src = (uint8_t *)left;
  uint8_t *dest = (uint8_t *)&i2sDMABuffers[I2S_SINGLE_DMA_SIZE*buf_index];
  int bytes_copied = 3*pIn->dbHeader.numDataElements*pIn->dbHeader.dataElementSize;

  memcpy(dest, src, bytes_copied);

  configASSERT(bytes_copied == (I2S_SINGLE_DMA_SIZE* sizeof(uint32_t)/I2S_CHANNEL_COUNT));
  
  return bytes_copied;

}
#endif //ENABLE_I2S_STEREO_TRANSMIT
#endif //I2S_INTP_FACTOR

#if (ENABLE_I2S_STEREO_TRANSMIT == 1)
/* The left and right channels are copied interleaved for DMA
* Note: The loop uses the Stereo element count in the Data Block
*/
static int fill_i2s_DMA_buf_stereo_16K(uint8_t buf_index, int offset, QAI_DataBlock_t *pIn)
{
  int16_t *left = (int16_t *)&pIn->p_data;
  int16_t *right = left + pIn->dbHeader.numDataElements;
  
  int16_t *stereo = (int16_t *)&i2sDMABuffers[I2S_SINGLE_DMA_SIZE*buf_index + (offset/2)];
  for(int i =0; i < pIn->dbHeader.numDataElements; i++)
  {
    *stereo++ = *left++; //first left
    *stereo++ = *right++;
  }
  
  int bytes_copied = pIn->dbHeader.numDataElements*4;//4 since explicitly copying 2 samples
  configASSERT(bytes_copied == (I2S_SINGLE_DMA_SIZE * sizeof(uint32_t)/I2S_CHANNEL_COUNT));
  
  return pIn->dbHeader.numDataElements*pIn->dbHeader.dataElementSize;
}
#else
/* copy the input into one of the DMA buffers */
static int fill_i2s_DMA_buf_mono_16K(int buf_index, int offset, QAI_DataBlock_t *pIn)
{
  uint8_t *src = (uint8_t *)&pIn->p_data;;
  uint8_t *dest = (uint8_t *)&i2sDMABuffers[I2S_SINGLE_DMA_SIZE*buf_index];
  int bytes_copied = pIn->dbHeader.numDataElements*pIn->dbHeader.dataElementSize;
  memcpy(dest, src, bytes_copied);

  configASSERT(bytes_copied == (I2S_SINGLE_DMA_SIZE * sizeof(uint32_t)/I2S_CHANNEL_COUNT));
               
  return bytes_copied;
}
#endif

/* Wrapper API to call one of the 4 functions based on MACRO values */
static int fill_i2s_DMA_buf(int buf_index, int offset, QAI_DataBlock_t *pIn)
{
  int filled;
#if (I2S_INTP_FACTOR == 3)

#if (I2S_CHANNEL_COUNT == 2)
  filled = fill_i2s_DMA_buf_stereo_48K(buf_index, offset, pIn);
#else
  filled = fill_i2s_DMA_buf_mono_48K(buf_index, offset, pIn);
#endif //I2S_CHANNEL_COUNT
  
#else  //16K channels
  
#if (I2S_CHANNEL_COUNT == 2)
  filled = fill_i2s_DMA_buf_stereo_16K(buf_index, offset, pIn);
#else
  filled = fill_i2s_DMA_buf_mono_16K(buf_index, offset, pIn);
#endif //I2S_CHANNEL_COUNT
  
#endif // I2S_INTP_FACTOR
  return filled;
}
#if (FEATURE_I2S_MASTER_DATA == 1)
/* The left and right channels are copied interleaved for DMA
* Note: The loop uses the Stereo element count in the Data Block
*/
static int fill_i2s_DMA_buf_stereo_48K_32bit(uint8_t buf_index, int offset, QAI_DataBlock_t *pIn, int datablock_count)
{
  int16_t *left = (int16_t *)&pIn->p_data; //interleaved left and right samples
  
  int16_t *stereo = (int16_t *)&i2sDMABuffers[I2S_SINGLE_DMA_SIZE*buf_index + (offset/4)];
  for(int i =0; i < pIn->dbHeader.numDataElements; i++)
  {
    *stereo++ = (int32_t)*left++; //first left
    *stereo++ = (int32_t)*left++;
  }
  
  int bytes_copied = pIn->dbHeader.numDataElements*4;//4 since explicitly copying 2 16bit samples 
  configASSERT(bytes_copied == (I2S_SINGLE_DMA_SIZE * sizeof(uint32_t)/datablock_count));
  
  return pIn->dbHeader.numDataElements*pIn->dbHeader.dataElementSize;
}
#endif


/* set the ISR for I2S transmit mode 
* Need to handle 4 cases for stereo or mono, 16K or 48K I2S bus rate
*/
static void setup_i2s_tx(void)
{
  uint32_t ret_val = HAL_I2S_SUCCESS;
  I2S_Config_t p_i2s_cfg;

  /* first register with HAL */
  HAL_I2S_Slave_Assp_Register();
  
#if ENABLE_I2S_STEREO_TRANSMIT == 1
  /* setup the DMA config for stereo input buffers*/
  p_i2s_cfg.ch_sel = I2S_CHANNELS_STEREO;
  p_i2s_cfg.i2s_wd_clk = 16;
  p_i2s_cfg.mono_sel = I2S_CHANNEL_MONO_LEFT;//not used when stereo?
  p_i2s_cfg.sdma_used = 0;

#else
  /* setup the DMA config for stereo input buffers*/
  p_i2s_cfg.ch_sel = I2S_CHANNELS_MONO;
  p_i2s_cfg.i2s_wd_clk = 16;
  p_i2s_cfg.mono_sel = I2S_CHANNEL_MONO_LEFT;
  p_i2s_cfg.sdma_used = 0;

#endif

#if (FEATURE_I2S_MASTER_DATA == 1)
  p_i2s_cfg.i2s_wd_clk = 32;
#endif  

  /* setup the config and register ISR callback  */
  ret_val = HAL_I2S_Init(I2S_SLAVE_ASSP_TX, &p_i2s_cfg, I2S_SDMA_callback);
  printf("ret_val=%x, HAL_SUCCESS=%x\n", ret_val,  HAL_I2S_SUCCESS); 
  configASSERT(ret_val == HAL_I2S_SUCCESS)


#if (ENABLE_I2S_48K_TRANSMIT == 1)
  int output_blk_size = 0;
#if (ENABLE_I2S_STEREO_TRANSMIT == 1)
  //init stereo interpolator from 16K to 48K
  output_blk_size = init_interpolate_by_3_stereo();
  
#else
  //init one channel interpolator from 16K to 48K
  output_blk_size = init_interpolate_by_3_left();
  
#endif
  //The output size must match the macro definition
  configASSERT(output_blk_size == (FIR_BLK_SIZE_16K_TO_48K*3));
#endif
  return;
}
/* clear the buffers and reset the state */
static void init_i2s_buffers(void)
{
  memset(i2sDMABuffers, 0, sizeof(i2sDMABuffers));
  memset(i2sBuffValids, 0, sizeof(i2sBuffValids));
  memset(i2sSilenceBuffer, 0x0, sizeof(i2sSilenceBuffer));
  
  i2s_buf_rd_index = 0;
  i2s_buf_wr_index = 0;
  
  i2s_sdma_start = 0;
  
#if (DBG_BUF_ENABLE == 1)
  //memset(i2s_test_Buffer,  0xAA, sizeof(i2s_test_Buffer));
  int16_t *p16 = (int16_t *)i2s_test_Buffer;
  int count = (sizeof(i2s_test_Buffer)/2);
  for(int i=0; i < count;)
  {
    *p16++ =  0x5555;
    *p16++ =  0x5555;
    *p16++ =  0xAAAA;
    *p16++ =  0xAAAA;
    i += 4;
  }
#endif
  
  return;
}
/* when i2s transmission is disabled, release all the datablocks held */
void empty_i2sdata_queue(void)
{
  BaseType_t qret;
  QAI_DataBlock_t *pIn;
  int queue_count;
  /* check if the queue is empty */
  queue_count = uxQueueMessagesWaiting(I2SDataQ);
  if (queue_count <= 0)
    return;
  
  /* there are non- zero number of datablocks in the queue */
  for(int i = 0;i < queue_count; i++)
  {
    qret = xQueueReceive(I2SDataQ, &pIn, 0);
    configASSERT(qret == pdTRUE);
    
    /* if in the queue, the usecount must greater than 0 */ 
    if(pIn->dbHeader.numUseCount > 0)
      datablk_mgr_release_generic(pIn);
  }
  
  return;
}
/* This will stop the I2S Transmission */
void stop_i2sTx(void)
{
  HAL_I2S_Stop(I2S_SLAVE_ASSP_TX);
  enable_i2s_tranmission = 0;
  return;
}
/* This will restart the I2S Transmission.
 * If the start_blks is non-zero then transmission starts only after
 * that many blocks are in the I2S data queue.
 */
void start_i2sTx(int start_blks)
{
  init_i2s_buffers();
  if(start_blks > 0)
    start_block_count = start_blks;
  else
    start_block_count = (I2S_QUEUE_LENGTH/2);
  
  enable_i2s_tranmission = 1;
  return;
}
/* return non-zero if the I2S data queue has at least one space for a data block */
int check_i2s_space_available(void)
{
  if( uxQueueSpacesAvailable(I2SDataQ) > 0)
    return 1;
  else
    return 0;
}
/* I2S task for processiong Data Transmit over I2S interface */
void i2sTaskHandler(void *pParameter)
{
    BaseType_t qret;
    QAI_DataBlock_t *pIn;
    int queue_count;
    int datablock_count;

#if (FEATURE_I2S_MASTER_DATA == 1)
    datablock_count = 3*I2S_CHANNEL_COUNT; //since 48K = 3*16Khz
#else
    datablock_count = I2S_CHANNEL_COUNT; //since 16Khz
#endif
    
    init_i2s_buffers();
    setup_i2s_tx();
    
    /* wait until half the queue is filled, to prevent underflow in slave mode */
    start_block_count = (I2S_QUEUE_LENGTH/2);

    /*
     * This task will wait for ever until i2s tranmission is enabled. Once
     * enabled, it will check if there are any Data blocks in the queue.
     * If there are enough Data blocks (for stereo you need at least 2).
     * an I2S DMA buffer is filled, only if an empty buffer is available.
     */
    while(1) {
      /* if Transmission is not enabled release all the data blocks and wait */
      if(enable_i2s_tranmission == 0)
      {
        empty_i2sdata_queue();
        vTaskDelay(10);
        continue;
      }
      /* check how many data blocks are waiting in the Queue */
      queue_count = uxQueueMessagesWaiting(I2SDataQ);
      /* need 1 or 2 Data Blocks to create one DMA buffer */
      if(queue_count < datablock_count)
      {
        vTaskDelay(3);//it may take upto 7.5ms for next datablock
        continue;
      }
      /* wait until half the queue is filled, to prevent underflow in slave mode */
      /* Note: this is not an issue when S3 is the Master, assuming PDM and I2S clocks are synchronized */
      if(queue_count >= start_block_count) {
       
        /* program the first buffer */
        if(i2s_sdma_start == 0) {
            /* This will start I2S Transmission */
           //HAL_I2S_TX_RX_Buffer(I2S_SLAVE_ASSP_TX, NULL, i2sDMABuffers, I2S_SINGLE_DMA_SIZE * sizeof(uint32_t));
            HAL_I2S_TX_RX_Buffer(I2S_SLAVE_ASSP_TX, NULL, i2sSilenceBuffer, I2S_SINGLE_DMA_SIZE * sizeof(uint32_t));
        }
        i2s_sdma_start = 1;
      }
      if(i2s_sdma_start == 0){

        I2S_ISR_ErrCount = 0;
        i2s_buf_wr_index = 0;
        i2s_buf_rd_index = I2S_DMA_BUFFER_COUNT; //make first buffer silence
        
        vTaskDelay(3); // Give PE a chance to run
        continue;
      }
      /* fill only if there is any emptied buffer */  
      if(i2sBuffValids[i2s_buf_wr_index]) {
        vTaskDelay(3); // Give PE a chance to run
        continue;
      }
      int offset = 0;
      for(int i= 0;i < datablock_count; i++)
      {
        /* get a data blk from queue first */
        qret = xQueueReceive(I2SDataQ, &pIn, 0);
        configASSERT(qret == pdTRUE);

#if (FEATURE_I2S_MASTER_DATA == 1)
        /* fill partial/full DMA buffer at 48KHz, 32bit samples*/
        offset += fill_i2s_DMA_buf_stereo_48K_32bit(i2s_buf_wr_index, offset, pIn, datablock_count);
#else
        /* fill partial/full DMA buffer */
        offset += fill_i2s_DMA_buf(i2s_buf_wr_index, offset, pIn);
#endif
        
        /* if in the queue, the usecount must greater than 0 */ 
        if(pIn->dbHeader.numUseCount > 0)
          datablk_mgr_release_generic(pIn);
      }
      
#if (FEATURE_I2S_MASTER_DATA == 1)
extern int fill_i2s_queue(void);
      fill_i2s_queue();
#endif

      /* set state to filled */
      set_i2s_buf_state(i2s_buf_wr_index++);
      if(i2s_buf_wr_index >= I2S_DMA_BUFFER_COUNT)
        i2s_buf_wr_index = 0;
#if (DBG_BUF_ENABLE == 1)
set_dbg_buf(2);
#endif
    }

    //vTaskDelete(NULL); 

} /* i2sTaskHandler() */

/* Setup Data queue and Task Handler for I2S Task */
signed portBASE_TYPE StartRtosTaskI2S( void)
{
    static uint8_t ucParameterToPass;
    
    /* Create queue for I2S Task */
    I2SDataQ = xQueueCreate( I2S_QUEUE_LENGTH, sizeof(QAI_DataBlock_t *) );
    configASSERT( I2SDataQ );
    vQueueAddToRegistry(I2SDataQ, "I2S_Data_inQ");
    
    /* Create I2S Task */
    xTaskCreate ( i2sTaskHandler, "I2STaskHandler", STACK_SIZE_TASK_I2S, &ucParameterToPass, PRIORITY_TASK_I2S, &xHandleTaskI2S);
    configASSERT( xHandleTaskI2S );

    return pdPASS;
}

