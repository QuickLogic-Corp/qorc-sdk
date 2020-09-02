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

/*
* Recognition.c
*
*  This Example is similar to Intel Arduino 101 Example DrawingInTheAir.
*  But uses different vectors and measures.
*
*/
#include "Fw_global_config.h"

/* define the following macro to 1, to indicate that datablock processor 
 * functions are directly invoking the SensiML recognition APIs
 * (sml_recognition_run_batch and/or sml_recognition_run_single)
 *
 * define the following macro to 0, to use the recog_data() APIs
 * for recognition
 *
 */
#define USE_DBP_FOR_SENSIML_RECOG (1) // indicate whether to use datablock processor

#if S3AI_FIRMWARE_IS_RECOGNITION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//#include "eoss3_dev.h"
//#include "s3x_clock_hal.h"
//#include "s3x_clock.h"
#include "RtosTask.h"
//#include "QL_Trace.h"
//#include "eoss3_hal_fpga_adc_api.h"
#include "dcl_commands.h"
#include "DataCollection.h"
//#include "eoss3_hal_uart.h"
#include "dbg_uart.h"
#include "qlsh_commands.h"
#include "Recognition.h"
#include "math.h"
//#include "ql_bleTask.h"
//#include "ql_adcTask.h"
#include "iop_messages.h"
//#include "ble_collection_defs.h"
#include "Sensor_Attributes.h"
//#include "QL_FFE_SensorConfig.h"
#include "dma_buffer.h"            /* For getting audio buffer length, it should have come fron audio_config instead */
#include "sensor_config.h"
#include "sensor_live.h"

//FILL_USE_TEST_DATA
//#define SML_USE_TEST_DATA  // Define it to run classification over test data


#if    ( FFE_DRIVERS )
#include "FFE_AccelGyro.h"
#else
#define SENSIML_FFE_BATCH_DATA_SZ 6
#endif

//SensiML Includes
#include "kb.h"
#include "sml_output.h"
#include "sml_recognition_config.h"
#include "sml_recognition_run.h"
#include "circularBuffQueue.h"

#ifdef SML_USE_TEST_DATA
#include "testdata.h"
int td_index = 0;
#endif //SML_USE_TEST_DATA

#define SENSOR_DATA_BASE_SIZE            2     // every base senor data size is 2 bytes

//#define MAX_CLASS_BUFF_SIZE              14400 // 14400*SENSOR_DATA_BASE_SIZE = 28800 bytes to accomodate audio buffer which come in multiple of 240 bytes

#define MAX_CLASS_BUFF_SIZE              1440 // for SensorTile this is too large to fit. need modify the algo?

/* defines related to MIU processing */

#define MOTION_DATA_BATCH_PROCESSING     0    // Define it to '1' for motion data batch processing

#define MAX_MOTION_DATA_BUF             1000  // maximum buffer of motion data. Note this needs to adjust based on sampling frequency

#define RECONGNITION_DATA_BATCH_SIZE    40

#define SENSIML_RECO_QUEUE_LENGTH       40   //(MAX_MOTION_DATA_BUF/RECONGNITION_DATA_BATCH_SIZE) + some buffer

/* Audio batch size */
#define AUDIO_BATCH_FRAME_COUNT        (int)((MAX_CLASS_BUFF_SIZE*SENSOR_DATA_BASE_SIZE)/(FRAME_SIZE*2)) // batch of 30 audio frame data


struct recog_task_vars {
    xTaskHandle   task_handle;
    QueueHandle_t cmd_q;
    recognition_state_t currentState;
    SensorEnableStatus sensor_status;   // Keep the current status of the sensor which are configured currently for classification

    /* where next motion data will be placed */
    int motionData_indx;         // Index pointer to motion data

    /* how large is each "batch of audio samples" (in sample counts) */
    int audio_batchSize;
    int audio_batchCount;

    /* when batching the IMU - how big should the batch be? */
    int imu_batchSize;

    /* The IMU buffer size is fixed (via a #define)
     * The IMU batch size is based on time not samples.
     *
     * Thus the batch may not evenly divide the buffer size.
     * But - we must still "wrap" the imu buffer based on the
     * batch size, not the buffer size
     *
     * Thus we calculate the wrap point dynamically
     * and store that wrap point here for later use.
     */
    int imu_batchCount_wrap;

    /* how many adc samples are in a batch */
    int adc_batchSize;

    /* when writing data, the ADC should wrap when pointer goes past here */
    uint8_t *adc_wrapPointer;


    /* how many channels are enabled for the ADC? */
    int adc_channel_count;

    /* used to determine if the recognition task is running or waiting */
    int task_ready;
    /* this keeps track of how many data packets where dropped */
    /* IF a drop occurs, then the recogntion task is taking too much time */

    /* These two member variables represent the "recogntion work load"
     *
     * Each new message is a "unit of work" that must be calculated.
     * As new data arrives, a new entry is placed in the work queue.
     *
     * First question/concern:
     * IF the recogntion task is "too busy" and it overflows
     *    Then - DATA LOSS will appear (see: data_loss_count)
     *
     * Second question/concern:
     * How to determine if the recogntion task is "keeping up" with the data?
     *
     * We can measure that by looking at the depth of the work queue.
     * This is very simplistic - "what is the maximum" backlog.
     */
    int data_loss_count;
    int max_queue_backlog;
    // Pointer to the buffer used for storing lasted sensor sample data
    uint8_t *currBuffPtr;
};

struct recog_task_vars recog_task_vars;

/* these buffers are *LARGE* so we keep them out of the vars struct
* makes things work nicer in the debugger watch window
*/
static uint8_t classBuffer[MAX_CLASS_BUFF_SIZE*SENSOR_DATA_BASE_SIZE];       // buffer to store classification data before processing
#define CLASS_BUFFER_END  (&(classBuffer[MAX_CLASS_BUFF_SIZE*SENSOR_DATA_BASE_SIZE]))
static ble_accel_gyro_t motion_buffer[MAX_MOTION_DATA_BUF];  // buffer for reconstructing motion data

/*
* To prevent sending of messages when Recognition is not ready. Else msgQ will overflow.
*/
int CheckRecognitionReady(void)
{
	return recog_task_vars.task_ready;
}

/*
* Return Recog sensor status.
*/
SensorEnableStatus GetRecogSensorStatus(void)
{
    return recog_task_vars.sensor_status;
}

/*
* @fn      GetRecognitionCurrentState
* @brief   Return the current state of recognition task status such as
*          recognition state
* @param   None
* @return  recognition_state_t
*/
recognition_state_t GetRecognitionCurrentState(void)
{
	return recog_task_vars.currentState;
}


/*
* @fn      getBuffIndex
* @brief   Extracts the data buffer Index from queue pointer.
* @param   msg - pointer to message queue.
* @return  buffer index
*/
static intptr_t getBuffIndex(struct xQ_Packet *msg)
{
    // Get the index of the message buffer
    intptr_t buffIndx = msg->ucData[0];
    buffIndx = buffIndx | (msg->ucData[1] << 8);
    buffIndx = buffIndx | (msg->ucData[2] << 16);
    buffIndx = buffIndx | (msg->ucData[3] << 24);
    return buffIndx;
}

/*
* @fn      setBuffIndex
* @brief   Set the buffer index in queue message.
* @param   msg - pointer to message queue.
* @return  buffer index
*/
void setBuffIndex(struct xQ_Packet *msg, intptr_t valuetopass)
{
	msg->ucData[0] = valuetopass & 0xFF;   // index to motion data
    msg->ucData[1] = (valuetopass & (0xFF << 8)) >> 8 ;
    msg->ucData[2] = (valuetopass & (0xFF << 16)) >> 16 ;
    msg->ucData[3] = (valuetopass & (uint32_t)(0xFF << 24)) >> 24 ;
}

/*
* @fn      sendRecognitionDataReadyMsg
* @brief   Sends the RecognitionDataReady message to the recongnition task
* @param   command - Command ID
* @param   valuetopass - a parameter for the message
* @return  None
*/
static void sendRecognitionMsg(reco_cmd_t command, intptr_t valuetopass)
{
	struct xQ_Packet Msg;
	if (!CheckRecognitionReady())
	{
		return;
	}

	Msg.ucCommand = command;

    setBuffIndex(&Msg, valuetopass);
    //printf("\n --- put =%x", bufferIndx);
    if(getBuffIndex(&Msg) != valuetopass)
    {
        dbg_fatal_error("recogntion task internal error\n");
    }

	if (xQueueSend(recog_task_vars.cmd_q, &Msg, 0) != pdPASS)
	{
		printf("Error- Sending Accel Msg to Reco Task \n");
	}
	return;
}

/*
 * Send a "NEW DATA" message to the Recognition task.
 *
 * Also track the amount of work, and backlog
 * to determine if the recogntiion task is keeping up or not
 */
static void sendNewDataMessage( reco_cmd_t command, intptr_t valuetopass )
{
    UBaseType_t space;
    UBaseType_t used;

    /* how much space is in the queue */
    space = uxQueueSpacesAvailable( recog_task_vars.cmd_q );

    /* how much back log do we have */
    used = SENSIML_RECO_QUEUE_LENGTH - space;

    /* Track max level */
    if( used > recog_task_vars.max_queue_backlog ){
        recog_task_vars.max_queue_backlog = used;
    }

    /* did we overflow (too much work?) */
    if( space  == 0 ){
        /* recognition cannot keep up */
        recog_task_vars.data_loss_count += 1;
    } else {
        sendRecognitionMsg( command, valuetopass );
    }
}

/*
 * calculate the size of the IMU wrap in the IMU buffer.
 */
static void calculate_imu_wrap_point(void)
{
    int tmp;

    recog_task_vars.imu_batchCount_wrap = 0;
    recog_task_vars.imu_batchSize = 0;

    /* not enabled ?? */
    if( recog_task_vars.sensor_status.isIMUEnabled || recog_task_vars.sensor_status.isAccelEnabled || recog_task_vars.sensor_status.isGyroEnabled ){
            /* at least one is enabled */
    } else {
        /* not enabled, bye */
        return;
    }

    /* calculate the IMU batch size */

    /* we need the sample rate */
    tmp = 0;
    if( !imu_config.accel.enabled ){
        tmp = imu_config.accel.rate_hz;
    }
    if( imu_config.gyro.enabled ){
        if( tmp == 0 ){
            tmp = imu_config.gyro.rate_hz;
        }
        if( tmp!= imu_config.gyro.rate_hz ){
            dbg_fatal_error("recog-gyro-accel-different-rates");
        }
    }

    /* we don't do this sensor yet */
    if( imu_config.mag.enabled ){
        dbg_fatal_error("recog-no-mag-yet\n");
    }

    if( tmp == 0 ){
        /* we should have a rate in hz? */
        dbg_fatal_error("recog-imu-no-rate\n");
    }

    /* we want to feed recognition IMU data every 100 milliseconds */
    tmp = tmp / 10;
    /* thus that is our batch size */
    recog_task_vars.imu_batchSize = tmp;

    /*
     * Next, calculate when do we 'wrap' the motion buffer.
     */

    /* how many batches fit in one buffer ? */
    tmp = MAX_MOTION_DATA_BUF / tmp;
    if( tmp < 4 ){
        /* we should have at least 4 batches in the buffer */
        dbg_fatal_error("recog-imu-buff-too-small\n");
    }

    /* convert BATCHES to SAMPLES */
    tmp = tmp * recog_task_vars.imu_batchSize;

    /* thus we have from 0 to "tmp" samples in our motion buffer, then we wrap */
    recog_task_vars.imu_batchCount_wrap = tmp;

}


static void calculate_adc_wrap_point( const struct sensor_data *pSensorData )
{
    int tmp;

    /* Sadly, we cannot calculate the adc wrap point
     * until we *KNOW* how many bytes per transfer
     * the ADC is going to give us.
     *
     * That size is dynamic ...
     *
     * So instead we wait for the *FIRST*
     * adc data to arrive then calculate it.
     *
     * For now, we just reset the vars
     */
    recog_task_vars.adc_batchSize = 0;
    recog_task_vars.adc_wrapPointer = NULL;
    recog_task_vars.adc_channel_count = 0;

    /* is this the startup? */
    if( pSensorData == NULL ){
        return;
    } else {
        /* fist data transfer */
    }

    recog_task_vars.adc_channel_count = pSensorData->bytes_per_reading / sizeof(int16_t);


    /* start with the size of the class buffer */
    tmp = sizeof( classBuffer );

    /* Determine the number of frames that fit */
    tmp = tmp / pSensorData->n_bytes;

    /* turn this back into bytes */
    tmp = tmp * pSensorData->n_bytes;

    recog_task_vars.adc_wrapPointer = &(classBuffer[ tmp ]);
}


/*
* @fn      initRecognition
* @brief   Initilaizes the classification parameters and updates task status
* @param   None.
* @return  None
*/
static void init_recognition_state(recognition_state_t new_state)
{
    if( new_state == RECOG_STATE_AWAITING_CMD ){
        sensor_clear( &recog_task_vars.sensor_status );
        return;
    }

    //Initialize Model
	kb_model_init();

    // FIXME with this configuration, regonition will add default sensor based on
    // recognition_config which is generated by sml tool. This preconofiged sensor
    // could be conflict with cli sensor add (A,G,G+A etc.). To solve this conflict
    // we could clear sensor (cli or host) first, add sensor late as needed.
    sensor_config_apply_sequence(recognition_config, &recog_task_vars.sensor_status);
    recog_task_vars.currBuffPtr  = classBuffer;

    calculate_imu_wrap_point();

    calculate_adc_wrap_point( NULL );

    /* AUDIO batch size is currently hard coded and fixed */
    /* Make sure the class buffer size of multiple of audio frame size */
#if ((MAX_CLASS_BUFF_SIZE*SENSOR_DATA_BASE_SIZE)%(FRAME_SIZE*2)) != 0
#error "Class buffer size must be multiple of audio frame size"
#endif
    /* do this *LAST */
    recog_task_vars.currentState = new_state;

    /* tell sensors to RUN */
    sensors_all_startstop( 1 );
}

/*
* @fn      processMotionData
* @brief   Proesses the new motion/IMU data received for from sensor and copies
*          on the internal queue and send message for classifcation on
*          recongnition task.
* @param   None.
* @return  None
*/
static void processMotionData(ble_accel_gyro_t *motionData )
{
    int tmp;
    // Fill motion data buffer
    motion_buffer[recog_task_vars.motionData_indx].accel.x = motionData->accel.x;
    motion_buffer[recog_task_vars.motionData_indx].accel.y = motionData->accel.y;
    motion_buffer[recog_task_vars.motionData_indx].accel.z = motionData->accel.z;

    /* Note use batching if the sample rate is higher than 416Hz,
    otherwise there would be dansger of Recognition task queue overflow */

    // send message to the recognition task queue for claissification
#if !defined(MOTION_DATA_BATCH_PROCESSING)
#error Must be 0 or 1
#endif

    // increment motion data buffer pointer

#if (MOTION_DATA_BATCH_PROCESSING == 0)
    sendNewDataMessage(RECOG_CMD_NEW_MOTION_DATA, recog_task_vars.motionData_indx);
    recog_task_vars.motionData_indx = (recog_task_vars.motionData_indx+1) % recog_task_vars.imu_batchCount_wrap;
#else    //#if (MOTION_DATA_BATCH_PROCESSING)
    // Batch processing
    recog_task_vars.motionData_indx = (recog_task_vars.motionData_indx+1) % recog_task_vars.imu_batchCount_wrap;
    if(0 == (recog_task_vars.motionData_indx % recog_task_vars.imu_batchSize))
    {
        tmp = recog_task_vars.motionData_indx - recog_task_vars.imu_batchSize;
        tmp = tmp + recog_task_vars.imu_batchCount_wrap;
        tmp = tmp % recog_task_vars.imu_batchCount_wrap;
        sendNewDataMessage(RECOG_CMD_NEW_MOTION_DATA_BATCH, tmp);
    }
#endif


}

/*
 * Validate that this data pointer and byte count
 * are totally within the specified buffer.
 */
static void assert_ptr_within( const void *pBuf, size_t buf_size,
                               const void *pData, size_t nDataBytes)
{
    const void *pEndData;
    const void *pEndBuf;

    pEndBuf  = (const void *)(((const char *)(pBuf ))+buf_size);
    pEndData = (const void *)(((const char *)(pData))+nDataBytes);

    if( (pData >= pBuf) && (pEndData <= pEndBuf) ){
        /* ALL IS WELL */
        return;
    }
    /* we are dead :-( */
    dbg_str_ptr( "\nrecog-data-ptr", pData );
    dbg_str_ptr( "recog-data-end", pEndData );
    dbg_str_ptr( "recog-buf-ptr", pBuf );
    dbg_str_ptr( "recog-buf-end", pEndBuf );
    dbg_fatal_error("invalid-recog-buf-ptr\n");
}


/*
* @fn      processAudioData
* @brief   Proesses the new audio data received for from sensor and copies
*          on the internal queue and send message for classifcation on
*          recongnition task.
* @param   None.
* @return  None
*/
static void processAudioData(const struct sensor_data *audioData )
{
    uint8_t *pDataBytes;
    uint8_t *pNext;
    int tmp;

    /* audio frames must exactly this size for buffering reasons */
    if( (audioData->n_bytes != (FRAME_SIZE*2)) ||
        (audioData->bytes_per_reading != 2) ){
        dbg_fatal_error("invalid audio size\n");
    }

    // copy the audio data samples
    pDataBytes = recog_task_vars.currBuffPtr;
    assert_ptr_within(  (const void *)(&classBuffer[0]),
                         sizeof( classBuffer),
                         (const void *)(pDataBytes),
                         audioData->n_bytes );

    memcpy(  pDataBytes, audioData->vpData, audioData->n_bytes);
    //printf("\n --%x", currBuffPtr);
    tmp = audioData->n_bytes;
    tmp = tmp / audioData->bytes_per_reading;
    recog_task_vars.audio_batchSize = tmp;
    recog_task_vars.audio_batchCount++;

    /* get next location for buffer */
    pNext = pDataBytes + audioData->n_bytes;
    /* next data will go here */
    if( pNext < recog_task_vars.adc_wrapPointer ){
        recog_task_vars.currBuffPtr = pNext;
    } else {
        recog_task_vars.currBuffPtr = &classBuffer[0];
    }
#if 1
    sendNewDataMessage( RECOG_CMD_NEW_AUDIO_DATA, (intptr_t)pDataBytes );
#else
    if(recog_task_vars.audio_batchCount >= AUDIO_BATCH_FRAME_COUNT/2) // Half buffer is full send for classification
    {

        //printf("\n ***%x", currBuffPtr);
        /* Note use batching if the sample rate is higher than 416Hz,
        otherwise there would be dansger of Recognition task queue overflow */
        tmp = ((recog_task_vars.audio_batchCount -1)*(audioData->n_bytes));
        /* point at start */
        pDataBytes -= tmp;
        sendNewDataMessage(RECOG_CMD_NEW_AUDIO_DATA, (intptr_t)(pDataBytes) );
        recog_task_vars.audio_batchCount = 0;
    }
#endif
}

/*
* @fn      processADCData
* @brief   Proesses the new ADC data received for from sensor and copies
*          on the internal queue and send message for classifcation on
*          recongnition task.
* @param   None.
* @return  None
*/
static void processADCData(const struct sensor_data *pData )
{
    uint8_t *pDataBytes;
    int tmp;


    if( recog_task_vars.adc_wrapPointer == 0 ){
        /* We have not yet calculated our wrap point */
        /* we need to calculate it now */
        calculate_adc_wrap_point( pData );
    }


    assert_ptr_within(  (const void *)(&classBuffer[0]),
                         sizeof( classBuffer),
                         (const void *)(recog_task_vars.currBuffPtr),
                         pData->n_bytes );

    // copy the adc data samples
    // If the recognition task is not keeping up (too slow)
    // then data is lost and overwritten
    pDataBytes = recog_task_vars.currBuffPtr;
    memcpy( pDataBytes, pData->vpData, pData->n_bytes);

    /*
     * ADC task tries to make data appear every 100mSec
     * or when it's DMA buffer is full which ever comes first.
     */
#if 1
    tmp = pData->n_bytes;
    tmp = tmp / pData->bytes_per_reading;
    recog_task_vars.adc_batchSize = tmp;
    sendNewDataMessage(RECOG_CMD_NEW_ADC_DATA, (intptr_t)(recog_task_vars.currBuffPtr) );

#else
    recog_task_vars.adc_batchCount++;
    if(recog_task_vars.adc_batchCount >= recog_task_vars.adc_batchCount_wrap) // Half buffer is full send for classification
    {

        //printf("\n ***%x", currBuffPtr);
        /* Note use batching if the sample rate is higher than 416Hz,
        otherwise there would be dansger of Recognition task queue overflow */
        //printf("\n addr=%x\n", ((int)currBuffPtr - (adcBatchCount -1)*pData->n_bytes));
        tmp = ((recog_task_vars.adc_batchCount -1)*(pData->n_bytes));
        /* go back to the start of this chunk */
        pDataBytes -= tmp;

        recog_task_vars.adc_batchSize = tmp / pData->bytes_per_reading;
        sendNewDataMessage( RECOG_CMD_NEW_ADC_DATA, pDataBytes );
        recog_task_vars.adc_batchCount = 0;
    }
#endif
    recog_task_vars.currBuffPtr += pData->n_bytes;
    if( recog_task_vars.currBuffPtr >= recog_task_vars.adc_wrapPointer ){
        recog_task_vars.currBuffPtr = &classBuffer[0];
    }
}

/*
* @fn      recog_data
* @brief   .
* @param   None.
* @return  None
*/
void recog_data( struct sensor_data *pSensorData )
{
    int ignore;

    /* if we are not recognizing .. then ignore */
    ignore = 1;
    switch( recog_task_vars.currentState ){
    default:
        ignore = 1;
        break;
    case RECOG_STATE_RUN:
    case RECOG_STATE_RUN_W_FV:
        ignore = 0;
        break;
    }

    // Check if there is imu sensor (A,G,A_G_SEP,A_G_COM) added
    if ( !is_sensor_active(pSensorData->sensor_id, IMU_RECOGNITION) )
    {
        ignore = 1;
    }

    if( ignore ){
        return;
    }

    switch( pSensorData->sensor_id )
    {
    case SENSOR_ENG_VALUE_ACCEL_GYRO:
        // Process motion data
        if(recog_task_vars.sensor_status.isIMUEnabled){
            if( pSensorData->n_bytes != (6 * sizeof(int16_t)) ){
                /* bufering code assumes 1 sample per per, and both ACCEL & GYRO */
                dbg_fatal_error("recog-imu-bad-size\n");
            }
            processMotionData( (ble_accel_gyro_t *)(pSensorData->vpData));
        }
        break;

    case SENSOR_ENG_VALUE_GYRO:
        /* model with only GYRO */
        break;

    case SENSOR_ENG_VALUE_ACCEL:
        if(recog_task_vars.sensor_status.isIMUEnabled){
            if( pSensorData->n_bytes != (3 * sizeof(int16_t)) ){
                /* bufering code assumes 1 sample per per, and both ACCEL & GYRO */
                dbg_fatal_error("recog-imu-bad-size\n");
            }
            processMotionData( (ble_accel_gyro_t *)(pSensorData->vpData));
        }
        break;

    case SENSOR_ENG_VALUE_MAGNETOMETER:
        /* future... */
        break;

    case SENSOR_AUDIO:
        if( recog_task_vars.sensor_status.isAudioEnabled)
            processAudioData(pSensorData);
        break;

    case SENSOR_ADC_LTC_1859_MAYHEW:
        if( recog_task_vars.sensor_status.isADCEnabled)
            processADCData(pSensorData);
        break;

    default:
        dbg_str_int("rec-unknown-sensor", pSensorData->sensor_id );
        break;
    }
}

void recog_data_using_dbp(signed short *data_batch, int batch_sz, uint8_t num_sensors, uint32_t sensor_id)
{
#if (USE_DBP_FOR_SENSIML_RECOG)
  sml_recognition_run_batch(data_batch, batch_sz, num_sensors, sensor_id);
#endif /* USE_DBP_FOR_SENSIML_RECOG */
}

/*
* Main Recognition task for detecting characters
*/
void RecognitionTaskHandler(void *pParameter)
{
    int idx;
    int16_t *pSensorData16;
	BaseType_t qret;
	unsigned int TaskStop = 0;
	struct xQ_Packet Msg;
    uint32_t sensor_id;
    int batch_size;
    int n_sensors;

	//init training variables
    wait_ffe_fpga_load();

    /* We *DO*NOT* wait for the sensors to configure
    * because this is the task that will configure sensors.
    */
	//clear the Msg Q buffer
	memset(&Msg, 0, sizeof(Msg));

    vTaskDelay(10);

    /* send ourself a start message */
	recog_task_vars.task_ready = 1;
    // TODO Recognition should be started when "RECOG_START" is received from host.
    recognition_startstop( RECOG_CMD_RECOGNIZE_START );
	recog_task_vars.task_ready = 0;


	printf("**************************************\n");
	printf("****** QuickAI Recognition Init ******\n");
	printf("**************************************\n");

	while (!TaskStop)
	{

		recog_task_vars.task_ready = 1;
		qret = xQueueReceive( recog_task_vars.cmd_q, &Msg, SENSIML_RECO_MSGQ_WAIT_TIME);
		configASSERT(qret == pdTRUE);
		recog_task_vars.task_ready = 0;

        pSensorData16 = NULL;

		switch (Msg.ucCommand)
		{
		case RECOG_CMD_RECOGNIZE_START:
            init_recognition_state(RECOG_STATE_RUN);
			break;

		case RECOG_CMD_RECOGNIZE_STOP:
            init_recognition_state(RECOG_STATE_AWAITING_CMD);
			break;

		case RECOG_CMD_RECOGNIZE_START_W_FV:
            init_recognition_state(RECOG_STATE_RUN_W_FV);
			break;

		case RECOG_CMD_NEW_MOTION_DATA:
            {
                // Get the index of the message buffer
                idx = (int)getBuffIndex(&Msg);
                pSensorData16 = (int16_t *)(&motion_buffer[idx]);
                sensor_id = SENSOR_ENG_VALUE_ACCEL;
                /* no M present here */
                batch_size = 1;
                n_sensors = 3;

                assert_ptr_within(  (const void *)(&motion_buffer[0]),
                                  sizeof( motion_buffer),
                                  (const void *)pSensorData16,
                                  (batch_size * n_sensors) * sizeof(int16_t) );
                sml_recognition_run_single( pSensorData16, sensor_id );
            }
		    break;

		case RECOG_CMD_NEW_MOTION_DATA_BATCH:
            {
			    idx = (int)getBuffIndex(&Msg);
                pSensorData16 = (int16_t *)(&motion_buffer[idx]);

                sensor_id = SENSOR_ENG_VALUE_ACCEL;
                /* no M present here */
                batch_size = recog_task_vars.imu_batchSize;
                n_sensors = 3;
				/* Note number of sensor are 6 accel-x-y-z and gyro x-y-z data */

                assert_ptr_within(  (const void *)(&motion_buffer[0]),
                                  sizeof( motion_buffer),
                                  (const void *)pSensorData16,
                                  (batch_size * n_sensors) * sizeof(int16_t) );

                sml_recognition_run_batch( pSensorData16,
                                          batch_size,
                                          n_sensors,
                                          sensor_id);
            }
		    break;

        case RECOG_CMD_NEW_AUDIO_DATA:
            {
                pSensorData16 = (int16_t *)getBuffIndex(&Msg);
                // Process audio
                sensor_id = SENSOR_AUDIO;
                n_sensors = 1;
                batch_size = recog_task_vars.audio_batchSize;

                assert_ptr_within(  (const void *)(&classBuffer[0]),
                                  sizeof( classBuffer),
                                  (const void *)pSensorData16,
                                  (batch_size * n_sensors) * sizeof(int16_t) );

                sml_recognition_run_batch(pSensorData16,
                                          batch_size,
                                          n_sensors,
                                          sensor_id); /* only one audio channel is enabled */
            }
            break;

        case RECOG_CMD_NEW_ADC_DATA:
            {
                pSensorData16 = (int16_t *)getBuffIndex(&Msg);
                // Process ADC
                sensor_id = SENSOR_ADC_LTC_1859_MAYHEW;
                n_sensors = recog_task_vars.adc_channel_count;
                batch_size = recog_task_vars.adc_batchSize;

                assert_ptr_within(  (const void *)(&classBuffer[0]),
                                  sizeof( classBuffer),
                                  (const void *)pSensorData16,
                                  (batch_size * n_sensors) * sizeof(int16_t) );
                sml_recognition_run_batch(pSensorData16,
                                          batch_size,
                                          n_sensors,
                                          sensor_id);
            }
            break;

		default:
			printf("Recognition: Unknown message %d\n", Msg.ucCommand);
			break;
		} /* switch  end */
	}	 /* while loop */

	return;

} /* RecognitionTaskHandler() */

/* Setup msg queue and Task Handler for Recognition Task */
portBASE_TYPE StartRtosTaskRecognition(void)
{
	static UINT8_t ucParameterToPass;

    memset( (void *)&(recog_task_vars), 0, sizeof(recog_task_vars) );
    recog_task_vars.currentState = RECOG_STATE_IDLE;
    recog_task_vars.currBuffPtr = classBuffer;
    kb_model_init();

	/* Create queue for Recognition Task */
	recog_task_vars.cmd_q = xQueueCreate(SENSIML_RECO_QUEUE_LENGTH, sizeof(struct xQ_Packet));
    configASSERT(recog_task_vars.cmd_q);
	configASSERT(SENSIML_RECO_QUEUE_LENGTH);
    vQueueAddToRegistry( recog_task_vars.cmd_q, "Rec_Q" );
	/* Create Recognition Task */
	xTaskCreate(RecognitionTaskHandler,	"RecognitionTaskHandler",STACK_SIZE_ALLOC(STACK_SIZE_TASK_SENSIML_RECO),
				&ucParameterToPass, PRIORITY_TASK_SENSIML_RECO, &recog_task_vars.task_handle);
	configASSERT(recog_task_vars.task_handle);

	return pdPASS;
}

portBASE_TYPE SuspendRecognitionTask(void)
{
	vTaskSuspend(recog_task_vars.task_handle);
	return pdPASS;
}

portBASE_TYPE ResumeRecognitionTask(void)
{
	vTaskResume(recog_task_vars.task_handle);
    return pdPASS;
}

void recognition_startstop( reco_cmd_t cmd )
{
    sendRecognitionMsg( cmd, 0 );
}

#endif

