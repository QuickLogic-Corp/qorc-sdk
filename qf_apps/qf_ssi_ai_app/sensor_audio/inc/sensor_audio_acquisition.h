/** @file sensor_audio_acquisition.h */

/*==========================================================
 *
 *-  Copyright Notice  -------------------------------------
 *                                                          
 *    Licensed Materials - Property of QuickLogic Corp.     
 *    Copyright (C) 2019 QuickLogic Corporation             
 *    All rights reserved                                   
 *    Use, duplication, or disclosure restricted            
 *                                                          
 *    File   : sensor_audio_acquisition.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __SENSOR_AUDIO_ACQUISITION_H__
#define __SENSOR_AUDIO_ACQUISITION_H__

#include "Fw_global_config.h"

#include "datablk_mgr.h"

extern QAI_DataBlock_t *paudio_data_block_prev;
extern int              audio_samples_collected;
extern outQ_processor_t audio_isr_outq_processor;

extern void audio_set_first_data_block();
extern void audio_acquisition_read_callback(void);
extern int audio_sensordata_buffer_ready(void);

#define SENSORS_AUDIO_ACQUISITION_READ_FROM_FILE (0)
#if (SENSORS_AUDIO_ACQUISITION_READ_FROM_FILE)
extern void audio_dataTimerStart(void);
extern void audio_dataTimerStop(void);
#endif

#endif /* __SENSOR_FOO_ACQUISITION_H__ */