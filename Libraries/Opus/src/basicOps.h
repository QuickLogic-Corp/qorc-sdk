//Basic ARM intrinsic
#ifndef BASIC_OPS__H
#define BASIC_OPS__H
#ifndef WIN32
#define USE_INLINE_ASM
#endif
typedef int fract32;
typedef short int fract16;



#ifdef USE_INLINE_ASM
#pragma inline=forced
static  float ql_macro_sqrt(float a)
{
  float res;
  __asm("VSQRT.F32 %0,%1" : "=t"(res) : "t"(a));
  return res;
}
// VABS.F32 S0,S0
static  float ql_macro_fabs(float a)
{
  float res;
  __asm("VABS.F32 %0,%1" : "=t"(res) : "t"(a));
  return res;
}
// CLZ     R0,R0
static  int ql_macro_clz(int a)
{
  int res;
  __asm("CLZ %0,%1" : "=r"(res) : "r"(a));
  return res;
}

static  fract32 ql_macro_a_plus_b_by2(int a, int b)
{
  int res;
  __asm("add %0, %1, %2, asr #1" : "=r"(res) : "r"(a), "r"(b));
  return res;
}
static  fract32 ql_macro_a_minus_b_by2(int a, int b)
{
  int res;
  __asm("sub %0, %1, %2, asr #1" : "=r"(res) : "r"(a), "r"(b));
  return res;
}

static  fract32 ql_macro_aby2_plus_b(int a, int b)
{
  int res;
  __asm("add %0, %1, %2, asr #1" : "=r"(res) : "r"(b), "r"(a));
  return res;
}
static  fract32 ql_macro_aby2_minus_b(int a, int b)
{
  int res;
  __asm("rsb %0, %1, %2, asr #1" : "=r"(res) : "r"(b), "r"(a));
  return res;
}

#pragma inline=forced
static fract32 ql_cplxmultdiv2(fract32 ar, fract32 ai, fract32 br, fract32 bi, fract32 *or, fract32 *oi)
{
  fract32 abrl, abrh;
  fract32 abil, abih;
   __asm( "smull %0, %1, %2, %3\n\t" : "=r"(abrl), "=r"(abrh) : "r"(ar), "r"(br)  ); 
   __asm( "smull %0, %1, %2, %3\n\t" : "=r"(abil), "=r"(abih) : "r"(ai), "r"(bi)  ); 
   __asm( "sub %0, %1, %2\n\t" : "=r"(abrh) : "r"(abrh), "r"(abih) );
   __asm( "lsl %0, %1, #11\n\t" : "=r"(abrh)  : "r"(abrh)  );
   __asm( "add %0, %1, %2, lsr #21\n\t" : "=r"(abrh) : "r"(abrh), "r"(abrl) );
   __asm( "sub %0, %1, %2, lsr #21\n\t" : "=r"(abrh) : "r"(abrh), "r"(abil) );
   *or = abrh;

   __asm( "smull %0, %1, %2, %3\n\t" : "=r"(abrl), "=r"(abrh) : "r"(ar), "r"(bi)  ); 
   __asm( "smull %0, %1, %2, %3\n\t" : "=r"(abil), "=r"(abih) : "r"(ai), "r"(br)  ); 
   __asm( "add %0, %1, %2\n\t" : "=r"(abrh) : "r"(abrh), "r"(abih) );
   __asm( "lsl %0, %1, #11\n\t" : "=r"(abrh)  : "r"(abrh)  );
   __asm( "add %0, %1, %2, lsr #21\n\t" : "=r"(abrh) : "r"(abrh), "r"(abrl) );
   __asm( "add %0, %1, %2, lsr #21\n\t" : "=r"(abrl) : "r"(abrh), "r"(abil) );  
   *oi = abrl;
  return 0;
}

#else
static fract32 ql_cplxmultdiv2(fract32 ar, fract32 ai, fract32 br, fract32 bi, fract32 * or , fract32 *oi)
{
    *or = ar * br - ai * bi;
    *oi = ar * bi + ai * br;
    return 0;
}
static  fract32 ql_macro_a_plus_b_by2(fract32 a, fract32 b)
{
    return( a + (b>>1));
}
static  fract32 ql_macro_aby2_plus_b(fract32 a, fract32 b)
{
    return((a>>1) + b);

}
static  fract32 ql_macro_a_minus_b_by2(fract32 a, fract32 b)
{
    return(a - (b >> 1));
}
static  fract32 ql_macro_aby2_minus_b(fract32 a, fract32 b)
{
    return((a >> 1) - b);
}

float vfpu_sqrt(float input);
float VABS_ASM(float x);
int CLZ_ASM(unsigned int input);
float vfpu_test(float input);
#endif

#ifndef WIN32
#pragma inline=forced
#endif
static float ql_cos(float x)
{
float ql_cos_tab[] = {
 -0.5f,
 0.04166666667f,
 -0.001388888889f,
 0.0000248015873f
};
float y =1;
  float _XX = x*x;
  
  y += _XX*(ql_cos_tab[0] + _XX*(ql_cos_tab[1] + _XX*(ql_cos_tab[2] + ql_cos_tab[3]*_XX)));

  return y; 
}


#ifndef BASIC_OPS_ASM
#define complex_mult32_q16  complex_mult32_q16_c
#define complex_mult32_q16_tableMultiplex complex_mult32_q16_tableMultiplex_c
#else
#define complex_mult32_q16  complex_mult32_q16_asm
#define complex_mult32_q16_tableMultiplex complex_mult32_q16_tableMultiplex_asm
#endif


extern fract32 mult32_q16 (fract32 a32, fract16 *ptab16);
extern fract32 array_mult32_q16 (fract32 *pa32, fract16 *ptab16, int length);
extern void complex_mult32_q16 (fract32 a_Re, fract32 a_Im, fract16* b_Re_Tab, fract16* b_Im_Tab, fract32* ReO, fract32* ImO);
extern void complex_mult32_q16_tableMultiplex (fract32 a_Re, fract32 a_Im, fract32* b_Tab, fract32* ReO, fract32* ImO);



#endif /* BASIC_OPS__H */
