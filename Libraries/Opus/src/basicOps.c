#include "basicOps.h"

fract32 mult32_q16(fract32 a32, fract16 *ptab16)
{
  fract32 result = 0;
  result = ((long long)a32 * ((*ptab16) << 16)) >> 32;
  return result;
}

fract32 array_mult32_q16(fract32 *pa32, fract16 *ptab16, int length)
{
  fract32 result = 0;
  int i;
  for (i = 0; i<length; i++)
  {
    result += ((long long)pa32[i] * (ptab16[i] << 16)) >> 32;
  }
  return result;
}


void complex_mult32_q16_c(fract32 a_Re, fract32 a_Im, fract16* b_Re_Tab, fract16* b_Im_Tab, fract32* ReO, fract32* ImO)
{
  fract32 r0,r1;
  fract32 r32Tab, i32Tab;

  //result = ((long long)a32 * ((*ptab16) << 16)) >> 32;
  r32Tab = (*b_Re_Tab << 16);
  i32Tab = (*b_Im_Tab << 16);

  //r0 =  fMult(a_Re,b_Re) - fMult(a_Im, b_Im);
  //r1 =  fMult(a_Re,b_Im) + fMult(a_Im, b_Re);

  r0 = ((long long)a_Im * i32Tab) >> 32;  //fMult(a_Im, b_Im)
  r1 = ((long long)a_Im * r32Tab) >> 32;  //fMult(a_Im, b_Re);
  
  r0  = (fract32)(((long long)a_Re * r32Tab) >> 32) - r0;  //fMult(a_Re,b_Re)
  r1  = (fract32)(((long long)a_Re * i32Tab) >> 32) + r1;  //fMult(a_Re,b_Im)

  *ReO = r0;
  *ImO = r1;
}


void complex_mult32_q16_tableMultiplex_c(fract32 a_Re, fract32 a_Im, fract32* b_Tab, fract32* ReO, fract32* ImO)
{
  fract32 r0,r1;
  fract32 r32Tab, i32Tab;

  //result = ((long long)a32 * ((*ptab16) << 16)) >> 32;
  r0 = *b_Tab; 
  r32Tab = (r0 << 16);
  i32Tab = r0 & 0xFFFF0000; 

  //r0 =  fMult(a_Re,b_Re) - fMult(a_Im, b_Im);
  //r1 =  fMult(a_Re,b_Im) + fMult(a_Im, b_Re);

  r0 = ((long long)a_Im * i32Tab) >> 32;  //fMult(a_Im, b_Im)
  r1 = ((long long)a_Im * r32Tab) >> 32;  //fMult(a_Im, b_Re);
  
  r0  = (fract32)(((long long)a_Re * r32Tab) >> 32) - r0;  //fMult(a_Re,b_Re)
  r1  = (fract32)(((long long)a_Re * i32Tab) >> 32) + r1;  //fMult(a_Re,b_Im)

  *ReO = r0;
  *ImO = r1;
}


//#define TEST_BASIC_OPS
#ifdef TEST_BASIC_OPS
float mult32_float(float a32, float *ptab32)
{
  float result = 0;
  result = a32 * (*ptab32);
  return result;
}

float array_mult32_float(float *pa32, float *ptab32, int length)
{
  float result = 0;
  int i;
  for (i = 0; i<length; i++)
  {
    result += pa32[i] * (ptab32[i]);
  }
  return result;
}

void complex_mult32_float(float a_Re, float a_Im, float* b_Re_Tab, float* b_Im_Tab, float* ReO, float* ImO)
{
  float r0,r1;
  //float r32Tab, i32Tab;

  //result = ((long long)a32 * ((*ptab16) << 16)) >> 32;
  //r32Tab = (*b_Re_Tab << 16);
  //i32Tab = (*b_Im_Tab << 16);

  //r0 =  fMult(a_Re,b_Re) - fMult(a_Im, b_Im);
  //r1 =  fMult(a_Re,b_Im) + fMult(a_Im, b_Re);

  r0 = a_Im * (*b_Im_Tab);  //fMult(a_Im, b_Im)
  r1 = a_Im * (*b_Re_Tab);  //fMult(a_Im, b_Re);
  
  r0  = a_Re * (*b_Re_Tab) - r0;  //fMult(a_Re,b_Re)
  r1  = a_Re * (*b_Im_Tab) + r1;  //fMult(a_Re,b_Im)

  *ReO = r0;
  *ImO = r1;
}

#if 1
void complex_mult32_tableMultiplex_float(float a_Re, float a_Im, float* b_Re_Tab, float* b_Im_Tab, float* ReO, float* ImO)
{
  float r0,r1;
  //float r32Tab, i32Tab;

  //result = ((long long)a32 * ((*ptab16) << 16)) >> 32;
  //r32Tab = (*b_Re_Tab << 16);
  //i32Tab = (*b_Im_Tab << 16);

  //r0 =  fMult(a_Re,b_Re) - fMult(a_Im, b_Im);
  //r1 =  fMult(a_Re,b_Im) + fMult(a_Im, b_Re);

  r0 = a_Im * (*b_Im_Tab);  //fMult(a_Im, b_Im)
  r1 = a_Im * (*b_Re_Tab);  //fMult(a_Im, b_Re);
  
  r0  = a_Re * (*b_Re_Tab) - r0;  //fMult(a_Re,b_Re)
  r1  = a_Re * (*b_Im_Tab) + r1;  //fMult(a_Re,b_Im)

  *ReO = r0;
  *ImO = r1;
}
#endif


#define SAMPLES_LENGTH 32

fract32 pcmSamples[SAMPLES_LENGTH] = 
{
  0x00fe, 0xfd00, 0x8b00, 0x00fc, 0xeb00, 0xfc00, 0x00d0, 0xfc00, 0xc900, 0x00fc, 0x9d00, 0x00fc, 0x9c00, 0x00fc, 0x9b00, 0x00fc,
  0x11fe, 0x1200, 0x3400, 0x34fc, 0x4300, 0x4300, 0x43d0, 0x6700, 0x7800, 0x89fc, 0x8900, 0x89fc, 0x8900, 0x11fc, 0x9b56, 0x49fc
};

fract16 tab[SAMPLES_LENGTH] = {
	0xfe12, 0x3333, 0x1b77, 0x1234, 0x22eb, 0x22fc, 0x22d0, 0x23fc, 0x45c9, 0x39fc, 0x079d, 0xeefc, 0xdd9c, 0xccfc, 0xdb9b, 0xfafc,
        0x12dd, 0x33dd, 0x1b78, 0x1245, 0x1eb1, 0xfffc, 0xffd0, 0xfffc, 0xccc9, 0x12fc, 0xcc9d, 0xdfc1, 0xef9c, 0xeefc, 0xee9b, 0x0afc
};

float pcmSamplesFloat[SAMPLES_LENGTH] = 
{
  0.12f, 0xfd00, 0x8b00, 0x00fc, 0xeb00, 0xfc00, 0x00d0, 0xfc00, 0xc900, 0x00fc, 0x9d00, 0x00fc, 0x9c00, 0x00fc, 0x9b00, 0x00fc
};

float tabFloat[] = {
	0xfe, 0xfd, 0x8b, 0xfc, 0xeb, 0xfc, 0xd0, 0xfc, 0xc9, 0xfc, 0x9d, 0xfc, 0x9c, 0xfc, 0x9b, 0xfc
};

#define FIX16(x) ((x) * (1 << 16))
#define FIX15(x) ((x) * (1 << 15))
#define FIX20(x) ((x) * (1 << 20))

fract16 tabCoef15[SAMPLES_LENGTH] = {
    FIX15(0.1), FIX15(0.2), FIX15(0.3), FIX15(0.4), FIX15(-0.1), FIX15(-0.2), FIX15(-0.3), FIX15(-0.4), 
    FIX15(0.15), FIX15(0.25), FIX15(0.35), FIX15(0.45), FIX15(-0.15), FIX15(-0.25), FIX15(-0.35), FIX15(-0.45), 
    FIX15(0.2), FIX15(0.3), FIX15(0.4), FIX15(0.5), FIX15(0.4), FIX15(0.3), FIX15(0.2), FIX15(0.1), 
    FIX15(-0.2), FIX15(-0.3), FIX15(-0.4), FIX15(-0.5), FIX15(-0.4), FIX15(-0.3), FIX15(-0.2), FIX15(-0.1)
};

fract16 tabCoefReal[SAMPLES_LENGTH] = {
    FIX15(0.1), FIX15(0.2), FIX15(0.3), FIX15(0.4), FIX15(-0.1), FIX15(-0.2), FIX15(-0.3), FIX15(-0.4), 
    FIX15(0.15), FIX15(0.25), FIX15(0.35), FIX15(0.45), FIX15(-0.15), FIX15(-0.25), FIX15(-0.35), FIX15(-0.45), 
    FIX15(0.2), FIX15(0.3), FIX15(0.4), FIX15(0.5), FIX15(0.4), FIX15(0.3), FIX15(0.2), FIX15(0.1), 
    FIX15(-0.2), FIX15(-0.3), FIX15(-0.4), FIX15(-0.5), FIX15(-0.4), FIX15(-0.3), FIX15(-0.2), FIX15(-0.1)
};


fract16 tabCoefImag[SAMPLES_LENGTH] = {
    FIX15(0.2), FIX15(0.3), FIX15(0.4), FIX15(0.45), FIX15(0.4), FIX15(0.3), FIX15(0.2), FIX15(0.1), 
    FIX15(0.1), FIX15(0.2), FIX15(0.3), FIX15(0.4), FIX15(-0.1), FIX15(-0.2), FIX15(-0.3), FIX15(-0.4), 
    FIX15(-0.2), FIX15(-0.3), FIX15(-0.4), FIX15(-0.5), FIX15(-0.4), FIX15(-0.3), FIX15(-0.2), FIX15(-0.1),
    FIX15(0.15), FIX15(0.25), FIX15(0.35), FIX15(0.45), FIX15(-0.15), FIX15(-0.25), FIX15(-0.35), FIX15(-0.45)
};


#define mult32x15(a, b)         ((((long long)(a))*(((long long)(b))))>> (15))

fract32 inputSamples[SAMPLES_LENGTH] = {
    FIX20(0.1), FIX20(0.2), FIX20(0.3), FIX20(0.4), FIX20(-0.1), FIX20(-0.2), FIX20(-0.3), FIX20(-0.4), 
    FIX20(0.15), FIX20(0.25), FIX20(0.35), FIX20(0.45), FIX20(-0.15), FIX20(-0.25), FIX20(-0.35), FIX20(-0.45), 
    FIX20(0.2), FIX20(0.3), FIX20(0.4), FIX20(0.45), FIX20(0.4), FIX20(0.3), FIX20(0.2), FIX20(0.1), 
    FIX20(-0.2), FIX20(-0.3), FIX20(-0.4), FIX20(-0.5), FIX20(-0.4), FIX20(-0.3), FIX20(-0.2), FIX20(-0.1)
};

fract32 inputSamplesComplex[SAMPLES_LENGTH * 2] = {
    FIX20(0.1), FIX20(0.2), FIX20(0.3), FIX20(0.4), FIX20(-0.1), FIX20(-0.2), FIX20(-0.3), FIX20(-0.4), 
    FIX20(0.15), FIX20(0.25), FIX20(0.35), FIX20(0.45), FIX20(-0.15), FIX20(-0.25), FIX20(-0.35), FIX20(-0.45), 
    FIX20(0.2), FIX20(0.3), FIX20(0.4), FIX20(0.45), FIX20(0.4), FIX20(0.3), FIX20(0.2), FIX20(0.1), 
    FIX20(-0.2), FIX20(-0.3), FIX20(-0.4), FIX20(-0.5), FIX20(-0.4), FIX20(-0.3), FIX20(-0.2), FIX20(-0.1)
    FIX20(0.1), FIX20(0.2), FIX20(0.3), FIX20(0.4), FIX20(-0.1), FIX20(-0.2), FIX20(-0.3), FIX20(-0.4), 
    FIX20(0.15), FIX20(0.25), FIX20(0.35), FIX20(0.45), FIX20(-0.15), FIX20(-0.25), FIX20(-0.35), FIX20(-0.45), 
    FIX20(0.2), FIX20(0.3), FIX20(0.4), FIX20(0.45), FIX20(0.4), FIX20(0.3), FIX20(0.2), FIX20(0.1), 
    FIX20(-0.2), FIX20(-0.3), FIX20(-0.4), FIX20(-0.5), FIX20(-0.4), FIX20(-0.3), FIX20(-0.2), FIX20(-0.1)
};


fract32 mac32_q15(fract32 *input, fract16* coeff, int length)
{
    fract32 result = 0;
    int i;
    for (i = 0; i < length; i++)
    {
        result += mult32x15(input[i], coeff[i]);
    }
    return result;
}

void complex_mac32_q15(fract32 *complexInput, fract16 *rTable, fract16 *iTable, int length, fract32* realOutput, fract32* imagOutput)
{
    fract32 realResult = 0;
    fract32 imagResult = 0;
    fract32 tempReal;
    fract32 tempImag;
    int i;
    for (i = 0; i < length; i++)
    {
        complex_mult32_q16_c(complexInput[i*2], ImagInput[i*2+1], rTable[i], iTable[i], &tempReal, &tempImag);
        realResult += tempReal;
        imagResult += tempImag;
    }
    
    *realOutput = realResult;
    *imagOutput = imagResult;
    return;
}

void testBasicOps(void)
{
    fract32 result_real_ref1 = 0;
    fract32 result_imag_ref1 = 0; 

    fract32 result_real_ref2 = 0;
    fract32 result_imag_ref2 = 0; 

    
    complex_mac32_q15(inputSamplesComplex, tabCoefReal, tabCoefImag, int length, result_real_ref1, result_imag_ref1)
    
}


void testBasicOps2(void)
{
    fract32 result_ref1 = 0;
    fract32 result_ref2 = 0;
 
    result_ref1 = mac32_q15(inputSamples, tabCoef15, SAMPLES_LENGTH);
    //result_ref2 = array_mult32_q16(inputSamples, tabCoef16, SAMPLES_LENGTH);
    result_ref2 = array_mult32_q16(inputSamples, tabCoef15, SAMPLES_LENGTH);
    
    printf("%f %f\n", (result_ref1*1.0f)/(1 << 20), (result_ref2*1.0f)/(1 << 19) );
}

void testBasicOps1(void)
{
  fract32 r0 = 0x12345678;
  fract32 r1,r2,r3,r4,r5,r6,r7,r8;

  float s0 = 0x12345678;
  float s1,s2,s3,s4,s5,s6;

  s1 = mult32_float(s0, tabFloat);
  r1 = mult32_q16(r0, tab);
  
  s2 = array_mult32_float(pcmSamplesFloat, tabFloat, SAMPLES_LENGTH);
  r2 = array_mult32_q16(pcmSamples, tab, SAMPLES_LENGTH);  
  
  complex_mult32_float(pcmSamplesFloat[0], pcmSamplesFloat[1], tabFloat, tabFloat, &s1, &s2);
  complex_mult32_q16_c(pcmSamples[0], pcmSamples[1], &tab[0], &tab[1], &r1, &r2);
  complex_mult32_q16_asm(pcmSamples[0], pcmSamples[1], &tab[0], &tab[1], &r3, &r4);
  
  complex_mult32_tableMultiplex_float(pcmSamplesFloat[0], pcmSamplesFloat[1], tabFloat, tabFloat, &s3, &s4);
  complex_mult32_q16_tableMultiplex_c(pcmSamples[0], pcmSamples[1], (fract32 *)tab, &r5, &r6);
  complex_mult32_q16_tableMultiplex_asm(pcmSamples[0], pcmSamples[1], (fract32 *)tab, &r7, &r8);
  
  complex_mult32_float(pcmSamplesFloat[0], pcmSamplesFloat[1], tabFloat, tabFloat, &s3, &s4);
  complex_mult32_q16_c(pcmSamples[0], pcmSamples[1], &tab[2], &tab[3], &r1, &r2);
  complex_mult32_q16(pcmSamples[0], pcmSamples[1], &tab[2], &tab[3], &r3, &r4);
  
  complex_mult32_tableMultiplex_float(pcmSamplesFloat[0], pcmSamplesFloat[1], tabFloat, tabFloat, &s5, &s6);
  complex_mult32_q16_tableMultiplex(pcmSamples[0], pcmSamples[1], (fract32 *)&tab[2], &r5, &r6);
  complex_mult32_q16_tableMultiplex_asm(pcmSamples[0], pcmSamples[1], (fract32 *)&tab[2], &r7, &r8);

  s2 = array_mult32_float(pcmSamplesFloat, tabFloat, SAMPLES_LENGTH);
  r2 = array_mult32_q16(pcmSamples, tab, SAMPLES_LENGTH);  
 
}

float vfpu_test(float input)
{
    float output;
    output = vfpu_sqrt(input);
    output = VABS_ASM(output);    

    return output;
}

#endif















//#define DUMMY_STUBS
#ifdef DUMMY_STUBS
float intrin1(float input)
{
    float output;
    output = input * input;
    return output;
}

float intrin2(float input)
{
    float output;
    output = -input;
    return output;
}


unsigned char intrin3(unsigned int input)
{
    unsigned char output;
    output = (unsigned char)input;
    return output;
}

#endif