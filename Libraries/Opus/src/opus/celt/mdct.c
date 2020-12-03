/* Copyright (c) 2007-2008 CSIRO
   Copyright (c) 2007-2008 Xiph.Org Foundation
   Written by Jean-Marc Valin */
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* This is a simple MDCT implementation that uses a N/4 complex FFT
   to do most of the work. It should be relatively straightforward to
   plug in pretty much and FFT here.

   This replaces the Vorbis FFT (and uses the exact same API), which
   was a bit too messy and that was ending up duplicating code
   (might as well use the same FFT everywhere).

   The algorithm is similar to (and inspired from) Fabrice Bellard's
   MDCT implementation in FFMPEG, but has differences in signs, ordering
   and scaling in many places.
*/

#ifndef SKIP_CONFIG_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#endif

#include "mdct.h"
#include "kiss_fft.h"
#include "_kiss_fft_guts.h"
#include <math.h>
#include "os_support.h"
#include "mathops.h"
#include "stack_alloc.h"

#if defined(MIPSr1_ASM)
#include "mips/mdct_mipsr1.h"
#endif

#ifndef USE_KISS_FFT
#include "ql_dsp.h"
#else
typedef kiss_fft_scalar ql_sample_float_in_t;
typedef kiss_fft_cpx ql_complex_t;
#endif

int frame_count = 0;
typedef float ql_sample_float_in_t;
/* Forward MDCT trashes the input array */
#ifndef OVERRIDE_clt_mdct_forward
#ifdef USE_QL_MDCT_TWIDDLE
#define QL_WTIDDLE_MULT(a, b)         ((((int_acc_t)(a))*(((int_acc_t)(b))))>> (QL_MDCT_TWIDDLE_BITS))
#define WIN_MULT16_32_Q15(a,b)        ((a)*(b))
#define WIN_MULT1(a)        ((a))
#else
#define QL_WTIDDLE_MULT(a, b)         ((a)*(b))
#define WIN_MULT16_32_Q15(a,b)        ((a)*(b))
#define WIN_MULT1(a)        ((a))
#endif

void clt_mdct_forward_c(const mdct_lookup *l, ql_sample_mdct_in_t *in, ql_sample_mdct_in_t * OPUS_RESTRICT out,
        const ql_window_t *window, int overlap, int shift, int stride, int arch)
{
   int i;
   int N, N2, N4;
   VARDECL(ql_sample_in_t, f);
#ifndef ENABLE_FIXEDPOINT
   VARDECL(ql_complex_t, f2);
#else
   VARDECL(ql_complex_fix_t, f2);
#endif
   const ql_twiddle_t *trig;
   float scale;
#ifdef FIXED_POINT
   /* Allows us to scale with MULT16_32_Q16(), which is faster than
      MULT16_32_Q15() on ARM. */
   int scale_shift = st->scale_shift-1;
#endif
   SAVE_STACK;
   (void)arch;
   int ql_fft_scale;
   //printf("shift = %d\n", shift);
   N = l->n;
   
   trig = l->trig;
   for (i=0;i<shift;i++)
   {
      N >>= 1;
      trig += N;
   }
   N2 = N>>1;
   N4 = N>>2;

   ALLOC(f, N2, ql_sample_in_t);
#ifndef ENABLE_FIXEDPOINT
   ALLOC(f2, N4, ql_complex_t);
#else
   ALLOC(f2, N4, ql_complex_fix_t);
#endif

   if (N4 == 60)
       ql_fft_scale = 5 + 15;
   else 
       ql_fft_scale = 8 + 15;
   frame_count++;
   if (N4 == 480)
   {
       N4 = N4;
   }
   if ((!((N4 == 60) || (N4 == 480))))
   {
       printf("n = %d at %d\n", N4, frame_count);
   }
   celt_assert2(((N4 == 60) || (N4 == 480)), "the FFT order supported are either 60 or 480 ");
#ifdef USE_QL_MDCT_TWIDDLE
   scale = l->scales[shift] *(1<< ql_fft_scale);// QL_MDCT_TWIDDLE_BITS
#else
   scale = l->scales[shift] * (1 << ql_fft_scale);
#endif
   /* Consider the input to be composed of four blocks: [a, b, c, d] */
   /* Window, shuffle, fold */
   {
      /* Temp pointers to make it really clear to the compiler what we're doing */
      const ql_sample_float_in_t * OPUS_RESTRICT xp1 = in+(overlap>>1);
      const ql_sample_float_in_t * OPUS_RESTRICT xp2 = in+N2-1+(overlap>>1);
      ql_sample_in_t * OPUS_RESTRICT yp = f;
      const ql_window_t * OPUS_RESTRICT wp1 = window+(overlap>>1);
      const ql_window_t * OPUS_RESTRICT wp2 = window+(overlap>>1)-1;
      for(i=0;i<((overlap+3)>>2);i++)
      {
         /* Real part arranged as -d-cR, Imag part arranged as -b+aR*/
         *yp++ = WIN_MULT16_32_Q15(*wp2, xp1[N2]) + WIN_MULT16_32_Q15(*wp1,*xp2);
         *yp++ = WIN_MULT16_32_Q15(*wp1, *xp1)    - WIN_MULT16_32_Q15(*wp2, xp2[-N2]);
         xp1+=2;
         xp2-=2;
         wp1+=2;
         wp2-=2;
      }
      wp1 = window;
      wp2 = window+overlap-1;
      for(;i<N4-((overlap+3)>>2);i++)
      {
         /* Real part arranged as a-bR, Imag part arranged as -c-dR */
         *yp++ = WIN_MULT1(*xp2);
         *yp++ = WIN_MULT1(*xp1);
         xp1+=2;
         xp2-=2;
      }
      for(;i<N4;i++)
      {
         /* Real part arranged as a-bR, Imag part arranged as -c-dR */
         *yp++ =  -WIN_MULT16_32_Q15(*wp1, xp1[-N2]) + WIN_MULT16_32_Q15(*wp2, *xp2);
         *yp++ = WIN_MULT16_32_Q15(*wp2, *xp1)     + WIN_MULT16_32_Q15(*wp1, xp2[N2]);
         xp1+=2;
         xp2-=2;
         wp1+=2;
         wp2-=2;
      }
   }
   /* Pre-rotation */
   {
      ql_sample_in_t * OPUS_RESTRICT yp = f;
      const ql_twiddle_t *t = &trig[0];
      for(i=0;i<N4;i++)
      {
          ql_complex_fix_t yc;
          ql_twiddle_t t0, t1;
         ql_sample_in_t re, im, yr, yi;
         t0 = t[i];
         t1 = t[N4+i];
         re = *yp++;
         im = *yp++;
         yr = QL_WTIDDLE_MULT(re,t0)  - QL_WTIDDLE_MULT(im,t1);
         yi = QL_WTIDDLE_MULT(im,t0)  + QL_WTIDDLE_MULT(re,t1);
		 yc.re = yr;
		 yc.im = yi;
		 f2[i].re = (yc.re* scale);
		 f2[i].im = (yc.im* scale);
	  }
   }
   int Scalefactor = 0;
#ifdef ENABLE_FIXEDPOINT
   ql_fft(N4, (ql_sample_in_t*) f2, &Scalefactor);
#else
   ql_fft(N4, (float *)f2, &Scalefactor);

#endif
   celt_assert2(((Scalefactor == 5) || (Scalefactor == 8)), "the scale factor of FFT short/long is not in the range");
   float inv_scale = 1.0f / (1<<(15));

   /* Post-rotate */
   {
      /* Temp pointers to make it really clear to the compiler what we're doing */
#ifndef ENABLE_FIXEDPOINT
      const ql_complex_t * OPUS_RESTRICT fp = f2;
#else
       const ql_complex_fix_t * OPUS_RESTRICT fp = f2;
#endif
	   ql_sample_float_in_t * OPUS_RESTRICT yp1 = out;
	   ql_sample_float_in_t * OPUS_RESTRICT yp2 = out + stride * (N2 - 1);
	   const ql_twiddle_t *t = &trig[0];
	   /* Temp pointers to make it really clear to the compiler what we're doing */
	   for (i = 0; i<N4; i++)
	   {
		   ql_sample_in_t yr, yi;

		   yr = QL_WTIDDLE_MULT(fp->im, t[N4 + i]) - QL_WTIDDLE_MULT(fp->re, t[i]);
		   yi = QL_WTIDDLE_MULT(fp->re, t[N4 + i]) + QL_WTIDDLE_MULT(fp->im, t[i]);
		   *yp1 = yr * inv_scale;
		   *yp2 = yi * inv_scale;
		   fp++;
		   yp1 += 2 * stride;
		   yp2 -= 2 * stride;
	   }
   }
   FREE1(f, N2, ql_sample_float_in_t);
   FREE1(f2, N4, ql_complex_t);

   RESTORE_STACK;
}
#endif /* OVERRIDE_clt_mdct_forward */
