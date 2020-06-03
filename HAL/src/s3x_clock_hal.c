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
 *    File   : s3x_clock_hal.c
 *    Purpose: This file contains source related to Reset and
 *             Clock configuration.
 *
 *  This module provides APIs for enabling/disabling clock
 *  and reset configurations. It also provides to configure
 *  clock gating for each clock domain tree.
 *
 *  This version of APIs currently allow to set default clock
 *  configuration or disable configuration options. The APIs
 *  will later support full configuration support in next version.
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

#include "FreeRTOS.h"
#include "eoss3_hal_def.h"
#include "s3x_clock_hal.h"
#include "s3x_clock.h"

int S3x_Clk_Enable(UINT32_t clk_id)
{
    return _S3x_Clk_Enable(clk_id);
}

int S3x_Clk_Disable(UINT32_t clk_id)
{
    return _S3x_Clk_Disable(clk_id);
}

int S3x_Clk_Set_Rate(UINT32_t clk_id, UINT32_t rate)
{
    return _S3x_Clk_Set_Rate(clk_id, rate);
}

int S3x_Clk_Get_Rate(UINT32_t clk_id)
{
    return _S3x_Clk_Get_Rate(clk_id);
}

int S3x_Clk_Get_Status(UINT32_t clk_id)
{
    return _S3x_Clk_Get_Status(clk_id);
}

int S3x_Clk_Get_Usecnt(UINT32_t clk_id)
{
    return _S3x_Clk_Get_Usecnt(clk_id);
}

int S3x_Register_Qos_Node(UINT32_t clk_id)
{
  return _S3x_register_qos_node(clk_id);
}

int S3x_Set_Qos_Req(UINT32_t clk_id, QOS_REQ_TYPE req, UINT32_t val)
{
  return _S3x_set_qos_req(clk_id, req, val);
}

int S3x_Get_Qos_Req(UINT32_t clk_id, QOS_REQ_TYPE req)
{
  return _S3x_get_qos_req(clk_id, req);
}

int S3x_Clear_Qos_Req(UINT32_t clk_id, QOS_REQ_TYPE req)
{
  return _S3x_set_qos_req(clk_id, req, 0);
}
