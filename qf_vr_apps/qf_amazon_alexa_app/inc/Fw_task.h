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
 *    File   : Fw_task.h
 *    Purpose: 
 *                                                          
 *=========================================================*/


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FW_TASK_H
#define __FW_TASK_H

#if !defined( _EnD_Of_Fw_global_config_h )
#error "Include Fw_global_config.h first"
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
//#include <stdio.h>
//#include <qlsh_commands.h>

/* Exported types ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

/* FIXME: Get rid of the uint8 and use uint8_t instead */
typedef uint8_t   uint8;
typedef int8_t     sint8;
typedef uint16_t uint16;
typedef int16_t          sint16;
typedef uint32_t   uint32;
typedef int32_t       sint32;

/* Priority for each tasks ---------------------------------------*/
/* Higher values indicate higher priorities. */




#define FFE_TASK_STACK_SIZE	( configMINIMAL_STACK_SIZE )
#define SYS_TASK_STACK_SIZE DO NOT USE THIS



#endif /* __TASK_H */


