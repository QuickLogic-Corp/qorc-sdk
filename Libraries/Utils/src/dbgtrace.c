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
 * This package is designed to help track down rare issues.
 *
 * The basic idea is that you insert trace points in your code which track the line number and a value.
 * You can use the line number to determine what branches were executed, and the value to understand
 * critical data.
 *
 * You may have multiple debug traces, and a single debug trace can extend over mutiple subroutines and mutiple files.
 *
 * Each unique debug trace uses an array to track the values, and an index into the array to determine where to write
 * the next value.  To set up a debug trace you must:
 * 1) Include dbgtrace.h
 * 2) Define the number of elements that you want to track
 * 3) Define the trace array
 * 4) Define the index
 *
 *  #include "dbgtrace.h"
 *  #define     K_DBGTRACE_FRED
 *  dbgtrace_t  adbgtraceFred[K_DBGTRACE_FRED];
 *  int         idbgtraceFred = 0;
 *
 * To trace a key location or value simply make a call to dbgtrace:
 *
 *  dbgtrace(__LINE__, value, adbgtraceFred, K_DBGTRACE_FRED, &idbgtraceFred);
 *
 * The default behavior is to treat the trace buffer as a circular buffer an overwrite the oldest result
 * If you just want to trace the most recent pass through a routine simply set the index to zero at the start
 * of the routine.
 *
 */
     
#include "dbgtrace.h"
     
void dbgtrace( int lineno, uint32_t value,  dbgtrace_t* adbgtrace, int kdbgtrace, int* pidbgtrace)
{
    if( *pidbgtrace >= kdbgtrace ){
        *pidbgtrace = 0;
    }
    adbgtrace[*pidbgtrace].lineno = lineno;
    adbgtrace[*pidbgtrace].value = value;
    *pidbgtrace += 1;
}

void dbgtracePrint(dbgtrace_t* adbgtrace, int kdbgtrace, int idbgtrace) {
    int idbgtraceLocal = idbgtrace;
    
    do {
        if (--idbgtraceLocal < 0)
            idbgtraceLocal = kdbgtrace - 1;
        dbg_str("line: ");
        dbg_int(adbgtrace[idbgtraceLocal].lineno);
        dbg_str_int(" ,", adbgtrace[idbgtraceLocal].value);
    } while (idbgtraceLocal != idbgtrace);
}