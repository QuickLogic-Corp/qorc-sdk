/* Copyright (c) 2007-2008 CSIRO
   Copyright (c) 2007-2009 Xiph.Org Foundation
   Copyright (c) 2008-2009 Gregory Maxwell
   Written by Jean-Marc Valin and Gregory Maxwell */
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include "bands.h"
#include "modes.h"
#include "vq.h"
#include "cwrs.h"
#include "stack_alloc.h"
#include "os_support.h"
#include "mathops.h"
#include "rate.h"
#include "quant_bands.h"
#include "pitch.h"
#ifdef ENABLE_STEREO
/*Compute floor(sqrt(_val)) with exact arithmetic.
  This has been tested on all possible 32-bit inputs.*/
static unsigned isqrt32(opus_uint32 _val){
  unsigned b;
  unsigned g;
  int      bshift;
  /*Uses the second method from
     http://www.azillionmonkeys.com/qed/sqroot.html
    The main idea is to search for the largest binary digit b such that
     (g+b)*(g+b) <= _val, and add it to the solution g.*/
  g=0;
  bshift=(EC_ILOG(_val)-1)>>1;
  b=1U<<bshift;
  do{
    opus_uint32 t;
    t=(((opus_uint32)g<<1)+b)<<bshift;
    if(t<=_val){
      g+=b;
      _val-=t;
    }
    b>>=1;
    bshift--;
  }
  while(bshift>=0);
  return g;
}
#endif

int hysteresis_decision(opus_val16 val, const opus_val16 *thresholds, const opus_val16 *hysteresis, int N, int prev)
{
   int i;
   for (i=0;i<N;i++)
   {
      if (val < thresholds[i])
         break;
   }
   if (i>prev && val < thresholds[prev]+hysteresis[prev])
      i=prev;
   if (i<prev && val > thresholds[prev-1]-hysteresis[prev-1])
      i=prev;
   return i;
}


opus_uint32 celt_lcg_rand(opus_uint32 seed)
{
   return 1664525 * seed + 1013904223;
}
#ifdef SUPPORT_CTX_SPLIT
/* This is a cos() approximation designed to be bit-exact on any platform. Bit exactness
   with this approximation is important because it has an impact on the bit allocation */
opus_int16 bitexact_cos(opus_int16 x)
{
   opus_int32 tmp;
   opus_int16 x2;
   tmp = (4096+((opus_int32)(x)*(x)))>>13;
   celt_sig_assert(tmp<=32767);
   x2 = tmp;
   x2 = (32767-x2) + FRAC_MUL16(x2, (-7651 + FRAC_MUL16(x2, (8277 + FRAC_MUL16(-626, x2)))));
   celt_sig_assert(x2<=32766);
   return 1+x2;
}

int bitexact_log2tan(int isin,int icos)
{
   int lc;
   int ls;
   lc=EC_ILOG(icos);
   ls=EC_ILOG(isin);
   icos<<=15-lc;
   isin<<=15-ls;
   return (ls-lc)*(1<<11)
         +FRAC_MUL16(isin, FRAC_MUL16(isin, -2597) + 7932)
         -FRAC_MUL16(icos, FRAC_MUL16(icos, -2597) + 7932);
}
#endif /* SUPPORT_CTX_SPLIT */

#ifdef FIXED_POINT
#else /* FIXED_POINT */
/* Compute the amplitude (sqrt energy) in each of the bands */

#if 0

void compute_band_energies(const CELTMode *m, const celt_sig *X, celt_ener *bandE, int end, int C, int LM, int arch)
{
   int i, c, N;
   const opus_int16 *eBands = m->eBands;
   N = m->shortMdctSize<<LM;
   c=0; do {
      for (i=0;i<end;i++)
      {
         opus_val32 sum;
         sum = 1e-27f + celt_inner_prod(&X[c*N+(eBands[i]<<LM)], &X[c*N+(eBands[i]<<LM)], (eBands[i+1]-eBands[i])<<LM, arch);
         bandE[i+c*m->nbEBands] = celt_sqrt(sum);
         /*printf ("%f ", bandE[i+c*m->nbEBands]);*/
      }
   } while (++c<C);
   /*printf ("\n");*/
}
#else
void compute_band_energies(const CELTMode *m, const celt_sig *X, celt_ener *bandE, int end, int C, int LM, int arch)
{
    int i;
    const opus_int16 *eBands = m->eBands;
    int bstart = 0, bend = 0;
        for (i = 0; i<end; i++)
        {
            opus_val32 sum;
            bend = (eBands[i + 1]) << LM;

            sum = 1e-27f + celt_inner_prod(&X[ bstart], &X[ bstart], bend-bstart, arch);
            bstart = bend;
            bandE[i ] = celt_sqrt(sum);
        }
}


#endif
/* Normalise each band such that the energy is one. */
void normalise_bands(const CELTMode *m, const celt_sig * OPUS_RESTRICT freq, celt_norm * OPUS_RESTRICT X, const celt_ener *bandE, int end, int C, int M)
{
   int i, c, N;
   const opus_int16 *eBands = m->eBands;
   N = M*m->shortMdctSize;
   c=0; do {
      for (i=0;i<end;i++)
      {
         int j;
         opus_val16 g = 1.f/(1e-27f+bandE[i+c*m->nbEBands]);
         for (j=M*eBands[i];j<M*eBands[i+1];j++)
            X[j+c*N] = freq[j+c*N]*g;
      }
   } while (++c<C);
}

#endif /* FIXED_POINT */




/* Decide whether we should spread the pulses in the current frame */
int spreading_decision(const CELTMode *m, const celt_norm *X, int *average,
      int last_decision, int *hf_average, int *tapset_decision, int update_hf,
      int end, int C, int M, const int *spread_weight)
{
   int i, c, N0;
   int sum = 0, nbBands=0;
   const opus_int16 * OPUS_RESTRICT eBands = m->eBands;
   int decision;
   int hf_sum=0;

   celt_assert(end>0);

   N0 = M*m->shortMdctSize;

   if (M*(eBands[end]-eBands[end-1]) <= 8)
      return SPREAD_NONE;
   c=0; do {
      for (i=0;i<end;i++)
      {
         int j, N, tmp=0;
         int tcount[3] = {0,0,0};
         const celt_norm * OPUS_RESTRICT x = X+M*eBands[i]+c*N0;
         N = M*(eBands[i+1]-eBands[i]);
         if (N<=8)
            continue;
         /* Compute rough CDF of |x[j]| */
         for (j=0;j<N;j++)
         {
            opus_val32 x2N; /* Q13 */

            x2N = MULT16_16(MULT16_16_Q15(x[j], x[j]), N);
            if (x2N < QCONST16(0.25f,13))
               tcount[0]++;
            if (x2N < QCONST16(0.0625f,13))
               tcount[1]++;
            if (x2N < QCONST16(0.015625f,13))
               tcount[2]++;
         }

         /* Only include four last bands (8 kHz and up) */
         if (i>m->nbEBands-4)
            hf_sum += celt_udiv(32*(tcount[1]+tcount[0]), N);
         tmp = (2*tcount[2] >= N) + (2*tcount[1] >= N) + (2*tcount[0] >= N);
         sum += tmp*spread_weight[i];
         nbBands+=spread_weight[i];
      }
   } while (++c<C);

   if (update_hf)
   {
      if (hf_sum)
         hf_sum = celt_udiv(hf_sum, C*(4-m->nbEBands+end));
      *hf_average = (*hf_average+hf_sum)>>1;
      hf_sum = *hf_average;
      if (*tapset_decision==2)
         hf_sum += 4;
      else if (*tapset_decision==0)
         hf_sum -= 4;
      if (hf_sum > 22)
         *tapset_decision=2;
      else if (hf_sum > 18)
         *tapset_decision=1;
      else
         *tapset_decision=0;
   }
   /*printf("%d %d %d\n", hf_sum, *hf_average, *tapset_decision);*/
   celt_assert(nbBands>0); /* end has to be non-zero */
   celt_assert(sum>=0);
   sum = celt_udiv((opus_int32)sum<<8, nbBands);
   /* Recursive averaging */
   sum = (sum+*average)>>1;
   *average = sum;
   /* Hysteresis */
   sum = (3*sum + (((3-last_decision)<<7) + 64) + 2)>>2;
   if (sum < 80)
   {
      decision = SPREAD_AGGRESSIVE;
   } else if (sum < 256)
   {
      decision = SPREAD_NORMAL;
   } else if (sum < 384)
   {
      decision = SPREAD_LIGHT;
   } else {
      decision = SPREAD_NONE;
   }
#ifdef FUZZING
   decision = rand()&0x3;
   *tapset_decision=rand()%3;
#endif
   return decision;
}

/* Indexing table for converting from natural Hadamard to ordery Hadamard
   This is essentially a bit-reversed Gray, on top of which we've added
   an inversion of the order because we want the DC at the end rather than
   the beginning. The lines are for N=2, 4, 8, 16 */
static const int ordery_table[] = {
       1,  0,
       3,  0,  2,  1,
       7,  0,  4,  3,  6,  1,  5,  2,
      15,  0,  8,  7, 12,  3, 11,  4, 14,  1,  9,  6, 13,  2, 10,  5,
};

static void deinterleave_hadamard(celt_norm *X, int N0, int stride, int hadamard)
{
   int i,j;
   VARDECL(celt_norm, tmp);
   int N;
   SAVE_STACK;
   N = N0*stride;
   ALLOC(tmp, N, celt_norm);
   celt_assert(stride>0);
   if (hadamard)
   {
      const int *ordery = ordery_table+stride-2;
      for (i=0;i<stride;i++)
      {
         for (j=0;j<N0;j++)
            tmp[ordery[i]*N0+j] = X[j*stride+i];
      }
   } else {
      for (i=0;i<stride;i++)
         for (j=0;j<N0;j++)
            tmp[i*N0+j] = X[j*stride+i];
   }
   OPUS_COPY(X, tmp, N);
   FREE1(tmp, N, celt_norm);
   RESTORE_STACK;
}

#if 0
void haar1(celt_norm *X, int N0, int stride)
{
   int i, j;
   N0 >>= 1;
   for (i = 0; i < stride; i++) 
   {
       for (j = 0; j < N0; j++)
       {
           opus_val32 tmp1, tmp2;
           tmp1 = MULT16_16(QCONST16(.70710678f, 15), X[stride * 2 * j + i]);
           tmp2 = MULT16_16(QCONST16(.70710678f, 15), X[stride*(2 * j + 1) + i]);
           X[stride * 2 * j + i] = EXTRACT16(PSHR32(ADD32(tmp1, tmp2), 15));
           X[stride*(2 * j + 1) + i] = EXTRACT16(PSHR32(SUB32(tmp1, tmp2), 15));
       }
   }
}
#else
void haar1(celt_norm *X, int N0, int stride)
{
    int i, j;
    N0 >>= 1;
    int idx = 0;
    int idx1 = 0;
    for (i = 0; i < stride; i++, idx+= i )
    {
        for (j = 0, idx = i; j < N0; j++, idx += stride * 2)
        {
            opus_val32 tmp1, tmp2;
            idx1 = idx + stride;
            tmp1 = MULT16_16(QCONST16(.70710678f, 15), X[idx]);
            tmp2 = MULT16_16(QCONST16(.70710678f, 15), X[idx1]);
            X[idx] = EXTRACT16(PSHR32(ADD32(tmp1, tmp2), 15));
            X[idx1] = EXTRACT16(PSHR32(SUB32(tmp1, tmp2), 15));
        }
    }
}

#endif
struct band_ctx {
   int encode;
   int resynth;
   const CELTMode *m;
   int i;
   int intensity;
   int spread;
   int tf_change;
   ec_ctx *ec;
   opus_int32 remaining_bits;
   const celt_ener *bandE;
   opus_uint32 seed;
   int arch;
   int theta_round;
   int disable_inv;
   int avoid_split_noise;
};
struct split_ctx {
   int inv;
   int imid;
   int iside;
   int delta;
   int itheta;
   int qalloc;
};

#ifdef SUPPORT_CTX_SPLIT 

#endif /* SUPPORT_CTX_SPLIT */
static unsigned quant_band_n1(struct band_ctx *ctx, celt_norm *X, celt_norm *Y, int b,
      celt_norm *lowband_out)
{
   int c;
   int stereo;
   celt_norm *x = X;
   int encode;
   ec_ctx *ec;

   encode = ctx->encode;
   ec = ctx->ec;

   stereo = Y != NULL;
   c=0; do {
      int sign=0;
      if (ctx->remaining_bits>=1<<BITRES)
      {
         if (encode)
         {
            sign = x[0]<0;
            ec_enc_bits(ec, sign, 1);
         }
#ifdef ENABLE_DECODE
         else {
            sign = ec_dec_bits(ec, 1);
         }
#endif 
         ctx->remaining_bits -= 1<<BITRES;
         b-=1<<BITRES;
      }
      if (ctx->resynth)
         x[0] = sign ? -NORM_SCALING : NORM_SCALING;
      x = Y;
   } while (++c<1+stereo);
   if (lowband_out)
      lowband_out[0] = SHR16(X[0],4);
   return 1;
}

static int compute_qn(int N, int b, int offset, int pulse_cap, int stereo)
{
	static const opus_int16 exp2_table8[8] =
	{ 16384, 17866, 19483, 21247, 23170, 25267, 27554, 30048 };
	int qn, qb;
	int N2 = 2 * N - 1;
	if (stereo && N == 2)
		N2--;
	/* The upper limit ensures that in a stereo split with itheta==16384, we'll
	always have enough bits left over to code at least one pulse in the
	side; otherwise it would collapse, since it doesn't get folded. */
	qb = celt_sudiv(b + N2 * offset, N2);
	qb = IMIN(b - pulse_cap - (4 << BITRES), qb);

	qb = IMIN(8 << BITRES, qb);

	if (qb<(1 << BITRES >> 1)) {
		qn = 1;
	}
	else {
		qn = exp2_table8[qb & 0x7] >> (14 - (qb >> BITRES));
		qn = (qn + 1) >> 1 << 1;
	}
	celt_assert(qn <= 256);
	return qn;
}
#ifdef ENABLE_STEREO
static void intensity_stereo(const CELTMode *m, celt_norm * OPUS_RESTRICT X, const celt_norm * OPUS_RESTRICT Y, const celt_ener *bandE, int bandID, int N)
{
	int i = bandID;
	int j;
	opus_val16 a1, a2;
	opus_val16 left, right;
	opus_val16 norm;
#ifdef FIXED_POINT
	int shift = celt_zlog2(MAX32(bandE[i], bandE[i + m->nbEBands])) - 13;
#endif
	left = VSHR32(bandE[i], shift);
	right = VSHR32(bandE[i + m->nbEBands], shift);
	norm = EPSILON + celt_sqrt(EPSILON + MULT16_16(left, left) + MULT16_16(right, right));
	a1 = DIV32_16(SHL32(EXTEND32(left), 14), norm);
	a2 = DIV32_16(SHL32(EXTEND32(right), 14), norm);
	for (j = 0; j<N; j++)
	{
		celt_norm r, l;
		l = X[j];
		r = Y[j];
		X[j] = EXTRACT16(SHR32(MAC16_16(MULT16_16(a1, l), a2, r), 14));
		/* Side is not encoded, no need to calculate */
	}
}
#endif

int stereo_itheta(const celt_norm *X, const celt_norm *Y, int stereo, int N, int arch)
{
	int i;
	int itheta;
	opus_val16 mid, side;
	opus_val32 Emid, Eside;

	Emid = Eside = EPSILON;
	if (stereo)
	{
		for (i = 0; i<N; i++)
		{
			celt_norm m, s;
			m = ADD16(SHR16(X[i], 1), SHR16(Y[i], 1));
			s = SUB16(SHR16(X[i], 1), SHR16(Y[i], 1));
			Emid = MAC16_16(Emid, m, m);
			Eside = MAC16_16(Eside, s, s);
		}
	}
	else {
		Emid += celt_inner_prod(X, X, N, arch);
		Eside += celt_inner_prod(Y, Y, N, arch);
	}
	mid = celt_sqrt(Emid);
	side = celt_sqrt(Eside);
#ifdef FIXED_POINT
	/* 0.63662 = 2/pi */
	itheta = MULT16_16_Q15(QCONST16(0.63662f, 15), celt_atan2p(side, mid));
#else
	itheta = (int)floorf(.5f + 16384 * 0.63662f*fast_atan2f(side, mid));
#endif

	return itheta;
}
#ifdef ENABLE_STEREO
static void stereo_split(celt_norm * OPUS_RESTRICT X, celt_norm * OPUS_RESTRICT Y, int N)
{
	int j;
	for (j = 0; j<N; j++)
	{
		opus_val32 r, l;
		l = MULT16_16(QCONST16(.70710678f, 15), X[j]);
		r = MULT16_16(QCONST16(.70710678f, 15), Y[j]);
		X[j] = EXTRACT16(SHR32(ADD32(l, r), 15));
		Y[j] = EXTRACT16(SHR32(SUB32(r, l), 15));
	}
}
#endif
static void compute_theta(struct band_ctx *ctx, struct split_ctx *sctx,
	celt_norm *X, celt_norm *Y, int N, int *b, int B, int B0,
	int LM,
	int stereo, int *fill)
{
	int qn;
	int itheta = 0;
	int delta;
	int imid, iside;
	int qalloc;
	int pulse_cap;
	int offset;
	opus_int32 tell;
	int inv = 0;
	int encode;
	const CELTMode *m;
	int i;
	//int intensity;
	ec_ctx *ec;
	//const celt_ener *bandE;

	encode = ctx->encode;
	m = ctx->m;
	i = ctx->i;
	//intensity = ctx->intensity;
	ec = ctx->ec;
	//bandE = ctx->bandE;

	/* Decide on the resolution to give to the split parameter theta */
	pulse_cap = m->logN[i] + LM * (1 << BITRES);
	offset = (pulse_cap >> 1) - (stereo&&N == 2 ? QTHETA_OFFSET_TWOPHASE : QTHETA_OFFSET);
	qn = compute_qn(N, *b, offset, pulse_cap, stereo);
#ifdef ENABLE_STEREO
	if (stereo && i >= intensity)
		qn = 1;
#endif // ENABLE_STEREO
	if (encode)
	{
		/* theta is the atan() of the ratio between the (normalized)
		side and mid. With just that parameter, we can re-scale both
		mid and side because we know that 1) they have unit norm and
		2) they are orthogonal. */
		itheta = stereo_itheta(X, Y, stereo, N, ctx->arch);
	}
	tell = ec_tell_frac(ec);
	if (qn != 1)
	{
		if (encode)
		{
			if (!stereo || ctx->theta_round == 0)
			{
				itheta = (itheta*(opus_int32)qn + 8192) >> 14;
				if (!stereo && ctx->avoid_split_noise && itheta > 0 && itheta < qn)
				{
					/* Check if the selected value of theta will cause the bit allocation
					to inject noise on one side. If so, make sure the energy of that side
					is zero. */
					int unquantized = celt_udiv((opus_int32)itheta * 16384, qn);
					imid = bitexact_cos((opus_int16)unquantized);
					iside = bitexact_cos((opus_int16)(16384 - unquantized));
					delta = FRAC_MUL16((N - 1) << 7, bitexact_log2tan(iside, imid));
					if (delta > *b)
						itheta = qn;
					else if (delta < -*b)
						itheta = 0;
				}
			}
			else {
				int down;
				/* Bias quantization towards itheta=0 and itheta=16384. */
				int bias = itheta > 8192 ? 32767 / qn : -32767 / qn;
				down = IMIN(qn - 1, IMAX(0, (itheta*(opus_int32)qn + bias) >> 14));
				if (ctx->theta_round < 0)
					itheta = down;
				else
					itheta = down + 1;
			}
		}
#ifdef ENABLE_STEREO
		/* Entropy coding of the angle. We use a uniform pdf for the
		time split, a step for stereo, and a triangular one for the rest. */
		if (stereo && N>2)
		{
			int p0 = 3;
			int x = itheta;
			int x0 = qn / 2;
			int ft = p0 * (x0 + 1) + x0;
			/* Use a probability of p0 up to itheta=8192 and then use 1 after */
			if (encode)
			{
				ec_encode(ec, x <= x0 ? p0 * x : (x - 1 - x0) + (x0 + 1)*p0, x <= x0 ? p0 * (x + 1) : (x - x0) + (x0 + 1)*p0, ft);
			}
			else {
				int fs;
				fs = ec_decode(ec, ft);
				if (fs<(x0 + 1)*p0)
					x = fs / p0;
				else
					x = x0 + 1 + (fs - (x0 + 1)*p0);
				ec_dec_update(ec, x <= x0 ? p0 * x : (x - 1 - x0) + (x0 + 1)*p0, x <= x0 ? p0 * (x + 1) : (x - x0) + (x0 + 1)*p0, ft);
				itheta = x;
			}
		}
		else 
#endif // ENABLE_STEREO      
        if (B0>1 || stereo) 
		{
			/* Uniform pdf */
			ec_enc_uint(ec, itheta, qn + 1);
		}
		else 
		{
			int fs = 1, ft;
			ft = ((qn >> 1) + 1)*((qn >> 1) + 1);
                        int fl;

                        fs = itheta <= (qn >> 1) ? itheta + 1 : qn + 1 - itheta;
                        fl = itheta <= (qn >> 1) ? itheta * (itheta + 1) >> 1 :
                                ft - ((qn + 1 - itheta)*(qn + 2 - itheta) >> 1);

                        ec_encode(ec, fl, fl + fs, ft);
		}
		celt_assert(itheta >= 0);
		itheta = celt_udiv((opus_int32)itheta * 16384, qn);
#ifdef ENABLE_STEREO
		if (encode && stereo)
		{
			if (itheta == 0)
				intensity_stereo(m, X, Y, bandE, i, N);
			else
				stereo_split(X, Y, N);
		}
#endif // ENABLE_STEREO
		/* NOTE: Renormalising X and Y *may* help fixed-point a bit at very high rate.
		Let's do that at higher complexity */
	}
#ifdef ENABLE_STEREO
	else 
          if (stereo) {
		if (encode)
		{
			inv = itheta > 8192 && !ctx->disable_inv;
			if (inv)
			{
				int j;
				for (j = 0; j<N; j++)
					Y[j] = -Y[j];
			}
			intensity_stereo(m, X, Y, bandE, i, N);
		}
		if (*b>2 << BITRES && ctx->remaining_bits > 2 << BITRES)
		{
			if (encode)
				ec_enc_bit_logp(ec, inv, 2);
#ifdef ENABLE_DECODE
			else
				inv = ec_dec_bit_logp(ec, 2);
#endif // ENABLE_DECODE
		}
		else
			inv = 0;
		/* inv flag override to avoid problems with downmixing. */
		if (ctx->disable_inv)
			inv = 0;
		itheta = 0;
	}
#endif // ENABLE_STEREO
	qalloc = ec_tell_frac(ec) - tell;
	*b -= qalloc;

	if (itheta == 0)
	{
		imid = 32767;
		iside = 0;
		*fill &= (1 << B) - 1;
		delta = -16384;
	}
	else if (itheta == 16384)
	{
		imid = 0;
		iside = 32767;
		*fill &= ((1 << B) - 1) << B;
		delta = 16384;
	}
	else {
		imid = bitexact_cos((opus_int16)itheta);
		iside = bitexact_cos((opus_int16)(16384 - itheta));
		/* This is the mid vs side allocation that minimizes squared error
		in that band. */
		delta = FRAC_MUL16((N - 1) << 7, bitexact_log2tan(iside, imid));
	}

	sctx->inv = inv;
	sctx->imid = imid;
	sctx->iside = iside;
	sctx->delta = delta;
	sctx->itheta = itheta;
	sctx->qalloc = qalloc;
}

/* This function is responsible for encoding and decoding a mono partition.
   It can split the band in two and transmit the energy difference with
   the two half-bands. It can be called recursively so bands can end up being
   split in 8 parts. */
static unsigned quant_partition(struct band_ctx *ctx, celt_norm *X,
      int N, int b, int B, celt_norm *lowband,
      int LM,
      opus_val16 gain, int fill)
{
   int q;
   int curr_bits;
   unsigned cm=0;
   //int encode;
   const CELTMode *m;
   int i;
   int spread;
   ec_ctx *ec;

   //encode = ctx->encode;
   m = ctx->m;
   i = ctx->i;
   spread = ctx->spread;
   ec = ctx->ec;

#ifdef SUPPORT_CTX_SPLIT
   const unsigned char *cache;
   int imid=0, iside=0;
   int B0=B;
   opus_val16 mid=0, side=0;
   celt_norm *Y=NULL;
   /* If we need 1.5 more bit than we can produce, split the band in two. */
   cache = m->cache.bits + m->cache.index[(LM+1)*m->nbEBands+i];
   if (LM != -1 && b > cache[cache[0]]+12 && N>2)
   {
      int mbits, sbits, delta;
      int itheta;
      int qalloc;
      struct split_ctx sctx;
      celt_norm *next_lowband2=NULL;
      opus_int32 rebalance;

      N >>= 1;
      Y = X+N;
      LM -= 1;
      if (B==1)
         fill = (fill&1)|(fill<<1);
      B = (B+1)>>1;

      compute_theta(ctx, &sctx, X, Y, N, &b, B, B0, LM, 0, &fill);
      imid = sctx.imid;
      iside = sctx.iside;
      delta = sctx.delta;
      itheta = sctx.itheta;
      qalloc = sctx.qalloc;
#ifdef FIXED_POINT
      mid = imid;
      side = iside;
#else
      mid = (1.f/32768)*imid;
      side = (1.f/32768)*iside;
#endif

      /* Give more bits to low-energy MDCTs than they would otherwise deserve */
      if (B0>1 && (itheta&0x3fff))
      {
         if (itheta > 8192)
            /* Rough approximation for pre-echo masking */
            delta -= delta>>(4-LM);
         else
            /* Corresponds to a forward-masking slope of 1.5 dB per 10 ms */
            delta = IMIN(0, delta + (N<<BITRES>>(5-LM)));
      }
      mbits = IMAX(0, IMIN(b, (b-delta)/2));
      sbits = b-mbits;
      ctx->remaining_bits -= qalloc;

      if (lowband)
         next_lowband2 = lowband+N; /* >32-bit split case */

      rebalance = ctx->remaining_bits;
      if (mbits >= sbits)
      {
         cm = quant_partition(ctx, X, N, mbits, B, lowband, LM,
               MULT16_16_P15(gain,mid), fill);
         rebalance = mbits - (rebalance-ctx->remaining_bits);
         if (rebalance > 3<<BITRES && itheta!=0)
            sbits += rebalance - (3<<BITRES);
         cm |= quant_partition(ctx, Y, N, sbits, B, next_lowband2, LM,
               MULT16_16_P15(gain,side), fill>>B)<<(B0>>1);
      } else {
         cm = quant_partition(ctx, Y, N, sbits, B, next_lowband2, LM,
               MULT16_16_P15(gain,side), fill>>B)<<(B0>>1);
         rebalance = sbits - (rebalance-ctx->remaining_bits);
         if (rebalance > 3<<BITRES && itheta!=16384)
            mbits += rebalance - (3<<BITRES);
         cm |= quant_partition(ctx, X, N, mbits, B, lowband, LM,
               MULT16_16_P15(gain,mid), fill);
      }
   } else 
#endif /* SUPPORT_CTX_SPLIT */
   {
      /* This is the basic no-split case */
      q = bits2pulses(m, i, LM, b);
      curr_bits = pulses2bits(m, i, LM, q);
      ctx->remaining_bits -= curr_bits;

      /* Ensures we can never bust the budget */
      while (ctx->remaining_bits < 0 && q > 0)
      {
         ctx->remaining_bits += curr_bits;
         q--;
         curr_bits = pulses2bits(m, i, LM, q);
         ctx->remaining_bits -= curr_bits;
      }

      if (q!=0)
      {
         int K = get_pulses(q);

         /* Finally do the actual quantization */
         cm = alg_quant(X, N, K, spread, B, ec, gain, ctx->resynth, ctx->arch);
      } 
   }

   return cm;
}


/* This function is responsible for encoding and decoding a band for the mono case. */
static unsigned quant_band(struct band_ctx *ctx, celt_norm *X,
      int N, int b, int B, celt_norm *lowband,
      int LM, celt_norm *lowband_out,
      opus_val16 gain, celt_norm *lowband_scratch, int fill)
{
   int N_B=N;
   int B0=B;
   int time_divide=0;
   int recombine=0;
   int longBlocks;
   unsigned cm=0;
   int k;
   //int encode;
   int tf_change;

   //encode = ctx->encode;
   tf_change = ctx->tf_change;

   longBlocks = B0==1;

   N_B = celt_udiv(N_B, B);

   /* Special case for one sample */
   if (N==1)
   {
      return quant_band_n1(ctx, X, NULL, b, lowband_out);
   }

   if (tf_change>0)
      recombine = tf_change;
   /* Band recombining to increase frequency resolution */

   if (lowband_scratch && lowband && (recombine || ((N_B&1) == 0 && tf_change<0) || B0>1))
   {
      OPUS_COPY(lowband_scratch, lowband, N);
      lowband = lowband_scratch;
   }

   for (k=0;k<recombine;k++)
   {
      static const unsigned char bit_interleave_table[16]={
            0,1,1,1,2,3,3,3,2,3,3,3,2,3,3,3
      };
      haar1(X, N>>k, 1<<k);
      if (lowband)
         haar1(lowband, N>>k, 1<<k);
      fill = bit_interleave_table[fill&0xF]|bit_interleave_table[fill>>4]<<2;
   }
   B>>=recombine;
   N_B<<=recombine;

   /* Increasing the time resolution */
   while ((N_B&1) == 0 && tf_change<0)
   {
      haar1(X, N_B, B);
      if (lowband)
         haar1(lowband, N_B, B);
      fill |= fill<<B;
      B <<= 1;
      N_B >>= 1;
      time_divide++;
      tf_change++;
   }
   B0=B;

   /* Reorganize the samples in time order instead of frequency order */
   if (B0>1)
   {
     deinterleave_hadamard(X, N_B>>recombine, B0<<recombine, longBlocks);
      if (lowband)
         deinterleave_hadamard(lowband, N_B>>recombine, B0<<recombine, longBlocks);
   }

   cm = quant_partition(ctx, X, N, b, B, lowband, LM, gain, fill);

   return cm;
}

void quant_all_bands(int encode, const CELTMode *m, int start, int end,
      celt_norm *X_, celt_norm *Y_, unsigned char *collapse_masks,
      const celt_ener *bandE, int *pulses, int shortBlocks, int spread,
      int dual_stereo, int intensity, int *tf_res, opus_int32 total_bits,
      opus_int32 balance, ec_ctx *ec, int LM, int codedBands,
      opus_uint32 *seed, int complexity, int arch, int disable_inv)
{
   int i;
   opus_int32 remaining_bits;
   const opus_int16 * OPUS_RESTRICT eBands = m->eBands;
   celt_norm * OPUS_RESTRICT norm;
   VARDECL(celt_norm, _norm);
   celt_norm *lowband_scratch;
   int B;
   int M;
   int lowband_offset;
   int update_lowband = 1;
   int C = Y_ != NULL ? 2 : 1;
   int norm_offset;
   int theta_rdo = encode && Y_!=NULL && !dual_stereo && complexity>=8;
#ifdef RESYNTH
   int resynth = 1;
#else
   int resynth = !encode || theta_rdo;
#endif
   struct band_ctx ctx;
   SAVE_STACK;

   M = 1<<LM;
   B = shortBlocks ? M : 1;
   norm_offset = M*eBands[start];
   /* No need to allocate norm for the last band because we don't need an
      output in that band. */
   ALLOC(_norm, C*(M*eBands[m->nbEBands-1]-norm_offset), celt_norm);
   norm = _norm;

      lowband_scratch = X_+M*eBands[m->nbEBands-1];

   lowband_offset = 0;
   ctx.bandE = bandE;
   ctx.ec = ec;
   ctx.encode = encode;
   ctx.intensity = intensity;
   ctx.m = m;
   ctx.seed = *seed;
   ctx.spread = spread;
   ctx.arch = arch;
   ctx.disable_inv = disable_inv;
   ctx.resynth = resynth;
   ctx.theta_round = 0;
   /* Avoid injecting noise in the first band on transients. */
   ctx.avoid_split_noise = B > 1;
   for (i=start;i<end;i++)
   {
      opus_int32 tell;
      int b;
      int N;
      opus_int32 curr_balance;
      int effective_lowband=-1;
      celt_norm * OPUS_RESTRICT X;
      int tf_change=0;
      unsigned x_cm;
      int last;

      ctx.i = i;
      last = (i==end-1);

      X = X_+M*eBands[i];

      N = M*eBands[i+1]-M*eBands[i];
      celt_assert(N > 0);
      tell = ec_tell_frac(ec);

      /* Compute how many bits we want to allocate to this band */
      if (i != start)
         balance -= tell;
      remaining_bits = total_bits-tell-1;
      ctx.remaining_bits = remaining_bits;
      if (i <= codedBands-1)
      {
         curr_balance = celt_sudiv(balance, IMIN(3, codedBands-i));
         b = IMAX(0, IMIN(16383, IMIN(remaining_bits+1,pulses[i]+curr_balance)));
      } else {
         b = 0;
      }

#ifndef DISABLE_UPDATE_DRAFT
      if (resynth && (M*eBands[i]-N >= M*eBands[start] || i==start+1) && (update_lowband || lowband_offset==0))
            lowband_offset = i;
      if (i == start+1)
         special_hybrid_folding(m, norm, norm2, start, M, dual_stereo);
#else
      if (resynth && M*eBands[i]-N >= M*eBands[start] && (update_lowband || lowband_offset==0))
            lowband_offset = i;
#endif

      tf_change = tf_res[i];
      ctx.tf_change = tf_change;
      if (i>=m->effEBands)
      {
         X=norm;
         lowband_scratch = NULL;
      }
      if (last && !theta_rdo)
         lowband_scratch = NULL;

      /* Get a conservative estimate of the collapse_mask's for the bands we're
         going to be folding from. */
      if (lowband_offset != 0 && (spread!=SPREAD_AGGRESSIVE || B>1 || tf_change<0))
      {
         int fold_start;
         int fold_end;
         int fold_i;
         /* This ensures we never repeat spectral content within one band */
         effective_lowband = IMAX(0, M*eBands[lowband_offset]-norm_offset-N);
         fold_start = lowband_offset;
         while(M*eBands[--fold_start] > effective_lowband+norm_offset);
         fold_end = lowband_offset-1;
#ifndef DISABLE_UPDATE_DRAFT
         while(++fold_end < i && M*eBands[fold_end] < effective_lowband+norm_offset+N);
#else
         while(M*eBands[++fold_end] < effective_lowband+norm_offset+N);
#endif
         x_cm =  0;
         fold_i = fold_start; do {
           x_cm |= collapse_masks[fold_i*C+0];
         } while (++fold_i<fold_end);
      }
      /* Otherwise, we'll be using the LCG to fold, so all blocks will (almost
         always) be non-zero. */
      else
         x_cm  = (1<<B)-1;

	
            x_cm = quant_band(&ctx, X, N, b, B,
                  effective_lowband != -1 ? norm+effective_lowband : NULL, LM,
                  last?NULL:norm+M*eBands[i]-norm_offset, Q15ONE, lowband_scratch, x_cm);
 
     
      collapse_masks[i*C+0] = (unsigned char)x_cm;
      balance += pulses[i] + tell;

      /* Update the folding position only as long as we have 1 bit/sample depth. */
      update_lowband = b>(N<<BITRES);
      /* We only need to avoid noise on a split for the first band. After that, we
         have folding. */
      ctx.avoid_split_noise = 0;
   }
   *seed = ctx.seed;
   FREE1(_norm, C*(M*eBands[m->nbEBands - 1] - norm_offset), celt_norm);
   RESTORE_STACK;
}