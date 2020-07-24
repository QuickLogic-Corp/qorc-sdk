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
* DataCollection.c
*
*  This example code collects the sensor data over BLE as well as on SD card.
*  The communication over BLE requires SensiML specific App running in the
*  Nordic BLE chip on board QuickAI Hardware module.
*
*  Also requires IOP Library for handling BLE messages from SensiML Data Capture Lab.
*
*/
#define DATACOLLECTION_C
#include "Fw_global_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "RtosTask.h"
#include "math.h"
#include "ql_bleTask.h"
#include "iop_messages.h"
#include "ble_collection_defs.h"
//#include "ql_adcTask.h"

#include "dcl_commands.h"
#include "DataCollection.h"
//#include "eoss3_hal_time.h"
//#include "eoss3_hal_rtc.h"
#include "dbg_uart.h"
//#include "QL_FFE_SensorConfig.h"

#if ( FFE_DRIVERS )
#include "FFE_AccelGyro.h"
#endif

#include "Sensor_Attributes.h"

#if 0

void app_datastorage_set_filename( const char *filename )
{
  /* this is the prefix for the filenames we create */
    strncpy( file_save_config.cur_filename_template,
             filename,
             sizeof(file_save_config.cur_filename_template)-1 );
    /* force terminate */
    file_save_config.cur_filename_template[sizeof(file_save_config.cur_filename_template)-1]=0;
}

#endif

