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

#ifndef _KB_DEFINES_H_
#define _KB_DEFINES_H_
#include "kb_typedefs.h"
#ifdef DSW_BUILD
#include "infra/log.h"
#endif
#include "stdio.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifndef SML_KP_DEBUG
#define SML_KP_DEBUG 1
#endif
#define KB_LOG_LEVEL 1

#if SML_KP_DEBUG
	#if defined(WIN32) || defined(KBSIM)
		#define dbgprintlev(level, ...) if(level <= KB_LOG_LEVEL) {printf(__VA_ARGS__);printf("\n");}
		#define pr_info(logger, ...) {printf(__VA_ARGS__); printf("\n");}
	#elif defined(DSW_BUILD)
		#define dbgprintlev(level, ...) if(level <= kb_log_level) pr_info(LOG_MODULE_MAIN, __VA_ARGS__)
	#elif NRF_LOG_ENABLED
		#define dbgprintlev(level, ...) if(level <= KB_LOG_LEVEL) {NRF_LOG_INFO(__VA_ARGS__);}
	#else
    		#define dbgprintlev(level,...)
	#endif

#else
    #define dbgprintlev(level,...)
#endif //DEBUG

#ifdef __cplusplus
}
#endif

#endif //_KB_DEFINES_H_
