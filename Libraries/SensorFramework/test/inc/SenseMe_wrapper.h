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

/*==========================================================
 *                                                          
 *    File   : SenseMe_wrapper.h
 *    Purpose: Macros and Structures for SenseMe library support
 *                                                          
 *=========================================================*/

#ifndef __SENSEME_WRAPPER_H_
#define __SENSEME_WRAPPER_H_

#include "test_types.h"

#define  PEDOMETER     0x1
#define  ADV_PEDOMETER 0x2
#define  IN_ACTIVE     0x4  /* Long Sleep  */
#define  B2C_GEST      0x8
#define  DOUBLETAP     0x10  /* TBD */



/*!	\enum SenseMe_contexts_type
	\brief SenseMe Contexts types
*/
typedef enum {
	UNKNOWN = 0,
	NOT_ON_PERSON,
	WALKING,
	RUNNING,
	JOGGING,
	INVEHICLE,
	UNKNOWN_POSTURE,
	IN_POCKET,
	UNKNOWN_MOTION,
	STATIONARY,
	OFF_VEHICLE,
	ON_BIKE,
	LAST_CONTEXT
}SenseMe_contexts_type;


/*!	\enum SenseMe_sports_type
	\brief SenseMe sports type
*/
typedef enum
{
	SP_RUNNING=1,
	SP_BIKING,
	SP_SWIMMING
}SenseMe_sports_type;



/*!	\enum SenseMe_output_type
	\brief SenseMe output type definition
*/
typedef enum
{
	OUTPUT_INTEGER,												// Integer type
	OUTPUT_FLOAT												// Float Type
}SenseMe_output_type;

/*! \struct SenseMe_cmd_args SenseMe_wrapper.h "inc/SenseMe_wrapper.h"
 * 	\brief SenseMe command params structure
*/
typedef struct
{
	UINT32_t sensor_ID;											/*!< unique Sensor ID */
	UINT32_t value;												/*!< command value */
}SenseMe_cmd_args;


/*! \struct Sensor_raw_data SenseMe_wrapper.h "inc/SenseMe_wrapper.h"
 * 	\brief Structure to pass Sensor Raw data to SenseMe
*/
typedef struct
{
	UINT32_t sensor_ID;											/*!< Unique Sensor ID information */
	UINT32_t timestamp;											/*!< Timestamp 32-bit */
	UINT32_t sensor_rate;										/*!< Sensor data rate */
	UINT32_t data[5];											/*!< Sensor raw data */
}Sensor_raw_data;

#ifdef OLDOUTPUT
/*! \union event_ds SenseMe_wrapper.h "inc/SenseMe_wrapper.h"
 * 	\brief Union to output sensor data for float/integer
*/
typedef union {
	UINT32_t iEvent_data[8];									/*!< integer event data associated with Events */
	FLOAT_t  fEvent_data[8];									/*!< float event data associated with Events */
}event_ds;


/*! \struct Sensor_SenseMe_output SenseMe_wrapper.h "inc/SenseMe_wrapper.h"
 * 	\brief Structure to receive SenseMe output
*/
typedef struct
{
	UINT8_t  event_ID;											/*!< 0 = No events */
	UINT8_t	 output_type;										/*!< 0 = integer, 1 = float */
	UINT32_t event_timestamp;									/*!< Timestamp when event occurred */
	event_ds event_data;										/*!< Event data */
} Sensor_SenseMe_output;
#else

/*! \struct SenseMe_output_PedConAdv SenseMe_wrapper.h "inc/SenseMe_wrapper.h"
 * 	\brief Structure to receive Pedometer, Context and Advanced pedometer output
*/
typedef struct
{
	UINT32_t event_timestamp;									/*!< Timestamp when event occurred */
	UINT32_t walkingSteps;										/*!< Number of steps counted for walking */
	UINT32_t runningSteps;										/*!< Number of steps counted for running */
	//UINT32_t steplikeCount;									    /*!< Number of steps counted for step like activities */
	UINT32_t calories;											/*!< Number of calories burned */
	UINT32_t distanceTravelled;									/*!< Distance travelled in meters */
	UINT16_t  speed;											/*!< Speed in 1000 times m/s*/
	UINT8_t  contextDetected;									/*!< Context detected */
	UINT8_t	 inactiveDetected;								    /*!< long sitting activity detected */
}SenseMe_output_PedConAdv;

/*! \struct SenseMe_output_gesture SenseMe_wrapper.h "inc/SenseMe_wrapper.h"
 * 	\brief Structure to receive gesture output from SenseMe library
*/
typedef struct
{
	UINT32_t event_timestamp;									/*!< Timestamp when event occurred */
	UINT8_t  gestureDetected;									/*!< Gesture detected */
}SenseMe_output_gesture;

/*! \struct SenseMe_output_sportsmode SenseMe_wrapper.h "inc/SenseMe_wrapper.h"
 * 	\brief Structure to receive sports mode output from SenseMe library
*/
typedef struct
{
	UINT32_t event_timestamp;									/*!< Timestamp when event occurred */
	UINT32_t event_data[3];										/*!< event data */
	UINT8_t	 event_type;											/*!< Activity detected */
}SenseMe_output_sportsmode;

/*! \struct SenseMe_output_sleepdetect SenseMe_wrapper.h "inc/SenseMe_wrapper.h"
 * 	\brief Structure to receive sleep detection output from SenseMe library
*/
#if 0
typedef struct
{
	UINT32_t event_size;											/*!< Number of valid events in packet */
	UINT32_t event_start_time;								/*!< start time of sleep event data */
	UINT8_t	 event_data[60];									/*!< Sleep level buffer 0 - deep, 1 - moderate, 2 - light, 3 - awake*/
}SenseMe_output_sleepdetect;
#else
typedef struct
{
	UINT8_t	 acti_data;												/*!< Sleep actigraph data of every minute*/
}SenseMe_output_sleepacti;
#endif
/*! \struct SenseMe_output_altitudedir SenseMe_wrapper.h "inc/SenseMe_wrapper.h"
 * 	\brief Structure to receive altitude direction output from SenseMe library
*/
typedef struct
{
	INT32_t dir_data;											/*!< altitude direction 1-unknown, 2-up, 3-down */
}SenseMe_output_altitudedir;


/*! \struct Sensor_SenseMe_output SenseMe_wrapper.h "inc/SenseMe_wrapper.h"
 * 	\brief Structure to receive SenseMe output
*/
typedef struct
{
	UINT8_t  event_ID;											/*!< 0 = No events */
	void     *event_data;										/*!< Event data information*/
} Sensor_SenseMe_output;


#endif

void PCGAlgo_Test(float AccelData[]);



//Pedometer and contest algo
void alg_pedcon (float AccelData[], SenseMe_output_PedConAdv* outputData);

//Gesture algo
int alg_b2see (float AccelData[]);


#endif
