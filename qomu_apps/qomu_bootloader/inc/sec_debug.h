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
 *    File   : sec_debug.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef _SEC_DEBUG_H_
#define _SEC_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */




// variables
extern unsigned int fault_depth;



// functions
void save_assert_info(char* file, int line);
void invoke_soft_fault();
void check_debug_uart_msgs(char* str);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* _SEC_DEBUG_H_ */



