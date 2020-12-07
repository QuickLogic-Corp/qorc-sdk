#ifndef _KB_DEFINES_H_
#define _KB_DEFINES_H_
#include "kb_typedefs.h"
#if defined(WIN32) || defined(KBSIM)
#include <sys/time.h>
#endif
#include "stdio.h"
#ifdef __cplusplus
extern "C" {
#endif


#ifndef SML_KP_DEBUG
#define SML_KP_DEBUG 0
#endif
#ifndef KB_LOG_LEVEL
#define KB_LOG_LEVEL 1
#endif
#ifndef SML_PROFILER
#define SML_PROFILER 0
#endif




#if SML_KP_DEBUG
	#if defined(WIN32) || defined(KBSIM)
		#if SML_PROFILER
			#define dbgprintlev(level, ...) if(level <= KB_LOG_LEVEL) {printf(__VA_ARGS__);printf("\n");}
			#define dbgprintlev_no_nl(level, ...) if(level <= KB_LOG_LEVEL) {printf(__VA_ARGS__);}
			#define pr_info(logger, ...) {printf(__VA_ARGS__); printf("\n");}
		#else
			#define dbgprintlev(level, ...) if(level <= KB_LOG_LEVEL) {printf(__VA_ARGS__);printf("\n");}
			#define dbgprintlev_no_nl(level, ...) if(level <= KB_LOG_LEVEL) {printf(__VA_ARGS__);}
			#define pr_info(logger, ...) {printf(__VA_ARGS__); printf("\n");}
		#endif
	#elif defined(STM32L476xx)
		#include "main.h"
		#define dbgprintlev(level, ...) if(level <= KB_LOG_LEVEL) {STLBLE_PRINTF(__VA_ARGS__);STLBLE_PRINTF("\n");}
		#define dbgprintlev_no_nl(level, ...) if(level <= KB_LOG_LEVEL) {STLBLE_PRINTF(__VA_ARGS__);}
	#elif NRF_LOG_ENABLED
		#include "nrf_log.h"
		#define dbgprintlev(level, ...) if(level <= KB_LOG_LEVEL) {NRF_LOG_INFO(__VA_ARGS__);}
		#define dbgprintlev_no_nl(level, ...) dbgprintlev(level, __VA_ARGS__);
	#else
    	#define dbgprintlev(level,...)
		#define dbgprintlev_no_nl(level, ...)
	#endif

#else
    #define dbgprintlev(level,...)
	#define dbgprintlev_no_nl(level, ...)
#endif //DEBUG

#ifdef __cplusplus
}
#endif

#endif //_KB_DEFINES_H_
