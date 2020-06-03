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
 * If the assert occurs - you can look at this
 * trace array and see values of variables at
 * various points in time
 *
 * The "lineno" tells you the flow and decisions
 * made during execution of the code.
 */
 
#ifndef _DBGTRACE_H_
#define _DBGTRACE_H_

#include <stdint.h>
#include "Fw_global_config.h"
#include "dbg_uart.h"

typedef struct dbgtrace {
    uint32_t lineno;
    uint32_t value;
} dbgtrace_t;

void dbgtrace( int lineno, uint32_t value,  dbgtrace_t* adbgtrace, int kdbgtrace, int* pidbgtrace);
void dbgtracePrint(dbgtrace_t* adbgtrace, int kdbgtrace, int idbgtrace);
#endif