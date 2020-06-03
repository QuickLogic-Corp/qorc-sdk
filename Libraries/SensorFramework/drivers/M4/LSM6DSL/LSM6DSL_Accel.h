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
 *    File   : LSM6DSL_Accel.h
 *    Purpose: This file has all the definitions for Accessing 
 *             LSM6DSL_Accel Sensor
 *                                                          
 *=========================================================*/

#ifndef __LSM6DSL_ACCEL_H__
#define __LSM6DSL_ACCEL_H__
//These are internal States for the Driver 
//Note: Keep values for states same to reduce FFE memory
//      For example: _FAIL = 9, _DONE =10 etc.


//Use these to remove unwanted Code modules from the Project
//Note: Make sure the Source code files are also removed

//#include "DDStates.h"

#include "LSM6DSL.h"

#define LSM6DSL_ERR_STATUS_NO_ERROR					0x00

#define ACCEL_PROBE_IN_M4       1

#define PROBE_ACCEL_CHIP_ID        1
#define PROBE_ACCEL_RESET          2
#define PROBE_ACCEL_FAIL           9
#define PROBE_ACCEL_DONE           10

#define OPEN_ACCEL_INIT              1
#define OPEN_ACCEL_WAIT              2
//#define OPEN_ACCEL_RATE_AND_RANGE    3
#define OPEN_ACCEL_ATTRIBUTE_CHANGE    3
#define OPEN_ACCEL_CLOSED            4 //Open() State when Device is closed after successful open
#define OPEN_ACCEL_FAIL              9
#define OPEN_ACCEL_DONE              10 //Successfull Open()

#define CLOSE_ACCEL_INIT     1
#define CLOSE_ACCEL_WAIT     2
#define CLOSE_ACCEL_OPENED   3  //Close() State when Device is in Open state
#define CLOSE_ACCEL_FAIL     9
#define CLOSE_ACCEL_DONE     10 //Successful Close()

#define READ_ACCEL_COPY      1
#define READ_ACCEL_WAIT      2
#define READ_ACCEL_IDLE      3
#define READ_ACCEL_FAIL      9  //Not useful
#define READ_ACCEL_DONE      10 //Not useful

#define LSM6DSL_CHIP_ID_VALUE        LSM6DSL_DEVICE_ID
#define LSM6DSL_MAX_INIT_WAIT        6  // Chip ID reads retry times
#define LSM6DSL_MAX_INIT_WAIT2       20 // at least another 200ms wait for softreset

//Note: These are Register values to be Programmed
//Note: These are also used as indices to lookup Sample Periods
//#define LSM6DSL_ACCEL_ODR_RATE_PowerDown     0
//#define LSM6DSL_ACCEL_ODR_RATE_1_6_HZ        1  //  1.6 Hz
//#define LSM6DSL_ACCEL_ODR_RATE_12_5_HZ       2  // 12.5 Hz
//#define LSM6DSL_ACCEL_ODR_RATE_26_HZ         3  //   26 Hz
//#define LSM6DSL_ACCEL_ODR_RATE_52_HZ         4  //   52 Hz
//#define LSM6DSL_ACCEL_ODR_RATE_104_HZ        5  //  104 Hz
//#define LSM6DSL_ACCEL_ODR_RATE_208_HZ        6  //  208 Hz
//#define LSM6DSL_ACCEL_ODR_RATE_416_HZ        7  //  416 Hz
//#define LSM6DSL_ACCEL_ODR_RATE_833_HZ        8  //  833 Hz
//#define LSM6DSL_ACCEL_ODR_RATE_1660_HZ       9  // 1660 Hz
//#define LSM6DSL_ACCEL_ODR_RATE_3330_HZ      10  // 3330 Hz
//#define LSM6DSL_ACCEL_ODR_RATE_6660_HZ      11  // 6660 Hz


#define LSM6DSL_DEFAULT_ACCEL_SAMPLE_RATE    LSM6DSL_ACCEL_ODR_RATE_52_HZ //default rate is 52 Hz

//These are the only valid values for Accelerometer Range
#define ACCEL_RANGE_2G        0 //=+/-2g
#define ACCEL_RANGE_4G        1 //=+/-4g
#define ACCEL_RANGE_8G        2 //=+/-8g
#define ACCEL_RANGE_16G       3 //=+/-16g

#define DEFAULT_ACCEL_RANGE  ACCEL_RANGE_8G

//These are the only valid values for Accelerometer Range
#define LSM6DSL_ACCEL_RANGE_2G        0x0 //=+/-2g
#define LSM6DSL_ACCEL_RANGE_4G        0x2 //=+/-4g
#define LSM6DSL_ACCEL_RANGE_8G        0x3 //=+/-8g
#define LSM6DSL_ACCEL_RANGE_16G       0x1 //=+/-16g

#define LSM6DSL_DEFAULT_ACCEL_RANGE  LSM6DSL_ACCEL_RANGE_8G

#define LSM6DSL_ACCEL_NORMAL 0
#define LSM6DSL_ACCEL_HIGH_PERFORMANCE 1
#define LSM6DSL_DEFAULT_ACCEL_MEASUREMENT_MODE  LSM6DSL_ACCEL_HIGH_PERFORMANCE

//Frequency in Hz. Frequency settings set by M4.
#define ACCEL_RATE_1_6HZ   1.6
#define ACCEL_RATE_12_5HZ 12.5
#define ACCEL_RATE_26HZ     26
#define ACCEL_RATE_52HZ     52
#define ACCEL_RATE_104HZ   104
#define ACCEL_RATE_208HZ   208
#define ACCEL_RATE_416HZ   416
#define ACCEL_RATE_833HZ   833
#define ACCEL_RATE_1660HZ 1660
#define ACCEL_RATE_3330HZ 3330
#define ACCEL_RATE_6660HZ 6660

//ODR for Freq
#define LSM6DSL_ODR_ACCEL_RATE_0HZ      0x00    // Use to power down accel
#define LSM6DSL_ODR_ACCEL_RATE_1_6HZ    0x0B
#define LSM6DSL_ODR_ACCEL_RATE_12_5HZ   0x01
#define LSM6DSL_ODR_ACCEL_RATE_26HZ     0x02
#define LSM6DSL_ODR_ACCEL_RATE_52HZ     0x03
#define LSM6DSL_ODR_ACCEL_RATE_104HZ    0x04
#define LSM6DSL_ODR_ACCEL_RATE_208HZ    0x05
#define LSM6DSL_ODR_ACCEL_RATE_416HZ    0x06
#define LSM6DSL_ODR_ACCEL_RATE_833HZ    0x07
#define LSM6DSL_ODR_ACCEL_RATE_1660HZ   0x08
#define LSM6DSL_ODR_ACCEL_RATE_3330HZ   0x09
#define LSM6DSL_ODR_ACCEL_RATE_6660HZ   0x0A

#define LSM6DSL_DEFAULT_ODR_ACCEL_RATE LSM6DSL_ODR_ACCEL_RATE_52HZ

//LSM6DSL Accelerometer state
typedef struct {
    //Init Sensor state Variable

    int probe_state;     //State of Probe State Machine
    int probe_cntr;      //This is to check number of ffe ticks
    int probe_timer;     //This is to check elapsed ffe time 
    
    int open_state;      //State of Open State Machine
    
    int close_state;     //State of Close State Machine

    int read_state;      //State of Runtime State Machine       

    //=1 - Sensor is in sleep mode, =0 - Sensor in Wakeup mode
    int sleep;

    //Polling times are accumulated FFE Calling Rate (=Timer period in ms)
    //Used to check polling time for each sensors
    float poll_time;
    float poll_time_batch;
    float poll_time_batch_uncalib;
    int batch_push; //Push batch data for every batch period.
    int batch_push_uncalib;

    //Should be calculated Dynamically based on Sensor Freq setup (in fractions of ms)
    float sample_period;    //1000/(sample rate in Hz)

    int sample_rate;        //current sample Rate
    int accel_range;        //Range: 0 maps to +-2G, 1 maps to +-4G, 2 maps to +-8G, 3 maps to +-16G
    
    int Accel_xyz[3];
    int sample_count; //This is number of samples available from Device Driver
    //int Accel_buf[FFE_ACCEL_FIFO_READ_SIZE];  //8 samples 400Hz/50Hz + 1 for overflow

    //Used to enable Runtime Segments for the SM
    int run;
    int read_enable;

    //Dynamic parameters
	//unsigned int range; ==> same as accel_range
	unsigned int odr;  //sensor rate configuration  Sensor to FFE rate (Hz)
	unsigned int powerMode; //measure mode normal or low power. (HP/LP)
	unsigned int axisDirection;
	// TBD !! Additional attributes

	//Kyocera Enhancements
    int AccDataUncalib[3];  //accel samples used for Acc CAL Calculation
    int AccDataCalib[3];    //accel samples used for FFE application task.
    int AccDataUncalibFilt[3]; //Gyro uncalibrated samples for Android sensors.
    int AccDataCalibFilt[3]; //accel samples for Android Sensors.

    int Xofs;  //Offset correction updated by Acc CAL calc module provided by kyocera
    int Yofs;  //These offest will be subtracted from raw accel data to get calibrated output.
	int Zofs;

	int FilterState;  //Filter state, 0->disable; 1->Disable to enable state, 2->continuous  enabled state
	int Xout_prev;  //previous state of filter.
	int Yout_prev;  //Default prev value is Xin[0], Yin[0], Zin[0]
	int Zout_prev;  //Xout[n] = Xout[n-1]+(Xin[n]-Xout[n-1]) >> Coeff
	int Xout_prev_uncalib;
	int Yout_prev_uncalib;
	int Zout_prev_uncalib;
} Accel_State_t;


//Declare Global variables

//This holds the State for LSM6DSL Accel Driver
extern Accel_State_t Accel_State;

//Open the Sensor with Rate and Range
extern int Open_Accel(int, int, int);
//Copy Sensor data from SM
extern void Copy_Sensor_Accel_Data(void);
//Copy FFE data to SM
extern void Copy_FFE_Data_For_Accel_Sensor(void);
//Process the Accel sensor 
extern void Process_Accel(void);

extern int convert_m4_freq_to_ffe_freq_accel(int frequency);
double Get_Accel_Scale_Factor(void);

#endif //__LSM6DSL_ACCEL_H__
