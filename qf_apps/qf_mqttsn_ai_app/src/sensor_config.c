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
//#include "FFE_AccelGyro.h"
#include "DataCollection.h"
#include "ql_fs.h"
#include "ql_adcTask.h"
#include "ql_time.h"
//#include "eoss3_hal_rtc.h"
#include "dbg_uart.h"
#include "Mqttsn_Topics.h"
#include "sensor_config.h"

#include "sensor_ad7476_config.h"

extern void sensor_imu_startstop( int is_start );

SensorEnableStatus datacollection_sensor_status;
static int sensors_configured = 0;

/* in data capture mode,
 * this is the default configuration
 */
const struct sensor_config_msg datacapture_default_config[] = {
        SENSOR_CONFIG_CLEAR_MSG(),
        SENSOR_CONFIG_IMU_2_MSG(SENSOR_ENG_VALUE_ACCEL_GYRO,104,20,0),
        //SENSOR_CONFIG_IMU_MSG(SENSOR_ENG_VALUE_ACCEL,104,20),
        //SENSOR_CONFIG_IMU_MSG(SENSOR_ENG_VALUE_GYRO,104,0),
        SENSOR_CONFIG_DONE_MSG()
};

struct sensor_config_msg sensor_config_msg;
     
/* given an END sample time, and a rate
 * determine when the first sample occured.
 */
void back_calculate_start_time( struct sensor_data *pInfo )
{
    int n_samples;
    int n_usecs;
    if( pInfo->time_start != 0 ){
        return;
    }

    /* calculate how many samples are present */
    n_samples = pInfo->n_bytes / pInfo->bytes_per_reading;
    
    /* Given the rate - calculate how many microseconds
     * this many samples took, rounding up
     */
    n_usecs = ((n_samples * 1000000) + (pInfo->rate_hz/2)) / pInfo->rate_hz;

    /* and thus the start time is the END minus that many uSecs */
    pInfo->time_start = pInfo->time_end - n_usecs;
}


void configure_all_sensors(Mqttsn_IOMsgData_t *pIoMsgData)
{
#if IMU_M4_DRIVERS
    sensor_accel_configure();
    sensor_gyro_configure();
    sensor_mag_configure();
#endif
#if AD7476_FPGA_DRIVER
    sensor_ad7476_configure();  // sensor_adc_fpga_configure();
    sensor_ad7476_startstop(1); // sensor_adc_fpga_startstop(1);
#endif
#if AUDIO_DRIVER
    sensor_audio_configure();
#endif
}

/*
 * Start or stop all sensor operation.
 * If the "is_start" is zero - we are stopping.
 *
 * Else, we are starting (non-zero)
 */
void sensors_all_startstop( int is_start )
{
#if AUDIO_DRIVER
    sensor_audio_startstop( is_start );
#endif
#if IMU_M4_DRIVERS
    sensor_imu_startstop( is_start );
#endif
#if AD7476_FPGA_DRIVER
    //sensor_adc_fpga_startstop( is_start );
    sensor_ad7476_startstop( is_start );
#endif
}

/* clear stop all sensors */
void sensor_clear(SensorEnableStatus *pSensorStatus)
{
    memset( (void *)(pSensorStatus), 0, sizeof(*pSensorStatus) );
    sensor_set_virtual_sensor(IMU_V_SENSOR_NO);
#if IMU_M4_DRIVERS
    sensor_imu_clear(SENSOR_ENG_VALUE_ACCEL);
    sensor_imu_clear(SENSOR_ENG_VALUE_GYRO);
    sensor_mag_clear();
#endif
#if AUDIO_DRIVER
    sensor_audio_clear();
#endif
#if AD7476_FPGA_DRIVER
    //sensor_adc_fpga_clear();
    sensor_ad7476_clear();
#endif
}

#if 0 // ADC_FPGA_DRIVER
/* these must be the same, cross check */
#if (HAL_SENSOR_ID_LTC1859 != SENSOR_ADC_LTC_1859_MAYHEW)
#error LTC1859 sensor id is not consistent with HAL layer
#endif
#if HAL_SENSOR_ID_AD7476  != SENSOR_ADC_AD7476
#error AD7476 sensor id is not consistent with HAL layer
#endif

void sensor_ad7476_add(void)
{
    memset( (void *)(&adc_fpga_task_config), 0,sizeof(adc_fpga_task_config));
    if( sensor_config_msg.sensor_common.sensor_id != HAL_SENSOR_ID_AD7476 ){
        dbg_fatal_error("INVALID sensor id for adc");
    }
    adc_fpga_task_config.sensor_id = HAL_SENSOR_ID_AD7476;
    dbg_str_hex32("add-adc-ad7476", adc_fpga_task_config.sensor_id );
    adc_fpga_task_config.frequency = 
        sensor_config_msg.sensor_common.rate_hz;
    adc_fpga_task_config.ad7476.param0 = 
        sensor_config_msg.unpacked.ad7476.param0;
    adc_fpga_task_config.ad7476.param1 =
        sensor_config_msg.unpacked.ad7476.param1;
    _sensor_ad7476_add();
}

void sensor_ltc1859a_add(void)
{
    int x;
    int enable_bits;

    memset( (void *)(&adc_fpga_task_config), 0,sizeof(adc_fpga_task_config));
    
    adc_fpga_task_config.frequency = 
        sensor_config_msg.sensor_common.rate_hz;

    /* FIXME future support 8 channels */
    enable_bits = 0;
   
    for( x = 0 ; x < LTC1859_N_CHANNNELS  ; x++ ){
        adc_fpga_task_config.ltc1859.chnl_commands[x] = 1; /* disable */
    }
    /* ble only supports channels 0 to 3 */
    for( x = 0 ; x < 4 ; x++ ){
        if( sensor_config_msg.unpacked.ltc1859_a.chnl_config[x] & 1 ){
            /* disabled */
        } else {
            adc_fpga_task_config.ltc1859.chnl_commands[x] = 
                sensor_config_msg.unpacked.ltc1859_a.chnl_config[x];
            enable_bits |= (1 << x);
        }
    }
    
    if( (enable_bits >= 1) && (enable_bits  <= 0x0f) ){
        /* all is well */
    } else {
        enable_bits = 0;
    }

    adc_fpga_task_config.ltc1859.channel_enable_bits = enable_bits;
    if( enable_bits ){
        sensor_adc_fpga_configure();
    }
}
#endif

/**
 * @brief get current virtual sensor type.
 **/
IMU_VIRTUAL_SENSOR_t sensor_get_virtual_sensor(void)
{
    return imu_config.v_sensor;
}

/**
 * @brief Set virtual sensor type.
 *        To clear virtual sensor type, use IMU_V_SENSOR_NO.
 * @note  The machnism is to make the highest efficient packet has the highest priority.
 *        No reverse machnism is defined. This means to get back to lower efficient sending
 *        clear has to be done, and sensor will be added back again. 
 * @key   This function should be called with sensor add so that the virtual seosor matches
 *        physical sensor.
 * @param[in] v_sensor : Desired virtual sensor type.
 *
 * @retutn : The actual virtual sensor type being set.
 **/
IMU_VIRTUAL_SENSOR_t sensor_set_virtual_sensor(IMU_VIRTUAL_SENSOR_t v_sensor)
{

    // reset virtual sensor type
    if (v_sensor == IMU_V_SENSOR_NO)
    {
        imu_config.v_sensor = v_sensor;
        return imu_config.v_sensor;
    }

    switch (imu_config.v_sensor)
    {
        case IMU_V_SENSOR_NO:
            imu_config.v_sensor = v_sensor;
            break;
        case IMU_V_SENSOR_ACCEL:
            if ( (v_sensor == IMU_V_SENSOR_GYRO) || (v_sensor == IMU_V_SENSOR_A_G_SEP) )
                imu_config.v_sensor = IMU_V_SENSOR_A_G_SEP;
            else if (v_sensor == IMU_V_SENSOR_A_G_COM)
                imu_config.v_sensor = IMU_V_SENSOR_A_G_COM;
            // else (IMU_V_SENSOR_ACCEL) nothing changed
            break;
        case IMU_V_SENSOR_GYRO:
            if ( (v_sensor == IMU_V_SENSOR_ACCEL) || (v_sensor == IMU_V_SENSOR_A_G_SEP) )
                imu_config.v_sensor = IMU_V_SENSOR_A_G_SEP;
            else if (v_sensor == IMU_V_SENSOR_A_G_COM)
                imu_config.v_sensor = IMU_V_SENSOR_A_G_COM;
            // else (IMU_V_SENSOR_GYRO) nothing changed
            break;
        case IMU_V_SENSOR_A_G_SEP:
            if (v_sensor == IMU_V_SENSOR_A_G_COM)
                imu_config.v_sensor = IMU_V_SENSOR_A_G_COM;
            // else ( (v_sensor == IMU_V_SENSOR_ACCEL) ||
            //        (v_sensor == IMU_V_SENSOR_GYRO)  ||
            //        (v_sensor == IMU_V_SENSOR_A_G_SEP) )
            break;
        default:
            // we reach the highest efficient virtual sensor type IMU_V_SENSOR_A_G_COM
            break;
    }
    return imu_config.v_sensor;
}

/* handle ADD from BLE or from RECOGNITION- */
void sensor_add(SensorEnableStatus *pSensorStatus)
{
  /* todo, add more sensors here */
    switch( sensor_config_msg.sensor_common.sensor_id ){
#if IMU_M4_DRIVERS
    case SENSOR_ENG_VALUE_ACCEL:
        // We use virtual sensor to control streaming samples.
        sensor_set_virtual_sensor(IMU_V_SENSOR_ACCEL);
        sensor_imu_add(SENSOR_ENG_VALUE_ACCEL);
        pSensorStatus->isAccelEnabled = TRUE;
        pSensorStatus->isIMUEnabled = TRUE;
        //CLI_printf("ACCEL is added\n");
        break;
    case SENSOR_ENG_VALUE_GYRO:
        // We use virtual sensor to control streaming samples.
        sensor_set_virtual_sensor(IMU_V_SENSOR_GYRO);
        sensor_imu_add(SENSOR_ENG_VALUE_GYRO);
        pSensorStatus->isGyroEnabled = TRUE;
        pSensorStatus->isIMUEnabled = TRUE;
        //CLI_printf("GYRO is added\n");
        break;
    case SENSOR_ENG_VALUE_ACCEL_GYRO:
        // We use virtual sensor to control streaming samples.
        sensor_set_virtual_sensor(IMU_V_SENSOR_A_G_COM);
        sensor_imu_add(SENSOR_ENG_VALUE_ACCEL_GYRO);
        pSensorStatus->isAccelEnabled = TRUE;
        pSensorStatus->isGyroEnabled = TRUE;
        pSensorStatus->isIMUEnabled = TRUE;
        //CLI_printf("ACCEL+GYRO is added\n");
        break;
    case SENSOR_ENG_VALUE_MAGNETOMETER:
        sensor_mag_add();
        pSensorStatus->isMagEnabled = TRUE;
        break;
#endif
#if AUDIO_DRIVER
    case SENSOR_AUDIO:
        pSensorStatus->isAudioEnabled = TRUE;
        sensor_audio_add();
        break;
#endif
#if AD7476_FPGA_DRIVER
    case SENSOR_ADC_AD7476:
        pSensorStatus->isADCEnabled = TRUE;
        sensor_ad7476_add();
        break;
#endif
#if LTC1859_DRIVER
    case SENSOR_ADC_LTC_1859_MAYHEW:
        pSensorStatus->isADCEnabled = TRUE;
        //sensor_ltc1859a_add();
        break;
#endif
    default:
        dbg_str_hex32("sensor-add-unknown",sensor_config_msg.sensor_common.sensor_id  );
        set_sys_error( SYS_ERR_ENODEV, sensor_config_msg.sensor_common.sensor_id  );
        break;
    }
}

void sensor_config_apply_sequence( const struct sensor_config_msg *pConfigSequence, SensorEnableStatus *pSensorStatus )
{
    int n;
    
    /* we pre-increment, so back up one */
    pConfigSequence--;
    n = 0;
    do {
        n++;
        pConfigSequence++;
        sensor_config_msg = *pConfigSequence;
        
        switch( sensor_config_msg.msg_type ){
        case GET_TOPIC_MINOR(TOPIC_SENSOR_DONE) :
            configure_all_sensors(NULL);
            break;
        case GET_TOPIC_MINOR(TOPIC_SENSOR_CLEAR):
            sensors_all_startstop( 0 );
            sensor_clear(pSensorStatus);
            break;
        case GET_TOPIC_MINOR(TOPIC_SENSOR_ADD):
            sensor_add( pSensorStatus );
            break;
        default:
            dbg_fatal_error_int("invalid-sensor-sequence-cmd", (int)(sensor_config_msg.msg_type) );
            break;
        }
        if( n > 10 ){
            /* something is wrong, we probably should not have this many */
            /* did they forget to terminate the list? */
            dbg_fatal_error_int("invalid-sensor-sequence-cmd2", n);
        }
    }
    while( pConfigSequence->msg_type != GET_TOPIC_MINOR(TOPIC_SENSOR_DONE ))
        ;
    sensors_configured = 1;
}

void wait_for_sensor_config(void)
{
    while( ! sensors_configured ){
        vTaskDelay( 5 );
    }
}

bool is_sensor_available(uint32_t sensor_id)
{
    bool yes = FALSE;
    switch (sensor_id)
    {
        case SENSOR_ENG_VALUE_ACCEL:
        case SENSOR_ENG_VALUE_GYRO:
        case SENSOR_ENG_VALUE_ACCEL_GYRO:
        case SENSOR_ENG_VALUE_MAGNETOMETER:
            yes = TRUE;
            break;
        default:
            break;
    }

    return yes;
}

#if S3AI_FIRMWARE_IS_RECOGNITION
extern SensorEnableStatus GetRecogSensorStatus(void );
#endif
//FIXME FFE always enable ACCEL and GYRO at same time.
// APP should always be ABLE to get ACCEL and GYRO samples at same time.
// When check which imu sensor is active, always check ACCEL_GYRO first !!
// In this sense, ACCEL_GYRO is a virtual rather than physical sensor !!
bool is_sensor_active(uint32_t sensor_id, IMU_SENSOR_MODE_t mode)
{
    SensorEnableStatus status;
    bool yes = FALSE;

    if (mode)
    {
#if S3AI_FIRMWARE_IS_RECOGNITION
        status = GetRecogSensorStatus();
#else
        // In collection only mode while check recog state.
        return yes;
#endif
    }
    else
    {
#if S3AI_FIRMWARE_IS_COLLECTION
        status = datacollection_sensor_status;
#else
        // In recognition only mode while check collect state.
        return yes;
#endif
    }

    switch (sensor_id)
    {
        case SENSOR_ENG_VALUE_ACCEL:
            yes = ( status.isAccelEnabled ) ? TRUE : FALSE;
            break;
        case SENSOR_ENG_VALUE_GYRO:
            yes = ( status.isGyroEnabled ) ? TRUE : FALSE;
            break;
        case SENSOR_ENG_VALUE_ACCEL_GYRO:
        {
            bool both = (status.isAccelEnabled) &&
                        (status.isGyroEnabled);
            yes = both ? TRUE : FALSE;
            break;
        }
        case SENSOR_ENG_VALUE_MAGNETOMETER:
            yes = status.isMagEnabled ? TRUE : FALSE;
            break;
            
        case SENSOR_AUDIO:
        yes = status.isAudioEnabled;
        break;
        
        case SENSOR_ADC_LTC_1859_MAYHEW:
        case SENSOR_ADC_AD7476: 
        yes = status.isADCEnabled;
        break;

    }

    return yes;
}

uint32_t sensor_get_sampling_rate(uint32_t sensor_id)
{
    // 0 means this sensor is not enabled.
    uint32_t rate = 0;

    switch (sensor_id)
    {
        case SENSOR_ENG_VALUE_ACCEL:
            if (imu_config.accel.enabled)
                rate = imu_config.accel.rate_hz;
            break;
        case SENSOR_ENG_VALUE_GYRO:
            if (imu_config.gyro.enabled)
                rate = imu_config.gyro.rate_hz;
            break;
        case SENSOR_ENG_VALUE_ACCEL_GYRO:
        {
            bool both = (imu_config.accel.enabled != 0) && (imu_config.gyro.enabled != 0);
            if (both)
                rate = imu_config.accel.rate_hz; // accel and gyro should have the same rate
            break;
        }
        case SENSOR_ENG_VALUE_MAGNETOMETER:
            if (imu_config.mag.enabled)
                rate = imu_config.mag.rate_hz;
            break;
        default:
            break;
    }

    return rate;
}

