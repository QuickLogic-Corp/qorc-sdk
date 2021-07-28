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

#ifndef APP_CONFIG_H_INCLUDED
#define APP_CONFIG_H_INCLUDED


/*######################## FIRMAWARE MODE SETTINGS  ################################*/


#define S3AI_FIRMWARE_IS_COLLECTION  1		/* Enable sensor data collection       */
#define S3AI_FIRMWARE_IS_RECOGNITION 0		/* Enable knowledgepack recognition    */


/*######################## SENSOR CONFIGURATION SETTINGS  ################################*/


/* Settings for selecting either Audio or an I2C sensor, Enable only one of these mode */
#define SSI_SENSOR_SELECT_SSSS     1 // 1 => Select SSSS sensor data for live-streaming of recognition modes
#define SSI_SENSOR_SELECT_AUDIO    0 // 1 => Select Audio data for live-streaming or recognition modes


/*######################## DATA CAPTURE METHOD  ################################*/


#define S3AI_FIRMWARE_LIVESTREAM 1   /* Enable livestream via SSI Interface  (supports sensor or recognition)  */
#define S3AI_FIRMWARE_DATASAVE 0     /* Enable SD card for collection        (supports sensor and recognition) */



/*######################## OPTIONS  ################################*/


#define SENSOR_COMMS_KNOWN_PATTERN 0 // 1 => replace sensor data with a known sawtooth pattern
#define SSI_OUTPUT_FEATURE_VECTOR 0  // 1 => Include feature vector as part of output



/*######################## ADVANCED SETTINGS  ################################*/


/***************    DATA COLLECTION SETTINGS   *****************/

#if (S3AI_FIRMWARE_DATASAVE)
#define DATA_CAPTURE_BUFFER_SIZE_K_BYTES   100
#endif

/***************    AUTOCONFIGURED DATA COLLECTION SETTINGS   *****************/

#define SENSOR_AUDIO_LIVESTREAM_ENABLED   (S3AI_FIRMWARE_IS_COLLECTION)  && (S3AI_FIRMWARE_LIVESTREAM) &&  (SSI_SENSOR_SELECT_AUDIO)
#define SENSOR_AUDIO_DATASAVE_ENABLED     (S3AI_FIRMWARE_DATASAVE) && (SSI_SENSOR_SELECT_AUDIO)

#define SENSOR_SSSS_LIVESTREAM_ENABLED     (S3AI_FIRMWARE_IS_COLLECTION) && (S3AI_FIRMWARE_LIVESTREAM) && (SSI_SENSOR_SELECT_SSSS)
#define SENSOR_SSSS_DATASAVE_ENABLED       (S3AI_FIRMWARE_DATASAVE) && (SSI_SENSOR_SELECT_SSSS)


/***************  AUTOCONFIGURED RECOGNITION SETTINGS   *****************/

#define SENSOR_AUDIO_RECOG_ENABLED    (S3AI_FIRMWARE_IS_RECOGNITION) && (SSI_SENSOR_SELECT_AUDIO)
#define SENSOR_SSSS_RECOG_ENABLED     (S3AI_FIRMWARE_IS_RECOGNITION) && (SSI_SENSOR_SELECT_SSSS)
#define DATASAVE_RECOGNITION_RESULTS  (S3AI_FIRMWARE_IS_RECOGNITION) && (S3AI_FIRMWARE_DATASAVE)



// Toggle GPIO whenever a datablock buffer is dispatched to the UART
// Datablocks are dispatched every (SENSOR_SSSS_LATENCY) ms. Default is 20ms or 50Hz
#define SENSOR_SSSS_RATE_DEBUG_GPIO  (1)    // Set to 1 to toggle configured GPIO


typedef struct st_fw_global_config
{
	int ssi_sensor_select_audio ;
	int sensor_audio_livestream_enabled;
	int sensor_audio_recog_enabled;

	int ssi_sensor_select_ssss  ;
	int sensor_ssss_livestream_enabled;
	int sensor_ssss_recog_enabled;
} fw_global_config_t;



/*######################## VALIDATION  ################################*/ 

#if (SSI_SENSOR_SELECT_AUDIO == 1) && (SSI_SENSOR_SELECT_SSSS == 1)
#error "Enable only one of the sensors SSI_SENSOR_SELECT_AUDIO or SSI_SENSOR_SELECT_SSSS"
#endif

#if (S3AI_FIRMWARE_IS_COLLECTION == 1) && (S3AI_FIRMWARE_IS_RECOGNITION == 1) && (S3AI_FIRMWARE_LIVESTREAM==1)
#error "Enable only one of  S3AI_FIRMWARE_IS_COLLECTION and S3AI_FIRMWARE_IS_RECOGNITION for S3AI_FIRMWARE_LIVESTREAM"
#endif

#endif
