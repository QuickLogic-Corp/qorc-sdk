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
 *    File   : ble_collection_defs.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

/*
 * FIXME: Many of these need to be cleaned up and/or deleted.
 */

#ifndef BLE_MOTION_H__
#define BLE_MOTION_H__

#ifdef BLE_STACK_SUPPORT_REQD
#include "ble.h"
#include "ble_srv_common.h"
#include "app_util_platform.h"
#endif
#include <stdint.h>
#include <stdbool.h>

#ifdef __GNUC__
	#ifdef PACKED
		#undef PACKED
	#endif

	#define PACKED(TYPE) TYPE __attribute__ ((packed))

#else

	#ifdef PACKED
		#undef PACKED
	#endif
//#define PACKED(TYPE) TYPE
	#define PACKED(TYPE) __packed TYPE
#endif

/**< The UUID of the Motion Service. */
#define BLE_UUID_MOTION_SERVICE 			0x0100
#define BLE_GATT_ATT_MTU_DEFAULT    23
#define BLE_MOTION_MAX_DATA_LEN (BLE_GATT_ATT_MTU_DEFAULT - 3) /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Motion service module. */

#define BLE_MOTION_CONFIG_MPUF_MIN         5    //< Minimum motion processing frequency [Hz]. (4 Hz minimum to mpu_set_sample_rate())
#define BLE_MOTION_CONFIG_MPUF_MAX			6666 //< Maximum motion processing frequency [Hz].
#define BLE_MOTION_CONFIG_WOM_MAX          1    //< Wake on motion off.
#define BLE_MOTION_CONFIG_SD_STORE_MAX     1    //< SD Storage On.
#define BLE_MOTION_CONFIG_SENSOR_USE_MAX   1   	//< SD Storage On.
#define BLE_MOTION_BUFFER_SAMPLES 			6
#define QUICKAI_BLE_UART_ENABLE 1  //1= Enable QUICKAI board BLE as a UART transport


typedef PACKED( struct { int16_t x; int16_t y; int16_t z; }) ble_xyz16_t;
typedef PACKED( struct ble_accel_gyro { ble_xyz16_t accel; ble_xyz16_t gyro; } ) ble_accel_gyro_t;

#if 0
typedef PACKED( struct
{
	int16_t x;
	int16_t y;
	int16_t z;
})ble_motion_raw_accel_t;
#endif

#if 0
typedef PACKED( struct
{
	int16_t x;
	int16_t y;
	int16_t z;
})ble_motion_raw_gyro_t;
#endif

#if 0
typedef PACKED( struct
{
	int16_t x;
	int16_t y;
	int16_t z;
})ble_motion_raw_compass_t;
#endif

#if 0
typedef PACKED( struct
{
	ble_motion_raw_accel_t   accel;
	ble_motion_raw_gyro_t    gyro;
	//ble_motion_raw_compass_t compass;
})ble_motion_raw_t;
#endif



typedef enum
{
	BLE_MOTION_EVT_CONFIG_RECEIVED,
	BLE_MOTION_EVT_NOTIF_RAW,
	BLE_MOTION_EVT_NOTIF_SCALED,
}ble_motion_evt_type_t;

#endif // BLE_MOTION_H__
