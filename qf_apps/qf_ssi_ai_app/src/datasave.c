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
#include "RtosTask.h"
#include "iop_messages.h"
#include "ql_riff.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "dcl_commands.h"
#include "DataCollection.h"
#if (USE_FATFS_APIS == 1)
#include "ff.h"
#elif (USE_FREERTOS_FAT_APIS == 1)
#include "media_drv_spi_sd.h"
#include "ff_headers.h"
#include "ff_stdio.h"
#else /* USE_QLFS_APIS */
#include "ql_fs.h"
#endif
//#include "ql_adcTask.h"
#include "ql_time.h"
//#include "eoss3_hal_rtc.h"
#include "dbg_uart.h"
#include "sensor_ssss.h"
#include "sensor_audio_config.h"
#include "micro_tick64.h"

#if S3AI_FIRMWARE_DATASAVE

SensorEnableStatus datacollection_sensor_status;

/* The states the data save state machine has are here */
enum datasave_state {
    DATASAVE_init,
    DATASAVE_idle,
    DATASAVE_run
};

struct file_save_config file_save_config;

/* have two riff blocks ready to write into */
struct riff_pingpong {
    struct riff_object *pObj;
    /* how many riff objects have we written for this sensor */
    uint32_t *pDataLen;
    /* the 0 is the start timestamp */
    /* the 1 is the end timestamp */
    uint64_t *pTimeStamps;
};

/* This represents the object that a sensor is writing into
 * One exists for each active sensor that that is saving to the file.
*/
struct riff_sensor_obj {
    /* sensor this object is for */
    uint32_t sensor_id;
    /* what block number is this we are writing */
    int sequence_number;
    /* object for this sensor */
    struct riff_pingpong cur_block;
    /* next object we will use */
    struct riff_pingpong next_block;
};



struct datasave_task_vars {
    int queue_empty;
    enum datasave_state cur_state;
    SemaphoreHandle_t   wait_sem;
    xTaskHandle task_handle;
    struct riff_file *cur_riff_file;
/* we don't expect more then 8 active sensors */
#define N_RIFF_OBJS 8
    struct riff_sensor_obj all_sensor_objs[ N_RIFF_OBJS ];
};

struct datasave_task_vars datasave_task_vars;

/* FIXME: Future, make this configurable based on filesystem?
 * OR - maybe 4K is a really good number and we should stick with 4K
 */
#define RIFF_BLOCK_SIZE (4*1024) /* we save data in 4K blocks */
/* this is our total buffer */
#define _rounded_size (((DATA_CAPTURE_BUFFER_SIZE_K_BYTES * 1024) / RIFF_BLOCK_SIZE) * RIFF_BLOCK_SIZE)
#ifndef riff_bytebuffer
uint8_t riff_bytebuffer[ _rounded_size ];
#endif

#if (USE_FATFS_APIS == 1)
FATFS FatFsObj; // FatFs work area needed for each volume
char  DiskDefaultMountPoint[] = "/SPISD" ;
#endif /* USE_FATFS_APIS */

#if (USE_FREERTOS_FAT_APIS == 1)
#if (DEFAULT_STORAGE_LOCATION == FREERTOS_SPI_SD)
char  DiskDefaultMountPoint[] = "/default" ;
#else
char  DiskDefaultMountPoint[] = "/SPIFlash" ;
#endif /* DEFAULT_STORAGE_LOCATION */
#endif /* USE_FREERTOS_FAT_APIS */

static void 
wake_storage_task(void)
{
    /* storage needs to check things */
    xSemaphoreGive( datasave_task_vars.wait_sem );
}

/* constructs the filename we are going to save data to */
static void construct_filename(void)
{
    time_t t;
    struct tm l_time;
    const char *end;
    char *dst;


    /*
     * Filename is based on:
     *  (A) prefix text
     *  (B) current date & time.
     */

    /* get now */
    ql_time( &t );
    ql_localtime_r( &t, &l_time );

    memset( (void *)(&file_save_config.cur_filename[0]), 0, sizeof(file_save_config.cur_filename) );

    /* do not insert volume name, QLFS library does this for you
     * strcpy( file_save_config.cur_filename, QLFS_DEFAULT_FILESYTEM->mountVolume );
     * do not start with a slash, QLFS does it for you.
     * strcat( file_save_config.cur_filename, "/");
     */
    strcat( file_save_config.cur_filename,
           file_save_config.cur_filename_template );
    dst = strchr(file_save_config.cur_filename,0);

    end = &file_save_config.cur_filename[ sizeof(file_save_config.cur_filename) ];
    /* leave room for null */
    end--;
#if (USE_DCL_FILENAME_ONLY == 0) //don't write time or extension in the file name. DCL will do that
#if (RIFF_TIMESTAMP_SEQUENCE_FILENAMES == 1)
    snprintf( dst,
             end - dst,
             "_%04d%02d%02dT%02d%02d%02d%s",
             l_time.tm_year+1900,
             l_time.tm_mon+1,
             l_time.tm_mday,
             l_time.tm_hour,
             l_time.tm_min,
             l_time.tm_sec,
             ".qlsm" );
#elif (RIFF_AUTO_SEQUENCE_FILENAMES == 1)
    snprintf( &file_save_config.cur_filename,
    		sizeof(file_save_config.cur_filename)-1,
			riff_get_newfilename() );
#endif /* RIFF_TIMESTAMP_SEQUENCE_FILENAMES */
#endif /* USE_DCL_FILENAME_ONLY */
}

/* common strings for the JSON header */
static const char s_two[] = "2";
static const char s_one[] = "1";
static const char s_zero[] = "0";
static const char s_enabled[] = "enabled";
static const char s_sensor_id[] = "sensor_id";
static const char s_name[] = "name";
static const char s_version[]="version";
static const char s_titles[]="titles";
static const char s_error[] = "error";
static const char s_rate[] = "rate";
static const char s_range[] = "range";
static const char s_gain[] = "gain";
static const char s_accel_xyz[] = "AccelerometerX,AccelerometerY,AccelerometerZ";
static const char s_gyro_xyz[] = "GyroscopeX,GyroscopeY,GyroscopeZ";
static const char s_mag_xyz[] = "MagnetometerX,MagnetometerY,MagnetometerZ";
static const char s_ch[] = "ch";
static const char s_nbits[] = "nbits";
static const char s_3h[] = "<hhh";
static const char s_6h[] = "<hhhhhh";
static const char s_config[] = "config";
static const char s_format[] = "format";
static const char s_nchannels[] = "nchannels";
static const char s_accelrange[] = "accel range";
static const char s_gyrorange[] = "gyro range";
static const char s_xyzxyz[] = "AccelerometerX,AccelerometerY,AccelerometerZ,GyroscopeX,GyroscopeY,GyroscopeZ";

extern void sensor_config_apply_sequence( const struct sensor_config_msg *pConfigSequence, SensorEnableStatus *pSensorStatus );

#if (SENSOR_SSSS_DATASAVE_ENABLED == 1)
static const char s_ssss_titles[] = "AccelerometerX,AccelerometerY,AccelerometerZ";
/* for simple sensor with multiple channels and a range for each channel */
char sensor_json_add_ssss[] =
 "\"ssss\" : { \n"
 "  \"name\"   : \"ssss\",\n"
 "  \"version\" : %d ,\n"
 "  \"sensor_id\" : %d,\n"
 "  \"format\" : \"%s\",\n"
 "  \"titles\" : \"%s\",\n"
 "  \"enabled\" : %d,\n"
 "  \"rate\" : %d,\n"
 "  \"range\" : %d\n"
 " } "
 ;

static void json_add_ssss( struct riff_object *pObj )
{
    int version = 2;
    int sensor_id = SENSOR_SSSS_ID;
    char format_string[] = "<hhh";
    char column_titles[] = "AccelerometerX,AccelerometerY,AccelerometerZ";
    int enabled = sensor_ssss_config.enabled;
    int nbits = 16;
    int rate_hz = sensor_ssss_config.rate_hz;
    int range = 0;
    RIFF_sprintf( pObj, sensor_json_add_ssss,
            version,       // an integer,  = 1 (QLSM version)
            sensor_id,     // an integer
            format_string, // a string, python format string,
                           // e.g. "<h" for single channel 16-bit signed data
            column_titles, // a string, title for each column
            enabled,       // an integer indicating if this sensor is enabled
            rate_hz,       // an integer, sensor sampling rate in Hz
            range          // an integer, indicates if MIC Channel 0 is enabled
    )   ;
}
#endif /* SENSOR_SSSS_DATASAVE_ENABLED */

#if IMU_DRIVERS
/* add json info for the accel module */
static void json_add_accel( struct riff_object *pObj )
{

    RIFF_add_json_tag_group_open(pObj, "accel", '{' );
    RIFF_add_json_tag_quoted(pObj, s_name, 1, "accel" );
    RIFF_add_json_tag_plain( pObj, s_version, 1, s_two );
    RIFF_add_json_tag_plain( pObj, s_sensor_id, 1,"%u", (unsigned)SENSOR_ENG_VALUE_ACCEL );
    if( !imu_config.accel.enabled ){
        RIFF_add_json_tag_plain( pObj, s_enabled, 0, s_zero );
    } else {
        RIFF_add_json_tag_plain( pObj, s_enabled, 1, s_one );
        RIFF_add_json_tag_quoted( pObj, s_format, 1,s_3h ); /* 16bit signed */
        RIFF_add_json_tag_quoted( pObj, s_titles, 1, s_accel_xyz);
        RIFF_add_json_tag_plain( pObj, s_rate, 1,"%d", imu_config.accel.rate_hz );
        RIFF_add_json_tag_plain( pObj, s_range, 0,"%d", imu_config.accel.range );
    }
    RIFF_add_json_group_close( pObj, '}', 0 );
}


/* add json info for the accel module */
static void json_add_accel_gyro( struct riff_object *pObj )
{
    RIFF_add_json_tag_group_open(pObj, "accel+gyro", '{' );
    RIFF_add_json_tag_quoted(pObj, s_name, 1, "accel+gyro" );
    RIFF_add_json_tag_plain( pObj, s_version, 1, s_two );
    RIFF_add_json_tag_plain( pObj, s_sensor_id, 1,"%u", (unsigned)SENSOR_ENG_VALUE_ACCEL_GYRO );

    RIFF_add_json_tag_plain( pObj, s_enabled, 1, s_one );
    RIFF_add_json_tag_quoted( pObj, s_format, 1,s_6h ); /* 16bit signed */
    RIFF_add_json_tag_quoted( pObj, s_titles, 1, s_xyzxyz);
    RIFF_add_json_tag_plain( pObj, s_rate, 1,"%d", imu_config.accel.rate_hz );
    RIFF_add_json_tag_plain( pObj, s_accelrange, 1,"%d", imu_config.accel.range );
    RIFF_add_json_tag_plain( pObj, s_gyrorange, 0,"%d", imu_config.gyro.range );

    RIFF_add_json_group_close( pObj, '}', 0 );
}

#endif

#if IMU_DRIVERS
/* add json info for the gyro module */
static void json_add_gyro( struct riff_object *pObj )
{
    RIFF_add_json_tag_group_open(pObj, "gyro", '{' );
    RIFF_add_json_tag_quoted(pObj, s_name, 1, "gyro" );
    RIFF_add_json_tag_plain( pObj, s_version, 1, s_two );
    RIFF_add_json_tag_plain( pObj, s_sensor_id, 1, "%u", (unsigned)SENSOR_ENG_VALUE_GYRO );
    if( !imu_config.gyro.enabled ){
        RIFF_add_json_tag_plain( pObj, s_enabled, 0, s_zero );
    } else {
        RIFF_add_json_tag_quoted( pObj, s_format, 1, s_3h );
        RIFF_add_json_tag_quoted( pObj, s_titles, 1, s_gyro_xyz);
        RIFF_add_json_tag_plain( pObj, s_enabled,  1,s_one );
        RIFF_add_json_tag_plain( pObj, s_rate,  1,"%d", imu_config.gyro.rate_hz );
        RIFF_add_json_tag_plain( pObj, s_range,  0,"%d", imu_config.gyro.range );
    }
    RIFF_add_json_group_close( pObj, '}', 0 );
}
#endif

#if IMU_DRIVERS
/* add info for the magnetometer */
static void json_add_mag( struct riff_object *pObj )
{
    RIFF_add_json_tag_group_open(pObj, "mag", '{' );
    RIFF_add_json_tag_plain( pObj, s_version, 1, s_two );
    RIFF_add_json_tag_quoted( pObj, s_name, 1, "mag" );
    RIFF_add_json_tag_plain( pObj, s_sensor_id, 1, "%u", (unsigned)SENSOR_ENG_VALUE_MAGNETOMETER );
    if( !imu_config.mag.enabled ){
        RIFF_add_json_tag_plain( pObj, s_enabled, 0, s_zero );
    } else {
        RIFF_add_json_tag_plain( pObj, s_enabled, 1, s_one );
        RIFF_add_json_tag_quoted( pObj, s_format, 1, s_3h );
        RIFF_add_json_tag_quoted( pObj, s_titles, 1, s_mag_xyz);
        RIFF_add_json_tag_plain( pObj, s_rate, 1, "%d", imu_config.mag.rate_hz );
        RIFF_add_json_tag_plain( pObj, s_range, 0, "%d", imu_config.mag.range );
    }
    RIFF_add_json_group_close( pObj, '}', 0 );
}
#endif

#if AUDIO_DRIVER
/* add json data for the audio */

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

/* for audio : invoke this as follows
 *   RIFF_sprintf( jsonObj, sensor_json_add_ssss,
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

static void json_add_audio( struct riff_object *pObj )
{
	   int version = 2;
	   int sensor_id = SENSOR_AUDIO;
	   char format_string[] = "<h";
	   char column_titles[] = "ch0";
	   int enabled = sensor_audio_config.enabled;
	   int nbits = 16;
	   int rate_hz = sensor_audio_config.rate_hz;
	   int ch0_config = 1;
	   int ch1_config = 0;
	   int ch2_config = 0;
	   int ch3_config = 0;

	   RIFF_sprintf( pObj, sensor_json_add_audio_fmt,
	                   version,        // an integer,  = 2 (QLSM version)
	                   sensor_id,      // an integer
	                   format_string,  // a string, python format string,
	                                   // e.g. "<h" for single channel 16-bit signed data
	                   column_titles,  // a string, title for each column
	                   enabled,        // an integer indicating if this sensor is enabled
	                   nbits,          // an integer, number of bits per channel
	                   rate_hz,        // an integer, sensor sampling rate in Hz
	                   ch0_config,     // an integer, indicates if MIC Channel 0 is enabled
	                   ch1_config,     // an integer, indicates if MIC Channel 1 is enabled
	                   ch2_config,     // an integer, indicates if MIC Channel 2 is enabled
	                   ch3_config      // an integer, indicates if MIC Channel 3 is enabled
	              )   ;
}

#if 0 // use RIFF json tag apis to add audio configuration
static const char * const mic_format[] = {
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

static const char * const mic_titles[] = {
    NULL,
    "audio_ch0",
    "audio_ch0,audio_ch1",
    "audio_ch0,audio_ch1,audio_ch2",
    "audio_ch0,audio_ch1,audio_ch2,audio_ch3"
};

static void json_add_audio( struct riff_object *pObj )
{
    int x;
    const char *cp;

    RIFF_add_json_tag_group_open(pObj, "audio", '{' );

    RIFF_add_json_tag_plain( pObj, s_version, 1, s_two);
    RIFF_add_json_tag_quoted( pObj, s_name, 1, "audio" );
    RIFF_add_json_tag_plain( pObj, s_sensor_id, 1, "%u", (unsigned)SENSOR_AUDIO);

    if( !audio_config.n_channels_enabled ){
        RIFF_add_json_tag_plain( pObj, s_enabled, 0, s_zero );
    } else {
        RIFF_add_json_tag_plain( pObj, s_enabled, 1, s_one );
        RIFF_add_json_tag_plain( pObj, s_nbits, 1, "%d", audio_config.nbits );
        RIFF_add_json_tag_plain( pObj, s_rate, 1, "%d", audio_config.sample_rate_hz);
        if( audio_config.nbits == 8 ){
            cp = mic_format[ 0 + audio_config.n_channels_enabled ];
        } else {
            cp = mic_format[ 5 + audio_config.n_channels_enabled ];
        }
        if( cp == NULL ){
            cp = s_error;
        }
        RIFF_add_json_tag_quoted( pObj, s_format, 1, cp );
        cp = mic_titles[ audio_config.n_channels_enabled  ];
        if( cp == NULL ){
            cp = s_error;
        }
        RIFF_add_json_tag_quoted( pObj, s_titles, 1, cp );

        RIFF_add_json_tag_group_open( pObj, "mic", '[');
        /* we have zero mics configured */
        for( x = 0 ; x < S3_MAX_AUDIO_MICROPHONE ; x++ ){
            RIFF_add_json_group_open( pObj, '{' );
            RIFF_add_json_tag_plain( pObj, s_ch, 1, "%d", x );
            RIFF_add_json_tag_plain( pObj, s_config, 0, "%d", audio_config.mic_config[x] );
            RIFF_add_json_group_close( pObj, '}', (x < (S3_MAX_AUDIO_MICROPHONE-1)) ? 1 : 0 );
        }
         RIFF_add_json_group_close( pObj, ']', 0 );
    }
    RIFF_add_json_group_close( pObj, '}', 0);
}
#endif /* SENSOR_AUDIO_DATASAVE_ENABLED */
#endif

#if LTC1859_DRIVER


static const char *  const ltc1859_titles[] = {
    NULL,
    "ltc1859_mayhew_ch0", /* 1 channel */
    "ltc1859_mayhew_ch0,ltc1859_mayhew_ch1",  /* 2 channel */
    "ltc1859_mayhew_ch0,ltc1859_mayhew_ch1,ltc1859_mayhew_ch2",  /* 3channel */
    "ltc1859_mayhew_ch0,ltc1859_mayhew_ch1,ltc1859_mayhew_ch2,ltc1859_mayhew_ch3",  /* 4channel */
    "ltc1859_mayhew_ch0,ltc1859_mayhew_ch1,ltc1859_mayhew_ch2,ltc1859_mayhew_ch3,ltc1859_mayhew_ch4",   /* 5channel */
    "ltc1859_mayhew_ch0,ltc1859_mayhew_ch1,ltc1859_mayhew_ch2,ltc1859_mayhew_ch3,ltc1859_mayhew_ch4,ltc1859_mayhew_ch5",   /* 6 channel */
    "ltc1859_mayhew_ch0,ltc1859_mayhew_ch1,ltc1859_mayhew_ch2,ltc1859_mayhew_ch3,ltc1859_mayhew_ch4,ltc1859_mayhew_ch5,ltc1859_mayhew_ch6", /* 7channels */
    "ltc1859_mayhew_ch0,ltc1859_mayhew_ch1,ltc1859_mayhew_ch2,ltc1859_mayhew_ch3,ltc1859_mayhew_ch4,ltc1859_mayhew_ch5,ltc1859_mayhew_ch6,ltc1859_mayhew_ch7", /* 8channels */
};

/* add data for the ltc1859 */
static void json_add_ltc1859a( struct riff_object *pObj )
{
    int x;
    int b;
    int  nChannels;
    const char *cp;
    char *wr_cp;
    char fmt_str[ LTC1859_MAX_CHANNELS  + 4 ];


    RIFF_add_json_tag_group_open(pObj, "ltc1859_mayhew", '{' );
    RIFF_add_json_tag_plain( pObj, s_version, 1, s_two );
    RIFF_add_json_tag_quoted( pObj, s_name, 1, "ltc1859_mayhew");
    RIFF_add_json_tag_plain( pObj, s_sensor_id, 1, "%u", (unsigned)SENSOR_ADC_LTC_1859_MAYHEW );
    if( ltc1859_task_config.channel_enable_bits == 0 ){
        RIFF_add_json_tag_plain( pObj, s_enabled, 0, "%d", 0 );
    } else {
        RIFF_add_json_tag_plain( pObj, s_enabled, 1, "%d", 1 );
        RIFF_add_json_tag_plain( pObj, s_rate, 1, "%d", ltc1859_task_config.frequency);
        /* determine how many channels are used */
        nChannels = 0;
        for( x = 0 ; x < LTC1859_MAX_CHANNELS ; x++ ){
            b = 1 << x;
            if( ltc1859_task_config.channel_enable_bits & b ){
                nChannels += 1;
            }
        }
        RIFF_add_json_tag_plain( pObj, s_nchannels, 1, "%d", nChannels );
        /* calculate format strinbased upon config value */
        wr_cp = fmt_str;
        /* we only support little endian */
        *wr_cp++ = '<'; /* little endian */
        for( x = 0 ; x < LTC1859_MAX_CHANNELS ; x++ ){
            const struct LTC1859_ymx_plus_b *p;
            b = 1 << x;
            /* if the channel is NOT enabled, skip */
            if( !(ltc1859_task_config.channel_enable_bits & b) ){
                /* not enabled skip */
                continue;
        }
            /* using the command byte get the conversion info */
            /* that conversion also has the python format char */
            p = HAL_LTC1859_CfgToYmxB( ltc1859_task_config.chnl_commands[x] );
            /* add this channel's configuration char */
            *wr_cp++ = p->python_format_char;
        }
        /* terminate format string */
        *wr_cp++ = 0;
        RIFF_add_json_tag_quoted( pObj, s_format, 1,fmt_str );

        /* get column titles */
        cp = ltc1859_titles[ nChannels ];
        if ( cp == NULL ){
            cp = s_error;
        }
        RIFF_add_json_tag_quoted( pObj, s_titles, 1, cp );

        RIFF_add_json_tag_group_open( pObj, s_config, '[' );
        for( x = 0 ; x < nChannels ; x++ ){
            RIFF_add_json_group_open(pObj, '{' );
            RIFF_add_json_tag_plain( pObj, s_ch, 1, "%d", x );
            RIFF_add_json_tag_plain( pObj, s_config, 0, "%u", ltc1859_task_config.chnl_commands[x]);
            if( (x+1) == nChannels ){
                /* last one does not get a comma */
                RIFF_add_json_group_close(pObj, '}', 0 );
            } else {
                RIFF_add_json_group_close(pObj, '}', 1 );
            }
        }
        RIFF_add_json_group_close(pObj, ']', 0 );
    }
    RIFF_add_json_group_close( pObj, '}', 0 );
}
#endif

#if (DATASAVE_RECOGNITION_RESULTS == 1)
//extern const char recognition_model_string_json[];
//extern int recognition_model_string_json_len;
#include "model_json.h"
#endif

/* write the riff header to the object */
static void write_riff_header( void )
{
    struct riff_object *pObj;
    void *json_start;

    /* we need a block to write into */
    pObj = RIFF_blockpool_allocate( RIFF_block_pool_common(),
                                   MAKE_RIFF_ID( 'Q', 'L', 'S', 'M' ), 0 );
    QL_ASSERT(pObj);
    /* set our block size, we want this to be a FULL block not
     * partial block, so we set this to the maximum size of a block.
     * which is blocksize - header size
     *
     * Recall that a header is 2 32bit values
     */
    RIFF_override_size( pObj, RIFF_BLOCK_SIZE-(2*sizeof(uint32_t)));

    /* sub block */
    RIFF_write_one_u32( pObj, MAKE_RIFF_ID( 'J', 'S', 'O', 'N' ) );
    /* now write len, which will be the *REST* of the block
     *
     * At this point, we have written 2 values during allocation (above)
     * And just wrote the 3rd valu (JSON), and now the length
     *
     * In total, that is 4 values (32bits each) thus the size of
     * the JSON block is  SIZE - (4*sizeof(u32))
     *
     * The JSON block is defined as an ASCII text null terminated
     * string, followed by preferably null bytes until the end
     * end of the RIFF block.
     */
    RIFF_write_one_u32( pObj, RIFF_BLOCK_SIZE - (4*sizeof(uint32_t)));

    RIFF_dma_wr_get( pObj, &json_start, NULL );
    /* the rest is json text as ascii */
    RIFF_add_json_group_open( pObj, '{' );

    /* housekeeping details */
    RIFF_add_json_tag_plain( pObj, s_version, 1, s_two );
    RIFF_add_json_tag_quoted( pObj, "sw_version", 1, "%s", SOFTWARE_VERSION_STR );
    RIFF_add_json_tag_quoted( pObj, "compile_date", 1, "%s", __DATE__ );
    RIFF_add_json_tag_quoted( pObj, "compile_time", 1, "%s", __TIME__ );

    /* Microsoft COM libraries are encoded using mixed endian format. Out of the
       5 components of a GUID the first 3 components of the GUID are little
       endian and the last two are big-endian*/

    RIFF_add_json_tag_quoted( pObj, "start_guid", 1,
                             "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                             file_save_config.guid_bytes[3],
                             file_save_config.guid_bytes[2],
                             file_save_config.guid_bytes[1],
                             file_save_config.guid_bytes[0],
                             file_save_config.guid_bytes[5],
                             file_save_config.guid_bytes[4],
                             file_save_config.guid_bytes[7],
                             file_save_config.guid_bytes[6],
                             file_save_config.guid_bytes[8],
                             file_save_config.guid_bytes[9],
                             file_save_config.guid_bytes[10],
                             file_save_config.guid_bytes[11],
                             file_save_config.guid_bytes[12],
                             file_save_config.guid_bytes[13],
                             file_save_config.guid_bytes[14],
                             file_save_config.guid_bytes[15]);

    /* list of sensors and their configurations */
    RIFF_add_json_tag_group_open( pObj, "sensors", '{');
#if (SENSOR_SSSS_DATASAVE_ENABLED == 1)
    json_add_ssss(pObj);
#endif

#if IMU_DRIVERS
    
    if((datacollection_sensor_status.isAccelEnabled)&&
       (!datacollection_sensor_status.isGyroEnabled))
    {
        json_add_accel(pObj);
    }
    
    //RIFF_sprintf( pObj, ",");
    
    if((!datacollection_sensor_status.isAccelEnabled)&&
       (datacollection_sensor_status.isGyroEnabled))
    {    
        json_add_gyro( pObj );
    }
    
    //RIFF_sprintf( pObj, ",");
    
    if((datacollection_sensor_status.isAccelEnabled)&&
       (datacollection_sensor_status.isGyroEnabled))
    {    
        json_add_accel_gyro( pObj );
    }

    //RIFF_sprintf( pObj, ",");
    //json_add_mag( pObj );
#endif
#if AUDIO_DRIVER
    //RIFF_sprintf( pObj, ",");
    if(datacollection_sensor_status.isAudioEnabled)
    {
        json_add_audio( pObj );
    }
#endif
#if LTC1859_DRIVER
    //RIFF_sprintf( pObj, ",");
    if(datacollection_sensor_status.isADCEnabled)
    {
        json_add_ltc1859a( pObj );
    }
#endif
    RIFF_add_json_group_close( pObj, '}',0); /* close sensor */

#if (DATASAVE_RECOGNITION_RESULTS == 1)
    RIFF_sprintf( pObj, ",\n \"model_description\" : %s\n", recognition_model_string_json);
#endif
    RIFF_add_json_group_close( pObj, '}',0); /* close outside item */

    if( DBG_flags & DBG_FLAG_datasave_debug ){
        dbg_ch( '\n' );
        dbg_str( (const char *)json_start );
        dbg_ch( '\n' );
    }

    /* header is done, write it */
    RIFF_file_enqueue_object( datasave_task_vars.cur_riff_file, pObj );

    /* wake up storage just in case */
    wake_storage_task();
}

static void close_riff_block(struct riff_sensor_obj *pSensor)
{
    /* current RIFF block is done, close it off and flush to disk */
    if( pSensor->cur_block.pObj == NULL ){
        /* there is no block we have nothing to do */
        return;
    }

    RIFF_file_enqueue_object( datasave_task_vars.cur_riff_file, pSensor->cur_block.pObj );
    memset( &(pSensor->cur_block), 0, sizeof(pSensor->cur_block) );
    wake_storage_task();
}

static void cur_block_is_next(struct riff_sensor_obj *pSensor)
{
    /* replace CURRENT block with NEXT block */
    pSensor->cur_block = pSensor->next_block;
    memset( (void *)(&(pSensor->next_block)), 0, sizeof(pSensor->next_block));
}


/* Close off current RIFF block
* Allocate new riff block and initialize it for this sensor
*/
static void new_riff_block( struct riff_sensor_obj *pSensor )
{
    /* move next to current */
    cur_block_is_next(pSensor);
    if (pSensor->sensor_id == SSI_RECOGNITION_RIFF_ID)
    {
        /* we always allocate NEXT block */
        pSensor->next_block.pObj = RIFF_blockpool_allocate( RIFF_block_pool_common(),
                                          MAKE_RIFF_ID( 'R', 'E', 'C', 'O' ), 0 );
    }
    else
    {
    /* we always allocate NEXT block */
    pSensor->next_block.pObj = RIFF_blockpool_allocate( RIFF_block_pool_common(),
                                      MAKE_RIFF_ID( 'S', 'N', 'S', 'R' ), 0 );
    }
    if( pSensor->next_block.pObj== NULL ){
        dbg_str("block-alloc-fail\n");
        return;
    }

    pSensor->sequence_number += 1;

    /* force size */
    RIFF_override_size( pSensor->next_block.pObj, RIFF_BLOCK_SIZE-(2*sizeof(uint32_t)));

    /* sub-Riff object 1 is the sensor details */

    /* tag */
    /* offset = 0 */
    RIFF_write_one_u32( pSensor->next_block.pObj, MAKE_RIFF_ID( 'H', 'D', 'R', '!' ) );
    /* len */
    /* offset = 1 */
    RIFF_write_one_u32( pSensor->next_block.pObj,
                       sizeof(uint32_t) +
                       sizeof(uint32_t) +
                       sizeof(uint64_t) +
                       sizeof(uint64_t) );
    /* offset = 2 */
    RIFF_write_one_u32( pSensor->next_block.pObj, pSensor->sensor_id );
    /* offset = 3 */
    RIFF_write_one_u32( pSensor->next_block.pObj, pSensor->sequence_number );

    /* offset = 4 & 5 */
    /* remember this timestamp pointer for later when we write data */
    RIFF_dma_wr_get( pSensor->next_block.pObj, (void **)(&(pSensor->next_block.pTimeStamps)), NULL );
    /* write this later, use 0 for now */
    RIFF_write_one_u64( pSensor->next_block.pObj, 0 );

    /* offset = 6 & 7*/
    /* Write "ENDTIME" later, use 0 for now */
    RIFF_write_one_u64( pSensor->next_block.pObj, 0 );

    /* RIFF object 2 is the data */
    /* offset = 8 */
    RIFF_write_one_u32( pSensor->next_block.pObj, MAKE_RIFF_ID( 'D', 'A', 'T','A' ) );
    /* data goes next */
    /* offset = 9 */

    /* Remember where in the block LENGTH value for the DATA element is located
     * this is so we can write or update the length later.
     */
    RIFF_dma_wr_get( pSensor->next_block.pObj, (void **)(&(pSensor->next_block.pDataLen)), NULL );
    /* write length, currently 0 no data */
    RIFF_write_one_u32( pSensor->next_block.pObj, 0 );
    if( DBG_flags & DBG_FLAG_datasave_debug ){
        dbg_str_now();
        dbg_str_hex32( "new-riff", pSensor->sensor_id );
        dbg_str_hex32( "block-ptr", (uint32_t)(pSensor->next_block.pObj) );
    }

    if( pSensor->cur_block.pObj == NULL ){
        /* first time, get two blocks both cur & next */
        new_riff_block(pSensor);
    }
}



/* find riff object this sensor is using, if needed allocate one */
static struct riff_sensor_obj *find_riff_object( uint32_t sensor_id )
{
    int x;
    struct riff_sensor_obj *pSensor;

    /* (common) pass 1 find an existing riff block
     * Either we find one or
     * (rare) we have to allocate one and make another pass.
     */
    for( x = 0 ; x < N_RIFF_OBJS ; x++ ){
        pSensor = &(datasave_task_vars.all_sensor_objs[x]);
        if( pSensor->sensor_id == 0 ){
            /* skip unused */
            continue;
        }
        if( pSensor->sensor_id == sensor_id ){
            /* match? Success we are done */
            return pSensor;
        }
    }

    /* no EXISTING found must make one */;
    for( x = 0 ; x < N_RIFF_OBJS ; x++){
        pSensor = &datasave_task_vars.all_sensor_objs[x];
        if( pSensor->sensor_id != 0 ){
            /* skip used slots */
            continue;
        } else {
            /* found */
            break;
        }
    }

    /* too many? */
    if( x >= N_RIFF_OBJS ){
        dbg_str("ERR: no riff blocks!\n");
        return NULL;
    }
    /* save the sensor id so we can find it next time */
    pSensor->sensor_id = sensor_id;
    pSensor->sequence_number = 0;
    memset( (void *)(&pSensor->cur_block), 0, sizeof(pSensor->cur_block) );
    memset( (void *)(&pSensor->next_block), 0, sizeof(pSensor->cur_block) );
    return pSensor;
}

/* this performs saving of data to a RIFF block file */
void data_save( const struct sensor_data *pInfo )
{
    int space_avail;
    struct riff_sensor_obj *pSensor;
    size_t n_done;
    size_t n_this;

    // Check if there is imu sensor (A,G,A_G_SEP,A_G_COM) added
    //if ( !is_sensor_active(pInfo->sensor_id, IMU_DATA_COLLECT) )
    {
    //    return;
    }

    /* if no data to save */
    if( pInfo->n_bytes == 0 ){
        return;
    }

    /* sanity check */
    if( pInfo->n_bytes % pInfo->bytes_per_reading ){
        dbg_fatal_error("riff-non-reading-multiple\n");
    }

    /* sanity check */
    if( pInfo->vpData == NULL ){
        dbg_fatal_error("riff-data-ptr-err\n");
    }

    /* sanity check */
    /* we might get audio chunks that are large */
    if( pInfo->n_bytes  > ((3*RIFF_BLOCK_SIZE)/4) ){
        /* something is very wrong */
        dbg_fatal_error("riff-overflow?");
    }

    /* if we are not saving data - then leave now */
    if( !(iop_globals.cur_status.bit_flags & COLLECT_ACTIVE) ){
        return;
    }
    
    /* if the save task is not ready yet (ie: file open, header written) leave */
    if( datasave_task_vars.cur_state != DATASAVE_run ){
        return;
    }

    /* find the current riff block for this sensor */
    /* also allocates as needed */
    pSensor = find_riff_object(pInfo->sensor_id);
    if( pSensor == NULL ){
        iop_globals.cur_status.collect_oe_count += 1;
        dbg_str("drop-data-no-buff1\n");
        return;
    }

    /*
     * Add this data to the riff block.
     *
     * FIXME (TODO) optimization
     *    If needed, split this block in two parts.
     *    Part (A) goes into this block the current block.
     *    Part (B) goes into the next block
     *
     * Remember we'll need to recaculate start/end times if that happens.
     */
    n_done = 0;
    while( n_done < pInfo->n_bytes ){
        /* how much space is there? */
        if( pSensor->cur_block.pObj ){
            space_avail = RIFF_space_avail( pSensor->cur_block.pObj );
        } else {
            space_avail = 0;
        }

        /* can I put all of this in here? */
        n_this = (pInfo->n_bytes-n_done);
        if( n_this > space_avail ){
	  /* FIXME:
	   *  Optimization - can we fit *some* of this data?
	   *  If so - split up and into two chunks.
	   *  And calculate new start/end time for the two chunks.
	   */

	  /* FOR NOW - easy route... */
	  /* it does not fit, get a new block */
            close_riff_block(pSensor);
            new_riff_block(pSensor);
            if( pSensor->cur_block.pObj == NULL ){
                /* allocation failed? */
                iop_globals.cur_status.collect_oe_count += 1;
                pSensor->sequence_number += 1;
                dbg_str("drop-data-no-buff2\n");
                return;
            }
            /* This is a new block so write the START timestamp */
            pSensor->cur_block.pTimeStamps[0] = pInfo->time_start;
            continue;
        }
        /* it fits, write data */
        RIFF_write_vp(pSensor->cur_block.pObj,
                    (const char *)(pInfo->vpData)+n_done,
                    n_this,
                    0 );
        /* update the byte count */
        pSensor->cur_block.pDataLen[0] += pInfo->n_bytes;
        /* update *END* counter */
        pSensor->cur_block.pTimeStamps[1] = pInfo->time_end;
        n_done += n_this;
        continue;
    }
}

#if (RIFF_FILE_SIZE_MAX > 0)
/** currentFileSize keeps track of the number of bytes added to the current
 *  current RIFF file where the sensor data is being written.
 */
static uint32_t currentFileSize = 0;
#endif

/* drain all waiting blocks to the sd card */
static void drain_blocks(void)
{
    int r;
    int had_error;
    /* we loop until all data in queue is written to sd card or error occurs */
    for(;;){

        had_error = 0;
        if( DBG_flags & DBG_FLAG_datasave_debug ){
            dbg_str("q-svc\n");
        }

#if (0) && (RIFF_FILE_SIZE_MAX > 0)
        if (currentFileSize > RIFF_FILE_SIZE_MAX)
        {
            dbg_str_int("riff file size limit reached", currentFileSize);
            currentFileSize = 0;
            close_riff_file();

            // Open a new RIFF file
            open_riff_file();
        }
#endif

	/* Service at least 1 block */
        r = RIFF_busy_queue_service( RIFF_block_pool_common(), &had_error );

        if( had_error ){
            dbg_str("riff-drain-file-error\n");
            set_sys_error( SYS_ERR_EPERM, 'D' );
            iop_globals.cur_status.bit_flags &= ~COLLECT_ACTIVE;
        }

	/* done? */
        if( r == EOF ){
   	    /* queue is empty, nothing to do */
            if( DBG_flags & DBG_FLAG_datasave_debug ){
                dbg_str("q-done\n");
            }
            break;
        } else {
  	  /* we wrote 1 block, loop and do more */
            if( DBG_flags & DBG_FLAG_datasave_debug ){
                dbg_str("q-wrote\n");
            }
            iop_globals.cur_status.bytes_saved += RIFF_BLOCK_SIZE;
#if (RIFF_FILE_SIZE_MAX > 0)
            currentFileSize += RIFF_BLOCK_SIZE;
#endif
        }
    }
}

/* close off the RIFF file */
int close_riff_file(void)
{
    int retStatus = -1;
    int x;
    struct riff_sensor_obj *pSensor;

    if( DBG_flags & DBG_FLAG_datasave_debug ){
        dbg_str_str("riff-close-start", file_save_config.cur_filename );
    }

    /* step1 - move all pending blocks into the busy queue */
    for( x = 0 ; x < N_RIFF_OBJS ; x++ ){
        pSensor = &datasave_task_vars.all_sensor_objs[x];
        if( pSensor->sensor_id == 0 ){
            continue;
        }
        close_riff_block(pSensor);

        /* release the 'next' block' we don't need it, nothing is in it */
        if( pSensor->next_block.pObj ){
            RIFF_block_pool_discard( pSensor->next_block.pObj );
        }
	/* zap the next block */
        memset( (void *)(&(pSensor->next_block)), 0, sizeof( pSensor->next_block ) );
    }

    /* step 2 write all blocks to file */
    drain_blocks();

    /* step 3 close the file */
    dbg_str_str("\nclose", file_save_config.cur_filename );
    /* retStatus = */ RIFF_file_close( datasave_task_vars.cur_riff_file );

    /* mark file as closed */
    datasave_task_vars.cur_riff_file = NULL;

    if( DBG_flags & DBG_FLAG_datasave_debug ){
        dbg_str_str("riff-close-end", file_save_config.cur_filename );
    }
    return retStatus;
}


/* open and write the riff header block */
int open_riff_file(void)
{
    int retStatus = -1;
    struct riff_file *fp;

    /* close previous */
    if( datasave_task_vars.cur_riff_file ){
        close_riff_file();
    }

    /* set default just in case DCL did not send */
    if( file_save_config.cur_filename_template[0] == 0 ){
#if (USE_FREERTOS_FAT_APIS == 1)
    	snprintf(file_save_config.cur_filename_template, 20, "%s/%s", DiskDefaultMountPoint, "data");
#else
        strcpy( file_save_config.cur_filename_template, "data" );
#endif
    }

    /* create or construct the filename */
    construct_filename();

    dbg_str_str("datafile", file_save_config.cur_filename);

    /* open the file */
    fp = RIFF_file_open( file_save_config.cur_filename, "w");
    if( DBG_flags & DBG_FLAG_datasave_debug ){
        dbg_str_ptr("datafile-handle", (void *)(fp) );
    }

    if( fp== NULL ){
        dbg_str_str("riff-open-err", file_save_config.cur_filename );
        set_sys_error( SYS_ERR_EPERM , 'O'   );
    } else {
        datasave_task_vars.cur_riff_file = fp;
	/* HEADER ... */
        write_riff_header();
        retStatus = 0;
    }
#if (RIFF_FILE_SIZE_MAX > 0)
    currentFileSize = 0;
#endif
    return retStatus;
}

extern const struct sensor_config_msg datacapture_default_config[];
extern int app_datastorage_start_stop(int is_start, const uint8_t *pGuid, size_t guid_len);
static uint8_t datastorage_guid[16];
/*
 * This saves data to the SD Card or SPI flash
 * (depends on the default mounted filesystem)
 */
void DataSaveTaskHandler(void *pData)
{
#if (USE_FATFS_APIS == 1)
	FRESULT fr;
#endif
    wait_ffe_fpga_load();

#if !S3AI_FIRMWARE_IS_RECOGNITION
    /*
     * in the recognition case - the recognizer configures sensors
     * in all other cases, this task does that work
     */
    //sensor_config_apply_sequence( datacapture_default_config, &datacollection_sensor_status );
#endif

#if ( (SENSOR_SSSS_DATASAVE_ENABLED == 1) || \
      (SENSOR_AUDIO_DATASAVE_ENABLED == 1) \
    )
  app_datastorage_start_stop(1, datastorage_guid, 16);
#endif

    for(;;){

        xSemaphoreTake( datasave_task_vars.wait_sem, 1000 );

	/* this is a statemachine */
        switch( datasave_task_vars.cur_state ){
        case DATASAVE_init:
            if( DBG_flags & DBG_FLAG_datasave_debug ){
                dbg_str("save-init\n");
            }
#if (USE_FATFS_APIS == 1)
            fr = f_mount(&FatFsObj, DiskDefaultMountPoint, 1);
            if (fr != FR_OK)
            {
            	dbg_str_int("Error: FatFs mount failed", fr);
            }
#elif (USE_FREERTOS_FAT_APIS == 1)
            FF_Disk_t *pDisk = NULL;
            if (DEFAULT_STORAGE_LOCATION == FREERTOS_SPI_SD)
               pDisk = FF_SPISDMount(DiskDefaultMountPoint);
            //else if (DEFAULT_STORAGE_LOCATION == FREERTOS_SPI_FLASH)
            //   pDisk = FF_SPIFlashMount(DiskDefaultMountPoint);
            if (pDisk == NULL)
            {
            	dbg_str("Error: FreeRTOS FAT mount failed");
            }
#else /* USE_QLFS_APIS */
            QLFS_mount_as_default( DEFAULT_STORAGE_LOCATION );
#endif /* USE_FATFS_APIS */
            RIFF_block_pool_init( RIFF_block_pool_common(),
                         riff_bytebuffer,
                         sizeof(riff_bytebuffer),
                         RIFF_BLOCK_SIZE );

            datasave_task_vars.cur_state = DATASAVE_idle;
            break;
        case DATASAVE_idle:
  	  /* Is it time to do something? Or do we go back to sleep? */
            if( DBG_flags & DBG_FLAG_datasave_debug ){
                dbg_str("save-idle\n");
            }
            if( !(iop_globals.cur_status.bit_flags & COLLECT_ACTIVE) ){
                continue;
            }
	    /* We do something! */
            if( DBG_flags & DBG_FLAG_datasave_debug ){
                dbg_str("save-open\n");
            }
	    /* open our file */
            open_riff_file();
            if( datasave_task_vars.cur_riff_file != NULL ){
	      /* if success ... turn on sensors */
                datasave_task_vars.cur_state = DATASAVE_run;
                //sensors_all_startstop(1);
            } else {
	      /* ERROR - auto-turn the save bit off because we failed */
                iop_globals.cur_status.bit_flags ^= COLLECT_ACTIVE;
            }
            break;
        case DATASAVE_run:
	  /* this is the run state
	   * We wake up here and need to determine
	   * (A) is there data to write
	   * (B) should we stop writting?
	   */
            if( DBG_flags & DBG_FLAG_datasave_debug ){
                dbg_str("datasave-drain\n");
            }

	    /* drain/clean the queues */
            drain_blocks();

            /* where we told to stop? */
            if( !(iop_globals.cur_status.bit_flags & COLLECT_ACTIVE) ){
                /* we've been told to stop */
                if( DBG_flags & DBG_FLAG_datasave_debug ){
                    dbg_str("datasave-close\n");
                }
		/* Shut down, stop accepting data */
                datasave_task_vars.cur_state = DATASAVE_idle;
                //sensors_all_startstop(0);
                close_riff_file();
                if( DBG_flags & DBG_FLAG_datasave_debug ){
                    dbg_str("datasave-done\n");
                }
            }
            break;
        }
    }
}

/* DCL has told us to start or stop saving data */
int app_datastorage_start_stop(int is_start, const uint8_t *pGuid, size_t guid_len)
{
    /* we basically set flags, and let the data save task do the work. */

    if( !is_start ){
        if( DBG_flags & DBG_FLAG_datasave_debug ){
            dbg_str("storage: stop\n");
        }

        /* CLEAR */
        iop_globals.cur_status.bit_flags &= ~COLLECT_ACTIVE;
    } else {
        if( DBG_flags & DBG_FLAG_datasave_debug ){
            dbg_str("storage: start\n");
        }
        /* SET */
        iop_globals.cur_status.bit_flags |= COLLECT_ACTIVE;
        if( guid_len > sizeof( file_save_config.guid_bytes ) ){
            guid_len = sizeof( file_save_config.guid_bytes );
        }
        memcpy( (void *)file_save_config.guid_bytes,
               (const void *)pGuid,
               guid_len );
    }

    /* WAKE */
    wake_storage_task();
    return 0;
}

/*
 * This is here to stop compiler warnings.
 * If something is disabled, then one of these
 * small strings might not be used.. and we get
 * a warning -
 * By putting them in this table... we don't get the warning.
 * We then use the table below...
 */
static const char * const use_this_table[] = {
    s_zero,
    s_one,
    s_enabled,
    s_sensor_id,
    s_name,
    s_version,
    s_titles,
    s_error,
    s_rate,
    s_range,
    s_gain,
    s_accel_xyz,
    s_ch,
    s_nbits,
    s_3h,
    s_config,
    s_nchannels,
    s_format,
    NULL
};

static uint64_t prev_rec_start_time = 0;
static uint64_t curr_rec_start_time = 0;
void
create_datasave_task(void)
{
    int x;

    static UINT8_t ucParameterToPass;


    /* this is here to get rid of "unused warnings"
     * when things are disabled, we need to use the table of strings
    */
    x = (int)(use_this_table[0]);
    /* this will never fail, as the first element is always a non-null pointer */
    configASSERT( x );

    datasave_task_vars.cur_state = DATASAVE_init;
    datasave_task_vars.wait_sem = xSemaphoreCreateCounting( 0x7fffffff, 1 );
    vQueueAddToRegistry( datasave_task_vars.wait_sem, "DS_wait" );

    /* Create DataSave Task */
    xTaskCreate(DataSaveTaskHandler,
                "DataSaveTaskHandler",
                STACK_SIZE_ALLOC(STACK_SIZE_TASK_DATASAVE),
                &ucParameterToPass,
                PRIORITY_TASK_DATASAVE,
                &datasave_task_vars.task_handle);
    configASSERT(datasave_task_vars.task_handle);
}

void data_save_recognition_results(char *sensor_ssss_ai_result_buf, int wbytes)
{
    struct sensor_data Info, *pInfo = &Info;
#if (SENSOR_SSSS_DATASAVE_ENABLED == 1)
    pInfo->rate_hz = sensor_ssss_config.rate_hz; // what should go here?
#endif

#if (SENSOR_AUDIO_DATASAVE_ENABLED == 1)
    pInfo->rate_hz = sensor_audio_config.rate_hz; // what should go here?
#endif
    uint64_t curr_time = convert_to_uSecCount(xTaskGetTickCount());
    pInfo->bytes_per_reading = 1;
    pInfo->n_bytes = wbytes;
    pInfo->sensor_id = SSI_RECOGNITION_RIFF_ID; // use a fixed ID for recognition results
    pInfo->time_end = curr_time; // convert_to_uSecCount(pIn->dbHeader.Tend);
    pInfo->time_start = prev_rec_start_time; // convert_to_uSecCount(pIn->dbHeader.Tstart);
    pInfo->vpData = sensor_ssss_ai_result_buf;
    data_save((const struct sensor_data *)pInfo);
    prev_rec_start_time = curr_time;
    return;
}

void set_recognition_start_time(void)
{
	prev_rec_start_time = convert_to_uSecCount(xTaskGetTickCount());
	return;
}

void set_recognition_current_block_time(void)
{
	prev_rec_start_time = curr_rec_start_time;
	curr_rec_start_time = convert_to_uSecCount(xTaskGetTickCount());
	return;
}

#endif

