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

/*==========================================================
 *                                                          
 *    File   : LSM6DSL_Gyro.h
 *    Purpose: This file has all the definitions for Accessing 
 *             LSM6DSL_Gyro Sensor
 *                                                          
 *=========================================================*/


//These are internal States for the Driver
//Note: Keep values for states same to reduce FFE memory
//      For example: _FAIL = 9, _DONE =10 etc.


//Use these to remove unwanted Code modules from the Project
//Note: Make sure the Source code files are also removed

//#include "DDStates.h"

#include "LSM6DSL.h"


#define GYRO_PROBE_IN_M4       1


//These are the only valid values for Gyroscope Range
#define GYRO_RANGE_250        0x0 //= 250 dps
#define GYRO_RANGE_500        0x1 //= 500 dps
#define GYRO_RANGE_1000       0x2 //= 100 dps
#define GYRO_RANGE_2000       0x3 //= 2000 dps

#define DEFAULT_GYRO_RANGE  GYRO_RANGE_1000

//These are the only valid values for Gyro Range
#define LSM6DSL_GYRO_RANGE_250        0x0 //= 250 dps
#define LSM6DSL_GYRO_RANGE_500        0x1 //= 500 dps
#define LSM6DSL_GYRO_RANGE_1000       0x2 //= 100 dps
#define LSM6DSL_GYRO_RANGE_2000       0x3 //= 2000 dps

//this is mutually exclusive with above 4 ranges
#define LSM6DSL_GYRO_RANGE_FS_125     0x1 //1= 125 dps, 0= disable 125 dps

#define LSM6DSL_DEFAULT_GYRO_RANGE  LSM6DSL_GYRO_RANGE_1000


//Frequency in Hz. Frequency settings set by M4.
#define GYRO_RATE_0HZ      0
#define GYRO_RATE_12_5HZ   12 //=12.5
#define GYRO_RATE_26HZ     26
#define GYRO_RATE_52HZ     52
#define GYRO_RATE_104HZ   104
#define GYRO_RATE_208HZ   208
#define GYRO_RATE_416HZ   416
#define GYRO_RATE_833HZ   833
#define GYRO_RATE_1660HZ 1660
#define GYRO_RATE_3330HZ 3330
#define GYRO_RATE_6660HZ 6660

//ODR for Freq
#define LSM6DSL_ODR_GYRO_RATE_0HZ      0x00    // Use to power down gyro
#define LSM6DSL_ODR_GYRO_RATE_12_5HZ   0x01
#define LSM6DSL_ODR_GYRO_RATE_26HZ     0x02
#define LSM6DSL_ODR_GYRO_RATE_52HZ     0x03
#define LSM6DSL_ODR_GYRO_RATE_104HZ    0x04
#define LSM6DSL_ODR_GYRO_RATE_208HZ    0x05
#define LSM6DSL_ODR_GYRO_RATE_416HZ    0x06
#define LSM6DSL_ODR_GYRO_RATE_833HZ    0x07
#define LSM6DSL_ODR_GYRO_RATE_1660HZ   0x08
#define LSM6DSL_ODR_GYRO_RATE_3330HZ   0x09
#define LSM6DSL_ODR_GYRO_RATE_6660HZ   0x0A

#define LSM6DSL_DEFAULT_ODR_GYRO_RATE LSM6DSL_ODR_GYRO_RATE_52HZ


float Get_Gyro_Scale_Factor(void);
