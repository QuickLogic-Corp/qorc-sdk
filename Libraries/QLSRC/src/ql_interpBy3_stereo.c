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
 *    File   : ql_interpBy3_stereo.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

#include <stdio.h>
#include "ql_interpBy3_stereo.h"


//a1 coef: LPF filter
static short stage1LpfCoef[77+1] = {
 -4,   8, -11,  13, -13,   9,   0, -13,  30, -48,      //0-9 
  62, -68,  62, -40,   0,  54,-116, 176,-220, 233,     //10-9 
-205, 127,   0,-165, 349,-520, 643,-681, 599,-375,     //20-9 
   0, 515,-1141,1830,-2523,3153,-3658,3983,28672,3983, //30-9 
-3658,3153,-2523,1830,-1141, 515,   0,-375, 599,-681,  //40-9 
 643,-520, 349,-165,   0, 127,-205, 233,-220, 176,     //50-9 
-116,  54,   0, -40,  62, -68,  62, -48,  30, -13,     //60-9 
   0,   9, -13,  13, -11,   8,  -4, 0                  //70-76 + 1 to make even for CMSIS lib
};

#if 0
/* This is implemented simply as delay 
//b1(1:3:end)
short stage2InterpCoef1[48 +1] = {
0,0,0,0,0,0,0,0,0,0,0,0, //0-11
0,0,0,0,0,0,0,0,0,0,0,0, //12-23
10923,  //24 th is center
0,0,0,0,0,0,0,0,0,0,0,0, //25-36
0,0,0,0,0,0,0,0,0,0,0,0 //37 - 48
};
*/
static short stage2InterpCoef1 = 10923;  //24 th is center 
//b1(2:3:end) : polyphase interpolator
static short stage2InterpCoef2[48] = {
  -1,   2,  -3,   5,  -8,  12, -17,  24, -34,  45, -61,  80,  //0-11
-103, 132,-168, 212,-268, 338,-432, 561,-753,1079,-1775,4504, //12-23
9026,-2233,1246,-841, 616,-470, 367,-289, 229,-181, 143,-112, //24-35
  87, -66,  50, -37,  27, -19,  13,  -9,   6,  -3,   2,  -1,  //36-47
};
//b1(3:3:end) : polyphase interpolator
static short stage2InterpCoef3[48] = {          
  -1,   2,  -3,   6,  -9,  13, -19,  27, -37,  50, -66,  87,  //0-11
-112, 143,-181, 229,-289, 367,-470, 616,-841,1246,-2233,9026, //12-23
4504,-1775,1079,-753, 561,-432, 338,-268, 212,-168, 132,-103, //24-35
  80, -61,  45, -34,  24, -17,  12,  -8,   5,  -3,   2,  -1,  //36-47
};

#endif
//These filter coeficients are 3 times but it should not overflow
/* This is implemented simply as delay 
//b1(1:3:end)
short stage2InterpCoef1[48 +1] = {
0,0,0,0,0,0,0,0,0,0,0,0, //0-11
0,0,0,0,0,0,0,0,0,0,0,0, //12-23
32768, //24 th is center
0,0,0,0,0,0,0,0,0,0,0,0, //25-36
0,0,0,0,0,0,0,0,0,0,0,0, //37 - 48
};
*/
//b1(2:3:end) : polyphase interpolator
static short stage2InterpCoef2[48] = {
-2,5,-8,14,-23,35,-51,73,-101,136,-182,239,                      //0-11
-309,396,-503,636,-803,1015,-1295,1682,-2260,3237,-5324,13511,   //12-23
27079,-6698,3739,-2523,1847,-1410,1100,-867,687,-544,429,-336,   //24-35
260,-199,150,-112,81,-58,40,-26,17,-10,6,-3,                       
};
//b1(3:3:end) : polyphase interpolator
static short stage2InterpCoef3[48] = {          
-3,6,-10,17,-26,40,-58,81,-112,150,-199,260,                     //0-11
-336,429,-544,687,-867,1100,-1410,1847,-2523,3739,-6698,27079,   //12-23
13511,-5324,3237,-2260,1682,-1295,1015,-803,636,-503,396,-309,   //24-35
239,-182,136,-101,73,-51,35,-23,14,-8,5,-2,                      //36-47
};


Interpolate_By_3_t Interpolate_State_Left;
Interpolate_By_3_t Interpolate_State_Right;

/*
* This initializes the Interpolation filter structure for one channel and returns the block size.
* The input and output Block sizes can be accessed any time usig Interpolate_State
*/
int init_interpolate_by_3_channel(Interpolate_By_3_t *channel_state)
{
  Interpolate_By_3_t *intp = channel_state;
  arm_fir_instance_q15 *fs =  &intp->Lpf_State;
  int a_status;
  
  //Init Filter State for DSP Libary
/*  
  fs->numTaps = FIR_LENGTH_STAGE1_LPF;
  fs->pCoeffs = &stage1LpfCoef[0];
  fs->pState  = &is->lpf_data[0];
*/
  //Function prototype:
  //arm_fir_init_q15( arm_fir_instance_q15 *S, uint16_t numTaps, q15_t *pCoeffs, q15_t *pState,uint32_t blockSize) 	
  a_status = arm_fir_init_q15(fs, FIR_LENGTH_STAGE1_LPF, &stage1LpfCoef[0], &intp->lpf_data[0], FIR_BLK_SIZE_INTERPOLATOR);

/*  
  fs->numTaps = FIR_LENGTH_STAGE2_INTERP;
  fs->pCoeffs = &stage2InterpCoef2[0];
  fs->pState  = &is->intp_data1[0];
*/
  fs = &intp->Intp_State1;
  a_status |= arm_fir_init_q15(fs, FIR_LENGTH_STAGE2_INTERP, &stage2InterpCoef2[0], &intp->intp_data1[0], FIR_BLK_SIZE_INTERPOLATOR);

/*  
  fs->numTaps = FIR_LENGTH_STAGE2_INTERP;
  fs->pCoeffs = &stage2InterpCoef3[0];
  fs->pState  = &is->intp_data2[0];
*/
  fs = &intp->Intp_State2;
  a_status |= arm_fir_init_q15(fs, FIR_LENGTH_STAGE2_INTERP, &stage2InterpCoef3[0], &intp->intp_data2[0], FIR_BLK_SIZE_INTERPOLATOR);
  
  if(a_status != ARM_MATH_SUCCESS)
  {
    printf("ERROR - Incorrect Interpolator Filter configuration. \n");
    return 0;
  }

  //make sure input blk size and internal block sizes are exact multiples
  intp->internal_blks = (FIR_BLK_SIZE_16K_TO_48K/FIR_BLK_SIZE_INTERPOLATOR);
  if((intp->internal_blks*FIR_BLK_SIZE_INTERPOLATOR) != FIR_BLK_SIZE_16K_TO_48K)
  {
    intp->internal_blks = 0; //prevent filtering
    printf("ERROR - Incorrect Filter Block configuration. \n");
    return 0;
  }

  intp->input_blk_size  = FIR_BLK_SIZE_16K_TO_48K;
  intp->output_blk_size = (FIR_BLK_SIZE_16K_TO_48K*3);

  return intp->output_blk_size;
}

/*
* This converts a block of 16K rate samples into 48K rate samples.
* The block is predefined in the InterpBy3.h  and is accessible
* during run time in Interpolate_State.input_blk_size.
* The outblock size is 3 times input block size always.
*/
int convert_16_to_48_channel(int16_t *input_16K, int16_t *output_48K, Interpolate_By_3_t *channel_state )
{
  Interpolate_By_3_t *intp = channel_state;
  arm_fir_instance_q15 *fs =  &intp->Lpf_State;

  //The following logic will not work unless this condition is met
  if(!intp->internal_blks)
    return 0;
  
  //Function prototype:
  //arm_fir_fast_q15 (const arm_fir_instance_q15 *S, q15_t *pSrc, q15_t *pDst, uint32_t blockSize)

  int blk_count;
  int input_offset = 0;
  int blk_offset = 0;
  int i;
  for(blk_count = 0; blk_count < intp->internal_blks; blk_count++)
  {
      fs = &intp->Lpf_State; //lpf filter
      //First do LPF for internal block size
      arm_fir_fast_q15(fs, (input_16K+input_offset), &intp->lpf_output[FIR_STAGE2_DELAY], FIR_BLK_SIZE_INTERPOLATOR);
#if 0       //test lpf  only   
//copy the output directly since first filter is implemented as a delay
for(i=0;i<FIR_BLK_SIZE_INTERPOLATOR;i++)
{
    output_48K[i*1 + blk_offset] = intp->lpf_output[i];
    //also move the delay for first interp filter
    intp->lpf_output[i] = intp->lpf_output[i+FIR_BLK_SIZE_INTERPOLATOR];
}
     
blk_offset += (1*FIR_BLK_SIZE_INTERPOLATOR);
#endif
#if 0       //test interp only
//copy just an impulse in the lpf output
for(i=0;i<(FIR_BLK_SIZE_INTERPOLATOR+FIR_STAGE2_DELAY);i++)
{
    intp->lpf_output[i] = 0;
}
intp->lpf_output[FIR_STAGE2_DELAY] = 0x7FFF;

#endif

#if 1   
       //long k;
      //copy the output directly since first filter is implemented as a delay
      for(i=0;i<FIR_BLK_SIZE_INTERPOLATOR;i++)
      {
        //k = (long)intp->lpf_output[i] * stage2InterpCoef1 + 0x04000;
        //output_48K[i*3+0 + blk_offset] = (short) (k >> 15); //intp->lpf_output[i];
        output_48K[i*3+0 + blk_offset] = intp->lpf_output[i];
      }

      fs = &intp->Intp_State1; //interp 2 filter
      //Next do Intperp2 for internal block size
      arm_fir_fast_q15(fs, &intp->lpf_output[FIR_STAGE2_DELAY], &intp->intrp_output[0], FIR_BLK_SIZE_INTERPOLATOR);
      
      //copy the output after the first filter
      for(i=0;i<FIR_BLK_SIZE_INTERPOLATOR;i++)
        output_48K[i*3+2 + blk_offset] = intp->intrp_output[i];
      
      fs = &intp->Intp_State2; //interp 3 filter
      //Next do Intperp3 for internal block size 
      arm_fir_fast_q15(fs, &intp->lpf_output[FIR_STAGE2_DELAY], &intp->intrp_output[0], FIR_BLK_SIZE_INTERPOLATOR);
      
      //copy the output after the second filter
      for(i=0;i<FIR_BLK_SIZE_INTERPOLATOR;i++)
        output_48K[i*3+1 + blk_offset] = intp->intrp_output[i];

      //also move the delay for first interp filter
      for(i=0;i<FIR_STAGE2_DELAY;i++)
      {
        intp->lpf_output[i] = intp->lpf_output[i+FIR_BLK_SIZE_INTERPOLATOR];
      }

      blk_offset += (3*FIR_BLK_SIZE_INTERPOLATOR);
      input_offset += FIR_BLK_SIZE_INTERPOLATOR;
#endif
  }
  
  return 1;
}

/*
* This initializes the Interpolation filter structure for Left and Right hannel 
* and returns the block size.
* The input and output Block sizes can be accessed any time usig Interpolate_State
*/
int init_interpolate_by_3_stereo(void)
{
  //init left channel
  init_interpolate_by_3_channel(&Interpolate_State_Left);
  
  //init right channel
  init_interpolate_by_3_channel(&Interpolate_State_Right);
  
  return Interpolate_State_Left.output_blk_size;
}

/*
* This converts a stereo block of 16K rate samples into 48K rate samples.
* The left and right channels are passed as different arrays.
* This is because of the Block FIR filter routines used
* The block size is predefined and is accessible
* during run time in Interpolate_State.input_blk_size.
* The outblock size is 3 times input block size always.
*/
int convert_16_to_48_stereo(int16_t *input_16K_left, int16_t *output_48K_left, int16_t *input_16K_right, int16_t *output_48K_right)
{
  int result1, result2;
  //convert left channel from 16K to 48K
  result1 = convert_16_to_48_channel(input_16K_left, output_48K_left, &Interpolate_State_Left);

  //convert right channel from 16K to 48K
  result2 = convert_16_to_48_channel(input_16K_right, output_48K_right, &Interpolate_State_Right);

  return result1 & result2;
}
/*
* This initializes the Interpolation filter structure for Left channel 
* and returns the block size.
* The input and output Block sizes can be accessed any time usig Interpolate_State
*/
int init_interpolate_by_3_left(void)
{
  //init left channel
  init_interpolate_by_3_channel(&Interpolate_State_Left);
  return Interpolate_State_Left.output_blk_size;
}
/*
* This converts left channel of stereo 16K rate samples into 48K rate samples.
* The outblock size is 3 times input block size always.
*/
int convert_16_to_48_left(int16_t *input_16K_left, int16_t *output_48K_left)
{
  int result;
  //convert left channel from 16K to 48K
  result = convert_16_to_48_channel(input_16K_left, output_48K_left, &Interpolate_State_Left);
  
  return result;
}

