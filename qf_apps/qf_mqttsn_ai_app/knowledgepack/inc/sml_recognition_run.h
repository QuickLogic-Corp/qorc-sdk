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

#ifndef __SENSIML_RECOGNITION_RUN_H__
#define __SENSIML_RECOGNITION_RUN_H__



int sml_recognition_run_batch(signed short *data_batch, int batch_sz, uint8_t num_sensors, uint32_t sensor_id);
int sml_recognition_run_single(signed short *data, uint32_t sensor_id);

#endif //__SENSIML_RECOGNITION_RUN_H__