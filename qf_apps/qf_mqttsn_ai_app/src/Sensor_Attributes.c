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

/*
 * Sensor_Attributes.c
 *
 */
#include "Fw_global_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "Sensor_Attributes.h"
   
#if    ( IMU_M4_DRIVERS  ) // implementation for Quickfeather mCube MC3635
#include "mc3635.h"
extern void dataTimerStart(void);
extern void dataTimerStop(void);

static int s_accel_state = 0;
static int s_gyro_state = 0;

static int16_t s_accel_range = 2; // indicates 2g, 4g, 8g or 16g for full-scale value
static int16_t s_gyro_range = 2000; // indicates 2000, 1000, 500, 250, 125 for full-scale value

static int16_t s_accel_resolution = MC3635_DEFAULT_SAMPLE_RESOLUTION;

static void Control_Sensor(unsigned int sensor_id, int enable)
{
    /* Set sensor ODR. */
    if (enable) 
    {
      dataTimerStart();
    }
    else
    {
      dataTimerStop();
    }
    
    if (sensor_id == QL_SAL_SENSOR_ID_ACCEL) 
      s_accel_state = enable;
}

static void Set_Sensor_State(unsigned int sensor_id, unsigned state)
{
   Control_Sensor(sensor_id, state);
}

static int Get_Sensor_State(unsigned int sensor_id)
{
  if (sensor_id == QL_SAL_SENSOR_ID_ACCEL)
    return s_accel_state;
  else if (sensor_id == QL_SAL_SENSOR_ID_GYRO)
    return s_gyro_state;
  else
    return 0;
}
    
static QL_Status Set_Sensor_ODR(unsigned int sensor_id, unsigned int odr_val)
{
  /* Set sensor ODR. */
  // Set sample rate for the sensor
  if (sensor_id == QL_SAL_SENSOR_ID_ACCEL)
     mc3635_set_sample_rate(odr_val);
  
  return QL_STATUS_OK;
}

QL_Status Set_AccelGyro_SensorEnable( int enable )
{
  Set_Sensor_State( QL_SAL_SENSOR_ID_ACCEL, enable );
  //Set_Sensor_State( QL_SAL_SENSOR_ID_GYRO , enable );
  return QL_STATUS_OK;
}


QL_Status Set_AccelGyro_SensorODR( unsigned int odr_val)
{
  QL_Status ret = QL_STATUS_ERROR;

  unsigned int accel_state = 0;
  
  accel_state = Get_Sensor_State(QL_SAL_SENSOR_ID_ACCEL);
  if(ENABLE_SENSOR == accel_state)
  {
      ret = Set_Sensor_ODR(QL_SAL_SENSOR_ID_ACCEL, odr_val);
  }

  unsigned int gyro_state = 0;
  gyro_state = Get_Sensor_State(QL_SAL_SENSOR_ID_GYRO);
  if(ENABLE_SENSOR == gyro_state)
  {
      //ret = Set_Sensor_ODR(QL_SAL_SENSOR_ID_GYRO, odr_val);
  }

  return ret;
}

static int32_t accel_ranges[] = { 2, 4, 8, 16 };
static int32_t gyro_ranges[] = {2000, 1000, 500, 250, 125 };
QL_Status Set_Sensor_Range(unsigned int sensor_id, unsigned int range_index)
{
  int32_t full_scale_range ;
  
  if (sensor_id == QL_SAL_SENSOR_ID_ACCEL) {
    if (range_index > 3)
      range_index = 3;
    full_scale_range = accel_ranges[range_index];
    s_accel_range = full_scale_range;
  }
  else if (sensor_id == QL_SAL_SENSOR_ID_GYRO) {
    if (range_index > 4)
      range_index = 4;
    full_scale_range = gyro_ranges[range_index];  
    s_gyro_range = full_scale_range;  
  }
  else
    return QL_STATUS_ERROR;

  return QL_STATUS_OK;
}

QL_Status Get_Sensor_ODR(unsigned int sensor_id, unsigned int* odr_val)
{
  float f_odr_val = 0.f;
  int sid = sensor_id;
  int k, i_odr_val = (int)f_odr_val;
  if (sid == QL_SAL_SENSOR_ID_ACCEL) {
    for (k = 0; k < 4; k++) {
      if (i_odr_val >= accel_ranges[k]) {
        break;
      }
    }
    *odr_val = k;
  }
  else if (sid == QL_SAL_SENSOR_ID_GYRO)
  {
    for (k = 4; k >= 0; k--) {
      if (i_odr_val >= gyro_ranges[k]) {
        break;
      }
    }
    *odr_val = k;
  } else
    return QL_STATUS_ERROR;

  return QL_STATUS_OK;
}

QL_Status Get_Sensor_Range(unsigned int sensor_id, unsigned int* range)
{
  if (sensor_id == QL_SAL_SENSOR_ID_ACCEL) 
    *range = mc3635_get_sample_range();
  else if (sensor_id == QL_SAL_SENSOR_ID_GYRO)
     ; // MOTION_GYRO;
  else
    return QL_STATUS_ERROR;

  return QL_STATUS_OK;
}

QL_Status Sensor_Enable(unsigned int sensor_id, int enable)
{
  QL_Status ret = QL_STATUS_OK;
  if(QL_SAL_SENSOR_ID_ACCEL == sensor_id)
  {
    Control_Sensor(QL_SAL_SENSOR_ID_ACCEL, enable);
    if(DISABLE_SENSOR == enable)
    {
      /* If Accel disable is requested,
       * Disable Gyro also(if it is enabled). */
      unsigned int gyro_state = 0;
      gyro_state = Get_Sensor_State(QL_SAL_SENSOR_ID_GYRO);
      if(ENABLE_SENSOR == gyro_state)
          Control_Sensor(QL_SAL_SENSOR_ID_GYRO, DISABLE_SENSOR);
    }
  }
  else if(QL_SAL_SENSOR_ID_GYRO == sensor_id)
  {
    if(ENABLE_SENSOR == enable)
    {
       /* Check for Accel state, if it is enabled,
        * then only enable Gyro. If not, return Error */
       unsigned int accel_state = 0;
       accel_state = Get_Sensor_State(QL_SAL_SENSOR_ID_ACCEL);

       if(ENABLE_SENSOR == accel_state)
          ; //Control_Sensor(QL_SAL_SENSOR_ID_GYRO, enable); //Accel is enabled, so enabling Gyro also.
       else
          return QL_STATUS_ERROR; //Accel is not active, Gyro can not be enabled alone. So, Returing Error.
    }
  }
  return ret;
}

int16_t get_accel_range(void)
{
  return s_accel_range * 980; // in 10's of milli-g
}

int16_t get_accel_sample_resolution(void)
{
  return s_accel_resolution; // in 10's of milli-g
}

int16_t get_gyro_range(void)
{
  return s_gyro_range; // in degrees-per-second (DPS)
}

#endif /* ( IMU_M4_DRIVERS && defined(STM32L476xx) ) */
