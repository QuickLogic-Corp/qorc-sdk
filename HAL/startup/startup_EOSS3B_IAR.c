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
 *    File   : startup_EOSS3B_IAR.c
 *    Purpose: This file contains code for startup/initialization ,reset handler
 *
*/


extern void __iar_program_start( void );
extern void SystemInit(void);
void Reset_Handler(void);

void Reset_Handler(void)
{
    SystemInit();
	__iar_program_start();
}
#pragma weak __semihost_call
int __semihost_call( int r0, int r1 )
{
    asm("bkpt 0xab");
    return 1;                       // SJ :check this
}
