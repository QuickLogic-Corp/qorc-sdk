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

/*! \file eoss3_hal_fpga_adc_test_app.c
*
*  \brief This file contains test application for testing FPGA ADC INTERFACE IP.
*/
#include "Fw_global_config.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "Fw_global_config.h"

#include "FreeRTOS.h"
#include "task.h"
#include "RtosTask.h"
#include "Recognition.h"
#include "dcl_commands.h"
#include "DataCollection.h"
#include "dcl_commands.h"
#include "eoss3_hal_fpga_adc_api.h"
#include "eoss3_hal_gpio.h"
#include "ql_time.h"
#include "eoss3_hal_pads.h"
#include "eoss3_hal_pad_config.h"
#include "ql_adcTask.h"
#include "dbg_uart.h"
//#include "timers.h"
#include "qlsh_commands.h"
#include "micro_tick64.h"
#include "DataCollection.h"

#define MY_MSG_SRC  (('A'+'D'+'C') & 0x0ff)
#define USE_FSDMA_DRIVER_V2     (1)
// Commands to configure/start/stop ADC
// These are *NOT* public, so they are in header file
// The public api is via function calls.
#define ADC_FPGA_TASK_DATAREADY       1
#define ADC_FPGA_TASK_START           2
#define ADC_FPGA_TASK_CONFIGURE       3


/* this is set/configured by:
 *  Bluetooth - via the DCL
 *  The recognition task when it switches modes.
 */
xTaskHandle xHandleTaskADC;
QueueHandle_t xHandleQueueADC;
HAL_ADC_FPGA_cfg_t adc_fpga_task_config;

int adc_data_ready_time;

struct adc_fpga_task_vars {
    
  /* non-zero if the task is collecting data */
  int is_running;

  /* calculated size of each dma transaction */
  int dma_size;
 
  /* true if DMA should stop on next transfer */
  int stop_request;

  /* ack from task that stop has occured. */
  int stop_ack;

  /* how many channels are active */
  int  n_active_channels;

  /* which buffer are we using for the DMA operation? */
  int  buf_id;

  /* the active configuration for the sensor
   * This comes from the global "ltc1859_task_confg" at start.
   */
  HAL_ADC_FPGA_cfg_t active_config;

  /* precomputed sensor data information */
  struct sensor_data sdi;
};

struct adc_fpga_task_vars adc_fpga_task_vars;

/* two dma buffers for adc samples
 * not static so they are visible in the debugger
 *
 * The RIFF block holds 0x1000 (4096) - 0x30 (48) header bytes
 * 
 */

#if (USE_FSDMA_DRIVER_V2==0)
#define DMA_BUF_SIZE (RIFF_DATA_BLOCK_PAYLOAD_SIZE / sizeof(uint16_t))
#else
#define DMA_BUF_SIZE (16)
#endif
// #define DMA_BUF_SIZE (2048)
uint16_t adc_fpga_buffer0[ DMA_BUF_SIZE ];
uint16_t adc_fpga_buffer1[ DMA_BUF_SIZE ];

int DUMP_DATA = 0;

static void dump_data(void)
{
    int x;
    
    if( !DUMP_DATA ){
        return;
    }
    
    printf("Timestart = %u\n", t_dma_start );
    for( x = 0 ; x < MAX_ADC_STAMPS ; x++ ){
        struct adc_timestamp_s *p;
        p = adc_timestamps+x;
        printf("%u,%u,%u,%u,%u,%u,%u,0x%08x,%d\n", x, 
               p->fsdma_enter - t_dma_start,
               p->time_dma_callback - t_dma_start, 
               p->task_data_ready - t_dma_start, 
               p->time_dma_before - t_dma_start,
               p->time_dma_after - t_dma_start,
               p->task_data_done - t_dma_start,
               p->overflow_value,
               (p->overflow_value >> 16) & 0x0ffff
               );
    }
    
    for( x = 0 ; x < adc_fpga_task_vars.dma_size/2 ; x++ ){
        printf("%d,%d,%d\n", x, adc_fpga_buffer0[x], adc_fpga_buffer1[x]);
    }
}

/* NULL if not running */
const uint16_t *ltc1859_watch_data;
const uint16_t *ad7476_watch_data;

/* return non-zero if task is running */
int  sensor_adc_fpga_is_running( void )
{
    return adc_fpga_task_vars.is_running;
}

/* send a message to the ADC task */
static void send_msg( int cmd )
{
    /* configuration has changed */
    struct xQ_Packet   msg;
    
    memset( (void *)(&msg),0, sizeof(msg) );
    msg.ucSrc = MY_MSG_SRC;
    msg.ucCommand = cmd;

    xQueueSend( xHandleQueueADC, &msg, 0 );
}

/* called by DCL command processor to disable and clear all sensor activity */
void sensor_adc_fpga_clear(void)
{
    /* disable */
    adc_fpga_task_config.sensor_id = 0;
    adc_fpga_task_config.ltc1859.channel_enable_bits = 0;
    adc_fpga_task_vars.active_config.sensor_id = 0;
    adc_fpga_task_vars.stop_request = 1;
    ltc1859_watch_data = NULL;
    ad7476_watch_data = NULL;

}

/* start/stop collecting data from the ADC */
void sensor_adc_fpga_startstop(int is_start)
{
    intptr_t token;
    if( is_start ){
        if( adc_fpga_task_config.sensor_id  == 0 ){
            dbg_str("adc-not-configured/enabled\n");
            return;
        }
        send_msg( ADC_FPGA_TASK_START );
        return;
    }
    
    ltc1859_watch_data = NULL;
    ad7476_watch_data = NULL;
    
    /* Goal is to stop... */
    if( !adc_fpga_task_vars.is_running ){
        /* we are not running we are done */
        return;
    }
    adc_fpga_task_vars.stop_request = 1;
    /* need a better "actuall stopped" solution.
     *
     * Problem Cannot call De_Init()
     * if there is an active DMA operation.
     * And there might be an active DMA operation.
     */
    
    token = ql_lw_timer_start();

    /* make sure the adc is actually stopping not hung */
    while( !adc_fpga_task_vars.stop_ack ){
        /* 1 second timeout */
        if( ql_lw_timer_is_expired( token, 1000 ) ){
            dbg_fatal_error( "adc-fpga-adc-stop-timeout\n");
        }
        vTaskDelay( 10 );
    }
               
    adc_fpga_task_vars.is_running = 0;
    HAL_ADC_FPGA_De_Init();
}

/* tell the task to configure sensor */
void sensor_adc_fpga_configure(void)
{
    send_msg( ADC_FPGA_TASK_CONFIGURE );
}

/* this occurs in the interrupt context, a DMA buffer has completed
 * what we are not allowed to do is call the HAL READ function
 * the HAL read function can *ONLY* be called from a TASK context
 *
 */
void adc_fpga_read_callback(void)
{
    //Send message for read complete.
    struct xQ_Packet packet;
    packet.ucSrc = MY_MSG_SRC;
    packet.ucCommand = ADC_FPGA_TASK_DATAREADY;
    
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if( xQueueSendFromISR( xHandleQueueADC, &packet, &xHigherPriorityTaskWoken ) != pdPASS )
        dbg_fatal_error("adc-callback-fail\n");
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


        
    

// process data from the IRQ.
static void data_ready(void)
{
    void *vp;
    uint64_t tnow;

    tnow = xTaskGet_uSecCount();
    
    /* short and simple this can be high speed */
    if( DBG_flags & DBG_FLAG_adc_task ){
        dbg_ch('A');
    }
    
    /* we have been asked to stop */
    if( !adc_fpga_task_vars.stop_request ){
        if( adc_fpga_task_vars.buf_id == 0 ){
            vp = &adc_fpga_buffer1[0];
        } else {
            vp = &adc_fpga_buffer0[0];
        }
		/* update watch pointer */
        switch( adc_fpga_task_vars.active_config.sensor_id ){
        case HAL_SENSOR_ID_AD7476:
            ad7476_watch_data = vp;
            break;
        case HAL_SENSOR_ID_LTC1859:
            ltc1859_watch_data = vp;
            break;
        }
        
		/* and start next read (we are in the task context now) */
 
        adc_timestamps[adc_data_ready_time].time_dma_before = DWT->CYCCNT;       
        HAL_ADC_FPGA_Read( vp, adc_fpga_task_vars.dma_size );
        adc_timestamps[adc_data_ready_time].time_dma_after = DWT->CYCCNT;       
    }
    
    /* get data pointer for just filled buffer */
    if( adc_fpga_task_vars.buf_id == 0 ){
        vp = &adc_fpga_buffer0[0];
    } else {
        vp = &adc_fpga_buffer1[0];
    }

    /* update the sensor data structure and timestamp */
    adc_fpga_task_vars.sdi.vpData = vp;
    adc_fpga_task_vars.sdi.time_end = tnow;

  
    ble_send( &adc_fpga_task_vars.sdi  );
#if S3AI_FIRMWARE_IS_COLLECTION
    //data_save( &adc_fpga_task_vars.sdi );
#endif
#if S3AI_FIRMWARE_IS_RECOGNITION
    recog_data( &adc_fpga_task_vars.sdi );
#endif
        /* next packet is when this one completed, ie: tnow */
    adc_fpga_task_vars.sdi.time_start = tnow;
        
	    /* toggle buffer id */
    adc_fpga_task_vars.buf_id = !adc_fpga_task_vars.buf_id;
        
	    /* are we done? Or did we start another transfer */
    if( adc_fpga_task_vars.stop_request ){
        ltc1859_watch_data = NULL;
        ad7476_watch_data = NULL;
        /* ACK we are done */
        adc_fpga_task_vars.stop_ack = 1;
    }
}

static void do_start(void)
{
    void *vp;
    uint64_t tnow;
    int tmp;
    int status;
    int bytes_per_second;
    int bytes_per_100msec;
    int readings_per_100msec;
    
    tnow = xTaskGet_uSecCount();
    
    adc_fpga_task_vars.stop_request = 0;
    adc_fpga_task_vars.stop_ack     = 0;
    if( adc_fpga_task_config.sensor_id == 0 ){
        adc_fpga_task_vars.stop_ack = 1;
        dbg_str("adc-not-configured\n");
        return;
    }
    /* make *OUR* copy of the configuration */
    adc_fpga_task_vars.active_config = adc_fpga_task_config;
    
    adc_fpga_task_vars.sdi.rate_hz = 
        adc_fpga_task_vars.active_config.frequency;
    adc_fpga_task_vars.sdi.sensor_id = adc_fpga_task_vars.active_config.sensor_id;
    /* goal is 10mSecs of data */
    /* how many channels are enabled? */
    adc_fpga_task_vars.n_active_channels = 0;
    if( adc_fpga_task_vars.active_config.sensor_id == HAL_SENSOR_ID_AD7476 ){
        adc_fpga_task_vars.n_active_channels = 1;
    } else {
        for( tmp = 0x80 ; tmp ; tmp = tmp / 2 ){
            if( adc_fpga_task_vars.active_config.ltc1859.channel_enable_bits & tmp ){
                adc_fpga_task_vars.n_active_channels += 1;
            }
        }
    }
    if( adc_fpga_task_vars.n_active_channels == 0 ){
        /* WE DO NOT START */
        return;
    }
    
    /* readings are done in semi-32bit chunks
    * We also require that the data saved to files
    * are semi-32bit aligned values.
    *
    * If we have an odd number of channels... (1,3,5,7)
    * To force alignment we double
    */
    tmp = adc_fpga_task_vars.n_active_channels;
    tmp = tmp * sizeof(uint16_t);
    adc_fpga_task_vars.sdi.bytes_per_reading = tmp;
    adc_fpga_task_vars.sdi.rate_hz = adc_fpga_task_vars.sdi.rate_hz;
    
    /* 
    * If we have an odd number of channels
    * Example 3 channels, we have 6 bytes of data per reading
    * Example 7 channels, we have 28 bytes of data per reading
    *
    * NOTE: Future proofing, we consider 8 channel configuration.
    *
    * We need 2 things:
    *  
    * 1) buffers must begin/end on 32bit boundaries.
    *    (HW dma requirement)
    * 2) buffers must hold complete readings.
    *    RIFF saving application requirement.
    */
    
    /* calculate bytes per second */
    tmp = adc_fpga_task_vars.sdi.bytes_per_reading;
    tmp = tmp * adc_fpga_task_vars.sdi.rate_hz;
    bytes_per_second = tmp;
    
    /* we can't cancel DMA operations, so we limit
    * DMA operations to 100mSecs at most 
    */
    bytes_per_100msec = bytes_per_second / 100;
    
    /* begin clipping dma transfer size */
    /* Never larger then the buffer */
    if( bytes_per_100msec > sizeof(adc_fpga_buffer0) ){
        bytes_per_100msec = sizeof(adc_fpga_buffer0);
    }
    
    /* some minimal size */
    if( bytes_per_100msec < 50 ){
        bytes_per_100msec = 50;
    }
    /* how many readings can we fit within that buffer */
    tmp = bytes_per_100msec / adc_fpga_task_vars.sdi.bytes_per_reading;
    /* if this is odd, make it even
    * This solves the problem of 3 channels discussed above
    *  we have alignment every 2 reading
    */
    tmp &= (~1);
    /* DMA has an issue - we must read 2 sets of readings not 1 */
    /* min value */
    if( tmp < 4 ){
        tmp = 4;
    }
    
    /* so we read this many per DMA transfer */
    readings_per_100msec = tmp;
   
    /* determine bytes per dma transfer */
    tmp = readings_per_100msec * adc_fpga_task_vars.sdi.bytes_per_reading;
    
    
    adc_fpga_task_vars.dma_size = tmp;
    dbg_str_int("adc-dma-size", (int)(tmp));
    adc_fpga_task_vars.sdi.n_bytes = tmp;
    
    /* pick a buffer to start with */
    adc_fpga_task_vars.buf_id = 0;

#if (USE_FSDMA_DRIVER_V2 == 1)
    extern void ad7476_isr_DmacDone(void);
    extern void ad7476_start_dma(void);
    status = HAL_ADC_FPGA_Init( &adc_fpga_task_config, &ad7476_isr_DmacDone);
    configASSERT( status == HAL_OK );
    ad7476_start_dma();
    dbg_str_int("ADC DMA Start", xTaskGetTickCount());
#else
    status = HAL_ADC_FPGA_Init( &adc_fpga_task_config, &adc_fpga_read_callback);
    configASSERT( status == HAL_OK );
    /* start reading */
    adc_fpga_task_vars.sdi.time_start = tnow;
    if( adc_fpga_task_vars.buf_id == 0 ){
        vp = &adc_fpga_buffer0[0];
    } else {
        vp = &adc_fpga_buffer1[0];
    }
    switch(adc_fpga_task_config.sensor_id){
    case HAL_SENSOR_ID_LTC1859:
        ltc1859_watch_data = vp;
        break;
    case HAL_SENSOR_ID_AD7476:
        ad7476_watch_data = vp;
        break;
    }
   
    t_dma_start = DWT->CYCCNT;;
    status = HAL_ADC_FPGA_Read( vp, adc_fpga_task_vars.dma_size );
    configASSERT( status == HAL_OK );
#endif
    adc_fpga_task_vars.is_running = 1;
}


/* this is the ADC_FPGA task handler */
static void adcTaskHandler(void *pParameter)
{
    struct xQ_Packet   rcvdMsg;
    BaseType_t         xResult;
    
    //wait_ffe_fpga_load();
    //wait_for_sensor_config();

    memset( (void *)(&rcvdMsg), 0, sizeof(rcvdMsg) );
    while(1)
    {
      /* Wait for messages in our queue */
        xResult = xQueueReceive( xHandleQueueADC, &(rcvdMsg), portMAX_DELAY);
        if(xResult != pdPASS ){
            continue;
        }
        
        if( rcvdMsg.ucSrc != MY_MSG_SRC ){
            continue;
        }

	/* Remember the start time of this message
	 * This is effectively the *END-TIME* of the current ADC buffer
	 * and the *START_TIME* of the next ADC buffer
	 */
        if( adc_data_ready_time == 45 ){
            dump_data();
        }
        
        switch(rcvdMsg.ucCommand)
        {
        case ADC_FPGA_TASK_DATAREADY:
	  /* DATA is ready we need to process it */
        // what time is it?
            if( adc_data_ready_time < MAX_ADC_STAMPS ){
                adc_timestamps[adc_data_ready_time].task_data_ready = DWT->CYCCNT;
            }
            data_ready();
            if( adc_data_ready_time < MAX_ADC_STAMPS ){
                adc_timestamps[adc_data_ready_time].task_data_done = DWT->CYCCNT;
                adc_data_ready_time++;
            }
            break;
        case ADC_FPGA_TASK_CONFIGURE:
            /* nothing to do, we do this in START instead */
            /* other drivers might do it in configure */
            break;
        case ADC_FPGA_TASK_START:
	  /* we have been commanded to start capturing ADC data */
            do_start();
            break;
                

        default:
            dbg_fatal_error("adc-unknown\n");
            break;
        }
    } //for loop
}
    
signed portBASE_TYPE StartRtosTaskADC(void)
{
    static unsigned int ucParameterToPass;
    
    xHandleQueueADC = xQueueCreate( 80, sizeof(struct xQ_Packet) );
    configASSERT(xHandleQueueADC);

    vQueueAddToRegistry( xHandleQueueADC, "Adc_Q" );
    /* Create Adc IF Test Task */
    xTaskCreate (adcTaskHandler, "adcTaskHandler", STACK_SIZE_ALLOC(256), &ucParameterToPass, PRIORITY_TASK_ADC, &xHandleTaskADC);
    configASSERT( xHandleTaskADC );
    return pdPASS;
}
