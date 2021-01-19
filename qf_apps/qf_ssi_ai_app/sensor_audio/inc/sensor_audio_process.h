/** @file sensor_audio_process.h */

/*==========================================================
 *
 *-  Copyright Notice  -------------------------------------
 *                                                          
 *    Licensed Materials - Property of QuickLogic Corp.     
 *    Copyright (C) 2019 QuickLogic Corporation             
 *    All rights reserved                                   
 *    Use, duplication, or disclosure restricted            
 *                                                          
 *    File   : sensor_audio_process.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __SENSOR_AUDIO_PROCESS_H__
#define __SENSOR_AUDIO_PROCESS_H__

#include "datablk_mgr.h"
#include "process_ids.h"
#include "datablk_processor.h"

#include "sensor_audio_config.h"

/* These should be add to process_ids.h, replace AUDIO with the sensor name
   for example in case of Accelerometer sensor, replace AUDIO_ISR_PID with
   ACCEL_ISR_PID
*/
#define AUDIO_ISR_PID                ((process_id_t)21)
#define AUDIO_AI_PID                 ((process_id_t)22)
#define AUDIO_DATASAVE_PID           ((process_id_t)23)
#define AUDIO_LIVESTREAM_PID         ((process_id_t)24)

#define AUDIO_NUM_CHANNELS           (SENSOR_AUDIO_CHANNELS)     ///< Sensor AUDIO channel count
#define AUDIO_DATA_SIZE_BYTES        (sizeof(int16_t))
#define AUDIO_FRAME_SIZE_PER_CHANNEL ((( (15*(SENSOR_AUDIO_RATE_HZ_MAX) + 999)/1000 ) + 1) & ~1)   ///< 10ms of data @ 1666Hz sample rate
#define AUDIO_FRAME_SIZE             ( (AUDIO_FRAME_SIZE_PER_CHANNEL) * (AUDIO_NUM_CHANNELS) )
#define AUDIO_NUM_DATA_BLOCKS        ( (SENSOR_AUDIO_MEMSIZE_MAX) / ((AUDIO_FRAME_SIZE)*(AUDIO_DATA_SIZE_BYTES)+32) )

extern uint8_t               audio_data_blocks[SENSOR_AUDIO_MEMSIZE_MAX] ;
extern QAI_DataBlockMgr_t    audioBuffDataBlkMgr;
extern QueueHandle_t         audio_dbp_thread_q;

extern void audio_block_processor(void);

#endif /* __SENSOR_AUDIO_PROCESS_H__ */
