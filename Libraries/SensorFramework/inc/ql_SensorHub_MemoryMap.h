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
 *    File   : ql_SensorHub_MemoryMap.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef APP_SHUB_INC_QL_SENSORHUB_MEMORYMAP_H_
#define APP_SHUB_INC_QL_SENSORHUB_MEMORYMAP_H_

#if !defined(_EnD_Of_Fw_global_config_h)
#error "include Fw_global_config.h first"
#endif

#define SENSORHUB_MEMORYMAP_BASE_ADDR				0x2007C000


/* SECTION 1: LIVE SENSOR DATA */
#define LIVE_SENSOR_DATA_SECTION_ADDR				(SENSORHUB_MEMORYMAP_BASE_ADDR)

#define LIVE_SENSOR_DATA_PACKET_SIZE				(16)

#define LIVE_SENSOR_DATA_ADDR						(LIVE_SENSOR_DATA_SECTION_ADDR)

#define LIVE_SENSOR_DATA_ACCEL_ADDR					(LIVE_SENSOR_DATA_ADDR)
#define LIVE_SENSOR_DATA_ACCEL2_ADDR				(LIVE_SENSOR_DATA_ADDR + LIVE_SENSOR_DATA_PACKET_SIZE)
#define LIVE_SENSOR_DATA_MAG_ADDR					(LIVE_SENSOR_DATA_ACCEL2_ADDR + LIVE_SENSOR_DATA_PACKET_SIZE)
#define LIVE_SENSOR_DATA_GYRO_ADDR					(LIVE_SENSOR_DATA_MAG_ADDR + LIVE_SENSOR_DATA_PACKET_SIZE)
#define LIVE_SENSOR_DATA_PCG_ADDR					(LIVE_SENSOR_DATA_GYRO_ADDR + LIVE_SENSOR_DATA_PACKET_SIZE)
#define LIVE_SENSOR_DATA_DT_ADDR					(LIVE_SENSOR_DATA_PCG_ADDR + LIVE_SENSOR_DATA_PACKET_SIZE)

#define LIVE_SENSOR_DATA_SECTION_SIZE				(10*LIVE_SENSOR_DATA_PACKET_SIZE) // support 10 sensors.


/* SECTION 2: BATCH BUFFER INFO FOR SENSOR */
#define BATCH_BUFFER_INFO_SECTION_ADDR				(LIVE_SENSOR_DATA_SECTION_ADDR + LIVE_SENSOR_DATA_SECTION_SIZE)

#define BATCH_BUFFER_INFO_ADDR						(BATCH_BUFFER_INFO_SECTION_ADDR)
#define BATCH_BUFFER_INFO_SIZE						(8)


#define BATCH_BUFFER_INFO_ADDR_ACCEL				(BATCH_BUFFER_INFO_ADDR)
#define BATCH_BUFFER_INFO_ADDR_ACCEL1				(BATCH_BUFFER_INFO_ADDR_ACCEL + BATCH_BUFFER_INFO_SIZE)
#define BATCH_BUFFER_INFO_ADDR_MAG					(BATCH_BUFFER_INFO_ADDR_ACCEL1 + BATCH_BUFFER_INFO_SIZE)
#define BATCH_BUFFER_INFO_ADDR_GYRO					(BATCH_BUFFER_INFO_ADDR_MAG + BATCH_BUFFER_INFO_SIZE)
#define BATCH_BUFFER_INFO_ADDR_SENSIML_APP			(BATCH_BUFFER_INFO_ADDR_GYRO + BATCH_BUFFER_INFO_SIZE)

#define BATCH_BUFFER_INFO_SECTION_SIZE				(5*BATCH_BUFFER_INFO_SIZE) // support 5 sensors.


/* SECTION 3: BATCH BUFFER DATA FOR SENSOR */
#define BATCH_BUFFER_DATA_SECTION_ADDR				(BATCH_BUFFER_INFO_SECTION_ADDR + BATCH_BUFFER_INFO_SECTION_SIZE)

#define BATCH_BUFFER_DATA_ADDR_ACCEL				(BATCH_BUFFER_DATA_SECTION_ADDR)
#define BATCH_BUFFER_DATA_SIZE_ACCEL				(12*400*1) // 12B packet * 800 packets/second * 1 second MAX.

#if (defined(PCG_AND_DT_ON_M4_BATCH) || defined (PCG_ALGO_ON_M4_BATCH)) // we do not have extra batch buffer, this is special case of 2 ACCELs only.

#define BATCH_BUFFER_DATA_ADDR_ACCEL1					(BATCH_BUFFER_DATA_ADDR_ACCEL + BATCH_BUFFER_DATA_SIZE_ACCEL)
#define BATCH_BUFFER_DATA_SIZE_ACCEL1					(12*400*1) // 12B packet * 800 packets/second * 1 second MAX.

#define BATCH_BUFFER_DATA_ADDR_MAG						(0)
#define BATCH_BUFFER_DATA_SIZE_MAG						(0) // 12B packet * 800 packets/second * 1 second MAX.


#else

#define BATCH_BUFFER_DATA_ADDR_ACCEL1					(0)
#define BATCH_BUFFER_DATA_SIZE_ACCEL1					(0) // 12B packet * 800 packets/second * 1 second MAX.

#define BATCH_BUFFER_DATA_ADDR_MAG						(BATCH_BUFFER_DATA_ADDR_ACCEL + BATCH_BUFFER_DATA_SIZE_ACCEL)
#define BATCH_BUFFER_DATA_SIZE_MAG						(12*400*1) // 12B packet * 800 packets/second * 1 second MAX.

#endif // PCG_AND_DT_ON_M4_BATCH


#define BATCH_BUFFER_DATA_ADDR_GYRO					(BATCH_BUFFER_DATA_ADDR_MAG + BATCH_BUFFER_DATA_SIZE_MAG)
#define BATCH_BUFFER_DATA_SIZE_GYRO					(12*400*1) // 12B packet * 800 packets/second * 1 second MAX.

#define BATCH_BUFFER_DATA_SECTION_SIZE				(BATCH_BUFFER_DATA_SIZE_ACCEL + BATCH_BUFFER_DATA_SIZE_MAG + BATCH_BUFFER_DATA_SIZE_GYRO)

//This is for SensiML Data Collection App using FFE_DRIVERS   
#define BATCH_BUFFER_DATA_ADDR_SENSIML_APP          (BATCH_BUFFER_DATA_SECTION_ADDR)
#define BATCH_BUFFER_DATA_SECTION_SIZE_SENSIML_APP  (12 * 200 * 1)
   

#endif /* APP_SHUB_INC_QL_SENSORHUB_MEMORYMAP_H_ */
