//#define TST_QL_FFT

#include "ql_dsp.h"
#include "ql_mem.h"

typedef  ql_sample_in_t fft_in_sample_t;
typedef  ql_sample_in_t fft_coeff_t;



#ifndef TST_QL_FFT
#include "stack_alloc.h"
#else
#define VARDECL(type, var) type *var
#  define ALLOC(var, size, type)  var = ((type*)MALLOC2(sizeof(type)*(size))); // printf("alloc %s[%d]\t\t", #var, size*sizeof(type));

#  define FREE1(var, size, type)  FREE2_ql(size*sizeof(type));  //printf("free %s[%d]", #var, size*sizeof(type));
#define  QL_OPUS_ENC_COMPLEXITY  4
// #define QL_FFT_240
#define QL_FFT_480 

#endif

#ifndef USE_KISS_FFT
#if ((defined QL_FFT_240) || defined QL_FFT_480)

#if defined QL_FFT_240    
#define QL_FFT_16
#define QL_FFT_15
#define QL_FFT_5
#endif

#if defined QL_FFT_480    
// #define QL_FFT_8
#define QL_FFT_32
#define QL_FFT_15
#if(QL_OPUS_ENC_COMPLEXITY >= 4)
#define QL_FFT_60
#define QL_FFT_4
#define QL_FFT_5
#endif
#endif
#else
#error "either QL_FFT_240 OR QL_FFT_480 to defined "
#endif
#endif
#ifdef ENABLE_FIXEDPOINT
#ifndef WIN32
#define USE_ASM
#endif

#define     FL2FXCONST(x)   ((fft_coeff_t)((x)*(1<<QL_OPUS_FIXEDPOINT_BITS)))
#define STC(x)   ((x)>> (31 - QL_OPUS_FIXEDPOINT_BITS))

#define ROTA(x) ((rota_coeff_t)(x*(1<<QL_OPUS_ROTA_BITS)))

#define     W_PiTHIRD       FL2FXCONST(-0.86602540f)   // (F3C(0x91261468))  /// Sin(pi/3) 
#define     W_PiFOURTH      FL2FXCONST(0.707106781f)    // STC(0x5a82799a)   /// Sin(pi/4) it is 1/sqrt(2)
#define SIGN(a)      0 // ((a) < 0 ? -1 : 1)
#define Div2(a)       ( ((a) >> 1)    + SIGN(a) )
#define Div4(a)       ( ((a) >> 2)    + SIGN(a) )
#if (QL_OPUS_FIXEDPOINT_BITS > 15)
#include "basicOps.h"

#ifdef USE_ASM
#pragma inline = forced 
fract32 ql_macro_mult_q32_q16(fract32 a, fract32 b)
{
  fract32 rdl, rdh, res;
//  asm("smmul r1, r1, r2"); 
//  asm("add r0, r0, r1, lsr #1");
   __asm(
        "smull %0, %1, %2, %3\n\t"
         : "=r"(rdl), "=r"(rdh)
         : "r"(a), "r"(b)
        ); 

   __asm(
         "lsr %0, %1, #20\n\t"
         : "=r"(res)
         : "r"(rdl)
        );
   __asm(
         "add %0, %1, %2, lsl #12\n\t"
         : "=r"(res)
         : "r"(res), "r"(rdh)
        );

  
return res;
}
fract32 ql_macro_mult_q32_q16_sh2(fract32 a, fract32 b)
{
  fract32 rdl, rdh, res;
//  asm("smmul r1, r1, r2"); 
//  asm("add r0, r0, r1, lsr #1");
//  asm("lsr r0, r1, #20");
//  asm("lsr r0, r1, r2");
   __asm(
        "smull %0, %1, %2, %3\n\t"
         : "=r"(rdl), "=r"(rdh)
         : "r"(a), "r"(b)
        ); 

   __asm(
         "lsr %0, %1, #21\n\t"
         : "=r"(res)
         : "r"(rdl)
        );

   __asm(
         "add %0, %1, %2, lsl #11\n\t"
         : "=r"(res)
         : "r"(res), "r"(rdh)
        );

  
return res;
}

#pragma inline = forced 
fract32 ql_macro_mult_fract32_fract16_15(fract32 a, fract32 b)
{
  fract32 rdl, rdh, res;
//  asm("smmul r1, r1, r2"); 
//  asm("add r0, r0, r1, lsr #1");
   __asm(
        "smull %0, %1, %2, %3\n\t"
         : "=r"(rdl), "=r"(rdh)
         : "r"(a), "r"(b)
        ); 

   __asm(
         "lsr %0, %1, #15\n\t"
         : "=r"(res)
         : "r"(rdl)
        );
   __asm(
         "add %0, %1, %2, lsl #17\n\t"
         : "=r"(res)
         : "r"(res), "r"(rdh)
        );

  
return res;
}
#define fMult(a, b)           ql_macro_mult_q32_q16(a, b)
#define fMultDiv2(a, b)       ql_macro_mult_q32_q16_sh2(a, b)
#define fMultRota(a, b)       ql_macro_mult_fract32_fract16_15(a, b)

#else
#define fMult(a, b)         ((((int_acc_t)(a))*(((int_acc_t)(b))))>> (QL_OPUS_FIXEDPOINT_BITS))
#define fMultDiv2(a, b)     ((((int_acc_t)(a))*(((int_acc_t)(b))))>> (QL_OPUS_FIXEDPOINT_BITS + 1))
#define fMultRota(a, b)         ((((int_acc_t)(a))*(((int_acc_t)(b))))>> (QL_OPUS_ROTA_BITS))

#endif

#else
#define fMult(a, b)         ((((int_acc_t)(a))*(((int_acc_t)(b))))>> (QL_OPUS_FIXEDPOINT_BITS))
#define fMultDiv2(a, b)     ((((int_acc_t)(a))*(((int_acc_t)(b))))>> (QL_OPUS_FIXEDPOINT_BITS + 1))

#define fMultRota(a, b)         ((((int_acc_t)(a))*(((int_acc_t)(b))))>> (QL_OPUS_ROTA_BITS))

#endif
#define AddDiv2(a, b)       (((a) + (b)) >> 1)    
#define AddDiv4(a, b)       (((a) + (b)) >> 2)    
#define SubDiv4(a, b)       (((a) - (b)) >> 2)    
#include <math.h>

#define QL_B2(x)  Div2(x)


#define ARRAY_SCALE4_STRIDE(pD_, stride_, n_)         {\
for (int i = 0; i<n_*stride_; i += stride_) { \
    pD_[i] >>= 2;    \
    pD_[i + 1] >>= 2; \
} \
}
#else
#define     FL2FXCONST(x)   (x)
#define STC(x)   (x*1.0f/2147483648)
#define fMultRota(a, b)         ((a)*(b))

#define ROTA(x) (x)

#define     W_PiTHIRD       FL2FXCONST(-0.86602540f)   // (F3C(0x91261468))  /// Sin(pi/3) 
#define     W_PiFOURTH      FL2FXCONST(0.707106781f)    // STC(0x5a82799a)   /// Sin(pi/4) it is 1/sqrt(2)

#define Div2(a)       ((a) * 0.5f)    
#define Div4(a)       ((a) * 0.25f)    
#define fMultDiv2(a, b) (((a)*(b)) * 0.5f)
#define fMult(a, b) ((a)*(b))
#define AddDiv2(a, b)       (((a) + (b)) * 0.5f)    
#define AddDiv4(a, b)       (((a) + (b)) * 0.25f)    
#define SubDiv4(a, b)       (((a) - (b)) * 0.25f)    

#define QL_B2(x)  ((x)*0.5f)

#define ARRAY_SCALE4_STRIDE(pD_, stride_, n_)         {\
for (int i = 0; i<n_*stride_; i += stride_) { \
    pD_[i] *= 0.25f;    \
    pD_[i + 1] *= 0.25f; \
} \
}
#endif

/* Performs the FFT of length 3 according to the algorithm after winograd.
No scaling of the input vector because the scaling is already done in the rotation vector. */

#ifdef QL_FFT_3
#define FFT_DIV2(x) ((x)*0.5f)
void fft3(fft_in_sample_t * pDat)
{
	fft_in_sample_t r1, r2;
	fft_in_sample_t s1, s2;
	/* real part */
	r1 = pDat[2] + pDat[4];
    r2 = fMult((pDat[2] - pDat[4]), W_PiTHIRD);
	pDat[0] = pDat[0] + r1;
	r1 = pDat[0] - r1 - FFT_DIV2(r1);

	/* imaginary part */
	s1 = pDat[3] + pDat[5];
    s2 = fMult((pDat[3] - pDat[5]), W_PiTHIRD);
	pDat[1] = pDat[1] + s1;
	s1 = pDat[1] - s1 - FFT_DIV2(s1);

	/* combination */
	pDat[2] = r1 - s2;
	pDat[4] = r1 + s2;
	pDat[3] = s1 + r2;
	pDat[5] = s1 - r2;
}
#endif
#ifdef QL_FFT_4
static void fft_4(fft_in_sample_t *x)
{
	fft_in_sample_t a00, a10, a20, a30, tmp0, tmp1;

    a00 = AddDiv2(x[0] , x[4]); // >> 1;  /* Re A + Re B */
    a10 = AddDiv2(x[2] , x[6]); //  >> 1;  /* Re C + Re D */
    a20 = AddDiv2(x[1] , x[5]); //  >> 1;  /* Im A + Im B */
    a30 = AddDiv2(x[3] , x[7]); //  >> 1;  /* Im C + Im D */

	x[0] = a00 + a10;       /* Re A' = Re A + Re B + Re C + Re D */
	x[1] = a20 + a30;       /* Im A' = Im A + Im B + Im C + Im D */

	tmp0 = a00 - x[4];       /* Re A - Re B */
	tmp1 = a20 - x[5];       /* Im A - Im B */

	x[4] = a00 - a10;       /* Re C' = Re A + Re B - Re C - Re D */
	x[5] = a20 - a30;       /* Im C' = Im A + Im B - Im C - Im D */

	a10 = a10 - x[6];       /* Re C - Re D */
	a30 = a30 - x[7];       /* Im C - Im D */

	x[2] = tmp0 + a30;       /* Re B' = Re A - Re B + Im C - Im D */
	x[6] = tmp0 - a30;       /* Re D' = Re A - Re B - Im C + Im D */
	x[3] = tmp1 - a10;       /* Im B' = Im A - Im B - Re C + Re D */
	x[7] = tmp1 + a10;       /* Im D' = Im A - Im B + Re C - Re D */
}
#endif


#ifdef QL_FFT_5
/* performs the FFT of length 5 according to the algorithm after winograd */

#define     C51     FL2FXCONST( 0.95105652f)       /*   (F5C(0x79bc3854))    */
#define     C52     FL2FXCONST(-1.53884180f * 2)   /*   (F5C(0x9d839db0))    */
#define     C53     FL2FXCONST(-0.36327126f)       /*   (F5C(0xd18053ce))    */
#define     C54     FL2FXCONST( 0.55901699f)       /*   (F5C(0x478dde64))    */
#define     C55     FL2FXCONST(-1.25f *2)          /*   (F5C(0xb0000001))    */

void fft5(fft_in_sample_t *pDat)
{
	fft_in_sample_t r1, r2, r3, r4;
	fft_in_sample_t s1, s2, s3, s4;
	fft_in_sample_t t;

	/* real part */
	r1 = pDat[2] + pDat[8];
	r4 = pDat[2] - pDat[8];
	r3 = pDat[4] + pDat[6];
	r2 = pDat[4] - pDat[6];
	t = fMult((r1 - r3), C54);
	r1 = r1 + r3;
	pDat[0] = pDat[0] + r1;
	/* Bit shift left because of the constant C55 which was scaled with the factor 0.5 because of the representation of
	the values as fracts */
	r1 = pDat[0] + (fMultDiv2(r1, C55));
	r3 = r1 - t;
	r1 = r1 + t;
	t = fMult((r4 + r2), C51);
	/* Bit shift left because of the constant C55 which was scaled with the factor 0.5 because of the representation of
	the values as fracts */
	r4 = t + (fMultDiv2(r4, C52));
	r2 = t + fMult(r2, C53);

	/* imaginary part */
	s1 = pDat[3] + pDat[9];
	s4 = pDat[3] - pDat[9];
	s3 = pDat[5] + pDat[7];
	s2 = pDat[5] - pDat[7];
	t = fMult((s1 - s3), C54);
	s1 = s1 + s3;
	pDat[1] = pDat[1] + s1;
	/* Bit shift left because of the constant C55 which was scaled with the factor 0.5 because of the representation of
	the values as fracts */
	s1 = pDat[1] + (fMultDiv2(s1, C55));
	s3 = s1 - t;
	s1 = s1 + t;
	t = fMult((s4 + s2), C51);
	/* Bit shift left because of the constant C55 which was scaled with the factor 0.5 because of the representation of
	the values as fracts */
	s4 = t + (fMultDiv2(s4, C52));
	s2 = t + fMult(s2, C53);

	/* combination */
	pDat[2] = r1 + s2;
	pDat[8] = r1 - s2;
	pDat[4] = r3 - s4;
	pDat[6] = r3 + s4;

	pDat[3] = s1 - r2;
	pDat[9] = s1 + r2;
	pDat[5] = s3 + r4;
	pDat[7] = s3 - r4;
}
#endif

#ifdef QL_FFT_8
static  void fft_8(fft_in_sample_t *x)
{

	fft_in_sample_t a00, a10, a20, a30;
	fft_in_sample_t y[16];

	a00 = (x[0] + x[8]) /2;
	a10 = x[4] + x[12];
	a20 = (x[1] + x[9]) /2;
	a30 = x[5] + x[13];

	y[0] = a00 + (a10 /2);
	y[4] = a00 - (a10 /2);
	y[1] = a20 + (a30 /2);
	y[5] = a20 - (a30 /2);

	a00 = a00 - x[8];
	a10 = (a10 /2) - x[12];
	a20 = a20 - x[9];
	a30 = (a30 /2) - x[13];

	y[2] = a00 + a30;
	y[6] = a00 - a30;
	y[3] = a20 - a10;
	y[7] = a20 + a10;

	a00 = (x[2] + x[10]) /2;
	a10 = x[6] + x[14];
	a20 = (x[3] + x[11]) /2;
	a30 = x[7] + x[15];

	y[8] = a00 + (a10  /2);
	y[12] = a00 - (a10 /2);
	y[9] = a20 + (a30  /2);
	y[13] = a20 - (a30 /2);

	a00 = a00 - x[10];
	a10 = (a10 /2) - x[14];
	a20 = a20 - x[11];
	a30 = (a30 /2) - x[15];

	y[10] = a00 + a30;
	y[14] = a00 - a30;
	y[11] = a20 - a10;
	y[15] = a20 + a10;

	fft_in_sample_t vr, vi, ur, ui;

	ur = y[0] /2;
	ui = y[1] /2;
	vr = y[8];
	vi = y[9];
	x[0] = ur + (vr /2);
	x[1] = ui + (vi /2);
	x[8] = ur - (vr /2);
	x[9] = ui - (vi /2);

	ur = y[4] /2;
	ui = y[5] /2;
	vi = y[12];
	vr = y[13];
	x[4] = ur + (vr  /2);
	x[5] = ui - (vi  /2);
	x[12] = ur - (vr /2);
	x[13] = ui + (vi /2);

	ur = y[10];
	ui = y[11];
	vr = fMultDiv2(ui + ur, W_PiFOURTH);
	vi = fMultDiv2(ui - ur, W_PiFOURTH);
	ur = y[2];
	ui = y[3];
	x[2] = (ur  /2) + vr;
	x[3] = (ui  /2) + vi;
	x[10] = (ur /2) - vr;
	x[11] = (ui /2) - vi;

	ur = y[14];
	ui = y[15];
	vr = fMultDiv2(ui - ur, W_PiFOURTH);
	vi = fMultDiv2(ui + ur, W_PiFOURTH);
	ur = y[6];
	ui = y[7];
	x[6] = (ur  /2) + vr;
	x[7] = (ui  /2) - vi;
	x[14] = (ur /2) - vr;
	x[15] = (ui /2) + vi;
}
#endif


#ifdef QL_FFT_15

#define N3                    3
#define N5                    5
#define N6                    6
#define N15                   15
/* Performs the FFT of length 15. It is split into FFTs of length 3 and length 5. */
static inline void fft15(fft_in_sample_t *pInput)
{
	fft_in_sample_t  aDst[2 * N15];
	fft_in_sample_t  aDst1[2 * N15];
	int    i, k, l;

	/* Sort input vector for fft's of length 3
	input3(0:2)   = [input(0) input(5) input(10)];
	input3(3:5)   = [input(3) input(8) input(13)];
	input3(6:8)   = [input(6) input(11) input(1)];
	input3(9:11)  = [input(9) input(14) input(4)];
	input3(12:14) = [input(12) input(2) input(7)]; */
	{
		const fft_in_sample_t *pSrc = pInput;
		fft_in_sample_t * pDst = aDst;
		/* Merge 3 loops into one, skip call of fft3 */
		for (i = 0, l = 0, k = 0; i<N5; i++, k += 6)
		{
			pDst[k + 0] = pSrc[l];
			pDst[k + 1] = pSrc[l + 1];
			l += 2 * N5;
			if (l >= (2 * N15))
				l -= (2 * N15);

			pDst[k + 2] = pSrc[l];
			pDst[k + 3] = pSrc[l + 1];
			l += 2 * N5;
			if (l >= (2 * N15))
				l -= (2 * N15);
			pDst[k + 4] = pSrc[l];
			pDst[k + 5] = pSrc[l + 1];
			l += (2 * N5) + (2 * N3);
			if (l >= (2 * N15))
				l -= (2 * N15);

			/* fft3 merged with shift right by 2 loop */
			fft_in_sample_t r1, r2, r3;
			fft_in_sample_t s1, s2;
			/* real part */
			r1 = pDst[k + 2] + pDst[k + 4];
            r2 = fMult((pDst[k + 2] - pDst[k + 4]), W_PiTHIRD);
			s1 = pDst[k + 0];
            pDst[k + 0] = AddDiv4(s1 , r1); // >> 2;
            r1 = s1 - Div2(r1); //  >> 1);

			/* imaginary part */
			s1 = pDst[k + 3] + pDst[k + 5];
            s2 = fMult((pDst[k + 3] - pDst[k + 5]), W_PiTHIRD);
			r3 = pDst[k + 1];
            pDst[k + 1] = AddDiv4(r3 , s1); // >> 2;
            s1 = r3 - Div2(s1); // >> 1);

			/* combination */
            pDst[k + 2] = SubDiv4(r1 , s2); // >> 2;
            pDst[k + 4] = AddDiv4(r1 , s2); // >> 2;
            pDst[k + 3] = AddDiv4(s1 , r2); // >> 2;
            pDst[k + 5] = SubDiv4(s1 , r2); //  >> 2;
		}
	}
	/* Sort input vector for fft's of length 5
	input5(0:4)   = [output3(0) output3(3) output3(6) output3(9) output3(12)];
	input5(5:9)   = [output3(1) output3(4) output3(7) output3(10) output3(13)];
	input5(10:14) = [output3(2) output3(5) output3(8) output3(11) output3(14)]; */
	/* Merge 2 loops into one, brings about 10% */
	{
		const fft_in_sample_t *pSrc = aDst;
		fft_in_sample_t * pDst = aDst1;
		for (i = 0, l = 0, k = 0; i<N3; i++, k += 10)
		{
			l = 2 * i;
			pDst[k + 0] = pSrc[l + 0];
			pDst[k + 1] = pSrc[l + 1];
			pDst[k + 2] = pSrc[l + 0 + (2 * N3)];
			pDst[k + 3] = pSrc[l + 1 + (2 * N3)];
			pDst[k + 4] = pSrc[l + 0 + (4 * N3)];
			pDst[k + 5] = pSrc[l + 1 + (4 * N3)];
			pDst[k + 6] = pSrc[l + 0 + (6 * N3)];
			pDst[k + 7] = pSrc[l + 1 + (6 * N3)];
			pDst[k + 8] = pSrc[l + 0 + (8 * N3)];
			pDst[k + 9] = pSrc[l + 1 + (8 * N3)];
			fft5(&pDst[k]);
		}
	}
	/* Sort output vector of length 15
	output = [out5(0)  out5(6)  out5(12) out5(3)  out5(9)
	out5(10) out5(1)  out5(7)  out5(13) out5(4)
	out5(5)  out5(11) out5(2)  out5(8)  out5(14)]; */
	/* optimize clumsy loop, brings about 5% */
	{
		const fft_in_sample_t *pSrc = aDst1;
		fft_in_sample_t * pDst = pInput;
		for (i = 0, l = 0, k = 0; i<N3; i++, k += 10)
		{
			pDst[k + 0] = pSrc[l];
			pDst[k + 1] = pSrc[l + 1];
			l += (2 * N6);
			if (l >= (2 * N15))
				l -= (2 * N15);
			pDst[k + 2] = pSrc[l];
			pDst[k + 3] = pSrc[l + 1];
			l += (2 * N6);
			if (l >= (2 * N15))
				l -= (2 * N15);
			pDst[k + 4] = pSrc[l];
			pDst[k + 5] = pSrc[l + 1];
			l += (2 * N6);
			if (l >= (2 * N15))
				l -= (2 * N15);
			pDst[k + 6] = pSrc[l];
			pDst[k + 7] = pSrc[l + 1];
			l += (2 * N6);
			if (l >= (2 * N15))
				l -= (2 * N15);
			pDst[k + 8] = pSrc[l];
			pDst[k + 9] = pSrc[l + 1];
			l += 2;    /* no modulo check needed, it cannot occur */
		}
	}
}

#endif


#define FFT_TWO_STAGE_MACRO_ENABLE

#ifdef FFT_TWO_STAGE_MACRO_ENABLE
/* FDK_ASSERT(length == dim1*dim2); \ */

typedef void(*fp_fft_leaf_t)(ql_sample_in_t *pData);

static inline void fft_apply_rot_vector(fft_in_sample_t * pData, const int cl, const int l, const rota_coeff_t *pVecRe, const rota_coeff_t *pVecIm);
#define MEM_CPY_STRIDE(pSrc_, pDst_, src_stride, dst_stride, n)         {\
for(int k_ = 0,  j_= 0; j_<n; j_+=dst_stride, k_ += src_stride) \
    { \
        pDst_[j_] = pSrc_[k_]; \
        pDst_[j_ + 1] = pSrc_[k_ + 1]; \
    } \
}

// loop count is based on src_stride 
#define MEM_CPY_STRIDE_S(pSrc_, pDst_, src_stride, dst_stride, n)         {\
for(int k_ = 0,  j_= 0; k_<n; j_+=dst_stride, k_ += src_stride) \
    { \
        pDst_[j_] = pSrc_[k_]; \
        pDst_[j_ + 1] = pSrc_[k_ + 1]; \
    } \
}



int fftN2(fft_in_sample_t *pInput, int length, int dim1, int dim2, fp_fft_leaf_t fft_func1, fp_fft_leaf_t  fft_func2, const rota_coeff_t *RotVectorReal, const rota_coeff_t *RotVectorImag)
{ 
  int       i; 
  int dim1x2 = 2 * dim1;
  int dim2x2 = 2 * dim2;
 
  VARDECL( fft_in_sample_t, aDst); 
  VARDECL(fft_in_sample_t, aDst2); 
  ALLOC(aDst, length*2, fft_in_sample_t);
  ALLOC(aDst2,  dim2x2, fft_in_sample_t);

 
  /* Perform dim2 times the fft of length dim1. The input samples are at the address of pSrc and the \
    output samples are at the address of pDst. The input vector for the fft of length dim1 is built \
	  of the interleaved samples in pSrc, the output samples are stored consecutively. \
	    */ 
  { 
    const fft_in_sample_t* pSrc = pInput; 
    fft_in_sample_t  *pDst = aDst; 
    
    for(i=0; i<dim2; i++) 
    { 
      MEM_CPY_STRIDE(pSrc, pDst, dim2x2, 2, dim1x2);
      fft_func1(pDst); 
      pSrc += 2; 
      pDst = pDst + dim1x2; 
    } 
  } 
  
  /* Perform the modulation of the output of the fft of length dim1 */ 
  fft_apply_rot_vector(aDst, dim1, length, RotVectorReal, RotVectorImag); 
  
  /* Perform dim1 times the fft of length dim2. The input samples are at the address of aDst and the 
    output samples are at the address of pInput. The input vector for the fft of length dim2 is built 
	  of the interleaved samples in aDst, the output samples are stored consecutively at the address 
	    of pInput. 
		  */ 
  { 
    const fft_in_sample_t* pSrc       = aDst; 
    fft_in_sample_t *pDst    = aDst2; 
    fft_in_sample_t *pDstOut = pInput; 
    
    for(i=0; i<dim1; i++) 
    { 
      MEM_CPY_STRIDE(pSrc, pDst, dim1x2, 2, dim2x2);
      fft_func2(pDst); 
      MEM_CPY_STRIDE_S(pDst, pDstOut, 2, dim1x2, dim2x2);
      pSrc += 2; 
      pDstOut += 2; 
    } 
  } 
  
  FREE1(aDst2, dim2x2, fft_in_sample_t);
  FREE1(aDst, length*2, fft_in_sample_t);
  return 0;
} 

#endif

//#define W_PiFOURTH STC(0x5a82799a)
#ifndef SUMDIFF_PIFOURTH
#ifndef USE_ASM
#define SUMDIFF_PIFOURTH(diff,sum,a,b) \
  { \
    fft_in_sample_t wa, wb;\
    wa = fMultDiv2(a, W_PiFOURTH);\
    wb = fMultDiv2(b, W_PiFOURTH);\
    diff = wb - wa;\
    sum  = wb + wa;\
  }
#else
#pragma inline = forced 
void sumdiff_pifourth(fract32 a, fract32 b, fract32 *diff, fract32 *sum)
{
  fract32 c = W_PiFOURTH;
  fract32 s1,s2;
  fract32 rah, ral;
  fract32 rbh, rbl;
   __asm("smull %0, %1, %2, %3\n\t" : "=r"(ral), "=r"(rah) : "r"(a), "r"(c) ); 
   __asm("smull %0, %1, %2, %3\n\t" : "=r"(rbl), "=r"(rbh) : "r"(b), "r"(c) ); 
   
   __asm(  "add %0, %1, %2\n\t" : "=r"(s1)  : "r"(rah), "r"(rbh));
   __asm( "lsl %0, %1, #11\n\t" : "=r"(s1)  : "r"(s1)  );
   __asm(  "add %0, %1, %2, lsr #21\n\t" : "=r"(s1)  : "r"(s1), "r"(ral));
   __asm(  "add %0, %1, %2, lsr #21\n\t" : "=r"(s1)  : "r"(s1), "r"(rbl));
   //*sum = s1;
   
   __asm(  "sub %0, %1, %2\n\t" : "=r"(s2)  : "r"(rbh), "r"(rah));
   __asm( "lsl %0, %1, #11\n\t" : "=r"(s2)  : "r"(s2)  );
   __asm(  "add %0, %1, %2, lsr #21\n\t" : "=r"(s2)  : "r"(s2), "r"(rbl));
   __asm(  "sub %0, %1, %2, lsr #21\n\t" : "=r"(s2)  : "r"(s2), "r"(ral));

    *sum = s1;
   *diff = s2;
  
}
#define SUMDIFF_PIFOURTH(diff,sum,a,b)  sumdiff_pifourth(a, b, &diff, &sum)
#endif
#endif


#define USE_MACROS_A1

#ifdef USE_MACROS_A1
#if WIN32
#define cplxMultDiv2(ReO, ImO, a_Re, a_Im, b_Re, b_Im)     ReO =(fMult(a_Re,b_Re)- fMult(a_Im, b_Im))/2; \
                                                        ImO = (fMult(a_Re, b_Im) + fMult(a_Im , b_Re)) / 2;
#else
#include "basicOps.h"
#define cplxMultDiv2(ReO, ImO, a_Re, a_Im, b_Re, b_Im)        ql_cplxmultdiv2(a_Re, a_Im, b_Re, b_Im, &ReO, &ImO)

#endif



#define cplxMultRotaDiv4(ReO, ImO, a_Re, a_Im, b_Re, b_Im)     ReO =(fMultRota(a_Re, b_Re)- fMultRota(a_Im, b_Im))/4; \
                                                        ImO = (fMultRota(a_Re, b_Im) + fMultRota(a_Im , b_Re)) / 4;

#else
void cplxMultDiv2(fft_in_sample_t *c_Re,
	fft_in_sample_t *c_Im,
	const fft_in_sample_t  a_Re,
	const fft_in_sample_t  a_Im,
	const fft_in_sample_t  b_Re,
	const fft_in_sample_t  b_Im)
{
#if 0  
	*c_Re = fMultDiv2(a_Re, b_Re) - fMultDiv2(a_Im, b_Im);
	*c_Im = fMultDiv2(a_Re, b_Im) + fMultDiv2(a_Im, b_Re);
#else    
	*c_Re =(a_Re*b_Re- a_Im* b_Im)/2;
	*c_Im =(a_Re*b_Im+ a_Im* b_Re)/2;
#endif    
}
#endif // USE_MACROS_A1
#ifdef QL_FFT_16

ql_complex_t fft16_w16_0 = {
    .re = 0.923879533f,
    .im = 0.382683432f
};
ql_complex_t fft16_w16_1 = {  // conjugate of the above !. 
    .re = 0.382683432f,
    .im = 0.923879533f
};
/* This version is more overflow save, but less cycle optimal. */
#define SUMDIFF_EIGTH(x, y, ix, iy, vr, vi, ur, ui) \
  vr = ((x[ 0 + ix]) + (x[16 + ix]))/2;  /* Re A + Re B */ \
  vi = ((x[ 8 + ix]) + (x[24 + ix]))/2;     /* Re C + Re D */ \
  ur = ((x[ 1 + ix]) + (x[17 + ix]))/2;  /* Im A + Im B */ \
  ui = ((x[ 9 + ix]) + (x[25 + ix]))/2;     /* Im C + Im D */ \
  y[ 0 + iy] = vr + vi;     /* Re A' = ReA + ReB +ReC + ReD */    \
  y[ 4 + iy] = vr - vi;     /* Re C' = -(ReC+ReD) + (ReA+ReB) */  \
  y[ 1 + iy] = ur + ui;     /* Im A' = sum of imag values */      \
  y[ 5 + iy] = ur - ui;     /* Im C' = -Im C -Im D +Im A +Im B */ \
  vr -= x[16 + ix];              /* Re A - Re B */ \
  vi = vi - x[24 + ix];          /* Re C - Re D */ \
  ur -= x[17 + ix];              /* Im A - Im B */ \
  ui = ui - x[25 + ix];          /* Im C - Im D */ \
  y[ 2 + iy] = ui + vr;          /* Re B' = Im C - Im D  + Re A - Re B */ \
  y[ 6 + iy] = vr - ui;          /* Re D' = -Im C + Im D + Re A - Re B */ \
  y[ 3 + iy] = ur - vi;          /* Im B'= -Re C + Re D + Im A - Im B */  \
  y[ 7 + iy] = vi + ur;          /* Im D'= Re C - Re D + Im A - Im B */


void fft_16(fft_in_sample_t *x)
{
	fft_in_sample_t vr, vi, ur, ui;
	fft_in_sample_t y[32];

	SUMDIFF_EIGTH(x, y, 0, 0, vr, vi, ur, ui);
	SUMDIFF_EIGTH(x, y, 4, 8, vr, vi, ur, ui);
	SUMDIFF_EIGTH(x, y, 2, 16, vr, vi, ur, ui);
	SUMDIFF_EIGTH(x, y, 6, 24, vr, vi, ur, ui);

	// xt1 =  0
	// xt2 =  8
	vr = y[8];
	vi = y[9];
	ur = y[0] /2;
	ui = y[1] /2;
	x[0] = ur + (vr /2);
	x[1] = ui + (vi /2);
	x[8] = ur - (vr /2);
	x[9] = ui - (vi /2);

	// xt1 =  4
	// xt2 = 12
	vr = y[13];
	vi = y[12];
	ur = y[4] /2;
	ui = y[5] /2;
	x[4] = ur + (vr  /2);
	x[5] = ui - (vi  /2);
	x[12] = ur - (vr /2);
	x[13] = ui + (vi /2);

	// xt1 = 16
	// xt2 = 24
	vr = y[24];
	vi = y[25];
	ur = y[16] /2;
	ui = y[17] /2;
	x[16] = ur + (vr /2);
	x[17] = ui + (vi /2);
	x[24] = ur - (vr /2);
	x[25] = ui - (vi /2);

	// xt1 = 20
	// xt2 = 28
	vr = y[29];
	vi = y[28];
	ur = y[20] /2;
	ui = y[21] /2;
	x[20] = ur + (vr /2);
	x[21] = ui - (vi /2);
	x[28] = ur - (vr /2);
	x[29] = ui + (vi /2);

	// xt1 =  2
	// xt2 = 10
	SUMDIFF_PIFOURTH(vi, vr, y[10], y[11])
		//vr = fMultDiv2((y[11] + y[10]),W_PiFOURTH);
		//vi = fMultDiv2((y[11] - y[10]),W_PiFOURTH);
		ur = y[2];
	ui = y[3];
	x[2] = (ur  /2) + vr;
	x[3] = (ui  /2) + vi;
	x[10] = (ur /2) - vr;
	x[11] = (ui /2) - vi;

	// xt1 =  6
	// xt2 = 14
	SUMDIFF_PIFOURTH(vr, vi, y[14], y[15])
		ur = y[6];
	ui = y[7];
	x[6] = (ur  /2) + vr;
	x[7] = (ui  /2) - vi;
	x[14] = (ur /2) - vr;
	x[15] = (ui /2) + vi;

	// xt1 = 18
	// xt2 = 26
	SUMDIFF_PIFOURTH(vi, vr, y[26], y[27])
		ur = y[18];
	ui = y[19];
	x[18] = (ur /2) + vr;
	x[19] = (ui /2) + vi;
	x[26] = (ur /2) - vr;
	x[27] = (ui /2) - vi;

	// xt1 = 22
	// xt2 = 30
	SUMDIFF_PIFOURTH(vr, vi, y[30], y[31])
		ur = y[22];
	ui = y[23];
	x[22] = (ur /2) + vr;
	x[23] = (ui /2) - vi;
	x[30] = (ur /2) - vr;
	x[31] = (ui /2) + vi;

	// xt1 =  0
	// xt2 = 16
	vr = x[16];
	vi = x[17];
	ur = x[0] /2;
	ui = x[1] /2;
	x[0] = ur + (vr /2);
	x[1] = ui + (vi /2);
	x[16] = ur - (vr/2);
	x[17] = ui - (vi/2);

	// xt1 =  8
	// xt2 = 24
	vi = x[24];
	vr = x[25];
	ur = x[8] /2;
	ui = x[9] /2;
	x[8] = ur + (vr /2);
	x[9] = ui - (vi /2);
	x[24] = ur - (vr/2);
	x[25] = ui + (vi/2);

	// xt1 =  2
	// xt2 = 18
	cplxMultDiv2(vi, vr, x[19], x[18], fft16_w16_0.re, fft16_w16_0.im);
	ur = x[2];
	ui = x[3];
	x[2] = (ur  /2) + vr;
	x[3] = (ui  /2) + vi;
	x[18] = (ur /2) - vr;
	x[19] = (ui /2) - vi;

	// xt1 = 10
	// xt2 = 26
	cplxMultDiv2(vr, vi, x[27], x[26], fft16_w16_0.re, fft16_w16_0.im);
	ur = x[10];
	ui = x[11];
	x[10] = (ur /2) + vr;
	x[11] = (ui /2) - vi;
	x[26] = (ur /2) - vr;
	x[27] = (ui /2) + vi;

	// xt1 =  4
	// xt2 = 20
	SUMDIFF_PIFOURTH(vi, vr, x[20], x[21])
		ur = x[4];
	ui = x[5];
	x[4] = (ur  /2) + vr;
	x[5] = (ui  /2) + vi;
	x[20] = (ur /2) - vr;
	x[21] = (ui /2) - vi;

	// xt1 = 12
	// xt2 = 28
	SUMDIFF_PIFOURTH(vr, vi, x[28], x[29])
		ur = x[12];
	ui = x[13];
	x[12] = (ur /2) + vr;
	x[13] = (ui /2) - vi;
	x[28] = (ur /2) - vr;
	x[29] = (ui /2) + vi;

	// xt1 =  6
	// xt2 = 22
	cplxMultDiv2(vi, vr, x[23], x[22], fft16_w16_1.re, fft16_w16_1.im);
	ur = x[6];
	ui = x[7];
	x[6] = (ur /2) + vr;
	x[7] = (ui /2) + vi;
	x[22] = (ur/2) - vr;
	x[23] = (ui/2) - vi;

	// xt1 = 14
	// xt2 = 30
	cplxMultDiv2(vr, vi, x[31], x[30], fft16_w16_1.re, fft16_w16_1.im);
	ur = x[14];
	ui = x[15];
	x[14] = (ur /2) + vr;
	x[15] = (ui /2) - vi;
	x[30] = (ur /2) - vr;
	x[31] = (ui /2) + vi;
}

#endif
#ifdef QL_FFT_32

static ql_complex_fix_t fft32_w32[6] =
{
	{STC(0x7641af3d), STC(0x30fbc54d)}, {STC(0x30fbc54d), STC(0x7641af3d)}, {STC(0x7d8a5f40), STC(0x18f8b83c) },
	{STC(0x6a6d98a4), STC(0x471cece7)}, {STC(0x471cece7), STC(0x6a6d98a4)}, {STC(0x18f8b83c), STC(0x7d8a5f40)}
};
#ifdef REPLACE
// followinf sh is used to repalce the macro.
sed 's/vr + QL_B2(vi);/ql_macro_vrPvi2(vr, vi);\/\/vr + QL_B2(vi);/g' fft_32.c > t1.c
sed 's/vr - QL_B2(vi);/ql_macro_vrMvi2(vr, vi);\/\/vr - QL_B2(vi);/g' t1.c > t2.c
sed 's/ur + QL_B2(ui);/ql_macro_urPui2(ur, ui);\/\/ur + QL_B2(ui);/g' t2.c > t3.c
sed 's/ur - QL_B2(ui);/ql_macro_urMui2(ur, ui);\/\/ur - QL_B2(ui);/g' t3.c > t4.c
sed 's/ur + QL_B2(vr);/ql_macro_urPvr2(ur, vr);\/\/ur + QL_B2(vr);/g' t4.c > t5.c
sed 's/ur - QL_B2(vr);/ql_macro_urMvr2(ur, vr);\/\/ur - QL_B2(vr);/g' t5.c > t6.c
sed 's/ui + QL_B2(vi);/ql_macro_uiPvi2(ui, vi);\/\/ui + QL_B2(vi);/g' t6.c > t7.c
sed 's/ui - QL_B2(vi);/ql_macro_uiMvi2(ui, vi);\/\/ui - QL_B2(vi);/g' t7.c > tx.c


sed 's/QL_B2(ur) + vr;/ql_macro_ur2Pvr(ur, vr);\/\/\/QL_B2(ur) + vr;/g' tx.c > t11.c
sed 's/QL_B2(ui) + vi;/ql_macro_ui2Pvi(ui, vi);\/\/\/QL_B2(ui) + vi;/g' t11.c > t12.c
sed 's/QL_B2(ur) - vr;/ql_macro_ur2Mvr(ur, vr);\/\/\/QL_B2(ur) - vr;/g' t12.c > t13.c
sed 's/QL_B2(ui) - vi;/ql_macro_ui2Mvi(ui, vi);\/\/\/QL_B2(ui) - vi;/g' t13.c > ty.c

#endif
 
#define ql_macro_vrPvi2(vr, vi) ql_macro_a_plus_b_by2(vr, vi)
#define ql_macro_vrMvi2(vr, vi) ql_macro_a_minus_b_by2(vr, vi)
#define ql_macro_urPui2(ur, ui) ql_macro_a_plus_b_by2(ur, ui)
#define ql_macro_urMui2(ur, ui) ql_macro_a_minus_b_by2(ur, ui)
#define ql_macro_urPvr2(ur, vr) ql_macro_a_plus_b_by2(ur, vr)
#define ql_macro_urMvr2(ur, vr) ql_macro_a_minus_b_by2(ur, vr)
#define ql_macro_uiPvi2(ui, vi) ql_macro_a_plus_b_by2(ui, vi)
#define ql_macro_uiMvi2(ui, vi) ql_macro_a_minus_b_by2(ui, vi)


#define ql_macro_ur2Pvr(ur, vr) ql_macro_aby2_plus_b(ur, vr)
#define ql_macro_ui2Pvi(ui, vi) ql_macro_aby2_plus_b(ui, vi)
#define ql_macro_ur2Mvr(ur, vr) ql_macro_aby2_minus_b(ur, vr)
#define ql_macro_ui2Mvi(ui, vi) ql_macro_aby2_minus_b(ui, vi)


void fft_32(fft_in_sample_t *x)
{

    //#define W_PiFOURTH STC(0x5a82799a)

    fft_in_sample_t vr, vi, ur, ui;
    fft_in_sample_t y[64];
    //float tw = STC(0x5a82799a);
    /*
    * 1+2 stage radix 4
    */

    /////////////////////////////////////////////////////////////////////////////////////////

    // i = 0
    vr = QL_B2(x[0] + x[32]); /* Re A + Re B */
    vi = (x[16] + x[48]); /* Re C + Re D */
    ur = QL_B2(x[1] + x[33]); /* Im A + Im B */
    ui = (x[17] + x[49]); /* Im C + Im D */

    y[0] = ql_macro_vrPvi2(vr, vi);//vr + QL_B2(vi); /* Re A' = ReA + ReB +ReC + ReD */
    y[4] = ql_macro_vrMvi2(vr, vi);//vr - QL_B2(vi); /* Re C' = -(ReC+ReD) + (ReA+ReB) */
    y[1] = ql_macro_urPui2(ur, ui);//ur + QL_B2(ui); /* Im A' = sum of imag values */
    y[5] = ql_macro_urMui2(ur, ui);//ur - QL_B2(ui); /* Im C' = -Im C -Im D +Im A +Im B */

    vr -= x[32]; /* Re A - Re B */
    vi = QL_B2(vi) - x[48]; /* Re C - Re D */
    ur -= x[33]; /* Im A - Im B */
    ui = QL_B2(ui) - x[49]; /* Im C - Im D */

    y[2] = ui + vr; /* Re B' = Im C - Im D + Re A - Re B */
    y[6] = vr - ui; /* Re D' = -Im C + Im D + Re A - Re B */
    y[3] = ur - vi; /* Im B'= -Re C + Re D + Im A - Im B */
    y[7] = vi + ur; /* Im D'= Re C - Re D + Im A - Im B */

                    //i=8
    vr = QL_B2(x[8] + x[40]); /* Re A + Re B */
    vi = (x[24] + x[56]); /* Re C + Re D */
    ur = QL_B2(x[9] + x[41]); /* Im A + Im B */
    ui = (x[25] + x[57]); /* Im C + Im D */

    y[8] = ql_macro_vrPvi2(vr, vi);//vr + QL_B2(vi); /* Re A' = ReA + ReB +ReC + ReD */
    y[12] = ql_macro_vrMvi2(vr, vi);//vr - QL_B2(vi); /* Re C' = -(ReC+ReD) + (ReA+ReB) */
    y[9] = ql_macro_urPui2(ur, ui);//ur + QL_B2(ui); /* Im A' = sum of imag values */
    y[13] = ql_macro_urMui2(ur, ui);//ur - QL_B2(ui); /* Im C' = -Im C -Im D +Im A +Im B */

    vr -= x[40]; /* Re A - Re B */
    vi = QL_B2(vi) - x[56]; /* Re C - Re D */
    ur -= x[41]; /* Im A - Im B */
    ui = QL_B2(ui) - x[57]; /* Im C - Im D */

    y[10] = ui + vr; /* Re B' = Im C - Im D + Re A - Re B */
    y[14] = vr - ui; /* Re D' = -Im C + Im D + Re A - Re B */
    y[11] = ur - vi; /* Im B'= -Re C + Re D + Im A - Im B */
    y[15] = vi + ur; /* Im D'= Re C - Re D + Im A - Im B */

                     //i=16
    vr = QL_B2(x[4] + x[36]); /* Re A + Re B */
    vi = (x[20] + x[52]); /* Re C + Re D */
    ur = QL_B2(x[5] + x[37]); /* Im A + Im B */
    ui = (x[21] + x[53]); /* Im C + Im D */

    y[16] = ql_macro_vrPvi2(vr, vi);//vr + QL_B2(vi); /* Re A' = ReA + ReB +ReC + ReD */
    y[20] = ql_macro_vrMvi2(vr, vi);//vr - QL_B2(vi); /* Re C' = -(ReC+ReD) + (ReA+ReB) */
    y[17] = ql_macro_urPui2(ur, ui);//ur + QL_B2(ui); /* Im A' = sum of imag values */
    y[21] = ql_macro_urMui2(ur, ui);//ur - QL_B2(ui); /* Im C' = -Im C -Im D +Im A +Im B */

    vr -= x[36]; /* Re A - Re B */
    vi = QL_B2(vi) - x[52]; /* Re C - Re D */
    ur -= x[37]; /* Im A - Im B */
    ui = QL_B2(ui) - x[53]; /* Im C - Im D */

    y[18] = ui + vr; /* Re B' = Im C - Im D + Re A - Re B */
    y[22] = vr - ui; /* Re D' = -Im C + Im D + Re A - Re B */
    y[19] = ur - vi; /* Im B'= -Re C + Re D + Im A - Im B */
    y[23] = vi + ur; /* Im D'= Re C - Re D + Im A - Im B */

                     //i=24
    vr = QL_B2(x[12] + x[44]); /* Re A + Re B */
    vi = (x[28] + x[60]); /* Re C + Re D */
    ur = QL_B2(x[13] + x[45]); /* Im A + Im B */
    ui = (x[29] + x[61]); /* Im C + Im D */

    y[24] = ql_macro_vrPvi2(vr, vi);//vr + QL_B2(vi); /* Re A' = ReA + ReB +ReC + ReD */
    y[28] = ql_macro_vrMvi2(vr, vi);//vr - QL_B2(vi); /* Re C' = -(ReC+ReD) + (ReA+ReB) */
    y[25] = ql_macro_urPui2(ur, ui);//ur + QL_B2(ui); /* Im A' = sum of imag values */
    y[29] = ql_macro_urMui2(ur, ui);//ur - QL_B2(ui); /* Im C' = -Im C -Im D +Im A +Im B */

    vr -= x[44]; /* Re A - Re B */
    vi = QL_B2(vi) - x[60]; /* Re C - Re D */
    ur -= x[45]; /* Im A - Im B */
    ui = QL_B2(ui) - x[61]; /* Im C - Im D */

    y[26] = ui + vr; /* Re B' = Im C - Im D + Re A - Re B */
    y[30] = vr - ui; /* Re D' = -Im C + Im D + Re A - Re B */
    y[27] = ur - vi; /* Im B'= -Re C + Re D + Im A - Im B */
    y[31] = vi + ur; /* Im D'= Re C - Re D + Im A - Im B */

                     // i = 32
    vr = QL_B2(x[2] + x[34]); /* Re A + Re B */
    vi = (x[18] + x[50]); /* Re C + Re D */
    ur = QL_B2(x[3] + x[35]); /* Im A + Im B */
    ui = (x[19] + x[51]); /* Im C + Im D */

    y[32] = ql_macro_vrPvi2(vr, vi);//vr + QL_B2(vi); /* Re A' = ReA + ReB +ReC + ReD */
    y[36] = ql_macro_vrMvi2(vr, vi);//vr - QL_B2(vi); /* Re C' = -(ReC+ReD) + (ReA+ReB) */
    y[33] = ql_macro_urPui2(ur, ui);//ur + QL_B2(ui); /* Im A' = sum of imag values */
    y[37] = ql_macro_urMui2(ur, ui);//ur - QL_B2(ui); /* Im C' = -Im C -Im D +Im A +Im B */

    vr -= x[34]; /* Re A - Re B */
    vi = QL_B2(vi) - x[50]; /* Re C - Re D */
    ur -= x[35]; /* Im A - Im B */
    ui = QL_B2(ui) - x[51]; /* Im C - Im D */

    y[34] = ui + vr; /* Re B' = Im C - Im D + Re A - Re B */
    y[38] = vr - ui; /* Re D' = -Im C + Im D + Re A - Re B */
    y[35] = ur - vi; /* Im B'= -Re C + Re D + Im A - Im B */
    y[39] = vi + ur; /* Im D'= Re C - Re D + Im A - Im B */

                     //i=40
    vr = QL_B2(x[10] + x[42]); /* Re A + Re B */
    vi = (x[26] + x[58]); /* Re C + Re D */
    ur = QL_B2(x[11] + x[43]); /* Im A + Im B */
    ui = (x[27] + x[59]); /* Im C + Im D */

    y[40] = ql_macro_vrPvi2(vr, vi);//vr + QL_B2(vi); /* Re A' = ReA + ReB +ReC + ReD */
    y[44] = ql_macro_vrMvi2(vr, vi);//vr - QL_B2(vi); /* Re C' = -(ReC+ReD) + (ReA+ReB) */
    y[41] = ql_macro_urPui2(ur, ui);//ur + QL_B2(ui); /* Im A' = sum of imag values */
    y[45] = ql_macro_urMui2(ur, ui);//ur - QL_B2(ui); /* Im C' = -Im C -Im D +Im A +Im B */

    vr -= x[42]; /* Re A - Re B */
    vi = QL_B2(vi) - x[58]; /* Re C - Re D */
    ur -= x[43]; /* Im A - Im B */
    ui = QL_B2(ui) - x[59]; /* Im C - Im D */

    y[42] = ui + vr; /* Re B' = Im C - Im D + Re A - Re B */
    y[46] = vr - ui; /* Re D' = -Im C + Im D + Re A - Re B */
    y[43] = ur - vi; /* Im B'= -Re C + Re D + Im A - Im B */
    y[47] = vi + ur; /* Im D'= Re C - Re D + Im A - Im B */

                     //i=48
    vr = QL_B2(x[6] + x[38]); /* Re A + Re B */
    vi = (x[22] + x[54]); /* Re C + Re D */
    ur = QL_B2(x[7] + x[39]); /* Im A + Im B */
    ui = (x[23] + x[55]); /* Im C + Im D */

    y[48] = ql_macro_vrPvi2(vr, vi);//vr + QL_B2(vi); /* Re A' = ReA + ReB +ReC + ReD */
    y[52] = ql_macro_vrMvi2(vr, vi);//vr - QL_B2(vi); /* Re C' = -(ReC+ReD) + (ReA+ReB) */
    y[49] = ql_macro_urPui2(ur, ui);//ur + QL_B2(ui); /* Im A' = sum of imag values */
    y[53] = ql_macro_urMui2(ur, ui);//ur - QL_B2(ui); /* Im C' = -Im C -Im D +Im A +Im B */

    vr -= x[38]; /* Re A - Re B */
    vi = QL_B2(vi) - x[54]; /* Re C - Re D */
    ur -= x[39]; /* Im A - Im B */
    ui = QL_B2(ui) - x[55]; /* Im C - Im D */

    y[50] = ui + vr; /* Re B' = Im C - Im D + Re A - Re B */
    y[54] = vr - ui; /* Re D' = -Im C + Im D + Re A - Re B */
    y[51] = ur - vi; /* Im B'= -Re C + Re D + Im A - Im B */
    y[55] = vi + ur; /* Im D'= Re C - Re D + Im A - Im B */

                     //i=56
    vr = QL_B2(x[14] + x[46]); /* Re A + Re B */
    vi = (x[30] + x[62]); /* Re C + Re D */
    ur = QL_B2(x[15] + x[47]); /* Im A + Im B */
    ui = (x[31] + x[63]); /* Im C + Im D */

    y[56] = ql_macro_vrPvi2(vr, vi);//vr + QL_B2(vi); /* Re A' = ReA + ReB +ReC + ReD */
    y[60] = ql_macro_vrMvi2(vr, vi);//vr - QL_B2(vi); /* Re C' = -(ReC+ReD) + (ReA+ReB) */
    y[57] = ql_macro_urPui2(ur, ui);//ur + QL_B2(ui); /* Im A' = sum of imag values */
    y[61] = ql_macro_urMui2(ur, ui);//ur - QL_B2(ui); /* Im C' = -Im C -Im D +Im A +Im B */

    vr -= x[46]; /* Re A - Re B */
    vi = QL_B2(vi) - x[62]; /* Re C - Re D */
    ur -= x[47]; /* Im A - Im B */
    ui = QL_B2(ui) - x[63]; /* Im C - Im D */

    y[58] = ui + vr; /* Re B' = Im C - Im D + Re A - Re B */
    y[62] = vr - ui; /* Re D' = -Im C + Im D + Re A - Re B */
    y[59] = ur - vi; /* Im B'= -Re C + Re D + Im A - Im B */
    y[63] = vi + ur; /* Im D'= Re C - Re D + Im A - Im B */


    fft_in_sample_t *xt = &x[0];
    fft_in_sample_t *yt = &y[0];

    int j = 4;
    do
    {
        vr = yt[8];
        vi = yt[9];
        ur = QL_B2(yt[0]);
        ui = QL_B2(yt[1]);
        xt[0] = ql_macro_urPvr2(ur, vr);//ur + QL_B2(vr);
        xt[1] = ql_macro_uiPvi2(ui, vi);//ui + QL_B2(vi);
        xt[8] = ql_macro_urMvr2(ur, vr);//ur - QL_B2(vr);
        xt[9] = ql_macro_uiMvi2(ui, vi);//ui - QL_B2(vi);

        vr = yt[13];
        vi = yt[12];
        ur = QL_B2(yt[4]);
        ui = QL_B2(yt[5]);
        xt[4] = ql_macro_urPvr2(ur, vr);//ur + QL_B2(vr);
        xt[5] = ql_macro_uiMvi2(ui, vi);//ui - QL_B2(vi);
        xt[12] = ql_macro_urMvr2(ur, vr);//ur - QL_B2(vr);
        xt[13] = ql_macro_uiPvi2(ui, vi);//ui + QL_B2(vi);

        SUMDIFF_PIFOURTH(vi, vr, yt[10], yt[11]);
        ur = yt[2];
        ui = yt[3];
        xt[2] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
        xt[3] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;
        xt[10] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
        xt[11] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;

        SUMDIFF_PIFOURTH(vr, vi, yt[14], yt[15]);
        ur = yt[6];
        ui = yt[7];

        xt[6] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
        xt[7] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;
        xt[14] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
        xt[15] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;
        xt += 16;
        yt += 16;
    } while (--j != 0);

    vr = x[16];
    vi = x[17];
    ur = QL_B2(x[0]);
    ui = QL_B2(x[1]);
    x[0] = ql_macro_urPvr2(ur, vr);//ur + QL_B2(vr);
    x[1] = ql_macro_uiPvi2(ui, vi);//ui + QL_B2(vi);
    x[16] = ql_macro_urMvr2(ur, vr);//ur - QL_B2(vr);
    x[17] = ql_macro_uiMvi2(ui, vi);//ui - QL_B2(vi);

    vi = x[24];
    vr = x[25];
    ur = QL_B2(x[8]);
    ui = QL_B2(x[9]);
    x[8] = ql_macro_urPvr2(ur, vr);//ur + QL_B2(vr);
    x[9] = ql_macro_uiMvi2(ui, vi);//ui - QL_B2(vi);
    x[24] = ql_macro_urMvr2(ur, vr);//ur - QL_B2(vr);
    x[25] = ql_macro_uiPvi2(ui, vi);//ui + QL_B2(vi);

    vr = x[48];
    vi = x[49];
    ur = QL_B2(x[32]);
    ui = QL_B2(x[33]);
    x[32] = ql_macro_urPvr2(ur, vr);//ur + QL_B2(vr);
    x[33] = ql_macro_uiPvi2(ui, vi);//ui + QL_B2(vi);
    x[48] = ql_macro_urMvr2(ur, vr);//ur - QL_B2(vr);
    x[49] = ql_macro_uiMvi2(ui, vi);//ui - QL_B2(vi);

    vi = x[56];
    vr = x[57];
    ur = QL_B2(x[40]);
    ui = QL_B2(x[41]);
    x[40] = ql_macro_urPvr2(ur, vr);//ur + QL_B2(vr);
    x[41] = ql_macro_uiMvi2(ui, vi);//ui - QL_B2(vi);
    x[56] = ql_macro_urMvr2(ur, vr);//ur - QL_B2(vr);
    x[57] = ql_macro_uiPvi2(ui, vi);//ui + QL_B2(vi);

    cplxMultDiv2(vi, vr, x[19], x[18], fft32_w32[0].re, fft32_w32[0].im);
    ur = x[2];
    ui = x[3];
    x[2] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[3] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;
    x[18] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[19] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;

    cplxMultDiv2(vr, vi, x[27], x[26], fft32_w32[0].re, fft32_w32[0].im);
    ur = x[10];
    ui = x[11];
    x[10] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[11] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;
    x[26] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[27] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;

    cplxMultDiv2(vi, vr, x[51], x[50], fft32_w32[0].re, fft32_w32[0].im);
    ur = x[34];
    ui = x[35];
    x[34] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[35] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;
    x[50] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[51] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;

    cplxMultDiv2(vr, vi, x[59], x[58], fft32_w32[0].re, fft32_w32[0].im);
    ur = x[42];
    ui = x[43];
    x[42] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[43] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;
    x[58] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[59] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;

    SUMDIFF_PIFOURTH(vi, vr, x[20], x[21]);
    ur = x[4];
    ui = x[5];
    x[4] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[5] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;
    x[20] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[21] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;

    SUMDIFF_PIFOURTH(vr, vi, x[28], x[29]);
    ur = x[12];
    ui = x[13];
    x[12] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[13] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;
    x[28] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[29] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;

    SUMDIFF_PIFOURTH(vi, vr, x[52], x[53]);
    ur = x[36];
    ui = x[37];
    x[36] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[37] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;
    x[52] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[53] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;

    SUMDIFF_PIFOURTH(vr, vi, x[60], x[61]);
    ur = x[44];
    ui = x[45];
    x[44] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[45] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;
    x[60] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[61] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;


    cplxMultDiv2(vi, vr, x[23], x[22], fft32_w32[1].re, fft32_w32[1].im);
    ur = x[6];
    ui = x[7];
    x[6] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[7] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;
    x[22] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[23] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;

    cplxMultDiv2(vr, vi, x[31], x[30], fft32_w32[1].re, fft32_w32[1].im);
    ur = x[14];
    ui = x[15];
    x[14] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[15] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;
    x[30] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[31] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;

    cplxMultDiv2(vi, vr, x[55], x[54], fft32_w32[1].re, fft32_w32[1].im);
    ur = x[38];
    ui = x[39];
    x[38] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[39] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;
    x[54] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[55] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;

    cplxMultDiv2(vr, vi, x[63], x[62], fft32_w32[1].re, fft32_w32[1].im);
    ur = x[46];
    ui = x[47];

    x[46] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[47] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;
    x[62] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[63] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;

    vr = x[32];
    vi = x[33];
    ur = QL_B2(x[0]);
    ui = QL_B2(x[1]);
    x[0] = ql_macro_urPvr2(ur, vr);//ur + QL_B2(vr);
    x[1] = ql_macro_uiPvi2(ui, vi);//ui + QL_B2(vi);
    x[32] = ql_macro_urMvr2(ur, vr);//ur - QL_B2(vr);
    x[33] = ql_macro_uiMvi2(ui, vi);//ui - QL_B2(vi);

    vi = x[48];
    vr = x[49];
    ur = QL_B2(x[16]);
    ui = QL_B2(x[17]);
    x[16] = ql_macro_urPvr2(ur, vr);//ur + QL_B2(vr);
    x[17] = ql_macro_uiMvi2(ui, vi);//ui - QL_B2(vi);
    x[48] = ql_macro_urMvr2(ur, vr);//ur - QL_B2(vr);
    x[49] = ql_macro_uiPvi2(ui, vi);//ui + QL_B2(vi);

    cplxMultDiv2(vi, vr, x[35], x[34], fft32_w32[2].re, fft32_w32[2].im);
    ur = x[2];
    ui = x[3];
    x[2] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[3] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;
    x[34] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[35] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;

    cplxMultDiv2(vr, vi, x[51], x[50], fft32_w32[2].re, fft32_w32[2].im);
    ur = x[18];
    ui = x[19];
    x[18] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[19] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;
    x[50] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[51] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;

    cplxMultDiv2(vi, vr, x[37], x[36], fft32_w32[0].re, fft32_w32[0].im);
    ur = x[4];
    ui = x[5];
    x[4] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[5] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;
    x[36] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[37] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;

    cplxMultDiv2(vr, vi, x[53], x[52], fft32_w32[0].re, fft32_w32[0].im);
    ur = x[20];
    ui = x[21];
    x[20] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[21] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;
    x[52] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[53] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;

    cplxMultDiv2(vi, vr, x[39], x[38], fft32_w32[3].re, fft32_w32[3].im);
    ur = x[6];
    ui = x[7];
    x[6] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[7] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;
    x[38] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[39] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;

    cplxMultDiv2(vr, vi, x[55], x[54], fft32_w32[3].re, fft32_w32[3].im);
    ur = x[22];
    ui = x[23];
    x[22] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[23] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;
    x[54] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[55] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;

    SUMDIFF_PIFOURTH(vi, vr, x[40], x[41]);
    ur = x[8];
    ui = x[9];
    x[8] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[9] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;
    x[40] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[41] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;

    SUMDIFF_PIFOURTH(vr, vi, x[56], x[57]);
    ur = x[24];
    ui = x[25];
    x[24] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[25] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;
    x[56] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[57] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;

    cplxMultDiv2(vi, vr, x[43], x[42], fft32_w32[4].re, fft32_w32[4].im);
    ur = x[10];
    ui = x[11];

    x[10] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[11] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;
    x[42] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[43] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;

    cplxMultDiv2(vr, vi, x[59], x[58], fft32_w32[4].re, fft32_w32[4].im);
    ur = x[26];
    ui = x[27];
    x[26] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[27] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;
    x[58] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[59] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;

    cplxMultDiv2(vi, vr, x[45], x[44], fft32_w32[1].re, fft32_w32[1].im);
    ur = x[12];
    ui = x[13];
    x[12] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[13] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;
    x[44] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[45] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;

    cplxMultDiv2(vr, vi, x[61], x[60], fft32_w32[1].re, fft32_w32[1].im);
    ur = x[28];
    ui = x[29];
    x[28] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[29] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;
    x[60] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[61] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;

    cplxMultDiv2(vi, vr, x[47], x[46], fft32_w32[5].re, fft32_w32[5].im);
    ur = x[14];
    ui = x[15];
    x[14] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[15] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;
    x[46] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[47] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;

    cplxMultDiv2(vr, vi, x[63], x[62], fft32_w32[5].re, fft32_w32[5].im);
    ur = x[30];
    ui = x[31];
    x[30] = ql_macro_ur2Pvr(ur, vr);///QL_B2(ur) + vr;
    x[31] = ql_macro_ui2Mvi(ui, vi);///QL_B2(ui) - vi;
    x[62] = ql_macro_ur2Mvr(ur, vr);///QL_B2(ur) - vr;
    x[63] = ql_macro_ui2Pvi(ui, vi);///QL_B2(ui) + vi;
}


#endif




#ifdef QL_FFT_60
const rota_coeff_t RotVectorReal60[] =
{
	ROTA(0.9945068359375),
	ROTA(0.9781494140625),
	ROTA(0.9510498046875),
	ROTA(0.9781494140625),
	ROTA(0.913543701171875),
	ROTA(0.80902099609375),
	ROTA(0.9510498046875),
	ROTA(0.80902099609375),
	ROTA(0.587799072265625),
	ROTA(0.913543701171875),
	ROTA(0.66912841796875),
	ROTA(0.30902099609375),
	ROTA(0.86602783203125),
	ROTA(0.5),
	ROTA(0),
	ROTA(0.80902099609375),
	ROTA(0.30902099609375),
	ROTA(-0.30902099609375),
	ROTA(0.743133544921875),
	ROTA(0.104522705078125),
	ROTA(-0.587799072265625),
	ROTA(0.66912841796875),
	ROTA(-0.104522705078125),
	ROTA(-0.80902099609375),
	ROTA(0.587799072265625),
	ROTA(-0.30902099609375),
	ROTA(-0.9510498046875),
	ROTA(0.5),
	ROTA(-0.5),
	ROTA(-1),
	ROTA(0.40673828125),
	ROTA(-0.66912841796875),
	ROTA(-0.9510498046875),
	ROTA(0.30902099609375),
	ROTA(-0.80902099609375),
	ROTA(-0.80902099609375),
	ROTA(0.207916259765625),
	ROTA(-0.913543701171875),
	ROTA(-0.587799072265625),
	ROTA(0.104522705078125),
	ROTA(-0.9781494140625),
	ROTA(-0.30902099609375),

};

const rota_coeff_t RotVectorImag60[] =
{
	ROTA(0.104522705078125),
	ROTA(0.207916259765625),
	ROTA(0.30902099609375),
	ROTA(0.207916259765625),
	ROTA(0.40673828125),
	ROTA(0.587799072265625),
	ROTA(0.30902099609375),
	ROTA(0.587799072265625),
	ROTA(0.80902099609375),
	ROTA(0.40673828125),
	ROTA(0.743133544921875),
	ROTA(0.9510498046875),
	ROTA(0.5),
	ROTA(0.86602783203125),
	ROTA(0.999969482421875),
	ROTA(0.587799072265625),
	ROTA(0.9510498046875),
	ROTA(0.9510498046875),
	ROTA(0.66912841796875),
	ROTA(0.9945068359375),
	ROTA(0.80902099609375),
	ROTA(0.743133544921875),
	ROTA(0.9945068359375),
	ROTA(0.587799072265625),
	ROTA(0.80902099609375),
	ROTA(0.9510498046875),
	ROTA(0.30902099609375),
	ROTA(0.86602783203125),
	ROTA(0.86602783203125),
	ROTA(0),
	ROTA(0.913543701171875),
	ROTA(0.743133544921875),
	ROTA(-0.30902099609375),
	ROTA(0.9510498046875),
	ROTA(0.587799072265625),
	ROTA(-0.587799072265625),
	ROTA(0.9781494140625),
	ROTA(0.40673828125),
	ROTA(-0.80902099609375),
	ROTA(0.9945068359375),
	ROTA(0.207916259765625),
	ROTA(-0.9510498046875),
};
#endif // QL_FFT_60
#ifdef QL_FFT_240
const rota_coeff_t RotVectorReal240[] =
{
	ROTA(0.999664306640625),
	ROTA(0.998626708984375),
	ROTA(0.996917724609375),
	ROTA(0.9945068359375),
	ROTA(0.991455078125),
	ROTA(0.987701416015625),
	ROTA(0.983245849609375),
	ROTA(0.9781494140625),
	ROTA(0.972381591796875),
	ROTA(0.965911865234375),
	ROTA(0.958831787109375),
	ROTA(0.9510498046875),
	ROTA(0.942626953125),
	ROTA(0.93359375),
	ROTA(0.92388916015625),
	ROTA(0.998626708984375),
	ROTA(0.9945068359375),
	ROTA(0.987701416015625),
	ROTA(0.9781494140625),
	ROTA(0.965911865234375),
	ROTA(0.9510498046875),
	ROTA(0.93359375),
	ROTA(0.913543701171875),
	ROTA(0.891021728515625),
	ROTA(0.86602783203125),
	ROTA(0.83868408203125),
	ROTA(0.80902099609375),
	ROTA(0.77716064453125),
	ROTA(0.743133544921875),
	ROTA(0.70709228515625),
	ROTA(0.996917724609375),
	ROTA(0.987701416015625),
	ROTA(0.972381591796875),
	ROTA(0.9510498046875),
	ROTA(0.92388916015625),
	ROTA(0.891021728515625),
	ROTA(0.852630615234375),
	ROTA(0.80902099609375),
	ROTA(0.760406494140625),
	ROTA(0.70709228515625),
	ROTA(0.649444580078125),
	ROTA(0.587799072265625),
	ROTA(0.522491455078125),
	ROTA(0.4539794921875),
	ROTA(0.3826904296875),
	ROTA(0.9945068359375),
	ROTA(0.9781494140625),
	ROTA(0.9510498046875),
	ROTA(0.913543701171875),
	ROTA(0.86602783203125),
	ROTA(0.80902099609375),
	ROTA(0.743133544921875),
	ROTA(0.66912841796875),
	ROTA(0.587799072265625),
	ROTA(0.5),
	ROTA(0.40673828125),
	ROTA(0.30902099609375),
	ROTA(0.207916259765625),
	ROTA(0.104522705078125),
	ROTA(0),
	ROTA(0.991455078125),
	ROTA(0.965911865234375),
	ROTA(0.92388916015625),
	ROTA(0.86602783203125),
	ROTA(0.793365478515625),
	ROTA(0.70709228515625),
	ROTA(0.6087646484375),
	ROTA(0.5),
	ROTA(0.3826904296875),
	ROTA(0.258819580078125),
	ROTA(0.130523681640625),
	ROTA(0),
	ROTA(-0.130523681640625),
	ROTA(-0.258819580078125),
	ROTA(-0.3826904296875),
	ROTA(0.987701416015625),
	ROTA(0.9510498046875),
	ROTA(0.891021728515625),
	ROTA(0.80902099609375),
	ROTA(0.70709228515625),
	ROTA(0.587799072265625),
	ROTA(0.4539794921875),
	ROTA(0.30902099609375),
	ROTA(0.15643310546875),
	ROTA(0),
	ROTA(-0.15643310546875),
	ROTA(-0.30902099609375),
	ROTA(-0.4539794921875),
	ROTA(-0.587799072265625),
	ROTA(-0.70709228515625),
	ROTA(0.983245849609375),
	ROTA(0.93359375),
	ROTA(0.852630615234375),
	ROTA(0.743133544921875),
	ROTA(0.6087646484375),
	ROTA(0.4539794921875),
	ROTA(0.284027099609375),
	ROTA(0.104522705078125),
	ROTA(-0.078460693359375),
	ROTA(-0.258819580078125),
	ROTA(-0.430511474609375),
	ROTA(-0.587799072265625),
	ROTA(-0.725372314453125),
	ROTA(-0.83868408203125),
	ROTA(-0.92388916015625),
	ROTA(0.9781494140625),
	ROTA(0.913543701171875),
	ROTA(0.80902099609375),
	ROTA(0.66912841796875),
	ROTA(0.5),
	ROTA(0.30902099609375),
	ROTA(0.104522705078125),
	ROTA(-0.104522705078125),
	ROTA(-0.30902099609375),
	ROTA(-0.5),
	ROTA(-0.66912841796875),
	ROTA(-0.80902099609375),
	ROTA(-0.913543701171875),
	ROTA(-0.9781494140625),
	ROTA(-1),
	ROTA(0.972381591796875),
	ROTA(0.891021728515625),
	ROTA(0.760406494140625),
	ROTA(0.587799072265625),
	ROTA(0.3826904296875),
	ROTA(0.15643310546875),
	ROTA(-0.078460693359375),
	ROTA(-0.30902099609375),
	ROTA(-0.522491455078125),
	ROTA(-0.70709228515625),
	ROTA(-0.852630615234375),
	ROTA(-0.9510498046875),
	ROTA(-0.996917724609375),
	ROTA(-0.987701416015625),
	ROTA(-0.92388916015625),
	ROTA(0.965911865234375),
	ROTA(0.86602783203125),
	ROTA(0.70709228515625),
	ROTA(0.5),
	ROTA(0.258819580078125),
	ROTA(0),
	ROTA(-0.258819580078125),
	ROTA(-0.5),
	ROTA(-0.70709228515625),
	ROTA(-0.86602783203125),
	ROTA(-0.965911865234375),
	ROTA(-1),
	ROTA(-0.965911865234375),
	ROTA(-0.86602783203125),
	ROTA(-0.70709228515625),
	ROTA(0.958831787109375),
	ROTA(0.83868408203125),
	ROTA(0.649444580078125),
	ROTA(0.40673828125),
	ROTA(0.130523681640625),
	ROTA(-0.15643310546875),
	ROTA(-0.430511474609375),
	ROTA(-0.66912841796875),
	ROTA(-0.852630615234375),
	ROTA(-0.965911865234375),
	ROTA(-0.999664306640625),
	ROTA(-0.9510498046875),
	ROTA(-0.824127197265625),
	ROTA(-0.62933349609375),
	ROTA(-0.3826904296875),
	ROTA(0.9510498046875),
	ROTA(0.80902099609375),
	ROTA(0.587799072265625),
	ROTA(0.30902099609375),
	ROTA(0),
	ROTA(-0.30902099609375),
	ROTA(-0.587799072265625),
	ROTA(-0.80902099609375),
	ROTA(-0.9510498046875),
	ROTA(-1),
	ROTA(-0.9510498046875),
	ROTA(-0.80902099609375),
	ROTA(-0.587799072265625),
	ROTA(-0.30902099609375),
	ROTA(0),
	ROTA(0.942626953125),
	ROTA(0.77716064453125),
	ROTA(0.522491455078125),
	ROTA(0.207916259765625),
	ROTA(-0.130523681640625),
	ROTA(-0.4539794921875),
	ROTA(-0.725372314453125),
	ROTA(-0.913543701171875),
	ROTA(-0.996917724609375),
	ROTA(-0.965911865234375),
	ROTA(-0.824127197265625),
	ROTA(-0.587799072265625),
	ROTA(-0.284027099609375),
	ROTA(0.052337646484375),
	ROTA(0.3826904296875),
	ROTA(0.93359375),
	ROTA(0.743133544921875),
	ROTA(0.4539794921875),
	ROTA(0.104522705078125),
	ROTA(-0.258819580078125),
	ROTA(-0.587799072265625),
	ROTA(-0.83868408203125),
	ROTA(-0.9781494140625),
	ROTA(-0.987701416015625),
	ROTA(-0.86602783203125),
	ROTA(-0.62933349609375),
	ROTA(-0.30902099609375),
	ROTA(0.052337646484375),
	ROTA(0.40673828125),
	ROTA(0.70709228515625),

};

const fft_coeff_t RotVectorImag240[] =
{
	ROTA(0.02618408203125),
	ROTA(0.052337646484375),
	ROTA(0.078460693359375),
	ROTA(0.104522705078125),
	ROTA(0.130523681640625),
	ROTA(0.15643310546875),
	ROTA(0.182220458984375),
	ROTA(0.207916259765625),
	ROTA(0.23345947265625),
	ROTA(0.258819580078125),
	ROTA(0.284027099609375),
	ROTA(0.30902099609375),
	ROTA(0.33380126953125),
	ROTA(0.358367919921875),
	ROTA(0.3826904296875),
	ROTA(0.052337646484375),
	ROTA(0.104522705078125),
	ROTA(0.15643310546875),
	ROTA(0.207916259765625),
	ROTA(0.258819580078125),
	ROTA(0.30902099609375),
	ROTA(0.358367919921875),
	ROTA(0.40673828125),
	ROTA(0.4539794921875),
	ROTA(0.5),
	ROTA(0.544647216796875),
	ROTA(0.587799072265625),
	ROTA(0.62933349609375),
	ROTA(0.66912841796875),
	ROTA(0.70709228515625),
	ROTA(0.078460693359375),
	ROTA(0.15643310546875),
	ROTA(0.23345947265625),
	ROTA(0.30902099609375),
	ROTA(0.3826904296875),
	ROTA(0.4539794921875),
	ROTA(0.522491455078125),
	ROTA(0.587799072265625),
	ROTA(0.649444580078125),
	ROTA(0.70709228515625),
	ROTA(0.760406494140625),
	ROTA(0.80902099609375),
	ROTA(0.852630615234375),
	ROTA(0.891021728515625),
	ROTA(0.92388916015625),
	ROTA(0.104522705078125),
	ROTA(0.207916259765625),
	ROTA(0.30902099609375),
	ROTA(0.40673828125),
	ROTA(0.5),
	ROTA(0.587799072265625),
	ROTA(0.66912841796875),
	ROTA(0.743133544921875),
	ROTA(0.80902099609375),
	ROTA(0.86602783203125),
	ROTA(0.913543701171875),
	ROTA(0.9510498046875),
	ROTA(0.9781494140625),
	ROTA(0.9945068359375),
	ROTA(0.999969482421875),
	ROTA(0.130523681640625),
	ROTA(0.258819580078125),
	ROTA(0.3826904296875),
	ROTA(0.5),
	ROTA(0.6087646484375),
	ROTA(0.70709228515625),
	ROTA(0.793365478515625),
	ROTA(0.86602783203125),
	ROTA(0.92388916015625),
	ROTA(0.965911865234375),
	ROTA(0.991455078125),
	ROTA(0.999969482421875),
	ROTA(0.991455078125),
	ROTA(0.965911865234375),
	ROTA(0.92388916015625),
	ROTA(0.15643310546875),
	ROTA(0.30902099609375),
	ROTA(0.4539794921875),
	ROTA(0.587799072265625),
	ROTA(0.70709228515625),
	ROTA(0.80902099609375),
	ROTA(0.891021728515625),
	ROTA(0.9510498046875),
	ROTA(0.987701416015625),
	ROTA(0.999969482421875),
	ROTA(0.987701416015625),
	ROTA(0.9510498046875),
	ROTA(0.891021728515625),
	ROTA(0.80902099609375),
	ROTA(0.70709228515625),
	ROTA(0.182220458984375),
	ROTA(0.358367919921875),
	ROTA(0.522491455078125),
	ROTA(0.66912841796875),
	ROTA(0.793365478515625),
	ROTA(0.891021728515625),
	ROTA(0.958831787109375),
	ROTA(0.9945068359375),
	ROTA(0.996917724609375),
	ROTA(0.965911865234375),
	ROTA(0.902587890625),
	ROTA(0.80902099609375),
	ROTA(0.6883544921875),
	ROTA(0.544647216796875),
	ROTA(0.3826904296875),
	ROTA(0.207916259765625),
	ROTA(0.40673828125),
	ROTA(0.587799072265625),
	ROTA(0.743133544921875),
	ROTA(0.86602783203125),
	ROTA(0.9510498046875),
	ROTA(0.9945068359375),
	ROTA(0.9945068359375),
	ROTA(0.9510498046875),
	ROTA(0.86602783203125),
	ROTA(0.743133544921875),
	ROTA(0.587799072265625),
	ROTA(0.40673828125),
	ROTA(0.207916259765625),
	ROTA(0),
	ROTA(0.23345947265625),
	ROTA(0.4539794921875),
	ROTA(0.649444580078125),
	ROTA(0.80902099609375),
	ROTA(0.92388916015625),
	ROTA(0.987701416015625),
	ROTA(0.996917724609375),
	ROTA(0.9510498046875),
	ROTA(0.852630615234375),
	ROTA(0.70709228515625),
	ROTA(0.522491455078125),
	ROTA(0.30902099609375),
	ROTA(0.078460693359375),
	ROTA(-0.15643310546875),
	ROTA(-0.3826904296875),
	ROTA(0.258819580078125),
	ROTA(0.5),
	ROTA(0.70709228515625),
	ROTA(0.86602783203125),
	ROTA(0.965911865234375),
	ROTA(0.999969482421875),
	ROTA(0.965911865234375),
	ROTA(0.86602783203125),
	ROTA(0.70709228515625),
	ROTA(0.5),
	ROTA(0.258819580078125),
	ROTA(0),
	ROTA(-0.258819580078125),
	ROTA(-0.5),
	ROTA(-0.70709228515625),
	ROTA(0.284027099609375),
	ROTA(0.544647216796875),
	ROTA(0.760406494140625),
	ROTA(0.913543701171875),
	ROTA(0.991455078125),
	ROTA(0.987701416015625),
	ROTA(0.902587890625),
	ROTA(0.743133544921875),
	ROTA(0.522491455078125),
	ROTA(0.258819580078125),
	ROTA(-0.02618408203125),
	ROTA(-0.30902099609375),
	ROTA(-0.56640625),
	ROTA(-0.77716064453125),
	ROTA(-0.92388916015625),
	ROTA(0.30902099609375),
	ROTA(0.587799072265625),
	ROTA(0.80902099609375),
	ROTA(0.9510498046875),
	ROTA(0.999969482421875),
	ROTA(0.9510498046875),
	ROTA(0.80902099609375),
	ROTA(0.587799072265625),
	ROTA(0.30902099609375),
	ROTA(0),
	ROTA(-0.30902099609375),
	ROTA(-0.587799072265625),
	ROTA(-0.80902099609375),
	ROTA(-0.9510498046875),
	ROTA(-1),
	ROTA(0.33380126953125),
	ROTA(0.62933349609375),
	ROTA(0.852630615234375),
	ROTA(0.9781494140625),
	ROTA(0.991455078125),
	ROTA(0.891021728515625),
	ROTA(0.6883544921875),
	ROTA(0.40673828125),
	ROTA(0.078460693359375),
	ROTA(-0.258819580078125),
	ROTA(-0.56640625),
	ROTA(-0.80902099609375),
	ROTA(-0.958831787109375),
	ROTA(-0.998626708984375),
	ROTA(-0.92388916015625),
	ROTA(0.358367919921875),
	ROTA(0.66912841796875),
	ROTA(0.891021728515625),
	ROTA(0.9945068359375),
	ROTA(0.965911865234375),
	ROTA(0.80902099609375),
	ROTA(0.544647216796875),
	ROTA(0.207916259765625),
	ROTA(-0.15643310546875),
	ROTA(-0.5),
	ROTA(-0.77716064453125),
	ROTA(-0.9510498046875),
	ROTA(-0.998626708984375),
	ROTA(-0.913543701171875),
	ROTA(-0.70709228515625),

};

#endif // QL_FFT_240
#ifdef QL_FFT_480 
const rota_coeff_t RotVectorReal480[] =
{
	ROTA(0.999908447265625),
	ROTA(0.999664306640625),
	ROTA(0.999237060546875),
	ROTA(0.998626708984375),
	ROTA(0.99786376953125),
	ROTA(0.996917724609375),
	ROTA(0.995819091796875),
	ROTA(0.9945068359375),
	ROTA(0.993072509765625),
	ROTA(0.991455078125),
	ROTA(0.989654541015625),
	ROTA(0.987701416015625),
	ROTA(0.985565185546875),
	ROTA(0.983245849609375),
	ROTA(0.98077392578125),
	ROTA(0.9781494140625),
	ROTA(0.975341796875),
	ROTA(0.972381591796875),
	ROTA(0.96923828125),
	ROTA(0.965911865234375),
	ROTA(0.96246337890625),
	ROTA(0.958831787109375),
	ROTA(0.95501708984375),
	ROTA(0.9510498046875),
	ROTA(0.946929931640625),
	ROTA(0.942626953125),
	ROTA(0.938201904296875),
	ROTA(0.93359375),
	ROTA(0.928802490234375),
	ROTA(0.92388916015625),
	ROTA(0.918792724609375),
	ROTA(0.999664306640625),
	ROTA(0.998626708984375),
	ROTA(0.996917724609375),
	ROTA(0.9945068359375),
	ROTA(0.991455078125),
	ROTA(0.987701416015625),
	ROTA(0.983245849609375),
	ROTA(0.9781494140625),
	ROTA(0.972381591796875),
	ROTA(0.965911865234375),
	ROTA(0.958831787109375),
	ROTA(0.9510498046875),
	ROTA(0.942626953125),
	ROTA(0.93359375),
	ROTA(0.92388916015625),
	ROTA(0.913543701171875),
	ROTA(0.902587890625),
	ROTA(0.891021728515625),
	ROTA(0.878814697265625),
	ROTA(0.86602783203125),
	ROTA(0.852630615234375),
	ROTA(0.83868408203125),
	ROTA(0.824127197265625),
	ROTA(0.80902099609375),
	ROTA(0.793365478515625),
	ROTA(0.77716064453125),
	ROTA(0.760406494140625),
	ROTA(0.743133544921875),
	ROTA(0.725372314453125),
	ROTA(0.70709228515625),
	ROTA(0.6883544921875),
	ROTA(0.999237060546875),
	ROTA(0.996917724609375),
	ROTA(0.993072509765625),
	ROTA(0.987701416015625),
	ROTA(0.98077392578125),
	ROTA(0.972381591796875),
	ROTA(0.96246337890625),
	ROTA(0.9510498046875),
	ROTA(0.938201904296875),
	ROTA(0.92388916015625),
	ROTA(0.90814208984375),
	ROTA(0.891021728515625),
	ROTA(0.87249755859375),
	ROTA(0.852630615234375),
	ROTA(0.83148193359375),
	ROTA(0.80902099609375),
	ROTA(0.785308837890625),
	ROTA(0.760406494140625),
	ROTA(0.73431396484375),
	ROTA(0.70709228515625),
	ROTA(0.678802490234375),
	ROTA(0.649444580078125),
	ROTA(0.61907958984375),
	ROTA(0.587799072265625),
	ROTA(0.555572509765625),
	ROTA(0.522491455078125),
	ROTA(0.488616943359375),
	ROTA(0.4539794921875),
	ROTA(0.418670654296875),
	ROTA(0.3826904296875),
	ROTA(0.34613037109375),
	ROTA(0.998626708984375),
	ROTA(0.9945068359375),
	ROTA(0.987701416015625),
	ROTA(0.9781494140625),
	ROTA(0.965911865234375),
	ROTA(0.9510498046875),
	ROTA(0.93359375),
	ROTA(0.913543701171875),
	ROTA(0.891021728515625),
	ROTA(0.86602783203125),
	ROTA(0.83868408203125),
	ROTA(0.80902099609375),
	ROTA(0.77716064453125),
	ROTA(0.743133544921875),
	ROTA(0.70709228515625),
	ROTA(0.66912841796875),
	ROTA(0.62933349609375),
	ROTA(0.587799072265625),
	ROTA(0.544647216796875),
	ROTA(0.5),
	ROTA(0.4539794921875),
	ROTA(0.40673828125),
	ROTA(0.358367919921875),
	ROTA(0.30902099609375),
	ROTA(0.258819580078125),
	ROTA(0.207916259765625),
	ROTA(0.15643310546875),
	ROTA(0.104522705078125),
	ROTA(0.052337646484375),
	ROTA(0),
	ROTA(-0.052337646484375),
	ROTA(0.99786376953125),
	ROTA(0.991455078125),
	ROTA(0.98077392578125),
	ROTA(0.965911865234375),
	ROTA(0.946929931640625),
	ROTA(0.92388916015625),
	ROTA(0.896881103515625),
	ROTA(0.86602783203125),
	ROTA(0.83148193359375),
	ROTA(0.793365478515625),
	ROTA(0.7518310546875),
	ROTA(0.70709228515625),
	ROTA(0.659332275390625),
	ROTA(0.6087646484375),
	ROTA(0.555572509765625),
	ROTA(0.5),
	ROTA(0.442291259765625),
	ROTA(0.3826904296875),
	ROTA(0.321441650390625),
	ROTA(0.258819580078125),
	ROTA(0.195098876953125),
	ROTA(0.130523681640625),
	ROTA(0.065399169921875),
	ROTA(0),
	ROTA(-0.065399169921875),
	ROTA(-0.130523681640625),
	ROTA(-0.195098876953125),
	ROTA(-0.258819580078125),
	ROTA(-0.321441650390625),
	ROTA(-0.3826904296875),
	ROTA(-0.442291259765625),
	ROTA(0.996917724609375),
	ROTA(0.987701416015625),
	ROTA(0.972381591796875),
	ROTA(0.9510498046875),
	ROTA(0.92388916015625),
	ROTA(0.891021728515625),
	ROTA(0.852630615234375),
	ROTA(0.80902099609375),
	ROTA(0.760406494140625),
	ROTA(0.70709228515625),
	ROTA(0.649444580078125),
	ROTA(0.587799072265625),
	ROTA(0.522491455078125),
	ROTA(0.4539794921875),
	ROTA(0.3826904296875),
	ROTA(0.30902099609375),
	ROTA(0.23345947265625),
	ROTA(0.15643310546875),
	ROTA(0.078460693359375),
	ROTA(0),
	ROTA(-0.078460693359375),
	ROTA(-0.15643310546875),
	ROTA(-0.23345947265625),
	ROTA(-0.30902099609375),
	ROTA(-0.3826904296875),
	ROTA(-0.4539794921875),
	ROTA(-0.522491455078125),
	ROTA(-0.587799072265625),
	ROTA(-0.649444580078125),
	ROTA(-0.70709228515625),
	ROTA(-0.760406494140625),
	ROTA(0.995819091796875),
	ROTA(0.983245849609375),
	ROTA(0.96246337890625),
	ROTA(0.93359375),
	ROTA(0.896881103515625),
	ROTA(0.852630615234375),
	ROTA(0.801239013671875),
	ROTA(0.743133544921875),
	ROTA(0.678802490234375),
	ROTA(0.6087646484375),
	ROTA(0.533599853515625),
	ROTA(0.4539794921875),
	ROTA(0.37054443359375),
	ROTA(0.284027099609375),
	ROTA(0.195098876953125),
	ROTA(0.104522705078125),
	ROTA(0.013092041015625),
	ROTA(-0.078460693359375),
	ROTA(-0.169342041015625),
	ROTA(-0.258819580078125),
	ROTA(-0.34613037109375),
	ROTA(-0.430511474609375),
	ROTA(-0.51129150390625),
	ROTA(-0.587799072265625),
	ROTA(-0.659332275390625),
	ROTA(-0.725372314453125),
	ROTA(-0.785308837890625),
	ROTA(-0.83868408203125),
	ROTA(-0.884979248046875),
	ROTA(-0.92388916015625),
	ROTA(-0.95501708984375),
	ROTA(0.9945068359375),
	ROTA(0.9781494140625),
	ROTA(0.9510498046875),
	ROTA(0.913543701171875),
	ROTA(0.86602783203125),
	ROTA(0.80902099609375),
	ROTA(0.743133544921875),
	ROTA(0.66912841796875),
	ROTA(0.587799072265625),
	ROTA(0.5),
	ROTA(0.40673828125),
	ROTA(0.30902099609375),
	ROTA(0.207916259765625),
	ROTA(0.104522705078125),
	ROTA(0),
	ROTA(-0.104522705078125),
	ROTA(-0.207916259765625),
	ROTA(-0.30902099609375),
	ROTA(-0.40673828125),
	ROTA(-0.5),
	ROTA(-0.587799072265625),
	ROTA(-0.66912841796875),
	ROTA(-0.743133544921875),
	ROTA(-0.80902099609375),
	ROTA(-0.86602783203125),
	ROTA(-0.913543701171875),
	ROTA(-0.9510498046875),
	ROTA(-0.9781494140625),
	ROTA(-0.9945068359375),
	ROTA(-1),
	ROTA(-0.9945068359375),
	ROTA(0.993072509765625),
	ROTA(0.972381591796875),
	ROTA(0.938201904296875),
	ROTA(0.891021728515625),
	ROTA(0.83148193359375),
	ROTA(0.760406494140625),
	ROTA(0.678802490234375),
	ROTA(0.587799072265625),
	ROTA(0.488616943359375),
	ROTA(0.3826904296875),
	ROTA(0.271453857421875),
	ROTA(0.15643310546875),
	ROTA(0.03924560546875),
	ROTA(-0.078460693359375),
	ROTA(-0.195098876953125),
	ROTA(-0.30902099609375),
	ROTA(-0.418670654296875),
	ROTA(-0.522491455078125),
	ROTA(-0.61907958984375),
	ROTA(-0.70709228515625),
	ROTA(-0.785308837890625),
	ROTA(-0.852630615234375),
	ROTA(-0.90814208984375),
	ROTA(-0.9510498046875),
	ROTA(-0.98077392578125),
	ROTA(-0.996917724609375),
	ROTA(-0.999237060546875),
	ROTA(-0.987701416015625),
	ROTA(-0.96246337890625),
	ROTA(-0.92388916015625),
	ROTA(-0.87249755859375),
	ROTA(0.991455078125),
	ROTA(0.965911865234375),
	ROTA(0.92388916015625),
	ROTA(0.86602783203125),
	ROTA(0.793365478515625),
	ROTA(0.70709228515625),
	ROTA(0.6087646484375),
	ROTA(0.5),
	ROTA(0.3826904296875),
	ROTA(0.258819580078125),
	ROTA(0.130523681640625),
	ROTA(0),
	ROTA(-0.130523681640625),
	ROTA(-0.258819580078125),
	ROTA(-0.3826904296875),
	ROTA(-0.5),
	ROTA(-0.6087646484375),
	ROTA(-0.70709228515625),
	ROTA(-0.793365478515625),
	ROTA(-0.86602783203125),
	ROTA(-0.92388916015625),
	ROTA(-0.965911865234375),
	ROTA(-0.991455078125),
	ROTA(-1),
	ROTA(-0.991455078125),
	ROTA(-0.965911865234375),
	ROTA(-0.92388916015625),
	ROTA(-0.86602783203125),
	ROTA(-0.793365478515625),
	ROTA(-0.70709228515625),
	ROTA(-0.6087646484375),
	ROTA(0.989654541015625),
	ROTA(0.958831787109375),
	ROTA(0.90814208984375),
	ROTA(0.83868408203125),
	ROTA(0.7518310546875),
	ROTA(0.649444580078125),
	ROTA(0.533599853515625),
	ROTA(0.40673828125),
	ROTA(0.271453857421875),
	ROTA(0.130523681640625),
	ROTA(-0.013092041015625),
	ROTA(-0.15643310546875),
	ROTA(-0.296539306640625),
	ROTA(-0.430511474609375),
	ROTA(-0.555572509765625),
	ROTA(-0.66912841796875),
	ROTA(-0.768829345703125),
	ROTA(-0.852630615234375),
	ROTA(-0.918792724609375),
	ROTA(-0.965911865234375),
	ROTA(-0.993072509765625),
	ROTA(-0.999664306640625),
	ROTA(-0.985565185546875),
	ROTA(-0.9510498046875),
	ROTA(-0.896881103515625),
	ROTA(-0.824127197265625),
	ROTA(-0.73431396484375),
	ROTA(-0.62933349609375),
	ROTA(-0.51129150390625),
	ROTA(-0.3826904296875),
	ROTA(-0.24615478515625),
	ROTA(0.987701416015625),
	ROTA(0.9510498046875),
	ROTA(0.891021728515625),
	ROTA(0.80902099609375),
	ROTA(0.70709228515625),
	ROTA(0.587799072265625),
	ROTA(0.4539794921875),
	ROTA(0.30902099609375),
	ROTA(0.15643310546875),
	ROTA(0),
	ROTA(-0.15643310546875),
	ROTA(-0.30902099609375),
	ROTA(-0.4539794921875),
	ROTA(-0.587799072265625),
	ROTA(-0.70709228515625),
	ROTA(-0.80902099609375),
	ROTA(-0.891021728515625),
	ROTA(-0.9510498046875),
	ROTA(-0.987701416015625),
	ROTA(-1),
	ROTA(-0.987701416015625),
	ROTA(-0.9510498046875),
	ROTA(-0.891021728515625),
	ROTA(-0.80902099609375),
	ROTA(-0.70709228515625),
	ROTA(-0.587799072265625),
	ROTA(-0.4539794921875),
	ROTA(-0.30902099609375),
	ROTA(-0.15643310546875),
	ROTA(0),
	ROTA(0.15643310546875),
	ROTA(0.985565185546875),
	ROTA(0.942626953125),
	ROTA(0.87249755859375),
	ROTA(0.77716064453125),
	ROTA(0.659332275390625),
	ROTA(0.522491455078125),
	ROTA(0.37054443359375),
	ROTA(0.207916259765625),
	ROTA(0.03924560546875),
	ROTA(-0.130523681640625),
	ROTA(-0.296539306640625),
	ROTA(-0.4539794921875),
	ROTA(-0.59832763671875),
	ROTA(-0.725372314453125),
	ROTA(-0.83148193359375),
	ROTA(-0.913543701171875),
	ROTA(-0.96923828125),
	ROTA(-0.996917724609375),
	ROTA(-0.995819091796875),
	ROTA(-0.965911865234375),
	ROTA(-0.90814208984375),
	ROTA(-0.824127197265625),
	ROTA(-0.71630859375),
	ROTA(-0.587799072265625),
	ROTA(-0.442291259765625),
	ROTA(-0.284027099609375),
	ROTA(-0.117523193359375),
	ROTA(0.052337646484375),
	ROTA(0.220703125),
	ROTA(0.3826904296875),
	ROTA(0.533599853515625),
	ROTA(0.983245849609375),
	ROTA(0.93359375),
	ROTA(0.852630615234375),
	ROTA(0.743133544921875),
	ROTA(0.6087646484375),
	ROTA(0.4539794921875),
	ROTA(0.284027099609375),
	ROTA(0.104522705078125),
	ROTA(-0.078460693359375),
	ROTA(-0.258819580078125),
	ROTA(-0.430511474609375),
	ROTA(-0.587799072265625),
	ROTA(-0.725372314453125),
	ROTA(-0.83868408203125),
	ROTA(-0.92388916015625),
	ROTA(-0.9781494140625),
	ROTA(-0.999664306640625),
	ROTA(-0.987701416015625),
	ROTA(-0.942626953125),
	ROTA(-0.86602783203125),
	ROTA(-0.760406494140625),
	ROTA(-0.62933349609375),
	ROTA(-0.4771728515625),
	ROTA(-0.30902099609375),
	ROTA(-0.130523681640625),
	ROTA(0.052337646484375),
	ROTA(0.23345947265625),
	ROTA(0.40673828125),
	ROTA(0.56640625),
	ROTA(0.70709228515625),
	ROTA(0.824127197265625),

};

const rota_coeff_t RotVectorImag480[] =
{
	ROTA(0.013092041015625),
	ROTA(0.02618408203125),
	ROTA(0.03924560546875),
	ROTA(0.052337646484375),
	ROTA(0.065399169921875),
	ROTA(0.078460693359375),
	ROTA(0.09149169921875),
	ROTA(0.104522705078125),
	ROTA(0.117523193359375),
	ROTA(0.130523681640625),
	ROTA(0.14349365234375),
	ROTA(0.15643310546875),
	ROTA(0.169342041015625),
	ROTA(0.182220458984375),
	ROTA(0.195098876953125),
	ROTA(0.207916259765625),
	ROTA(0.220703125),
	ROTA(0.23345947265625),
	ROTA(0.24615478515625),
	ROTA(0.258819580078125),
	ROTA(0.271453857421875),
	ROTA(0.284027099609375),
	ROTA(0.296539306640625),
	ROTA(0.30902099609375),
	ROTA(0.321441650390625),
	ROTA(0.33380126953125),
	ROTA(0.34613037109375),
	ROTA(0.358367919921875),
	ROTA(0.37054443359375),
	ROTA(0.3826904296875),
	ROTA(0.394744873046875),
	ROTA(0.02618408203125),
	ROTA(0.052337646484375),
	ROTA(0.078460693359375),
	ROTA(0.104522705078125),
	ROTA(0.130523681640625),
	ROTA(0.15643310546875),
	ROTA(0.182220458984375),
	ROTA(0.207916259765625),
	ROTA(0.23345947265625),
	ROTA(0.258819580078125),
	ROTA(0.284027099609375),
	ROTA(0.30902099609375),
	ROTA(0.33380126953125),
	ROTA(0.358367919921875),
	ROTA(0.3826904296875),
	ROTA(0.40673828125),
	ROTA(0.430511474609375),
	ROTA(0.4539794921875),
	ROTA(0.4771728515625),
	ROTA(0.5),
	ROTA(0.522491455078125),
	ROTA(0.544647216796875),
	ROTA(0.56640625),
	ROTA(0.587799072265625),
	ROTA(0.6087646484375),
	ROTA(0.62933349609375),
	ROTA(0.649444580078125),
	ROTA(0.66912841796875),
	ROTA(0.6883544921875),
	ROTA(0.70709228515625),
	ROTA(0.725372314453125),
	ROTA(0.03924560546875),
	ROTA(0.078460693359375),
	ROTA(0.117523193359375),
	ROTA(0.15643310546875),
	ROTA(0.195098876953125),
	ROTA(0.23345947265625),
	ROTA(0.271453857421875),
	ROTA(0.30902099609375),
	ROTA(0.34613037109375),
	ROTA(0.3826904296875),
	ROTA(0.418670654296875),
	ROTA(0.4539794921875),
	ROTA(0.488616943359375),
	ROTA(0.522491455078125),
	ROTA(0.555572509765625),
	ROTA(0.587799072265625),
	ROTA(0.61907958984375),
	ROTA(0.649444580078125),
	ROTA(0.678802490234375),
	ROTA(0.70709228515625),
	ROTA(0.73431396484375),
	ROTA(0.760406494140625),
	ROTA(0.785308837890625),
	ROTA(0.80902099609375),
	ROTA(0.83148193359375),
	ROTA(0.852630615234375),
	ROTA(0.87249755859375),
	ROTA(0.891021728515625),
	ROTA(0.90814208984375),
	ROTA(0.92388916015625),
	ROTA(0.938201904296875),
	ROTA(0.052337646484375),
	ROTA(0.104522705078125),
	ROTA(0.15643310546875),
	ROTA(0.207916259765625),
	ROTA(0.258819580078125),
	ROTA(0.30902099609375),
	ROTA(0.358367919921875),
	ROTA(0.40673828125),
	ROTA(0.4539794921875),
	ROTA(0.5),
	ROTA(0.544647216796875),
	ROTA(0.587799072265625),
	ROTA(0.62933349609375),
	ROTA(0.66912841796875),
	ROTA(0.70709228515625),
	ROTA(0.743133544921875),
	ROTA(0.77716064453125),
	ROTA(0.80902099609375),
	ROTA(0.83868408203125),
	ROTA(0.86602783203125),
	ROTA(0.891021728515625),
	ROTA(0.913543701171875),
	ROTA(0.93359375),
	ROTA(0.9510498046875),
	ROTA(0.965911865234375),
	ROTA(0.9781494140625),
	ROTA(0.987701416015625),
	ROTA(0.9945068359375),
	ROTA(0.998626708984375),
	ROTA(0.999969482421875),
	ROTA(0.998626708984375),
	ROTA(0.065399169921875),
	ROTA(0.130523681640625),
	ROTA(0.195098876953125),
	ROTA(0.258819580078125),
	ROTA(0.321441650390625),
	ROTA(0.3826904296875),
	ROTA(0.442291259765625),
	ROTA(0.5),
	ROTA(0.555572509765625),
	ROTA(0.6087646484375),
	ROTA(0.659332275390625),
	ROTA(0.70709228515625),
	ROTA(0.7518310546875),
	ROTA(0.793365478515625),
	ROTA(0.83148193359375),
	ROTA(0.86602783203125),
	ROTA(0.896881103515625),
	ROTA(0.92388916015625),
	ROTA(0.946929931640625),
	ROTA(0.965911865234375),
	ROTA(0.98077392578125),
	ROTA(0.991455078125),
	ROTA(0.99786376953125),
	ROTA(0.999969482421875),
	ROTA(0.99786376953125),
	ROTA(0.991455078125),
	ROTA(0.98077392578125),
	ROTA(0.965911865234375),
	ROTA(0.946929931640625),
	ROTA(0.92388916015625),
	ROTA(0.896881103515625),
	ROTA(0.078460693359375),
	ROTA(0.15643310546875),
	ROTA(0.23345947265625),
	ROTA(0.30902099609375),
	ROTA(0.3826904296875),
	ROTA(0.4539794921875),
	ROTA(0.522491455078125),
	ROTA(0.587799072265625),
	ROTA(0.649444580078125),
	ROTA(0.70709228515625),
	ROTA(0.760406494140625),
	ROTA(0.80902099609375),
	ROTA(0.852630615234375),
	ROTA(0.891021728515625),
	ROTA(0.92388916015625),
	ROTA(0.9510498046875),
	ROTA(0.972381591796875),
	ROTA(0.987701416015625),
	ROTA(0.996917724609375),
	ROTA(0.999969482421875),
	ROTA(0.996917724609375),
	ROTA(0.987701416015625),
	ROTA(0.972381591796875),
	ROTA(0.9510498046875),
	ROTA(0.92388916015625),
	ROTA(0.891021728515625),
	ROTA(0.852630615234375),
	ROTA(0.80902099609375),
	ROTA(0.760406494140625),
	ROTA(0.70709228515625),
	ROTA(0.649444580078125),
	ROTA(0.09149169921875),
	ROTA(0.182220458984375),
	ROTA(0.271453857421875),
	ROTA(0.358367919921875),
	ROTA(0.442291259765625),
	ROTA(0.522491455078125),
	ROTA(0.59832763671875),
	ROTA(0.66912841796875),
	ROTA(0.73431396484375),
	ROTA(0.793365478515625),
	ROTA(0.845733642578125),
	ROTA(0.891021728515625),
	ROTA(0.928802490234375),
	ROTA(0.958831787109375),
	ROTA(0.98077392578125),
	ROTA(0.9945068359375),
	ROTA(0.999908447265625),
	ROTA(0.996917724609375),
	ROTA(0.985565185546875),
	ROTA(0.965911865234375),
	ROTA(0.938201904296875),
	ROTA(0.902587890625),
	ROTA(0.859405517578125),
	ROTA(0.80902099609375),
	ROTA(0.7518310546875),
	ROTA(0.6883544921875),
	ROTA(0.61907958984375),
	ROTA(0.544647216796875),
	ROTA(0.465606689453125),
	ROTA(0.3826904296875),
	ROTA(0.296539306640625),
	ROTA(0.104522705078125),
	ROTA(0.207916259765625),
	ROTA(0.30902099609375),
	ROTA(0.40673828125),
	ROTA(0.5),
	ROTA(0.587799072265625),
	ROTA(0.66912841796875),
	ROTA(0.743133544921875),
	ROTA(0.80902099609375),
	ROTA(0.86602783203125),
	ROTA(0.913543701171875),
	ROTA(0.9510498046875),
	ROTA(0.9781494140625),
	ROTA(0.9945068359375),
	ROTA(0.999969482421875),
	ROTA(0.9945068359375),
	ROTA(0.9781494140625),
	ROTA(0.9510498046875),
	ROTA(0.913543701171875),
	ROTA(0.86602783203125),
	ROTA(0.80902099609375),
	ROTA(0.743133544921875),
	ROTA(0.66912841796875),
	ROTA(0.587799072265625),
	ROTA(0.5),
	ROTA(0.40673828125),
	ROTA(0.30902099609375),
	ROTA(0.207916259765625),
	ROTA(0.104522705078125),
	ROTA(0),
	ROTA(-0.104522705078125),
	ROTA(0.117523193359375),
	ROTA(0.23345947265625),
	ROTA(0.34613037109375),
	ROTA(0.4539794921875),
	ROTA(0.555572509765625),
	ROTA(0.649444580078125),
	ROTA(0.73431396484375),
	ROTA(0.80902099609375),
	ROTA(0.87249755859375),
	ROTA(0.92388916015625),
	ROTA(0.96246337890625),
	ROTA(0.987701416015625),
	ROTA(0.999237060546875),
	ROTA(0.996917724609375),
	ROTA(0.98077392578125),
	ROTA(0.9510498046875),
	ROTA(0.90814208984375),
	ROTA(0.852630615234375),
	ROTA(0.785308837890625),
	ROTA(0.70709228515625),
	ROTA(0.61907958984375),
	ROTA(0.522491455078125),
	ROTA(0.418670654296875),
	ROTA(0.30902099609375),
	ROTA(0.195098876953125),
	ROTA(0.078460693359375),
	ROTA(-0.03924560546875),
	ROTA(-0.15643310546875),
	ROTA(-0.271453857421875),
	ROTA(-0.3826904296875),
	ROTA(-0.488616943359375),
	ROTA(0.130523681640625),
	ROTA(0.258819580078125),
	ROTA(0.3826904296875),
	ROTA(0.5),
	ROTA(0.6087646484375),
	ROTA(0.70709228515625),
	ROTA(0.793365478515625),
	ROTA(0.86602783203125),
	ROTA(0.92388916015625),
	ROTA(0.965911865234375),
	ROTA(0.991455078125),
	ROTA(0.999969482421875),
	ROTA(0.991455078125),
	ROTA(0.965911865234375),
	ROTA(0.92388916015625),
	ROTA(0.86602783203125),
	ROTA(0.793365478515625),
	ROTA(0.70709228515625),
	ROTA(0.6087646484375),
	ROTA(0.5),
	ROTA(0.3826904296875),
	ROTA(0.258819580078125),
	ROTA(0.130523681640625),
	ROTA(0),
	ROTA(-0.130523681640625),
	ROTA(-0.258819580078125),
	ROTA(-0.3826904296875),
	ROTA(-0.5),
	ROTA(-0.6087646484375),
	ROTA(-0.70709228515625),
	ROTA(-0.793365478515625),
	ROTA(0.14349365234375),
	ROTA(0.284027099609375),
	ROTA(0.418670654296875),
	ROTA(0.544647216796875),
	ROTA(0.659332275390625),
	ROTA(0.760406494140625),
	ROTA(0.845733642578125),
	ROTA(0.913543701171875),
	ROTA(0.96246337890625),
	ROTA(0.991455078125),
	ROTA(0.999908447265625),
	ROTA(0.987701416015625),
	ROTA(0.95501708984375),
	ROTA(0.902587890625),
	ROTA(0.83148193359375),
	ROTA(0.743133544921875),
	ROTA(0.639434814453125),
	ROTA(0.522491455078125),
	ROTA(0.394744873046875),
	ROTA(0.258819580078125),
	ROTA(0.117523193359375),
	ROTA(-0.02618408203125),
	ROTA(-0.169342041015625),
	ROTA(-0.30902099609375),
	ROTA(-0.442291259765625),
	ROTA(-0.56640625),
	ROTA(-0.678802490234375),
	ROTA(-0.77716064453125),
	ROTA(-0.859405517578125),
	ROTA(-0.92388916015625),
	ROTA(-0.96923828125),
	ROTA(0.15643310546875),
	ROTA(0.30902099609375),
	ROTA(0.4539794921875),
	ROTA(0.587799072265625),
	ROTA(0.70709228515625),
	ROTA(0.80902099609375),
	ROTA(0.891021728515625),
	ROTA(0.9510498046875),
	ROTA(0.987701416015625),
	ROTA(0.999969482421875),
	ROTA(0.987701416015625),
	ROTA(0.9510498046875),
	ROTA(0.891021728515625),
	ROTA(0.80902099609375),
	ROTA(0.70709228515625),
	ROTA(0.587799072265625),
	ROTA(0.4539794921875),
	ROTA(0.30902099609375),
	ROTA(0.15643310546875),
	ROTA(0),
	ROTA(-0.15643310546875),
	ROTA(-0.30902099609375),
	ROTA(-0.4539794921875),
	ROTA(-0.587799072265625),
	ROTA(-0.70709228515625),
	ROTA(-0.80902099609375),
	ROTA(-0.891021728515625),
	ROTA(-0.9510498046875),
	ROTA(-0.987701416015625),
	ROTA(-1),
	ROTA(-0.987701416015625),
	ROTA(0.169342041015625),
	ROTA(0.33380126953125),
	ROTA(0.488616943359375),
	ROTA(0.62933349609375),
	ROTA(0.7518310546875),
	ROTA(0.852630615234375),
	ROTA(0.928802490234375),
	ROTA(0.9781494140625),
	ROTA(0.999237060546875),
	ROTA(0.991455078125),
	ROTA(0.95501708984375),
	ROTA(0.891021728515625),
	ROTA(0.801239013671875),
	ROTA(0.6883544921875),
	ROTA(0.555572509765625),
	ROTA(0.40673828125),
	ROTA(0.24615478515625),
	ROTA(0.078460693359375),
	ROTA(-0.09149169921875),
	ROTA(-0.258819580078125),
	ROTA(-0.418670654296875),
	ROTA(-0.56640625),
	ROTA(-0.697784423828125),
	ROTA(-0.80902099609375),
	ROTA(-0.896881103515625),
	ROTA(-0.958831787109375),
	ROTA(-0.993072509765625),
	ROTA(-0.998626708984375),
	ROTA(-0.975341796875),
	ROTA(-0.92388916015625),
	ROTA(-0.845733642578125),
	ROTA(0.182220458984375),
	ROTA(0.358367919921875),
	ROTA(0.522491455078125),
	ROTA(0.66912841796875),
	ROTA(0.793365478515625),
	ROTA(0.891021728515625),
	ROTA(0.958831787109375),
	ROTA(0.9945068359375),
	ROTA(0.996917724609375),
	ROTA(0.965911865234375),
	ROTA(0.902587890625),
	ROTA(0.80902099609375),
	ROTA(0.6883544921875),
	ROTA(0.544647216796875),
	ROTA(0.3826904296875),
	ROTA(0.207916259765625),
	ROTA(0.02618408203125),
	ROTA(-0.15643310546875),
	ROTA(-0.33380126953125),
	ROTA(-0.5),
	ROTA(-0.649444580078125),
	ROTA(-0.77716064453125),
	ROTA(-0.878814697265625),
	ROTA(-0.9510498046875),
	ROTA(-0.991455078125),
	ROTA(-0.998626708984375),
	ROTA(-0.972381591796875),
	ROTA(-0.913543701171875),
	ROTA(-0.824127197265625),
	ROTA(-0.70709228515625),
	ROTA(-0.56640625),

};
#endif // QL_FFT_480

/**
The function performs the fft of length 60. It is splittet into fft's of length 4 and fft's of
length 15. Between the fft's a modolation is calculated.
*/
#ifdef QL_FFT_60
#define SCALEFACTOR60         5
static inline void fft60(fft_in_sample_t *pInput, int *pScalefactor)
{
	fftN2(
		pInput, 60, 4, 15,
		fft_4, fft15,
		RotVectorReal60, RotVectorImag60
	);
	*pScalefactor += SCALEFACTOR60;
}

#endif

/* Fallback implementation in case of no better implementation available. */


/**
The function performs the fft of length 240. It is splittet into fft's of length 16 and fft's of
length 15. Between the fft's a modulation is calculated.
*/
#ifdef QL_FFT_240
#define SCALEFACTOR240        7
void fft240(fft_in_sample_t *pInput, int *pScalefactor)
{
	fftN2(
		pInput, 240, 16, 15,
		fft_16, fft15,
		RotVectorReal240, RotVectorImag240
	);
	*pScalefactor += SCALEFACTOR240;
}
#endif


/**
The function performs the fft of length 480. It is splittet into fft's of length 32 and fft's of
length 15. Between the fft's a modulation is calculated.
*/
#ifdef QL_FFT_480
#define SCALEFACTOR480        8
#define N32                   32
#define TABLE_SIZE_16        (32/2)
static inline void fft480(fft_in_sample_t *pInput, int *pScalefactor)
{
	fftN2(
		pInput, 480, 32, 15,
		fft_32, fft15,
		RotVectorReal480, RotVectorImag480
	);
	*pScalefactor += SCALEFACTOR480;
}
#endif


void ql_fft(int length, fft_in_sample_t *pInput, int *pScalefactor)
{
#ifdef QL_FFT_32
#define SCALEFACTOR32          4
	if (length == 32)
	{
		fft_32(pInput);
        *pScalefactor += SCALEFACTOR32; 
	}
	else
#endif
	{

		switch (length) {
 #ifdef QL_FFT_16
#define SCALEFACTOR16          3
		case 16:
			fft_16(pInput);
			*pScalefactor += SCALEFACTOR16;
			break;
#endif
#ifdef QL_FFT_8
#define SCALEFACTOR8           2
		case 8:
			fft_8(pInput);
			*pScalefactor += SCALEFACTOR8;
			break;
#endif
#ifdef QL_FFT_3
		case 3:
			fft3(pInput);
			break;
#endif
#ifdef QL_FFT_4
		case 4:
			fft_4(pInput);
			*pScalefactor += 1;
			break;
#endif
#ifdef QL_FFT_5
		case 5:
			fft5(pInput);
			break;
#endif
#ifdef QL_FFT_15                       
		case 15:
			fft15(pInput);
			*pScalefactor += 2;
			break;
#endif
#ifdef QL_FFT_60                       
		case 60:
			fft60(pInput, pScalefactor);
			break;
#endif                        
#if 0
		case 64:
			dit_fft(pInput, 6, SineTable512, 512);
			*pScalefactor += SCALEFACTOR64;
			break;
#endif
#ifdef QL_FFT_240
		case 240:
			fft240(pInput, pScalefactor);
			break;
#endif                        
#if 0
		case 256:
			dit_fft(pInput, 8, SineTable512, 512);
			*pScalefactor += SCALEFACTOR256;
			break;
#endif
#ifdef QL_FFT_480                        
		case 480:
			fft480(pInput, pScalefactor);
			break;
#endif                        
#if 0 
		case 512:
			dit_fft(pInput, 9, SineTable512, 512);
			*pScalefactor += SCALEFACTOR512;
			break;
#endif
		default:
            printf("   ERROR : FFT length not supported!, %d\n", length);
	//		FDK_ASSERT(0); /* FFT length not supported! */
			break;
		}
	}
}



/**
* \brief Apply rotation vectors to a data buffer.
* \param cl length of each row of input data.
* \param l total length of input data.
* \param pVecRe real part of rotation ceofficient vector.
* \param pVecIm imaginary part of rotation ceofficient vector.
*/


static inline void fft_apply_rot_vector(fft_in_sample_t * pData, const int cl, const int l, const rota_coeff_t *pVecRe, const rota_coeff_t *pVecIm)
{
    fft_in_sample_t re, im;
    rota_coeff_t vre, vim;
	
	int cl2 = 2 * cl;
    ARRAY_SCALE4_STRIDE(pData, 2, cl);

    for (int  i = cl2; i<2*l; i+= cl2)
	{
        pData[i] = Div4(pData[i]);     /* * 0.25 */
        pData[i + 1] = Div4(pData[i+1]);     /* * 0.25 */

        for (int c = i+2; c< i + cl2; c+=2)
		{
			re = pData[c];
			im = pData[c + 1];
			vre = *pVecRe++;
			vim = *pVecIm++;

            cplxMultRotaDiv4(pData[c + 1], pData[c], im, re, vre, vim);
	}
	}
}

#ifdef TST_QL_FFT
#include <stdio.h>
#define PCM_BITS (16)
int main(int argc, char* argv[])
	{
	FILE *fp, *fp1;
	VARDECL(ql_complex_t, f2);

	int Scalefactor = 0;
    int NN[] = {4, 5, 8, 15,32 ,60, 480 };
    int N = NN[6];
	char outfile_name[1024];
    float re, im;

	fopen_s(&fp, argv[1], "r");

	printf("Testing QL FFT \n");

	ALLOC(f2, N, ql_complex_t);

	if (fp != NULL)
	{
		for (int i = 0; i < N; i++)
		{
#ifdef ENABLE_FIXEDPOINT
            fscanf_s(fp, "%f%f", &re, &im);
            f2[i].re = (fft_in_sample_t)( re* (1 << PCM_BITS));
            f2[i].im = (fft_in_sample_t)( im* (1 << PCM_BITS));
#else
			fscanf_s(fp, "%f%f", &f2[i].re, &f2[i].im);

#endif
		}

        sprintf_s(outfile_name, 1024, "%s_%s", "out_fft3", argv[1]);
		fopen_s(&fp1, outfile_name, "w");
	}
    ql_fft(N, (fft_in_sample_t *)f2, &Scalefactor);

    printf("file = %s \t Scalefactor = %d\n", outfile_name, Scalefactor);
	if (fp1 != NULL)
	{
		for (int i = 0; i < N; i++)
		{
#ifdef ENABLE_FIXEDPOINT
            re = ((float)f2[i].re) / (1 << PCM_BITS);
            im = ((float)f2[i].im) / (1 << PCM_BITS);
            fprintf_s(fp1, "%f\t%f\n",re, im);
#else
			fprintf_s(fp1, "%f\t%f\n", f2[i].re, f2[i].im);
#endif
		}
	}
	else
	{
		printf("   ERROR : Unable to access output file %s\n", outfile_name);
}
	fclose(fp1);
	fclose(fp);

	return Scalefactor;
	
}

#endif

