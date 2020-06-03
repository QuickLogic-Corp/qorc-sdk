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

/** @file micro_tick64.h */
/*==========================================================
 *
 *    File   : micro_tick64.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef _MICRO_TICK64_H_

#define _MICRO_TICK64_H_

/** @brief set tick count to the given value
 *
 *  @param[in] new_val new value for the tick count in micro-seconds
 *
 */
extern void xTaskSet_uSecCount(uint64_t new_val);

/** @brief return the current tick count in micro-seconds
 * 
 * @return return current tick count in micro-seconds
 */
extern uint64_t xTaskGet_uSecCount(void);


/** @brief return given tick count in micro-seconds
 * 
 * @return return given tick count in micro-seconds
 */
extern uint64_t convert_to_uSecCount(uint32_t tickCount);

#endif /* _MICRO_TICK64_H_ */
