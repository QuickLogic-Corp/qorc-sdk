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
 *    File   : test_types.h
 *    Purpose: 
 *                                                          
 *=========================================================*/
 
#ifndef __TEST_TYPES_H_
#define __TEST_TYPES_H_

#include <stdio.h>
#include <stdint.h>

/* FIXME:  The CAPNAME versions need to go away */

/*! \typedef char INT8_t
 *	\brief signed character type
 */
typedef int8_t	INT8_t;

/*! \typedef unsigned char UINT8_t
 *	\brief unsigned character type
 */
typedef uint8_t	UINT8_t;

/*! \typedef short INT16_t
 *	\brief signed short type
 */
typedef int16_t	INT16_t;

/*! \typedef unsigned short UINT16_t
 *	\brief unsigned short type
 */
typedef uint16_t	UINT16_t;

/*! \typedef int INT32_t
 *	\brief signed integer type
 */
typedef int32_t	INT32_t;

/*! \typedef unsigned int UINT32_t
 *	\brief unsigned integer type
 */
typedef uint32_t	UINT32_t;

/*! \typedef float FLOAT_t
 *	\brief float type
 */
typedef float	FLOAT_t;

/*! \typedef unsigned char	BYTE
 *	\brief unsigned char type
 *	\ This type MUST be 8-bit
 */
typedef unsigned char	BYTE;



#endif /* __TEST_TYPES_H_ */
