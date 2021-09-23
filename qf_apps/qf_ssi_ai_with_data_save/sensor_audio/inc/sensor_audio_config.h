/*==========================================================
 *
 *-  Copyright Notice  -------------------------------------
 *                                                          
 *    Licensed Materials - Property of QuickLogic Corp.     
 *    Copyright (C) 2019 QuickLogic Corporation             
 *    All rights reserved                                   
 *    Use, duplication, or disclosure restricted            
 *                                                          
 *    File   : sensor_audio_config.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

/** @file sensor_audio_config.h */

#ifndef __SENSOR_AUDIO_CONFIG_H__
#define __SENSOR_AUDIO_CONFIG_H__

#include "Fw_global_config.h"

#define SENSOR_AUDIO_ID              (0x4155444F)
#define SENSOR_AUDIO_NAME             audio
#define SENSOR_AUDIO_NAME_UPPERCASE   AUDIO
#define SENSOR_AUDIO_RATE_HZ_MAX     (16000)  ///< Maximum sample rate (in Hz) for this sensor
#define SENSOR_AUDIO_MEMSIZE_MAX     (8192)
#define SENSOR_AUDIO_CHANNELS        (1)

/* Default settings for the sensor processing configurations.
 * These may be overridden by user settings in the
 * sensor_audio_config_user.h
 */
#ifndef SENSOR_AUDIO_RECOG_ENABLED
#define SENSOR_AUDIO_RECOG_ENABLED 1    /* Enable SensiML recognition */
#endif /* SENSOR_AUDIO_RECOG_ENABLED */

#ifndef SENSOR_AUDIO_DATASAVE_ENABLED
#define SENSOR_AUDIO_DATASAVE_ENABLED 1 /* Enable data-collection */
#endif /* SENSOR_AUDIO_DATASAVE_ENABLED */

#ifndef SENSOR_AUDIO_LIVESTREAM_ENABLED
#define SENSOR_AUDIO_LIVESTREAM_ENABLED 1 /* Enable live-streaming */
#endif /* SENSOR_AUDIO_LIVESTREAM_ENABLED */

typedef struct st_sensor_audio_config {
    int      is_running; 
    int      enabled;
    uint32_t rate_hz;
} sensor_audio_config_t;
extern sensor_audio_config_t sensor_audio_config;
extern char sensor_json_add_audio[] ;

extern void sensor_audio_startstop( int is_start );
extern void sensor_audio_configure(void);
extern void sensor_audio_clear( void );
extern void sensor_audio_add(void);

#endif /* __SENSOR_AUDIO_CONFIG_H__ */
