/** @file sensor_ad7476_process.c */

/*==========================================================
 *
 *-  Copyright Notice  -------------------------------------
 *                                                          
 *    Licensed Materials - Property of QuickLogic Corp.     
 *    Copyright (C) 2019 QuickLogic Corporation             
 *    All rights reserved                                   
 *    Use, duplication, or disclosure restricted            
 *                                                          
 *    File   : sensor_ad7476_process.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

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
#if 0
#define AD7476_MAX_SAMPLE_RATE_HZ           (1666)  ///< Maximum sample rate (in Hz) for this sensor
#define AD7476_FRAME_SIZE_PER_CHANNEL       (18)    ///< 10ms of data @ 1666Hz sample rate
#define AD7476_NUM_CHANNELS                 (6)     ///< (X,Y,Z) for Accel and (X,Y,Z) for Gryo
#define AD7476_FRAME_SIZE                   ( (AD7476_FRAME_SIZE_PER_CHANNEL) * (AD7476_NUM_CHANNELS) )
#define AD7476_NUM_DATA_BLOCKS              (60)    ///< 125 data blocks approximately 1 sec of data
#endif

#define _STR(x)   #x
#define STR(x)    _STR(x)
#pragma message("frame size per channel" STR(AD7476_FRAME_SIZE_PER_CHANNEL))

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

/* AD7476 datasave processing element functions */
extern void ad7476_datasave_data_processor(
       QAI_DataBlock_t *pIn,
       QAI_DataBlock_t *pOut,
       QAI_DataBlock_t **pRet,
       datablk_pe_event_notifier_t *pevent_notifier
     );
extern void ad7476_datasave_config(void *pDatablockManagerPtr);

datablk_pe_funcs_t ad7476_datasave_funcs = 
{
  .pconfig     = ad7476_datasave_config, 
  .pprocess    = ad7476_datasave_data_processor, 
  .pstart      = NULL, 
  .pstop       = NULL, 
  .p_pe_object = (void *)&ad7476BuffDataBlkMgr 
} ;

datablk_pe_descriptor_t  ad7476_datablk_pe_descr[] =
{ // { IN_ID, OUT_ID, ACTIVE, fSupplyOut, fReleaseIn, outQ, &pe_function_pointers, bypass_function, pe_semaphore }
#if (SENSOR_AD7476_RECOG_ENABLED)
    /* processing element descriptor for SensiML AI for AD7476 sensor */
    { AD7476_ISR_PID, AD7476_AI_PID, true, false, true, &ad7476_sensiml_ai_outq_processor, &ad7476_sensiml_ai_funcs, NULL, NULL},   
#endif

#if (SENSOR_AD7476_DATASAVE_ENABLED) // && (S3AI_FIRMWARE_IS_COLLECTION)
    /* processing element descriptor for AD7476 sesnsor datasave */
    { AD7476_ISR_PID, AD7476_DATASAVE_PID, true, false, false, NULL, &ad7476_datasave_funcs, NULL, NULL},   
#endif

#if (SENSOR_AD7476_LIVESTREAM_ENABLED) // && (S3AI_FIRMWARE_IS_COLLECTION)
    /* processing element descriptor for AD7476 sesnsor datasave */
    { AD7476_ISR_PID, AD7476_LIVESTREAM_PID, true, false, true, &ad7476_livestream_outq_processor, &ad7476_livestream_funcs, NULL, NULL},   
#endif

#if (0) // (SENSOR_AD7476_ENABLED == 0)
    { (process_id_t)0, (process_id_t)0, false, false, false, NULL, NULL, NULL, NULL}
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
  sensor_ad7476_configure(); 
  sensor_ad7476_startstop(1);
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

#if (0)
#include "save_datablocks.h"
static sensor_blocks_info_t ad7476_blocks_info; /* ad7476 sensor */

/*
* Count the total number of available bytes in all the block held in the array.
* If there enrough for 1 RIFF block save them.
* But first check if there a free RIFF block available.
*/
static void ad7476_save_blocks(void)
{
    sensor_blocks_info_t *ad7476 = &ad7476_blocks_info;
    
    int avail_bytes;

    /* need 1 free RIFF blocks for saving ad7476 */
    if(!check_free_riff_blocks(1))
      return;
    
    /* should have enough bytes to fill a RIFF block */
    avail_bytes = get_available_bytes(ad7476);
    if(avail_bytes < get_riff_block_size())
      return;

    /* store data, starting from first stored index block */
    store_one_sensor(ad7476, ad7476_config.rate_hz, SENSOR_AD7476_ID);
    
    return;
}
/* 
 * Check if enough data for 1 RIFF and block and then Save and Release the 
 *  data blocks held.
 */
void ad7476_block_save(QAI_DataBlock_t *pIn)
{
    sensor_blocks_info_t *ad7476 = &ad7476_blocks_info;

    /* first put the block in the array */
    ad7476->sensor_data_blocks[ad7476->array_hold_index++] = pIn;

    /* no point holding blocks if not saving */
    /* if not enabled... then throw data away */
    if(!datablock_save_status(SENSOR_AD7476_ID) || (ad7476_config.enabled == 0))
    {
      release_save_blocks(ad7476, ad7476->array_hold_index);
      ad7476->block_rd_index = 0;
      return;
    }
    
    /* if enough data for 1 RIFF block, store */
    ad7476_save_blocks();
    
    /* if the count exceeds, then release half (?) of them */
    if(ad7476->array_hold_index >= ad7476->array_hold_max)
    {
      int release_count = ad7476->array_hold_index/2;
      
      release_save_blocks(ad7476, release_count);
      ad7476->block_rd_index = 0;
    } 
    
    return;
}

/* 
 * Only data block instantiating function in main knows the maximum blocks.
 */
void ad7476_set_hold_count(int total_count)
{
    /* Generally approximately 3/4 of the blocks could be held until stored */
    total_count = (3*total_count)/4;
    if(total_count > MAX_SAVE_BLOCKS_ARRAY_SIZE)
      total_count = MAX_SAVE_BLOCKS_ARRAY_SIZE;
    
    reset_sensor_array_counts(&ad7476_blocks_info, total_count);
    
    return;
}

void ad7476_datasave_data_processor(
       QAI_DataBlock_t *pIn,
       QAI_DataBlock_t *pOut,
       QAI_DataBlock_t **pRet,
       datablk_pe_event_notifier_t *pevent_notifier
     )
{
   ad7476_block_save(pIn);
   *pRet = NULL;
   return;
}

void ad7476_datasave_config(void *pDatablockManagerPtr)
{
  QAI_DataBlockMgr_t *pdbm = (QAI_DataBlockMgr_t *)pDatablockManagerPtr;
  int max_blocks = pdbm->max_data_blocks ;
  ad7476_set_hold_count(max_blocks);
}

#endif