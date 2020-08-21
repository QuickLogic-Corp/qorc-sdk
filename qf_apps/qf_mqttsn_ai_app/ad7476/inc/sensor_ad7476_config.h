/*==========================================================
 *
 *-  Copyright Notice  -------------------------------------
 *                                                          
 *    Licensed Materials - Property of QuickLogic Corp.     
 *    Copyright (C) 2019 QuickLogic Corporation             
 *    All rights reserved                                   
 *    Use, duplication, or disclosure restricted            
 *                                                          
 *    File   : sensor_ad7476_config.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

/** @file sensor_ad7476_config.h */

#ifndef __SENSOR_AD7476_CONFIG_H__
#define __SENSOR_AD7476_CONFIG_H__

#include "Fw_global_config.h"
#include "sensor_ad7476_config_user.h"

#define SENSOR_AD7476_ID              ((('A') << 24) + (('D')<<16) + ((0)<<8) + ((7476)<<0))
#define SENSOR_AD7476_NAME             ad7476
#define SENSOR_AD7476_NAME_UPPERCASE   AD7476
#define SENSOR_AD7476_RATE_HZ_MAX     (1000000)  ///< Maximum sample rate (in Hz) for this sensor
#define SENSOR_AD7476_MEMSIZE_MAX     (4096*4)
#define SENSOR_AD7476_CHANNELS        (1)

/* Default settings for the sensor processing configurations.
 * These may be overridden by user settings in the
 * sensor_ad7476_config_user.h
 */
#ifndef SENSOR_AD7476_RECOG_ENABLED
#define SENSOR_AD7476_RECOG_ENABLED 1    /* Enable SensiML recognition */
#endif /* SENSOR_AD7476_RECOG_ENABLED */

#ifndef SENSOR_AD7476_DATASAVE_ENABLED
#define SENSOR_AD7476_DATASAVE_ENABLED 1 /* Enable data-collection for */
#endif /* SENSOR_AD7476_DATASAVE_ENABLED */

#ifndef SENSOR_AD7476_LIVESTREAM_ENABLED
#define SENSOR_AD7476_LIVESTREAM_ENABLED 1 /* Enable data-collection for */
#endif /* SENSOR_AD7476_LIVESTREAM_ENABLED */

typedef struct st_ad7476_config {
    int      is_running; /* non-zero if the task is collecting data */
    int      enabled;    /* */
    uint32_t rate_hz;
} ad7476_config_t;

extern ad7476_config_t ad7476_config;
extern char sensor_json_add_ad7476[] ;

extern void sensor_ad7476_startstop( int is_start );
extern void sensor_ad7476_configure(void);
extern void sensor_ad7476_clear( void );
extern void sensor_ad7476_add(void);

#endif /* __SENSOR_AD7476_CONFIG_H__ */
