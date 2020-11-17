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
 *    File   : ql_interpBy3.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#include <stdio.h>
#include "ql_interpBy3.h"

#if ENABLE_16K_TO_48K_INTERPOLATOR
Interpolate_By_3_t Interpolate_State;

/*
* This initializes the Interpolation filter structure and returns the block size.
* The input and output Block sizes can be accessed any time usig Interpolate_State
*/
int init_interpolate_by_3(void)
{
  Interpolate_By_3_t *intp = &Interpolate_State;
  arm_fir_instance_q15 *fs =  &Interpolate_State.Lpf_State;
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
  fs = &Interpolate_State.Intp_State1;
  a_status |= arm_fir_init_q15(fs, FIR_LENGTH_STAGE2_INTERP, &stage2InterpCoef2[0], &intp->intp_data1[0], FIR_BLK_SIZE_INTERPOLATOR);

/*  
  fs->numTaps = FIR_LENGTH_STAGE2_INTERP;
  fs->pCoeffs = &stage2InterpCoef3[0];
  fs->pState  = &is->intp_data2[0];
*/
  fs = &Interpolate_State.Intp_State2;
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
int convert_16_to_48(int16_t *input_16K, int16_t *output_48K)
{
  Interpolate_By_3_t *intp = &Interpolate_State;
  arm_fir_instance_q15 *fs =  &Interpolate_State.Lpf_State;

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
      fs = &Interpolate_State.Lpf_State; //lpf filter
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

      fs = &Interpolate_State.Intp_State1; //interp 2 filter
      //Next do Intperp2 for internal block size
      arm_fir_fast_q15(fs, &intp->lpf_output[FIR_STAGE2_DELAY], &intp->intrp_output[0], FIR_BLK_SIZE_INTERPOLATOR);
      
      //copy the output after the first filter
      for(i=0;i<FIR_BLK_SIZE_INTERPOLATOR;i++)
        output_48K[i*3+2 + blk_offset] = intp->intrp_output[i];
      
      fs = &Interpolate_State.Intp_State2; //interp 3 filter
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

#if 0 // enable for testing the filter. disable after testing

short test_input_buffer[FIR_BLK_SIZE_16K_TO_48K];

void get_impulse_input(void)
{
  static int impulse = 0x7fff; //0x7fff;
  
  int i;
  for(i=0;i<FIR_BLK_SIZE_16K_TO_48K;i++)
    test_input_buffer[i] = 0;
  
  test_input_buffer[0] = impulse;
  impulse = 0;
}

#define PI_VALUE       (3.14159265358979f)
#define TWOPI_VALUE    (2*3.14159265358979f)
void get_1KHz_input(void)
{
  static float f = TWOPI_VALUE*1000/16000.0f;
  static float acc = 0;
  int i;
  float s;
  for(i=0;i<FIR_BLK_SIZE_16K_TO_48K;i++)
  {
    s = sin(acc)*32767/2; //6db down
    test_input_buffer[i] = (short)s;
    acc += f;
    if(acc >= (TWOPI_VALUE))
      acc -= (TWOPI_VALUE);
  }
}

void get_multitone_input(void)
{

  static float f1 = TWOPI_VALUE*1000/16000.0f;
  static float f2 = TWOPI_VALUE*3000/16000.0f;
  static float f3 = TWOPI_VALUE*5000/16000.0f;
  static float f4 = TWOPI_VALUE*7000/16000.0f;
  
  static float f5 = TWOPI_VALUE*7200/16000.0f;
  static float f6 = TWOPI_VALUE*7400/16000.0f;
  static float f7 = TWOPI_VALUE*7600/16000.0f;
  static float f8 = TWOPI_VALUE*7800/16000.0f;
  
  static float acc1 = 0;
  static float acc2 = 0;
  static float acc3 = 0;
  static float acc4 = 0;
  static float acc5 = 0;
  static float acc6 = 0;
  static float acc7 = 0;
  static float acc8 = 0;
  
  int i;
  float s;
      for(i=0;i<FIR_BLK_SIZE_16K_TO_48K;i++)
      {
        s = sin(acc1) + sin(acc2) + sin(acc3) + sin(acc4) ;
        s += sin(acc5) + sin(acc6) + sin(acc7) + sin(acc8) ;
        s = (s*32767.0f/8);
        s = (s/2); //6db down
        
        test_input_buffer[i] = (short)s;
        acc1 += f1;acc2 += f2;acc3 += f3;acc4 += f4;
        acc5 += f5;acc6 += f6;acc7 += f7;acc8 += f8;
        if(acc1 >= (TWOPI_VALUE))
          acc1 -= (TWOPI_VALUE);
        if(acc2 >= (TWOPI_VALUE))
          acc2 -= (TWOPI_VALUE);
        if(acc3 >= (TWOPI_VALUE))
          acc3 -= (TWOPI_VALUE);
        if(acc4 >= (TWOPI_VALUE))
          acc4 -= (TWOPI_VALUE);
        if(acc5 >= (TWOPI_VALUE))
          acc5 -= (TWOPI_VALUE);
        if(acc6 >= (TWOPI_VALUE))
          acc6 -= (TWOPI_VALUE);
        if(acc7 >= (TWOPI_VALUE))
          acc7 -= (TWOPI_VALUE);
        if(acc8 >= (TWOPI_VALUE))
          acc8 -= (TWOPI_VALUE);
        
      }

}

//1.Test impulse response
//2.Test 1000Hz sinusiod response
//3.Multitone respose
void test_interpolator(void)
{
  int count1 =0;
  int i=0;
  int max_count = (DMA_NUMBER_OF_BUFFERS*DMA_SINGLE_BUFFER_SIZE); ///(3*FIR_BLK_SIZE_16K_TO_48K);

#if 1
   //first input generated by sin functions
  for(i=0;i < (max_count); )
  {
    get_1KHz_input();
    short *ptr = &gDmaBuffer.mem[0][0] + i;
    for(int k=0; k < FIR_BLK_SIZE_16K_TO_48K; k++)
    {
      *(ptr + k) =  test_input_buffer[k];
      
     //*(ptr + k*3) =  test_input_buffer[k];
     //*(ptr + k*3 + 1) =  0;
     //*(ptr + k*3 + 2) =  0;
    }
    i += (1*FIR_BLK_SIZE_16K_TO_48K);
    //i += (3*FIR_BLK_SIZE_16K_TO_48K);
  }
#endif
    
  //get the impulse response
  for(i=0;i < (4*(3*FIR_BLK_SIZE_16K_TO_48K)); )
  {
    get_impulse_input();
    convert_16_to_48(test_input_buffer, (&gDmaBuffer.mem[0][0] + i));  
    i += (3*FIR_BLK_SIZE_16K_TO_48K);
  }

#if 1
  //get interpolated 1KHz wave
  for(;i < (max_count/2); )
  {
    get_1KHz_input();
    convert_16_to_48(test_input_buffer, (&gDmaBuffer.mem[0][0] + i));  
    i += (3*FIR_BLK_SIZE_16K_TO_48K);
  }
#endif
#if 1  
  //check the suppression of band edge signal after 7KHz
  for(;i < (max_count); )
  {

    get_multitone_input();
    convert_16_to_48(test_input_buffer, (&gDmaBuffer.mem[0][0] + i));  
    i += (3*FIR_BLK_SIZE_16K_TO_48K);
  }
#endif  
  while(1);
}

#endif // enable for testing the filter

#endif //ENABLE_16K_TO_48K_INTERPOLATOR
