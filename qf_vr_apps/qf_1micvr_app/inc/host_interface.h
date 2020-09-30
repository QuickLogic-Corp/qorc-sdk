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
*    File   : host_interface.h
*    Purpose:
*=========================================================*/

#ifndef HOST_INTERFACE_H
#define HOST_INTERFACE_H

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <queue.h>
#include "common.h"
#include <stdbool.h>

#include "test_types.h"
#include "ql_util.h"
#include "qlsh_commands.h"
#include "datablk_mgr.h"
#include "dbg_queue_monitor.h"
#include "s3_host_proto_defs.h"

#define HOST_TRANSPORT_CHUNK_SIZE  (TX_SPI_AUDIO_RAW_BLOCK_SZ)

#define HOSTIF_ISR_MESSAGE  ( 0x0ff & ('H' + 'I' + 'S' + 'R') )
#define HOSTIF_TASK_MESSAGE  ( 0x0ff & ('H' + 'T' + 'A' + 'S' + 'K') )
#define HIF_COMMAND_TABLE_SIZE   (64)

#if 0
typedef enum HIF_MESSAGE_
{
  MESSAGE_VM_WAKEUP             = 0x01,
  MESSAGE_AUDIO_OPUS_CHUNK_DONE = 0x02,
  MESSAGE_KPT_DETECTED          = 0x03, 
  MESSAGE_STOP_AUDIO            = 0x04,
  MESSAGE_START_AUDIO           = 0x05,
  MESSAGE_AUDIO_OPUS_BLOCK_READY= 0x06,
  MESSAGE_AUDIO_RAW_BLOCK_READY = 0x07,
  MESSAGE_AUDIO_PCM_CHUNK_DONE  = 0x22,
  MESSAGE_STOP_TX,
  MESSAGE_NOP                               // Does nothing (used for side effect of monitoring D2H status */
}HIF_MESSAGE;

typedef enum HIF_EVT_
{
  EVT_KP_DETECTED     = 0x10,
  EVT_OPUS_PKT_READY  = 0x11,
  EVT_OPUS_PKT_END    = 0x12,
  EVT_RAW_PKT_READY   = 0x21,
  EVT_RAW_PKT_END     = 0x22,
  EVT_EOT
}HIF_EVT;

enum HIF_CMD
{
  CMD_HOST_READY_TO_RECEIVE = 0x1,
  CMD_HOST_PROCESS_OFF      = 0x2,
  CMD_HOST_PROCESS_ON       = 0x3,
  CMD_HOST_MUTE_OFF         = 0x4,
  CMD_HOST_MUTE_ON          = 0x5
};
#endif
enum HIF_CHAN_NUM_E{
 HIF_CHAN_NUM_AUDIO = 10,    /* PROTOCOL_CHANNEL_NUMBER for all audio */
 HIF_CHAN_NUM_AUDIO_PCM1 = 10    /* PCM mono 16 bit */
};
typedef struct _hif_channel_info
{
  int8_t           sequence_number;
  int8_t           sequence_numberCompleted;
  int32_t           channel_number;
  int32_t           firstTime;
  SemaphoreHandle_t xSemaphore;
  uint8_t*          pucData;
  QAI_DataBlock_t *pdata_block;
}hif_channel_info_t;

typedef struct st_hif_command_table {
  int icommand;
  void (*pcommand_func)(void *p_data, int length);
} hif_command_table_t ;
extern hif_command_table_t hif_command_table[];
extern int hif_command_table_size;

extern QueueHandle_t  q_id_inQ_hif;
extern SemaphoreHandle_t g_host_ready_lock;

void    hostIfTaskHandler(void *pParameter);
void    init_protocol_variables(hif_channel_info_t *p_hif_channel_info);
void    ql_hif_msg_send_KPDetected(void);
void    hif_data_addToQueueFromISR(struct xQ_Packet* pxMsg);
void    hif_msg_sendOpusChunkDone(UINT8_t* uintchunkStartAddress, int length);
void    hif_msg_sendRawChunkDone(UINT8_t* p_buffer, int length);
void    opus_host_msg_callback(int opus_bytes_available);
void    hif_msg_send_streamKPDetected(void);
void    hif_msg_sendKPDetected(void);
void    hif_msg_sendStopTx(void);
void    hif_enableCommunication(uint8_t val);
bool    hif_isCommunicationEnabled();
bool    isStreamingOn(void);
void    setStreamingOff(void);
void    hif_SendData(hif_channel_info_t* p_hif_channel_info);
void    hif_release_inputQ(void);

bool isLpsdIgnoreSet(void);
void setLpsdIgnoreState(int val);

void cbuff_audio_flow_disable(void);

void cbuff_audio_flow_enable(void);
void send_data_callback(int16_t *blockPtr, int block_size);
int32_t  hif_tx_buffer_addSamples(int16_t *p_buff, int32_t n_samples);
void  hif_tx_buffer_sendSamples(int32_t n_samples);
void  hif_tx_buffer_sendSamples_opus(int32_t n_samples);

/* internal APIs for taking clock QoS*/
/*!
* \fn       void set_spi_session_qos(uint8_t status)
* \brief   Function to set C01 clock to min value for spi read from host to succeed
* \param   status -- 1 : set qos. 0- release QoS
* \returns -
*/
void set_spi_session_qos(uint8_t status);

/*!
* \fn       void set_audio_clock_qos(uint8_t status)
* \brief   Function to set Hsosc and C10 clocks to min value for audio pipeline to work
* \param   status -- 1 : set qos. 0- release QoS
* \returns -
*/
void set_audio_clock_qos(uint8_t status);
/*!
* \fn       void hif_register_qos(void)
* \brief   Function toregister QoS node for C01 and C30
* \param   -
* \returns -
*/
void hif_register_qos(void);
void hif_msg_sendOpusBlockReady(QAI_DataBlock_t *pdata_block);
void hif_msg_sendRawBlockReady(QAI_DataBlock_t *pdata_block);
#endif  /* HOST_INTERFACE_H */
