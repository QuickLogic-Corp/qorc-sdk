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
*    File   : host_interface.c
*    Purpose: This file contains APIs for host_interface.
*
*=========================================================*/
#include "Fw_global_config.h"
#include <d2h_protocol.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "qlsh_commands.h"
#include <queue.h>
#include "timers.h"
#include "RtosTask.h"
#include <host_interface.h>
#include "s3_host_proto_defs.h"
#include "ql_audio.h"
#include "dbg_uart.h"
//#include "vm1010.h"

#include "s3x_clock_hal.h"
#include "s3x_clock.h"
#include "eoss3_hal_gpio.h"
#include "task.h"
#include <eoss3_hal_audio.h>
#include "ql_controlTask.h"
#include "ww_metadata.h"
#if (ENABLE_I2S_16KHz_RX_SLAVE == 1)
//#include "ql_i2sRxTask.h"
#endif
#if (FEATURE_FLL_I2S_DEVICE == 1)
#include "eoss3_hal_fpga_FLL.h"
#endif

#include "eoss3_hal_i2s.h" 

#if 0 //(AEC_ENABLED == 1)
extern uint32_t HAL_I2S_Stop (uint8_t i2s_id_sel);
extern void set_signal_detect_state(int state);
//extern void Enable_I2S_data(void);

extern int get_signal_detect_state(void);
#endif
///////////// Debug Trace ////////////////////////////////
#include "dbgtrace.h"
#define     K_DBGTRACE_HIF  200
dbgtrace_t  adbgtraceHIF[K_DBGTRACE_HIF];
int         idbgtraceHIF = 0;

///////// define this to enable detection of KP while streaming the data
#define STREAM_KP_DETECT_ENABLE (0)
static int stream_kp_detect_enable = 0;

///////////// Queue/Heap Length and audio block drop count monitor ////////////////////////////////
#include "eoss3_hal_audio.h"
#include "dbg_queue_monitor.h"
s3_mon_info_t s3_monitor_info;

#define HOSTIF_QUEUE_LENGTH 100 //must able to handle messages while waiting for Host response
#define AUDIO_SESSION_TIMER_PERIOD_IN_MS        (10000)

extern void reset_audio_isr_data_blocks(void);
extern void hifcb_cmd_host_process_off(void *pdata, int len);

//transport protocol related variables
extern t_ql_audio_meta_data o_ql_audio_meta_data;



static bool streamingOn = false;
/* This flag is used to ignore the lpsd interrupts (i.e Audio DMA to continue) 
when transmission session between device and host is started*/
// static bool g_lpsd_ignore_flag = false;
#define D2H_MAX_DATA_SIZE   (4*960)
uint8_t             aucAudioBuffer[D2H_MAX_DATA_SIZE];

hif_channel_info_t hif_channel_info_audio = {
  .sequence_number = 0,
  .sequence_numberCompleted = -1,
  .channel_number = HIF_CHAN_NUM_AUDIO,
  .firstTime    = 0,
  .xSemaphore   = 0,
  .pucData     = aucAudioBuffer,
  .pdata_block = NULL
};
//hif_channel_info_t hif_channel_info_pcm1 = {
//  .sequence_number = 0,
//  .channel_number = HIF_CHAN_NUM_AUDIO_PCM1,
//  .firstTime = 0,
//  .pdata_block = NULL
//};
xTaskHandle xHandleTaskHostIf;
QueueHandle_t xHandleQueueHostIf;
int xHostIfQueueOverflow;

/*Semaphore for synchronization with host ready to receive opus chunks.
*Initially it will be in lock state unless host send ready to receive opus pkts.
released in callback when device receives the CMD_HOST_READY_TO_RECEIVE command*/
SemaphoreHandle_t g_host_ready_lock = NULL;
 
//TIM TODO Check if this is still appropriate
/* set aside at least 100ms audio data (=100*16*2 = 3.2KByes) tx buffer for Host */
/* In case of Opus data the size can be reduced further */
typedef struct _hif_buffer_info_t
{
   uint8_t a_buffer[HOST_TRANSPORT_CHUNK_SIZE];
   uint32_t n_buffer_size;
   uint32_t n_start_index;
   uint32_t n_data_size;
}hif_buffer_info_t;

hif_buffer_info_t o_hif_tx_buffer = {
  .n_buffer_size = HOST_TRANSPORT_CHUNK_SIZE,
  .n_start_index = 0
};

//TIM
QueueHandle_t       q_id_inQ_hif = NULL;
#if (FEATURE_MULTI_CHANNEL_STREAM_DEVICE == 1)
#define SINGLE_CHANNEL_QUEUE_LENGTH  (60 - 3)
//#define MULTICHANNEL_QUEUE_LENGTH (1*3*60) //(2*100) //must be able to handle messages while waiting for Host response
#define MULTICHANNEL_QUEUE_LENGTH (150) 
  //Keep the q_id_inQ_hif to receive the Pkts from Circular Buffer
  //This Queue will take Pkts with multi-channel Data only 
  QueueHandle_t q_mch_inQ_hif = NULL;
void enable_multi_channel_stream(void);
void disable_multi_channel_stream(void);
void hif_release_mch_inputQ(void);
void init_wifi_buffer(void); //for wifi
#else
#define SINGLE_CHANNEL_QUEUE_LENGTH  (120)
#endif

/*********************************************************************************
 *
 *  HIF-FSM interface
 *
 ********************************************************************************/
#define MOVE_INIT_CODE_1 (1) //1 = move the code 0=donot move
enum process_state HIF_State = PSTATE_UNCONFIG;
int                HIF_FSMConfigData;

enum process_state HIF_FSMAction(enum process_action pa, void* pv) {
    struct xQ_Packet packet={0};
    
    switch(pa) {
    case PACTION_CONFIG:
        configASSERT(HIF_State == PSTATE_UNCONFIG);
#if (MOVE_INIT_CODE_1 == 0) //1 = moved it to task setup 
        if (q_id_inQ_hif == NULL) {
            // Setup input queue
            //TIM TODO make size appropriate
            //q_id_inQ_hif = xQueueCreate(120, sizeof(QAI_DataBlock_t *));
            q_id_inQ_hif = xQueueCreate(SINGLE_CHANNEL_QUEUE_LENGTH, sizeof(QAI_DataBlock_t *));
            vQueueAddToRegistry(q_id_inQ_hif, "q_id_inQ_hif");
        }
#endif        
        HIF_State = PSTATE_STOPPED;
        break;
        
    case PACTION_START:
        packet.ucCommand = MESSAGE_KPT_DETECTED;
        packet.ucSrc = HOSTIF_TASK_MESSAGE;
        if(pdTRUE != (xQueueSend( xHandleQueueHostIf, ( void * )&packet, 0 )))        // do not wait if queue is full, throw error
        {
            dbg_fatal_error("Host if queue full\n");
        }
        HIF_State = PSTATE_STARTED;
        break;
        
    case PACTION_STOP:
        HIF_State = PSTATE_STOPPED;
        break;
    
    default:
        configASSERT(0);
        break;
    }
    return(HIF_State);
}

/*********************************************************************************
 *
 *  D2H-FSM interface
 *
 ********************************************************************************/
enum process_state D2H_State = PSTATE_UNCONFIG;


enum process_state D2H_FSMAction(enum process_action pa, void* pv){
    struct xQ_Packet packet;
            
    switch(pa) {
    case PACTION_CONFIG:
        D2H_State = PSTATE_STOPPED;
        break;
               
    case PACTION_START:
        hif_msg_sendRawBlockReady(NULL);    // Inform D2H that5 it has data to process
        packet.ucCommand = MESSAGE_NOP;     // Poke HIF CMD queue to ensure it starts looking for DATA packets
        packet.ucSrc = HOSTIF_ISR_MESSAGE;
        hif_data_addToQueueFromTask(&packet);
        D2H_State = PSTATE_STARTED;
        break;
        
    case PACTION_BEGIN_STOPPING:
        hif_msg_sendStopTx();
        D2H_State = PSTATE_STOPPING;
        break;
        
    case PACTION_FINISH_STOPPING:
        D2H_State = PSTATE_STOPPED;
        break;
        
    default:
        configASSERT(0);
    }
    
    return(D2H_State);
};

static void SendEventToD2H(hif_channel_info_t* p_ci, uint8_t evt, uint8_t * ucData)
{
    D2H_Pkt_Info d2h_evt_info;
    d2h_evt_info.seq = p_ci->sequence_number;
    d2h_evt_info.channel = p_ci->channel_number;
    d2h_evt_info.cmd = evt;
    if(ucData == NULL){
        memset(&d2h_evt_info.data[0], 0, sizeof(d2h_evt_info.data));
    } else {
        memcpy(&d2h_evt_info.data[0], ucData, 6);
    }
    p_ci->sequence_number++;
    if (p_ci->sequence_number >= 16)
        p_ci->sequence_number = 0;

    d2h_transmit_cmd(&d2h_evt_info);
    return;
}


static void SendEventAudioTransportBlockReadyD2H(hif_channel_info_t* p_ci, uint8_t* pucData, uint16_t kbytes) {
    D2H_Pkt_Info d2h_evt_info;
    d2h_evt_info.seq = p_ci->sequence_number;
    d2h_evt_info.channel = p_ci->channel_number;
#if (FEATURE_MULTI_CHANNEL_STREAM_DEVICE == 1)
#if (NUM_AUDIO_STREAM_CHANNELS == 2)
    d2h_evt_info.cmd = EVT_RAW_PKT_READY_2; //2 channels
#else
    d2h_evt_info.cmd = EVT_RAW_PKT_READY_3; //3 channels
#endif
#else
    d2h_evt_info.cmd = EVT_RAW_PKT_READY;
#endif
    if(kbytes > 0) {
        d2h_evt_info.data[0] = ((kbytes >> 0) & 0xff);
        d2h_evt_info.data[1] = ((kbytes >> 8) & 0xff);
        d2h_evt_info.data[2] = ((uint32_t)pucData >> 0) & 0xff;
        d2h_evt_info.data[3] = ((uint32_t)pucData >> 8) & 0xff;
        d2h_evt_info.data[4] = ((uint32_t)pucData >> 16) & 0xff;
        d2h_evt_info.data[5] = ((uint32_t)pucData >> 24) & 0xff;

        p_ci->sequence_number++;
        if (p_ci->sequence_number >= 16)
            p_ci->sequence_number = 0;
        if (p_ci->firstTime == 0){
            p_ci->firstTime = 1;
            dbgtrace(__LINE__, 0, adbgtraceHIF, K_DBGTRACE_HIF, &idbgtraceHIF);
            if (xSemaphoreTake(g_host_ready_lock, 2000) != pdTRUE){
                dbgtracePrint(adbgtraceHIF, K_DBGTRACE_HIF, idbgtraceHIF);
                dbg_fatal_error("[host interface] : Error unable to take lock to g_host_ready_lock\n");
            }
        }
        d2h_transmit_cmd(&d2h_evt_info);
    }
}
static char host_sprintf_buf[100];
static void SendEventKPDetected(hif_channel_info_t* p_hif_channel_info, uint8_t evt)
{
  uint8_t ucData[6];
  uint32_t addr;
  uint16_t size;
  addr = (uint32_t)(&o_ql_audio_meta_data);
  size = sizeof (o_ql_audio_meta_data);
  //printf ("addr = 0x%X, size = %d\n", addr, size);
  int n_count = sprintf(host_sprintf_buf,"addr = 0x%X, size = %d\n", addr, size);
  dbg_str(host_sprintf_buf);
  
  // send wake word meta data size and address in little endian order
  ucData[0] = (uint8_t)(size & 0xFF);
  ucData[1] = (uint8_t)((size >> 8) & 0xFF);
  ucData[2] = (uint8_t)(addr & 0xFF);
  ucData[3] = (uint8_t)((addr >> 8) & 0xFF);
  ucData[4] = (uint8_t)((addr >> 16)& 0xFF);
  ucData[5] = (uint8_t)((addr >> 24) & 0xFF);
  SendEventToD2H(p_hif_channel_info, evt,(uint8_t *)(&ucData[0]));
}

////////////////////////////////////////////////////////////////////////////////////////


static void stop_audio_streaming(hif_channel_info_t *p_hif_channel_info)
{
  if (isStreamingOn())
  {
       dbgtrace(__LINE__, 17, adbgtraceHIF, K_DBGTRACE_HIF, &idbgtraceHIF);
      //Send EVT_EOT (End Of Transmission) event to host
      SendEventToD2H(p_hif_channel_info, EVT_EOT, NULL);
      dbgtrace(__LINE__, 0, adbgtraceHIF, K_DBGTRACE_HIF, &idbgtraceHIF);
      // wait for transmit cmd to complete
      h2d_transmit_lock_acquire();
      //clear the QoS
      set_spi_session_qos(0);
      //setLpsdIgnoreState(0);
      // release the tranmit lock
      h2d_transmit_lock_release();
      //Set streaming off
      setStreamingOff();
  }
  // reset the audio data blocks
  //reset_audio_isr_data_blocks();
   dbgtrace(__LINE__, 18, adbgtraceHIF, K_DBGTRACE_HIF, &idbgtraceHIF);
  hif_release_inputQ();
  // release audio QoS
  set_audio_clock_qos(0);
  return;
}

static int timeout_count_bytes = 0;

void hostIfTaskHandler(void *pParameter)
{
  hif_channel_info_t *p_hif_channel_info = (hif_channel_info_t*)pParameter;
  struct xQ_Packet   receivedMsg;
  BaseType_t         xResult = pdFAIL;
  D2H_Pkt_Info d2hpkt;
  int  payload_len ;
  int  payload_address ;
  memset(&receivedMsg,0,sizeof(struct xQ_Packet));

#if 0
//extern void set_task_stack_watermark_monitor(int timer_on, int assert_on); 
set_task_stack_watermark_monitor(0, 0);
#endif  
  //////////////////////////////////////////////////////////////////
  /* Start of while loop */
  while(1)
  {
    /* QL read message from Message queue.  If D2H is active, wait no more than 5ms on control queue before looking at data Q, otherwise wait indefinitely */
      if (D2H_State == PSTATE_STARTED) {
        xResult = xQueueReceive( xHandleQueueHostIf, &( receivedMsg ), pdMS_TO_TICKS(5));
      } else {
          xResult = xQueueReceive( xHandleQueueHostIf, &( receivedMsg ), portMAX_DELAY);
      }

    if( xResult != pdPASS ) {
       // See if there is input data to process
        //dbgtrace(__LINE__, 2, adbgtraceHIF, K_DBGTRACE_HIF, &idbgtraceHIF);
        if (uxQueueMessagesWaiting(q_id_inQ_hif)) {
            // dbgtrace(__LINE__, 3, adbgtraceHIF, K_DBGTRACE_HIF, &idbgtraceHIF);
            hif_SendData(p_hif_channel_info);
        }
    } else {
        // Process messages/data received from ISR Task
        if(receivedMsg.ucSrc == HOSTIF_ISR_MESSAGE)
        {
          switch( receivedMsg.ucCommand ) {
          case MESSAGE_NOP:
              // Do nothing -- used to trigger reassesment of D2H_state == PSTATE_STARTED
              break;

          default:
            QL_LOG_WARN("Undef cmd %d\n", receivedMsg.ucCommand);
            break;
          } /* switch( receivedMsg.ucCommand) from eISR_Task  */

        } /* End of Process messages/data received from ISR Task (M4 events) */

        // Process messages/data received from CLI Task
        if(receivedMsg.ucSrc == HOSTIF_TASK_MESSAGE)
        {
          /* Parse input message and process the request */
          switch( receivedMsg.ucCommand ) {
          case MESSAGE_STOP_TX:

#if 0 //(AEC_ENABLED == 1)  //also send message to disable 
            HAL_I2S_Stop(I2S_SLAVE_FABRIC_RX);
            //cb_notify_i2sRx_disable_intr_from_FPGA();
            set_signal_detect_state(0);
#endif

#if (FEATURE_MULTI_CHANNEL_STREAM_DEVICE == 1)
            disable_multi_channel_stream();
            hif_release_mch_inputQ();
#endif
            stop_audio_streaming(p_hif_channel_info);
            handle_led_for_audio_stop();
            ControlEventSend(CEVENT_HIF_EOT);

            break;

          case MESSAGE_AUDIO_OPUS_CHUNK_DONE:
            //SendEventOpusTransportChunkReadyD2h(p_hif_channel_info, &receivedMsg.ucData[0]);
            configASSERT(0);
            break;
            
          case MESSAGE_AUDIO_OPUS_BLOCK_READY:
            //TIM SendEventAudioTransportBlockReadyD2h(p_hif_channel_info, &receivedMsg.ucData[0]);
            break;
            
          case MESSAGE_AUDIO_RAW_BLOCK_READY:
            hif_SendData(p_hif_channel_info);
            break;

          //streaming is already going on. so just send the message
          case MESSAGE_STREAM_KPT_DETECTED: 
            //vTaskDelay(50);
            SendEventKPDetected(p_hif_channel_info, EVT_STREAM_KP_DETECTED);
            timeout_count_bytes = 0; //reset the timeout since VR is re-enable based on this
            stream_kp_detect_enable = 0; //re-enable 
            break;

          case MESSAGE_KPT_DETECTED:
            //start audio session timer
            p_hif_channel_info->firstTime = 0;     // SJ
            set_spi_session_qos(1);
            stream_kp_detect_enable = 0;
            SendEventKPDetected(p_hif_channel_info, EVT_KP_DETECTED);
            timeout_count_bytes = 0;

#if 0 //(AEC_ENABLED == 1) //also send message to enable AEC 
            cb_notify_i2sRx_intr_from_FPGA();

//Enable_I2S_data();
//NVIC_EnableIRQ(FbMsg_IRQn);
#endif

#if (FEATURE_MULTI_CHANNEL_STREAM_DEVICE == 1)
            enable_multi_channel_stream();
#endif


            break;

          case MESSAGE_GET_MONINFO:
             s3_mon_info.num_drop_count = display_num_drop_count();
             s3_mon_info.heap_size = xPortGetFreeHeapSize();
             dbg_queue_monitor_update();
             dbg_queue_monitor_print("S3 queue,heap monitor info");
             payload_len = sizeof(s3_mon_info_t);
             payload_address = (int)&s3_mon_info;

             d2hpkt.channel = HIF_CHAN_NUM_AUDIO;
             d2hpkt.cmd = EVT_GET_MONINFO;
             d2hpkt.seq = 0;
             d2hpkt.data[0] = (payload_len >> 0) & 0xff;
             d2hpkt.data[1] = (payload_len >> 8) & 0xff;

             d2hpkt.data[2] = (payload_address >>  0) & 0xff;
             d2hpkt.data[3] = (payload_address >>  8) & 0xff;
             d2hpkt.data[4] = (payload_address >> 16) & 0xff;
             d2hpkt.data[5] = (payload_address >> 24) & 0xff;

            SendEventToD2H(p_hif_channel_info, EVT_GET_MONINFO, d2hpkt.data);
            break;
          default:
            QL_LOG_WARN("Undef cmd %d\n", receivedMsg.ucCommand);
            break;
          } /* switch( receivedMsg.ucCommand) from cli Task  */
        } /* End of Process messages/data received from Cli Task */
    }
  } /* End of while loop */
}

Rx_Cb_Ret receive_cmd_pkt(void * cookie, D2H_Pkt_Info rx_pkt_info)
{
    Rx_Cb_Ret stTemp;					 
    hif_channel_info_t *p_hif_channel_info = (hif_channel_info_t*)cookie;
    int icommand = rx_pkt_info.cmd;
    int k;

    int n_count = sprintf(host_sprintf_buf,"Recd %d command\n",icommand);
    dbg_str(host_sprintf_buf);

    dbgtrace(__LINE__, rx_pkt_info.cmd, adbgtraceHIF, K_DBGTRACE_HIF, &idbgtraceHIF);
    ql_assert(p_hif_channel_info->channel_number == rx_pkt_info.channel);
    for (k = 0; k < hif_command_table_size; k++)
    {
      if (icommand == hif_command_table[k].icommand)
         break;
    }

    if ((hif_command_table[k].icommand == 0) && (hif_command_table[k].pcommand_func == NULL)) {
        dbg_str("Received UNKNOWN COMMAND from host\n");
        configASSERT(0);
    } else {
      hif_command_table[k].pcommand_func(&rx_pkt_info, sizeof(rx_pkt_info));
    }
    if(icommand == CMD_HOST_READY_TO_RECEIVE)
      streamingOn = true;

    stTemp.data_read_req = 0;
    return stTemp;
}

void audio_ql_set_transmission_over_status (bool flag);
/* Dummy tx done callback function
tx_pkt_info stores the info of the packet sent
*/




void transmit_done_callback(void *cookie, D2H_Pkt_Info tx_pkt_info)
{
    hif_channel_info_t *pchannel_info = (hif_channel_info_t*) cookie;

    if( (tx_pkt_info.cmd == EVT_RAW_PKT_READY) ||
        (tx_pkt_info.cmd == EVT_RAW_PKT_READY_2) ||
        (tx_pkt_info.cmd == EVT_RAW_PKT_READY_3) )
    {
        if(xSemaphoreGive(pchannel_info->xSemaphore) != pdTRUE) {   // Release lock on buffer
            dbg_str("HIF: Failed to release buffer lock");
        }
    }
    hif_msg_sendRawBlockReady(NULL);                            // Trigger check for input data
    return;
}

void hif_data_addToQueueFromISR(struct xQ_Packet *pxMsg)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if( xQueueIsQueueFullFromISR( xHandleQueueHostIf ) )
  {
    xHostIfQueueOverflow++;
    return;
  }
  xQueueSendFromISR( xHandleQueueHostIf, ( void * )pxMsg, &xHigherPriorityTaskWoken );

  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void hif_data_addToQueueFromTask(struct xQ_Packet *pxMsg)
{
  if( !(uxQueueSpacesAvailable( xHandleQueueHostIf )) ){
    xHostIfQueueOverflow++;
    return;
  }
  xQueueSend( xHandleQueueHostIf, ( void * )pxMsg, 10 );
  return;
}

//D2h Protocol related initialization.
void init_protocol_variables(hif_channel_info_t *p_hif_channel_info)
{
  p_hif_channel_info->sequence_number = 0;

  //register with D2H Task for Rx and Tx callbacks
  d2h_register_callback(&receive_cmd_pkt, p_hif_channel_info->channel_number, (void*)p_hif_channel_info, &transmit_done_callback, (void*)p_hif_channel_info);
  
  //Create Semaphore for synchronization with Host.
  //create tx lock
  if(g_host_ready_lock == NULL)
  {
    g_host_ready_lock = xSemaphoreCreateBinary();
    if( g_host_ready_lock == NULL ) {
      dbg_str("[Host Interface] : Error : Unable to Create Mutex\n");
    }
    vQueueAddToRegistry(g_host_ready_lock, "host ready lock" );
    p_hif_channel_info->firstTime = 0;
  }
  // Create mutex to lock data buffer
  if(p_hif_channel_info->xSemaphore == NULL)
  {
    p_hif_channel_info->xSemaphore = xSemaphoreCreateBinary();
    if( p_hif_channel_info->xSemaphore == NULL ) {
      dbg_str("[Host Interface] : Error : Unable to Create buffer Mutex\n");
    }
    xSemaphoreGive(p_hif_channel_info->xSemaphore);
    vQueueAddToRegistry(p_hif_channel_info->xSemaphore, "hif_chan_buffer_mutex" );
  }
  return;
}



////////////////////////////////////////////////////////////////////////////////////////////
void hif_msg_sendOpusChunkDone(UINT8_t* p_buffer, int length)
{
  UINT32_t address = (UINT32_t)p_buffer;
  struct xQ_Packet packet={0};
  packet.ucCommand = MESSAGE_AUDIO_OPUS_CHUNK_DONE;
  packet.ucSrc = HOSTIF_TASK_MESSAGE;

  packet.ucData[0] = length & 0xff;
  packet.ucData[1] = length >> 8 & 0xff;

  packet.ucData[2] = address & 0xff;
  packet.ucData[3] = (address >> 8) & 0xff;
  packet.ucData[4] = (address >> 16) & 0xff;
  packet.ucData[5] = (address >> 24) & 0xff;

  /* there should not be any  delay for queueing, since called from Processing Element */
  xQueueSend( xHandleQueueHostIf, ( void * )&packet, 0 );
}
////////////////////////////////////////////////////////////////////////////////////////////
void hif_msg_sendRawChunkDone(UINT8_t* p_buffer, int length)
{
  UINT32_t address = (UINT32_t)p_buffer;
  struct xQ_Packet packet={0};
  packet.ucCommand = MESSAGE_AUDIO_PCM_CHUNK_DONE;
  packet.ucSrc = HOSTIF_TASK_MESSAGE;

  if(address != 0)
  {  
  packet.ucData[0] = length & 0xff;
  packet.ucData[1] = length >> 8 & 0xff;

  packet.ucData[2] = address & 0xff;
  packet.ucData[3] = (address >> 8) & 0xff;
  packet.ucData[4] = (address >> 16) & 0xff;
  packet.ucData[5] = (address >> 24) & 0xff;

  /* there should not be any  delay for queueing, since called from Processing Element */
  xQueueSend( xHandleQueueHostIf, ( void * )&packet, 0 );
  }
}
#if ENABLE_OPUS_ENCODER == 1
void opus_host_msg_callback(int opus_bytes_available)
{
    if (hif_isCommunicationEnabled())
    {
      int bytes_per_chunk, chunk_time;
      get_opus_out_block_size(&bytes_per_chunk, &chunk_time);

      if(opus_bytes_available >= bytes_per_chunk)
      {
        hif_msg_sendOpusChunkDone(&o_hif_tx_buffer.a_buffer[0], bytes_per_chunk);
      }
    }

  return;
}
#endif

#if (FEATURE_MULTI_CHANNEL_STREAM_DEVICE == 1)
extern int get_audio_block_size_samples(void);
extern QAI_DataBlockMgr_t* get_audio_block_manager(void);

#define MAX_DATA_BLOCK_SIZE  (2*240) //should be at least 1 datblock size 

static int16_t mch_data_buf[MAX_DATA_BLOCK_SIZE];
static int mch_buf_wrcount = 0;
static QAI_DataBlockMgr_t* mch_db_manager;
static int mch_db_size = 240; //audio data block size in samples
static int mch_stream_enable = 0; //enable only after KP detect
//static int db_count = 0;
void enable_multi_channel_stream(void)
{
  mch_stream_enable = 1;
  mch_buf_wrcount = 0;
  //db_count = 0;
}
void disable_multi_channel_stream(void)
{
  mch_stream_enable = 0;
  mch_buf_wrcount = 0;
}
static void sendto_mch_queue(void)
{
  QAI_DataBlock_t* pdatablk ;
  int ret;

  //first check if there is enough space in queue, else release 1 block
  int count = uxQueueMessagesWaiting(q_mch_inQ_hif);
  if(count > (MULTICHANNEL_QUEUE_LENGTH - 3)) {

    //if(get_signal_detect_state() ) {
      //count = 1; //remove just 1 
      count = 9; //remove just 3 
      //dbg_str("@");
    //} else {
      //count = (MULTICHANNEL_QUEUE_LENGTH/2); //remove half
      //dbg_str("%");
    //}
    //empty the queue
    while(count > 0) {
      pdatablk = 0;
      ret = xQueueReceive(q_mch_inQ_hif, &pdatablk, 0);
//      configASSERT(ret != pdFAIL);
      if(pdatablk) { 
          if(pdatablk->dbHeader.numUseCount > 0)
            datablk_mgr_release_generic(pdatablk);
      }
      count--;
    }
    //printf("=%d=",count);
  }
  //send to queue
  ret = datablk_mgr_acquire(mch_db_manager, &pdatablk, 10);
#if 1
  configASSERT(ret == 0);
#else
  if(ret != 0)
  {
    //printf("\nError in sending AEC\n");
    dbg_str("\n==Error AEC==\n");
    return;
  }
#endif
  //copy the buffer to new datablock and increment usecount
  memcpy((uint8_t *)pdatablk->p_data, (uint8_t *)mch_data_buf, mch_db_size*2);
  pdatablk->dbHeader.numUseCount = 1;
  ret = xQueueSend(q_mch_inQ_hif, &pdatablk, 0);
  configASSERT(pdTRUE == ret);
  return;
}

void addto_2channel_queue_1(int16_t *ch1_data, int16_t *ch2_data, int data_size)
{
  //enable only after the host ready to receive
//  if(mch_stream_enable == 0)
//    return;
  
  //copy the input data interspersed in a buffer
  for(int i=0; i < data_size; i++)
  {
    //assumption: mch_db_size is exact multple of 4 (240/2 = 120)
    mch_data_buf[mch_buf_wrcount++] = *ch1_data++; //channel 1
    mch_data_buf[mch_buf_wrcount++] = *ch2_data++; //channel 2
    
    //if enough data to fill a datablock send it to queue
    if(mch_buf_wrcount >= mch_db_size) {
      //send to the queue and reset buffer count
      sendto_mch_queue();
      mch_buf_wrcount = 0;
//      printf(".%d",db_count++);
    }
  }
  return;
}

void addto_3channel_queue(int16_t *ch1_data, int16_t *ch2_data,int16_t *ch3_data, int data_size)
{

  //enable only after the host ready to receive
//  if(mch_stream_enable == 0)
//    return;
  
  //copy the input data interspersed in a buffer
  for(int i=0; i < data_size; i++)
  {
    //assumption: mch_db_size is exact multple of 4 (240/2 = 120)
    mch_data_buf[mch_buf_wrcount++] = *ch1_data++; //channel 1
    mch_data_buf[mch_buf_wrcount++] = *ch2_data++; //channel 2
    if(ch3_data != 0 ) {
      //assumption: mch_db_size is exact multple of 6 (240/3 = 80)
      mch_data_buf[mch_buf_wrcount++] = *ch3_data++; //channel 3
    }
    
    //if enough data to fill a datablock send it to queue
    if(mch_buf_wrcount >= mch_db_size) {
      //send to the queue and reset buffer count
      sendto_mch_queue();
      mch_buf_wrcount = 0;
//      printf(".%d",db_count++);
    }
  }
  return;
}
void addto_2channel_queue(int16_t *ch1_data, int16_t *ch2_data, int data_size)
{
  addto_3channel_queue(ch1_data, ch2_data, 0,data_size);
  return;
}
#endif
signed portBASE_TYPE hif_task_Start(void)
{

  static hif_channel_info_t *p_hif_channel_info = &hif_channel_info_audio;

#if (MOVE_INIT_CODE_1 == 1) //1 = moved it from HIF_FSMAction
  /* Create data queue for HostIf Task */
  //q_id_inQ_hif = xQueueCreate(120, sizeof(QAI_DataBlock_t *));
  q_id_inQ_hif = xQueueCreate(SINGLE_CHANNEL_QUEUE_LENGTH, sizeof(QAI_DataBlock_t *));
  vQueueAddToRegistry(q_id_inQ_hif, "q_id_inQ_hif");
  if(q_id_inQ_hif == 0) {
    configASSERT(0);
  }
#endif

  /* Create queue for HostIf Task */
  xHandleQueueHostIf = xQueueCreate( HOSTIF_QUEUE_LENGTH, sizeof(struct xQ_Packet) );
  if(xHandleQueueHostIf == 0)
  {
    configASSERT(0);
    //return pdFAIL;
  }
  vQueueAddToRegistry( xHandleQueueHostIf, "HostIf_Q" );

#if (FEATURE_MULTI_CHANNEL_STREAM_DEVICE == 1)

  //first init the audio buffer
  init_wifi_buffer();  
  
  /* Create mutli-channel data queue for HostIf Task */
  q_mch_inQ_hif = xQueueCreate( MULTICHANNEL_QUEUE_LENGTH, sizeof(struct xQ_Packet) );
  if(q_mch_inQ_hif == 0)
  {
    configASSERT(0);
  }
  vQueueAddToRegistry( q_mch_inQ_hif, "Multi_chan_Q" );

  //also initialize the datablock manager value and block size
  mch_db_manager = get_audio_block_manager();
  mch_db_size = get_audio_block_size_samples();
  //printf("\nGot DB Manager. DB size = %d \n", mch_db_size);
  
#endif

  /* Create HostIf Task */
  xTaskCreate (hostIfTaskHandler, "HostIfTaskHandler", STACK_SIZE_ALLOC(STACK_SIZE_TASK_HOSTIF), p_hif_channel_info, PRIORITY_TASK_HOSTIF, &xHandleTaskHostIf);
  configASSERT( xHandleTaskHostIf );
  
  init_protocol_variables(p_hif_channel_info);
  
  // register QoS node for clocks
  hif_register_qos();

  return pdPASS;
}

//this will be called while streaming is going on
void hif_msg_send_streamKPDetected(void)
{
    struct xQ_Packet packet={0};
    packet.ucCommand = MESSAGE_STREAM_KPT_DETECTED;
    packet.ucSrc = HOSTIF_TASK_MESSAGE;
    if(pdTRUE != (xQueueSend( xHandleQueueHostIf, ( void * )&packet, 0 )))        // do not wait if queue is full, throw error
    {
        dbg_fatal_error("host if queue full\n");
    }
}
#if 0
void hif_msg_sendKPDetected(void)
{
    struct xQ_Packet packet={0};
    packet.ucCommand = MESSAGE_KPT_DETECTED;
    packet.ucSrc = HOSTIF_TASK_MESSAGE;
    if(pdTRUE != (xQueueSend( xHandleQueueHostIf, ( void * )&packet, 0 )))        // do not wait if queue is full, throw error
    {
        dbg_fatal_error("host if queue full\n");
    }
}
#endif

bool host_communication_status = true;  //By default host communication interface is on.

void hif_enableCommunication(uint8_t val)
{
  if(1 == val)
  {
    //Enable host communication
    host_communication_status = true;
  }
  else
  {
    //Disable host communication
    host_communication_status = false;
  }
}

bool hif_isCommunicationEnabled()
{
  return host_communication_status;
}

bool isStreamingOn(void)
{
  return streamingOn;
}

void setStreamingOff(void)
{
  streamingOn = false;
}

void setStreamingOn(void)
{
  streamingOn = true;
}

void  hif_tx_buffer_sendSamples(int32_t n_samples)
{
   hif_msg_sendRawChunkDone(&o_hif_tx_buffer.a_buffer[0], n_samples);
  return;
  }
  
void  hif_tx_buffer_sendSamples_opus(int32_t n_samples)
{
  hif_msg_sendOpusChunkDone(&o_hif_tx_buffer.a_buffer[0], n_samples);
  return;
}


void set_spi_session_qos(uint8_t status)
{
    if(0 == status)         // release QoS
    {
        if(S3x_Get_Qos_Req(S3X_AUDIO_APB,MIN_OP_FREQ))  // if QoS is set
        {
            S3x_Clear_Qos_Req(S3X_AUDIO_APB,MIN_OP_FREQ);   // acquire C01 QoS
            // SJ : Do I need to take/release C10 QoS also ??
        }
        if(S3x_Get_Qos_Req(S3X_A0_01_CLK,MIN_OP_FREQ))  // if QoS is set
        {
            S3x_Clear_Qos_Req(S3X_A0_01_CLK,MIN_OP_FREQ);   // acquire C01 QoS
            // SJ : Do I need to take/release C10 QoS also ??
        }
        if(S3x_Get_Qos_Req(S3X_AUDIO_DMA_CLK,MIN_OP_FREQ))  // if QoS is set
        {
            S3x_Clear_Qos_Req(S3X_AUDIO_DMA_CLK,MIN_OP_FREQ);   // acquire C01 QoS
            // SJ : Do I need to take/release C10 QoS also ??
        }
        set_sram_lpm_blocks(1);
    }
    else                    // acquire QoS
    {
        // Disable sram block from going into sleep
        set_sram_lpm_blocks(0);
        S3x_Set_Qos_Req(S3X_A0_01_CLK,MIN_OP_FREQ,HSOSC_6MHZ);
        S3x_Set_Qos_Req(S3X_AUDIO_APB,MIN_OP_FREQ,HSOSC_6MHZ);
        S3x_Set_Qos_Req(S3X_AUDIO_DMA_CLK,MIN_OP_FREQ,HSOSC_6MHZ);

        // SJ : Do I need to take/release C10 QoS also ??
    }
}

void set_audio_clock_qos(uint8_t status)
{
    if(0 == status)         //release QoS
    {
        // release QoS for Hsosc
        if(S3x_Get_Qos_Req(S3X_LPSD,MIN_HSOSC_FREQ))  // if QoS is set
        {
            S3x_Clear_Qos_Req(S3X_LPSD,MIN_HSOSC_FREQ);   // acquire C01 QoS
            // SJ : Do I need to take/release C10 QoS also ??
        }
    }
    else                    // acquire QoS
    {
        S3x_Set_Qos_Req(S3X_LPSD,MIN_HSOSC_FREQ,HSOSC_QOS_VAL);
    }
}

void hif_register_qos(void)
{
    S3x_Register_Qos_Node(S3X_A0_01_CLK);
    S3x_Register_Qos_Node(S3X_LPSD);
    S3x_Register_Qos_Node(S3X_AUDIO_APB);
    S3x_Register_Qos_Node(S3X_AUDIO_DMA_CLK);
    return;
}


void    hif_release_inputQ(void) {
    BaseType_t  xResult;
    
    while(uxQueueMessagesWaiting(q_id_inQ_hif)) {
        QAI_DataBlock_t* pdatablk;
        xResult = xQueueReceive(q_id_inQ_hif, &pdatablk, 0);
        configASSERT(xResult != pdFAIL);
        if(pdatablk) { 
            /* only if nonzero usecount, else it is an error */
            /* Should we assert here since this should never happen ? */
            if(pdatablk->dbHeader.numUseCount > 0)
            datablk_mgr_release_generic(pdatablk);
        }
    }
}

int hif_release_inputQ_count(int msg_count) {
    BaseType_t  xResult;
    int kbytes = 0;
    
    int wait_count = uxQueueMessagesWaiting(q_id_inQ_hif);
    if(wait_count <= msg_count)
      return 0;
    wait_count = msg_count;
    while(wait_count > 0) {
        wait_count--;
        QAI_DataBlock_t* pdatablk;
        xResult = xQueueReceive(q_id_inQ_hif, &pdatablk, 0);
        configASSERT(xResult != pdFAIL);
        kbytes += (pdatablk->dbHeader.numDataElements*pdatablk->dbHeader.dataElementSize);
        if(pdatablk) { 
            /* only if nonzero usecount, else it is an error */
            /* Should we assert here since this should never happen ? */
            if(pdatablk->dbHeader.numUseCount > 0)
            datablk_mgr_release_generic(pdatablk);
        }
        
    }
    timeout_count_bytes += kbytes;
    return kbytes;
}

#define D2H_NUM_AUDIO_BLOCKS_TO_SEND    (4)
static int send_data_to_host(hif_channel_info_t* p_hif_channel_info, QueueHandle_t in_Q)
{
    BaseType_t  xResult;
    uint16_t     kbytes = 0;
    int byte_count = 0; 
    //send 4 blocks
    if (uxQueueMessagesWaiting(in_Q) >= D2H_NUM_AUDIO_BLOCKS_TO_SEND) {
        // Check buffer availability
        if (xSemaphoreTake(p_hif_channel_info->xSemaphore, 0) == pdTRUE) {
            QAI_DataBlock_t* pdatablk;
            for (int num_blocks = 0; num_blocks < D2H_NUM_AUDIO_BLOCKS_TO_SEND; num_blocks++)
            {
                xResult = xQueueReceive(in_Q, &pdatablk, 0);
                configASSERT(xResult != pdFAIL);
                int block_size = (pdatablk->dbHeader.numDataElements*pdatablk->dbHeader.dataElementSize);
                configASSERT( (block_size+kbytes) <= D2H_MAX_DATA_SIZE ) ;
                memcpy(aucAudioBuffer+kbytes, pdatablk->p_data, block_size);
                kbytes += block_size; // pdatablk->dbHeader.numDataElements*pdatablk->dbHeader.dataElementSize;
                /* release only if valid block pointer */
                if(pdatablk) {
                    /* only if nonzero usecount, else it is an error */
                    /* Should we assert here since this should never happen ? */
                    if(pdatablk->dbHeader.numUseCount > 0)
                       datablk_mgr_release_generic(pdatablk);
                }
            }
            SendEventAudioTransportBlockReadyD2H(p_hif_channel_info, aucAudioBuffer, kbytes);
            //timeout_count_bytes += kbytes;
            byte_count += kbytes;
        }
    }
  
    return byte_count;
}

#if (FEATURE_MULTI_CHANNEL_STREAM_DEVICE == 1)

void hif_release_mch_inputQ(void) {
    BaseType_t  xResult;
    
    while(uxQueueMessagesWaiting(q_mch_inQ_hif)) {
        QAI_DataBlock_t* pdatablk;
        xResult = xQueueReceive(q_mch_inQ_hif, &pdatablk, 0);
        configASSERT(xResult != pdFAIL);
        if(pdatablk) { 
            /* only if nonzero usecount, else it is an error */
            /* Should we assert here since this should never happen ? */
            if(pdatablk->dbHeader.numUseCount > 0)
            datablk_mgr_release_generic(pdatablk);
        }
    }
}

void hif_SendData(hif_channel_info_t* p_hif_channel_info) {

    int byte_count = 1;

#if 1    
    //release same number of datablocks from circular buffer
    hif_release_inputQ_count(D2H_NUM_AUDIO_BLOCKS_TO_SEND);
    
    //keep sending until queue has less blocks 
    while(byte_count) {
      byte_count = send_data_to_host(p_hif_channel_info, q_mch_inQ_hif);
    }
#else //this is when 2 or 3 channel data is disabled
    hif_release_mch_inputQ();
    byte_count = send_data_to_host(p_hif_channel_info, q_id_inQ_hif);
    
#endif    
#if 1 //have a built in 20 sec data timeout to stop streaming    
    if(timeout_count_bytes >= (20*16000*2))
    {
      timeout_count_bytes = 0;

      printf("===Internal Timeout 13 secs\n");
      hifcb_cmd_host_process_off((void *)0, 0);

    }
#endif    
}
#else //use the Circular buffer queue to stream
void    hif_SendData(hif_channel_info_t* p_hif_channel_info) {
    BaseType_t  xResult;
    uint16_t     kbytes = 0;

    //send 4 blocks
    if (uxQueueMessagesWaiting(q_id_inQ_hif) >= D2H_NUM_AUDIO_BLOCKS_TO_SEND) {
        // Check buffer availability
        if (xSemaphoreTake(p_hif_channel_info->xSemaphore, 0) == pdTRUE) {
            QAI_DataBlock_t* pdatablk;
            for (int num_blocks = 0; num_blocks < D2H_NUM_AUDIO_BLOCKS_TO_SEND; num_blocks++)
            {
                xResult = xQueueReceive(q_id_inQ_hif, &pdatablk, 0);
                configASSERT(xResult != pdFAIL);
                int block_size = (pdatablk->dbHeader.numDataElements*pdatablk->dbHeader.dataElementSize);
                configASSERT( (block_size+kbytes) <= D2H_MAX_DATA_SIZE ) ;
                memcpy(aucAudioBuffer+kbytes, pdatablk->p_data, block_size);
                kbytes += block_size; // pdatablk->dbHeader.numDataElements*pdatablk->dbHeader.dataElementSize;
                /* release only if valid block pointer */
                if(pdatablk) {
                    /* only if nonzero usecount, else it is an error */
                    /* Should we assert here since this should never happen ? */
                    if(pdatablk->dbHeader.numUseCount > 0)
                       datablk_mgr_release_generic(pdatablk);
                }
            }
            SendEventAudioTransportBlockReadyD2H(p_hif_channel_info, aucAudioBuffer, kbytes);
            timeout_count_bytes += kbytes;
        }
    }
#if (STREAM_KP_DETECT_ENABLE == 1) //after ~2sec of data streamed, re-enable VR
extern void enable_stream_VR(void);
    if(timeout_count_bytes >= (2*16000*2))
    {
      if(stream_kp_detect_enable == 0)
      {
        enable_stream_VR();
        stream_kp_detect_enable = 1;
      }
    }
    
#endif
    
#if 1 //have a built in 10 sec data timeout to stop streaming    
    if(timeout_count_bytes >= (12*16000*2))
    {
      timeout_count_bytes = 0;

      printf("===Internal Timeout 12 secs\n");
      hifcb_cmd_host_process_off((void *)0, 0);

    }
#endif    
}
#endif //FEATURE_MULTI_CHANNEL_STREAM_DEVICE

void hif_msg_sendOpusBlockReady(QAI_DataBlock_t *pdata_block)
{
  UINT32_t address = (UINT32_t)pdata_block;
  struct xQ_Packet packet={0};
  packet.ucCommand = MESSAGE_AUDIO_OPUS_BLOCK_READY;
  packet.ucSrc = HOSTIF_TASK_MESSAGE;
  packet.ucData[0] = address & 0xff;
  packet.ucData[1] = (address >> 8) & 0xff;
  packet.ucData[2] = (address >> 16) & 0xff;
  packet.ucData[3] = (address >> 24) & 0xff;
  xQueueSend( xHandleQueueHostIf, ( void * )&packet, 0 );
}
void hif_msg_sendRawBlockReady(QAI_DataBlock_t *pdata_block)
{
  UINT32_t address = (UINT32_t)pdata_block;
  struct xQ_Packet packet={0};
  packet.ucCommand = MESSAGE_AUDIO_RAW_BLOCK_READY;
  packet.ucSrc = HOSTIF_TASK_MESSAGE;
  packet.ucData[0] = address & 0xff;
  packet.ucData[1] = (address >> 8) & 0xff;
  packet.ucData[2] = (address >> 16) & 0xff;
  packet.ucData[3] = (address >> 24) & 0xff;
  xQueueSend( xHandleQueueHostIf, ( void * )&packet, 0 );
}

void hif_msg_sendStopTx(void)
{
  UINT32_t address = NULL;
  struct xQ_Packet packet={0};
  packet.ucCommand = MESSAGE_STOP_TX;
  packet.ucSrc = HOSTIF_TASK_MESSAGE;
  packet.ucData[0] = address & 0xff;
  packet.ucData[1] = (address >> 8) & 0xff;
  packet.ucData[2] = (address >> 16) & 0xff;
  packet.ucData[3] = (address >> 24) & 0xff;
  xQueueSend( xHandleQueueHostIf, ( void * )&packet, 0 );
}
#if (FEATURE_MULTI_CHANNEL_STREAM_DEVICE == 1)

// for 3 channel wifi

//this is also defined in ql_vr_app.c
typedef struct {
  QAI_DataBlockHeader_t dbHeader;
  int16_t pcm_data[AUDIO_BLOCK_SIZE_IN_SAMPLES];
} QAI_AudioBrick_t ;

QAI_AudioBrick_t wifi_dma_buffer[MULTICHANNEL_QUEUE_LENGTH];
QAI_DataBlockMgr_t wifi_DataBlkMgr;
/*
struct st_dbm_init {
  QAI_DataBlockMgr_t *pdatablk_mgr_handle;
  void  *pmem;
  int mem_size;
  int item_count;
  int item_size_bytes;
} ;
*/
void init_wifi_buffer(void)
{
  memset(wifi_dma_buffer, 0, sizeof(wifi_dma_buffer));
  datablk_mgr_init(&wifi_DataBlkMgr ,
                  wifi_dma_buffer,
                  sizeof(wifi_dma_buffer),
                  AUDIO_BLOCK_SIZE_IN_SAMPLES,
                  2);
   dbg_queue_monitor_add(wifi_DataBlkMgr.dataBlockFreeQ);
   vQueueAddToRegistry( wifi_DataBlkMgr.dataBlockFreeQ, "WifiDataBQ" );

   return;
}

QAI_DataBlockMgr_t* get_audio_block_manager(void)
{
  return &wifi_DataBlkMgr;
}
int get_audio_block_size_samples(void)
{
  //return AUDIO_BRICK_SIZE_SAMPLES;
  return AUDIO_BLOCK_SIZE_IN_SAMPLES;
}

#endif // FEATURE_MULTI_CHANNEL_STREAM_DEVICE 