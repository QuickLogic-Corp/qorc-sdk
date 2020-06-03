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
 *    File   : hearable_pwrcfg.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifdef VOICE_OVER_QLUART
#ifndef __HEARABLE_PWRCFG_H
#define __HEARABLE_PWRCFG_H


/*Sensor Only*/
#ifndef ENABLE_VOICE_SOLUTION
#if (defined(DT_ALGO_ON_FFE) || defined(DT_ALGO_ON_M4_BATCH) || \
		defined(DT_ALGO_ON_M4_LIVE) || defined(PCG_AND_DT_ON_FFE) \
                 || defined(PCG_AND_DT_ON_M4_BATCH) || defined(PCG_ALGO_ON_M4_LIVE)  \
                 || defined(LIVE_SENSOR_DATA) )
#define FFE_MHZ HSOSC_12MHZ
#else
#define FFE_MHZ HSOSC_6MHZ
#endif

#if (defined (PCG_ALGO_ON_M4_LIVE) || defined (DT_ALGO_ON_M4_LIVE))
#define C01_N0_CLK  HSOSC_1MHZ
#define C09_N0_CLK  HSOSC_1MHZ
#define C10_N0_CLK  HSOSC_1MHZ
#define C8X4_N0_CLK FFE_MHZ
#else
#define C01_N0_CLK  HSOSC_1MHZ
#define C09_N0_CLK  HSOSC_1MHZ
#define C10_N0_CLK  HSOSC_1MHZ
#define C8X4_N0_CLK FFE_MHZ
#endif


#define C01_N1_CLK  HSOSC_1MHZ
#define C09_N1_CLK  HSOSC_1MHZ
#define C10_N1_CLK  HSOSC_12MHZ
#define C8X4_N1_CLK FFE_MHZ
#define STEP_1 500

#define C01_N2_CLK  HSOSC_2MHZ
#define C09_N2_CLK  HSOSC_2MHZ
#define C10_N2_CLK  HSOSC_24MHZ
#define C8X4_N2_CLK FFE_MHZ
#define STEP_2 500

#define C01_N3_CLK  HSOSC_3MHZ
#define C09_N3_CLK  HSOSC_3MHZ
#define C10_N3_CLK  HSOSC_48MHZ
#define C8X4_N3_CLK FFE_MHZ
#define STEP_3 500

#define C01_N4_CLK  HSOSC_6MHZ
#define C09_N4_CLK  HSOSC_6MHZ
#define C10_N4_CLK  HSOSC_72MHZ
#define C8X4_N4_CLK FFE_MHZ

#else
/* Only voice */
#ifdef ONLY_VOICE_SOLUTION
#define C01_N0_CLK  HSOSC_512KHZ
#define C09_N0_CLK  HSOSC_512KHZ
#define C10_N0_CLK  HSOSC_512KHZ
#define C8X4_N0_CLK HSOSC_1MHZ

#define C01_N1_CLK  HSOSC_3MHZ
#define C09_N1_CLK  HSOSC_1MHZ
#define C10_N1_CLK  HSOSC_18MHZ
#define C8X4_N1_CLK HSOSC_1MHZ
#define STEP_1 500

#define C01_N2_CLK  HSOSC_3MHZ
#define C09_N2_CLK  HSOSC_2MHZ
#define C10_N2_CLK  HSOSC_36MHZ
#define C8X4_N2_CLK HSOSC_1MHZ
#define STEP_2 500

#define C01_N3_CLK  HSOSC_3MHZ
#define C09_N3_CLK  HSOSC_3MHZ
#define C10_N3_CLK  HSOSC_48MHZ
#define C8X4_N3_CLK HSOSC_1MHZ
#define STEP_3 500

#define C01_N4_CLK  HSOSC_6MHZ
#define C09_N4_CLK  HSOSC_6MHZ
#define C10_N4_CLK  HSOSC_72MHZ
#define C8X4_N4_CLK HSOSC_1MHZ

/* sensor + voice */
#else

#define C01_N0_CLK  HSOSC_2MHZ
#define C09_N0_CLK  HSOSC_2MHZ
#define C10_N0_CLK  HSOSC_4MHZ
#define C8X4_N0_CLK HSOSC_12MHZ

#define C01_N1_CLK  HSOSC_2MHZ
#define C09_N1_CLK  HSOSC_2MHZ
#define C10_N1_CLK  HSOSC_24MHZ
#define C8X4_N1_CLK HSOSC_12MHZ
#define STEP_1 500

#define C01_N2_CLK  HSOSC_2MHZ
#define C09_N2_CLK  HSOSC_2MHZ
#define C10_N2_CLK  HSOSC_36MHZ
#define C8X4_N2_CLK HSOSC_12MHZ
#define STEP_2 500

#define C01_N3_CLK  HSOSC_4MHZ
#define C09_N3_CLK  HSOSC_4MHZ
#define C10_N3_CLK  HSOSC_48MHZ
#define C8X4_N3_CLK HSOSC_12MHZ
#define STEP_3 500

#define C01_N4_CLK  HSOSC_6MHZ
#define C09_N4_CLK  HSOSC_6MHZ
#define C10_N4_CLK  HSOSC_72MHZ
#define C8X4_N4_CLK HSOSC_12MHZ

#endif /* ONLY_VOICE_SOLUTION */
#endif /* ENABLE_VOICE_SOLUTION */

#define C01_IDX   0
#define C09_IDX   1
#define C10_IDX   2
#define C8X4_IDX  3

#define INIT_GATE_ON 1
#define INIT_GATE_OFF 0

#define CRU_CTRL(d, m, en, so, go, gm, sds) { .div_off = d,\
    .div_max = m, .div_en_shift = en, .src_sel_off = so,\
    .gate_off = go, .gate_mask = gm, .src_div_shift = sds,}


#define INIT_STATE(ir, im, men) { .irate = ir, .imask = im, .en = men }

#define SYNC_CLKD(c,id0, id1) { .sd_clk.cnt = c,\
    .sd_clk.sd_id[0] = id0, .sd_clk.sd_id[1] = id1, }

#define SRC_DOMAIN(c) { .sclk.src_domain = c, .sclk.src_rate = 0,}

#define PI_CTRL(so, co, to, wo, pm, tm, wm) { .st_off = so, .cfg_off = co,\
                .trig_off = to, .swu_off = wo, .pmask = pm, .trig_mask = tm,\
                .swu_mask = wm, }

#define PI_GINFO(c, i0, i1, i2, i3, i4) { .gcnt = c, .gid[0] = i0,\
                    .gid[1] = i1, .gid[2] = i2, .gid[3] = i3, .gid[4] = i4, }


#endif      /* __HEARABLE_PWRCFG_H  */
#endif /* VOICE_OVER_QLUART */