/** @file ssi_comms.h */

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

#ifndef SSI_COMMS_H
#define SSI_COMMS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Fw_global_config.h"
#include "FreeRTOS.h"

extern bool is_ssi_connected;
extern signed portBASE_TYPE StartSimpleStreamingInterfaceTask( void);
void ssi_publish_sensor_data( uint8_t *p_source, int ilen );
void ssi_publish_sensor_results( uint8_t *p_source, int ilen );

#ifdef __cplusplus
}
#endif

#endif /* SSI_COMMS_H */
