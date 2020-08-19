/** @file sensor_ad7476_process.h */

/*==========================================================
 *
 *-  Copyright Notice  -------------------------------------
 *                                                          
 *    Licensed Materials - Property of QuickLogic Corp.     
 *    Copyright (C) 2019 QuickLogic Corporation             
 *    All rights reserved                                   
 *    Use, duplication, or disclosure restricted            
 *                                                          
 *    File   : sensor_ad7476_process.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __SENSOR_AD7476_PROCESS_H__
#define __SENSOR_AD7476_PROCESS_H__

#include "datablk_mgr.h"
#include "process_ids.h"
#include "datablk_processor.h"

#include "sensor_ad7476_config.h"

/* These should be add to process_ids.h, replace AD7476 with the sensor name
   for example in case of Accelerometer sensor, replace AD7476_ISR_PID with
   ACCEL_ISR_PID
*/
#define AD7476_ISR_PID                ((process_id_t)241)
#define AD7476_AI_PID                 ((process_id_t)242)
#define AD7476_DATASAVE_PID           ((process_id_t)243)
#define AD7476_LIVESTREAM_PID         ((process_id_t)244)

#define AD7476_NUM_CHANNELS           (SENSOR_AD7476_CHANNELS)     ///< Sensor AD7476 channel count
#define AD7476_DATA_SIZE_BYTES        (sizeof(int16_t))
#define AD7476_FRAME_SIZE_PER_CHANNEL ((( (2*(SENSOR_AD7476_RATE_HZ_MAX) + 999)/1000 ) + 1) & ~1)   ///< 10ms of data @ 1666Hz sample rate
#define AD7476_FRAME_SIZE             ( (AD7476_FRAME_SIZE_PER_CHANNEL) * (AD7476_NUM_CHANNELS) )
#define AD7476_NUM_DATA_BLOCKS        ( (SENSOR_AD7476_MEMSIZE_MAX) / ((AD7476_FRAME_SIZE)*(AD7476_DATA_SIZE_BYTES)+32) )

extern uint8_t               ad7476_data_blocks[SENSOR_AD7476_MEMSIZE_MAX] ;
extern QAI_DataBlockMgr_t    ad7476BuffDataBlkMgr;
extern QueueHandle_t         ad7476_dbp_thread_q;

#endif /* __SENSOR_AD7476_PROCESS_H__ */