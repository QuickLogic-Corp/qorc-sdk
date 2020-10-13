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
 *    File   : RtosTask.h
 *    Purpose: Define the Task handles, queues and priorities
 *             
 *
 *=========================================================*/
/* Define task priorities for bl1 */

#include "FreeRTOS.h"

/* Enum so that all of these are visibile in 1 place */
typedef enum {
    PRIORITY_LOWEST = 0,
    
    PRIORITY_LOWER  = (configMAX_PRIORITIES/4),
        
    PRIORITY_NORMAL = (configMAX_PRIORITIES/2),
    
    PRIORITY_HIGH  = ((configMAX_PRIORITIES*3)/4),
    
    PRIORITY_HAL_TIMER = configMAX_PRIORITIES-2,
    PRIORITY_LOADER = configMAX_PRIORITIES-1,
    
    PRIORITY_HIGHEST = configMAX_PRIORITIES,
} TaskPriorities;
