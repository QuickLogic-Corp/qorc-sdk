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
#include "fsm.h"
#include "dbg_queue_monitor.h"
#include "ww_metadata.h"
#include "datablk_mgr.h"
#include "eoss3_hal_uart.h"
#include "ql_base64.h"

void    h2d_config(void);
void    h2d_start(void);
/*********************************************************************************
 *
 *  HOST-FSM interface
 *
 ********************************************************************************/
int                 H2D_FSMConfigData;
enum process_state  H2D_pstate;
       
enum process_state H2D_FSMAction(enum process_action pa, void* pv) {    
    switch(pa) {
    case PACTION_CONFIG:
        h2d_config();
        H2D_pstate = PSTATE_STOPPED;
        break;
        
    case PACTION_START:
        h2d_start();
        H2D_pstate = PSTATE_STARTED;
        break;
            
    default:
        configASSERT(0);
        break;
    }
    return(H2D_pstate);
}        

/*********************************************************************************
 *  Print Audio Data to the USB Serial port 
 ********************************************************************************/
#if (FEATURE_USBSERIAL == 1)

int disable_usb_prints = 0;

const char kp_info_buf[] =  "\nKP detected. Count = ";
const char kp_info_buf2[] =  "Base64 encoded data follows ... \n\n\n\n";

const char kp_info_buf3[] =  "\n\n\n\nEnd of Transmission. Recvd sample count = ";

void set_usb_print_disable(void) {
  disable_usb_prints = 1;
}
void print_kp_detect_info(int count) {
  char kp_count_buf[10];
  int count_size = sprintf(kp_count_buf, "%d. ",count);
  
  uart_tx_raw_buf( UART_ID_USBSERIAL, (uint8_t const *)kp_info_buf, sizeof(kp_info_buf)); 
  uart_tx_raw_buf( UART_ID_USBSERIAL, (uint8_t const *)kp_count_buf, count_size);
  uart_tx_raw_buf( UART_ID_USBSERIAL, (uint8_t const *)kp_info_buf2, sizeof(kp_info_buf2));

  return;
}
void print_kp_end_info(int count) {
  char sample_count_buf[12];
  int count_size = sprintf(sample_count_buf, "%d. ",count);
  sample_count_buf[count_size++] = '\n';
  sample_count_buf[count_size++] = '\n';
  
  uart_tx_raw_buf( UART_ID_USBSERIAL, (uint8_t const *)kp_info_buf3, sizeof(kp_info_buf3)); 
  uart_tx_raw_buf( UART_ID_USBSERIAL, (uint8_t const *)sample_count_buf, count_size);
  
  return;
}
#define BASE64_LINE_SIZE_SAMPLES    (30)  //must be a multiple of 3
#define BASE64_LINE_SIZE_OUTBYTES   (BASE64_LINE_SIZE_SAMPLES*2*4/3) //30*2*4/3 = 80 bytes
uint8_t base64_line_buf[BASE64_LINE_SIZE_OUTBYTES +2]; // +2 to add new line

#if 0
char test_base64_line_buf[81] = {
  '0','1','2','3','4','5','6','7','8','9',     '0','1','2','3','4','5','6','7','8','9',   
  '0','1','2','3','4','5','6','7','8','9',     '0','1','2','3','4','5','6','7','8','9',   
  '0','1','2','3','4','5','6','7','8','9',     '0','1','2','3','4','5','6','7','8','9',   
  '0','1','2','3','4','5','6','7','8','9',     '0','1','2','3','4','5','6','7','8','9',     
  '\n'  
};
#endif

int sine_1khz_index = 0;
int16_t sine_1khz[16] = {
    0,  6270, 11585, 15137, 16384, 15137, 11585,  6270,
    0, -6270,-11585,-15137,-16384,-15137,-11585, -6270
};
//replace the samples with 1kHz sine wave values
void test_sine_1khz_values(short *buf, int size)
{
  int count = sine_1khz_index;
  for(int i = 0; i < size; i++)
  {
    buf[i] = sine_1khz[count++];
    if(count >= 16)   {
      count =0;
    }
  }
  sine_1khz_index = count;
  return;
}
void print_kp_raw_audio_data(uint8_t *input_buf, int input_size)
{
  //test_sine_1khz_values((short *)input_buf, input_size/2);
    
  //if USB is not working or disabled return
  if(disable_usb_prints)
    return;

  int input_bytes_per_line = (2*BASE64_LINE_SIZE_SAMPLES);
  int input_size_lines = input_size/input_bytes_per_line;
  
  //input must be a multple of number samples per line 
  if(input_size != (input_size_lines*input_bytes_per_line)) {
    configASSERT(0);
  }
  
  int delay_count = 0;
  //print all the lines, giving time for other tasks
  for(int i = 0; i< input_size_lines; i++)
  {
    int out_count = base64EncodeLine((char *)(input_buf + i*input_bytes_per_line),
                                     input_bytes_per_line,
                                     (char *)base64_line_buf);
    //print only if there is enough space in the FIFO
    while(uart_tx_get_fifo_space_available(UART_ID_USBSERIAL) < out_count) {
      vTaskDelay(1); //let other tasks work
      delay_count++;
      //if there is no space available for 10 ms, then USB serial may be disconnected
      //so disable printing and return
      if(delay_count > 100)
      {
        disable_usb_prints = 1; 
        printf("\nError- Not able to get free space in USB FIFO. So disabling USP prints \n");
        return;
      }
    }
    //print the line buffer on USB port
    uart_tx_raw_buf( UART_ID_USBSERIAL, base64_line_buf, out_count);
    delay_count = 0;
  }
  
  //set the buffer to 0 to detect problems
  //memset(input_buf, 0, input_size);
  
  return;
}





#endif

xTaskHandle xHandleTaskHost;
QueueHandle_t Host_MsgQ;

extern int spi_master_init(uint32_t baud_rate);
extern void config_set_pad_for_device_bootstrap(void);
extern void spi_master_pad_setup();

#if DEBUG_H2D_PROTOCOL
uint8_t test_write_buf [DATA_READ_WRITE_SIZE] = {0};
uint8_t test_read_buf [DATA_READ_WRITE_SIZE] = {0};
uint8_t pattern = 0xAC;
#endif

#define HOST_TRANSPORT_CHUNK_SIZE  (240*2) //one 15ms data block
#define NUM_TRANSPORT_CHUNKS       (4)  //60ms.Fits 3 opus buffers of size 20ms
#define MAX_RX_STORAGE_BUFF_SIZE   (HOST_TRANSPORT_CHUNK_SIZE * NUM_TRANSPORT_CHUNKS) 
#define MAX_RX_STORAGE_BUFF_SIZE_OPUS (MAX_RX_STORAGE_BUFF_SIZE /3) //opus data is compressed by factor
#define RX_STORAGE_BUFFERS_COUNT   (12) //need enough buffers to take care of initial burst

uint8_t rx_storage_buf[RX_STORAGE_BUFFERS_COUNT][MAX_RX_STORAGE_BUFF_SIZE];
volatile int storage_write_bufcount = 0;
volatile int storage_print_bufcount = 0;

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
uint8_t *g_p_rx_storage_buffer = (uint8_t *)&rx_storage_buf[0][0];
static int storage_wr_index = 0;
static int storage_rd_index = 0;
//static int unfilled_data_size = MAX_RX_STORAGE_BUFF_SIZE;
//static int filled_data_size = 0;

/* will not overwrite the buffer. will hold the values till next session */
int opus_test_en = 1;

// to maintain the sequence number for cmds
static int seq = -1;

int g_recorded_duration = 0; 
int g_ts_last = -1;
int g_seq_num_last = -1;

void reset_storage_buf(void)
{
    storage_wr_index=0;
    storage_rd_index=0;
    storage_write_bufcount = 0;
    storage_print_bufcount = 0;

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

void store_raw_transport_chunks(int copy_length)
{
#if (FEATURE_USBSERIAL == 1)
  //if USB prints are not disabled, need to do flow control
  //if USB prints are not catching up with burst data delay the
  //the H2D protocol response
  if(disable_usb_prints == 0)
  {
    while((storage_write_bufcount - storage_print_bufcount) >= (RX_STORAGE_BUFFERS_COUNT-2) ) {
      vTaskDelay(5); //wait until more buffers are printed
    }
  }
#else
  //only first buffers will be stored
  if(storage_wr_index >= RX_STORAGE_BUFFERS_COUNT)
    return;
#endif

  //there should always be a known fixed data size per transport
  if(copy_length != MAX_RX_STORAGE_BUFF_SIZE) {
    configASSERT(0);
  }
 
  //copy the h2d buffer into a storage buffer
  memcpy(&rx_storage_buf[storage_wr_index][0], g_data_buf, copy_length);
  storage_wr_index++;
  if(storage_wr_index >= RX_STORAGE_BUFFERS_COUNT) {
    storage_wr_index = 0;
  }
  //used for controlling the H2D protocol response
  storage_write_bufcount++;
  
  return;
}
void store_opus_transport_chunks(int copy_length)
{
  //there should always be a known fixed data size per transport
  if(copy_length != MAX_RX_STORAGE_BUFF_SIZE_OPUS) {
    configASSERT(0);
  }
  
  memcpy(&rx_storage_buf[storage_wr_index][0], g_data_buf, copy_length);
  storage_wr_index++;
  if(storage_wr_index >= RX_STORAGE_BUFFERS_COUNT) {
    storage_wr_index = 0;
  }
  
  return;
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
#if 1
            /* Reset the device hardware. QL_RST -> low-> high*/
            HAL_GPIO_Write(GPIO_0, 0);
            
            /*set GPIO19/20 of device */
#if 0 // do we need this?
            config_set_pad_for_device_bootstrap();
#endif
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
            dbg_str_int_noln("wakeword length", pwwinfo->n_keyphrase_triggered_index);
            dbg_str_int_noln(" count", pwwinfo->n_keyphrase_count);
            dbg_str_int_noln(" start", pwwinfo->n_keyphrase_start_index);
            dbg_str_int_noln(" end  ", pwwinfo->n_keyphrase_end_index);
            dbg_str_int_noln(" score", pwwinfo->a_keyphrase_score);
            dbg_str_int     (" Length Estimate", pwwinfo->n_length_estimate);
              
            /* For debug, reset save buffer to make svaebin easier */
            reset_storage_buf(); 
            
            
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

#if (FEATURE_USBSERIAL == 1)
            print_kp_detect_info(pwwinfo->n_keyphrase_count);
#endif             
            
          break;
          }
        case EVT_OPUS_PKT_READY:
        case EVT_RAW_PKT_READY :
          {
            /* receive callback will send this msg only after reading the opus data in the buffer*/
          dbg_str(".");
#if (FEATURE_USBSERIAL == 1)
          
          int recvd_length = (hostMsg.ucData[0]) | (hostMsg.ucData[1] << 8 );
          print_kp_raw_audio_data(&rx_storage_buf[storage_rd_index++][0], recvd_length);
          if(storage_rd_index >= RX_STORAGE_BUFFERS_COUNT)
            storage_rd_index = 0;

          //used for controlling the H2D protocol response
          storage_print_bufcount++;
#endif  

          }
          break;
         
        case EVT_OPUS_PKT_END:
              /*opus packet end message*/
            dbg_str("EVT_DATA_PKT_END\n");
            //display_rx_buf_addr_size();
            break;
            
        case EVT_EOT:
            dbg_str("EVT_EOT\n");

#if (FEATURE_USBSERIAL == 1)
            //At the print the numbe of samples received. 
            //Note: The number of samples printed may not match the number received
            print_kp_end_info(storage_write_bufcount * MAX_RX_STORAGE_BUFF_SIZE/2);
#endif
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
void h2d_start( void)
{
    static uint8_t ucParameterToPass;

    /* Create queue for Host Task */
    Host_MsgQ = xQueueCreate( HOST_QUEUE_LENGTH, sizeof(struct xQ_Packet) );
    vQueueAddToRegistry( Host_MsgQ, "Host_MsgQ" );
    configASSERT( Host_MsgQ );
    
    /* Create HostTask Task */
    xTaskCreate ( hostTaskHandler, "h2dTaskHandler", STACK_SIZE_ALLOC(STACK_SIZE_TASK_HOST),  &ucParameterToPass, PRIORITY_TASK_HOST, &xHandleTaskHost);
    configASSERT( xHandleTaskHost );
}

void    h2d_config(void) {
    // init h2d protocol
    H2D_Platform_Info h2d_plat_info;
    
    h2d_plat_info.H2D_gpio = GPIO_2; //PAD_11
    h2d_plat_info.D2H_gpio = GPIO_6;

#if (USE_4PIN_D2H_PROTOCOL == 1)

    h2d_plat_info.H2D_ack = GPIO_3; //PAD_30
    h2d_plat_info.D2H_ack = GPIO_7;

#endif
    
    h2d_protocol_init(&h2d_plat_info);
}

