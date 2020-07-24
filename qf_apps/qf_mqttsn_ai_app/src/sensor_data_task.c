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
#include "iop_messages.h"
#include "mc3635.h"
/* Private define ------------------------------------------------------------*/

extern void dataTimer_Callback(void const *arg);
extern void dataTimerStart(void);
extern void dataTimerStop(void);
extern void Error_Handler(void);

extern void imu_sensordata_read_callback(void);

#include "datablk_mgr.h"
#include "process_ids.h"
//#include "mc3635.h"

QAI_DataBlock_t  *pimu_data_block_prev = NULL;
int              imu_samples_collected = 0;

extern outQ_processor_t imu_isr_outq_processor;
#define  IMU_ISR_EVENT_NO_BUFFER  (1)   ///< error getting a new datablock buffer

void set_first_imu_data_block()
{
    /* Acquire an audio buffer */
  if (NULL == pimu_data_block_prev) 
  {
    datablk_mgr_acquire(imu_isr_outq_processor.p_dbm, &pimu_data_block_prev, 0);
  }
    configASSERT(pimu_data_block_prev); // probably indicates uninitialized datablock manager handle
    imu_samples_collected = 0;
  pimu_data_block_prev->dbHeader.Tstart = xTaskGetTickCount();
}

void imu_event_notifier(int pid, int event_type, void *p_event_data, int num_data_bytes)
{
  char *p_data = (char *)p_event_data;
  printf("[IMU Event] PID=%d, event_type=%d, data=%02x\n", pid, event_type, p_data[0]);
}

extern int imu_batch_size_get(void);
extern int imu_get_accel_odr(void);
extern int imu_get_gyro_odr(void);
extern int16_t get_accel_range(void);
extern int16_t get_gyro_range(void);
extern int16_t get_accel_sample_resolution(void);

/* This function performs raw data to engineering value conversion.
 * Scale Accelerometer samples map such that samples units are in
 * centi-meter/sec^2 and the resulting 16-bit value accommodates
 * the case where the sensor range is 16G.
 *
 * The table below gives an example for various full-scale ranges.
 * 
 * |  Sensor   | Accel    |   Accel   |   Accel      | 
 * |  Range    | Value    | ADC Value | Scaled Value |
 * |           | (m/sec^2)|           | (cm /sec^2)  |
 * | --------- | -------- | --------- | ------------ |
 * |  +/-  2G  |   9.8    |   16384   |    980       |
 * |  +/-  4G  |   9.8    |    8192   |    980       |
 * |  +/-  8G  |   9.8    |    4096   |    980       |
 * |  +/- 16G  |   9.8    |    2048   |    980       |
 * 
 */
static void adjust_accel(xyz_t *p_accel_data)
{
    int32_t temp;
    int32_t s, fs;
    short r;
 
    r = get_accel_range();
    fs = get_accel_sample_resolution();
    
    s = p_accel_data->x;
    temp = (s * r) / fs;
    p_accel_data->x = (short)temp;

    s = p_accel_data->y;
    temp = (s * r) / fs;
    p_accel_data->y = (short)temp;

    s = p_accel_data->z;
    temp = (s * r) / fs;
    p_accel_data->z = (short)temp;
}

/* Scale Gyro samples such that samples map to +/- 2000 dps
 * full-scale range. The table below gives an example for
 * various full-scale ranges.
 * 
 * |  Sensor   |   Gyro    |   Gyro    |   Gyro       | 
 * |  Range    | DPS Value | ADC Value | Scaled Value |
 * | --------- | --------- | --------- | ------------ |
 * |  +/- 2000 | 123.3125  |   2020    |    2020      |
 * |  +/- 1000 | 123.3125  |   4041    |    2020      |
 * |  +/-  500 | 123.3125  |   8081    |    2020      |
 * |  +/-  250 | 123.3125  |  16163    |    2020      |
 * |  +/-  125 | 123.3125  |  32326    |    2020      |
 * 
 */
static void adjust_gyro(xyz_t *p_gyro_data)
{
    int32_t temp;
    int32_t s;
    short r;
    int   shift = 0;
 
    r = get_gyro_range();
    switch (r) {
      case 2000: shift = 0; break;
      case 1000: shift = 1; break;
      case  500: shift = 2; break;
      case  250: shift = 3; break;
      case  125: shift = 4; break;
    }
    
    s = p_gyro_data->x;
    temp = (s >> shift);
    p_gyro_data->x = (short)temp;

    s = p_gyro_data->y;
    temp = (s >> shift);
    p_gyro_data->y = (short)temp;

    s = p_gyro_data->z;
    temp = (s >> shift);
    p_gyro_data->z = (short)temp;
  return ;
}

#if (USE_IMU_FIFO_MODE)
extern int32_t ql_lsm6dsm_read_reg(uint8_t reg, uint8_t *data, uint16_t len);

extern int16_t fifo_samples[];

uint32_t imu_fifo_samples_to_ticks(int num_fifo_samples)
{
   int fifo_odr = imu_get_accel_odr();
   return (configTICK_RATE_HZ * num_fifo_samples) / (6 * fifo_odr);
}

int  imu_sensordata_buffer_ready_single_sample()
{
    int16_t *p_dest = (int16_t *) &pimu_data_block_prev->p_data[sizeof(int16_t)*imu_samples_collected];
    int batch_size;
    int32_t err;
    
    batch_size = imu_batch_size_get() * 6;
  
    uint32_t t_end = xTaskGetTickCount();
    uint32_t t_start;

    for (int k = 0; k < (MC3635_FIFO_SIZE*6); k += 6) {
       /* Attempt reading 1 sample data from the device.
        */
       err = mc3635_read_fifo_data((xyz_t *)p_dest);
       if (err == 0)
       {
         // No new data available, stop reading
         break;
       }

       /* Copy Gyroscope samples to IMU datablock */
       p_dest[3] = 0; // Gyroscope X co-ordinate
       p_dest[4] = 0; // Gyroscope Y co-ordinate
       p_dest[5] = 0; // Gyroscope Z co-ordinate
     
       // Apply scaling to the raw accel data samples
       adjust_accel((xyz_t *)(p_dest+0));

       // Apply scaling to the raw gyro data samples
       adjust_gyro((xyz_t *)(p_dest+3));

       p_dest += 6;
       imu_samples_collected += 6;

      if (imu_samples_collected >= batch_size) //  pimu_data_block_prev->dbHeader.numDataElements
      {
        pimu_data_block_prev->dbHeader.numDataElements = imu_samples_collected;
        pimu_data_block_prev->dbHeader.numDataChannels = 6;
        pimu_data_block_prev->dbHeader.Tend = t_end;
        t_start = t_end - imu_fifo_samples_to_ticks(imu_samples_collected);
        pimu_data_block_prev->dbHeader.Tstart = t_start;
        imu_samples_collected = 0;
        return 1;
      } 
    }
    return 0;
}

int16_t temp_fifo_buff[MC3635_FIFO_SIZE*6];
int16_t temp_fifo_remaining_count = 0;

int  imu_sensordata_buffer_ready()
{
    int16_t *p_dest = (int16_t *) &pimu_data_block_prev->p_data[sizeof(int16_t)*imu_samples_collected];
    int16_t *p_temp = &temp_fifo_buff[temp_fifo_remaining_count];
    int batch_size;
    int32_t err;
    
    int threshold_count = mc3635_get_threshold_count();
    imu_batch_size_set(threshold_count);

    batch_size = imu_batch_size_get() * 6;
  
    uint32_t t_end = xTaskGetTickCount();
    uint32_t t_start;

    int num_samples = temp_fifo_remaining_count;
    for (int k = 0; k < (MC3635_FIFO_SIZE); k += threshold_count) {
       /* Attempt reading 1 sample data from the device.
        */
       err = mc3635_read_fifo_burst((xyz_t *)p_temp, threshold_count);
       if (err == 0)
       {
         // No new data available, stop reading
         break;
       }
       p_temp += 3*threshold_count;
       num_samples += 3*threshold_count;
    }
    if (num_samples == 0)
      return 0;
    p_temp = temp_fifo_buff;
    for (int k = 0; k < num_samples; k+=3) {
       /* Copy Accelermoter samples to IMU datablock */
       p_dest[0] = *p_temp++; // Accelerometer X co-ordinate
       p_dest[1] = *p_temp++; // Accelerometer Y co-ordinate
       p_dest[2] = *p_temp++; // Accelerometer Z co-ordinate

       /* Copy Gyroscope samples to IMU datablock */
       p_dest[3] = 0; // Gyroscope X co-ordinate
       p_dest[4] = 0; // Gyroscope Y co-ordinate
       p_dest[5] = 0; // Gyroscope Z co-ordinate
     
       // Apply scaling to the raw accel data samples
       adjust_accel((xyz_t *)(p_dest+0));

       // Apply scaling to the raw gyro data samples
       adjust_gyro((xyz_t *)(p_dest+3));

       p_dest += 6;
       imu_samples_collected += 6;

      if (imu_samples_collected >= batch_size) //  pimu_data_block_prev->dbHeader.numDataElements
      {
        k = k+3;
        temp_fifo_remaining_count = num_samples - k ;
        if (temp_fifo_remaining_count > 0)
        {
          int l, j;
          for (l = 0, j = k; j < num_samples; j++)
            temp_fifo_buff[l++] = *p_temp++;
        }
        pimu_data_block_prev->dbHeader.numDataElements = imu_samples_collected;
        pimu_data_block_prev->dbHeader.numDataChannels = 6;
        pimu_data_block_prev->dbHeader.Tend = t_end;
        t_start = t_end - imu_fifo_samples_to_ticks(imu_samples_collected);
        pimu_data_block_prev->dbHeader.Tstart = t_start;
        imu_samples_collected = 0;
        return 1;
      } 

    }
    return 0;
}

#else
int  imu_sensordata_buffer_ready()
{
    xyz_t *p_dest = (xyz_t *) &pimu_data_block_prev->p_data[sizeof(int16_t)*imu_samples_collected];
    int ret;
    uint8_t accel_status, gyro_status;
    int batch_size;
    
    if (mc3635_read_data(p_dest) == 0) {
      return 0;
    }
    
    // Apply scaling to the raw accel data samples
    adjust_accel(p_dest);
    p_dest++;
 
    // Set Gyro values to 0s
    p_dest->x = 0;
    p_dest->y = 0;
    p_dest->z = 0;
    
    // Apply scaling to the raw gyro data samples
    adjust_gyro(p_dest);

    imu_samples_collected += 6;
    batch_size = imu_batch_size_get() * 6;
    
    if (imu_samples_collected >= batch_size) //  pimu_data_block_prev->dbHeader.numDataElements
    {
      pimu_data_block_prev->dbHeader.numDataElements = imu_samples_collected;
      pimu_data_block_prev->dbHeader.numDataChannels = 6;
      imu_samples_collected = 0;
      return 1;
    } 
    else
    {
      return 0;
    }
}
#endif

void imu_sensordata_read_callback(void)
{
    int gotNewBlock = 0;
    QAI_DataBlock_t  *pdata_block = NULL;
  
    if (!imu_sensordata_buffer_ready())
    {
      return;
    }
    /* Acquire a new data block buffer */
    datablk_mgr_acquire(imu_isr_outq_processor.p_dbm, &pdata_block, 0);
    if (pdata_block)
    {
        gotNewBlock = 1;
    }
    else
    {
        // send error message 
        // xQueueSendFromISR( error_queue, ... )
        if (imu_isr_outq_processor.p_event_notifier)
          (*imu_isr_outq_processor.p_event_notifier)(imu_isr_outq_processor.in_pid, IMU_ISR_EVENT_NO_BUFFER, NULL, 0);
        pdata_block = pimu_data_block_prev;
        pdata_block->dbHeader.Tstart = xTaskGetTickCount();
        pdata_block->dbHeader.numDropCount++;
    }

    if (gotNewBlock)
    {
        /* send the previously filled audio data to specified output Queues */     
        pimu_data_block_prev->dbHeader.Tend = pdata_block->dbHeader.Tstart;
        datablk_mgr_WriteDataBufferToQueues(&imu_isr_outq_processor, NULL, pimu_data_block_prev);
        pimu_data_block_prev = pdata_block;
    }
}

#include "RtosTask.h"
xTaskHandle xHandleTaskAccelReader;
QueueHandle_t accelReaderQ;
#define ACCEL_READER_QUEUE_LENGTH   (10)

void accel_reader_wakeup_handler(void)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  uint32_t accelMessage = 1;
  xQueueSendFromISR(accelReaderQ, &accelMessage, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken == pdTRUE)
  {
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

void AccelReaderTaskHandler(void *pParameter)
{
    BaseType_t ret;
    uint32_t accelMessage;
    while (1)
    {
      ret = xQueueReceive(accelReaderQ, &accelMessage, portMAX_DELAY);
      if (ret == pdTRUE)
      {
        imu_sensordata_read_callback();
      }
    }
}

/* Setup msg queue and Task Handler for IO Msg Task */
signed portBASE_TYPE StartRtosTaskAccelReader( void)
{
    static uint8_t ucParameterToPass;
    uint32_t accelMessage;
    /* Create queue for incoming messages to be processed at a later time */
    accelReaderQ = xQueueCreate( ACCEL_READER_QUEUE_LENGTH, sizeof(accelMessage) );
    vQueueAddToRegistry( accelReaderQ, "Accelerometer read interrupt Q" );
    configASSERT( accelReaderQ );
    
    /* Create IO Task */
    xTaskCreate ( AccelReaderTaskHandler, "AccelReaderTaskHandler", STACK_SIZE_ALLOC(256),  &ucParameterToPass, PRIORITY_NORMAL, &xHandleTaskAccelReader);
    configASSERT( xHandleTaskAccelReader );
    
    return pdPASS;
}
