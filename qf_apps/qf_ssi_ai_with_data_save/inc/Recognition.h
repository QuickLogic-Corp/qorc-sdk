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

#ifndef __RECOGNITION_TASK_H__
#define __RECOGNITION_TASK_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "common.h"
#include "test_types.h"

#include "ble_collection_defs.h"

#define SENSIML_RECO_MSGQ_WAIT_TIME	portMAX_DELAY

typedef enum
{
	RECOG_CMD_RECOGNIZE_STOP			=20,
	RECOG_CMD_RECOGNIZE_START		=21,
	RECOG_CMD_RECOGNIZE_START_W_FV   =22,
    RECOG_CMD_RECOGNIZE_START_W_S    =23,
    
    
	RECOG_CMD_NEW_MOTION_DATA	    =30,
	RECOG_CMD_NEW_MOTION_DATA_BATCH	=31,
    RECOG_CMD_NEW_AUDIO_DATA	        =32,
    RECOG_CMD_NEW_ADC_DATA	        =33,
} reco_cmd_t;



typedef enum
{
    RECOG_STATE_IDLE = -1,
	RECOG_STATE_AWAITING_CMD = 0,
	RECOG_STATE_RUN,
	RECOG_STATE_RUN_W_FV,
	RECOG_STATE_RUN_W_S
} recognition_state_t;


extern signed portBASE_TYPE StartRtosTaskRecognition(void);
extern recognition_state_t GetRecognitionCurrentState(void);

struct sensor_data;

void recog_data( struct sensor_data *pDSI );
void RecognitionMotion_Batch_DataReadyMsg(void);
void RecognitionMotion_MoreData( const ble_accel_gyro_t *pData);
void recognition_startstop( reco_cmd_t command );
void *get_sensor_recog_status(void);
extern void recog_data_using_dbp(signed short *data_batch, int batch_sz, uint8_t num_sensors, uint32_t sensor_id);

#endif //#define __RECOGNITION_TASK_H__