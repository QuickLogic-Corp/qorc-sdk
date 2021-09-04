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
 *    File   : debug.h
 *    Purpose:      
 *                                                          
 *=========================================================*/

#ifndef __DEBUG_H
#define __DEBUG_H

#if defined(DEBUG)
#define err(...) do { /*printf("[BL] [ERR] [%s] : ", __FUNCTION__);*/ printf(__VA_ARGS__); } while(0)
#else
#define err(...) do { } while(0)
#endif


#if 0
#if defined(DEBUG)
#define dbg(...) do { /*printf("[BL] [DBG] [%s] : ", __FUNCTION__);*/ printf(__VA_ARGS__); } while(0)
#else
#define dbg(...) do { } while(0)
#endif
#endif

#endif // __DEBUG_H
