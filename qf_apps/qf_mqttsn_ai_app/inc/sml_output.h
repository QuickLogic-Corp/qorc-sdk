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

#ifndef __SML_OUTPUT_H_
#define __SML_OUTPUT_H_

#include <stdbool.h>
#include <stdint.h>
#include <string.h>


uint32_t sml_output_results(uint16_t model, uint16_t classification);

uint32_t sml_output_init(void * p_module);


#endif //__SML_OUTPUT_H_