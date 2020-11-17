#include <stdio.h>
#ifdef USE_KISS_FFT

/*Copyright (c) 2003-2004, Mark Borgerding
  Lots of modifications by Jean-Marc Valin
  Copyright (c) 2005-2007, Xiph.Org Foundation
  Copyright (c) 2008,      Xiph.Org Foundation, CSIRO

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.*/

/* This code is originally from Mark Borgerding's KISS-FFT but has been
   heavily modified to better suit Opus */

#ifndef SKIP_CONFIG_H
#  ifdef HAVE_CONFIG_H
#    include "config.h"
#  endif
#endif

#include "_kiss_fft_guts.h"
#include "arch.h"
#include "os_support.h"
#include "mathops.h"
#include "stack_alloc.h"

/* The guts header contains all the multiplication and addition macros that are defined for
   complex numbers.  It also delares the kf_ internal functions.
*/

static void kf_bfly2(
                     kiss_fft_cpx * Fout,
                     int m,
                     int N
                    )
{
   kiss_fft_cpx * Fout2;
   int i;
   (void)m;
#ifdef CUSTOM_MODES
   if (m==1)
   {
      celt_assert(m==1);
      for (i=0;i<N;i++)
      {
         kiss_fft_cpx t;
         Fout2 = Fout + 1;
         t = *Fout2;
         C_SUB( *Fout2 ,  *Fout , t );
         C_ADDTO( *Fout ,  t );
         Fout += 2;
      }
   } else
#endif
   {
      opus_val16 tw;
      tw = QCONST16(0.7071067812f, 15);
      /* We know that m==4 here because the radix-2 is just after a radix-4 */
      celt_assert(m==4);
      for (i=0;i<N;i++)
      {
         kiss_fft_cpx t;
         Fout2 = Fout + 4;
         t = Fout2[0];
         C_SUB( Fout2[0] ,  Fout[0] , t );
         C_ADDTO( Fout[0] ,  t );

         t.r = S_MUL(ADD32_ovflw(Fout2[1].r, Fout2[1].i), tw);
         t.i = S_MUL(SUB32_ovflw(Fout2[1].i, Fout2[1].r), tw);
         C_SUB( Fout2[1] ,  Fout[1] , t );
         C_ADDTO( Fout[1] ,  t );

         t.r = Fout2[2].i;
         t.i = -Fout2[2].r;
         C_SUB( Fout2[2] ,  Fout[2] , t );
         C_ADDTO( Fout[2] ,  t );

         t.r = S_MUL(SUB32_ovflw(Fout2[3].i, Fout2[3].r), tw);
         t.i = S_MUL(NEG32_ovflw(ADD32_ovflw(Fout2[3].i, Fout2[3].r)), tw);
         C_SUB( Fout2[3] ,  Fout[3] , t );
         C_ADDTO( Fout[3] ,  t );
         Fout += 8;
      }
   }
}

static void kf_bfly4(
                     kiss_fft_cpx * Fout,
                     const size_t fstride,
                     const kiss_fft_state *st,
                     int m,
                     int N,
                     int mm
                    )
{
   int i;

   if (m==1)
   {
      /* Degenerate case where all the twiddles are 1. */
      for (i=0;i<N;i++)
      {
         kiss_fft_cpx scratch0, scratch1;

         C_SUB( scratch0 , *Fout, Fout[2] );
         C_ADDTO(*Fout, Fout[2]);
         C_ADD( scratch1 , Fout[1] , Fout[3] );
         C_SUB( Fout[2], *Fout, scratch1 );
         C_ADDTO( *Fout , scratch1 );
         C_SUB( scratch1 , Fout[1] , Fout[3] );

         Fout[1].r = ADD32_ovflw(scratch0.r, scratch1.i);
         Fout[1].i = SUB32_ovflw(scratch0.i, scratch1.r);
         Fout[3].r = SUB32_ovflw(scratch0.r, scratch1.i);
         Fout[3].i = ADD32_ovflw(scratch0.i, scratch1.r);
         Fout+=4;
      }
   } else {
      int j;
      kiss_fft_cpx scratch[6];
      const kiss_twiddle_cpx *tw1,*tw2,*tw3;
      const int m2=2*m;
      const int m3=3*m;
      kiss_fft_cpx * Fout_beg = Fout;
      for (i=0;i<N;i++)
      {
         Fout = Fout_beg + i*mm;
         tw3 = tw2 = tw1 = st->twiddles;
         /* m is guaranteed to be a multiple of 4. */
         for (j=0;j<m;j++)
         {
            C_MUL(scratch[0],Fout[m] , *tw1 );
            C_MUL(scratch[1],Fout[m2] , *tw2 );
            C_MUL(scratch[2],Fout[m3] , *tw3 );

            C_SUB( scratch[5] , *Fout, scratch[1] );
            C_ADDTO(*Fout, scratch[1]);
            C_ADD( scratch[3] , scratch[0] , scratch[2] );
            C_SUB( scratch[4] , scratch[0] , scratch[2] );
            C_SUB( Fout[m2], *Fout, scratch[3] );
            tw1 += fstride;
            tw2 += fstride*2;
            tw3 += fstride*3;
            C_ADDTO( *Fout , scratch[3] );

            Fout[m].r = ADD32_ovflw(scratch[5].r, scratch[4].i);
            Fout[m].i = SUB32_ovflw(scratch[5].i, scratch[4].r);
            Fout[m3].r = SUB32_ovflw(scratch[5].r, scratch[4].i);
            Fout[m3].i = ADD32_ovflw(scratch[5].i, scratch[4].r);
            ++Fout;
         }
      }
   }
}






#ifndef RADIX_TWO_ONLY

static void kf_bfly3(
                     kiss_fft_cpx * Fout,
                     const size_t fstride,
                     const kiss_fft_state *st,
                     int m,
                     int N,
                     int mm
                    )
{
   int i;
   size_t k;
   const size_t m2 = 2*m;
   const kiss_twiddle_cpx *tw1,*tw2;
   kiss_fft_cpx scratch[5];
   kiss_twiddle_cpx epi3;

   kiss_fft_cpx * Fout_beg = Fout;
#ifdef FIXED_POINT
   /*epi3.r = -16384;*/ /* Unused */
   epi3.i = -28378;
#else
   epi3 = st->twiddles[fstride*m];
#endif
   for (i=0;i<N;i++)
   {
      Fout = Fout_beg + i*mm;
      tw1=tw2=st->twiddles;
      /* For non-custom modes, m is guaranteed to be a multiple of 4. */
      k=m;
      do {

         C_MUL(scratch[1],Fout[m] , *tw1);
         C_MUL(scratch[2],Fout[m2] , *tw2);

         C_ADD(scratch[3],scratch[1],scratch[2]);
         C_SUB(scratch[0],scratch[1],scratch[2]);
         tw1 += fstride;
         tw2 += fstride*2;

         Fout[m].r = SUB32_ovflw(Fout->r, HALF_OF(scratch[3].r));
         Fout[m].i = SUB32_ovflw(Fout->i, HALF_OF(scratch[3].i));

         C_MULBYSCALAR( scratch[0] , epi3.i );

         C_ADDTO(*Fout,scratch[3]);

         Fout[m2].r = ADD32_ovflw(Fout[m].r, scratch[0].i);
         Fout[m2].i = SUB32_ovflw(Fout[m].i, scratch[0].r);

         Fout[m].r = SUB32_ovflw(Fout[m].r, scratch[0].i);
         Fout[m].i = ADD32_ovflw(Fout[m].i, scratch[0].r);

         ++Fout;
      } while(--k);
   }
}


#ifndef OVERRIDE_kf_bfly5
static void kf_bfly5(
                     kiss_fft_cpx * Fout,
                     const size_t fstride,
                     const kiss_fft_state *st,
                     int m,
                     int N,
                     int mm
                    )
{
   kiss_fft_cpx *Fout0,*Fout1,*Fout2,*Fout3,*Fout4;
   int i, u;
   kiss_fft_cpx scratch[13];
   const kiss_twiddle_cpx *tw;
   kiss_twiddle_cpx ya,yb;
   kiss_fft_cpx * Fout_beg = Fout;

#ifdef FIXED_POINT
   ya.r = 10126;
   ya.i = -31164;
   yb.r = -26510;
   yb.i = -19261;
#else
   ya = st->twiddles[fstride*m];
   yb = st->twiddles[fstride*2*m];
#endif
   tw=st->twiddles;

   for (i=0;i<N;i++)
   {
      Fout = Fout_beg + i*mm;
      Fout0=Fout;
      Fout1=Fout0+m;
      Fout2=Fout0+2*m;
      Fout3=Fout0+3*m;
      Fout4=Fout0+4*m;

      /* For non-custom modes, m is guaranteed to be a multiple of 4. */
      for ( u=0; u<m; ++u ) {
         scratch[0] = *Fout0;

         C_MUL(scratch[1] ,*Fout1, tw[u*fstride]);
         C_MUL(scratch[2] ,*Fout2, tw[2*u*fstride]);
         C_MUL(scratch[3] ,*Fout3, tw[3*u*fstride]);
         C_MUL(scratch[4] ,*Fout4, tw[4*u*fstride]);

         C_ADD( scratch[7],scratch[1],scratch[4]);
         C_SUB( scratch[10],scratch[1],scratch[4]);
         C_ADD( scratch[8],scratch[2],scratch[3]);
         C_SUB( scratch[9],scratch[2],scratch[3]);

         Fout0->r = ADD32_ovflw(Fout0->r, ADD32_ovflw(scratch[7].r, scratch[8].r));
         Fout0->i = ADD32_ovflw(Fout0->i, ADD32_ovflw(scratch[7].i, scratch[8].i));

         scratch[5].r = ADD32_ovflw(scratch[0].r, ADD32_ovflw(S_MUL(scratch[7].r,ya.r), S_MUL(scratch[8].r,yb.r)));
         scratch[5].i = ADD32_ovflw(scratch[0].i, ADD32_ovflw(S_MUL(scratch[7].i,ya.r), S_MUL(scratch[8].i,yb.r)));

         scratch[6].r =  ADD32_ovflw(S_MUL(scratch[10].i,ya.i), S_MUL(scratch[9].i,yb.i));
         scratch[6].i = NEG32_ovflw(ADD32_ovflw(S_MUL(scratch[10].r,ya.i), S_MUL(scratch[9].r,yb.i)));

         C_SUB(*Fout1,scratch[5],scratch[6]);
         C_ADD(*Fout4,scratch[5],scratch[6]);

         scratch[11].r = ADD32_ovflw(scratch[0].r, ADD32_ovflw(S_MUL(scratch[7].r,yb.r), S_MUL(scratch[8].r,ya.r)));
         scratch[11].i = ADD32_ovflw(scratch[0].i, ADD32_ovflw(S_MUL(scratch[7].i,yb.r), S_MUL(scratch[8].i,ya.r)));
         scratch[12].r = SUB32_ovflw(S_MUL(scratch[9].i,ya.i), S_MUL(scratch[10].i,yb.i));
         scratch[12].i = SUB32_ovflw(S_MUL(scratch[10].r,yb.i), S_MUL(scratch[9].r,ya.i));

         C_ADD(*Fout2,scratch[11],scratch[12]);
         C_SUB(*Fout3,scratch[11],scratch[12]);

         ++Fout0;++Fout1;++Fout2;++Fout3;++Fout4;
      }
   }
}
#endif /* OVERRIDE_kf_bfly5 */


#endif


void opus_fft_impl(const kiss_fft_state *st,kiss_fft_cpx *fout)
{
    int m2, m;
    int p;
    int L;
    int fstride[MAXFACTORS];
    int i;
    int shift;

    /* st->shift can be -1 */
    shift = st->shift>0 ? st->shift : 0;

    fstride[0] = 1;
    L=0;
    do {
       p = st->factors[2*L];
       m = st->factors[2*L+1];
       fstride[L+1] = fstride[L]*p;
       L++;
    } while(m!=1);
    m = st->factors[2*L-1];
    for (i=L-1;i>=0;i--)
    {
       if (i!=0)
          m2 = st->factors[2*i-1];
       else
          m2 = 1;
       switch (st->factors[2*i])
       {
       case 2:
          kf_bfly2(fout, m, fstride[i]);
          break;
       case 4:
          kf_bfly4(fout,fstride[i]<<shift,st,m, fstride[i], m2);
          break;
 #ifndef RADIX_TWO_ONLY
       case 3:
          kf_bfly3(fout,fstride[i]<<shift,st,m, fstride[i], m2);
          break;
       case 5:
          kf_bfly5(fout,fstride[i]<<shift,st,m, fstride[i], m2);
          break;
 #endif
       }
       m = m2;
    }
}

void opus_fft_c(const kiss_fft_state *st,const kiss_fft_cpx *fin,kiss_fft_cpx *fout)
{
   int i;
   opus_val16 scale;
#ifdef FIXED_POINT
   /* Allows us to scale with MULT16_32_Q16(), which is faster than
      MULT16_32_Q15() on ARM. */
   int scale_shift = st->scale_shift-1;
#endif
   scale = st->scale;

   celt_assert2 (fin != fout, "In-place FFT not supported");
   /* Bit-reverse the input */
   for (i=0;i<st->nfft;i++)
   {
      kiss_fft_cpx x = fin[i];
      fout[st->bitrev[i]].r = SHR32(MULT16_32_Q16(scale, x.r), scale_shift);
      fout[st->bitrev[i]].i = SHR32(MULT16_32_Q16(scale, x.i), scale_shift);
   }
   opus_fft_impl(st, fout);
}


void opus_ifft_c(const kiss_fft_state *st,const kiss_fft_cpx *fin,kiss_fft_cpx *fout)
{
   int i;
   celt_assert2 (fin != fout, "In-place FFT not supported");
   /* Bit-reverse the input */
   for (i=0;i<st->nfft;i++)
      fout[st->bitrev[i]] = fin[i];
   for (i=0;i<st->nfft;i++)
      fout[i].i = -fout[i].i;
   opus_fft_impl(st, fout);
   for (i=0;i<st->nfft;i++)
      fout[i].i = -fout[i].i;
}
#endif /* USE_KISS_FFT */
