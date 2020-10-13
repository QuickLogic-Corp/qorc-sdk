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
 *    File   : ql_vr_app.c
 *    Purpose: application code which sets the drivers and associated processing
 *    for voice recognition application.
 *
 *=========================================================*/

/** @file ql_vr_app.c */

#include "Fw_global_config.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "datablk_mgr.h"
#include "datablk_processor.h"
#include "process_ids.h"
#include "ql_util.h"
#include "leds.h"

#if (DSPC_AW == 1)
#include "ql_audio_preproc.h"
#endif
#include "ql_vr_common.h"
//#include "ql_vr_rdsp.h"
#include "ql_i2stx_pe.h"

#include "audio_encode_opus.h"


#include "audio_circularbuffer.h"
#include "audio_stereo2mono.h"

#include "eoss3_hal_audio.h"
#include "host_interface.h"
#include "dbg_uart.h"
#include "RtosTask.h"
#include "ql_controlTask.h"
#include "dbg_queue_monitor.h"
#include "ww_metadata.h"
#include "ql_audio.h"

//from ql_vr.c
extern void set_VR_status(uint8_t enable_flag);


/** @addtogroup QL_SMART_REMOTE_APP QuickAI SDK Audio smart remote example
 *
 *  @brief Smart remote application code
 * This example demonstrates VR engine which works with QL QuickAI framework.
 * (\ref THREAD_QL_VR_PROCESS) is provided in this example.
 *
 * @{
 */
/*
  design policies, within datablock framework:
  1. there are following processing blocks
     - pre-process one from this set {bypass, dspc-1mic, cons-ns} consumer and producer
     - vr no only rdsp (consumer only)
     - i2s output
  2. all processing blocks are running are in one thread. and one block in one pe
  3. there is only one data Block Mem Pool (dma_buffer) and hence only one FreeQ
  4. the task name is THREAD_QL_AUDIO_PROCESS
  5. the THREAD_QL_AUDIO_PROCESS has an inQ with name q_id_inQ_audio_processing
  6. the pre-process produces output, the produced pBuffInfo is pushed to front of the Q
  7. the vr, takes pBuffInfo from the top of the Q and hence get the latest output from pre-proc

*/
#define NUM_AUDIO_BRICKS (120)
#if QL_XPORT_INCLUDE_HEADER == 0
#define AUDIO_BRICK_SIZE_SAMPLES (AUDIO_BLOCK_SIZE_IN_SAMPLES)
#else
#define AUDIO_BRICK_SIZE_SAMPLES ((AUDIO_BLOCK_SIZE_IN_SAMPLES)  + sizeof(QAI_DataBlockHeader_t)/sizeof(int16_t))
#endif
#define AUDIO_BRICK_TIME_MSEC (15)

#define THREAD_1_Q_SIZE   (120)
#define THREAD_1_PRIORITY ((unsigned)(PRIORITY_NORMAL))
#define THREAD_1_STACK_DEPTH (3*256) // 3K bytes

typedef struct {
  QAI_DataBlockHeader_t dbHeader;
  int16_t pcm_data[AUDIO_BLOCK_SIZE_IN_SAMPLES];
} QAI_AudioBrick_t ;


///////////////////////////////VR PE BLOCK /////////////////////////////////////

#ifdef GCC_MAKE
QAI_AudioBrick_t __attribute__((section (".AUD_DMA_BUFF_LOCATION"))) dma_buffer[NUM_AUDIO_BRICKS];
#else
#pragma default_variable_attributes = @ "AUD_DMA_BUFF_LOCATION"
QAI_AudioBrick_t dma_buffer[NUM_AUDIO_BRICKS];
#pragma default_variable_attributes =
#endif

QAI_DataBlockMgr_t audioBuffDataBlkMgr;

struct st_dbm_init {
  QAI_DataBlockMgr_t *pdatablk_mgr_handle;
  void  *pmem;
  int mem_size;
  int item_count;
  int item_size_bytes;
} ;

struct st_dbm_init dbm_init_table[] =
{
  {&audioBuffDataBlkMgr, (void *)dma_buffer, sizeof(dma_buffer), AUDIO_BLOCK_SIZE_IN_SAMPLES, sizeof(int16_t)},
};

QueueHandle_t  q_id_inQ_audio_processing;

//////////////////////////// VR PE BLOCK /////////////////////////////////

void audio_ql_vr_event_handler(int pid, int event_type, void *p_event_data, int num_data_bytes);

/* ql vr processing element functions */
datablk_pe_funcs_t ql_vr_funcs = {datablk_pe_config_ql_vr, datablk_pe_process_ql_vr, NULL, NULL, NULL } ;

outQ_processor_t audio_ql_vr_outq_processor =
{
  .process_func = NULL,
  .p_dbm = &audioBuffDataBlkMgr,
  .in_pid = AUDIO_QL_VR_PID,
  .outQ_num = 0,
  .outQ = NULL,
  .p_event_notifier = audio_ql_vr_event_handler
};

//////////////////////////// PRE-PROC PE BLOCK /////////////////////////////////

void audio_ql_pre_proc_event_handler(int pid, int event_type, void *p_event_data, int num_data_bytes);

/* ql pre-proc processing element functions */
datablk_pe_funcs_t ql_pre_proc_funcs = {datablk_pe_config_ql_pre_process, datablk_pe_process_ql_pre_process, NULL, NULL, NULL } ;

/* Audio PDM capture ISR */
#define AUDIO_PRE_PROC_OUTQS_NUM (1)

QueueHandle_t *audio_pre_proc_outQs[AUDIO_PRE_PROC_OUTQS_NUM] = { &q_id_inQ_audio_processing};

outQ_processor_t audio_ql_pre_proc_outq_processor =
{
  .process_func = NULL,
  .p_dbm = &audioBuffDataBlkMgr,
  .in_pid = AUDIO_QL_PRE_PROC_PID,
  .outQ_num = AUDIO_PRE_PROC_OUTQS_NUM,
  .outQ = audio_pre_proc_outQs,
  .p_event_notifier = audio_ql_vr_event_handler
};

//////////////////////////// Audio Circular Buffer PE BLOCK /////////////////////////////////
extern void audio_circularbuffer_processor(QAI_DataBlock_t *pIn,QAI_DataBlock_t *pOut,QAI_DataBlock_t **pRet,datablk_pe_event_notifier_t *pevent_notifier);
extern void audio_circularbuffer_config(void *p_pe_object);

/* Circular Buffer processing element functions */
datablk_pe_funcs_t ql_audio_cirBuf_funcs = {NULL, audio_circularbuffer_processor, NULL, NULL, NULL };


outQ_processor_t audio_ql_cirBuf_outq_processor =
{
  .process_func = NULL,
  .p_dbm = &audioBuffDataBlkMgr,
  .in_pid = AUDIO_CIRCULAR_BUFFER_PID,
  .outQ_num = 0,
  .outQ = NULL,
  .p_event_notifier = NULL
};

// Configure Circular Buffer
// Opus: enable_transmit_audiobuffer(ql_cbuf_transmit_callback_audio_enc, OPUS_BUFFER_IN_SZ/AUDIO_BLOCK_SIZE_IN_SAMPLES, CIRCULARBUFFER_TRANSMIT_MODE_NO_RELEASE);
// Raw: enable_transmit_audiobuffer(ql_cbuf_transmit_callback_audio_raw, 2, CIRCULARBUFFER_TRANSMIT_MODE_NO_RELEASE
struct CircularBuffer_ConfigData CircularBuffer_FSMConfigData = {
    // How many buffers to hold
    .hold_count = 80,  // This is the num we want saved -- should be enough to hold to start of KWD
    // Parameters for enable_transmit_audiobuffer
    .transmit_callback = NULL,
    .transmit_count = 0,
    .e_mode = CIRCULARBUFFER_TRANSMIT_MODE_NO_RELEASE,
    // Parameters for enable_output2Q_audiobuffer
    .outQ_handle = &q_id_inQ_hif,
    .out_count = 4,
    .queue_mode = CIRCULARBUFFER_OUTQ_MODE_BACK
};


void audio_signal_flow_switch(int mode)
{
  switch(mode)
  {
  case 0:
    break;

  case 3:
    break;

  default:
	  dbg_str_int("audio_signal_flow_switch, Unsupported mode ", mode);
  }
}

datablk_pe_descriptor_t  datablk_pe_descr_audio[] =
{
/* processing element descriptor for pre-processing */
/*            in_id                    out_id            fActive  fSupplyOut fReleaseIn   poutq_processor         pe_funcs          pe_bypass_func   p_pe_sem */
/* NS   */  { AUDIO_ISR_PID,          AUDIO_QL_PRE_PROC_PID,     true, true,  true,  &audio_ql_pre_proc_outq_processor, &ql_pre_proc_funcs,     NULL,   &ql_pre_proc_sem },
/* VR   */  { AUDIO_QL_PRE_PROC_PID,  AUDIO_QL_VR_PID,           true, false, true,  &audio_ql_vr_outq_processor,       &ql_vr_funcs,           NULL,   &ql_vr_sem },
/*CirBuf*/  { AUDIO_QL_PRE_PROC_PID,  AUDIO_CIRCULAR_BUFFER_PID, true, false, false, &audio_ql_cirBuf_outq_processor,   &ql_audio_cirBuf_funcs, NULL,    NULL},
};

/* both PRE_PROC and VR in same thread */
datablk_processor_params_t datablk_processor_params[] = {

    { THREAD_1_PRIORITY, &q_id_inQ_audio_processing, sizeof(datablk_pe_descr_audio)/sizeof(datablk_pe_descr_audio[0]), datablk_pe_descr_audio, THREAD_1_STACK_DEPTH, "THREAD_QL_AUDIO_PROCESS", NULL},

};

/* Audio PDM capture ISR */
#define AUDIO_ISR_OUTQS_NUM (1)

QueueHandle_t *audio_isr_outQs[AUDIO_ISR_OUTQS_NUM] = { &q_id_inQ_audio_processing};

extern void audio_isr_onDmac0BufferDone(void);
outQ_processor_t audio_isr_outq_processor =
{
  .process_func = audio_isr_onDmac0BufferDone,
  .p_dbm = &audioBuffDataBlkMgr,
  .in_pid = AUDIO_ISR_PID,
  .outQ_num = AUDIO_ISR_OUTQS_NUM,
  .outQ = audio_isr_outQs,
  .p_event_notifier = NULL
};

void ql_smart_remote_example(void)
{
  /** Create Audio Queues */
  q_id_inQ_audio_processing = xQueueCreate(THREAD_1_Q_SIZE, sizeof(QAI_DataBlock_t *));
  vQueueAddToRegistry(q_id_inQ_audio_processing, "q_id_inQ_audio_processing");
  memset(dma_buffer, 0, sizeof(dma_buffer));
  /** Setup the data block manager */
  for (int k = 0; k < sizeof(dbm_init_table)/sizeof(struct st_dbm_init); k++) {
      datablk_mgr_init( dbm_init_table[k].pdatablk_mgr_handle,
                        dbm_init_table[k].pmem,
                        dbm_init_table[k].mem_size,
                        dbm_init_table[k].item_count,
                        dbm_init_table[k].item_size_bytes
                      );
      dbg_queue_monitor_add(dbm_init_table[k].pdatablk_mgr_handle->dataBlockFreeQ);
      vQueueAddToRegistry( dbm_init_table[k].pdatablk_mgr_handle->dataBlockFreeQ, "DataBlockFreeQ" );
  }

  /** Setup Audio Thread Handler Processing Elements */
  datablk_processor_task_setup(&datablk_processor_params[0]);
  
  //TIM
//  /* must set the hold count first to be able to use the circular buffer */
//  set_audiobuffer_hold_count(NUM_AUDIO_BRICKS, 0);
//  
//   cbuff_audio_flow_disable(); 

}

//TIM
//#if ENABLE_LPSD
//uint8_t lpsd_state = 1;
//#else
//uint8_t lpsd_state = 0;
//#endif
//void enable_lpsd_state(uint8_t val)
//{
//  lpsd_state = val;
//  if(1 == val)
//    HAL_Audio_Set_LPSDMode(1);
//  else
//    HAL_Audio_Set_LPSDMode(0);
//}

uint8_t frr_test_status = 0;

//TIM
///* Set OPUS status to active/disable.
//   1- enable, 0 - disable.
//   */
//void set_audio_status(uint8_t val)
//{
//   if(val == 0) // Disable OPUS
//   {
//      cbuff_audio_flow_disable();
//      set_VR_status(1);
//      HAL_Audio_Set_LPSDMode(lpsd_state);
//   }
//   else // Enable OPUS
//   {
//       frr_test_status = 0;
//       set_VR_status(0); // Before enabling OPUS, Disable/Bypass VR process.
//       lpsd_state = 0;
//       HAL_Audio_Set_LPSDMode(0);
//
//       cbuff_audio_flow_enable();
//
//   }     
//}

//TIM
//void enable_VR_continuous(uint8_t val)
//{
//  if(1 == val)
//  {
//    //Enable FRR test
//    frr_test_status = 1;
//    set_audio_status(0); //Diable OPUS, if it is enabled already
//  }
//  else
//  {
//    // Disable FRR test
//    frr_test_status = 0;
//    set_audio_status(0); //reset OPUS state. Will be enabled again, after next KPD
//  }
//}

#if PRINT_VR_STATISTICS == 1
float ave_score = 0;
int ave_count = 0;
#endif

t_ql_audio_meta_data o_ql_audio_meta_data = { NO_OF_KPDS, -1, -1, -1, 0 };

void audio_ql_vr_event_handler(int pid, int event_type, void *p_event_data, int num_data_bytes)
{
  static int kp_dtection_count = 1; //chalil, todo - void static, shall be read from structure.
  ql_vr_event_data_t *p_ql_vr_evt = (ql_vr_event_data_t *)p_event_data;
  // Wait on AudioSysQ handle
  if (pid != AUDIO_QL_VR_PID)
    return;
#if AUDIO_LED_TEST
    LedOrangeOn_AutoOff();
#endif
   uint32_t timestamp=xTaskGetTickCount();

   if (isStreamingOn())
      dbg_str_int_noln("\n   RESULT : STREAMING ON KP ", kp_dtection_count++);
   else
      dbg_str_int_noln("\n   RESULT : KP ", kp_dtection_count++);
   dbg_str_int_noln(" VR EVT at ", timestamp);
   //dbg_str_int_noln(" idx ", 0);
   dbg_str_str_nonl(" keyword ", p_ql_vr_evt->p_phrase_text);
   dbg_str_int_noln(" score ", p_ql_vr_evt->score);
   dbg_str_int(" offset ", p_ql_vr_evt->startFramesBack);

   
#if PRINT_VR_STATISTICS == 1
   ave_score += p_ql_vr_evt->score;
   ave_count++;
   if(ave_count == 10)
   {
       ave_score /= ave_count;
       dbg_str_int("ave_score ", (int)ave_score);
       ave_count = 0;
       ave_score = 0;
   }
#endif
   /* If frr test status is enabled, return immediately */
   if(frr_test_status)
     return;

   //if streaming is going on, then the message directly goes to HIF task
   if (isStreamingOn())
   {
     o_ql_audio_meta_data.n_rdsp_length_estimate = p_ql_vr_evt->startFramesBack;
     o_ql_audio_meta_data.a_keyphrase_score = p_ql_vr_evt->score;
     o_ql_audio_meta_data.n_keyphrase_triggered_index = p_ql_vr_evt->len_phrase_text;
     o_ql_audio_meta_data.n_keyphrase_end_index =  p_ql_vr_evt->startFramesBack;
     
     hif_msg_send_streamKPDetected();
     return;
   }
   
   /* compute the number of blocks to keep in the ciruclar based the info */
   /* Note: should replace 240 by a define of number of samples per block */
   int num_blocks = ((p_ql_vr_evt->startFramesBack + AUDIO_BLOCK_SIZE_IN_SAMPLES - 1)/AUDIO_BLOCK_SIZE_IN_SAMPLES) + PRE_TEXT_BEFORE_KEY_PHRASE;
   /* keep at least 1 sec worth of blocks */
   /* now NUM_AUDIO_BRICKS - MIN_AUDIO_SAVE_BLOCKS_SPARE_SIZE  */
   int max_blocks_to_keep = NUM_AUDIO_BRICKS - MIN_AUDIO_SAVE_BLOCKS_SPARE_SIZE;
   num_blocks = num_blocks < max_blocks_to_keep ? num_blocks : max_blocks_to_keep;
   num_blocks = truncate_audio_circularbuffer(num_blocks);
  
   o_ql_audio_meta_data.n_keyphrase_count++;
   o_ql_audio_meta_data.a_keyphrase_score = p_ql_vr_evt->score;
   o_ql_audio_meta_data.n_keyphrase_end_index = num_blocks * AUDIO_BLOCK_SIZE_IN_SAMPLES;
   o_ql_audio_meta_data.n_keyphrase_start_index = o_ql_audio_meta_data.n_keyphrase_end_index
                                                - p_ql_vr_evt->startFramesBack;
   if (o_ql_audio_meta_data.n_keyphrase_start_index < 0)
     o_ql_audio_meta_data.n_keyphrase_start_index = 0;
   o_ql_audio_meta_data.n_keyphrase_triggered_index = p_ql_vr_evt->len_phrase_text;
   o_ql_audio_meta_data.n_rdsp_length_estimate = p_ql_vr_evt->startFramesBack;
   //TIM
    struct xCQ_Packet CQpacket;
 
    CQpacket.ceEvent = CEVENT_VR_TRIGGER;
    addPktToControlQueue(&CQpacket);
   
//#if ENABLE_HOST_IF
//   if (hif_isCommunicationEnabled())
//   {
//     hif_msg_sendKPDetected();
//   }
//   //opus encoding postponed till host is ready.
//#else
//   //set_audio_status(1);
//#endif
}

void audio_ql_i2stx_event_handler(int pid, int event_type, void *p_event_data, int num_data_bytes)
{
  dbg_ch('*');
}

void ql_audio_raw_event_handler(int pid, int event_type, void *p_event_data, int num_data_bytes)
{
  dbg_ch('>');
}


void audio_ql_opus_event_handler(int pid, int event_type, void *p_event_data, int num_data_bytes)
{
  dbg_ch('#');
}

bool raw_sendBypass = false;

void raw_save_block2buffer(QAI_DataBlock_t *pBlock)
{
  
  if(!raw_sendBypass)
  {
      hif_tx_buffer_addSamples((int16_t *)pBlock->p_data, pBlock->dbHeader.numDataElements);
  }
}

//TIM
void cbuff_audio_flow_enable(void)
{
  /*After VR is detected, connect circulal buffer to RAW transmit callback */

    //TIM Moved to Opus_Start
//#if ENABLE_OPUS_ENCODER == 1
//  /* a new  param is added to the transmit callback so that the Datablocks will not be cleared autmatically, instead, the unit whoc gets call back will have 
//     ensure the release.*/
//  enable_transmit_audiobuffer(ql_cbuf_transmit_callback_audio_enc, OPUS_BUFFER_IN_SZ/AUDIO_BLOCK_SIZE_IN_SAMPLES, CIRCULARBUFFER_TRANSMIT_MODE_NO_RELEASE);
//#else
//  enable_transmit_audiobuffer(ql_cbuf_transmit_callback_audio_raw, 2, CIRCULARBUFFER_TRANSMIT_MODE_NO_RELEASE);  // render at 2x
//#endif
  // TIM Moved to opus_Config()
  //audio_enc_config(NULL);
  
}
void cbuff_audio_flow_disable(void)
{
  /* this should stop the timer */
  //stop_audio_raw_stream();
  
  /* reset and release the current blocks */
  disable_audiobuffer_hold();
  /* re-enable circular buffer mode */
  enable_audiobuffer_hold();


}
/*Note : 
Resetting the data blk mgr directly is not a good idea.
Though it free all the block and provide them in freeQ, it does not take care of internal states of PEs.
E.g. even after datablk_mgr_reset(), audio_buffer_info state variables are still maintained at prev values which is not good.
Other way is to reset all the PEs and release the used blocks.
truncate_audio_circularbuffer -> releases the block from audio circular buffer.
release_data_block_prev-> Releases last data block acquired befor audio stop.

This implementation is also not fool proof. As it is observed that not all the block get released in case when streaming is enabled.
Probable reason : this reset API is called before actual Stop msg is executed by Audio Task.
 Thus few blocks get posted to circular buffer after this reset API is called and before Audio actually stops.
 */
void reset_audio_isr_data_blocks(void)
{
#if 1
    datablk_mgr_reset(audio_isr_outq_processor.p_dbm);
#else
    int rem_count =0;
    rem_count = truncate_audio_circularbuffer(0);
    if(rem_count)
    {
        dbg_str_int("remaining buf in CB  ", rem_count );
    }
    release_data_block_prev();
#endif

    return;
}
/** @} */
