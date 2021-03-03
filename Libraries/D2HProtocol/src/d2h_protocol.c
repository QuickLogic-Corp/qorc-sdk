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
 *    File   : d2h_protocol.c
 *    Purpose: s3 device to host communication protocol implementaion
 *
 *=========================================================*/

#include "string.h"
#include "Fw_global_config.h"

#if (FEATURE_D2HPROTOCOL_DEVICE == 1)

#include "FreeRTOS.h"
#include "semphr.h"
#include "eoss3_dev.h"
#include "RtosTask.h"
#include "d2h_protocol.h"
#include "dbg_uart.h"
#include "eoss3_hal_gpio.h"
#include "eoss3_hal_pad_config.h"
#include "timers.h"

///////////// Debug Trace ////////////////////////////////
#include "dbgtrace.h"
#define             K_DBGTRACE_HIF  200
extern dbgtrace_t   adbgtraceHIF[K_DBGTRACE_HIF];
extern int          idbgtraceHIF;

typedef struct {
	D2H_Callback rx_cb_ptr;
    void *pchannel_info_rx;
    D2H_Tx_Done_Callback tx_done_cb_ptr;
    void *pchannel_info_tx;
} D2H_Callback_Info;

/* Structure to hold the h2d communication info*/
typedef struct D2H_Protocol_info {
#if (USE_4PIN_D2H_PROTOCOL == 1)
    D2H_Platform_Info pfm_info;                     // platform specific info
#endif
    D2H_Callback_Info cb_info[MAX_NUM_CHANNEL];     // callback for each channel
    uint8_t init_done;                              // flag to check if h2d init is done or not
}D2H_Protocol_info;


#define H2D_DATA_NUM_BYTES  (6)
#define H2D_SEQ_MASK        (0xF)
#define H2D_SEQ_BIT_POS     (4)
#define H2D_CHANNEL_MASK_0  (0xF)
#define H2D_CHANNEL_MASK_1  (0x3)
#define H2D_CHANNEL_MASK    (0x3F)
#define H2D_CHANNEL_BIT_POS (2)
#define H2D_CMD_MASK        (0x3F)


/* structure for msg to be sent to d2hrx task*/
typedef struct {
    uint8_t msg;
}D2H_Rx_Pkt;

#ifdef __GNUC__         //GCC

#if (USE_4PIN_D2H_PROTOCOL == 1)
uint8_t __attribute__((section (".BLE_SPI_TX_LOCATION"))) g_device_write_buff[D2H_SPI_TX_TOTAL_SIZE_BYTES];
#else
uint8_t __attribute__((section (".HOST_SPI_TX_LOCATION"))) g_device_write_buff[D2H_SPI_TX_TOTAL_SIZE_BYTES];
#endif
uint8_t __attribute__((section (".BLE_SPI_RX_LOCATION"))) g_device_read_buff[D2H_SPI_RX_TOTAL_SIZE_BYTES];

#else               // IAR

// These addresses are fixed for communication between BLE and S3 using QL SPI
#if (USE_4PIN_D2H_PROTOCOL == 1)
#pragma default_variable_attributes = @ "BLE_SPI_TX_LOCATION"   //starts at 0x7C400
#else
#pragma default_variable_attributes = @ "HOST_SPI_TX_LOCATION"  //starts at 0x7C000
#endif
uint8_t g_device_write_buff[D2H_SPI_TX_TOTAL_SIZE_BYTES]; //host reads from here, slave writes to this location
#pragma default_variable_attributes =

#pragma default_variable_attributes = @ "BLE_SPI_RX_LOCATION"
uint8_t g_device_read_buff[D2H_SPI_RX_TOTAL_SIZE_BYTES];////host writes here, slave reads from this location
#pragma default_variable_attributes =

#endif

/* Task and msg queue handle*/
xTaskHandle xHandleTaskD2HRx;
QueueHandle_t D2HRx_MsgQ;

/*Lock will be acquired when tx api is called and
released in ISR after device receives the data*/
SemaphoreHandle_t g_d2h_transmit_lock;
static int d2h_tx_cmd_sent = 0;

/*gloabal variable for d2h protocol info*/
D2H_Protocol_info g_d2h_protocol_info = {0};

/*global variable for tx packet*/
//H2D_packet g_h2d_tx_packet = {0};
uint8_t g_d2h_tx_buf [H2D_PACKET_SIZE_IN_BYTES] = {0};

int d2h_spurious_interrupts = 0;
int d2h_spurious_ack_interrupts = 0;
/*Internal APIs*/
static void clear_interrupt_to_host(void);

int d2h_error_count = 0;
extern void softreset_m4(void);
void check_d2h_errors(void)
{
    d2h_error_count++;
    if(d2h_error_count > 10) {
      //softreset_m4();
    }
    return;
}

#define MONITOR_D2H_TRANSMIT_TIME (1)
#if MONITOR_D2H_TRANSMIT_TIME == 1
static TimerHandle_t D2H_TimerHandle = NULL;
#define D2H_TRANSMIT_TIMEOUT_VALUE (200)              // 200ms

static int d2h_start_timeout_ms = 0;
int get_d2h_time_elapsed(void)
{
  return d2h_start_timeout_ms;
}
//Call back function, send a debug string
static void D2H_TimerCB(TimerHandle_t D2H_TimerHandle)
{
    d2h_start_timeout_ms += D2H_TRANSMIT_TIMEOUT_VALUE;
#if 0 //if no response for 2 secs go back to recognition mode
//extern void hif_msg_sendStopTx(void);    
    if( d2h_start_timeout_ms >= 2000) {
      clear_interrupt_to_host();
      d2h_start_timeout_ms = 0;
      xTimerStop(D2H_TimerHandle, pdMS_TO_TICKS(0));
      //hif_msg_sendStopTx(); //does not work
    }
#endif
    dbg_str_int_noln("D2H: No host reponse for", D2H_TRANSMIT_TIMEOUT_VALUE);
    dbg_str(" msecs");
    return;
}
#endif

/*!
* \fn      D2H_packet create_tx_packet(D2H_Pkt_Info *d2h_evt_info)
* \brief   Function to create d2h tx packet
* \param   d2h_evt_info -- input as unpacked evt packet
* \returns D2H_packet -- packed tx packet
*/
static void create_tx_packet(D2H_Pkt_Info *d2h_evt_info){

    /* copy the 4 bits seq and 4 bits of channel number to byte 0*/
    g_d2h_tx_buf[0] = ((d2h_evt_info->seq & H2D_SEQ_MASK) << H2D_SEQ_BIT_POS) |          \
                      ((d2h_evt_info->channel & H2D_CHANNEL_MASK) >> H2D_CHANNEL_BIT_POS);

    /* copy the remaing 2 bits (lsb of channel num) of channel number and 6 bits cmd to byte 1*/
    g_d2h_tx_buf[1] = ((d2h_evt_info->channel & H2D_CHANNEL_MASK) << (8-H2D_CHANNEL_BIT_POS)) |    \
                       (d2h_evt_info->cmd & H2D_CMD_MASK);

    /*copy remaining data as it is*/
    memcpy( &(g_d2h_tx_buf[2]), &(d2h_evt_info->data[0]),H2D_DATA_NUM_BYTES);

    return;
}

/*!
* \fn      D2H_packet extract_tx_packet_info(D2H_Pkt_Info *d2h_pkt_info)
* \brief   Function to extract d2h tx packet info from the fixed tx buffer location
* \param   d2h_pkt_info -- ouput as  cmd packet
* \returns --
*/
static void extract_tx_packet_info(D2H_Pkt_Info *d2h_pkt_info){

    d2h_pkt_info->seq = ((g_device_write_buff[0] >> H2D_SEQ_BIT_POS) & H2D_SEQ_MASK);
    d2h_pkt_info->channel = ((g_device_write_buff[0] & 0xF) << H2D_CHANNEL_BIT_POS) | \
                             (g_device_write_buff[1] >> (8-H2D_CHANNEL_BIT_POS));

    d2h_pkt_info->cmd = (g_device_write_buff[1] & H2D_CMD_MASK );

    /*copy remaining data as it is*/
    memcpy(&(d2h_pkt_info->data[0]),&(g_device_write_buff[2]),H2D_DATA_NUM_BYTES);

    return;
}


/*!
* \fn      D2H_packet extract_rx_packet(D2H_Pkt_Info *d2h_pkt_info)
* \brief   Function to extract d2h rx packet
* \param   d2h_pkt_info -- ouput as  cmd packet
* \returns --
*/
static void extract_rx_packet(D2H_Pkt_Info *d2h_pkt_info){

    d2h_pkt_info->seq = ((g_device_read_buff[0] >> H2D_SEQ_BIT_POS) & H2D_SEQ_MASK);
    d2h_pkt_info->channel = ((g_device_read_buff[0] & 0xF) << H2D_CHANNEL_BIT_POS) | \
                             (g_device_read_buff[1] >> (8-H2D_CHANNEL_BIT_POS));

    d2h_pkt_info->cmd = (g_device_read_buff[1] & H2D_CMD_MASK );

    /*copy remaining data as it is*/
    memcpy(&(d2h_pkt_info->data[0]),&(g_device_read_buff[2]),H2D_DATA_NUM_BYTES);

    return;
}


/*IMP : this api is temporary. Intr/Pad configuration should eventually move to app specific hardware setup.*/
/*!
* \fn      void h2d_config_intr(void)
* \brief   function to generate interrupt to device (s3)
* \param   -
* \returns -
*/
void h2d_config_intr(void *pv){

    //AP_INT configuration (Device to Host)
    /* SW_INTR2 -> outgoing interrupt to AP from S3 */

    /* clear interrupts */
	INTR_CTRL->SOFTWARE_INTR_2 = 0;

    /* enable SWINTR2 interrupt configuration as interrupt to AP */
	INTR_CTRL->SOFTWARE_INTR_2_EN_AP = 1;
	INTR_CTRL->SOFTWARE_INTR_2_EN_M4 = 0;

	/* enable interrupt to AP at NVIC */
	NVIC_ClearPendingIRQ(SwInt2_IRQn);
	NVIC_EnableIRQ(SwInt2_IRQn);

    /* Configure PAD43 for interrupt to AP */
	MISC_CTRL->LOCK_KEY_CTRL = 0x1ACCE551;
	IO_MUX->PAD_43_CTRL = PAD43_FUNC_SEL_AP_INTR | PAD_E_12MA | PAD_OEN_NORMAL;
	MISC_CTRL->LOCK_KEY_CTRL = 0x00000000;

    return;
}

/*!
* \fn      void generate_interrupt_to_s3(void)
* \brief   function to generate interrupt to device (s3)
* \param   -
* \returns -
*/
static void generate_interrupt_to_host(void){
    uint32_t start_ticks = xTaskGetTickCount() + 500;
    // PAD43 as AP_INT. generate soft intr2
    volatile uint32_t sts = 0;
	do
	{
		sts = INTR_CTRL->SOFTWARE_INTR_2;
        if(sts) {
          //clear the interrupt since this is called only after ACK is received
          INTR_CTRL->SOFTWARE_INTR_2 = 0;
          vTaskDelay(1);
        
        uint32_t current_ticks = xTaskGetTickCount();
        if(current_ticks > start_ticks)
        {
            start_ticks = current_ticks;
            dbg_str("ERROR - Host interrupt is not cleared for 500 sec");
            //configASSERT(0);
          }
        }
	}
	while(sts);

	INTR_CTRL->SOFTWARE_INTR_2 = 1;
    
#if D2H_LED_TEST == 1
    LedBlueOn();
#endif

  return;
}

static inline uint8_t read_gpio_intr_val(uint8_t gpio_num)
{
    // This register will reflect the value of the IO regardless of the type/polarity
    return ((INTR_CTRL->GPIO_INTR_RAW) & (1<< gpio_num) );
}

/*!
* \fn      void clear_interrupt_to_host(void)
* \brief   function to clear interrupt to host
* \param   -
* \returns -
*/
static void clear_interrupt_to_host(void){

    // PAD43 .. clear soft intr2
    volatile uint32_t sts = 1;
    uint32_t start_ticks = xTaskGetTickCount() + 200;
	INTR_CTRL->SOFTWARE_INTR_2 = 0;

	do{
		sts = INTR_CTRL->SOFTWARE_INTR_2;
        if(sts) {
          INTR_CTRL->SOFTWARE_INTR_2 = 0;
          vTaskDelay(1);
          uint32_t current_ticks = xTaskGetTickCount();
          if(current_ticks > start_ticks)
          {
            dbg_str("ERROR - Host interrupt (2) is not cleared for 200 sec");
            //configASSERT(0);
            break; //since this called when ACK is received
          }
        }
	}while(sts);

	do{
		sts++;
	}while (sts < 50);
    
    #if D2H_LED_TEST == 1
    LedBlueOff();
    #endif
    return;
}

#if (USE_4PIN_D2H_PROTOCOL == 1)
/*!
* \fn      void generate_pulse_to_host(void)
* \brief   function to generate pulse to host (s3)
*          assumes that AP_INT is alread zero(low) when calling this api
* \param   -
* \returns -
*/
static void generate_pulse_to_host(void){
uint8_t d2h_ack = g_d2h_protocol_info.pfm_info.D2H_ack;

    /* set the ack intr to host */
    HAL_GPIO_Write(d2h_ack, 1);

    int delay_in_ms = 1; // Add one ms delay
    vTaskDelay((delay_in_ms/portTICK_PERIOD_MS));

    /*clear ack intr to host */
    HAL_GPIO_Write(d2h_ack, 0);

    return;
}
#endif

/*!
* \fn      void send_msg_to_d2hrx_task(uint8_t msg_type)
* \brief   send msg to  D2HRx_MsgQ
* \param   msg id to be sent
* \returns -
*/
void send_msg_to_d2hrx_task(uint8_t msg_type) {
    D2H_Rx_Pkt d2h_msg;
    d2h_msg.msg = msg_type;

	if( xQueueSend( D2HRx_MsgQ, &(d2h_msg), 5 ) != pdPASS ) {
        dbg_fatal_error("[D2H Protocol] : Error : unable to send msg to D2HRx_MsgQ from Task\n");
    }
    return;
}

/*!
* \fn      void send_msg_to_d2hrx_task_fromISR(uint8_t msg_type)
* \brief   send msg to  D2HRx_MsgQ
* \param   msg id to be sent
* \returns -
*/
void send_msg_to_d2hrx_task_fromISR(uint8_t msg_type) {
    D2H_Rx_Pkt d2h_msg;
    d2h_msg.msg = msg_type;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if( xQueueSendFromISR( D2HRx_MsgQ, &(d2h_msg), &xHigherPriorityTaskWoken ) != pdPASS ) {
        dbg_fatal_error("[D2H Protocol] : Error : unable to send msg to D2HRx_MsgQ from ISR\n");
    }
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

    return;
}


/*!
* \fn      void service_intr_from_host(void)
* \brief   function to service interrupt received from the host (s3)
*          This function is called from ISR. should use only isr safe apis
* \param   -
* \returns -
*/
void service_intr_from_host(void){

    send_msg_to_d2hrx_task_fromISR(D2HRX_MSG_INTR_RCVD);
    return;
}

/*!
* \fn      void service_ack_from_host(void)
* \brief   function to service ack interrupt received from the host 
*          This function is called from ISR. should use only isr safe apis
* \param   -
* \returns -
*/
void service_ack_from_host(void){

    send_msg_to_d2hrx_task_fromISR(D2HRX_MSG_ACK_RCVD);
    return;
}

/* to hold the value of previous cmd sequence received from host*/
static int g_prev_seq = -1;
/* to hold the value of previous cmd sequence transmitted to host*/
static int g_prev_tx_seq = -1;


#if (USE_4PIN_D2H_PROTOCOL == 1)
/* Receive task handler */
void d2hRxTaskHandler(void *pParameter){

    BaseType_t qret;
    unsigned int d2hRxTaskStop = 0;
    D2H_Rx_Pkt d2hrx_msg;

    while(!d2hRxTaskStop){
        //clear the Msg Q buffer
        memset(&d2hrx_msg, 0, sizeof(d2hrx_msg));
        qret = xQueueReceive(D2HRx_MsgQ, &d2hrx_msg, D2HRX_MSGQ_WAIT_TIME);
        configASSERT(qret == pdTRUE);

        D2H_Pkt_Info d2h_rx_pkt = {0};
        D2H_Pkt_Info d2h_tx_pkt = {0};

//uint8_t h2dack = read_gpio_intr_val(g_d2h_protocol_info.pfm_info.H2D_ack);
//printf("\n[D2H %d %d]", d2hrx_msg.msg, h2dack);

        switch(d2hrx_msg.msg)
        {
        case D2HRX_MSG_INTR_RCVD:
            // D2H interrupt (AP_INT) means the Host initiated 
            extract_rx_packet(&d2h_rx_pkt);
            if (d2h_rx_pkt.seq == g_prev_seq) { // Trouble: host did not update rx buffer so assume this is a spurious interrupt and ignore
                d2h_spurious_interrupts++; //count the spurious interrupts
                #if 1 //DEBUG_D2H_PROTOCOL
                dbg_str_int("[D2H_PROTOCOL] unexpected host interrupt - ignoring ", d2h_spurious_interrupts);
                #endif
            } else {
                g_prev_seq = d2h_rx_pkt.seq;   // Update last completed sequence number
                /*Invoke callback*/
                if(g_d2h_protocol_info.cb_info[d2h_rx_pkt.channel].rx_cb_ptr){
                    void * pchannel_info =  g_d2h_protocol_info.cb_info[d2h_rx_pkt.channel].pchannel_info_rx;
                    g_d2h_protocol_info.cb_info[d2h_rx_pkt.channel].rx_cb_ptr(pchannel_info,d2h_rx_pkt);
                }
                // Send acknowledge pulse to host
                generate_pulse_to_host();
                
                #if DEBUG_D2H_PROTOCOL
                dbg_str("[D2H_PROTOCOL] received cmd from host. Pulse sent to host\n");
                #endif
            }
            break;
        case D2HRX_MSG_ACK_RCVD:
          {
            //Ack from Host

            //Extract tx packet info from the tx buffer and send to the tx_done_callback
            extract_tx_packet_info(&d2h_tx_pkt);
            if (d2h_tx_pkt.seq != g_prev_tx_seq) {
              d2h_spurious_ack_interrupts++;
              dbg_str("[D2H_PROTOCOL] Tx sequence does not match ACK sequence\n");
            }
            if(g_d2h_protocol_info.cb_info[d2h_tx_pkt.channel].tx_done_cb_ptr){
                void * pchannel_info =  g_d2h_protocol_info.cb_info[d2h_tx_pkt.channel].pchannel_info_tx;
                g_d2h_protocol_info.cb_info[d2h_tx_pkt.channel].tx_done_cb_ptr(pchannel_info, d2h_tx_pkt);
            }
            
            uint32_t start_ticks = xTaskGetTickCount();
            uint32_t current_ticks = start_ticks;
            //wait for H2D_ack to go low, which means host has completed the ACK
            while (read_gpio_intr_val(g_d2h_protocol_info.pfm_info.H2D_ack)) {
             vTaskDelay(1); 
             uint32_t elapsed_ticks = xTaskGetTickCount() - current_ticks; 
             //print msg every 200ms
             if(elapsed_ticks > 200)
             {
                current_ticks = xTaskGetTickCount();
                dbg_str_int_noln("D2H: Host Ack not down for (ms) ",current_ticks - start_ticks);
             }
            }
            clear_interrupt_to_host();  // pull AP_INT low (tell host we heard the ACK)

            //clear tx pkt header 2 bytes
            g_device_write_buff[0] = 0;
            g_device_write_buff[1] = 0;
            
            //release the tx lock only if it is in response to a transmit
            //Note: checking only g_prev_tx_seq is not enough since it may be 0
            //But checking only d2h_tx_cmd_sent should be good enough
            if ((d2h_tx_cmd_sent == 1) && (d2h_tx_pkt.seq == g_prev_tx_seq))
            {
              d2h_tx_cmd_sent = 0;
            //release the tx lock
            if (xSemaphoreGive(g_d2h_transmit_lock) != pdTRUE) {        // release the tx lock
                  dbg_fatal_error("[D2H Protocol] : Error : unable to release lock to g_d2h_transmit_lock\n");
              }
            } else {
              dbg_str("[D2H_PROTOCOL] unexpected host ACK interrupt - ignoring\n");
            }

            #if DEBUG_D2H_PROTOCOL
            dbg_str("[D2H_PROTOCOL] Device transmit complete. received pulse from host\n");
            #endif

            #if MONITOR_D2H_TRANSMIT_TIME
               xTimerStop(D2H_TimerHandle, pdMS_TO_TICKS(0));
            #endif
          }
            break;

        default:   
            dbg_str("[D2H_PROTOCOL] Error unknown msg \n");
            break;

        } //end of switch
//h2dack = read_gpio_intr_val(g_d2h_protocol_info.pfm_info.H2D_ack);        
//printf("\n[D2H done %d]", h2dack);
    } //end of while(1)

    return;
}
#else //use only 1-wire for the D2H protocol
static char d2h_sprintf_buf[60];//Assumption:it sever exceeds 60 bytes
/* Receive task handler */
void d2hRxTaskHandler(void *pParameter){

    BaseType_t qret;
    unsigned int d2hRxTaskStop = 0;
    D2H_Rx_Pkt d2hrx_msg;

    while(!d2hRxTaskStop){
        //clear the Msg Q buffer
        memset(&d2hrx_msg, 0, sizeof(d2hrx_msg));
        qret = xQueueReceive(D2HRx_MsgQ, &d2hrx_msg, D2HRX_MSGQ_WAIT_TIME);
        configASSERT(qret == pdTRUE);

        D2H_Pkt_Info d2h_rx_pkt = {0};
        D2H_Pkt_Info d2h_tx_pkt = {0};
        
        switch(d2hrx_msg.msg)
        {
        case D2HRX_MSG_INTR_RCVD:
          // D2H interrupt (AP_INT) means the Host initiated 
          extract_rx_packet(&d2h_rx_pkt);
          if (d2h_rx_pkt.seq == g_prev_seq) { // Trouble: host did not update rx buffer so assume this is a spurious interrupt and ignore
              d2h_spurious_interrupts++; //count the spurious interrupts
              #if 1 //DEBUG_D2H_PROTOCOL
              dbg_str("[D2H_PROTOCOL] unexpected host interrupt - ignoring\n");
              #endif
          } else {
              g_prev_seq = d2h_rx_pkt.seq;   // Update last completed sequence number
              /*Invoke callback*/
              if(g_d2h_protocol_info.cb_info[d2h_rx_pkt.channel].rx_cb_ptr){
                  void * pchannel_info =  g_d2h_protocol_info.cb_info[d2h_rx_pkt.channel].pchannel_info_rx;
                  g_d2h_protocol_info.cb_info[d2h_rx_pkt.channel].rx_cb_ptr(pchannel_info,d2h_rx_pkt);
              }
          }
          break;
        case D2HRX_MSG_WAIT_FOR_ACK:
          {
//          dbg_str("[.");
          int count =0;
          //host needs to clear the SW Int2 after reading the pkt 
          while(INTR_CTRL->SOFTWARE_INTR_2 == 1){
            vTaskDelay(2);
            count += 2;
          }
//          int n_count = sprintf(d2h_sprintf_buf,"%d.]",count); 
//          dbg_str(d2h_sprintf_buf);

          //Extract tx packet info from the tx buffer and send to the tx_done_callback
          extract_tx_packet_info(&d2h_tx_pkt);
          if(g_d2h_protocol_info.cb_info[d2h_tx_pkt.channel].tx_done_cb_ptr){
              void * pchannel_info =  g_d2h_protocol_info.cb_info[d2h_tx_pkt.channel].pchannel_info_tx;
              g_d2h_protocol_info.cb_info[d2h_tx_pkt.channel].tx_done_cb_ptr(pchannel_info, d2h_tx_pkt);
          }

          //release the tx lock
          if (xSemaphoreGive(g_d2h_transmit_lock) != pdTRUE) {        // release the tx lock
              dbg_str("[D2H Protocol] : Error : unable to release lock to g_d2h_transmit_lock\n");
              configASSERT(0);
          }
          
#if MONITOR_D2H_TRANSMIT_TIME
          xTimerStop(D2H_TimerHandle, pdMS_TO_TICKS(0));
#endif
          }
          break;          
        default:   
            dbg_str("[D2H_PROTOCOL] Error unknown msg \n");
            break;

        } //end of switch
    } //end of while(1)

    return;
}
#endif  // USE_4PIN_D2H_PROTOCOL 
/*!
* \fn      signed portBASE_TYPE start_rtos_task_d2hrx( void)
* \brief   Setup msg queue and Task Handler for D2H rx Task
* \param   -
* \returns - portBASE_TYPE pdPASS on success
*/
static signed portBASE_TYPE start_rtos_task_d2hrx( void) {
    static uint8_t ucParameterToPass;

    /* Create queue for d2h rx Task */
    D2HRx_MsgQ = xQueueCreate( D2HRX_QUEUE_LENGTH, sizeof(D2H_Rx_Pkt) );
    vQueueAddToRegistry( D2HRx_MsgQ, "D2HRx_Q" );
    configASSERT( D2HRx_MsgQ );

    /* Create D2H Rx Task */
    xTaskCreate ( d2hRxTaskHandler, "D2HRxTaskHandler", STACK_SIZE_ALLOC(STACK_SIZE_TASK_D2H_RX),  &ucParameterToPass, PRIORITY_TASK_D2H_RX, &xHandleTaskD2HRx);
    configASSERT( xHandleTaskD2HRx );

#if MONITOR_D2H_TRANSMIT_TIME == 1
    /* Create D2H Timer */
    D2H_TimerHandle = xTimerCreate("D2H_Timer", pdMS_TO_TICKS(D2H_TRANSMIT_TIMEOUT_VALUE), pdTRUE, (void*)0, D2H_TimerCB);
    configASSERT(D2H_TimerHandle != NULL);
#endif

    return pdPASS;
}

/*!
* \fn      int d2h_protocol_init(D2H_Platform_Info * d2h_platform_info)
* \brief   function to initialize the d2h communication,
*          creates the transmit lock and d2hrx task
* \param   platform info (input and output interrupt gpio)
* \returns - status of init operation
*/
int d2h_protocol_init(D2H_Platform_Info * d2h_platform_info) {
    if( g_d2h_protocol_info.init_done ) {
        dbg_str("d2h protocol already intialized.\n");
        return D2H_STATUS_OK;
    }

    // configure interrupt  (this call should eventually be changed to app specific hardware setuphardware)

#if (USE_4PIN_D2H_PROTOCOL == 1)    
    
    memcpy( &(g_d2h_protocol_info.pfm_info), d2h_platform_info, sizeof(D2H_Platform_Info));

    //clear the D2H Ack and D2H interrutps
    uint8_t d2h_ack = g_d2h_protocol_info.pfm_info.D2H_ack;
    HAL_GPIO_Write(d2h_ack, 0);

    // Check to see if H2D is inactive.  If it is active, print message and wait for it to go inactive
    if (read_gpio_intr_val(g_d2h_protocol_info.pfm_info.H2D_gpio)) {
        dbg_str("Waiting for H2D to go inactive\n");
        do {
            //this delay should not be there before the task started
            //vTaskDelay(1);
            dbg_str("Waiting for H2D to go inactive\n");
        }while (read_gpio_intr_val(g_d2h_protocol_info.pfm_info.H2D_gpio));
        dbg_str("H2D inactive - resuming config\n");
    }
#endif

    // clear the AP_INT
    clear_interrupt_to_host();
    
    //create tx lock
    if(g_d2h_transmit_lock == NULL) {
        g_d2h_transmit_lock = xSemaphoreCreateBinary();
        if( g_d2h_transmit_lock == NULL ) {
          dbg_str("[D2H Protocol] : Error : Unable to Create Mutex\n");
          return D2H_ERROR;
        }
        vQueueAddToRegistry(g_d2h_transmit_lock, "D2H_TX_Lock" );
        xSemaphoreGive(g_d2h_transmit_lock);
    }


    // create the rx task
    start_rtos_task_d2hrx();

    g_d2h_protocol_info.init_done = 1;
    d2h_spurious_interrupts = 0;

    return D2H_STATUS_OK;
}

#if (USE_4PIN_D2H_PROTOCOL == 1)
/*!
* \fn      int h2d_transmit_cmd(H2D_Cmd_Info *h2d_cmd_info)
* \brief   api to transmit cmd to device
* \param   hh2d_cmd_info -- input as unpacked cmd packet, addre where the cmd pckt is to be written
* \returns status of tx operation
*/
int d2h_transmit_cmd(D2H_Pkt_Info *d2h_evt_info) {
    if( !g_d2h_protocol_info.init_done )
        return D2H_ERROR;
    uint8_t in_gpio = g_d2h_protocol_info.pfm_info.H2D_gpio;

    dbgtrace(__LINE__, d2h_evt_info->cmd, adbgtraceHIF, K_DBGTRACE_HIF, &idbgtraceHIF);
   //take the lock to prevent another tx before ack 
   if (xSemaphoreTake(g_d2h_transmit_lock, 2000) != pdTRUE) {
        dbg_fatal_error("[D2H Protocol] : Error unable to take lock to g_d2h_transmit_lock\n");
        return D2H_ERROR;
   }
   
   //This is redundant in normal conditions, but error conditions like 
   //extra/spurous ACKs this is helpful
   d2h_tx_cmd_sent = 1;
   g_prev_tx_seq = d2h_evt_info->seq;
   
   // create tx packet
   create_tx_packet(d2h_evt_info);

   uint32_t start_ticks = xTaskGetTickCount();
   uint32_t current_ticks = start_ticks;

   // wait till intr from host is low (QL_INT is low)
   do {
        // wait for intr to go low
       vTaskDelay(1);
       uint32_t elapsed_ticks = xTaskGetTickCount() - current_ticks;
       //print msg every 200ms
       if(elapsed_ticks > 200)
       {
         current_ticks = xTaskGetTickCount();
         dbg_str_int_noln("D2H: Host Interrupt not down for (ms) ",current_ticks - start_ticks);
       }
   }while (read_gpio_intr_val(in_gpio));

   // write data to the shared memory location
   memcpy((void *)&(g_device_write_buff[0]), (void *)&(g_d2h_tx_buf[0]), H2D_PACKET_SIZE_IN_BYTES);

   // generate interrupt to device, QL_INT
   generate_interrupt_to_host();

#if DEBUG_D2H_PROTOCOL
   dbg_str("[D2H_PROTOCOL] Transmit from device. Host interrupted\n");
#endif

#if MONITOR_D2H_TRANSMIT_TIME
   d2h_start_timeout_ms = 0;
   xTimerStart(D2H_TimerHandle, pdMS_TO_TICKS(0));
#endif

   return D2H_STATUS_OK;
}
#else
/*!
* \fn      int h2d_transmit_cmd(H2D_Cmd_Info *h2d_cmd_info)
* \brief   api to transmit cmd to device
* \param   h2d_cmd_info -- input as unpacked cmd packet, addre where the cmd pckt is to be written
* \returns status of tx operation
*/
int d2h_transmit_cmd(D2H_Pkt_Info *d2h_evt_info) {
  if( !g_d2h_protocol_info.init_done )
      return D2H_ERROR;
  //uint8_t in_gpio = g_d2h_protocol_info.pfm_info.H2D_gpio;

  dbgtrace(__LINE__, d2h_evt_info->cmd, adbgtraceHIF, K_DBGTRACE_HIF, &idbgtraceHIF);
  //take the lock to prevent another tx before ack 
  if (xSemaphoreTake(g_d2h_transmit_lock, 200) != pdTRUE) {
      dbg_fatal_error("[D2H Protocol] : Error unable to take lock to g_d2h_transmit_lock\n");
      return D2H_ERROR;
  }

  //create tx packet
  create_tx_packet(d2h_evt_info);

  //write data to the shared memory location
  memcpy((void *)&g_device_write_buff[0], (void *)&g_d2h_tx_buf[0], H2D_PACKET_SIZE_IN_BYTES);

  //if there is a buffer pointer, copy that after the PKT header
  int byte_count = (d2h_evt_info->data[1] << 8) | d2h_evt_info->data[0];
  uint32_t byte_address = (d2h_evt_info->data[5] << 24) |
                          (d2h_evt_info->data[4] << 16) |
                          (d2h_evt_info->data[3] << 8) |
                          (d2h_evt_info->data[2] << 0);
    
  if((byte_count > 0) && (byte_address != 0)) {
    configASSERT(byte_count <= D2H_SPI_TX_BUF_SIZE_BYTES);
    memcpy((void *)&g_device_write_buff[H2D_PACKET_SIZE_IN_BYTES], (void *)byte_address, byte_count);
  }
  // generate interrupt to device, QL_INT
  generate_interrupt_to_host();
  
  //need to wait for host to clear the interrupt
  send_msg_to_d2hrx_task(D2HRX_MSG_WAIT_FOR_ACK);

#if DEBUG_D2H_PROTOCOL
  dbg_str("[D2H_PROTOCOL] Transmit from device. Host interrupted\n");
#endif

#if MONITOR_D2H_TRANSMIT_TIME
  d2h_start_timeout_ms = 0;
  xTimerStart(D2H_TimerHandle, pdMS_TO_TICKS(0));
#endif

  return D2H_STATUS_OK;
}

/*!
* \fn      configure_host_intrrupt(void)
* \brief   This function resets software interrupt registers
* \param   None
* \returns None
*/
void configure_host_interrupt(void)
{
	MISC_CTRL->SW_MB_1 = 0;
	MISC_CTRL->SW_MB_2 = 0;

	//Clear the interrupt and enable
	INTR_CTRL->SOFTWARE_INTR_1 = 0;
	INTR_CTRL->SOFTWARE_INTR_1_EN_M4 = 1; // Enable interrupt 1 from AP
   	INTR_CTRL->SOFTWARE_INTR_1_EN_AP = 0; // Disable interrupt 1 to AP

    //enable IRQ
	NVIC_ClearPendingIRQ(SwInt1_IRQn);
	NVIC_EnableIRQ(SwInt1_IRQn);
    
    return;
}

#endif //USE_4PIN_D2H_PROTOCOL

/*!
* \fn      int d2h_register_callback(H2D_Callback rx_cb, uint8_t ch_num, void *pchannel_info,D2H_Tx_Done_Callback tx_done_cb,void * pchannel_info_tx)
* \brief   function to register callback for channel
* \param   callback function, channel number, pchannel_info_rx, tx_done_cb, pchannel_info_tx
* \returns - status of register operation
*/
int d2h_register_callback(D2H_Callback rx_cb, uint8_t ch_num, void* pchannel_info_rx,D2H_Tx_Done_Callback tx_done_cb, void* pchannel_info_tx) {
    int ret = D2H_STATUS_OK;

    if ((NULL==rx_cb) || (ch_num >= MAX_NUM_CHANNEL)) {
        dbg_str("Invalid paramter for h2d register callback\n");
        ret = D2H_ERROR;
    }
    if(g_d2h_protocol_info.cb_info[ch_num].rx_cb_ptr != NULL) {
        dbg_str_int("callback for channel already registered. ch_num = \n", ch_num);
        ret = D2H_ERROR;
    }
    if(g_d2h_protocol_info.cb_info[ch_num].tx_done_cb_ptr != NULL) {
        dbg_str_int("tx done callback for channel already registered. ch_num = \n", ch_num);
        ret = D2H_ERROR;
    }
    else {
        g_d2h_protocol_info.cb_info[ch_num].rx_cb_ptr = rx_cb;
        g_d2h_protocol_info.cb_info[ch_num].pchannel_info_rx = pchannel_info_rx;
        // No need to check if tx callback is NULL or not.
        //This callback can be set to Null if user doesn't require tx done info
        g_d2h_protocol_info.cb_info[ch_num].tx_done_cb_ptr = tx_done_cb;
        g_d2h_protocol_info.cb_info[ch_num].pchannel_info_tx = pchannel_info_tx;
    }

    return ret;
}


/*!
* \fn      int h2d_transmit_lock_acquire(void)
* \brief   function to acquire tranmit lock from outside the h2d layer 
*           This API should be called in Task context only
* \param   -
* \returns - status of semaphore take operation 0-> OK
*/

int h2d_transmit_lock_acquire(void)
{
    int ret = D2H_STATUS_OK;
    if (xSemaphoreTake(g_d2h_transmit_lock, 2000) != pdTRUE) {
        dbg_fatal_error("[D2H Protocol] : Error (2) unable to take lock to g_d2h_transmit_lock\n");
        ret = D2H_ERROR;
   }
   return ret;
}

/*!
* \fn      int h2d_transmit_lock_release(void)
* \brief   function to release tranmit lock from outside the h2d layer 
*           This API should be called in Task context only
* \param   -
* \returns - status of semaphore give operation 0-> OK
*/

int h2d_transmit_lock_release(void)
{
    int ret = D2H_STATUS_OK;
    // release the lock
    if (xSemaphoreGive(g_d2h_transmit_lock) != pdTRUE) {
        dbg_fatal_error("[D2H Protocol] : Error (2) unable to release lock to g_d2h_transmit_lock\n");
        ret = D2H_ERROR;
        }
    return ret;
}

#endif /* FEATURE_D2HPROTOCOL_DEVICE */
