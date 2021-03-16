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
 *    File   : ql_time.c
 *    Purpose: This file contains time routines for EOSS3 platform
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <eoss3_dev.h>
#include <FreeRTOS.h>
#include <task.h>
#include <ql_time.h>
#include <eoss3_hal_rtc.h>

#define USE_RTC

#ifndef USE_RTC
static uint32_t seconds_since_epoch, ql_diff_ticks;
#endif
static signed long timezone = 0;                 // Difference in seconds between GMT and local time

#define YEAR0                   1900
#define EPOCH_YR                1970
#define SECS_DAY                (24L * 60L * 60L)
#define LEAPYEAR(year)          (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YEARSIZE(year)          (LEAPYEAR(year) ? 366 : 365)

#define TIME_MAX                2147483647L

const int _ytab[2][12] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

//static int _daylight = 0;                  // Non-zero if daylight savings time is used 
/*
void periodic_cb(void)
{
	time_t ltime;
	struct xQ_Disp_Pck	xSendMsg;
	
	ltime = ql_time(NULL);
	ql_localtime_r(&ltime, &calendar);
	printf("%04d/%02dm/%02d -- %02dh.%02dm.%02ds\r\n", calendar.tm_year+1900,
		calendar.tm_mon+1, calendar.tm_mday, calendar.tm_hour, calendar.tm_min, calendar.tm_sec);

	
	xSendMsg.ucCommand = 0x02;
	xSendMsg.ucSrc = 0xFF;
	if(calendar.tm_sec == 0)
		xQueueSend( xDispQueue, &xSendMsg, ( TickType_t ) 10 );
	
	return;
}
*/

struct tm *ql_gmtime_r(const time_t *timer, struct tm *tmbuf)
{
	time_t time = *timer;
	unsigned long dayclock, dayno;
	int year = EPOCH_YR;

	dayclock = (unsigned long) time % SECS_DAY;
	dayno = (unsigned long) time / SECS_DAY;

	tmbuf->tm_sec = dayclock % 60;
	tmbuf->tm_min = (dayclock % 3600) / 60;
	tmbuf->tm_hour = dayclock / 3600;
	tmbuf->tm_wday = (dayno + 4) % 7; // Day 0 was a thursday
	while (dayno >= (unsigned long) YEARSIZE(year)) {
		dayno -= YEARSIZE(year);
		year++;
	}
	tmbuf->tm_year = year - YEAR0;
	tmbuf->tm_yday = dayno;
	tmbuf->tm_mon = 0;
	while (dayno >= (unsigned long) _ytab[LEAPYEAR(year)][tmbuf->tm_mon]) {
		dayno -= _ytab[LEAPYEAR(year)][tmbuf->tm_mon];
		tmbuf->tm_mon++;
	}
	tmbuf->tm_mday = dayno + 1;
	tmbuf->tm_isdst = 0;
	return tmbuf;
}

struct tm *ql_localtime_r(const time_t *timer, struct tm *tmbuf)
{
	time_t t;
	t = *timer - timezone;
	return ql_gmtime_r(&t, tmbuf);
}

//time_t mktime(struct tm *tmbuf)
//{
//	long day, year;
//	int tm_year;
//	int yday, month;
//	/*unsigned*/ long seconds;
//	int overflow;
//	long dst;
//
//	tmbuf->tm_min += tmbuf->tm_sec / 60;
//	tmbuf->tm_sec %= 60;
//	if (tmbuf->tm_sec < 0) {
//		tmbuf->tm_sec += 60;
//		tmbuf->tm_min--;
//	}
//	tmbuf->tm_hour += tmbuf->tm_min / 60;
//	tmbuf->tm_min = tmbuf->tm_min % 60;
//	if (tmbuf->tm_min < 0) {
//		tmbuf->tm_min += 60;
//		tmbuf->tm_hour--;
//	}
//	day = tmbuf->tm_hour / 24;
//	tmbuf->tm_hour= tmbuf->tm_hour % 24;
//	if (tmbuf->tm_hour < 0) {
//		tmbuf->tm_hour += 24;
//		day--;
//	}
//	tmbuf->tm_year += tmbuf->tm_mon / 12;
//	tmbuf->tm_mon %= 12;
//	if (tmbuf->tm_mon < 0) {
//		tmbuf->tm_mon += 12;
//		tmbuf->tm_year--;
//	}
//	day += (tmbuf->tm_mday - 1);
//	while (day < 0) {
//		if(--tmbuf->tm_mon < 0) {
//			tmbuf->tm_year--;
//			tmbuf->tm_mon = 11;
//		}
//		day += _ytab[LEAPYEAR(YEAR0 + tmbuf->tm_year)][tmbuf->tm_mon];
//	}
//	while (day >= _ytab[LEAPYEAR(YEAR0 + tmbuf->tm_year)][tmbuf->tm_mon]) {
//		day -= _ytab[LEAPYEAR(YEAR0 + tmbuf->tm_year)][tmbuf->tm_mon];
//		if (++(tmbuf->tm_mon) == 12) {
//			tmbuf->tm_mon = 0;
//			tmbuf->tm_year++;
//		}
//	}
//	tmbuf->tm_mday = day + 1;
//	year = EPOCH_YR;
//	if (tmbuf->tm_year < year - YEAR0)
//		return (time_t) -1;
//	seconds = 0;
//	day = 0;                      // Means days since day 0 now
//	overflow = 0;
//
//// Assume that when day becomes negative, there will certainly
//// be overflow on seconds.
//// The check for overflow needs not to be done for leapyears
//// divisible by 400.
//// The code only works when year (1970) is not a leapyear.
//	tm_year = tmbuf->tm_year + YEAR0;
//
//	if (TIME_MAX / 365 < tm_year - year) overflow++;
//	day = (tm_year - year) * 365;
//	if (TIME_MAX - day < (tm_year - year) / 4 + 1) overflow++;
//	day += (tm_year - year) / 4 + ((tm_year % 4) && tm_year % 4 < year % 4);
//	day -= (tm_year - year) / 100 + ((tm_year % 100) && tm_year % 100 < year % 100);
//	day += (tm_year - year) / 400 + ((tm_year % 400) && tm_year % 400 < year % 400);
//
//	yday = month = 0;
//	while (month < tmbuf->tm_mon)
//	{
//		yday += _ytab[LEAPYEAR(tm_year)][month];
//		month++;
//	}
//	yday += (tmbuf->tm_mday - 1);
//	if (day + yday < 0) overflow++;
//	day += yday;
//
//	tmbuf->tm_yday = yday;
//	tmbuf->tm_wday = (day + 4) % 7;               // Day 0 was thursday (4)
//
//	seconds = ((tmbuf->tm_hour * 60L) + tmbuf->tm_min) * 60L + tmbuf->tm_sec;
//
//	if ((TIME_MAX - seconds) / SECS_DAY < day) overflow++;
//	seconds += day * SECS_DAY;
//
//// Now adjust according to timezone and daylight saving time
//	if (((timezone > 0) && (TIME_MAX - timezone < seconds)) || 
//	    ((timezone < 0) && (seconds < -timezone))) {
//		overflow++;
//	}
//	seconds += timezone;
//
//	if (tmbuf->tm_isdst) {
//		dst = _dstbias;
//	} else {
//		dst = 0;
//	}
//
//	if (dst > seconds)
//		overflow++;        // dst is always non-negative
//	seconds -= dst;
//
//	if (overflow)
//		return (time_t) -1;
//
//	if ((time_t) seconds != seconds)
//		return (time_t) -1;
//	return (time_t) seconds;
//}

#ifndef USE_RTC
static int get_time_from_ticks(time_t *secs, time_t *millisecs)
{
	volatile TickType_t ticks;
	ticks = xTaskGetTickCount();
	time_t sec;
	if (!secs)
		return -1;
	sec = (ticks - ql_diff_ticks) / configTICK_RATE_HZ; /* this is 1000 now */
	if (millisecs)
		*millisecs = ticks % 1000;
	*secs = seconds_since_epoch + sec;
	return 0;
}
#else
static int get_time_from_rtc(time_t *secs, time_t *millisecs)
{
	HAL_RTC_Init(0);
	if (millisecs)
		*millisecs = 0;
	if (HAL_RTC_GetTime((uint32_t *)secs) == HAL_OK)
		return 0;
	else
		return -1;
	    
}

static int setup_rtc_time(time_t secs_since_epoch)
{
	HAL_RTC_Init(0);
	if (HAL_RTC_SetTime(secs_since_epoch) == HAL_OK)
		return 0;
	else
		return -1;
}
#endif

time_t ql_time(time_t *t)
{
	time_t secs;
#ifndef USE_RTC
	if (get_time_from_ticks(&secs, NULL) != 0) {
		return -1;
	}
#else
	if (get_time_from_rtc(&secs, NULL) != 0) {
		return -1;
	}
#endif
	if (t)
		*t = secs;
	return (secs);
}

int ql_sys_settime(time_t secs_since_epoch)
{

#ifdef USE_RTC
	setup_rtc_time(secs_since_epoch);
#else
	seconds_since_epoch = secs_since_epoch;
	ql_diff_ticks = xTaskGetTickCount();
#endif
	return 0;
}

/* gives millisecond count from the start of system */
uint32_t get_msecs_ts(void)
{
	return xTaskGetTickCount();
}


intptr_t ql_lw_timer_start( void )
{
    return (intptr_t)(get_msecs_ts());
}

int      ql_lw_timer_consumed( intptr_t token )
{
    uint32_t now;
    uint32_t elapsed;
    
    now = (intptr_t)(get_msecs_ts());
    
    elapsed = now - token;
    return (int)(elapsed);
}

int      ql_lw_timer_remain( intptr_t token, int timeout_msecs )
{
    int tmp;
    
    if( timeout_msecs <= 0 ){
        return 0;
    }
    
    tmp = timeout_msecs - ql_lw_timer_consumed( token );
    if( tmp < 0 ){
        tmp = 0;
    }
    return tmp;
}

int      ql_lw_timer_is_expired( intptr_t token, int msecs )
{
    if( ql_lw_timer_remain(token,msecs) <= 0 ){
        return 1;
    } else {
        return 0;
    }
}

/* Equivalent of time() : returns the number of seconds after 1-1-1970. */
time_t FreeRTOS_time( time_t *pxTime )
{
   return ql_time(pxTime);
}
