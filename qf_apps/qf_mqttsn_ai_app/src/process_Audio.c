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

#include "Fw_global_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Recognition.h"

#include "iop_messages.h"
#include "dcl_commands.h"
#include "DataCollection.h"
#include "ble_collection_defs.h"

#include "process_ids.h"
#include "datablk_mgr.h"
#include "micro_tick64.h"

#include "dbg_uart.h"
#include "SensorTile_audio.h"

/** @brief Enumerators for audio errors */
typedef enum {
  AUDIO_ISR_EVENT_NO_BUFFER, ///< error getting a new datablock buffer
} audio_event_types_t;

struct audio_config audio_config;
static uint64_t last_audio_timestamp = 0;

BSP_AUDIO_Init_t MicParams;

uint16_t PCM_Buffer[AUDIO_CHANNELS * AUDIO_SAMPLING_FREQUENCY / 1000];

/** User application should define and instantiate the following structure */
extern outQ_processor_t audio_isr_outq_processor ;
extern void set_first_audio_data_block();
QAI_DataBlock_t  *paudio_data_block_prev = NULL;

/* DCL has sent a command to start or stop audio */
void sensor_audio_startstop( int is_start )
{
    audio_config.is_running = is_start;
    /* FIX ME put code here to stop the audio */
    if( !audio_config.is_running ){
      //BSP_AUDIO_IN_Pause(BSP_AUDIO_IN_INSTANCE);
        // QL_Audio_Voice_Stop();
    } else {
      BSP_AUDIO_IN_Resume(BSP_AUDIO_IN_INSTANCE);
        /* how do we restart audio */
    }

}


/*
 * FIXME: audio should not be hard coded configuration
 */
static void hard_code_audio(void)
{
    /* TODAY we only do 16khz, 1 channel mono */
    int x;
    audio_config.sample_rate_hz = 16000;
    audio_config.nbits = 16;
    for( x = 0 ; x < S3_MAX_AUDIO_MICROPHONE ; x++ ){
        audio_config.mic_config[x] = 0;
    }
    /* choose "platform default" and enable microphone0 */
    audio_config.mic_config[0] = 1;
    /* last item */
    audio_config.n_channels_enabled = 1;

    /* Configure Audio Input peripheral - DFSDM */  
    MicParams.BitsPerSample = 16;
    MicParams.ChannelsNbr = AUDIO_CHANNELS;
    MicParams.Device = AUDIO_IN_DIGITAL_MIC;
    MicParams.SampleRate = AUDIO_SAMPLING_FREQUENCY;
    MicParams.Volume = AUDIO_VOLUME_INPUT;
  
    BSP_AUDIO_IN_Init(BSP_AUDIO_IN_INSTANCE, &MicParams); 
}

QAI_DataBlock_t  *pdata_block_prev = NULL;

/* Time to configure the audio subsystem. */
void sensor_audio_configure(void)
{
    last_audio_timestamp  = 0;
    if( !audio_config.n_channels_enabled ){
        return;
    }
    hard_code_audio();
    

    /* TODO: how do I configure the audio here?
     * NOTE: today above configuration is hard coded
     * Future must support "reasonable configurations"
     *
     * Example A few different audio frequencies.
     * Such as:  8khz vrs 16khyz
     *
     * Example: Different microphone configurations.
     * given that 0= microphone disabled.
     * and that 1 = microphone platform default.
     * Are there other microphone configurations?
     * NOTE: While the merced board may not
     * other platforms (boards) may have options.
     */
#if 0
    QL_Audio_StartUp();
    QL_Audio_Voice_Stop();
    QL_Audio_Config(eCMD_DATA_AV_VOICE_CFG_LPSD_ONLY);
    if (QL_Audio_VoiceWaitForTrigger() !=0) {
        dbg_str( "WaitForTrigger Failed\n");
        configASSERT(0);
    }
#endif
    /* Acquire an audio buffer */
    datablk_mgr_acquire(audio_isr_outq_processor.p_dbm, &pdata_block_prev, 0);
    configASSERT(pdata_block_prev); // probably indicates uninitialized datablock manager handle

    /* Start Microphone acquisition */
    BSP_AUDIO_IN_Record(BSP_AUDIO_IN_INSTANCE, (uint8_t *) PCM_Buffer, AUDIO_IN_BUFFER_SIZE);
    
    BSP_AUDIO_IN_Pause(BSP_AUDIO_IN_INSTANCE);
}


/* DCL has sent a sensor CLEAR command, disable all things audio */
void sensor_audio_clear( void )
{
  audio_config.n_channels_enabled= 0;
  last_audio_timestamp  = 0;
// FIXME how do I do this? and make it start again?  QL_Audio_Voice_Stop();
}


/* DCL has sent a command to enable/add audio */
void sensor_audio_add(void)
{
  /* we do not support any config other then this hard coded config for now */
  hard_code_audio();
  last_audio_timestamp  = 0;
}



//Replace this by your own processing
void AudioProcessBrick16(const int16_t* pBuffer, size_t nSamples ) 
{
    struct sensor_data sdi;
  
    /* if not enabled... then throw data away */
    if( audio_config.n_channels_enabled == 0 ){
        return;
    }
    
    sdi.rate_hz = audio_config.sample_rate_hz;
    sdi.bytes_per_reading = (audio_config.nbits/8) * audio_config.n_channels_enabled;
    sdi.vpData= pBuffer;
    sdi.n_bytes = nSamples * sizeof(uint16_t);
    sdi.sensor_id = SENSOR_AUDIO;
    sdi.time_start = last_audio_timestamp ;
    last_audio_timestamp = xTaskGet_uSecCount();
    sdi.time_end = last_audio_timestamp ;
    
    back_calculate_start_time( &sdi );
    
    ble_send( &sdi  );
#if S3AI_FIRMWARE_IS_COLLECTION
    data_save( &sdi );
#endif
#if S3AI_FIRMWARE_IS_RECOGNITION
    recog_data( &sdi );
#endif
}

void audio_ai_data_processor(
       QAI_DataBlock_t *pIn,
       QAI_DataBlock_t *pOut,
       QAI_DataBlock_t **pRet,
       datablk_pe_event_notifier_t *pevent_notifier
     )
{
    int16_t *p_data = (int16_t *) ( (uint8_t *)pIn + offsetof(QAI_DataBlock_t, p_data) );
    AudioProcessBrick16(p_data, pIn->dbHeader.numDataElements) ;
    *pRet = NULL;
    return;
}

int              audio_samples_collected = 0;

extern outQ_processor_t audio_isr_outq_processor;
#define  AUDIO_ISR_EVENT_NO_BUFFER  (1)   ///< error getting a new datablock buffer

void set_first_audio_data_block()
{
    /* Acquire an audio buffer */
  if (NULL == paudio_data_block_prev) 
  {
    datablk_mgr_acquire(audio_isr_outq_processor.p_dbm, &paudio_data_block_prev, 0);
  }
    configASSERT(paudio_data_block_prev); // probably indicates uninitialized datablock manager handle
    audio_samples_collected = 0;
  paudio_data_block_prev->dbHeader.Tstart = xTaskGetTickCount();
}

void audio_event_notifier(int pid, int event_type, void *p_event_data, int num_data_bytes)
{
  char *p_data = (char *)p_event_data;
  printf("[AUDIO Event] PID=%d, event_type=%d, data=%02x\n", pid, event_type, p_data[0]);
}

int  audio_sensordata_buffer_ready()
{
    int16_t *p_dest = (int16_t *) &paudio_data_block_prev->p_data[sizeof(int16_t)*audio_samples_collected];
    //int ret;
    memcpy( p_dest, PCM_Buffer, DEFAULT_AUDIO_IN_BUFFER_SIZE);
    audio_samples_collected += DEFAULT_AUDIO_IN_BUFFER_SIZE/2;
    if (audio_samples_collected >= 240) {
      paudio_data_block_prev->dbHeader.numDataElements = audio_samples_collected;
      audio_samples_collected = 0;
      return 1;
    } else {
      return 0;
    }
}

void audio_isr_callback(void)
{
    QAI_DataBlock_t  *pdata_block = NULL;
    int  gotNewBlock = 0;
    if (!audio_sensordata_buffer_ready())
      return;
    /* Acquire an audio buffer */
    datablk_mgr_acquireFromISR(audio_isr_outq_processor.p_dbm, &pdata_block);
    if (pdata_block)
    {
        gotNewBlock = 1;
    }
    else
    {
        // send error message 
        // xQueueSendFromISR( error_queue, ... )
        if (audio_isr_outq_processor.p_event_notifier)
          (*audio_isr_outq_processor.p_event_notifier)(audio_isr_outq_processor.in_pid, AUDIO_ISR_EVENT_NO_BUFFER, NULL, 0);
        pdata_block = paudio_data_block_prev;
        pdata_block->dbHeader.Tstart = xTaskGetTickCountFromISR();
        pdata_block->dbHeader.numDropCount++;
    }
    //uint8_t *p_dest = (uint8_t *)pdata_block + offsetof(QAI_DataBlock_t, p_data);
    /* setup the DMA start address for next buffer */

    if (gotNewBlock)
    {
        //uint8_t *p_dest = (uint8_t *)paudio_data_block_prev->p_data;
        //memcpy(p_dest, PCM_Buffer, sizeof(PCM_Buffer));
        //paudio_data_block_prev->dbHeader.numDataElements = sizeof(PCM_Buffer)/sizeof(int16_t);
        /* send the previously filled audio data to specified output Queues */     
        paudio_data_block_prev->dbHeader.Tend = pdata_block->dbHeader.Tstart;
        datablk_mgr_WriteDataBufferToQueuesFromISR(&audio_isr_outq_processor, paudio_data_block_prev);
        paudio_data_block_prev = pdata_block;
    }
}

/**
* @brief  Half Transfer user callback, called by BSP functions.
* @param  None
* @retval None
*/
void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
  audio_isr_callback();
}

/**
* @brief  Transfer Complete user callback, called by BSP functions.
* @param  None
* @retval None
*/
void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
  audio_isr_callback();
}
