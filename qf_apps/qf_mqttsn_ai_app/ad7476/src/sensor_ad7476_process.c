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

/** @file sensor_ad7476_process.c */

#include <stdbool.h>
#include "datablk_mgr.h"
#include "process_ids.h"
#include "datablk_processor.h"

#include "sensor_ad7476_config.h"
#include "sensor_ad7476_acquisition.h"
#include "sensor_ad7476_process.h"

#include "micro_tick64.h"
#include "DataCollection.h"
#include "Recognition.h"

/** */

/*========== BEGIN: AD7476 SENSOR Datablock processor definitions =============*/
/** @addtogroup QAI_AD7476_PIPELINE_EXAMPLE QuickAI SDK AD7476 pipeline example
 *
 *  @brief AD7476 pipeline example code
 *
 * This example code demonstrates setting up AD7476 Queues,
 * setting up the datablock buffer manager (\ref DATABLK_MGR)
 * and setting up the datablock processor processing elements (\ref DATABLK_PE).
 * A specific AD7476 processing element for motion detection is provided in this
 * example.
 * 
 * @{
 */

/** Maximum number of ad7476 data blocks that may be queued for chain processing */
#define AD7476_MAX_DATA_BLOCKS              (AD7476_NUM_DATA_BLOCKS)

/** maximum number of vertical (parallel processing elements) that may generate datablock outputs 
 *  that may add to the front of the queue.
 *
 *  Queue size of a given datablock processor must be atleast 
 *  summation of maximum datablocks of all sensors registered for
 *  processing with some room to handle the vertical depth
 */
#define MAX_THREAD_VERTICAL_DEPTH_DATA_BLOCKS  (5)

#define AD7476_DBP_THREAD_Q_SIZE   (AD7476_MAX_DATA_BLOCKS+MAX_THREAD_VERTICAL_DEPTH_DATA_BLOCKS)
#define AD7476_DBP_THREAD_PRIORITY (10)

uint8_t               ad7476_data_blocks[SENSOR_AD7476_MEMSIZE_MAX] ;
QAI_DataBlockMgr_t    ad7476BuffDataBlkMgr;
QueueHandle_t         ad7476_dbp_thread_q;

/* AD7476 AI processing element */
extern void ad7476_ai_data_processor(
       QAI_DataBlock_t *pIn,
       QAI_DataBlock_t *pOut,
       QAI_DataBlock_t **pRet,
       datablk_pe_event_notifier_t *pevent_notifier
     );
extern void ad7476_ai_config(void *pDatablockManagerPtr);
extern int  ad7476_ai_start(void);
extern int  ad7476_ai_stop(void);

/** AD7476 AI processing element functions */

datablk_pe_funcs_t ad7476_sensiml_ai_funcs = 
{
  .pconfig     = ad7476_ai_config,
  .pprocess    = ad7476_ai_data_processor, 
  .pstart      = ad7476_ai_start, 
  .pstop       = ad7476_ai_stop, 
  .p_pe_object = (void *)NULL 
} ;

/** outQ processor for AD7476 AI processing element */
outQ_processor_t ad7476_sensiml_ai_outq_processor =
{
  .process_func = NULL,
  .p_dbm = &ad7476BuffDataBlkMgr,
  .in_pid = AD7476_AI_PID,
  .outQ_num = 0,
  .outQ = NULL,
  .p_event_notifier = NULL
};

/* AD7476 Live-stream processing element */
extern void ad7476_livestream_data_processor(
       QAI_DataBlock_t *pIn,
       QAI_DataBlock_t *pOut,
       QAI_DataBlock_t **pRet,
       datablk_pe_event_notifier_t *pevent_notifier
     );
extern void ad7476_livestream_config(void *pDatablockManagerPtr);
extern int  ad7476_livestream_start(void);
extern int  ad7476_livestream_stop(void);

/** AD7476 Live-stream processing element functions */

datablk_pe_funcs_t ad7476_livestream_funcs = 
{
  .pconfig     = ad7476_livestream_config,
  .pprocess    = ad7476_livestream_data_processor,
  .pstart      = ad7476_livestream_start,
  .pstop       = ad7476_livestream_stop,
  .p_pe_object = (void *)NULL 
} ;

/** outQ processor for AD7476 Live-stream processing element */
outQ_processor_t ad7476_livestream_outq_processor =
{
  .process_func = NULL,
  .p_dbm = &ad7476BuffDataBlkMgr,
  .in_pid = AD7476_AI_PID,
  .outQ_num = 0,
  .outQ = NULL,
  .p_event_notifier = NULL
};

datablk_pe_descriptor_t  ad7476_datablk_pe_descr[] =
{ // { IN_ID, OUT_ID, ACTIVE, fSupplyOut, fReleaseIn, outQ, &pe_function_pointers, bypass_function, pe_semaphore }
#if (SENSOR_AD7476_RECOG_ENABLED) && (S3AI_FIRMWARE_IS_RECOGNITION)
    /* processing element descriptor for SensiML AI for AD7476 sensor */
    { AD7476_ISR_PID, AD7476_AI_PID, true, false, true, &ad7476_sensiml_ai_outq_processor, &ad7476_sensiml_ai_funcs, NULL, NULL},   
#endif

#if (SENSOR_AD7476_LIVESTREAM_ENABLED) && (S3AI_FIRMWARE_IS_COLLECTION)
    /* processing element descriptor for AD7476 sesnsor live-streaming */
    { AD7476_ISR_PID, AD7476_LIVESTREAM_PID, true, false, true, &ad7476_livestream_outq_processor, &ad7476_livestream_funcs, NULL, NULL},   
#endif
};

datablk_processor_params_t ad7476_datablk_processor_params[] = { 
    { AD7476_DBP_THREAD_PRIORITY, 
      &ad7476_dbp_thread_q, 
      sizeof(ad7476_datablk_pe_descr)/sizeof(ad7476_datablk_pe_descr[0]), 
      ad7476_datablk_pe_descr,
      256, 
      "AD7476_DBP_THREAD", 
      NULL
    }
};

void ad7476_block_processor(void)
{
  printf("Sensor Name:                   %s\n", "SENSOR_AD7476_NAME");
  printf("Sensor Memory:                 %d\n", SENSOR_AD7476_MEMSIZE_MAX);
  printf("Sensor Sampling rate max:      %d Hz\n", (int)SENSOR_AD7476_RATE_HZ_MAX);
  printf("Sensor Number of channels:     %d\n", SENSOR_AD7476_CHANNELS);
  printf("Sensor frame size per channel: %d\n", AD7476_FRAME_SIZE_PER_CHANNEL);
  printf("Sensor frame size:             %d\n", AD7476_FRAME_SIZE);
  printf("Sensor datablock count:        %d\n", AD7476_NUM_DATA_BLOCKS);

  /* Initialize datablock manager */
   datablk_mgr_init( &ad7476BuffDataBlkMgr,
                      ad7476_data_blocks, 
                      sizeof(  ad7476_data_blocks), 
                      AD7476_FRAME_SIZE, 
                      sizeof(int16_t)
                    );

  /** AD7476 datablock processor thread : Create AD7476 Queues */
  ad7476_dbp_thread_q = xQueueCreate(AD7476_DBP_THREAD_Q_SIZE, sizeof(QAI_DataBlock_t *));
  vQueueAddToRegistry( ad7476_dbp_thread_q, "AD7476PipelineExampleQ" );
  
  /** AD7476 datablock processor thread : Setup AD7476 Thread Handler Processing Elements */
  datablk_processor_task_setup(&ad7476_datablk_processor_params[0]);
  
  /** Set the first data block for the ISR or callback function */
  ad7476_set_first_data_block();

  /* [TBD]: sensor configuration : should this be here or after scheduler starts? */
  //sensor_ad7476_configure(); 
  //sensor_ad7476_startstop(1);
}
/*========== END: AD7476 SENSOR Datablock processor definitions =============*/

/* AD7476 AI processing element functions */
void ad7476_ai_data_processor(
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

    recog_data_using_dbp(p_data, (nSamples/nChannels), nChannels, SENSOR_AD7476_ID);

    *pRet = NULL;
    return;
}

void ad7476_ai_config(void *pobj)
{
}

int  ad7476_ai_start(void)
{
  return 0;
}

int  ad7476_ai_stop(void)
{
  return 0;
}

void ad7476_event_notifier(int pid, int event_type, void *p_event_data, int num_data_bytes)
{
  char *p_data = (char *)p_event_data;
  printf("[AD7476 Event] PID=%d, event_type=%d, data=%02x\n", pid, event_type, p_data[0]);
}
#include "sensor_live.h"
/* AD7476 Live-stream processing element functions */
void ad7476_livestream_data_processor(
       QAI_DataBlock_t *pIn,
       QAI_DataBlock_t *pOut,
       QAI_DataBlock_t **pRet,
       datablk_pe_event_notifier_t *pevent_notifier
     )
{
    int16_t *p_data = (int16_t *)pIn->p_data; // ( (uint8_t *)pIn + offsetof(QAI_DataBlock_t, p_data) );
    struct sensor_data sdi; 
    uint64_t  time_start, time_curr, time_end, time_incr;

    // Invoke the Live-stream API
    int nSamples = pIn->dbHeader.numDataElements;
    int nChannels = pIn->dbHeader.numDataChannels;

    time_start = convert_to_uSecCount(pIn->dbHeader.Tstart);
    time_end   = convert_to_uSecCount(pIn->dbHeader.Tend);
    time_incr  = ((uint64_t)(pIn->dbHeader.Tend - pIn->dbHeader.Tstart) * 1000) / (nSamples / nChannels);
    time_curr  = time_start;

    memset( (void *)&(sdi), 0, sizeof(sdi) );
    sdi.time_start        = time_curr;
    sdi.time_end          = time_end;
    sdi.bytes_per_reading = pIn->dbHeader.dataElementSize;
    sdi.n_bytes           = nSamples * nChannels * sdi.bytes_per_reading;
    sdi.rate_hz           = ad7476_config.rate_hz;
    sdi.vpData            = (void *)p_data;
    sdi.sensor_id         = SENSOR_AD7476_ID;

    //dbg_str_int_noln("Sending", sdi.n_bytes);
    //dbg_str_int_noln(" bytes, sensor_id", sdi.sensor_id);
    //dbg_str_int     ("AD7476 ", xTaskGetTickCount());
    ble_send(&sdi);

    *pRet = NULL;
    return;
}

void ad7476_livestream_config(void *pobj)
{
}

int  ad7476_livestream_start(void)
{
  return 0;
}

int  ad7476_livestream_stop(void)
{
  return 0;
}

