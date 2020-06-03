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
 *    File   : ql_time.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __QL_TIME_H
#define __QL_TIME_H

#define USE_RTC
#include <time.h>

//struct tm
//{       /* date and time components */
//	int tm_sec;
//	int tm_min;
//	int tm_hour;
//	int tm_mday;
//	int tm_mon;
//	int tm_year;
//	int tm_wday;
//	int tm_yday;
//	int tm_isdst;
//};
//Tim (FAT) typedef signed long time_t;

struct tm *ql_gmtime_r(const time_t *timer, struct tm *tmbuf);
struct tm *ql_localtime_r(const time_t *timer, struct tm *tmbuf);
time_t mktime(struct tm *tmbuf);
time_t ql_time(time_t *t);
int ql_sys_settime(time_t secs_since_epoch);
void periodic_cb(void);
void update_calendar(void);

intptr_t ql_lw_timer_start( void );
int      ql_lw_timer_is_expired( intptr_t token, int msecs );
int      ql_lw_timer_remain( intptr_t token, int msecs );
int      ql_lw_timer_consumed( intptr_t token );


#endif	/* __QL_TIME_H */
