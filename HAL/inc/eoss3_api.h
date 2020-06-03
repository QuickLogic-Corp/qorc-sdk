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
 *    File   : eoss3_api.h
 *    Purpose: This header file contains the descriptions 
 *             and definitions for the application level APIs 
 *             for the various BSP subsystem firmware
 *                                                          
 *=========================================================*/

#ifndef _eoss3_api_h
#define _eoss3_api_h
/*
 * This header file contains the descriptions and definitions for the
 * application level APIs for the various BSP subsystem firmware.
 *
 * Copyright (c)2016 QuickLogic Corp., ALL RIGHTS RESERVED
 *
 * rfa - 160125
 */
#include <stdint.h>
#include <stddef.h>


/*
 * FLASH API
 */
int flash_pwrmode(int);
// int flash_init(...);

#endif /* !_eoss3_api_h */
