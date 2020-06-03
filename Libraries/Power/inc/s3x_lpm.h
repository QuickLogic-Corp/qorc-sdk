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
 *-  Copyright Notice  -------------------------------------
 *                                                          
 *    Licensed Materials - Property of QuickLogic Corp.     
 *    Copyright (C) 2019 QuickLogic Corporation             
 *    All rights reserved                                   
 *    Use, duplication, or disclosure restricted            
 *                                                          
 *    File   : s3x_lpm.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __S3X_LPM_H
#define __S3X_LPM_H


/* Compiler includes. */
//#include <intrinsics.h>
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include <test_types.h>
#include "s3x_clock_hal.h"
#include "s3x_clock.h"
//#include "s3x_lpm.h"
#include "s3x_dfs.h"
#include "portmacro.h"

/* need to be cycle count */
#define portMISSED_COUNTS_FACTOR            ( 45UL )

#define configSYSTICK_CLOCK_HZ 32768

typedef enum {
    ENTER_LPM,
    EXIT_LPM,
} LPM_MODES;

void S3x_lpm_handler( TickType_t xExpectedIdleTime );
void xPortSysTickHandler( void );

typedef int (*Dev_Lpm_Cb)(int state);


typedef struct {
    Dev_Lpm_Cb pm_cb;
    char name[10];
    void *next;
} lpm_node;

static void  S3x_add_lpm_cb_node (lpm_node *node);
void S3x_Register_Lpm_Cb(Dev_Lpm_Cb lpm_cb, char *name);
#endif /* __S3X_S3X_LPM_H  */