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
 *    File   : ble_pme_defs.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef _BLE_PME_DEFS_H_
#define _BLE_PME_DEFS_H_

#include <stdint.h>
#include <stdbool.h>
#ifdef SENSIML_RECOGNITION
#include "kb.h"
#include "kb_defines.h"

#define BLE_PME_MAX_FV_FRAME_SZ MAX_VECTOR_SIZE

#else
#define BLE_PME_MAX_FV_FRAME_SZ 2 //not for collection.
#endif
typedef struct
{
    uint16_t context;
    uint16_t classification;

    uint8_t fv_len; //Actual length to read
    uint8_t feature_vector[BLE_PME_MAX_FV_FRAME_SZ]; //Max features reporting out is 128

} ble_pme_result_w_fv_t;

typedef struct
{
    uint16_t context;
    uint16_t classification;
} ble_pme_result_t;


#endif //_BLE_PME_DEFS_H_
