#define SENSORS_AUDIO_ACQUISITION_READ_FROM_FILE (0)

#if (SENSORS_AUDIO_ACQUISITION_READ_FROM_FILE)

#include "sensor_audio_config.h"
#include "sensor_audio_acquisition.h"
#include "sensor_audio_process.h"

#include "datablk_mgr.h"
#include "process_ids.h"
#include "timers.h"
#include <string.h>

#define AUDIO_TIMER_PERIOD     (10)
TimerHandle_t audioTimId;

int audio_batch_size_get(void)
{
  int batch_size;
  batch_size  = 10*(audio_config.rate_hz) + 999;
  batch_size /= 1000;
  batch_size += 1;
  batch_size &= ~1;
  return batch_size;
}

int audio_sensordata_buffer_ready(void)
{
  int16_t *p_dest = (int16_t *)paudio_data_block_prev->p_data;
  int nSamples;
  int batch_size;

  nSamples  = (AUDIO_TIMER_PERIOD * audio_config.rate_hz);
  nSamples /= 1000;
  nSamples *= SENSOR_AUDIO_CHANNELS;
  memset( p_dest, 0x11, nSamples*2 );
  audio_samples_collected += nSamples;
  
  batch_size = audio_batch_size_get() * SENSOR_AUDIO_CHANNELS ;
  if (audio_samples_collected >= batch_size)
  {
    paudio_data_block_prev->dbHeader.numDataElements = audio_samples_collected;
    paudio_data_block_prev->dbHeader.numDataChannels = SENSOR_AUDIO_CHANNELS;
    audio_samples_collected = 0;
    return 1;
  }
  else
  {
    return 0;
  }
}

void audio_dataTimer_Callback(TimerHandle_t timHandle)
{ 
  // Warning: must not call vTaskDelay(), vTaskDelayUntil(), or specify a non zero 
  // block time when accessing a queue or a semaphore. 
  if (audio_isr_outq_processor.process_func)
    audio_isr_outq_processor.process_func();
} 

void audio_dataTimerStart(void)
{
  BaseType_t  status;
 
  // Create periodic timer
  if (!audioTimId) {
    audioTimId = xTimerCreate
                (
                   "audioTimer",
                   AUDIO_TIMER_PERIOD, // 10 ticks = ~10ms
                   pdTRUE,            // auto-reload when the timer expires
                   (void *)0,
                   audio_dataTimer_Callback
                );
  }
  
  if (audioTimId)  {
    status = xTimerStart (audioTimId, 0);                // start timer
    if (status != pdPASS)  {
      // Timer could not be started
    } 
  }
  audio_set_first_data_block();
}

void audio_dataTimerStop(void)
{
  if (audioTimId) {
    xTimerStop(audioTimId, 0);
  }
}
#endif

