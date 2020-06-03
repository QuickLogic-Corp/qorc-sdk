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

#ifndef _CRC32_GCC_TABLE_H
#define _CRC32_GCC_TABLE_H

//Use the gdb crc function directly
extern unsigned int xcrc32 (const unsigned char *buf, int len, unsigned int init);

//Or use the QuickAI Wrapper functions

//Initiliaze the static crc variable
extern void init_crc32(void);

//Compute the crc on the input 8-bit data buffer
extern void update_crc32(const unsigned char *buf, int count);

//Return the current computed value
extern unsigned int get_crc32(void);

//Set the start value for crc 
extern void set_crc32(unsigned int crc);

#endif
