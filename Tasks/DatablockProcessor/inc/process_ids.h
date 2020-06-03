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
 *    File   : process_ids.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __PROCESS_IDS__
#define __PROCESS_IDS__

typedef enum en_process_id {
  AUDIO_ISR_PID = 0x1,    // zero PID is not used 
#if 1 //Note: These will be removed and made specific before release
  AUDIO_QL_PRE_PROC_PID, // common id for ns precessing - DSPC, 2MC, CONS and AEC
  AUDIO_QL_VR_PID,    // common for both rdsp and others 
  AUDIO_QL_NULL_PID,  // dummy PID for unconnected output pins  
#endif
  AUDIO_CIRCULAR_BUFFER_PID, 
  
  AUDIO_NS_DSPC_PID, 
  AUDIO_NS_CONSILIENT_PID, 
  
  AUDIO_VR_SENSORY_PID,
  AUDIO_VR_NUANCE_PID, 
  AUDIO_VR_RDSP_PID,   

  AUDIO_AEC_CONSILIENT_PID, 

  AUDIO_OPUS_PID,
  AUDIO_D2H_PID,
  
  AUDIO_QL_I2STX_PID,

  AUDIO_SENSIML_AI_PID,
  IMU_ISR_PID,
  IMU_SENSIML_AI_PID,
} process_id_t ;

#endif /* __PROCESS_IDS__ */
