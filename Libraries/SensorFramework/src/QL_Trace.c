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
 *    File   : QL_Trace.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"
#include <stdio.h>
#include <stdarg.h>
#include "QL_Trace.h"

#ifndef TRACE_ENABLED_MODULES
#define TRACE_ENABLED_MODULES	(~0)
#endif

#ifndef TRACE_LEVEL_ENABLED
#define TRACE_LEVEL_ENABLED	(~0)
#endif

#define QL_TRACE_PRINT_MAX (512)

static unsigned int TraceEnabledModuleBitmask = TRACE_ENABLED_MODULES, TraceLevelsEnabled = TRACE_LEVEL_ENABLED;

QL_Status QL_Trace(unsigned int module, unsigned int tracelevel, char *header, const char *function, unsigned int linenum, char *format, ...)
{
	int size;
	char tracebuf[QL_TRACE_PRINT_MAX] = {0};
	va_list va;
	
	/* Check whether tracing for the module specified is enabled or not.
	 * Not enabling debugging for a particular module is not an error condition
	 * so we return QL_STATUS_OK here.
	 */
	if (!((module & TraceEnabledModuleBitmask) && (tracelevel & TraceLevelsEnabled)))
		return QL_STATUS_OK;
	
	va_start(va, format);
	size = vsnprintf(tracebuf, sizeof(tracebuf) - 1, format, va);
	va_end(va);

	if (size < 0)
		return QL_STATUS_ERROR;

	/* print to console using normal printf - any alternate functions can be used
	 * here to redirect the trace messages
	 */
	printf("%s:[%s@%d]:%s", header, function, linenum, tracebuf);

	return QL_STATUS_OK;
}
