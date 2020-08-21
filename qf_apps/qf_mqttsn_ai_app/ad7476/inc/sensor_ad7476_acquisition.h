/** @file sensor_ad7476_acquisition.h */

/*==========================================================
 *
 *-  Copyright Notice  -------------------------------------
 *                                                          
 *    Licensed Materials - Property of QuickLogic Corp.     
 *    Copyright (C) 2019 QuickLogic Corporation             
 *    All rights reserved                                   
 *    Use, duplication, or disclosure restricted            
 *                                                          
 *    File   : sensor_ad7476_acquisition.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __SENSOR_AD7476_ACQUISITION_H__
#define __SENSOR_AD7476_ACQUISITION_H__

#include "Fw_global_config.h"

#include "datablk_mgr.h"

/* Sensor AD7476 capture ISR */
#define AD7476_ISR_EVENT_NO_BUFFER  (1)   ///< error getting a new datablock buffer

extern QAI_DataBlock_t *pad7476_data_block_prev;
extern int              ad7476_samples_collected;
extern outQ_processor_t ad7476_isr_outq_processor;

extern void ad7476_set_first_data_block();
extern void ad7476_acquisition_read_callback(void);
extern int ad7476_sensordata_buffer_ready(void);

#define SENSORS_AD7476_ACQUISITION_READ_FROM_FILE (0)
#if (SENSORS_AD7476_ACQUISITION_READ_FROM_FILE)
extern void ad7476_dataTimerStart(void);
extern void ad7476_dataTimerStop(void);
#endif

#endif /* __SENSOR_FOO_ACQUISITION_H__ */