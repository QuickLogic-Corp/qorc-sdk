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
 *    File   : ql_hostTask.c
 *    Purpose: host task for QL smart remote test application 
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"

#include "FreeRTOS.h"
#include "timers.h"

#include "ql_hostTask.h"
#include "RtosTask.h"
#include "dbg_uart.h"
#include "qlspi_s3.h"
#include "qlspi_fw_loader.h"
#include "firmware_raw_image.h" 
#include "h2d_protocol.h"
#include "eoss3_hal_gpio.h"
#include "eoss3_hal_spi.h"
#include "dbg_queue_monitor.h"
#include "ww_metadata.h"

xTaskHandle xHandleTaskHost;
QueueHandle_t Host_MsgQ;

extern int spi_master_init(uint32_t baud_rate);

#if DEBUG_H2D_PROTOCOL
uint8_t test_write_buf [DATA_READ_WRITE_SIZE] = {0};
uint8_t test_read_buf [DATA_READ_WRITE_SIZE] = {0};
uint8_t pattern = 0xAC;
#endif

#define HOST_TRANSPORT_CHUNK_SIZE  1024
#define NUM_TRANSPORT_CHUNKS       (50*3)
#define MAX_RX_STORAGE_BUFF_SIZE   (280*1024)
//(HOST_TRANSPORT_CHUNK_SIZE * NUM_TRANSPORT_CHUNKS)

extern void config_set_pad_for_device_bootstrap(void);
extern void spi_master_pad_setup();

uint8_t g_host_device_channel_num = PROTOCOL_CHANNEL_NUMBER_DEFAULT;

int8_t host_set_rx_channel(int8_t channel)
{
  switch(channel)
  {
  case PROTOCOL_CHANNEL_NUMBER_OPUS:
    g_host_device_channel_num = channel;
    dbg_str_int("   INFO : OPUS channel ", g_host_device_channel_num);
    break;
  case PROTOCOL_CHANNEL_NUMBER_RAW:
    g_host_device_channel_num = channel;
    dbg_str_int("   INFO : RAW channel ", g_host_device_channel_num);
    break;
  default:
    dbg_str_int(" ERROR : invalid channel numner : ", channel);
    break;
  }
  return 0;
}

//unsigned char g_opus_transport_buffer[HOST_TRANSPORT_CHUNK_SIZE];
extern uint8_t g_data_buf[];

/* static overlay the rawBuff with rx buffer */
uint8_t *g_p_rx_storage_buffer = (uint8_t *)&rawData[0]; //[MAX_RX_STORAGE_BUFF_SIZE];
static int storage_wr_index = 0;
static int unfilled_data_size = MAX_RX_STORAGE_BUFF_SIZE;
static int filled_data_size = 0;

/* will not overwrite the buffer. will hold the values till next session */
int opus_test_en = 1;

// to maintain the sequence number for cmds
static int seq = -1;

int g_recorded_duration = 0; 
int g_ts_last = -1;
int g_seq_num_last = -1;

void flush_opus_storage_buf(void)
{
    storage_wr_index=0;
    unfilled_data_size = MAX_RX_STORAGE_BUFF_SIZE;
    filled_data_size = 0;
    return;
}
struct  {
  int channel;
  int packet_size;
}o_channel_info =  {
  .channel = -1,
  .packet_size = -1
};


void display_rx_buf_addr_size(void)
{
    dbg_str_hex32("RX buffer memory address  = ", (uint32_t)g_p_rx_storage_buffer);
    dbg_str_hex32("RX buffer size to read  = ", storage_wr_index);
    dbg_str_hex32("RX buffer mem end address  = ", (uint32_t)(g_p_rx_storage_buffer + storage_wr_index));
    dbg_str_int("channel = ",o_channel_info.channel);
    int byte_count = o_channel_info.packet_size; 
    dbg_str_int("byte_count = ",byte_count);
    dbg_str_int("channel number = ",g_host_device_channel_num);
    dbg_str_fraction("duration = ", g_recorded_duration, 1000);
    g_recorded_duration = 0;
    g_ts_last = -1;
    g_seq_num_last = -1;
}

#if TST_HEADER_VERIFY == 1
int q_raw_seqnum = -1;
int check_packet(uint8_t *p_chunk, int sz)
{
  int ret = 0;
  uint32_t* p_int = (uint32_t*)p_chunk;
#if 1
  uint32_t csum_ref = p_int[2];
  uint32_t csum = 0;
    for(int n = 4; n < sz>>2; n++)
    {
      csum += p_int[n];
    }
    if(csum_ref != csum)
    {
      printf("ERROR at %d : %x %x \n", p_int[3], csum_ref, csum);
    }
   else
   {
   }
#endif
   if(p_chunk[0] != 0) // numUseCount
   {
   }
     if(p_chunk[1] != 0) // numDropCount
   {
     printf(" numDropCount = %d ", p_chunk[1]);
   }
   if(p_int[3] - q_raw_seqnum != 1 & q_raw_seqnum != -1)
   {
      printf("ERROR at %d : %d %d %d\n", p_int[3], p_int[3], q_raw_seqnum, p_int[3] - q_raw_seqnum);
   }
   q_raw_seqnum = p_int[3];
 return 0;
}
void check_chunk(uint8_t *p_chunk, int sz)
{
  int block_sz = (sz>>2);
  check_packet(p_chunk, block_sz);
  check_packet(&p_chunk[block_sz], block_sz);
  check_packet(&p_chunk[block_sz*2], block_sz);
  check_packet(&p_chunk[block_sz*3], block_sz);
  q_raw_seqnum = -1; // reset 
}
#endif

#include "datablk_mgr.h"
int8_t *prn_hdr( QAI_DataBlock_t *pdata_block_in)
{
  int8_t *p_in = 0;
#if TST_HEADER_VERIFY == 1
  p_in =   (int8_t *)((uint8_t *)pdata_block_in  + offsetof(QAI_DataBlock_t, p_data));
  //printf("%d %d\n", pdata_block_in->dbHeader.Tstart, pdata_block_in->dbHeader.Tend);
  if( pdata_block_in->dbHeader.Tend != g_seq_num_last + 4  && g_seq_num_last != -1)
  {
    printf(" >>> [%d - %d = %d]", g_seq_num_last, pdata_block_in->dbHeader.Tend, pdata_block_in->dbHeader.Tend- g_seq_num_last);
  }
  else
  {
    if(g_seq_num_last != -1)
      g_recorded_duration += (pdata_block_in->dbHeader.Tstart - g_ts_last);
  }     
  g_seq_num_last = pdata_block_in->dbHeader.Tend;
  g_ts_last = pdata_block_in->dbHeader.Tstart;
#else
  p_in = (int8_t *)((uint8_t *)pdata_block_in );
#endif
  return p_in;
}

void store_raw_transport_chunks(int32_t kbytes)
{
  int8_t*   p_in = g_data_buf;
    if ( kbytes > unfilled_data_size)           // circular buffer wrap around case
    {        
        printf("     Buffer Full %d, overwriting \n", MAX_RX_STORAGE_BUFF_SIZE);
        filled_data_size = MAX_RX_STORAGE_BUFF_SIZE - unfilled_data_size;  //"filled_data_size" size will be used for file dump in jlink savebin command.
        unfilled_data_size = MAX_RX_STORAGE_BUFF_SIZE;
        storage_wr_index = 0; //We can put break point here to dump g_rx_storage_buffer to file using save bin command.
        g_recorded_duration = 0;
    }
    memcpy(&g_p_rx_storage_buffer[storage_wr_index], p_in, kbytes);
    storage_wr_index += kbytes;
    unfilled_data_size -= kbytes;
}

void store_opus_transport_chunks(int length)
{
  QAI_DataBlock_t *pdata_block_in = (QAI_DataBlock_t*)g_data_buf;
  int32_t payload_len = pdata_block_in->dbHeader.numDataElements*pdata_block_in->dbHeader.dataElementSize;
  int8_t *p_in;
  p_in = prn_hdr(pdata_block_in);
    if (unfilled_data_size <= length)           // circular buffer wrap around case
    {
        if(opus_test_en)
        {
            // do nothing 
          return;
        }        
        printf("     Buffer Full %d, overwriting \n", MAX_RX_STORAGE_BUFF_SIZE);
        //filled_data_size = MAX_RX_STORAGE_BUFF_SIZE - unfilled_data_size;  //"filled_data_size" size will be used for file dump in jlink savebin command.
        unfilled_data_size = MAX_RX_STORAGE_BUFF_SIZE;
        storage_wr_index = 0; //We can put break point here to dump g_rx_storage_buffer to file using save bin command.
        g_recorded_duration = 0;
    }
    memcpy(&g_p_rx_storage_buffer[storage_wr_index], p_in, payload_len);
    storage_wr_index += payload_len;
    unfilled_data_size -= payload_len;
}



/*  Add Msg to the Host task queue */
uint32_t addPktToQueue_Host(struct xQ_Packet *pxMsg, int ctx)
{
  
	uint32_t uiErrCode = eQL_SUCCESS;
    if( CTXT_ISR == ctx)         // ISR context
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if( xQueueSendFromISR( Host_MsgQ, ( void * )pxMsg, &xHigherPriorityTaskWoken ) != pdPASS )
            uiErrCode = eQL_ERR_MSG_SEND;
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
    else                        // TASK context
    {
        if(xQueueSend( Host_MsgQ, ( void * )pxMsg, HOST_MSGQ_WAIT_TIME ) != pdPASS)
            uiErrCode = eQL_ERR_MSG_SEND;
    }
	
	return uiErrCode;
}


Rx_Cb_Ret h2d_receive_callback(H2D_Cmd_Info rx_cmd_info, uint8_t data_buf_ready)
{
    Rx_Cb_Ret ret = {0};
    struct xQ_Packet rxPkt = {0};
#if DEBUG_H2D_PROTOCOL
    dbg_str("callback invoked\n");

    // display the unpacked cmd received
    dbg_str_int("seq = ",rx_cmd_info.seq);
    dbg_str_int("channel = ",rx_cmd_info.channel);
    dbg_str_int("cmd = ",rx_cmd_info.cmd);
    dbg_str_int("data[0] = ",rx_cmd_info.data[0]);
    dbg_str_int("data[1] = ",rx_cmd_info.data[1]);
    dbg_str_int("data[2] = ",rx_cmd_info.data[2]);
    dbg_str_int("data[3] = ",rx_cmd_info.data[3]);
    dbg_str_int("data[4] = ",rx_cmd_info.data[4]);
    dbg_str_int("data[5] = ",rx_cmd_info.data[5]);
#endif
    /* Check if the cmd received is EVT_OPUS_PKT_READY
        if yes, read the address and len from the data field and
        read that data over qlspi and store it in opus buffer
        after read is complete, send EVT_OPUS_PKT_READY msg to the host task
    */
    switch(rx_cmd_info.cmd)
    {
        case EVT_OPUS_PKT_READY :
            if(data_buf_ready){       // if data buf is read
                // store the data in opus buffer
                uint16_t length = (rx_cmd_info.data[0]) | (rx_cmd_info.data[1] << 8 );
                // this copy data from g_data_buf in protocol layer to opus buffer
                //store_opus_transport_chunks(length);

            }
            else
            {                     // read the data
                ret.data_read_req = 1;
                ret.len = (rx_cmd_info.data[0]) | (rx_cmd_info.data[1] << 8 );
                ret.addr = ((rx_cmd_info.data[2]) | (rx_cmd_info.data[3] << 8) |     \
                (rx_cmd_info.data[4] << 16) | (rx_cmd_info.data[5] << 24));
                o_channel_info.packet_size = ret.len;
                o_channel_info.channel = rx_cmd_info.channel;

                return ret;         // return from callback with more data read request.
            }
            break;
            
        case EVT_RAW_PKT_READY :
            if(data_buf_ready){       // if data buf is read
                // store the data in opus buffer
                uint16_t length = (rx_cmd_info.data[0]) | (rx_cmd_info.data[1] << 8 );
                // this copy data from g_data_buf in protocol layer to raw buffer
                store_raw_transport_chunks(length);

            }
            else
            {                     // read the data
                ret.data_read_req = 1;
                ret.len = (rx_cmd_info.data[0]) | (rx_cmd_info.data[1] << 8 );
                ret.addr = ((rx_cmd_info.data[2]) | (rx_cmd_info.data[3] << 8) |     \
                (rx_cmd_info.data[4] << 16) | (rx_cmd_info.data[5] << 24));
                o_channel_info.packet_size = ret.len;
                o_channel_info.channel = rx_cmd_info.channel;

                return ret;         // return from callback with more data read request.
            }
            break;
        case EVT_GET_MONINFO :
            if(data_buf_ready){       // if data buf is read
            }
            else
            {                     // read the data
               ret.data_read_req = 1;
               ret.len = (rx_cmd_info.data[0]) | (rx_cmd_info.data[1] << 8 );
               ret.addr = ((rx_cmd_info.data[2]) | (rx_cmd_info.data[3] << 8) |     \
                           (rx_cmd_info.data[4] << 16) | (rx_cmd_info.data[5] << 24));
               o_channel_info.packet_size = ret.len;
                o_channel_info.channel = rx_cmd_info.channel;
               return ret;         // return from callback with more data read request.
            }
            break;
        case EVT_KP_DETECTED :
            if(data_buf_ready){       // if data buf is read
            }
            else
            {                     // read the data
                ret.len = (rx_cmd_info.data[0]) | (rx_cmd_info.data[1] << 8 );
                ret.addr = ((rx_cmd_info.data[2]) | (rx_cmd_info.data[3] << 8) |     \
                (rx_cmd_info.data[4] << 16) | (rx_cmd_info.data[5] << 24));
                if ( (ret.len) && (ret.addr) )
                {
                  ret.data_read_req = 1;
                  o_channel_info.packet_size = ret.len;
                  o_channel_info.channel = rx_cmd_info.channel;
                }
                else
                {
                  memset(g_data_buf, 0, sizeof(t_ql_audio_meta_data));
                }
               return ret;         // return from callback with more data read request.
            }
            break;

    }
    
    rxPkt.ucCommand = rx_cmd_info.cmd;
    // copy the rest of the data as it is (6 bytes in our current implementation)
    memcpy(&(rxPkt.ucData[0]), &(rx_cmd_info.data[0]), MAX_QUEUE_PACKET_DATA_LEN );
    addPktToQueue_Host(&rxPkt,CTXT_TASK);
    
    return ret;
}

/*
function to increment the sequence number for each cmd sent
*/
static inline uint8_t increment_seq(void)
{
    seq++;
    if(0xFF < seq){
        seq = 0;
    }
    return ((uint8_t)(seq & 0xFF));
}
static TimerHandle_t StreamTimerHandle = NULL;
static void StreamTimerCB(TimerHandle_t StreamTimerHandle);
/* Host task andler*/
void hostTaskHandler(void * parameter)
{
    BaseType_t qret;
    unsigned int hostTaskStop = 0;
    struct xQ_Packet hostMsg = {0};
    s3_mon_info_t  *ps3info ;
#if DEBUG_H2D_PROTOCOL
    int i = 0;
    for(i=0;i<DATA_READ_WRITE_SIZE; ++i)
    {
        test_write_buf[i] = i;
    }
#endif
    StreamTimerHandle = xTimerCreate("StreamTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, StreamTimerCB);
    if(StreamTimerHandle == NULL)
    {
        configASSERT(0);
    }
    h2d_register_rx_callback(&h2d_receive_callback, g_host_device_channel_num );
    
    // send msg to host task to load device firmware
    hostMsg.ucCommand = HOST_LOAD_DEVICE_FW;
    addPktToQueue_Host(&hostMsg, CTXT_TASK);
    
    while(!hostTaskStop)
    {
        //clear the Msg Q buffer 
        memset(&hostMsg, 0, sizeof(hostMsg));
        qret = xQueueReceive(Host_MsgQ, &hostMsg, HOST_MSGQ_WAIT_TIME);
        configASSERT(qret == pdTRUE);
        
        switch(hostMsg.ucCommand)
        {
        case HOST_LOAD_DEVICE_FW:
          {
#if 0
            /* Reset the device hardware. QL_RST -> low-> high*/
            HAL_GPIO_Write(GPIO_0, 0);
            
            /*set GPIO19/20 of device */
            config_set_pad_for_device_bootstrap();
            vTaskDelay((1/portTICK_PERIOD_MS));
            
            
            /* Release QL_RST*/
            HAL_GPIO_Write(GPIO_0, 1);
            
            vTaskDelay((2/portTICK_PERIOD_MS));
            
            /*re configure host pads to be used for spi transaction*/
            spi_master_pad_setup();
            
            SLAVE_DEV_FW_LOAD_T slave_fw_image_info;
            /*
				Set, slave device firmware image information structure to zero.
				Initialize slave device firmware image information structure to necessary
				image to be loaded on to slave device memory.
            */
            memset(&slave_fw_image_info, 0x00, sizeof(slave_fw_image_info));

            slave_fw_image_info.m4_fw_addr = (uint8_t *)rawData;
            slave_fw_image_info.m4_fw_size = sizeof(rawData);
            
            
            if (QL_STATUS_OK != QLSPI_fw_download(&slave_fw_image_info))
            {
                printf("Device Firmware Download Failed \n");
                return;
            }
            printf("\n################# QL MCU Firmware Downloaded Successfully #################\r\n\r\n");
#else
              printf("***************** Skipped firmware download ************************\n\n");
#endif
             spi_master_init(SPI_BAUDRATE_5MHZ);
            break;
          }
          
        case EVT_KP_DETECTED:
          {
            /* Waking up process to be done*/
            t_ql_audio_meta_data *pwwinfo;
            pwwinfo = (t_ql_audio_meta_data *)g_data_buf;
            dbg_str_int_noln("wakeword index", pwwinfo->n_keyphrase_triggered_index);
            dbg_str_int_noln(" count", pwwinfo->n_keyphrase_count);
            dbg_str_int_noln(" start", pwwinfo->n_keyphrase_start_index);
            dbg_str_int_noln(" end  ", pwwinfo->n_keyphrase_end_index);
            dbg_str_int     (" score", pwwinfo->a_kephrase_score);
              
            /* For debug, reset save buffer to make svaebin easier */
            flush_opus_storage_buf();
            
          /* Once ready, send  "CMD_HOST_READY_TO_RECEIVE" to device*/
            H2D_Cmd_Info cmd_info = {0};
            cmd_info.channel = g_host_device_channel_num;
            cmd_info.seq = increment_seq();
            cmd_info.cmd = CMD_HOST_READY_TO_RECEIVE;
            
            
            if (h2d_transmit_cmd(&cmd_info)){
                dbg_str("Error returned from h2d tansmit api\n");
            }
            dbg_str("CMD_HOST_READY_TO_RECEIVE cmd sent\n");
            
            // Start timer that will trigger a stop
            xTimerStart(StreamTimerHandle, 0);
            printf("Started timer\n");
          
          break;
          }
        case EVT_OPUS_PKT_READY:
        case EVT_RAW_PKT_READY :
            /* receive callback will send this msg only after reading the opus data in the buffer*/
          dbg_str(".");
            break;
         
        case EVT_OPUS_PKT_END:
              /*opus packet end message*/
            dbg_str("EVT_DATA_PKT_END\n");
            display_rx_buf_addr_size();
            break;
            
        case EVT_EOT:
            dbg_str("EVT_EOT\n");
            break;
            
        case EVT_GET_MONINFO:
            ps3info = (s3_mon_info_t *)g_data_buf;
            dbg_str_int("num_drop_count", ps3info->num_drop_count);
            dbg_str_int("heap_size", ps3info->heap_size);
            dbg_str_int("Qs registered", ps3info->dqmArraySize);
            for (int k = 0; k < ps3info->dqmArraySize; k++) {
              QueueHandle_t q ;
              int           qsiz;
              q = ps3info->dbg_queue_monitor_array[k];
              qsiz = ps3info->dbg_queue_monitor_value[k];
              dbg_str_int_noln("Queue ", (int)q);
              dbg_str_int(" Length ", qsiz);
            }
            dbg_str("EVT_GETMONINFO\n");
            break;
            
        case HOST_CMD_READ_DATA_FROM_S3 :
            // for debug/testing purpose only
#if DEBUG_H2D_PROTOCOL
            dbg_str("received cmd HOST_CMD_READ_DATA_FROM_S3\n");
            QLSPI_Read_S3_Mem(H2D_READ_ADDR,test_read_buf,DATA_READ_WRITE_SIZE);
            dbg_memdump8((intptr_t)(test_read_buf),(void *)(test_read_buf),DATA_READ_WRITE_SIZE);
#endif
            break;
            
        case HOST_CMD_WRTIE_DATA_TO_S3 :
            {
              // for debug/testing purpose only
#if DEBUG_H2D_PROTOCOL
                dbg_str("received cmd HOST_CMD_WRTIE_DATA_TO_S3\n");
                int i = 0;
                for(i=0;i<DATA_READ_WRITE_SIZE; ++i)
                {
                    test_write_buf[i] = pattern;
                }
                dbg_memdump8((intptr_t)(test_write_buf),(void *)(test_write_buf),DATA_READ_WRITE_SIZE);
                QLSPI_Write_S3_Mem(H2D_WRITE_ADDR,test_write_buf,DATA_READ_WRITE_SIZE);
#endif
            break;
            }
            
        case HOST_SEND_CMD_TO_DEVICE :
            {
              // for debug/testing purpose only
                H2D_Cmd_Info cmd_info = {0};
                cmd_info.channel = g_host_device_channel_num;
                cmd_info.seq = increment_seq();
                cmd_info.cmd = hostMsg.ucData[0];
                cmd_info.data[0] = 0x01;
                cmd_info.data[1] = 0x11;
                cmd_info.data[2] = 0x22;
                cmd_info.data[3] = 0x33;
                cmd_info.data[4] = 0x44;
                cmd_info.data[5] = 0x55;
                
                if (h2d_transmit_cmd(&cmd_info)){
                    dbg_str("Error returned from h2d tansmit api\n");
                }
                break;
            }

            case CMD_HOST_PROCESS_OFF:
            case CMD_HOST_PROCESS_ON:
            case CMD_HOST_MUTE_OFF:
            case CMD_HOST_MUTE_ON:
            {
              // for debug/testing purpose only
                H2D_Cmd_Info cmd_info = {0};
                cmd_info.channel = g_host_device_channel_num;
                cmd_info.seq = increment_seq();
                cmd_info.cmd = hostMsg.ucData[0];
                cmd_info.data[0] = 0x01;
                cmd_info.data[1] = 0x11;
                cmd_info.data[2] = 0x22;
                cmd_info.data[3] = 0x33;
                cmd_info.data[4] = 0x44;
                cmd_info.data[5] = 0x55;
                
                if (h2d_transmit_cmd(&cmd_info)){
                    dbg_str("Error returned from h2d tansmit api\n");
                }
                break;
            }
            
        default :
            break;
            
        }
    }
    
    return;
}

void StreamTimerCB(TimerHandle_t StreamTimerHandle) {

    /* Once ready, send  "CMD_HOST_PROCESS_OFF" to device*/
    H2D_Cmd_Info cmd_info = {0};
    cmd_info.channel = g_host_device_channel_num;
    cmd_info.seq = increment_seq();
    cmd_info.cmd = CMD_HOST_PROCESS_OFF;


    if (h2d_transmit_cmd(&cmd_info)){
        dbg_str("Error returned from h2d tansmit api\n");
    }
    dbg_str("CMD_HOST_PROCESS_OFF cmd sent\n");
}

/* Setup msg queue and Task Handler for Host Task */
signed portBASE_TYPE StartRtosTaskHost( void)
{
    static uint8_t ucParameterToPass;

    /* Create queue for Host Task */
    Host_MsgQ = xQueueCreate( HOST_QUEUE_LENGTH, sizeof(struct xQ_Packet) );
    vQueueAddToRegistry( Host_MsgQ, "Host_MsgQ" );
    configASSERT( Host_MsgQ );
    
    /* Create BLE Task */
    xTaskCreate ( hostTaskHandler, "HostTaskHandler", STACK_SIZE_ALLOC(STACK_SIZE_TASK_HOST),  &ucParameterToPass, PRIORITY_TASK_HOST, &xHandleTaskHost);
    configASSERT( xHandleTaskHost );
    
    return pdPASS;
}



