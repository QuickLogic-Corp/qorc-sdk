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

#ifndef __SENSIML_DATACOLLECTION_TASK_H__
#define __SENSIML_DATACOLLECTION_TASK_H__

#if !defined( _EnD_Of_Fw_global_config_h )
#error "Include Fw_global_config.h first"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#if QL_SDCARD_SUPPORT_H
#include "ff.h"
#endif
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "common.h"
#include "test_types.h"
#include "ql_fs.h"
#include "ble_collection_defs.h"
#include "sensor_config.h"

#include "ql_riff.h"

#include "QL_Trace.h" /* for QL_status */

#ifdef __cplusplus
extern "C" {
#endif

#define SENSIML_DCL_MSGQ_WAIT_TIME	portMAX_DELAY

//#include "sensor_config.h"

#define SENSIML_DCL_QUEUE_LENGTH  10  //msg queue size for AirDrawTaskHandler
#define GUID_BYTES_MAX            16

#define SENSIML_MAX_BLE_SEND_MOTION_HZ 104

struct ble_data_config {
  /* fix me, today we do Accel & Gygro (A&G)
   * FUTURE need to support seperate selection
   * and need to support Magnometer 
   */

  /*
   * About "counter" & "reload"
   * These emulate a hardware counter/timer.
   *
   * And these are used to reduce the data rate
   * going to the BLE interface.
   *
   * BLE can only accept about 100hz worth of data.
   * Thus, if the sensor is running at 208hz
   * We send every 2nd value.
   * If the sensor is running at 1600hz, 
   * we send every 16th value.
   *
   * When this "divide" by operation is active,
   * the "reload" value is NON_ZERO.
   *
   * The "count" value is a down counter.
   * Initially the COUNTER is set to the RELOAD value.
   * And begins to count down ... eventually reaching 0.
   *
   * The 1->0 transition indicates time to send data via BLE.
   * And the COUNTER is reloaded with the RELOAD value.
   *
   * There are two pseudo counter:
   * One for the IMU (accel & gyro)
   * one for the "selected sensor"
   *   Which can be any selected sensor.
   */
   // TODO imu counter and reload in this struct are replaced by
   // count and reload in Sensor_live_t. Beside, ble_data_config is no longer needed,
   // Remove when Merced and all others don't use it !
  int imu_data_rate_counter;
  int imu_data_rate_reload;

  /* Major = The sensor id */
  uint32_t sensor_data_id_major;

  /* Minor = the channel within that sensor 
   *
   * Some sensors have multiple channels, 
   * and we may (or may not) need to select 
   * a specific channel value.
   *
   * That is the purpose of the MINOR value.
   *
   * If the ID_MINOR = 0xFFFFFFFF - then all channels should be sent.
   * If the ID_MINOR is anything other then 0xFFFFFFFF
   * That is value value index within the data to send.
   *
   * ie: the value might be 3, which would mean the 4th sub channel
   */
  uint32_t sensor_data_id_minor;

  /* timer variables for the selected sensor */
  int sensor_data_rate_counter;
  int sensor_data_rate_reload;
};
  
extern struct ble_data_config ble_data_config;

struct file_save_config {
  /*
   * How are we saving data
   */

  /* base filename */
  char cur_filename_template[20];
  /* computed filename (with timestamp as ascii) */
  char cur_filename[ QLFS_MAX_ABS_PATHLEN ];

  /* option flags from host, today options are 0 nothing.
   * future might select USB or SPI instead of SD CARD.
   */
  uint32_t optionflags;

  /* GUID to from the start command to be written to the JSON header */
  uint8_t guid_bytes[GUID_BYTES_MAX];
};

extern struct file_save_config file_save_config;

void sensor_audio_startstop(int is_start);

struct audio_config {
    int is_running;
    int n_channels_enabled;
    uint32_t sample_rate_hz;
    int      nbits;
    /* if mic_config==0 disabled.
    * if mic_config==1 normal operation
    * otherwise - "some custom configuration" TBD.
    */
#define S3_MAX_AUDIO_MICROPHONE 4
    uint8_t  mic_config[S3_MAX_AUDIO_MICROPHONE];
};

extern struct audio_config audio_config;

struct imu_config {
    int is_running; 
    struct imu_detail {
        uint32_t rate_hz;
        int range;
        int enabled;
    } accel, gyro, mag;
    IMU_VIRTUAL_SENSOR_t v_sensor;
};

extern struct imu_config  imu_config;

void sensor_gyro_configure(void);
void sensor_accel_configure(void);

void sensor_imu_clear(uint32_t sensor_id);
void sensor_imu_add(uint32_t sensor_id);

//TODO we will come back to consolidate
//sensor_accel_configure(void);
//sensor_gyro_configure(void);
//sensor_mag_configure(void);

void sensor_mag_clear( void );
void sensor_mag_add( void );
void sensor_mag_configure(void);

void sensors_all_startstop( int is_start );

struct ltc1859a_config {
    int enabled;
    uint32_t rate_hz;
#define LTC1859_N_CHANNNELS 8
    int n_channels_enabled;
    uint8_t ch_config[LTC1859_N_CHANNNELS];
};

void create_datasave_task(void);

void sensor_ltc1859a_clear( void );
void sensor_ltc1859a_add( void );
void sensor_ltc1859a_configure(void);

/* params for ltc & audio are in the sensor message */
void sensor_audio_add( void );
void sensor_audio_clear( void );
void sensor_audio_configure(void);

/* audio wave struct */
#define PCM_FMT_CHUNK_SIZE  16
#define PCM_NUM_CHANNELS 1
#define PCM_BYTES_PER_SAMPLE 2
#define PCM_FMT_NUM 1
#define AUDIO_FREQ  16000
#define PCM_BIT_DEPTH 16
typedef struct wav_header {
    // RIFF Header
    char riff_header[4]; // Contains "RIFF"
    int wav_size; // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    char wave_header[4]; // Contains "WAVE"

    // Format Header
    char fmt_header[4]; // Contains "fmt " (includes trailing space)
    int fmt_chunk_size; // Should be 16 for PCM
    short audio_format; // Should be 1 for PCM. 3 for IEEE Float
    short num_channels;
    int sample_rate;
    int byte_rate; // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
    short sample_alignment; // num_channels * Bytes Per Sample
    short bit_depth; // Number of bits per sample

    // Data
    char data_header[4]; // Contains "data"
    int data_bytes; // Number of bytes in data. Number of samples * num_channels * sample byte size
    // uint8_t bytes[]; // Remainder of wave file is bytes
} wav_hdr_t;


extern int CheckDataCollectionReady(void); //return 0- if not ready

/* tell audio to start or stop */
extern int audio_start_stop( int is_start );

extern SensorEnableStatus datacollection_sensor_status;

extern void watch_imu_data(void);

struct sensor_data {
    /* Rate in HZ the sensor is operating at */
    uint32_t rate_hz;
    
    /* ID of the sensor */
    uint32_t sensor_id;
    
    /* uSECs of the first reading in this block of data */
    uint64_t time_start;
    
    /* uSECS of last reading in this block of data */
    uint64_t time_end;
    
    /* The data it self */
    const void *vpData;
    
    /* how many bytes are in this block of data */
    int n_bytes; 
    
    /* How many bytes per reading, ie: IMU has 3 shorts, ie: 6 bytes */
    size_t   bytes_per_reading;
};

extern void data_save( const struct sensor_data *pSensorData );
int app_datastorage_start_stop(int is_start, const uint8_t *pGuid, size_t guid_len);
extern int open_riff_file(void);
extern int close_riff_file(void);

/* if "start time is zero" - back calculate the start time */
void back_calculate_start_time( struct sensor_data *pInfo );

extern void ble_send( const struct sensor_data *pSaveInfo );

void my_ble_callback( uint8_t *pData );

void set_recognition_start_time(void);
void set_recognition_current_block_time(void);
void data_save_recognition_results(char *sensor_ssss_ai_result_buf, int wbytes);

#ifdef __cplusplus
}
#endif

#endif  /* __SENSIML_DATACOLLECTION_TASK_H__ */
