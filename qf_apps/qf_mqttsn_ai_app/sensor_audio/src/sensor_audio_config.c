/*==========================================================
 *
 *-  Copyright Notice  -------------------------------------
 *                                                          
 *    Licensed Materials - Property of QuickLogic Corp.     
 *    Copyright (C) 2019 QuickLogic Corporation             
 *    All rights reserved                                   
 *    Use, duplication, or disclosure restricted            
 *                                                          
 *    File   : sensor_audio_config.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

/** @file sensor_audio_config.c */

#include <stdbool.h>

#include "Fw_global_config.h"
#include "sensor_audio_config.h"
#include "sensor_audio_acquisition.h"
#include "eoss3_hal_audio.h"
#include "s3x_clock_hal.h"
sensor_audio_config_t sensor_audio_config;

struct HAL_Audio_Config  Audio_Config_Local = {
    .fUsingLeftChannel = (PDM_MIC_CHANNELS == 2) || (PDM_MIC_LEFT_CH == 1),
    .fUsingRightChannel = (PDM_MIC_CHANNELS == 2) || (PDM_MIC_RIGHT_CH == 1),
    .fUsingDualBuffers = (EN_STEREO_DUAL_BUF == 1),
};

void sensor_audio_startstop( int is_start )
{
  /** @todo Replace contents of this function */
#if  (SENSORS_AUDIO_ACQUISITION_READ_FROM_FILE)
  if (is_start == 1) 
  {
    audio_dataTimerStart();
  }
  else
  {
    audio_dataTimerStop();
  }
  return;
#endif
  if ((is_start) && (sensor_audio_config.enabled) && (sensor_audio_config.is_running == 0) )
  {
     HAL_Audio_Start(&gPdmConnector, 0, true, &Audio_Config_Local);
     S3x_Clk_Enable(S3X_LPSD);
     HAL_Audio_StartDMA();   // Starting DMA immedately ships data
     sensor_audio_config.is_running = 1;
  }
  else if ( (is_start == 0) && (sensor_audio_config.is_running == 1) )
  {
    HAL_Audio_Stop(&gPdmConnector, &Audio_Config_Local); // Stops the DMA, among other things
    S3x_Clk_Disable(S3X_LPSD);
    sensor_audio_config.is_running = 0;
  }
}

void sensor_audio_configure(void)
{
  /** @todo Replace contents of this function */
  sensor_audio_config.rate_hz = SENSOR_AUDIO_RATE_HZ_MAX;
  static int sensor_audio_configured = false;
  if (sensor_audio_configured == false)
  {
    //Audio_Config_Local = *((struct HAL_Audio_Config*)pv); 
    QL_Audio_StartUp(NULL, &Audio_Config_Local);
    HAL_Audio_StopDMA(Audio_Config_Local.fUsingLeftChannel, Audio_Config_Local.fUsingRightChannel);
    /*Ask EOSS3 to stop and go in power save mode*/
    HAL_Audio_Stop(&gPdmConnector, &Audio_Config_Local);
    sensor_audio_configured = true;
  }
  sensor_audio_startstop(1);
}

void sensor_audio_clear( void )
{
  sensor_audio_config.enabled = false;
  /** @todo Replace contents of this function */
}

void sensor_audio_add(void)
{
  sensor_audio_config.enabled = true;
  /** @todo Replace contents of this function */
}

