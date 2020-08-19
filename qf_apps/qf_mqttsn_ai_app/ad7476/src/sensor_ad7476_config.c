/*==========================================================
 *
 *-  Copyright Notice  -------------------------------------
 *                                                          
 *    Licensed Materials - Property of QuickLogic Corp.     
 *    Copyright (C) 2019 QuickLogic Corporation             
 *    All rights reserved                                   
 *    Use, duplication, or disclosure restricted            
 *                                                          
 *    File   : sensor_ad7476_config.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

/** @file sensor_ad7476_config.c */

#include <stdbool.h>

#include "Fw_global_config.h"
#include "sensor_ad7476_config.h"

#include "sensor_ad7476_acquisition.h"

struct ad7476_config ad7476_config;

void sensor_ad7476_startstop( int is_start )
{
  /** @todo Replace contents of this function */
#if  0 // (SENSORS_AD7476_ACQUISITION_READ_FROM_FILE)
  if (is_start == 1) 
  {
    ad7476_dataTimerStart();
  }
  else
  {
    ad7476_dataTimerStop();
  }
#endif

}

void sensor_ad7476_configure(void)
{
  /** @todo Replace contents of this function */
  ad7476_config.rate_hz = SENSOR_AD7476_RATE_HZ_MAX;
}

void sensor_ad7476_clear( void )
{
  ad7476_config.enabled = false;
  /** @todo Replace contents of this function */
}

void _sensor_ad7476_add(void)
{
  ad7476_config.enabled = true;
  /** @todo Replace contents of this function */
}

/* for simple sensor with multiple channels and a range for
 * each channel
 */
char sensor1_json_add_ad7476[] =
 "\"ad7476\" : { "
 "  \"name\"   : \"ad7476\","
 "  \"version\" : %d ,"
 "  \"sensor_id\" : %d,"
 "  \"format\" : %s,"
 "  \"titles\" : %s,"
 "  \"enabled\" : %d,"
 "  \"rate\" : %d,"
 "  \"range\" : %d"
 " } "
 ;

/* for audio : invoke this as follows
 *   RIFF_sprintf( jsonObj, sensor_json_add_ad7476, 
 *                   version,    # an integer,  = 1 (QLSM version)
 *                   sensor_id,  # an integer
 *                   format_string, # a string, python format string, 
 *                                  # e.g. "<h" 
 *                                  # for single channel 16-bit signed data
 *                  column_titles,  # a string, title for each column 
 *                  enabled,        # an integer indicating if this sensor is enabled
 *                  nbits,          # an integer, number of bits per channel
 *                  rate_hz,        # an integer, sensor sampling rate in Hz
 *                  ch0_config,     # an integer, indicates if MIC Channel 0 is enabled
 *                  ch1_config,     # an integer, indicates if MIC Channel 1 is enabled
 *                  ch2_config,     # an integer, indicates if MIC Channel 2 is enabled
 *                  ch3_config,     # an integer, indicates if MIC Channel 3 is enabled
 *             )     
 */
char sensor_json_add_ad7476[] =
 "\"audio\" : { \n"
 "  \"name\"   : \"ad7476\",\n"
 "  \"version\" : %d ,\n"
 "  \"sensor_id\" : %d,\n"
 "  \"format\" : \"%s\",\n"
 "  \"titles\" : \"%s\",\n"
 "  \"enabled\" : %d,\n"
 "  \"nbits\"   : %d,\n"
 "  \"rate\" : %d,\n"
 "  \"mic\" : [\n"
 "    { \"ch\" : 0, \"config\" : %d },\n"
 "    { \"ch\" : 1, \"config\" : %d },\n"
 "    { \"ch\" : 2, \"config\" : %d },\n"
 "    { \"ch\" : 3, \"config\" : %d } \n"
 "   ]\n"
 " }\n "
 ;

char sensor_json_add_audio_fmt[] =
 "\"audio\" : { \n"
 "  \"name\"   : \"audio\",\n"
 "  \"version\" : %d ,\n"
 "  \"sensor_id\" : %d,\n"
 "  \"format\" : \"%s\",\n"
 "  \"titles\" : \"%s\",\n"
 "  \"enabled\" : %d,\n"
 "  \"nbits\"   : %d,\n"
 "  \"rate\" : %d,\n"
 "  \"mic\" : [\n"
 "    { \"ch\" : 0, \"config\" : %d },\n"
 "    { \"ch\" : 1, \"config\" : %d },\n"
 "    { \"ch\" : 2, \"config\" : %d },\n"
 "    { \"ch\" : 3, \"config\" : %d } \n"
 "   ]\n"
 " }\n "
 ;
#if 0
   int version = 1;
   int sensor_id = SENSOR_AUDIO;
   char format_string[] = "<h";
   char column_titles[] = "ch0";
   int enabled = audio_config.n_channels_enabled;
   int nbits = audio_config.nbits;
   int rate_hz = audio_config.sample_rate_hz;
   int ch0_config = 1;
   int ch1_config = 0;
   int ch2_config = 0;
   int ch3_config = 0;
   printf( sensor_json_add_audio_fmt, 
                   version,    // an integer,  = 1 (QLSM version)
                   sensor_id,  // an integer
                   format_string, // a string, python format string, 
                                   // e.g. "<h" 
                                   // for single channel 16-bit signed data
                   column_titles,  // a string, title for each column 
                   enabled,        // an integer indicating if this sensor is enabled
                   nbits,          // an integer, number of bits per channel
                   rate_hz,        // an integer, sensor sampling rate in Hz
                   ch0_config,     // an integer, indicates if MIC Channel 0 is enabled
                   ch1_config,     // an integer, indicates if MIC Channel 1 is enabled
                   ch2_config,     // an integer, indicates if MIC Channel 2 is enabled
                   ch3_config      // an integer, indicates if MIC Channel 3 is enabled
              )   ;
   
   RIFF_sprintf( pObj, sensor_json_add_audio_fmt, 
                   version,    // an integer,  = 1 (QLSM version)
                   sensor_id,  // an integer
                   format_string, // a string, python format string, 
                                   // e.g. "<h" 
                                   // for single channel 16-bit signed data
                   column_titles,  // a string, title for each column 
                   enabled,        // an integer indicating if this sensor is enabled
                   nbits,          // an integer, number of bits per channel
                   rate_hz,        // an integer, sensor sampling rate in Hz
                   ch0_config,     // an integer, indicates if MIC Channel 0 is enabled
                   ch1_config,     // an integer, indicates if MIC Channel 1 is enabled
                   ch2_config,     // an integer, indicates if MIC Channel 2 is enabled
                   ch3_config      // an integer, indicates if MIC Channel 3 is enabled
              )   ;
#endif

#if SENSOR_AD7476_ENABLED
#include "sensor_ad7476_config.h"
static const char * const ad7476_format[] = {
    NULL,
    "<B", /* unsigned 8bit */
    "<BB",
    "<BBB",
    "<BBBB",
    NULL,
    "<H", /* unsigned 16bit */
    "<HH",
    "<HHH",
    "<HHHH"
};

static const char * const ad7476_titles[] = {
    NULL,
    "ch0",
    "ch0,ch1",
    "ch0,ch1,ch2",
    "ch0,ch1,ch2,ch3"
};

/* add json data for the ad7476 */
static void json_add_ad7476( struct riff_object *pObj )
{
   int version = 1;
   int sensor_id = SENSOR_AD7476_ID;
   char format_string[] = "<h";
   char column_titles[] = "ch0";
   int enabled = ad7476_config.enabled;
   int nbits = 16;
   int rate_hz = ad7476_config.rate_hz;
   int ch0_config = 1;
   int ch1_config = 0;
   int ch2_config = 0;
   int ch3_config = 0;
   printf( sensor_json_add_ad7476, 
                   version,    // an integer,  = 1 (QLSM version)
                   sensor_id,  // an integer
                   format_string, // a string, python format string, 
                                   // e.g. "<h" 
                                   // for single channel 16-bit signed data
                   column_titles,  // a string, title for each column 
                   enabled,        // an integer indicating if this sensor is enabled
                   nbits,          // an integer, number of bits per channel
                   rate_hz,        // an integer, sensor sampling rate in Hz
                   ch0_config,     // an integer, indicates if MIC Channel 0 is enabled
                   ch1_config,     // an integer, indicates if MIC Channel 1 is enabled
                   ch2_config,     // an integer, indicates if MIC Channel 2 is enabled
                   ch3_config      // an integer, indicates if MIC Channel 3 is enabled
              )   ;
   
   RIFF_sprintf( pObj, sensor_json_add_ad7476, 
                   version,    // an integer,  = 1 (QLSM version)
                   sensor_id,  // an integer
                   format_string, // a string, python format string, 
                                   // e.g. "<h" 
                                   // for single channel 16-bit signed data
                   column_titles,  // a string, title for each column 
                   enabled,        // an integer indicating if this sensor is enabled
                   nbits,          // an integer, number of bits per channel
                   rate_hz,        // an integer, sensor sampling rate in Hz
                   ch0_config,     // an integer, indicates if MIC Channel 0 is enabled
                   ch1_config,     // an integer, indicates if MIC Channel 1 is enabled
                   ch2_config,     // an integer, indicates if MIC Channel 2 is enabled
                   ch3_config      // an integer, indicates if MIC Channel 3 is enabled
              )   ;
#if 0
    int x;
    const char *cp;
    int nbits = 16;
    RIFF_add_json_tag_group_open(pObj, "ad7476", '{' );

    RIFF_add_json_tag_plain( pObj, s_version, 1, s_one);
    RIFF_add_json_tag_quoted( pObj, s_name, 1, "audio" );
    RIFF_add_json_tag_plain( pObj, s_sensor_id, 1, "%u", (unsigned)SENSOR_AD7476_ID);

    if( !ad7476_config.enabled ){
        RIFF_add_json_tag_plain( pObj, s_enabled, 0, s_zero );
    } else {
        RIFF_add_json_tag_plain( pObj, s_enabled, 1, s_one );
        RIFF_add_json_tag_plain( pObj, s_nbits, 1, "%d", nbits );
        RIFF_add_json_tag_plain( pObj, s_rate, 1, "%d", ad7476_config.rate_hz);
        if( nbits == 8 ){
            cp = ad7476_format[ 0 + ad7476_config.enabled ];
        } else {
            cp = ad7476_format[ 5 + ad7476_config.enabled ];
        }
        if( cp == NULL ){
            cp = s_error;
        }
        RIFF_add_json_tag_quoted( pObj, s_format, 1, cp );
        cp = ad7476_titles[ ad7476_config.enabled  ];
        if( cp == NULL ){
            cp = s_error;
        }
        RIFF_add_json_tag_quoted( pObj, s_titles, 1, cp );

        RIFF_add_json_tag_group_open( pObj, "mic", '[');
        /* we have zero mics configured */
        for( x = 0 ; x < 4 ; x++ ){
            RIFF_add_json_group_open( pObj, '{' );
            RIFF_add_json_tag_plain( pObj, s_ch, 1, "%d", x );
            RIFF_add_json_tag_plain( pObj, s_config, 0, "%d", (x==0)?1:0 );
            RIFF_add_json_group_close( pObj, '}', (x < (4-1)) ? 1 : 0 );
        }
         RIFF_add_json_group_close( pObj, ']', 0 );
    }
    RIFF_add_json_group_close( pObj, '}', 0);
#endif
}
#endif

#if SENSOR_AD7476_ENABLED
    RIFF_sprintf( pObj, ",");
    json_add_ad7476( pObj );
#endif
