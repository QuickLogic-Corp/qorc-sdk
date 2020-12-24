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
 *    File   : ql_interpBy3_stereo.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#if 0//this will be defined in Fw_globalconfig.h
#define FIR_BLK_SIZE_16K_TO_48K  (15*16/2)  //Filter block size for input. Output is always 3 times this
//Use this to enable stereo Interpolator for 16KHz to 48KHz conversion
#define ENABLE_STEREO_16K_TO_48K_INTERPOLATOR 1 //1 = stereo, 0 = mono = 1 (left) channel
#endif


#include "Fw_global_config.h"

#ifndef QL_INTERPBY3_STEREO_H_
#define QL_INTERPBY3_STEREO_H_

#include <arm_math.h>


#define FIR_BLK_SIZE_INTERPOLATOR (FIR_BLK_SIZE_16K_TO_48K/3) //=(5*16) = 5ms to collect the data for each filter output.
#define FIR_LENGTH_STAGE1_LPF     (77+1) //Used to cut-off input above ~7KHz
#define FIR_LENGTH_STAGE2_INTERP  48 //Used to remove images due to interpoaltion
#define FIR_STAGE2_DELAY          (0+(FIR_LENGTH_STAGE2_INTERP/2)) //first filter is implemented using a delay

//Structure used by the Interpolation filter for 16K to 48K conversion
typedef struct {
    //DSP Library struct for fast FIR
    arm_fir_instance_q15 Lpf_State;
    arm_fir_instance_q15 Intp_State1;
    arm_fir_instance_q15 Intp_State2;
    
    //Block Data for Filters
    short lpf_data[FIR_LENGTH_STAGE1_LPF + FIR_BLK_SIZE_INTERPOLATOR];
    short intp_data1[FIR_LENGTH_STAGE2_INTERP + FIR_BLK_SIZE_INTERPOLATOR];
    short intp_data2[FIR_LENGTH_STAGE2_INTERP + FIR_BLK_SIZE_INTERPOLATOR];
    
    //for temporary data storage from filter outputs
    short lpf_output[FIR_BLK_SIZE_INTERPOLATOR + FIR_STAGE2_DELAY]; //input to the interpolator filters
    short intrp_output[FIR_BLK_SIZE_INTERPOLATOR];
    
    int input_blk_size;  //input block size at 16kK
    int output_blk_size; //ouput block size at 48K = 3 times of input_blk_size
    int internal_blks;   //should be an integer always = (FIR_BLK_SIZE_16K_TO_48K/FIR_BLK_SIZE_INTERPOLATOR)
  
} Interpolate_By_3_t;
//structs - only 2 predefined
extern Interpolate_By_3_t Interpolate_State_Left;

#if ENABLE_STEREO_16K_TO_48K_INTERPOLATOR == 1

extern Interpolate_By_3_t Interpolate_State_Righ;

#endif

//init function per channel
extern int init_interpolate_by_3_left(void);
//runtime function per channel
extern int convert_16_to_48_left(int16_t *input_16K_left, int16_t *output_48K_left);

#if ENABLE_STEREO_16K_TO_48K_INTERPOLATOR == 1

//init function for stereo
extern int init_interpolate_by_3_stereo(void);
//runtime function for stereo
extern int convert_16_to_48_stereo(int16_t *input_16K_left, int16_t *output_48K_left, int16_t *input_16K_right, int16_t *output_48K_right);

#endif

#endif  //QL_INTERPBY3_STEREO_H_
