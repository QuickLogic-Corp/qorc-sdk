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

#ifndef QL_INTERPBY3_H_
#define QL_INTERPBY3_H_

//#error DO NOT USE THIS FOR QUICK AI
/* This file defines: "FRAME_SIZE"
 * The file: "dma_buffer.h" also defines FRAME_SIZE
 * There should be one and only one definition.
 */

#include <arm_math.h>

//Use this to enable or disable Interpolator for 16KHz to 48KHz conversion
#define ENABLE_16K_TO_48K_INTERPOLATOR 1

// Sampling Frequency
#define FS              (16000)

// Base Frame Size gives number of samples in 1 msec
#define BASE_FRAME_SIZE ((FS)/1000)  // 16 samples in 1ms

// Frame size in msec
#define FRAME_SIZE_MS   (15)   // 1 Frame duration = 15 ms

// Frame size gives number of samples in one frame
#define FRAME_SIZE      ((FRAME_SIZE_MS)*(BASE_FRAME_SIZE)) // 1 Frame size = 15*16 = 240 (samples = 480 bytes)

// DMA block size of single buffer size for BLOCKINT to be fired
#define DMA_SINGLE_BUFFER_SIZE (FRAME_SIZE)


#define FIR_BLK_SIZE_16K_TO_48K  (DMA_SINGLE_BUFFER_SIZE)  //Filter block size for input. Output is always 3 times this

#define FIR_BLK_SIZE_INTERPOLATOR (FIR_BLK_SIZE_16K_TO_48K/3) //=(5*16) = 5ms to collect the data for each filter output.
#define FIR_LENGTH_STAGE1_LPF     (77+1) //Used to cut-input above ~7KHz
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
    
    int input_blk_size; //input block size at 16kK
    int output_blk_size; //ouput block size at 48K = 3 times of input_blk_size
    int internal_blks; //should integer always = (FIR_BLK_SIZE_16K_TO_48K/FIR_BLK_SIZE_INTERPOLATOR)
  
} Interpolate_By_3_t;

//struct
extern Interpolate_By_3_t Interpolate_State;

//init function
extern int init_Interpolate_by_3(void);
//runtime function
extern int convert_16_to_48(short *input_16K, short *output_48K);


/*
  MATALB code used to generate the coefs below
% first cut-off the lowpass signal from 7KHz- 8KHz
% 6dB point at 7.0KHz. At least attenuation 73.5 dB in the stop band
M1 = 90;
a1 = fir1(M1, 0.875, kaiser(M1+1, 8.84));
a1 = round(a1*32768); 
a1 = a1(8:end-7); % remove the first and last 0 tap values

% Next: interpolator with center filter just a delay
% 6dB point at 8.KHz. At least 73.5 dB attenuation in stop band.
% This gives at least 74 db in the suppression of images when interpolated
M = 50*3;M2 = M+1;
b1 = fir1(M, 1/3, kaiser(M2, 8.5));
b1 = round(b1*32768); 
b1 = b1(4:end-3); % remove the first and last 0 tap values

% plot the combined response
a1f = 20*log10(abs(fft(a1, 16000)));
b1f = 20*log10(abs(fft(b1, 3*16000)));

a1f = a1f - max(a1f);
b1f = b1f - max(b1f);;

plot(a1f(6001:10000));hold;
plot(b1f(6001:16000));
plot(a1f(1:16000) + b1f(1:16000),'*');grid; hold;

% Max of 74.4 db attenuation for images
max(a1f(8001:16000) + b1f(8001:16000))
a1'
b1(1:3:end)'
b1(2:3:end)'
b1(3:3:end)'

*/

//a1 coef: LPF filter
short stage1LpfCoef[77+1] = {
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
short stage2InterpCoef1 = 10923;  //24 th is center 
//b1(2:3:end) : polyphase interpolator
short stage2InterpCoef2[48] = {
  -1,   2,  -3,   5,  -8,  12, -17,  24, -34,  45, -61,  80,  //0-11
-103, 132,-168, 212,-268, 338,-432, 561,-753,1079,-1775,4504, //12-23
9026,-2233,1246,-841, 616,-470, 367,-289, 229,-181, 143,-112, //24-35
  87, -66,  50, -37,  27, -19,  13,  -9,   6,  -3,   2,  -1,  //36-47
};
//b1(3:3:end) : polyphase interpolator
short stage2InterpCoef3[48] = {          
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
short stage2InterpCoef2[48] = {
-2,5,-8,14,-23,35,-51,73,-101,136,-182,239,                      //0-11
-309,396,-503,636,-803,1015,-1295,1682,-2260,3237,-5324,13511,   //12-23
27079,-6698,3739,-2523,1847,-1410,1100,-867,687,-544,429,-336,   //24-35
260,-199,150,-112,81,-58,40,-26,17,-10,6,-3,                       
};
//b1(3:3:end) : polyphase interpolator
short stage2InterpCoef3[48] = {          
-3,6,-10,17,-26,40,-58,81,-112,150,-199,260,                     //0-11
-336,429,-544,687,-867,1100,-1410,1847,-2523,3739,-6698,27079,   //12-23
13511,-5324,3237,-2260,1682,-1295,1015,-803,636,-503,396,-309,   //24-35
239,-182,136,-101,73,-51,35,-23,14,-8,5,-2,                      //36-47
};

#endif  //QL_INTERPBY3_H_
