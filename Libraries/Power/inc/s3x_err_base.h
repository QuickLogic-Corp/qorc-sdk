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
 *    File   : s3x_err_base.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __S3X_ERR_BASE_H
#define __S3X_ERR_BASE_H



#define CLKD_ERR_BASE      10
#define PI_ERR_BASE      11

#define STATUS_OK 0
#define EINVALID_VAL ((CLKD_ERR_BASE << 16) | 1)
#define ENOT_ENABLE ((CLKD_ERR_BASE << 16) | 2)
#define EFIX_RATE_CLK ((CLKD_ERR_BASE << 16) | 3)
#define EALREADY_AT_MAX ((CLKD_ERR_BASE << 16) | 4)
#define ERATE_NOT_VALID_WITH_DFS ((CLKD_ERR_BASE << 16) | 5)
#define EINVALID_RATE ((CLKD_ERR_BASE << 16) | 6)
#define ESD_RATE_HIGHER ((CLKD_ERR_BASE << 16) | 7)
#define EINVALID_PTR ((CLKD_ERR_BASE << 16) | 8)
#define ENO_MEM ((CLKD_ERR_BASE << 16) | 9)
#define EHSOSC_LOCK_FAIL ((CLKD_ERR_BASE << 16) | 10)

#define EPI_INUSE ((PI_ERR_BASE << 16) | 1)
#define EPI_SET_STATE ((PI_ERR_BASE << 16) | 2)

#endif/* __S3X_ERR_BASE_H  */


