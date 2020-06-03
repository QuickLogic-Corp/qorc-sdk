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
 *    File   : QL_FFE_SensorConfig.h
 *    Purpose: FFE shared variable structure definition file
 *
 * This file has to be shared between FFE & M4 code to agree upon the type of 
 * shared data structures 
 *                                                          
 *=========================================================*/

#ifndef __QL_FFE_SENSORCONFIG_H__
#define __QL_FFE_SENSORCONFIG_H__

//Sensor IDs
#define QL_FFE_SENSOR_ID_ACCEL1		(1)
#define QL_FFE_SENSOR_ID_ACCEL2		(2)
#define QL_FFE_SENSOR_ID_DOUBLETAP 	(3)
#define QL_FFE_SENSOR_ID_PCG       	(4)
#define QL_FFE_SENSOR_ID_GYRO		(5)
#define QL_FFE_SENSOR_ID_MAG		(6)
#define QL_SENSI_ML_ID_APP1     	(7)
#define QL_SENSI_ML_ID_APP2		(8)

//Number of sensors
#define QL_FFE_MAX_SENSORS	7
#define MAX_ACCEL_SENSORS 	2
#define MAX_SENSIML_PACKETS 4 // Valid ids are QL_FFE_SENSOR_ID_ACCEL1, QL_FFE_SENSOR_ID_GYRO, QL_SENSI_ML_ID_APP1, QL_SENSI_ML_ID_APP2


//Algo enable/disable
//#define INCLUDE_PCG_ALGO        0
//#define INCLUDE_DOUBLETAP_ALGO  0
#define DCIMATION_FILTER_ON     0

#define SENSOR_LIVE_DATA_INTERRUPT  1
#define DYNAMIC_BATCH_MEMORY_UPDATE 1

//Interrupt masks for direct interrupts from FFE (8 bits)
#define INT_COMMAND							0x01

//Interrupt masks for interrupts through a command (15 bits)
#define INT_CMD_ACCEL_BATCH_PERIOD_EXPIRE	0x001
#define INT_CMD_ACCEL1_BATCH_PERIOD_EXPIRE	0x002
#define INT_CMD_GYRO_BATCH_PERIOD_EXPIRE	0x004
#define INT_CMD_MAG_BATCH_PERIOD_EXPIRE		0x008
#define INT_CMD_DOUBLE_TAP_EVENT			0x010
#define INT_CMD_PCG_EVENT					0x020
#define INT_CMD_ACCEL_LIVE_DATA_AVAILABLE	0x040
#define INT_CMD_ACCEL1_LIVE_DATA_AVAILABLE	0x080
#define INT_CMD_GYRO_LIVE_DATA_AVAILABLE	0x100
#define INT_CMD_MAG_LIVE_DATA_AVAILABLE		0x200
#define INT_CMD_SENSIML_BATCH_PERIOD_EXPIRE     0x400


struct QL_FFE_Sensor {
	unsigned int SensorId;
	unsigned int outputRate;
	unsigned int enable;            //For enabling sensor
	unsigned int live_data_enable;	//For getting live data into M4
	unsigned int live_data_memPtr;  //Memory pointer for collecting live sensor data. 32 bit value (not Q15.16)

	//For batching
	unsigned int batch_mem_start, batch_mem_end;//32 bit values (not Q15.16)
	unsigned int batch_enable;  				//Batch data enable. [Note: Batch enable is independent of sensor live data enable.]
	unsigned int batchSize, batchFlush; 		//Batch size is number of packets
	unsigned int batch_info_memPtr; 			//32 bit values (not Q15.16)
	unsigned int reset_state;                    //For resetting Sensor State variables. M4 will set to 1 and after resetting FFE clears this variable.
};

struct QL_FFE_SensorAttrib_t {
	unsigned int range;
};

struct sensiML_App_Options {
	unsigned int sensiml_packetids_selected[MAX_SENSIML_PACKETS];
};

struct sensiMLpacket1 
{ //TODO: Give a better name which reflect the content of packet
	unsigned int Id_TimeStamp; // 8 bit Id, 24 bit TS
	unsigned int GyroMagnitude_AccelRawX;
	unsigned int AccelRawYZ;
};

struct sensiMLpacket2 
{ //TODO: Give a better name which reflect the content of packet
	unsigned int Id_TimeStamp; // 8 bit Id, 24 bit TS
	unsigned int GyroRawZ_AccelRawX;
	unsigned int AccelMagnitude_GyroAbsAvg;
};

struct sensiMLpacket3 
{ //TODO: Give a better name which reflect the content of packet
	unsigned int Id_TimeStamp; // 8 bit Id, 24 bit TS
	unsigned int GyroRawXY;
	unsigned int GyroRawZ_Unused;
};

struct rawSensorData { // Packet for Accel or Gyro sensor raw data
	unsigned int Id_TimeStamp; // 8 bit Id, 24 bit TS
	unsigned int RawXY;
	unsigned int RawZ_Unused16;
};

struct QL_ExportSection {

	struct QL_FFE_Sensor QL_FFE_SensorAccel[MAX_ACCEL_SENSORS];
	struct QL_FFE_Sensor QL_FFE_SensorDoubletap;
	struct QL_FFE_Sensor QL_FFE_SensorPCG;  //All accel instances together.
	struct QL_FFE_Sensor QL_FFE_SensorGyro;
	struct QL_FFE_Sensor QL_FFE_SensorMag;
    struct QL_FFE_Sensor QL_FFE_SensiML_App;
    
	struct QL_FFE_SensorAttrib_t QL_FFE_SensorAccelAttrib;
	struct QL_FFE_SensorAttrib_t QL_FFE_SensorGyroAttrib;

	struct sensiML_App_Options sensiMLAppOptions;

	unsigned int ffe_tick_ms;
	unsigned int m4_req_state, ffe_resp_state;
	unsigned int ffe_interrupt_cmd;
};

void configure_sensiml_imu_sensors(void);

#endif	/*__QL_FFE_SENSORCONFIG_H__ */
