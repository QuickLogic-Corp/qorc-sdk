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
 *    File   : QL_Trace.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef _QL_TRACE_H_
#define _QL_TRACE_H_
#include <stdarg.h>

#if !defined( _EnD_Of_Fw_global_config_h )
#error "Include Fw_global_config.h first"
#endif
     
/* These (assert & return values) has to be moved to a separate file */
typedef enum {QL_STATUS_OK, QL_STATUS_ERROR, QL_STATUS_ERROR_NOMEM, QL_STATUS_BUSY, QL_STATUS_TIMEOUT, QL_STATUS_NOT_IMPLEMENTED} QL_Status;
extern void dbg_assert( const char *filename, int lineno, const char *msg );
#define QL_ASSERT(_x_)	do { if(!(_x_)){ dbg_assert( __FILE__, __LINE__, #_x_ ); } } while(0)

/* These macros can be used to disable particular trace level per file. 
 * QL_TRACE_XXXX_ENABLE	- can be undefined in a particular file to disable XXXX Trace level for that particular file alone.
 * QL_TRACE_XXXX_ENABLE	- can be undefined in a particular file to disable trace for XXXX module for that particular file alone.
 */
#define QL_TRACE_ERROR_ENABLE
#define QL_TRACE_WARN_ENABLE
#define QL_TRACE_DEBUG_ENABLE

#define QL_TRACE_HAL_ENABLE
//#define QL_TRACE_SENSOR_ENABLE
//#define QL_TRACE_RTOS_ENABLE
//#define QL_TRACE_TEST_ENABLE
//#define QL_TRACE_FFE_ENABLE

/**********************************/

#define BIT(_x_) (1 << (_x_))

/* Debug Levels */
#define QL_DEBUG_TRACE_ERROR	BIT(0)
#define QL_DEBUG_TRACE_WARN	BIT(1)
#define QL_DEBUG_TRACE_DEBUG	BIT(2)

/* Modules */
#define QL_MODULE_HAL		BIT(0)
#define QL_MODULE_SENSOR	BIT(1)
#define QL_MODULE_RTOS		BIT(2)
#define QL_MODULE_TEST		BIT(3)
#define QL_MODULE_FFE		BIT(4)


/* Enable Tracing for below given modules .
 * This will be used for runtime check to emit trace messages from the particular modules
 */
#define TRACE_ENABLED_MODULES	(QL_MODULE_HAL | QL_MODULE_SENSOR | QL_MODULE_RTOS | QL_MODULE_TEST | QL_MODULE_FFE)

/* Enable Trace levels as given below for all modules mentioned above (TRACE_ENABLED_MODULES).
 * This will be used for runtime check to emit trace messages of particular trace levels.
 */
#define TRACE_LEVEL_ENABLED	(QL_DEBUG_TRACE_ERROR | QL_DEBUG_TRACE_WARN | QL_DEBUG_TRACE_DEBUG)


QL_Status QL_Trace(unsigned int module, unsigned int tracelevel, char *header, const char *function, unsigned int linenum, char *format, ...);

#define QL_TRACE(_MODULE_, _TRACE_LEVEL_, _TRACE_HEADER_, _FMT_, ...)	\
	QL_Trace(_MODULE_, _TRACE_LEVEL_, _TRACE_HEADER_, ___FUNCTION__, __LINE__, _FMT_, ##__VA_ARGS__)

#define STR_HELPER(x) #x         											// to remove warnings
#ifdef QL_TRACE_ERROR_ENABLE                                                                                                    //JANGID added STR_HELPER to remove warning .._MODULE_ raw and expanded form form together
#define QL_TRACE_ERROR(_MODULE_, _FMT_, ...)	\
	QL_Trace(_MODULE_, QL_DEBUG_TRACE_ERROR, "["STR_HELPER(_MODULE_)"][ERROR]",__FUNCTION__, __LINE__, _FMT_, ##__VA_ARGS__)
#else
#define QL_TRACE_ERROR(_MODULE_, _FMT_, ...)
#endif

#ifdef QL_TRACE_WARN_ENABLE
#define QL_TRACE_WARN(_MODULE_, _FMT_, ...)	\
	QL_Trace(_MODULE_, QL_DEBUG_TRACE_WARN, "["STR_HELPER(_MODULE_)"][WARN]",__FUNCTION__, __LINE__, _FMT_, ##__VA_ARGS__)
#else
#define QL_TRACE_WARN(_MODULE_, _FMT_, ...)
#endif

#ifdef QL_TRACE_DEBUG_ENABLE
#define QL_TRACE_DEBUG(_MODULE_, _FMT_, ...)	\
	QL_Trace(_MODULE_, QL_DEBUG_TRACE_DEBUG, "[" STR_HELPER(_MODULE_) "][DEBUG]",__FUNCTION__, __LINE__, _FMT_, ##__VA_ARGS__)
#else
#define QL_TRACE_DEBUG(_MODULE_, _FMT_, ...)
#endif

#ifdef QL_TRACE_HAL_ENABLE
#define QL_TRACE_HAL_ERROR(_FMT_, ...)	QL_TRACE_ERROR(QL_MODULE_HAL, _FMT_, ##__VA_ARGS__)
#define QL_TRACE_HAL_WARN(_FMT_, ...)	QL_TRACE_WARN(QL_MODULE_HAL, _FMT_, ##__VA_ARGS__)
#define QL_TRACE_HAL_DEBUG(_FMT_, ...)	QL_TRACE_DEBUG(QL_MODULE_HAL, _FMT_, ##__VA_ARGS__)
#else
#define QL_TRACE_HAL_ERROR(_FMT_, ...)
#define QL_TRACE_HAL_WARN(_FMT_, ...)
#define QL_TRACE_HAL_DEBUG(_FMT_, ...)
#endif

#ifdef QL_TRACE_SENSOR_ENABLE
#define QL_TRACE_SENSOR_DEBUG(_FMT_, ...)	QL_TRACE_DEBUG(QL_MODULE_SENSOR, _FMT_, ##__VA_ARGS__)
#define QL_TRACE_SENSOR_ERROR(_FMT_, ...)	QL_TRACE_ERROR(QL_MODULE_SENSOR, _FMT_, ##__VA_ARGS__)
#define QL_TRACE_SENSOR_WARN(_FMT_, ...)	QL_TRACE_WARN(QL_MODULE_SENSOR, _FMT_, ##__VA_ARGS__)
#else
#define QL_TRACE_SENSOR_DEBUG(_FMT_, ...)
#define QL_TRACE_SENSOR_ERROR(_FMT_, ...)
#define QL_TRACE_SENSOR_WARN(_FMT_, ...)
#endif

#ifdef QL_TRACE_RTOS_ENABLE
#define QL_TRACE_RTOS_DEBUG(_FMT_, ...)	QL_TRACE_ERROR(QL_MODULE_RTOS, _FMT_, ##__VA_ARGS__)
#define QL_TRACE_RTOS_ERROR(_FMT_, ...)	QL_TRACE_ERROR(QL_MODULE_RTOS, _FMT_, ##__VA_ARGS__)
#define QL_TRACE_RTOS_WARN(_FMT_, ...)	QL_TRACE_ERROR(QL_MODULE_RTOS, _FMT_, ##__VA_ARGS__)
#else
#define QL_TRACE_RTOS_DEBUG(_FMT_, ...)	
#define QL_TRACE_RTOS_ERROR(_FMT_, ...)			
#define QL_TRACE_RTOS_WARN(_FMT_, ...)			
#endif

#ifdef QL_TRACE_TEST_ENABLE
#define QL_TRACE_TEST_DEBUG(_FMT_, ...)	QL_TRACE_DEBUG(QL_MODULE_TEST, _FMT_, ##__VA_ARGS__)
#define QL_TRACE_TEST_ERROR(_FMT_, ...)	QL_TRACE_ERROR(QL_MODULE_TEST, _FMT_, ##__VA_ARGS__)
#define QL_TRACE_TEST_WARN(_FMT_, ...)	QL_TRACE_WARN(QL_MODULE_TEST, _FMT_, ##__VA_ARGS__)
#else
#define QL_TRACE_TEST_DEBUG(_FMT_, ...)	
#define QL_TRACE_TEST_ERROR(_FMT_, ...)			
#define QL_TRACE_TEST_WARN(_FMT_, ...)			
#endif

#ifdef QL_TRACE_FFE_ENABLE
#define QL_TRACE_FFE_DEBUG(_FMT_, ...)	QL_TRACE_DEBUG(QL_MODULE_FFE, _FMT_, ##__VA_ARGS__)
#define QL_TRACE_FFE_ERROR(_FMT_, ...)	QL_TRACE_ERROR(QL_MODULE_FFE, _FMT_, ##__VA_ARGS__)
#define QL_TRACE_FFE_WARN(_FMT_, ...)	QL_TRACE_WARN(QL_MODULE_FFE, _FMT_, ##__VA_ARGS__)
#else
#define QL_TRACE_FFE_DEBUG(_FMT_, ...)	
#define QL_TRACE_FFE_ERROR(_FMT_, ...)			
#define QL_TRACE_FFE_WARN(_FMT_, ...)			
#endif

#endif	/* _QL_TRACE_H_ */
