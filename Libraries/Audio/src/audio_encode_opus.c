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

#include "Fw_global_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "process_ids.h"
#include "datablk_mgr.h"
#include "ql_opus.h"
#include "audio_raw_pcm_buffer.h"
#include "audio_encode_opus.h"
#include "audio_circularbuffer.h"
#include "fsm.h"
#include "ql_controlTask.h"



/* These shoulde be defined in the libary headers */

/* All for 20ms Opus Buffer/frame size */

#define SZ_QL_MEM_BLOCK1  (9*1024)
#define SZ_QL_MEM_BLOCK2  (27*1024)

#define E_QL_OPUS_ENC_OPTION E_QL_OPUS_ENCODER_OPTION4

#define MAX_AUDIO_OPUS_OUT_QUEUE_SIZE  (64)
#define MAX_AUDIO_OPUS_IN_QUEUE_SIZE  (64)

#define BLOCKS_PER_ONE_OPUS_FRAME    (OPUS_BUFFER_IN_SZ/AUDIO_BLOCK_SIZE_IN_SAMPLES)

/* opus encoder state and buffers */
static ql_opus_encoder_t *p_ql_opus_encoder;
static char opus_mem_buff_1[SZ_QL_MEM_BLOCK1];
static char opus_mem_buff_2[SZ_QL_MEM_BLOCK2];

/* local buffers and pointers for input and output **/
/* leave header size in the begning, just for db tracking purpose, primarily the timestamp */
static int16_t opus_in_buffer[OPUS_BUFFER_IN_SZ];

#include "host_interface.h"
#define E_AUDIO_ENCODER_STATE_DEFAULT_VAL   (0)   
typedef enum e_audio_encoder_state_t_
{
  e_audio_encoder_state_default = E_AUDIO_ENCODER_STATE_DEFAULT_VAL,
  e_audio_encoder_state_inited,
  e_audio_encoder_state_buffering,
  e_audio_encoder_state_encode,
  e_audio_encoder_state_transmit
}e_audio_encoder_state_t;

typedef struct _t_audio_encoder_info
{
  e_audio_encoder_state_t  e_state;
  bool new_outbuffer_required;
  bool last_transmission_over;
  int32_t Tstart;
  int32_t Tend;
  int32_t in_count;
  int32_t add2q_count;
  int32_t blk_count;
  int32_t out_count;
  int32_t cb_count;
  int32_t opus_in_buffer_samples;
  QAI_DataBlock_t *pdata_block_tx_last;
  QueueHandle_t p_audio_in_Queue;
  QueueHandle_t p_audio_out_Queue;
  QAI_DataBlock_t *ap_blocks[BLOCKS_PER_ONE_OPUS_FRAME];
}t_audio_encoder_info;
t_audio_encoder_info o_audio_encoder_info = 
{
  .new_outbuffer_required = true,
  .last_transmission_over = true,
  .in_count = 0,
  .add2q_count = 0,
  .blk_count = 0,
  .out_count = 0,
  .cb_count = 0,
  .pdata_block_tx_last = NULL
};
void start_opus_encode(void);
/*********************************************************************************
 *
 *  Opus-FSM interface
 *
 ********************************************************************************/
enum process_state Opus_State = PSTATE_UNCONFIG;
int                Opus_FSMConfigData;

enum process_state Opus_FSMAction(enum process_action pa, void* pv){
   
    switch(pa) {
    case PACTION_CONFIG:
        Opus_State = PSTATE_STOPPED;
        break;

    case PACTION_START:    
        configASSERT(Opus_State != PSTATE_UNCONFIG);
        audio_encode_reset();
        Opus_State = PSTATE_STARTED;
        break;
        
     case PACTION_STOP:
        configASSERT(Opus_State != PSTATE_UNCONFIG);
        Opus_State = PSTATE_STOPPED;
        break;
        
    default:
        configASSERT(pa);
    }
    return(Opus_State);
};


static int queue_clear(QueueHandle_t h_queue)
{
  int n_items = -1;
  if(h_queue)
  {
    n_items = uxQueueMessagesWaiting(h_queue);
    for(int i = 0 ; i < n_items; i++)
    {
      QAI_DataBlock_t *pdata_block;
      int  ret = xQueueReceive( h_queue, &pdata_block, 0);
      if(ret == pdTRUE)
      {
         datablk_mgr_release_generic(pdata_block);
      }
      else
      {
        break;
      }
    }
  }
  
  return n_items;
}


void audio_encode_reset(void)
{

  queue_clear(o_audio_encoder_info.p_audio_out_Queue);
  queue_clear(o_audio_encoder_info.p_audio_in_Queue);
  
  o_audio_encoder_info.e_state = e_audio_encoder_state_inited;
  o_audio_encoder_info.new_outbuffer_required = true;
  o_audio_encoder_info.last_transmission_over = true;
  o_audio_encoder_info.in_count = 0;
  o_audio_encoder_info.add2q_count = 0;
  o_audio_encoder_info.blk_count = 0;
  o_audio_encoder_info.out_count = 0;
  o_audio_encoder_info.cb_count = 0;
  o_audio_encoder_info.opus_in_buffer_samples = 0, // leave size for header, in 2 byte useful only in the case of RAW;
  o_audio_encoder_info.pdata_block_tx_last = NULL;
  for(int b = 0; b < BLOCKS_PER_ONE_OPUS_FRAME; b++)
  {
    o_audio_encoder_info.ap_blocks[b] = NULL;
  }
  
#if ENABLE_OPUS_ENCODER == 1
    start_opus_encode();
#endif
  
}

/*
 * This should be called every time a new Audio Stream is available.
 * Note: When ever there is a break in the Audio stream due to waking up
 * from sleep (due to LPSD) and detecting a KP, this should be initialized
*/
void start_opus_encode(void)
{
  /* create opus encoder */
  ql_opus_set_mem(opus_mem_buff_1, opus_mem_buff_2, SZ_QL_MEM_BLOCK1, SZ_QL_MEM_BLOCK2);

  /* init the encoder */
  p_ql_opus_encoder = ql_opus_init(E_QL_OPUS_ENC_OPTION);

  return;
}

static int32_t opus_encode_frames(int16_t *p_pcm_data, int32_t n_samples_in, uint8_t *p_encoded_buffer)
{
      uint8_t *p_out = p_encoded_buffer;
      int16_t *p_in = p_pcm_data = p_pcm_data;
      int32_t encoded_sample_count = 0;
      int32_t samples_consumed = 0;
      int32_t n_in = SAMPLES_PER_FRAME;
      int32_t n_out = 0;
      int32_t n_frames = 3; // use macro 

      configASSERT(n_samples_in == SAMPLES_PER_FRAME*n_frames);

      for(int frame = 0; frame < n_frames; frame++)
{
        e_ql_opus_encoder_status_t  ret = ql_opus_encode(p_ql_opus_encoder, p_in, n_in, 1, p_out, &n_out, &samples_consumed);

        configASSERT(ret == E_QL_OPUS_ENCODER_STATUS_OK);
        configASSERT(n_in == samples_consumed);

        encoded_sample_count += n_out;

        p_out += n_out;
        p_in  += n_in;
  }

      return encoded_sample_count;  
}

/*
 * Datablock PE config for Opus Audio Encoder.
 */
void audio_enc_config(void *p_pe_object)
{
    audio_encode_reset();

  o_audio_encoder_info.p_audio_out_Queue = xQueueCreate(MAX_AUDIO_OPUS_OUT_QUEUE_SIZE, sizeof(QAI_DataBlock_t *));
  vQueueAddToRegistry(o_audio_encoder_info.p_audio_out_Queue,"p_audio_out_Queue");
  
  o_audio_encoder_info.p_audio_in_Queue = xQueueCreate(MAX_AUDIO_OPUS_IN_QUEUE_SIZE, sizeof(QAI_DataBlock_t *));
  vQueueAddToRegistry(o_audio_encoder_info.p_audio_in_Queue,"p_audio_in_Queue");
}

/* 
* Get the Opus encoded data in the passed in buffer. If required amount of data
* is not available, 0 will be returned .
*
* Note: This module can not detect if there is buffer overflow
 */
int get_opus_enc_data(uint8_t *enc_output, int input_sz)
{
  int n_pcm = 0;
  int avail_size;
    
  n_pcm = get_audio_stream_data(opus_in_buffer, input_sz);

  if(n_pcm == OPUS_BUFFER_IN_SZ ){
    avail_size = opus_encode_frames(opus_in_buffer, n_pcm, enc_output);
   // configASSERT(request_size == avail_size);
  }
  else{
   // configASSERT(0);
    return 0;
  }
    
  return avail_size;
}

void audio_ql_set_transmission_over_status (bool flag)
{
 //  taskENTER_CRITICAL();
   o_audio_encoder_info.last_transmission_over = flag;
   if(flag == true)
     o_audio_encoder_info.cb_count++;
//  taskEXIT_CRITICAL();
}
bool audio_ql_get_transmission_over_status(void)
{
  return o_audio_encoder_info.last_transmission_over;
}






int32_t raw_enc(QAI_DataBlock_t *pdata_block_in, QAI_DataBlock_t *pdata_block_out, int32_t *n_consumed)
{
  int32_t n_in = AUDIO_BLOCK_SIZE_IN_SAMPLES;
  int32_t n_out;
  uint8_t *p_out;
  int16_t *p_in; 
  p_in =   (int16_t *)((uint8_t *)pdata_block_in  + offsetof(QAI_DataBlock_t, p_data));
  p_out = (uint8_t *)((uint8_t *)pdata_block_out + offsetof(QAI_DataBlock_t, p_data));
  n_out = sizeof(int16_t)*n_in;
  memcpy((void *)&p_out[o_audio_encoder_info.opus_in_buffer_samples], (void *)p_in, sizeof(int16_t)*n_in);
  o_audio_encoder_info.opus_in_buffer_samples +=  n_in;
  if(o_audio_encoder_info.opus_in_buffer_samples == OPUS_BUFFER_IN_SZ )
  {
     n_out = o_audio_encoder_info.opus_in_buffer_samples*sizeof(int16_t) + sizeof(QAI_DataBlockHeader_t);
     o_audio_encoder_info.opus_in_buffer_samples = 0;
  }else
  {
    n_out = 0;
  }
  *n_consumed = n_in;
  pdata_block_out->dbHeader.Tend = pdata_block_in->dbHeader.Tend;
  pdata_block_out->dbHeader.Tstart = pdata_block_in->dbHeader.Tstart;
  pdata_block_out->dbHeader.dataElementSize = 1;  // all are byte array. 
  pdata_block_out->dbHeader.numDataElements = n_out;
  return n_out;
}


int32_t opus_enc(QAI_DataBlock_t *pdata_block_in, QAI_DataBlock_t *pdata_block_out, int32_t *n_consumed)
{
  int32_t n_in = AUDIO_BLOCK_SIZE_IN_SAMPLES;
  int32_t n_out;
  uint8_t *p_out;
  int16_t *p_in; 
  p_in =   (int16_t *)((uint8_t *)pdata_block_in  + offsetof(QAI_DataBlock_t, p_data));
  p_out = (uint8_t *)((uint8_t *)pdata_block_out + offsetof(QAI_DataBlock_t, p_data));
  n_out = sizeof(int16_t)*n_in;
  memcpy((void *)&opus_in_buffer[o_audio_encoder_info.opus_in_buffer_samples], (void *)p_in, sizeof(int16_t)*n_in);
  o_audio_encoder_info.opus_in_buffer_samples +=  n_in;
  if(o_audio_encoder_info.opus_in_buffer_samples == OPUS_BUFFER_IN_SZ)
  {
    //  take actual PCM samples only after the header area, reserved for raw handling...
     int32_t avail_size = opus_encode_frames(&opus_in_buffer[0], o_audio_encoder_info.opus_in_buffer_samples, p_out);
     configASSERT(avail_size == OPUS_BUFFER_OUT_SZ);
     n_out = avail_size;

     o_audio_encoder_info.opus_in_buffer_samples = 0 ;
  }else
  {
    n_out = 0;
  }
  *n_consumed = n_in;
  pdata_block_out->dbHeader.Tend = pdata_block_in->dbHeader.Tend;
  pdata_block_out->dbHeader.Tstart = pdata_block_in->dbHeader.Tstart;
  pdata_block_out->dbHeader.dataElementSize = 1;  // all are byte array. 
  pdata_block_out->dbHeader.numDataElements = n_out;
  return n_out;
}




/** @brief ql_cbuf_transmit_callback_audio_enc                                  */
/* Function to encode opus for D2H trasnmission. Four db of 240 samples each are*/
/* encoded to 3 opus frame(for 320 samples each). OPUS encoded adds 8byte header*/
/* for OPUS reference encoder. this is only for debugging purpose.              */
/* every opus frame will have 8 bytes header and 80 bytes compressed bytes      */
/* So, totally there will be 3*88 = 264 bytes as the transport payload          */
/* 16 bytes of DB header is also transported to the host to communicate the ts  */
/* and other information. These header can be skipped to get above OPUS frames  */
/* QL_XPORT_INCLUDE_HEADER can be disabled if you don't want the 16-byte header */
/* This call back function takes blocks as input                                */
/* It performs the following :                                                  */
/*  A. Enqueue - Add the input to the inQueue                                   */
/*  B. Encode  - call encoder for every 4 blocks and push the output to outqueue*/
/*  C. Transmit - if the transmit is completed, send tx message to hif          */
/*******************************************************************************/
void ql_cbuf_transmit_callback_audio_enc(QAI_DataBlock_t *p_blk_in)
{
  int n_out = 0;   // holds the encoded out size
  int ret = 0;     // holds return value of the FreeRTOS APIs, for err check.
  QAI_DataBlock_t *p_blk_out_queue; // ptr of blk which keeps output and added to out queue queue
  QAI_DataBlock_t *pdata_block_tx;  // loca ptr of blk which is used for tx taken from out queue
  
  
  /****** A. Enqueue input blocks ******/
  /* add the input block to in queue */
  ret = xQueueSendToBack(o_audio_encoder_info.p_audio_in_Queue, &p_blk_in, 0);
  configASSERT(ret == pdTRUE);
  o_audio_encoder_info.in_count++;   
  
  /****** B. Encoding ******/
  o_audio_encoder_info.blk_count++;
  /* call encode for every four blocks */
  if(o_audio_encoder_info.blk_count == BLOCKS_PER_ONE_OPUS_FRAME){
    int b = 0;
    o_audio_encoder_info.e_state = e_audio_encoder_state_encode;
    
    /* B1. acquire a blk for keeping the output */
    ret = datablk_mgr_acquire(audio_isr_outq_processor.p_dbm, &p_blk_out_queue, 0);
    configASSERT(ret == 0);

    
    /* B2. call encoder with buffer popped from inQueue till output is produced */
    do {
      QAI_DataBlock_t *p_blk_temp;
      int32_t n_consumed = 0;
      
      ret = xQueueReceive( o_audio_encoder_info.p_audio_in_Queue, &p_blk_temp, 0);
      configASSERT(ret == pdTRUE);
      
      // save the time stamp of the first pkt so as to use with the output 
      if(b == 0){
         o_audio_encoder_info.Tstart = p_blk_temp->dbHeader.Tstart;
      }
      o_audio_encoder_info.ap_blocks[b] = p_blk_temp;

      /* perform encoding */

      n_out = opus_enc(p_blk_temp, p_blk_out_queue, &n_consumed);

      b++;
    }while(n_out == 0);
       
    configASSERT(o_audio_encoder_info.blk_count == 4);
    o_audio_encoder_info.blk_count = 0;

    /* B4. release the buffers from db data base, cbuf entry book keeping is already handled in the callback */
    for(int b = 0; b < BLOCKS_PER_ONE_OPUS_FRAME; b++)
    {
      datablk_mgr_release_generic(o_audio_encoder_info.ap_blocks[b]);
    }
      
    /* B4. enqueue the encoded out */
    if(n_out){
      o_audio_encoder_info.e_state = e_audio_encoder_state_transmit;

      o_audio_encoder_info.add2q_count++;
      datablk_mgr_usecount_increment(p_blk_out_queue,1);
      p_blk_out_queue->dbHeader.Tstart = o_audio_encoder_info.Tstart;  // of the first in blk
      ret = xQueueSendToBack(o_audio_encoder_info.p_audio_out_Queue, &p_blk_out_queue, 0);
      configASSERT(ret == pdTRUE);
    }
  }
    
  /****** C. Transmission ******/
  if(audio_ql_get_transmission_over_status()) { /* the transport is acknowledged */
    ret = xQueueReceive( o_audio_encoder_info.p_audio_out_Queue, &pdata_block_tx, 0);
    if(ret == pdTRUE)
    { 
      o_audio_encoder_info.out_count++;
      /* if any db is available in the queue, send message to hif to transmit it */
      /* release the last db as the tranmission is over and acknowledged. and reset the flag  */
      //datablk_mgr_release_generic(o_audio_encoder_info.pdata_block_tx_last);
      audio_ql_set_transmission_over_status(false);
      
      /* send message to hif to transport the new db */
#if ENABLE_RAW_TX_SPI == 1
      hif_msg_sendRawBlockReady(pdata_block_tx);
#endif
      /* save the new db as the last one so that the db can be released after transport is acknowledged */
       o_audio_encoder_info.pdata_block_tx_last = pdata_block_tx;
    }
  }
  
}

#define PCM_SAMPLES_IN_RAW_FRAME (2*AUDIO_BLOCK_SIZE_IN_SAMPLES*BLOCKS_PER_ONE_OPUS_FRAME)

typedef struct {
  QAI_DataBlockHeader_t dbHeader;
  int16_t pcm_data[PCM_SAMPLES_IN_RAW_FRAME];
} QAI_AudioBrickRaw_t ;

QAI_AudioBrickRaw_t o_rawa_block_out = {
  .dbHeader.numDataElements = PCM_SAMPLES_IN_RAW_FRAME,
  .dbHeader.dataElementSize = sizeof(int16_t)
};

/** @brief ql_cbuf_transmit_callback_audio_raw                                  */
/* This call back function takes blocks as input                                */
/* It performs the following :                                                  */
/*  A. Enqueue - Add the input to the inQueue                                   */
/*  B. Concatenate and Transmit - concat payload from four DB and send over d2h */
/* DB like structure is use for keeping the concatenated buffer. this helps to  */
/* keep DB header .                                                             */
/*******************************************************************************/
void ql_cbuf_transmit_callback_audio_raw(QAI_DataBlock_t *pdata_block_in)
{
    int n_out = 0;   // holds the encoded out size
    int ret = 0;     // holds return value of the FreeRTOS APIs, for err check.
#if ENABLE_RAW_TX_SPI == 1    
    QAI_DataBlock_t *pdata_block_tx = (QAI_DataBlock_t *)&o_rawa_block_out;  // loca ptr of blk which is used for tx taken from out queue
#endif


    /****** A. Enqueue input blocks ******/
    /* add the input block to in queue */
    ret = xQueueSendToBack(o_audio_encoder_info.p_audio_in_Queue, &pdata_block_in, 0);
    configASSERT(ret == pdTRUE);
    o_audio_encoder_info.in_count++;

    /****** B. Concatenating and Trasnmission ******/
    /* concatenate four blocks for transmission */
   o_audio_encoder_info.blk_count++;
   if (o_audio_encoder_info.blk_count == BLOCKS_PER_ONE_OPUS_FRAME) {
        int b = 0;
        o_audio_encoder_info.e_state = e_audio_encoder_state_encode;
        
        o_audio_encoder_info.blk_count = 0;

        if (audio_ql_get_transmission_over_status()) { /* the transport is acknowledged */
            do {
                QAI_DataBlock_t *p_blk_temp;

                ret = xQueueReceive(o_audio_encoder_info.p_audio_in_Queue, &p_blk_temp, 0);
                configASSERT(ret == pdTRUE);

                // save the time stamp of the first pkt so as to use with the output 
                if (b == 0) {
                    o_rawa_block_out.dbHeader.Tend = p_blk_temp->dbHeader.Tend;
                    o_rawa_block_out.dbHeader.Tstart = p_blk_temp->dbHeader.Tstart;
                }
                o_audio_encoder_info.ap_blocks[b] = p_blk_temp;

                /* copy payload into the transmit buffer - a static DB like object */
                int32_t count = p_blk_temp->dbHeader.numDataElements;
                int16_t *p_dst = &o_rawa_block_out.pcm_data[count*b];
                int16_t *p_src = (int16_t*)&p_blk_temp->p_data[0];
                memcpy(p_dst, p_src, count * sizeof(int16_t));

                n_out += count;
                b++;
            } while (n_out < PCM_SAMPLES_IN_RAW_FRAME);

            /* Transmit over d2H */
#if ENABLE_RAW_TX_SPI == 1
            hif_msg_sendRawBlockReady(pdata_block_tx);
#endif
            audio_ql_set_transmission_over_status(false);
            o_audio_encoder_info.e_state = e_audio_encoder_state_transmit;

            /* B4. release the buffers from db data base, cbuf entry book keeping is already handled in the callback */
            for (int b = 0; b < BLOCKS_PER_ONE_OPUS_FRAME; b++)
            {
                datablk_mgr_release_generic(o_audio_encoder_info.ap_blocks[b]);
            }
        }
    }
}


int32_t encode_audio_blocks_opus(QAI_DataBlock_t *pdata_block_in, QAI_DataBlock_t **pdata_block_out_passed)
{
  int32_t n_in = AUDIO_BLOCK_SIZE_IN_SAMPLES;
  uint16_t n_out;
  uint8_t *p_out;
  int16_t *p_in; 
  int ret = 0;
  QAI_DataBlock_t *pdata_block_out;
  p_in =   (int16_t *)((uint8_t *)pdata_block_in  + offsetof(QAI_DataBlock_t, p_data));
  //n_out = sizeof(int16_t)*n_in;
  
  memcpy((void *)&opus_in_buffer[o_audio_encoder_info.opus_in_buffer_samples], (void *)p_in, sizeof(int16_t)*n_in);
  o_audio_encoder_info.opus_in_buffer_samples +=  n_in;
  datablk_mgr_release_generic(pdata_block_in);
  
  if(o_audio_encoder_info.opus_in_buffer_samples == OPUS_BUFFER_IN_SZ)  {
    /* B1. acquire a blk for keeping the output */
    ret = datablk_mgr_acquire(audio_isr_outq_processor.p_dbm, &pdata_block_out, 0);
    configASSERT(ret == 0);
    p_out = (uint8_t *)((uint8_t *)pdata_block_out + offsetof(QAI_DataBlock_t, p_data));
    
    //  take actual PCM samples only after the header area, reserved for raw handling...
    uint16_t avail_size = opus_encode_frames(&opus_in_buffer[0], o_audio_encoder_info.opus_in_buffer_samples, p_out);
    configASSERT(avail_size == OPUS_BUFFER_OUT_SZ);
    n_out = avail_size;

    o_audio_encoder_info.opus_in_buffer_samples = 0 ;
    
    pdata_block_out->dbHeader.Tend = pdata_block_in->dbHeader.Tend;
    pdata_block_out->dbHeader.Tstart = pdata_block_in->dbHeader.Tstart;
    pdata_block_out->dbHeader.dataElementSize = 1;  // all are byte array. 
    pdata_block_out->dbHeader.numDataElements = n_out;
    datablk_mgr_usecount_increment(pdata_block_out,1);
    *pdata_block_out_passed = pdata_block_out;
  }
  else
     n_out = 0;
  
  return n_out;
}

