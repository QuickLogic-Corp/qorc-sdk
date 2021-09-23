/** @file sensor_ssss_process.c */

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

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"

#include "datablk_mgr.h"
#include "process_ids.h"
#include "datablk_processor.h"
#include "sensor_ssss.h"
#include "micro_tick64.h"
#include "dbg_uart.h"
#include "ssi_comms.h"
#include "eoss3_hal_i2c.h"

#include "mc3635_wire.h"
#include "sml_recognition_run.h"
#include "DataCollection.h"

// When enabled, GPIO (configured in pincfg_table.c) is toggled whenever a
// datablock is dispacthed for writing to the UART. Datablocks are dispatched
// every (SENSOR_SSSS_LATENCY) ms
#if (SENSOR_SSSS_RATE_DEBUG_GPIO == 1)
#include "eoss3_hal_gpio.h"
uint8_t sensor_rate_debug_gpio_val = 1;
#endif

/* BEGIN user include files
 * Add header files needed for accessing the sensor APIs
 */

/* User settable MACROs */

/* This section defines MACROs that may be user modified */

MC3635  qorc_ssi_accel;

/* User modifiable sensor descriptor in JSON format */
#if (SSI_SENSOR_SELECT_SSSS == 1)

/* BEGIN JSON descriptor for the sensor configuration */

const char json_string_sensor_config[] = \
"{"\
   "\"sample_rate\":100,"\
   "\"samples_per_packet\":6,"\
   "\"column_location\":{"\
	"  \"AccelerometerX\":0,"
	"  \"AccelerometerY\":1,"
	"  \"AccelerometerZ\":2"
   "}"\
"}\r\n" ;
/* END JSON descriptor for the sensor data */
#endif /* SSI_SENSOR_SELECT_SSSS */

/* User modifiable function. Update the below function
 * to initialize and setup I2C sensors */
void sensor_ssss_configure(void)
{
  /** @todo Replace contents of this function */
  sensor_ssss_config.rate_hz = SENSOR_SSSS_SAMPLE_RATE_HZ;
  sensor_ssss_config.n_channels = SENSOR_SSSS_CHANNELS_PER_SAMPLE;
  sensor_ssss_config.bit_depth = SENSOR_SSSS_BIT_DEPTH;
  sensor_ssss_config.sensor_id = SENSOR_SSSS_ID;
  static int sensor_ssss_configured = false;

  /*--- BEGIN User modifiable section ---*/
  qorc_ssi_accel.begin();
  qorc_ssi_accel.set_sample_rate(sensor_ssss_config.rate_hz);
  qorc_ssi_accel.set_sample_resolution(sensor_ssss_config.bit_depth);
  qorc_ssi_accel.set_mode(MC3635_MODE_CWAKE);

  /*--- END of User modifiable section ---*/

  if (sensor_ssss_configured == false)
  {
    sensor_ssss_configured = true;
  }
  sensor_ssss_startstop(1);
}

/* User modifiable function. Update the below function
 * to read sensor data sample from sensors and fill-in
 * the data block with these sensor data values */
int  sensor_ssss_acquisition_buffer_ready()
{
    int8_t *p_dest = (int8_t *) &psensor_ssss_data_block_prev->p_data;
    int dataElementSize = psensor_ssss_data_block_prev->dbHeader.dataElementSize;
    int ret;
    int batch_size;

    p_dest += sizeof(int8_t)*sensor_ssss_samples_collected*dataElementSize;

    /* Read 1 sample per channel, Fill the sample data to p_dest buffer */

    /*--- BEGIN User modifiable section ---*/
    xyz_t accel_data = qorc_ssi_accel.read();  /* Read accelerometer data from MC3635 */

    /* Fill this accelerometer data into the current data block */
    int16_t *p_accel_data = (int16_t *)p_dest;

    *p_accel_data++ = accel_data.x;
    *p_accel_data++ = accel_data.y;
    *p_accel_data++ = accel_data.z;

    p_dest += 6; // advance datablock pointer to retrieve and store next sensor data

    /* Read data from other sensors */
    int bytes_to_read = SENSOR_SSSS_CHANNELS_PER_SAMPLE * (SENSOR_SSSS_BIT_DEPTH/8) ;

    sensor_ssss_samples_collected += SENSOR_SSSS_CHANNELS_PER_SAMPLE;
    batch_size = sensor_ssss_batch_size_get() * SENSOR_SSSS_CHANNELS_PER_SAMPLE;

    // return 1 if batch_size of samples are collected
    // return 0 otherwise
    if (sensor_ssss_samples_collected >= batch_size)
    {
      psensor_ssss_data_block_prev->dbHeader.numDataElements = sensor_ssss_samples_collected;
      psensor_ssss_data_block_prev->dbHeader.numDataChannels = SENSOR_SSSS_CHANNELS_PER_SAMPLE;
      sensor_ssss_samples_collected = 0;
      return 1;
    }
    else
    {
      return 0;
    }
}

/** End of User modifiable code and variable */

/*========== BEGIN: SSSS SENSOR Datablock processor definitions =============*/
/** @addtogroup QAI_SSSS_PIPELINE_EXAMPLE QORC SDK SSSS pipeline example
 *
 *  @brief SSSS pipeline example code
 *
 * This example code demonstrates setting up SSSS Queues,
 * setting up the datablock buffer manager (\ref DATABLK_MGR)
 * and setting up the datablock processor processing elements (\ref DATABLK_PE).
 * A specific SSSS processing element for motion detection is provided in this
 * example.
 *
 * @{
 */

/** Maximum number of ssss data blocks that may be queued for chain processing */
#define SENSOR_SSSS_NUM_DATA_BLOCKS              (SENSOR_SSSS_MAX_DATA_BLOCKS)

/** maximum number of vertical (parallel processing elements) that may generate datablock outputs
 *  that may add to the front of the queue.
 *
 *  Queue size of a given datablock processor must be atleast
 *  summation of maximum datablocks of all sensors registered for
 *  processing with some room to handle the vertical depth
 */
#define MAX_THREAD_VERTICAL_DEPTH_DATA_BLOCKS  (5)

#define SENSOR_SSSS_DBP_THREAD_Q_SIZE   (SENSOR_SSSS_MAX_DATA_BLOCKS+MAX_THREAD_VERTICAL_DEPTH_DATA_BLOCKS)
#define SENSOR_SSSS_DBP_THREAD_PRIORITY (10)

uint8_t               sensor_ssss_data_blocks[SENSOR_SSSS_MEMSIZE_MAX] ; //PLACE_IN_SECTION("HWA");
QAI_DataBlockMgr_t    sensor_ssssBuffDataBlkMgr;
QueueHandle_t         sensor_ssss_dbp_thread_q;

/* SSSS AI processing element */
extern void sensor_ssss_ai_data_processor(
       QAI_DataBlock_t *pIn,
       QAI_DataBlock_t *pOut,
       QAI_DataBlock_t **pRet,
       datablk_pe_event_notifier_t *pevent_notifier
     );
extern void sensor_ssss_ai_config(void *pDatablockManagerPtr);
extern int  sensor_ssss_ai_start(void);
extern int  sensor_ssss_ai_stop(void);

/** Sensor SSSS AI processing element functions */

datablk_pe_funcs_t sensor_ssss_sensiml_ai_funcs =
{
  .pconfig     = sensor_ssss_ai_config,
  .pprocess    = sensor_ssss_ai_data_processor,
  .pstart      = sensor_ssss_ai_start,
  .pstop       = sensor_ssss_ai_stop,
  .p_pe_object = (void *)NULL
} ;

/** outQ processor for SSSS AI processing element */
outQ_processor_t sensor_ssss_sensiml_ai_outq_processor =
{
  .process_func = NULL,
  .p_dbm = &sensor_ssssBuffDataBlkMgr,
  .in_pid = SENSOR_SSSS_AI_PID,
  .outQ_num = 0,
  .outQ = NULL,
  .p_event_notifier = NULL
};

/* SSSS Live-stream processing element */
extern void sensor_ssss_livestream_data_processor(
       QAI_DataBlock_t *pIn,
       QAI_DataBlock_t *pOut,
       QAI_DataBlock_t **pRet,
       datablk_pe_event_notifier_t *pevent_notifier
     );
extern void sensor_ssss_livestream_config(void *pDatablockManagerPtr);
extern int  sensor_ssss_livestream_start(void);
extern int  sensor_ssss_livestream_stop(void);

/** Sensor SSSS AI processing element functions */

datablk_pe_funcs_t sensor_ssss_livestream_funcs =
{
  .pconfig     = sensor_ssss_livestream_config,
  .pprocess    = sensor_ssss_livestream_data_processor,
  .pstart      = sensor_ssss_livestream_start,
  .pstop       = sensor_ssss_livestream_stop,
  .p_pe_object = (void *)NULL
} ;

/** outQ processor for SSSS Live-stream processing element */
outQ_processor_t sensor_ssss_livestream_outq_processor =
{
  .process_func = NULL,
  .p_dbm = &sensor_ssssBuffDataBlkMgr,
  .in_pid = SENSOR_SSSS_LIVESTREAM_PID,
  .outQ_num = 0,
  .outQ = NULL,
  .p_event_notifier = NULL
};

/* SSSS datasave processing element */
extern void sensor_ssss_datasave_data_processor(
       QAI_DataBlock_t *pIn,
       QAI_DataBlock_t *pOut,
       QAI_DataBlock_t **pRet,
       datablk_pe_event_notifier_t *pevent_notifier
     );
extern void sensor_ssss_datasave_config(void *pDatablockManagerPtr);
extern int  sensor_ssss_datasave_start(void);
extern int  sensor_ssss_datasave_stop(void);

/** Sensor SSSS datasave processing element functions */

datablk_pe_funcs_t sensor_ssss_datasave_funcs =
{
  .pconfig     = sensor_ssss_datasave_config,
  .pprocess    = sensor_ssss_datasave_data_processor,
  .pstart      = sensor_ssss_datasave_start,
  .pstop       = sensor_ssss_datasave_stop,
  .p_pe_object = (void *)NULL
} ;

/** outQ processor for SSSS datasave processing element */
outQ_processor_t sensor_ssss_datasave_outq_processor =
{
  .process_func = NULL,
  .p_dbm = &sensor_ssssBuffDataBlkMgr,
  .in_pid = SENSOR_SSSS_LIVESTREAM_PID,
  .outQ_num = 0,
  .outQ = NULL,
  .p_event_notifier = NULL
};

datablk_pe_descriptor_t  sensor_ssss_datablk_pe_descr[] =
{ // { IN_ID, OUT_ID, ACTIVE, fSupplyOut, fReleaseIn, outQ, &pe_function_pointers, bypass_function, pe_semaphore }
#if (SENSOR_SSSS_RECOG_ENABLED)
    /* processing element descriptor for SensiML AI for SSSS sensor */
    { SENSOR_SSSS_ISR_PID, SENSOR_SSSS_AI_PID, true, false, true, &sensor_ssss_sensiml_ai_outq_processor, &sensor_ssss_sensiml_ai_funcs, NULL, NULL},
#endif

#if (SENSOR_SSSS_LIVESTREAM_ENABLED)
    /* processing element descriptor for SSSS sensor livestream */
    { SENSOR_SSSS_ISR_PID, SENSOR_SSSS_LIVESTREAM_PID, true, false, true, &sensor_ssss_livestream_outq_processor, &sensor_ssss_livestream_funcs, NULL, NULL},
#endif

#if (SENSOR_SSSS_DATASAVE_ENABLED)
    /* processing element descriptor for SSSS sensor datasave */
    { SENSOR_SSSS_ISR_PID, SENSOR_SSSS_DATASAVE_PID, true, false, true, &sensor_ssss_datasave_outq_processor, &sensor_ssss_datasave_funcs, NULL, NULL},
#endif

};

datablk_processor_params_t sensor_ssss_datablk_processor_params[] = {
    { SENSOR_SSSS_DBP_THREAD_PRIORITY,
      &sensor_ssss_dbp_thread_q,
      sizeof(sensor_ssss_datablk_pe_descr)/sizeof(sensor_ssss_datablk_pe_descr[0]),
      sensor_ssss_datablk_pe_descr,
      256*2,
      (char*)"SENSOR_SSSS_DBP_THREAD",
      NULL
    }
};

void sensor_ssss_block_processor(void)
{
  /* Initialize datablock manager */
   datablk_mgr_init( &sensor_ssssBuffDataBlkMgr,
                      sensor_ssss_data_blocks,
                      sizeof(sensor_ssss_data_blocks),
                      (SENSOR_SSSS_SAMPLES_PER_BLOCK),
                      ((SENSOR_SSSS_BIT_DEPTH)/8)
                    );

  /** SSSS datablock processor thread : Create SSSS Queues */
  sensor_ssss_dbp_thread_q = xQueueCreate(SENSOR_SSSS_DBP_THREAD_Q_SIZE, sizeof(QAI_DataBlock_t *));
  vQueueAddToRegistry( sensor_ssss_dbp_thread_q, "SENSOR_SSSSPipelineExampleQ" );

  /** SSSS datablock processor thread : Setup SSSS Thread Handler Processing Elements */
  datablk_processor_task_setup(&sensor_ssss_datablk_processor_params[0]);

  /** Set the first data block for the ISR or callback function */
  sensor_ssss_set_first_data_block();

  /* [TBD]: sensor configuration : should this be here or after scheduler starts? */
  sensor_ssss_add();
  sensor_ssss_configure();
#if 0
  printf("Sensor Name:                   %s\n", "SENSOR_SSSS_NAME");
  printf("Sensor Memory:                 %d\n", (int)SENSOR_SSSS_MEMSIZE_MAX);
  printf("Sensor Sampling rate:          %d Hz\n", (int)SENSOR_SSSS_SAMPLE_RATE_HZ);
  printf("Sensor Number of channels:     %d\n", (int)SENSOR_SSSS_CHANNELS_PER_SAMPLE);
  printf("Sensor frame size per channel: %d\n", (int)SENSOR_SSSS_SAMPLES_PER_CHANNEL);
  printf("Sensor frame size:             %d\n", (int)SENSOR_SSSS_SAMPLES_PER_BLOCK);
  printf("Sensor sample bit-depth:       %d\n", (int)SENSOR_SSSS_BIT_DEPTH);
  printf("Sensor datablock count:        %d\n", (int)SENSOR_SSSS_NUM_DATA_BLOCKS);
#endif
}
/*========== END: SSSS SENSOR Datablock processor definitions =============*/

/* BEGIN timer task related functions */
TimerHandle_t sensor_ssss_TimId ;
extern "C" void sensor_ssss_dataTimer_Callback(TimerHandle_t hdl);
void sensor_ssss_dataTimer_Callback(TimerHandle_t hdl)
{
  // Warning: must not call vTaskDelay(), vTaskDelayUntil(), or specify a non zero
  // block time when accessing a queue or a semaphore.
  sensor_ssss_acquisition_read_callback(); //osSemaphoreRelease(readDataSem_id);
}
void sensor_ssss_dataTimerStart(void)
{
  BaseType_t status;
  TimerCallbackFunction_t xCallback = sensor_ssss_dataTimer_Callback;
#if (USE_SENSOR_SSSS_FIFO_MODE)
  // setup FIFO mode
#else
  int milli_secs = (1000 / SENSOR_SSSS_SAMPLE_RATE_HZ); // reads when a sample is available (upto 416Hz)
#endif

  // Create periodic timer
  if (!sensor_ssss_TimId) {
    sensor_ssss_TimId = xTimerCreate("SensorSSSSTimer", pdMS_TO_TICKS(milli_secs), pdTRUE, (void *)0, xCallback);
    configASSERT(sensor_ssss_TimId != NULL);
  }

  if (sensor_ssss_TimId)  {
    status = xTimerStart (sensor_ssss_TimId, 0);  // start timer
    if (status != pdPASS)  {
      // Timer could not be started
    }
  }
#if (USE_SENSOR_SSSS_FIFO_MODE)
  // setup FIFO mode
#endif
  // start the sensor
}

void sensor_ssss_dataTimerStop(void)
{
  if (sensor_ssss_TimId) {
    xTimerStop(sensor_ssss_TimId, 0);
  }
  // stop the sensor
}

/* END timer task related functions */

/* BEGIN Sensor Generic Configuration */

sensor_generic_config_t sensor_ssss_config;

void sensor_ssss_startstop( int is_start )
{
  /** @todo Replace contents of this function */
  if ((is_start) && (sensor_ssss_config.enabled) && (sensor_ssss_config.is_running == 0) )
  {
     sensor_ssss_dataTimerStart();
     sensor_ssss_config.is_running = 1;
  }
  else if ( (is_start == 0) && (sensor_ssss_config.is_running == 1) )
  {
    sensor_ssss_dataTimerStop();
    sensor_ssss_config.is_running = 0;
  }
}

void sensor_ssss_clear( void )
{
  sensor_ssss_config.enabled = false;
  /** @todo Replace contents of this function */
}

void sensor_ssss_add(void)
{
  sensor_ssss_config.enabled = true;
  /** @todo Replace contents of this function */
}
/* End of Sensor Generic Configuration */

/* BEGIN SSSS Acquisition */
/* Sensor SSSS capture ISR */
#define SENSOR_SSSS_ISR_EVENT_NO_BUFFER  (1)   ///< error getting a new datablock buffer

#define SSSS_ISR_OUTQS_NUM        (1)
QueueHandle_t   *sensor_ssss_isr_outQs[SSSS_ISR_OUTQS_NUM] = { &sensor_ssss_dbp_thread_q };
QAI_DataBlock_t *psensor_ssss_data_block_prev = NULL;
int              sensor_ssss_samples_collected = 0;

outQ_processor_t sensor_ssss_isr_outq_processor =
{
  .process_func = sensor_ssss_acquisition_read_callback,
  .p_dbm = &sensor_ssssBuffDataBlkMgr,
  .in_pid = SENSOR_SSSS_ISR_PID,
  .outQ_num = 1,
  .outQ = sensor_ssss_isr_outQs,
  .p_event_notifier = NULL
};

void sensor_ssss_set_first_data_block()
{
  /* Acquire a datablock buffer */
  if (NULL == psensor_ssss_data_block_prev)
  {
    datablk_mgr_acquire(sensor_ssss_isr_outq_processor.p_dbm, &psensor_ssss_data_block_prev, 0);
  }
  configASSERT(psensor_ssss_data_block_prev); // probably indicates uninitialized datablock manager handle
  sensor_ssss_samples_collected = 0;
  psensor_ssss_data_block_prev->dbHeader.Tstart = xTaskGetTickCount();
}

int sensor_ssss_batch_size_get(void)
{
  return (SENSOR_SSSS_SAMPLES_PER_CHANNEL);
}

void sensor_ssss_acquisition_read_callback(void)
{
    int gotNewBlock = 0;
    QAI_DataBlock_t  *pdata_block = NULL;

    if (!sensor_ssss_acquisition_buffer_ready())
    {
      return;
    }
    /* Acquire a new data block buffer */
    datablk_mgr_acquire(sensor_ssss_isr_outq_processor.p_dbm, &pdata_block, 0);
    if (pdata_block)
    {
        gotNewBlock = 1;
    }
    else
    {
        // send error message
        // xQueueSendFromISR( error_queue, ... )
        if (sensor_ssss_isr_outq_processor.p_event_notifier)
          (*sensor_ssss_isr_outq_processor.p_event_notifier)(sensor_ssss_isr_outq_processor.in_pid, SENSOR_SSSS_ISR_EVENT_NO_BUFFER, NULL, 0);
        pdata_block = psensor_ssss_data_block_prev;
        pdata_block->dbHeader.Tstart = xTaskGetTickCount();
        pdata_block->dbHeader.numDropCount++;
    }

    if (gotNewBlock)
    {
        /* send the previously filled ssss data to specified output Queues */
        psensor_ssss_data_block_prev->dbHeader.Tend = pdata_block->dbHeader.Tstart;
        datablk_mgr_WriteDataBufferToQueues(&sensor_ssss_isr_outq_processor, NULL, psensor_ssss_data_block_prev);
        psensor_ssss_data_block_prev = pdata_block;
    }
}
/* END SSSS Acquisition */

/* SSSS AI processing element functions */
void sensor_ssss_ai_data_processor(
       QAI_DataBlock_t *pIn,
       QAI_DataBlock_t *pOut,
       QAI_DataBlock_t **pRet,
       datablk_pe_event_notifier_t *pevent_notifier
     )
{
    int16_t *p_data = (int16_t *) ( (uint8_t *)pIn + offsetof(QAI_DataBlock_t, p_data) );

    // Invoke the SensiML recognition API
    int nSamples = pIn->dbHeader.numDataElements;
    int nChannels = pIn->dbHeader.numDataChannels;
#if S3AI_FIRMWARE_DATASAVE
    set_recognition_current_block_time();
#endif
    int batch_sz = nSamples / nChannels;
    int classification = sml_recognition_run_batch(p_data, batch_sz, nChannels, sensor_ssss_config.sensor_id);
    *pRet = NULL;
    return;
}

void sensor_ssss_ai_config(void *pobj)
{
}

int  sensor_ssss_ai_start(void)
{
  return 0;
}

int  sensor_ssss_ai_stop(void)
{
  return 0;
}

void sensor_ssss_event_notifier(int pid, int event_type, void *p_event_data, int num_data_bytes)
{
  char *p_data = (char *)p_event_data;
  printf("[SSSS Event] PID=%d, event_type=%d, data=%02x\n", pid, event_type, p_data[0]);
}

#if (SENSOR_COMMS_KNOWN_PATTERN == 1)
int16_t sensor_ssss_debug_buffer[240]; // Buffer intended to send a known pattern such as sawtooth
int16_t sensor_ssss_debug_data = 0;    // state for holding current data for the known pattern
#endif

/* SSSS livestream processing element functions */

void sensor_ssss_livestream_data_processor(
       QAI_DataBlock_t *pIn,
       QAI_DataBlock_t *pOut,
       QAI_DataBlock_t **pRet,
       datablk_pe_event_notifier_t *pevent_notifier
     )
{
    int16_t *p_data = (int16_t *) ( (uint8_t *)pIn + offsetof(QAI_DataBlock_t, p_data) );
    //struct sensor_data sdi;
    uint64_t  time_start, time_curr, time_end, time_incr;

    if (sensor_ssss_config.enabled == true)
    {
      // Live-stream data to the host
      uint8_t *p_source = pIn->p_data ;
      int ilen = pIn->dbHeader.numDataElements * pIn->dbHeader.dataElementSize ;
#if (SENSOR_SSSS_RATE_DEBUG_GPIO == 1)
      // Toggle GPIO to indicate that a new datablock buffer is dispatched to UART
      // for transmission for data collection
      HAL_GPIO_Write(GPIO_2, sensor_rate_debug_gpio_val);
      sensor_rate_debug_gpio_val ^= 1;
#endif

#if (SENSOR_COMMS_KNOWN_PATTERN == 1)
      int nSamples = pIn->dbHeader.numDataElements;
      int nChannels = pIn->dbHeader.numDataChannels;
      // prepare the sawtooth known pattern data
      for (int k = 0; k < nSamples; k+=nChannels)
      {
    	  for (int l = 0; l < nChannels; l++)
              sensor_ssss_debug_buffer[k+l] = sensor_ssss_debug_data;
    	  sensor_ssss_debug_data++;
      }
	  memcpy (pIn->p_data, sensor_ssss_debug_buffer, nSamples * sizeof(int16_t));
#endif
      ssi_publish_sensor_data(p_source, ilen);
    }
    *pRet = NULL;
    return;
}

void sensor_ssss_livestream_config(void *pobj)
{
}

int  sensor_ssss_livestream_start(void)
{
  return 0;
}

int  sensor_ssss_livestream_stop(void)
{
  return 0;
}

void sensor_ssss_datasave_data_processor(
       QAI_DataBlock_t *pIn,
       QAI_DataBlock_t *pOut,
       QAI_DataBlock_t **pRet,
       datablk_pe_event_notifier_t *pevent_notifier
     )
{
    int16_t *p_data = (int16_t *) ( (uint8_t *)pIn + offsetof(QAI_DataBlock_t, p_data) );
    //struct sensor_data sdi;
    uint64_t  time_start, time_curr, time_end, time_incr;

    if (sensor_ssss_config.enabled == true)
    {
      // Live-stream data to the host
      uint8_t *p_source = pIn->p_data ;
      int ilen = pIn->dbHeader.numDataElements * pIn->dbHeader.dataElementSize ;
      /* Save data to the */
      struct sensor_data Info, *pInfo = &Info;
      pInfo->bytes_per_reading = pIn->dbHeader.dataElementSize;
      pInfo->n_bytes = ilen;
      pInfo->rate_hz = sensor_ssss_config.rate_hz;
      pInfo->sensor_id = sensor_ssss_config.sensor_id;
      pInfo->time_end = convert_to_uSecCount(pIn->dbHeader.Tend);
      pInfo->time_start = convert_to_uSecCount(pIn->dbHeader.Tstart);
      pInfo->vpData = p_source;
      data_save((const struct sensor_data *)pInfo);

      }
    *pRet = NULL;
    return;
}

void sensor_ssss_datasave_config(void *pobj)
{
}

int  sensor_ssss_datasave_start(void)
{
  return 0;
}

int  sensor_ssss_datasave_stop(void)
{
  return 0;
}

