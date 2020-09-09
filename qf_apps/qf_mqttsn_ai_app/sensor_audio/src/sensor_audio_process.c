/** @file sensor_audio_process.c */

/*==========================================================
 *
 *-  Copyright Notice  -------------------------------------
 *                                                          
 *    Licensed Materials - Property of QuickLogic Corp.     
 *    Copyright (C) 2019 QuickLogic Corporation             
 *    All rights reserved                                   
 *    Use, duplication, or disclosure restricted            
 *                                                          
 *    File   : sensor_audio_process.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

#include <stdbool.h>
#include "datablk_mgr.h"
#include "process_ids.h"
#include "datablk_processor.h"
#include "DataCollection.h"
#include "sensor_audio_config.h"
#include "sensor_audio_acquisition.h"
#include "sensor_audio_process.h"
/** */

/*========== BEGIN: AUDIO SENSOR Datablock processor definitions =============*/
/** @addtogroup QAI_AUDIO_PIPELINE_EXAMPLE QuickAI SDK AUDIO pipeline example
 *
 *  @brief AUDIO pipeline example code
 *
 * This example code demonstrates setting up AUDIO Queues,
 * setting up the datablock buffer manager (\ref DATABLK_MGR)
 * and setting up the datablock processor processing elements (\ref DATABLK_PE).
 * A specific AUDIO processing element for motion detection is provided in this
 * example.
 * 
 * @{
 */

/** Maximum number of audio data blocks that may be queued for chain processing */
#define AUDIO_MAX_DATA_BLOCKS              (AUDIO_NUM_DATA_BLOCKS)

/** maximum number of vertical (parallel processing elements) that may generate datablock outputs 
 *  that may add to the front of the queue.
 *
 *  Queue size of a given datablock processor must be atleast 
 *  summation of maximum datablocks of all sensors registered for
 *  processing with some room to handle the vertical depth
 */
#define MAX_THREAD_VERTICAL_DEPTH_DATA_BLOCKS  (5)

#define AUDIO_DBP_THREAD_Q_SIZE   (AUDIO_MAX_DATA_BLOCKS+MAX_THREAD_VERTICAL_DEPTH_DATA_BLOCKS)
#define AUDIO_DBP_THREAD_PRIORITY (10)

uint8_t               audio_data_blocks[SENSOR_AUDIO_MEMSIZE_MAX] PLACE_IN_SECTION("HWA");
QAI_DataBlockMgr_t    audioBuffDataBlkMgr;
QueueHandle_t         audio_dbp_thread_q;

/* AUDIO AI processing element */
extern void audio_ai_data_processor(
       QAI_DataBlock_t *pIn,
       QAI_DataBlock_t *pOut,
       QAI_DataBlock_t **pRet,
       datablk_pe_event_notifier_t *pevent_notifier
     );
extern void audio_ai_config(void *pDatablockManagerPtr);
extern int  audio_ai_start(void);
extern int  audio_ai_stop(void);

/** AUDIO AI processing element functions */

datablk_pe_funcs_t audio_sensiml_ai_funcs = 
{
  .pconfig     = audio_ai_config,
  .pprocess    = audio_ai_data_processor, 
  .pstart      = audio_ai_start, 
  .pstop       = audio_ai_stop, 
  .p_pe_object = (void *)NULL 
} ;

/** outQ processor for AUDIO AI processing element */
outQ_processor_t audio_sensiml_ai_outq_processor =
{
  .process_func = NULL,
  .p_dbm = &audioBuffDataBlkMgr,
  .in_pid = AUDIO_AI_PID,
  .outQ_num = 0,
  .outQ = NULL,
  .p_event_notifier = NULL
};

/* AUDIO Live-stream processing element */
extern void audio_livestream_data_processor(
       QAI_DataBlock_t *pIn,
       QAI_DataBlock_t *pOut,
       QAI_DataBlock_t **pRet,
       datablk_pe_event_notifier_t *pevent_notifier
     );
extern void audio_livestream_config(void *pDatablockManagerPtr);
extern int  audio_livestream_start(void);
extern int  audio_livestream_stop(void);

/** AUDIO AI processing element functions */

datablk_pe_funcs_t audio_livestream_funcs = 
{
  .pconfig     = audio_livestream_config,
  .pprocess    = audio_livestream_data_processor, 
  .pstart      = audio_livestream_start, 
  .pstop       = audio_livestream_stop, 
  .p_pe_object = (void *)NULL 
} ;

/** outQ processor for AUDIO Live-stream processing element */
outQ_processor_t audio_livestream_outq_processor =
{
  .process_func = NULL,
  .p_dbm = &audioBuffDataBlkMgr,
  .in_pid = AUDIO_LIVESTREAM_PID,
  .outQ_num = 0,
  .outQ = NULL,
  .p_event_notifier = NULL
};

datablk_pe_descriptor_t  audio_datablk_pe_descr[] =
{ // { IN_ID, OUT_ID, ACTIVE, fSupplyOut, fReleaseIn, outQ, &pe_function_pointers, bypass_function, pe_semaphore }
#if (SENSOR_AUDIO_RECOG_ENABLED) && (S3AI_FIRMWARE_IS_RECOGNITION)
    /* processing element descriptor for SensiML AI for AUDIO sensor */
    { AUDIO_ISR_PID, AUDIO_AI_PID, true, false, true, &audio_sensiml_ai_outq_processor, &audio_sensiml_ai_funcs, NULL, NULL},   
#endif

#if (SENSOR_AUDIO_LIVESTREAM_ENABLED) && (S3AI_FIRMWARE_IS_COLLECTION)
    /* processing element descriptor for AUDIO sesnsor livestream */
    { AUDIO_ISR_PID, AUDIO_LIVESTREAM_PID, true, false, true, &audio_livestream_outq_processor, &audio_livestream_funcs, NULL, NULL},   
#endif 
};

datablk_processor_params_t audio_datablk_processor_params[] = { 
    { AUDIO_DBP_THREAD_PRIORITY, 
      &audio_dbp_thread_q, 
      sizeof(audio_datablk_pe_descr)/sizeof(audio_datablk_pe_descr[0]), 
      audio_datablk_pe_descr,
      256, 
      "AUDIO_DBP_THREAD", 
      NULL
    }
};

void audio_block_processor(void)
{
  /* Initialize datablock manager */
   datablk_mgr_init( &audioBuffDataBlkMgr,
                      audio_data_blocks, 
                      sizeof(  audio_data_blocks), 
                      AUDIO_FRAME_SIZE, 
                      sizeof(int16_t)
                    );

  /** AUDIO datablock processor thread : Create AUDIO Queues */
  audio_dbp_thread_q = xQueueCreate(AUDIO_DBP_THREAD_Q_SIZE, sizeof(QAI_DataBlock_t *));
  vQueueAddToRegistry( audio_dbp_thread_q, "AUDIOPipelineExampleQ" );
  
  /** AUDIO datablock processor thread : Setup AUDIO Thread Handler Processing Elements */
  datablk_processor_task_setup(&audio_datablk_processor_params[0]);
  
  /** Set the first data block for the ISR or callback function */
  audio_set_first_data_block();

  /* [TBD]: sensor configuration : should this be here or after scheduler starts? */
  //sensor_audio_configure();
  
  printf("Sensor Name:                   %s\n", "SENSOR_AUDIO_NAME");
  printf("Sensor Memory:                 %d\n", SENSOR_AUDIO_MEMSIZE_MAX);
  printf("Sensor Sampling rate max:      %d Hz\n", (int)SENSOR_AUDIO_RATE_HZ_MAX);
  printf("Sensor Number of channels:     %d\n", SENSOR_AUDIO_CHANNELS);
  printf("Sensor frame size per channel: %d\n", AUDIO_FRAME_SIZE_PER_CHANNEL);
  printf("Sensor frame size:             %d\n", AUDIO_FRAME_SIZE);
  printf("Sensor datablock count:        %d\n", AUDIO_NUM_DATA_BLOCKS);
}
/*========== END: AUDIO SENSOR Datablock processor definitions =============*/

/* AUDIO AI processing element functions */
void audio_ai_data_processor(
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

    //recog_data_using_dbp(p_data, (nSamples/nChannels), nChannels, SENSOR_AUDIO_ID);

    *pRet = NULL;
    return;
}

void audio_ai_config(void *pobj)
{
}

int  audio_ai_start(void)
{
  return 0;
}

int  audio_ai_stop(void)
{
  return 0;
}

void audio_event_notifier(int pid, int event_type, void *p_event_data, int num_data_bytes)
{
  char *p_data = (char *)p_event_data;
  printf("[AUDIO Event] PID=%d, event_type=%d, data=%02x\n", pid, event_type, p_data[0]);
}

/* AUDIO livestream processing element functions */
void audio_livestream_data_processor(
       QAI_DataBlock_t *pIn,
       QAI_DataBlock_t *pOut,
       QAI_DataBlock_t **pRet,
       datablk_pe_event_notifier_t *pevent_notifier
     )
{
    int16_t *p_data = (int16_t *) ( (uint8_t *)pIn + offsetof(QAI_DataBlock_t, p_data) );
    struct sensor_data sdi; 
    uint64_t  time_start, time_curr, time_end, time_incr;

    if (sensor_audio_config.enabled == true)
    {
      // Live-stream data to the host
      int nSamples = pIn->dbHeader.numDataElements;
      int nChannels = pIn->dbHeader.numDataChannels;
      time_start = convert_to_uSecCount(pIn->dbHeader.Tstart);
      time_incr  = ((uint64_t)(pIn->dbHeader.Tend - pIn->dbHeader.Tstart) * 1000) / (nSamples / nChannels);
      time_curr  = time_start;
      time_end   = convert_to_uSecCount(pIn->dbHeader.Tend);

      sdi.time_start = time_start;
      sdi.time_end   = time_end;
      sdi.sensor_id  = SENSOR_AUDIO_ID;
      sdi.rate_hz    = sensor_audio_config.rate_hz;
      sdi.n_bytes    = nSamples * (pIn->dbHeader.dataElementSize);
      sdi.bytes_per_reading = pIn->dbHeader.dataElementSize;
      sdi.vpData     = (void *)pIn->p_data;

      ble_send( &sdi );
    }
    *pRet = NULL;
    return;
}

void audio_livestream_config(void *pobj)
{
}

int  audio_livestream_start(void)
{
  return 0;
}

int  audio_livestream_stop(void)
{
  return 0;
}
