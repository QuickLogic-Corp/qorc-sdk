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

#ifndef __TINYTESTER_H_
#define __TINYTESTER_H_

#include <stdbool.h>

#include <FreeRTOS.h>
#include <queue.h>

#include "eoss3_dev.h"
#include "eoss3_hal_def.h"
#include "s3x_clock.h"


#define TINYTESTER_SIGNATURE        0xDEEB
#define TINYTESTER_REV_NUM          0x0100

typedef struct tinytester_regs {
    volatile uint32_t   signature;          // 0x00
    volatile uint32_t   revnumber;          // 0x04
    volatile uint16_t   scratch_reg;        // 0x08
    volatile uint16_t   reserved1;          // 0x0A
    volatile uint32_t   reserved2;          // 0x0C
    volatile uint32_t   control;            // 0x10
    volatile uint32_t   status;             // 0x14
    volatile uint32_t   dataout;            // 0x18
    volatile uint32_t   datain;             // 0x1C
    volatile uint32_t   oe;                 // 0x20
	volatile uint32_t   active_on_p0;       // 0x24
	volatile uint32_t   active_on_p1;       // 0x28
	volatile uint32_t   active_on_p2;       // 0x2C
	volatile uint32_t   active_on_p3;       // 0x30
} tinytester_regs_t;

enum tinytester_mode {output, input, compare};
typedef struct tinytester_channelconfig {
    enum tinytester_mode    mode;
    bool                    afDrive[4];     // When mode = output true => drive value during the phase, false => do not drive value during the phase
} tinytester_channelconfig_t;

// Main routine
void tinytester_run(uint32_t kchannel, tinytester_channelconfig_t* atinytester_channelconfig, uint32_t kvectorOutput, uint32_t* auxOutputvector, uint32_t kvectorInput, uint32_t* auxInputvector); 
void tinytester_init(void);

// Misc routines
uint32_t    tinytester_signatureis(void);

// Control routines
void        tinytester_control(uint32_t uxcontrol);
uint32_t    tinytester_controlis(void);
uint32_t    tinytester_statusis(void);
void        tinytester_oe(uint32_t uxoe);
uint32_t    tinytester_oeis(void);
void        tinytester_active_on_p0(uint32_t uxactive_on_p0);
uint32_t    tinytester_active_on_p0is(void);
void        tinytester_active_on_p1(uint32_t uxactive_on_p1);
uint32_t    tinytester_active_on_p1is(void);
void        tinytester_active_on_p2(uint32_t uxactive_on_p2);
uint32_t    tinytester_active_on_p2is(void);
void        tinytester_active_on_p3(uint32_t uxactive_on_p3);
uint32_t    tinytester_active_on_p3is(void);

// Data routines
void        tinytester_dataout(uint32_t uxdataout);
uint32_t    tinytester_dataoutis(void);
uint32_t    tinytester_datainis(void);



#endif // __TINYTESTER_H_