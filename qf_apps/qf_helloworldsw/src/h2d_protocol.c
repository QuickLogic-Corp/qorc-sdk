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
 *    File   : h2d_protocol.c
 *    Purpose: host to s3 device communication protocol implementaion 
 *                                                          
 *=========================================================*/

#include "string.h"
#include "Fw_global_config.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "RtosTask.h"
#include "eoss3_dev.h"
#include "h2d_protocol.h"
#include "qlspi_s3.h"
#include "dbg_uart.h"
#include "eoss3_hal_gpio.h"


typedef struct {
	H2D_Callback rx_cb_ptr;
} H2D_Callback_Info;

typedef struct H2D_Protocol_info {
    H2D_Platform_Info pfm_info;
    H2D_Callback_Info cb_info[MAX_NUM_CHANNEL];
    uint8_t init_done;    
}H2D_Protocol_info;


/*Lock will be acquired when tx api is called and 
released in ISR after device receives the data*/ 
SemaphoreHandle_t g_h2d_transmit_lock;          // SJ : may not be required on the host side ??

xTaskHandle xHandleTaskH2DRx;
QueueHandle_t H2DRx_MsgQ;


#define H2D_DATA_NUM_BYTES  (6)
#define H2D_SEQ_MASK        (0xF)
#define H2D_SEQ_BIT_POS     (4)
#define H2D_CHANNEL_MASK_0  (0xF)
#define H2D_CHANNEL_MASK_1  (0x3)
#define H2D_CHANNEL_MASK    (0x3F)
#define H2D_CHANNEL_BIT_POS (2)
#define H2D_CMD_MASK        (0x3F)

/* structure for msg to be sent to h2drx task*/
typedef struct {
    uint8_t msg;
}H2D_Rx_Pkt;

/*gloabal variable for h2d protocol info*/
H2D_Protocol_info g_h2d_protocol_info = {0};



/*
    buf[0]          buf[1]           buf[2]         buf[3]          buf[4]          buf[5]          buf[7]          buf[7]
*--------------------------------------------------------------------------------------------------------------------------------
*               |               |               |               |               |               |               |               |
*--------------------------------------------------------------------------------------------------------------------------------
*       |          |            |               |               |               |               |               |               |
*   seq     ch           cmd         data[0]          data[1]                                                       data[5]
   [7:4]   [3:0]         [5:0]
           +[7:6]
*/
/*global variable for tx packet*/
uint8_t g_h2d_tx_buf [H2D_PACKET_SIZE_IN_BYTES] = {0};

#pragma data_alignment = 32
uint8_t g_h2d_rx_buf [H2D_PACKET_SIZE_IN_BYTES] = {0};

uint8_t g_data_buf[1024] = {0};

//uint8_t g_data_buf_ready = 0;

/* Internal APIs*/

/*!
* \fn      H2D_packet create_tx_packet(H2D_Cmd_Info *h2d_cmd_info)
* \brief   Function to create h2d tx packet 
* \param   hh2d_cmd_info -- input as unpacked cmd packet
* \returns H2D_packet -- packed tx packet
*/
static void create_tx_packet(H2D_Cmd_Info *h2d_cmd_info){
  
    /* copy the 4 bits seq and 4 bits of channel number to byte 0*/
    g_h2d_tx_buf[0] = ((h2d_cmd_info->seq & H2D_SEQ_MASK) << H2D_SEQ_BIT_POS) |          \
                      ((h2d_cmd_info->channel & H2D_CHANNEL_MASK) >> H2D_CHANNEL_BIT_POS);
    
    /* copy the remaing 2 bits (lsb of channel num) of channel number and 6 bits cmd to byte 1*/
    g_h2d_tx_buf[1] = ((h2d_cmd_info->channel & H2D_CHANNEL_MASK) << (8-H2D_CHANNEL_BIT_POS)) |    \
                       (h2d_cmd_info->cmd & H2D_CMD_MASK);
    
    /*copy remaining data as it is*/
    memcpy( &(g_h2d_tx_buf[2]), &(h2d_cmd_info->data[0]),H2D_DATA_NUM_BYTES);
    
    return;
}

static void extract_rx_packet(H2D_Cmd_Info *h2d_cmd_info){
    
    h2d_cmd_info->seq = ((g_h2d_rx_buf[0] >> H2D_SEQ_BIT_POS) & H2D_SEQ_MASK);
    h2d_cmd_info->channel = ((g_h2d_rx_buf[0] & 0xF) << H2D_CHANNEL_BIT_POS) | \
                             (g_h2d_rx_buf[1] >> (8-H2D_CHANNEL_BIT_POS));
                             
    h2d_cmd_info->cmd = (g_h2d_rx_buf[1] & H2D_CMD_MASK );
    
    /*copy remaining data as it is*/
    memcpy(&(h2d_cmd_info->data[0]),&(g_h2d_rx_buf[2]),H2D_DATA_NUM_BYTES);
    return;
}

/*!
* \fn      void generate_interrupt_to_s3(void)
* \brief   function to generate interrupt to device (s3)
* \param   -
* \returns -
*/
 void generate_interrupt_to_device(void){

    uint8_t out_gpio = g_h2d_protocol_info.pfm_info.H2D_gpio;
    HAL_GPIO_Write(out_gpio, 1);
    #if H2D_LED_TEST == 1
    LedBlueOn();
    #endif
    
}

/*!
* \fn      void clear_interrupt_to_s3(void)
* \brief   function to clear interrupt to device (s3)
* \param   -
* \returns -
*/
 void clear_interrupt_to_device(void){
  
    uint8_t out_gpio = g_h2d_protocol_info.pfm_info.H2D_gpio;
    HAL_GPIO_Write(out_gpio, 0);
    #if H2D_LED_TEST == 1
    LedBlueOff();
    #endif
}

static inline uint8_t read_gpio_intr_val(uint8_t gpio_num)
{
    // This register will reflect the value of the IO regardless of the type/polarity 
    return ((INTR_CTRL->GPIO_INTR_RAW) & (1<< gpio_num) );
}

static inline uint8_t read_gpio_out_val(uint8_t gpio_num)
{
    // This register will reflect the value of the IO regardless of the type/polarity 
    return ((MISC_CTRL->IO_OUTPUT >> gpio_num ) & 1 );
}


/*!
* \fn      void generate_pulse_to_device(void)
* \brief   function to generate pulse to device (s3)
*          assumes that QL_INT is alread zero(low) when calling this api           
* \param   - 
* \returns -
*/
static void generate_pulse_to_device(void){
    
    /*generate intr to device (QL_INT high)*/
    generate_interrupt_to_device();
    
    int delay_in_ms = 1; // Add one ms delay
    vTaskDelay((delay_in_ms/portTICK_PERIOD_MS));
    
    /*clear intr to device (QL_INT low)*/
    clear_interrupt_to_device();

    return;
}


/*!
* \fn      int read_device_mem(uint32_t addr, uint8_t * buf, uint32_t len)
* \brief   function to read from device memory using QLSPI
* \param   - memory address, destination buffer, length of data to be read
* \returns - status of qlspi read operation
*/
static inline int read_device_mem(uint32_t addr, uint8_t * buf, uint32_t len)
{
    int ret = 0;
    ret = QLSPI_Read_S3_Mem(addr, buf, len);
    return ret;
}

/*!
* \fn      void get_data_buf(uint8_t * dest_buf, uint32_t len_bytes)
* \brief   function to get data from the data_buf
* \param   -  destination buffer, length of data to be copied
* \returns - 
*/
void get_data_buf(uint8_t * dest_buf, uint32_t len_bytes)
{
    if(NULL == dest_buf){
        return;
    }
    memcpy((void *)dest_buf, &g_data_buf[0], len_bytes);
    return;
}

/* Receive task handler */
void h2dRxTaskHandler(void *pParameter){
  
    BaseType_t qret;
    unsigned int h2dRxTaskStop = 0;
    H2D_Rx_Pkt h2drx_msg;
    
    while(!h2dRxTaskStop){
        //clear the Msg Q buffer 
        memset(&h2drx_msg, 0, sizeof(h2drx_msg));
        qret = xQueueReceive(H2DRx_MsgQ, &h2drx_msg, H2DRX_MSGQ_WAIT_TIME);
        configASSERT(qret == pdTRUE);
        
        switch(h2drx_msg.msg)
        {
        case H2DRX_MSG_INTR_RCVD:
        {
            // check QL_INT level
            uint8_t out_gpio,in_gpio, out_gpio_val;
            in_gpio = g_h2d_protocol_info.pfm_info.D2H_gpio;
            out_gpio = g_h2d_protocol_info.pfm_info.H2D_gpio;

            out_gpio_val = read_gpio_out_val(out_gpio);
#if DEBUG_H2D_PROTOCOL
            dbg_str_int("QL_INT value : ", out_gpio_val);
#endif
            if (uxSemaphoreGetCount(g_h2d_transmit_lock) == 0 || out_gpio_val){ // Host is transmitting so this should be an ACK from device
            
                // wait for intr to go low
                while (read_gpio_intr_val(in_gpio)) {
                    vTaskDelay(2); // Leave 2 ticks for other process to execute
                }
                clear_interrupt_to_device();
                // release the lock
                if (xSemaphoreGive(g_h2d_transmit_lock) != pdTRUE) {
                    dbg_fatal_error("[H2D Protocol] : Error : unable to release lock to g_h2d_transmit_lock\n");
                }
#if DEBUG_H2D_PROTOCOL
                dbg_str("[H2D_PROTOCOL] Transmit from host complete. Received pulse from device\n");
#endif
            }
            else{           // H2D is low -- device is writing to us
                /* This is an event from device.
                    read the device mem for rx buf over qlspi
                    extract the ch_num and invoke the callback
                    check if second read required for this event and fill data_buf
                */
                // read rx buf from device mem
                if (read_device_mem(H2D_READ_ADDR,(uint8_t *)&(g_h2d_rx_buf[0]), (H2D_PACKET_SIZE_IN_BYTES))) {   //SJ
                    dbg_str("device memory read failed\n");
                }
                else {
                    // extract info from rx buf and fill info pkt to be sent to callback
                    H2D_Cmd_Info h2d_cmd;
                    Rx_Cb_Ret cb_ret = {0};
                    extract_rx_packet(&h2d_cmd);
                    uint8_t  ch_num = h2d_cmd.channel;
                   
                    /*invoke the callback*/
                    if (g_h2d_protocol_info.cb_info[ch_num].rx_cb_ptr) {
                        cb_ret = g_h2d_protocol_info.cb_info[ch_num].rx_cb_ptr(h2d_cmd,cb_ret.data_read_req);
                    }
                    /* keep checking from callback ret value if second read is needed */
                    /* if yes, then read the data from addr and length passed by cb ret value*/
                    while(cb_ret.data_read_req == 1){
                        // need to read data buffer from device memory
                        if (read_device_mem(cb_ret.addr,(uint8_t *)&(g_data_buf[0]), cb_ret.len)) {
                            dbg_str("device memory read failed\n");
                        }
                        else{
#if DEBUG_H2D_PROTOCOL
                            dbg_str("[H2D_PROTOCOL] Read data for received event type.\n");
#endif
                            cb_ret = g_h2d_protocol_info.cb_info[ch_num].rx_cb_ptr(h2d_cmd,1);
                        }
                    }
                    generate_pulse_to_device();
#if DEBUG_H2D_PROTOCOL
                    dbg_str("[H2D_PROTOCOL] Received event from device. Pulse sent\n");
#endif
                }
            }
          
            break;
        }
        }
    }
  
    return;
}


/*!
* \fn      void send_msg_to_h2drx_task_fromISR(uint8_t msg_type)
* \brief   send msg to  H2DRx_MsgQ
* \param   msg id to be sent
* \returns -
*/
void send_msg_to_h2drx_task_fromISR(uint8_t msg_type) {
    H2D_Rx_Pkt h2d_msg;
    h2d_msg.msg = msg_type;
    
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if( xQueueSendFromISR( H2DRx_MsgQ, &(h2d_msg), &xHigherPriorityTaskWoken ) != pdPASS ) {
        dbg_fatal_error("[H2D Protocol] : Error : unable to send msg to H2DRx_MsgQ from ISR\n");
    }		
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    
    return;
}


/*!
* \fn      void service_intr_from_device(void)
* \brief   function to service interrupt received from the device (s3)
*          This function is called from ISR. should use only isr safe apis  
* \param   - 
* \returns -
*/
void service_intr_from_device(void){
    send_msg_to_h2drx_task_fromISR(H2DRX_MSG_INTR_RCVD);
    return;
}


/*!
* \fn      signed portBASE_TYPE start_rtos_task_h2drx( void)
* \brief   Setup msg queue and Task Handler for H2D rx Task 
* \param   - 
* \returns - portBASE_TYPE pdPASS on success
*/
static signed portBASE_TYPE start_rtos_task_h2drx( void) {
    static uint8_t ucParameterToPass;
 
    /* Create queue for h2d rx Task */
    H2DRx_MsgQ = xQueueCreate( H2DRX_QUEUE_LENGTH, sizeof(H2D_Rx_Pkt) );
    vQueueAddToRegistry( H2DRx_MsgQ, "H2DRx_Q" );
    configASSERT( H2DRx_MsgQ );
    
    /* Create H2D Rx Task */
    xTaskCreate ( h2dRxTaskHandler, "H2DRxTaskHandler", STACK_SIZE_ALLOC(STACK_SIZE_TASK_H2D_RX),  &ucParameterToPass, PRIORITY_TASK_H2D_RX, &xHandleTaskH2DRx);
    configASSERT( xHandleTaskH2DRx );
    
    return pdPASS;
}

/*!
* \fn      int h2d_transmit_cmd(H2D_Cmd_Info *h2d_cmd_info)
* \brief   api to transmit cmd to device 
* \param   hh2d_cmd_info -- input as unpacked cmd packet, addre where the cmd pckt is to be written
* \returns status of tx operation
*/
int h2d_transmit_cmd(H2D_Cmd_Info *h2d_cmd_info) {
    if( !g_h2d_protocol_info.init_done )
        return H2D_ERROR;
    
   create_tx_packet(h2d_cmd_info);
   // If D2H is high, that means device got in first so we should wait for that trasaction to complete
   while (read_gpio_intr_val(g_h2d_protocol_info.pfm_info.D2H_gpio)) {
        vTaskDelay(2); // Leave 2 ticks for other process to execute
   }

   if (xSemaphoreTake(g_h2d_transmit_lock, 0) != pdTRUE) {
        dbg_fatal_error("[H2D Protocol] : Error unable to take lock to g_h2d_transmit_lock\n");
        return H2D_ERROR;
   }
   // transmit over qlspi
   if( QLSPI_Write_S3_Mem(H2D_WRITE_ADDR, (uint8_t *)&(g_h2d_tx_buf[0]), H2D_PACKET_SIZE_IN_BYTES )) {
        dbg_str_int("Error in h2d transmit ", __LINE__);
        //release the lock and return error
        if (xSemaphoreGive(g_h2d_transmit_lock) != pdTRUE) {
            dbg_fatal_error("[H2D Protocol] : Error : unable to release lock to g_h2d_transmit_lock\n");
        }
        return H2D_ERROR;
   }
   // generate interrupt to device, QL_INT
   generate_interrupt_to_device();
   
   return H2D_STATUS_OK;
}

/*!
* \fn      int h2d_register_rx_callback(H2D_Callback rx_cb, uint8_t ch_num)
* \brief   function to register rx callback for channel
* \param   callback function, channel number
* \returns - status of register operation
*/
int h2d_register_rx_callback(H2D_Callback rx_cb, uint8_t ch_num) {
    int ret = H2D_STATUS_OK;
    
    if ((NULL==rx_cb) || (ch_num >= MAX_NUM_CHANNEL)) {
        dbg_str("Invalid paramter for h2d register callback\n");
        ret = H2D_ERROR;
    }
    if(g_h2d_protocol_info.cb_info[ch_num].rx_cb_ptr != NULL) {
        dbg_str_int("callback for channel already registered. ch_num = \n", ch_num);
        ret = H2D_ERROR;
    } else {
        g_h2d_protocol_info.cb_info[ch_num].rx_cb_ptr = rx_cb;
    }
    
    return ret;
}


/*platform configuration*/
void h2d_platform_init (H2D_Platform_Info * pfm_info) {   
    // Nothing to do as of now
    return;
}

/*!
* \fn      int h2d_protocol_init(H2D_Platform_Info * h2d_platform_info)
* \brief   function to initialize the h2d communication,
*          creates the transmit lock and h2drx task
* \param   platform info (input and output interrupt gpio)
* \returns - status of init operation
*/
int h2d_protocol_init(H2D_Platform_Info * h2d_platform_info) {
    if( g_h2d_protocol_info.init_done ) {
        dbg_str("h2d protocol already intialized.\n");
        return H2D_STATUS_OK;
    }
    
    memcpy( &(g_h2d_protocol_info.pfm_info), h2d_platform_info, sizeof(H2D_Platform_Info)); 
    
    uint8_t out_gpio = g_h2d_protocol_info.pfm_info.H2D_gpio;
    HAL_GPIO_Write(out_gpio, 0);            // write 0 to the QL_INT at init
    
    // Check to see if D2H is active, if so wait for it to go inactive
    if (read_gpio_intr_val(g_h2d_protocol_info.pfm_info.D2H_gpio)) {
        dbg_str("Waiting for D2H to go inactive\n");
        while (read_gpio_intr_val(g_h2d_protocol_info.pfm_info.D2H_gpio)) {
            vTaskDelay(1);
        }
        dbg_str("D2H inactive - resuming config\n");
    }
    
    //create tx lock
    if(g_h2d_transmit_lock == NULL) {
        g_h2d_transmit_lock = xSemaphoreCreateBinary();
        if( g_h2d_transmit_lock == NULL ) {
          dbg_str("[H2D Protocol] : Error : Unable to Create Mutex\n");
          return H2D_ERROR;
        }
        vQueueAddToRegistry(g_h2d_transmit_lock, "h2d_transmit_lock" );
        xSemaphoreGive(g_h2d_transmit_lock);
    }
    
    
    // create the rx task
    start_rtos_task_h2drx();
    
    g_h2d_protocol_info.init_done = 1;
    
    return H2D_STATUS_OK;
}

