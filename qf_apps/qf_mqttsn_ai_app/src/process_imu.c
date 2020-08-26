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

/** @file process_imu.c */

/*==========================================================
 *
 *    File   : process_imu.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
   
//#include "QL_SAL.h"
//#include "QL_SensorIoctl.h"
#include "Sensor_Attributes.h"

#include "Recognition.h"
#include "iop_messages.h"
#include "dcl_commands.h"
#include "DataCollection.h"
#include "ble_collection_defs.h"

#include "Mqttsn_Topics.h"

#include "process_ids.h"
#include "datablk_mgr.h"
#include "micro_tick64.h"

#include "dbg_uart.h"

#if (IMU_M4_DRIVERS == 1)

extern void configure_sensiml_imu_sensors(void);
extern int imu_get_max_datablock_size(void);

/** IMU configuration for ACCEL, GYRO and MAG */
struct imu_config  imu_config;

#define MOTION_BUF_SIZE (SENSIML_FFE_MAX_BATCH_DATA_SZ * 2 )
static int              motion_count;
int                     motion_total;
/* FIX ME - should break this into accel & gyro buffers */
/* this will simlify saving and recognition code */
static ble_accel_gyro_t motion_buf[ MOTION_BUF_SIZE ];
int                      dup_imu_counter;

extern void RecognitionMotion_Batch_DataReadyMsg(void);


//static QL_SAL_SensorHandle_t sensorHandle_sensimlApp;
//unsigned int packetids_selected[MAX_SENSIML_PACKETS] = {QL_FFE_SENSOR_ID_ACCEL1, QL_FFE_SENSOR_ID_GYRO, 0, 0}; //only accel and Gyro

//static struct QL_SF_Ioctl_Set_Batch_Data_Buf batch_databuf_info;

xTaskHandle xHandleTaskFFESensors;
QueueHandle_t FFESensorsMsgQ;

/* Called by BLE DATASAVE or RECOG to "configure/apply" the mag sensor based on global */
void sensor_mag_configure(void)
{
    /* here for completeness - work is done in accel */
}

/* Called by BLE DATASAVE or RECOG to "configure/apply" the gyro sensor based on global */
void sensor_gyro_configure(void)
{
    /* here for completeness - work is done in accel */
}


/* Called by BLE DATASAVE or RECOG to start/stop the IMU sensors */
void sensor_imu_startstop( int is_start )
{
    is_start &= SW_ENABLE_FFE;
    imu_config.is_running = is_start;
    /* FIXME: Wake up the task and make it configure the FFE */
}

/* Called by BLE DATASAVE or RECOG to "configure/apply" the accel sensor based on global */
void sensor_accel_configure(void)
{
    
    /* disable both */
    Set_AccelGyro_SensorEnable( 0 );
    
    /* RULE #1: if only 1 is turned on - this platform requires BOTH */
    if( imu_config.accel.enabled ^ imu_config.gyro.enabled ){

        /* same settings but we don't know range so choose default */
        /* copy config A -> B or B -> A */
        if( imu_config.accel.enabled ){
            /* gyro is not enabled */
            imu_config.gyro.enabled = 1;
            imu_config.gyro.rate_hz = imu_config.accel.rate_hz;
            // FIXME revisit default gyro range
            imu_config.gyro.range = 0;
        } else {
            imu_config.accel.enabled = 1;
            imu_config.accel.rate_hz = imu_config.gyro.rate_hz;
            // FIXME revisit default accel range
            imu_config.accel.range = 20;
        } 
    } else if ( imu_config.accel.enabled & imu_config.gyro.enabled ) {
        /* There are 2 configuration processes could endup here:
         * - when A,G are added individually before DONE.
         * - when A+G is added as combined sensor. */
        /* RULE #2: A,G of the same rate is enforced here. Remove this enforcemnet if needed! */
        /* RULE #3: This also guarantees that rate from later "add" will overwite earlier one. */
        // RULE #2,#3 are related, #3 is removed automatically if RULE #2 is removed.
        imu_config.gyro.rate_hz = imu_config.accel.rate_hz ; // = sensor_config_msg.sensor_common.rate_hz;
    }
#if 0 //for MQTTSN, BLE is used only for recognition
    
    /* this platform has specific RATES for the IMU sensors validate them 
     * and calculate the BLE data rate now
     */
    int tmp;
    tmp = imu_config.accel.rate_hz;
    /* ble cannot go past 104 hz */
    if( tmp <= 104 ){
        ble_data_config.imu_data_rate_reload = 0;
    } else {
        /* calculate divisor */
        tmp = (tmp+103) / 104;
        ble_data_config.imu_data_rate_reload = tmp-1;
        ble_data_config.imu_data_rate_counter = 0;
        dbg_str_int("ble-imu-throttle", tmp );
    }
#endif    
    /* if this is not enabled we are done */
    if( !imu_config.accel.enabled  ){
        return;
    }
    
#if 0
    // FIXME Enable this check only when rate enforcement is removed!
    /* must both have the same rate */
    if( imu_config.accel.rate_hz != imu_config.gyro.rate_hz ){
        dbg_str("err: bad-imu-freq\n");
        set_sys_error( SYS_ERR_EINVAL, SENSOR_ENG_VALUE_ACCEL );
        return;
    }  
#endif

    //imu_config.gyro.rate_hz = imu_config.accel.rate_hz = 600;
    /* this is the ffe level settings */
    configure_sensiml_imu_sensors();
}

/* Disables the imu sensor */
void sensor_imu_clear( uint32_t sensor_id )
{ 
    switch ( sensor_id )
    {
        case SENSOR_ENG_VALUE_ACCEL:
            imu_config.accel.enabled = 0;
            break;
        case SENSOR_ENG_VALUE_GYRO:
            imu_config.gyro.enabled = 0;
            break;
        case SENSOR_ENG_VALUE_ACCEL_GYRO:
            imu_config.accel.enabled = 0;
            imu_config.gyro.enabled = 0;
            break;
    }
}

static void sensor_imu_range_validation( uint32_t sensor_id )
{
    if ( sensor_id == SENSOR_ENG_VALUE_ACCEL )
{
    switch( imu_config.accel.range ){
    case 20:
    case 40:
    case 80:
            case 160:
        break;
    default:
        dbg_str("err-accel-rate\n");
                set_sys_error( SYS_ERR_EINVAL, SENSOR_ENG_VALUE_ACCEL );
                break;
        }
    } else if ( sensor_id == SENSOR_ENG_VALUE_GYRO ) {
        switch ( imu_config.gyro.range ) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
                break;
            default:
                dbg_str("err-gyro-rate\n");
                set_sys_error( SYS_ERR_EINVAL, SENSOR_ENG_VALUE_ACCEL );
        break;
    } 
}
}

/* Add imu sensor (A,G,A+G) */
void sensor_imu_add( uint32_t sensor_id )
{
    switch ( sensor_id )
    {
        case SENSOR_ENG_VALUE_ACCEL:
            imu_config.accel.enabled = 1;
            imu_config.accel.rate_hz =sensor_config_msg.sensor_common.rate_hz;
            imu_config.accel.range   = sensor_config_msg.unpacked.imu_config.sensor1_range;
            sensor_imu_range_validation(SENSOR_ENG_VALUE_ACCEL);
            break;
        case SENSOR_ENG_VALUE_GYRO:
            imu_config.gyro.enabled = 1;
            imu_config.gyro.rate_hz =sensor_config_msg.sensor_common.rate_hz;
            imu_config.gyro.range   = sensor_config_msg.unpacked.imu_config.sensor1_range;
            sensor_imu_range_validation(SENSOR_ENG_VALUE_GYRO);
            break;
        case SENSOR_ENG_VALUE_ACCEL_GYRO:
            imu_config.accel.enabled = 1;
            imu_config.accel.rate_hz =sensor_config_msg.sensor_common.rate_hz;
            imu_config.accel.range   = sensor_config_msg.unpacked.imu_config.sensor1_range;
            sensor_imu_range_validation(SENSOR_ENG_VALUE_ACCEL);

            imu_config.gyro.enabled = 1;
            imu_config.gyro.rate_hz =sensor_config_msg.sensor_common.rate_hz;
            imu_config.gyro.range   = sensor_config_msg.unpacked.imu_config.sensor2_range;
            sensor_imu_range_validation(SENSOR_ENG_VALUE_GYRO);
            break;
    }
}

/* disable the MAG sensor */
void sensor_mag_clear( void )
{
    imu_config.mag.enabled = 0;
}

/* reads the BLE message and puts the MAG sensor config data away */
void sensor_mag_add(void)
{
    if( sensor_config_msg.msg_type == GET_TOPIC_MINOR(TOPIC_SENSOR_CLEAR) ){
        return;
    }
    imu_config.mag.enabled = 1;
    imu_config.mag.rate_hz =sensor_config_msg.sensor_common.rate_hz ;
    imu_config.mag.range   = sensor_config_msg.unpacked.imu_config.sensor1_range;
}

/* debug code from the CLI so you can watch the IMU values in debug */
void watch_imu_data(void)
{
    int ax,ay,az;
    int gx,gy,gz;
    ble_accel_gyro_t *pSample;
    int x;

    /* this is from the CLI, so we use cli print */
    CLI_printf("ACCEL | GYRO\n");
    for(;;){
        /* go back 16 samples */
#define AVG_SIZE 16
        
        /* where are we now */
        x = motion_count;
        /* go backwards */
        x -= AVG_SIZE;
        /* wrap */
        x = (x + MOTION_BUF_SIZE) % MOTION_BUF_SIZE;
        
        /* find start */
        pSample = &motion_buf[x];
    
        /* calculate an average value */
        ax = 0; ay = 0; az = 0;
        gx = 0; gy = 0; gz = 0;
        for( x = 0 ; x < 16 ; x++ ){
            ax += pSample->accel.x;
            ay += pSample->accel.y;
            az += pSample->accel.z;
            gx += pSample->gyro.x;
            gy += pSample->gyro.y;
            gz += pSample->gyro.z;
            pSample += 1;
            if( pSample >= &(motion_buf[MOTION_BUF_SIZE]) ){
                pSample = &motion_buf[0];
            }
        }
        ax /= AVG_SIZE;
        ay /= AVG_SIZE;
        az /= AVG_SIZE;
        gx /= AVG_SIZE;
        gy /= AVG_SIZE;
        gz /= AVG_SIZE;

        CLI_printf("\r% 5d % 5d % 5d | % 5d % 5d % 5d | %d  ",
                ax,ay,az,gx,gy,gz, motion_count );
        x = CLI_getkey( 100 );
        if( x != EOF ){
            break;
        }
    }
    CLI_printf("\nDone\n");
#undef AVG_SIZE
}

/* Set the batch size from the FFE, this also sets up the FFE memory buffers */
int g_imu_batch_size = 6; // default value
static QL_Status Set_Sensor_BatchSize(int size, int num_pkts)
{
    g_imu_batch_size = size;
    return QL_STATUS_OK;
}

void imu_batch_size_set(int batch_size)
{
  Set_Sensor_BatchSize(batch_size, 2); // use 2 packets for accel/gyro
}

int imu_batch_size_get(void)
{
  return g_imu_batch_size;
}

int imu_get_accel_odr(void)
{
  return imu_config.accel.rate_hz;
}

int imu_get_gyro_odr(void)
{
  return imu_config.gyro.rate_hz;
}

/* 
 * Configure the FFE side of the IMU sensors.
 */
void configure_sensiml_imu_sensors(void)
{
    QL_Status ret;
    int tmp;
    int batch_size;
    /* calculate batch size */
    if( imu_config.accel.rate_hz != imu_config.gyro.rate_hz ){
        dbg_str("ERROR: accel.rate_hz != gyro.rate.hz");
        dbg_str_int("accel.rate_hz", imu_config.accel.rate_hz);
        dbg_str_int(" gryo.rate_hz", imu_config.gyro.rate_hz);
    }
    int requested_freq;
    {
        if( imu_config.accel.rate_hz >= 601 ){
            /* CLIP down to 1600 */
            imu_config.accel.rate_hz = 600;
            imu_config.gyro.rate_hz = 600;
            batch_size = 18;
        } else if( imu_config.accel.rate_hz >= 401 ){
            /* round up to 1660 */
            /* CLIP down to 1600 */
            imu_config.accel.rate_hz = 600;
            imu_config.gyro.rate_hz = 600;
            batch_size = 18;
        } else if( imu_config.accel.rate_hz >= 211 ){
            /* round up to 833 */
            imu_config.accel.rate_hz = 400;
            imu_config.gyro.rate_hz = 400;
            batch_size = 10;
        } else if( imu_config.accel.rate_hz >= 106 ){
            /* round up to 416 */
            imu_config.accel.rate_hz = 210;
            imu_config.gyro.rate_hz = 210;
            batch_size = 6;
        } else if( imu_config.accel.rate_hz >= 55 ){
            /* round up to 208 */
            imu_config.accel.rate_hz = 105;
            imu_config.gyro.rate_hz = 105;
            batch_size = 4;
        } else if( imu_config.accel.rate_hz >= 29 ){
            /* round up to 104 */
            imu_config.accel.rate_hz = 54;
            imu_config.gyro.rate_hz = 54;
            batch_size = 4;
        } else if( imu_config.accel.rate_hz >= 15 ){
            /* round up to 52 */
            imu_config.accel.rate_hz = 28;
            imu_config.gyro.rate_hz = 28;
            batch_size = 4;
        } else {
            /* default is 26 */
            imu_config.accel.rate_hz = 14;
            imu_config.gyro.rate_hz = 14;
            batch_size = 4;
        }
        requested_freq = imu_config.accel.rate_hz;
        switch( requested_freq ){
        case 52:
            requested_freq = 51; /* workaround */
            break;
        case 208:
            requested_freq = 207; /* workaround */
            break;
        default:
            /* no change */
            break;
        }
    }

   /*
    * Example code to enable/disable Accel and Gyro sensors and
    * updating attributes Range and ODR.
    */

    /* Enable Accel Sensor */
    ret = Sensor_Enable(QL_SAL_SENSOR_ID_ACCEL, ENABLE_SENSOR);
    configASSERT(ret == QL_STATUS_OK);

    /* Enabling GYRO Sensor */
    ret = Sensor_Enable(QL_SAL_SENSOR_ID_GYRO, ENABLE_SENSOR);
    configASSERT(ret == QL_STATUS_OK);

    switch( imu_config.accel.range ){
    default:
        dbg_fatal_error_hex32("invalid accel range", imu_config.accel.range );
        break;
        /* fallthrough */
    case 20: tmp = Accel_Sensor_Range_2G; break;
    case 40: tmp = Accel_Sensor_Range_4G; break;
    case 80: tmp = Accel_Sensor_Range_8G; break;
    case 160:  tmp = Accel_Sensor_Range_16G; break;
    }
    
    ret = Set_Sensor_Range(QL_SAL_SENSOR_ID_ACCEL, tmp);
    configASSERT(ret == QL_STATUS_OK);

    switch( imu_config.gyro.range ){
    default:
        dbg_str_int("invalid-gyro-range",imu_config.gyro.range);
        tmp = Gyro_Sensor_Range_2000DPS;
        break;
        /* fallthrough */
    case 0: tmp = Gyro_Sensor_Range_2000DPS; break;
    case 1: tmp = Gyro_Sensor_Range_1000DPS; break;
    case 2: tmp = Gyro_Sensor_Range_500DPS; break;
    case 3: tmp = Gyro_Sensor_Range_245DPS; break;
    case 4: tmp = Gyro_Sensor_Range_125DPS; break;
    }
    
    // Limit the batch size to the maximum datablock size 
    int max_datablock_size = imu_get_max_datablock_size();
    if (batch_size > max_datablock_size)
      batch_size = max_datablock_size;

    Set_Sensor_BatchSize(batch_size, 2); //There are 2 pkts (Accel and Gyro) generated for each sample period        

    ret = Set_Sensor_Range(QL_SAL_SENSOR_ID_GYRO, tmp);
    configASSERT(ret == QL_STATUS_OK);

    ret = Set_AccelGyro_SensorODR( requested_freq );
    configASSERT(ret == QL_STATUS_OK);
}
#if 0
/* Time to configure the IMU subsystem. */
void sensor_imu_configure(void)
{
    //for sensortile testing only
    imu_config.accel.enabled = 1;
    imu_config.gyro.enabled = 1;
    imu_config.mag.enabled = 1;

    
    last_imu_timestamp  = 0;
    if( !imu_config.accel.enabled ){
        return;
    } 

    /* TODO: how do I configure the IMU here?
     * NOTE: today above configuration is hard coded
     * Future must support "reasonable configurations"
     *
     * Example A few different IMU frequencies.
     * Such as:  8khz vrs 16khyz
     *
     * Example: Different microphone configurations.
     * given that 0= microphone disabled.
     * and that 1 = microphone platform default.
     * Are there other microphone configurations?
     * NOTE: While the merced board may not
     * other platforms (boards) may have options.
     */
}
#endif
/**
 * @brief Setup sensor_data for live streaming.
 * @nots: When v-sensor type is IMU_V_SENSOR_A_G_SEP, we need to send A,G samples in
 *        separated packets for this imu event. The need for the 2nd packet is indicated
 *        by non-zero return of this function.
 * @param[in/out] sdi : pointer to the sensor_data struct to receive prepared data.
 * @param[in] pImu : pointerto source imu data struct where imu data to be processed.
 * @param[in] time : pointer to timestamp when this imp data is received.
 *
 * @return : 0=no 2nd packet, 1=second packet.
 */
static uint8_t imu_setup_data(struct sensor_data *sdi, ble_accel_gyro_t *pImu, IMU_SENSOR_MODE_t mode)
{
    /* sep is flip-flop when virtual sensor is IMU_V_SENSOR_A_G_SEP which represents A or G.
     * As it is 0 initially, it will send A first, G followed. */
    static uint8_t sep = 0;
    IMU_VIRTUAL_SENSOR_t vs = sensor_get_virtual_sensor();

    if ( (vs == IMU_V_SENSOR_A_G_COM) && (is_sensor_active(SENSOR_ENG_VALUE_ACCEL_GYRO, mode)) )
    {
        sdi->bytes_per_reading = sizeof(ble_accel_gyro_t);
        sdi->n_bytes           = sizeof(ble_accel_gyro_t);
        sdi->rate_hz           = imu_config.accel.rate_hz; //accel and gyro should have same rate under this mode.
        sdi->vpData            = (void *)(pImu);
        sdi->sensor_id         = SENSOR_ENG_VALUE_ACCEL_GYRO;
        // reset sep, in case we missed one of A_G_SEP packet
        sep = 0;
    } else if ( (vs == IMU_V_SENSOR_ACCEL) && (is_sensor_active(SENSOR_ENG_VALUE_ACCEL, mode)) ) {
        sdi->bytes_per_reading = sizeof(ble_xyz16_t);
        sdi->n_bytes           = sizeof(ble_xyz16_t);
        sdi->rate_hz           = imu_config.accel.rate_hz;
        sdi->vpData            = (void *)(pImu);
        sdi->sensor_id         = SENSOR_ENG_VALUE_ACCEL;
        // reset sep, in case we missed one of A_G_SEP packet
        sep = 0;
    } else if ( (vs == IMU_V_SENSOR_GYRO) && (is_sensor_active(SENSOR_ENG_VALUE_GYRO, mode)) ) {
        sdi->bytes_per_reading = sizeof(ble_xyz16_t);
        sdi->n_bytes           = sizeof(ble_xyz16_t);
        sdi->rate_hz           = imu_config.gyro.rate_hz;
        sdi->vpData            = (void *)( (uint32_t)pImu + sizeof(ble_xyz16_t) );
        sdi->sensor_id         = SENSOR_ENG_VALUE_GYRO;
        // reset sep, in case we missed one of A_G_SEP packet
        sep = 0;
    } else if ( (vs == IMU_V_SENSOR_A_G_SEP) && (is_sensor_active(SENSOR_ENG_VALUE_ACCEL_GYRO, mode)) ) {
        //TODO This is currently supported by cli only. Dcl GUI may support it in the future.
        sdi->bytes_per_reading = sizeof(ble_xyz16_t);
        sdi->n_bytes           = sizeof(ble_xyz16_t);
        if (sep) {
            sdi->rate_hz           = imu_config.gyro.rate_hz;
            sdi->vpData            = (void *)( (uint32_t)pImu + sizeof(ble_xyz16_t) );
            sdi->sensor_id         = SENSOR_ENG_VALUE_GYRO;
            sep = 0;
        } else {
            sdi->rate_hz           = imu_config.accel.rate_hz;
            sdi->vpData            = (void *)(pImu);
            sdi->sensor_id         = SENSOR_ENG_VALUE_ACCEL;
            sep = 1;
        }
    } else {
        sep = 0;
    }

    return sep;
}

extern void send_message_to_datalog(int16_t *pbuffer);
#if 0 // for sampling rate validation
static uint32_t imu_sample_count = 0;
static uint32_t imu_time_prev = 0;
#endif
void imu_ai_data_processor(
       QAI_DataBlock_t *pIn,
       QAI_DataBlock_t *pOut,
       QAI_DataBlock_t **pRet,
       datablk_pe_event_notifier_t *pevent_notifier
     )
{
    size_t nSamples;
    int    nChannels;
    int16_t* pBuffer;
    struct sensor_data sdi; 
    uint64_t  time_start, time_curr, time_end, time_incr;
    
    pBuffer = (int16_t *)(pIn->p_data);
    
    nSamples = pIn->dbHeader.numDataElements;
    nChannels = pIn->dbHeader.numDataChannels;

    /* if not enabled... then throw data away */
    if( !(imu_config.accel.enabled || 
          imu_config.mag.enabled || 
          imu_config.gyro.enabled 
          )
       )
    {
        return;
    }
  
    time_start = convert_to_uSecCount(pIn->dbHeader.Tstart);
    time_incr  = ((uint64_t)(pIn->dbHeader.Tend - pIn->dbHeader.Tstart) * 1000) / (nSamples / nChannels);
    time_curr  = time_start;
   
#if 0 // for sampling rate validation
    imu_sample_count += (nSamples / nChannels);
    if ( imu_sample_count >= 1000 ) {
      uint32_t tick_time = xTaskGetTickCount(); // pIn->dbHeader.Tend
      imu_time_prev = tick_time - imu_time_prev;
      printf("imu_sample_count: %ld, time: %ld, diff = %ld, sample_rate: %ld\n", 
             imu_sample_count, tick_time, imu_time_prev,
             (imu_sample_count * 1000 ) / (imu_time_prev) );
      imu_sample_count = 0;
      imu_time_prev = tick_time;
    }
#endif
    for (int k = 0; k < nSamples; k += nChannels)
    {
      int tmp;
        //send_message_to_datalog(pBuffer);
        time_end = (time_curr + time_incr);

#if S3AI_FIRMWARE_IS_COLLECTION
        do {
            // Prepare buffer for live streaming and data saving
        memset( (void *)&(sdi), 0, sizeof(sdi) );
            tmp = imu_setup_data(&sdi, (ble_accel_gyro_t *)pBuffer, IMU_DATA_COLLECT);
        sdi.time_start        = time_curr;
        sdi.time_end          = time_end;   

        ble_send( &sdi );
        //data_save( &sdi );
        } while (tmp);

#endif
#if S3AI_FIRMWARE_IS_RECOGNITION
        do {
            // Prepare buffer for Recognition
        memset( (void *)&(sdi), 0, sizeof(sdi) );
            tmp = imu_setup_data(&sdi, (ble_accel_gyro_t *)pBuffer, IMU_RECOGNITION);
        sdi.time_start        = time_curr;
        sdi.time_end          = time_end;

            recog_data( &sdi );
        } while (tmp);
#endif
        
        time_curr = (time_curr + time_incr);
        
        pBuffer += nChannels ;
    }

    //back_calculate_start_time( &sdi );
    *pRet = NULL;
    return;
}

#endif // IMU_M4_DRIVERS
