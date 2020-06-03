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

/*
 * s3x_cpuload.c
 *
 * (C) Copyrighted 2015 Quicklogic Inc.
 */

#include <stdio.h>
#include "Fw_global_config.h"
#include "FreeRTOS.h"
#include "eoss3_dev.h"
#include "s3x_cpuload.h"
#include "s3x_dfs.h"
#include "timers.h"

static volatile uint32_t ticksSleeptime;

uint16_t DFS_cpuload(void)
{  
    uint16_t    xcpuload;
    uint16_t    index = DFS_Get_Curr_Policy();
    uint32_t    ticksWaketime = dfs_node[index].step_width - ticksSleeptime; 

    xcpuload = (uint16_t)(100.0*((float)ticksWaketime)/((float)dfs_node[index].step_width));
    ticksSleeptime = 0;
    return xcpuload;
}

void DFS_updatesleepticks(uint32_t ticks)
{
    ticksSleeptime += ticks;
    return;
}

void DFS_resetsleepticks(void)
{
    ticksSleeptime = 0;
    return;
}