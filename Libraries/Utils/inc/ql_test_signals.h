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

#ifndef _QL_MATHOPS__H
#define _QL_MATHOPS__H

#include <stdint.h>

extern int16_t SineTable_test[240];
inline int32_t isin_S3(int32_t x);
short chirp(short *p_dest, uint32_t n);
short tone(short *p_dest, uint32_t n);
short amp(short *p_dest, uint32_t n);
short tone_lut(short *p_dest, uint32_t n);

#endif // _QL_MATHOPS__H
