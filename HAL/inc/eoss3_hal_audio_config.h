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
* eoss3_hal_audio.h
*
*/

#ifndef __EOSS3_HAL_AUDIO_CONFIG_H__
#define __EOSS3_HAL_AUDIO_CONFIG_H__

#include "eoss3_hal_audio.h"

#define QL_LPSD_THD_MASK 0xFFFF
#define QL_LPSD_RATIO_STOP_MASK 0x00FF0000
#define QL_LPSD_RATIO_RUN_MASK 0xFF000000

enum QL_LPSD_CONFIG {
	QL_LPSD_THD = 0,
	QL_LPSD_RATIO_STOP = 16,
        QL_LPSD_RATIO_RUN = 24,
};

typedef struct {
  char run_start;
  char run_stop;
  short thr;
}t_lpsd_mode_values;

#define MAX_LPSD_MODES 10
#define LPSD_DEFAULT_MODE 8


void taskENTER_CRITICAL_todo();
void taskEXIT_CRITICAL_todo();

#define QL_AUDIO_HAL_ENTER_CRITICAL_SECTION(id) //taskENTER_CRITICAL()
#define QL_AUDIO_HAL_EXIT_CRITICAL_SECTION(id)  //taskEXIT_CRITICAL()

typedef enum voice_config_interrupts_e
{
  e_voice_config_interrupts_unmask_all = 0,
  e_voice_config_interrupts_mask_all = 1
}voice_config_interrupts_e;

#define SRAM_ADDR_TO_DMA_ADDR(addr) ( SRAM_BASE | addr )
#define AUDIO_SRAM_HW_DS_CFG (1<<8)
#define DMAC_BLK_LEN   0
#define DMAC_BUF_LEN   16

t_lpsd_mode_values a_olpsd_mode_values[MAX_LPSD_MODES] = {
  {
    .run_start = 58,
    .run_stop = 66,
    .thr = 583,
  },  //mode0
  {
    .run_start = 64,
    .run_stop = 73,
    .thr = 700
  }, //mode1
  {
    .run_start = 70,
    .run_stop = 80,
    .thr = 830
  }, //mode2
  
  {
    .run_start = 77,
    .run_stop = 88,
    .thr = 1000,
  }, //mode3
  {
    .run_start = 85,
    .run_stop = 97,
    .thr = 1200
  }, //mode4
  {
    .run_start = 94,
    .run_stop = 107,
    .thr = 1440
  }, //mode5
  {
    .run_start = 103,
    .run_stop = 118,
    .thr = 1728
  }, //mode6
  {
    .run_start = 113,
    .run_stop = 130,
    .thr = 2074
  }, //mode7
  
  {
    .run_start = 77,
    .run_stop = 80,
    .thr = 1200
  }, //mode8 default mode.
  
  {
    .run_start = 77,
    .run_stop = 88,
    .thr = 1200
  }, //mode9 Sensory recommended mode.
  
};


#endif /* __EOSS3_HAL_AUDIO_CONFIG_H__ */

