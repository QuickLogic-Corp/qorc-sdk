/* Copyright (c) 2007-2008 CSIRO
   Copyright (c) 2007-2010 Xiph.Org Foundation
   Copyright (c) 2008 Gregory Maxwell
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
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,F
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, ORF
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define CELT_ENCODER_C
#include "ql_opus.h"

#include "cpu_support.h"
#include "os_support.h"
#include "mdct.h"
#include <math.h>
#include "celt.h"
#include "pitch.h"
#include "bands.h"
#include "modes.h"
#include "entcode.h"
#include "quant_bands.h"
#include "rate.h"
#include "stack_alloc.h"
#include "mathops.h"
#include "float_cast.h"
#include <stdarg.h>
#include "celt_lpc.h"
#include "vq.h"


/** Encoder state
 @brief Encoder state
 */
struct OpusCustomEncoder {
   const OpusCustomMode *mode;     /**< Mode used by the encoder */
   int channels;
   int stream_channels;

   int force_intra;
   int clip;
   int disable_pf;
   int complexity;
   int upsample;
   int start, end;

   opus_int32 bitrate;
   int vbr;
   int signalling;
   int constrained_vbr;      /* If zero, VBR can do whatever it likes with the rate */
   int loss_rate;
   int lsb_depth;
   int lfe;
   int disable_inv;
   int arch;

   /* Everything beyond this point gets cleared on a reset */
#define ENCODER_RESET_START rng

   opus_uint32 rng;
   int spread_decision;
   opus_val32 delayedIntra;
   int tonal_average;
   int lastCodedBands;
   int hf_average;
   int tapset_decision;

   int prefilter_period;
   opus_val16 prefilter_gain;
   int prefilter_tapset;
#ifdef RESYNTH
   int prefilter_period_old;
   opus_val16 prefilter_gain_old;
   int prefilter_tapset_old;
#endif
   int consec_transient;
   AnalysisInfo analysis;

 //  SILKInfo silk_info;

   opus_val32 preemph_memE[2];
   opus_val32 preemph_memD[2];

   /* VBR-related parameters */
   opus_int32 vbr_reservoir;
   opus_int32 vbr_drift;
   opus_int32 vbr_offset;
   opus_int32 vbr_count;
   opus_val32 overlap_max;
   opus_val16 stereo_saving;
   int intensity;
   opus_val16 *energy_mask;
   opus_val16 spec_avg;

#ifdef RESYNTH
   /* +MAX_PERIOD/2 to make space for overlap */
   celt_sig syn_mem[2][2*MAX_PERIOD+MAX_PERIOD/2];
#endif

   celt_sig in_mem[1]; /* Size = channels*mode->overlap */
   /* celt_sig prefilter_mem[],  Size = channels*COMBFILTER_MAXPERIOD */
   /* opus_val16 oldBandE[],     Size = channels*mode->nbEBands */
   /* opus_val16 oldLogE[],      Size = channels*mode->nbEBands */
   /* opus_val16 oldLogE2[],     Size = channels*mode->nbEBands */
   /* opus_val16 energyError[],  Size = channels*mode->nbEBands */
};

int celt_encoder_get_size(int channels)
{
   CELTMode *mode = opus_custom_mode_create(48000, 960, NULL);
   return opus_custom_encoder_get_size(mode, channels);
}

//OPUS_CUSTOM_NOSTATIC int opus_custom_encoder_get_size(const CELTMode *mode, int channels)
int opus_custom_encoder_get_size(const CELTMode *mode, int channels)
{
   int size = sizeof(struct CELTEncoder)
         + (channels*mode->overlap-1)*sizeof(celt_sig)    /* celt_sig in_mem[channels*mode->overlap]; */
         + channels*COMBFILTER_MAXPERIOD*sizeof(celt_sig) /* celt_sig prefilter_mem[channels*COMBFILTER_MAXPERIOD]; */
         + 4*channels*mode->nbEBands*sizeof(opus_val16);  /* opus_val16 oldBandE[channels*mode->nbEBands]; */
                                                          /* opus_val16 oldLogE[channels*mode->nbEBands]; */
                                                          /* opus_val16 oldLogE2[channels*mode->nbEBands]; */
                                                          /* opus_val16 energyError[channels*mode->nbEBands]; */
   return size;
}

//#define QL_OPUS_ENC_COMPLEXITY (4)


int opus_celt_resest_state(CELTEncoder * OPUS_RESTRICT st);
static int opus_custom_encoder_init_arch(CELTEncoder *st, const CELTMode *mode,
                                         int channels, int arch)
{
   if (channels < 0 || channels > 2)
      return OPUS_BAD_ARG;

   if (st==NULL || mode==NULL)
      return OPUS_ALLOC_FAIL;

   OPUS_CLEAR((char*)st, opus_custom_encoder_get_size(mode, channels));

   st->mode = mode;
   st->stream_channels = st->channels = channels;

   st->upsample = 1;
   st->start = 0;
   st->end = st->mode->effEBands;
   st->signalling = 1;
   st->arch = arch;

   st->constrained_vbr = 1;
   st->clip = 1;

   st->bitrate = OPUS_BITRATE_MAX;
   st->vbr = 0;
   st->force_intra  = 0;
   //st->complexity = 5;
   st->lsb_depth=24;
   st->signalling = 0;

   st->complexity = QL_OPUS_ENC_COMPLEXITY;
   opus_celt_resest_state(st);

   return OPUS_OK;
}


int celt_encoder_init(CELTEncoder *st, opus_int32 sampling_rate, int channels,
                      int arch)
{
   int ret;
   ret = opus_custom_encoder_init_arch(st,
           opus_custom_mode_create(48000, 960, NULL), channels, arch);
   if (ret != OPUS_OK)
      return ret;
   st->upsample = resampling_factor(sampling_rate);
#define OPUS_ENDBAND_BANDWIDTH_WIDEBAND (17)
   st->end = OPUS_ENDBAND_BANDWIDTH_WIDEBAND;
   st->lsb_depth = 16; 
#define OPUS_CELT_SET_START_BAND (0)
   st->start = OPUS_CELT_SET_START_BAND;
   st->loss_rate = 0;
   
   return OPUS_OK;
}

#if (QL_OPUS_ENC_COMPLEXITY >=1 )
static int transient_analysis(const opus_val32 * OPUS_RESTRICT in, int len, int C,
                              opus_val16 *tf_estimate, int *tf_chan, int allow_weak_transients,
                              int *weak_transient)
{
   int i;
   VARDECL(opus_val16, tmp);
   opus_val32 mem0,mem1;
   int is_transient = 0;
   opus_int32 mask_metric = 0;
   int c;
   opus_val16 tf_max;
   int len2;
   /* Forward masking: 6.7 dB/ms. */
#ifdef FIXED_POINT
   int forward_shift = 4;
#else
   opus_val16 forward_decay = QCONST16(.0625f,15);
#endif
   /* Table of 6*64/x, trained on real data to minimize the average error */
   static const unsigned char inv_table[128] = {
         255,255,156,110, 86, 70, 59, 51, 45, 40, 37, 33, 31, 28, 26, 25,
          23, 22, 21, 20, 19, 18, 17, 16, 16, 15, 15, 14, 13, 13, 12, 12,
          12, 12, 11, 11, 11, 10, 10, 10,  9,  9,  9,  9,  9,  9,  8,  8,
           8,  8,  8,  7,  7,  7,  7,  7,  7,  6,  6,  6,  6,  6,  6,  6,
           6,  6,  6,  6,  6,  6,  6,  6,  6,  5,  5,  5,  5,  5,  5,  5,
           5,  5,  5,  5,  5,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
           4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  3,  3,
           3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  2,
   };
   SAVE_STACK;
   ALLOC(tmp, len, opus_val16);
   OPUS_CLEAR(tmp, len);
   *weak_transient = 0;
   /* For lower bitrates, let's be more conservative and have a forward masking
      decay of 3.3 dB/ms. This avoids having to code transients at very low
      bitrate (mostly for hybrid), which can result in unstable energy and/or
      partial collapse. */
   if (allow_weak_transients)
   {
#ifdef FIXED_POINT
      forward_shift = 5;
#else
      forward_decay = QCONST16(.03125f,15);
#endif
   }
   len2=len/2;
   for (c=0;c<C;c++)
   {
      opus_val32 mean;
      opus_int32 unmask=0;
      opus_val32 norm;
      opus_val16 maxE;
      mem0=0.0f;
      mem1=0.0f;
      /* High-pass filter: (1 - 2*z^-1 + z^-2) / (1 - z^-1 + .5*z^-2) */
      for (i=0;i<len;i++)
      {
         opus_val32 x,y;
         x = SHR32(in[i+c*len],SIG_SHIFT);
         y = ADD32(mem0, x);
#ifdef FIXED_POINT
         mem0 = mem1 + y - SHL32(x,1);
         mem1 = x - SHR32(y,1);
#else
         mem0 = mem1 + y - 2*x;
         mem1 = x - .5f*y;
#endif
         tmp[i] = SROUND16(y, 2);
         /*printf("%f ", tmp[i]);*/
      }
      /*printf("\n");*/
      /* First few samples are bad because we don't propagate the memory */
      OPUS_CLEAR(tmp, 12);

#ifdef FIXED_POINT
      /* Normalize tmp to max range */
      {
         int shift=0;
         shift = 14-celt_ilog2(MAX16(1, celt_maxabs16(tmp, len)));
         if (shift!=0)
         {
            for (i=0;i<len;i++)
               tmp[i] = SHL16(tmp[i], shift);
         }
      }
#endif

      mean=0.0f;
      mem0=0.0f;
      /* Grouping by two to reduce complexity */
      /* Forward pass to compute the post-echo threshold*/
      for (i=0;i<len2;i++)
      {
         opus_val16 x2 = PSHR32(MULT16_16(tmp[2*i],tmp[2*i]) + MULT16_16(tmp[2*i+1],tmp[2*i+1]),16);
         mean += x2;
#ifdef FIXED_POINT
         /* FIXME: Use PSHR16() instead */
         tmp[i] = mem0 + PSHR32(x2-mem0,forward_shift);
#else
         tmp[i] = mem0 + MULT16_16_P15(forward_decay,x2-mem0);
#endif
         mem0 = tmp[i];
      }

      mem0=0.0f;
      maxE=0.0f;
      /* Backward pass to compute the pre-echo threshold */
      for (i=len2-1;i>=0;i--)
      {
         /* Backward masking: 13.9 dB/ms. */
#ifdef FIXED_POINT
         /* FIXME: Use PSHR16() instead */
         tmp[i] = mem0 + PSHR32(tmp[i]-mem0,3);
#else
         tmp[i] = mem0 + MULT16_16_P15(QCONST16(0.125f,15),tmp[i]-mem0);
#endif
         mem0 = tmp[i];
         maxE = MAX16(maxE, mem0);
      }
      /*for (i=0;i<len2;i++)printf("%f ", tmp[i]/mean);printf("\n");*/

      /* Compute the ratio of the "frame energy" over the harmonic mean of the energy.
         This essentially corresponds to a bitrate-normalized temporal noise-to-mask
         ratio */

      /* As a compromise with the old transient detector, frame energy is the
         geometric mean of the energy and half the max */
#ifdef FIXED_POINT
      /* Costs two sqrt() to avoid overflows */
      mean = MULT16_16(celt_sqrt(mean), celt_sqrt(MULT16_16(maxE,len2>>1)));
#else
      mean = celt_sqrt(mean * maxE*.5f*len2);
#endif
      /* Inverse of the mean energy in Q15+6 */
      norm = SHL32(EXTEND32(len2),6+14)/ADD32(EPSILON,SHR32(mean,1));
      /* Compute harmonic mean discarding the unreliable boundaries
         The data is smooth, so we only take 1/4th of the samples */
      unmask=0;
      for (i=12;i<len2-5;i+=4)
      {
         int id;
#ifdef FIXED_POINT
         id = MAX32(0,MIN32(127,MULT16_32_Q15(tmp[i]+EPSILON,norm))); /* Do not round to nearest */
#else
         id = (int)MAX32(0,MIN32(127,floorf(64*norm*(tmp[i]+EPSILON)))); /* Do not round to nearest */
#endif
         unmask += inv_table[id];
      }
      /*printf("%d\n", unmask);*/
      /* Normalize, compensate for the 1/4th of the sample and the factor of 6 in the inverse table */
      unmask = 64*unmask*4/(6*(len2-17));
      if (unmask>mask_metric)
      {
         *tf_chan = c;
         mask_metric = unmask;
      }
   }
   is_transient = mask_metric>200;
   /* For low bitrates, define "weak transients" that need to be
      handled differently to avoid partial collapse. */
   if (allow_weak_transients && is_transient && mask_metric<600) {
      is_transient = 0;
      *weak_transient = 1;
   }
   /* Arbitrary metric for VBR boost */
   tf_max = MAX16(0,celt_sqrt(27*mask_metric)-42);
   /* *tf_estimate = 1 + MIN16(1, sqrt(MAX16(0, tf_max-30))/20); */
   *tf_estimate = celt_sqrt(MAX32(0, SHL32(MULT16_16(QCONST16(0.0069f,14),MIN16(163,tf_max)),14)-QCONST32(0.139f,28)));
   /*printf("%d %f\n", tf_max, mask_metric);*/

   FREE1(tmp, len, opus_val16);
   RESTORE_STACK;
#ifdef FUZZING
   is_transient = rand()&0x1;
#endif

   /*printf("%d %f %d\n", is_transient, (float)*tf_estimate, tf_max);*/
   return is_transient;
}
#if (QL_OPUS_ENC_COMPLEXITY >= 5)
/* Looks for sudden increases of energy to decide whether we need to patch
   the transient decision */
static int patch_transient_decision(opus_val16 *newE, opus_val16 *oldE, int nbEBands,
      int start, int end, int C)
{
   int i, c;
   opus_val32 mean_diff=0;
   opus_val16 spread_old[26];
   /* Apply an aggressive (-6 dB/Bark) spreading function to the old frame to
      avoid false detection caused by irrelevant bands */
   if (C==1)
   {
      spread_old[start] = oldE[start];
      for (i=start+1;i<end;i++)
         spread_old[i] = MAX16(spread_old[i-1]-QCONST16(1.0f, DB_SHIFT), oldE[i]);
   } else {
      spread_old[start] = MAX16(oldE[start],oldE[start+nbEBands]);
      for (i=start+1;i<end;i++)
         spread_old[i] = MAX16(spread_old[i-1]-QCONST16(1.0f, DB_SHIFT),
                               MAX16(oldE[i],oldE[i+nbEBands]));
   }
   for (i=end-2;i>=start;i--)
      spread_old[i] = MAX16(spread_old[i], spread_old[i+1]-QCONST16(1.0f, DB_SHIFT));
   /* Compute mean increase */
   c=0; do {
      for (i=IMAX(2,start);i<end-1;i++)
      {
         opus_val16 x1, x2;
         x1 = MAX16(0, newE[i + c*nbEBands]);
         x2 = MAX16(0, spread_old[i]);
         mean_diff = ADD32(mean_diff, EXTEND32(MAX16(0, SUB16(x1, x2))));
      }
   } while (++c<C);
   mean_diff = DIV32(mean_diff, C*(end-1-IMAX(2,start)));
   /*printf("%f %f %d\n", mean_diff, max_diff, count);*/
   return mean_diff > QCONST16(1.f, DB_SHIFT);
}
#endif /* QL_OPUS_ENC_COMPLEXITY */
#endif

/** Apply window and compute the MDCT for all sub-frames and
    all channels in a frame */
static void compute_mdcts(const CELTMode *mode, int shortBlocks, celt_sig * OPUS_RESTRICT in,
                          celt_sig * OPUS_RESTRICT out, int C, int CC, int LM, int upsample,
                          int arch)
{
   const int overlap = mode->overlap;
   int N;
   int B;
   int shift;
   int i, b, c;
   if (shortBlocks)
   {
      B = shortBlocks;
      N = mode->shortMdctSize;
      shift = mode->maxLM;
   } else {
      B = 1;
      N = mode->shortMdctSize<<LM;
      shift = mode->maxLM-LM;
   }
   c=0; do {
      for (b=0;b<B;b++)
      {
         /* Interleaving the sub-frames while doing the MDCTs */
         clt_mdct_forward(&mode->mdct, in+c*(B*N+overlap)+b*N,
                          &out[b+c*N*B], mode->window, overlap, shift, B,
                          arch);
      }
   } while (++c<CC);
   if (CC==2&&C==1)
   {
      for (i=0;i<B*N;i++)
         out[i] = ADD32(HALF32(out[i]), HALF32(out[B*N+i]));
   }
   if (upsample != 1)
   {
      c=0; do
      {
         int bound = B*N/upsample;
         for (i=0;i<bound;i++)
            out[c*B*N+i] *= upsample;
         OPUS_CLEAR(&out[c*B*N+bound], B*N-bound);
      } while (++c<C);
   }
}

#ifdef PRE_EMPHASIS_OPT
void celt_preemphasis(const opus_val16 * OPUS_RESTRICT pcmp, celt_sig * OPUS_RESTRICT inp,
                        int N, int CC, int upsample, const opus_val16 *coef, celt_sig *mem, int clip)
{
   int i;
   opus_val16 coef0;
   celt_sig m;
   int Nu;
   celt_sig *inpPtr;

   coef0 = coef[0];
   m = *mem;

   Nu = N/upsample;
   OPUS_CLEAR(inp, N);

   inpPtr = inp;
   for (i = 0; i < Nu; i++)
   {
	   *inpPtr = SCALEIN(*pcmp++);
	   inpPtr += 3;
   }

#ifndef FIXED_POINT
   if (clip)
   {
      /* Clip input to avoid encoding non-portable files */
      for (i=0;i<Nu;i++)
         inp[i*upsample] = MAX32(-65536.f, MIN32(65536.f,inp[i*upsample]));
   }
#else
   (void)clip; /* Avoids a warning about clip being unused. */
#endif
   {
	  inpPtr = inp;
	  
      for (i = 0; i<Nu; i++)
	  {
		  opus_val16 x;
		  
		  x = *inpPtr++;
		  /* Apply pre-emphasis */
		  //*inpPtr++ = x - m; //0th phase no change.
		  m = MULT16_16(coef0, x);

		  //x = *inpPtr; //x is zero
		  *inpPtr = -m; //0th phase no change.
		  //m = MULT16_16(coef0, x);
		  //m = 0;

		  //x = *inpPtr;  //x is zero
		  //*inpPtr = 0;  //output is already zero.
		  //m = 0;
		  inpPtr += 2;
	  }
   }
   *mem = m;
}
#else  //#ifdef PRE_EMPHASIS_OPT
void celt_preemphasis(const opus_val16 * OPUS_RESTRICT pcmp, celt_sig * OPUS_RESTRICT inp,
	int N, int CC, int upsample, const opus_val16 *coef, celt_sig *mem, int clip)
{
	int i;
	opus_val16 coef0;
	celt_sig m;
	int Nu;

	coef0 = coef[0];
	m = *mem;

	/* Fast path for the normal 48kHz case and no clipping */
	if (coef[1] == 0 && upsample == 1 && !clip)
	{
		for (i = 0; i<N; i++)
		{
			opus_val16 x;
			x = SCALEIN(pcmp[CC*i]);
			/* Apply pre-emphasis */
			inp[i] = SHL32(x, SIG_SHIFT) - m;
			m = SHR32(MULT16_16(coef0, x), 15 - SIG_SHIFT);
		}
		*mem = m;
		return;
	}

	Nu = N / upsample;
	if (upsample != 1)
	{
		OPUS_CLEAR(inp, N);
	}
	for (i = 0; i<Nu; i++)
		inp[i*upsample] = SCALEIN(pcmp[CC*i]);

#ifndef FIXED_POINT
	if (clip)
	{
		/* Clip input to avoid encoding non-portable files */
		for (i = 0; i<Nu; i++)
			inp[i*upsample] = MAX32(-65536.f, MIN32(65536.f, inp[i*upsample]));
	}
#else
	(void)clip; /* Avoids a warning about clip being unused. */
#endif
#ifdef CUSTOM_MODES
	if (coef[1] != 0)
	{
		opus_val16 coef1 = coef[1];
		opus_val16 coef2 = coef[2];
		for (i = 0; i<N; i++)
		{
			celt_sig x, tmp;
			x = inp[i];
			/* Apply pre-emphasis */
			tmp = MULT16_16(coef2, x);
			inp[i] = tmp + m;
			m = MULT16_32_Q15(coef1, inp[i]) - MULT16_32_Q15(coef0, tmp);
		}
	}
	else
#endif
	{
		for (i = 0; i<N; i++)
		{
			opus_val16 x;
			x = inp[i];
			/* Apply pre-emphasis */
			inp[i] = SHL32(x, SIG_SHIFT) - m;
			m = SHR32(MULT16_16(coef0, x), 15 - SIG_SHIFT);
		}
	}
	*mem = m;
}
#endif  //#ifdef PRE_EMPHASIS_OPT



static opus_val32 l1_metric(const celt_norm *tmp, int N, int LM, opus_val16 bias)
{
   int i;
   opus_val32 L1;
   L1 = 0;
   for (i=0;i<N;i++)
      L1 += EXTEND32(ABS16(tmp[i]));
   /* When in doubt, prefer good freq resolution */
   L1 = MAC16_32_Q15(L1, LM*bias, L1);
   return L1;

}

static int tf_analysis(const CELTMode *m, int len, int isTransient,
      int *tf_res, int lambda, celt_norm *X, int N0, int LM,
      opus_val16 tf_estimate, int tf_chan, int *importance)
{
   int i;
   VARDECL(int, metric);
   int cost0;
   int cost1;
   VARDECL(int, path0);
   VARDECL(int, path1);
   VARDECL(celt_norm, tmp);
   VARDECL(celt_norm, tmp_1);
   int sel;
   int selcost[2];
   int tf_select=0;
   opus_val16 bias;

   SAVE_STACK;
   bias = MULT16_16_Q14(QCONST16(.04f,15), MAX16(-QCONST16(.25f,14), QCONST16(.5f,14)-tf_estimate));
   /*printf("%f ", bias);*/
   int tmp_size = (m->eBands[len] - m->eBands[len - 1]) << LM;

   ALLOC(metric, len, int);
   ALLOC(tmp, tmp_size, celt_norm);
   ALLOC(tmp_1, tmp_size, celt_norm);
   ALLOC(path0, len, int);
   ALLOC(path1, len, int);

   for (i=0;i<len;i++)
   {
      int k, N;
      int narrow;
      opus_val32 L1, best_L1;
      int best_level=0;
      N = (m->eBands[i+1]-m->eBands[i])<<LM;
      /* band is too narrow to be split down to LM=-1 */
      narrow = (m->eBands[i+1]-m->eBands[i])==1;
      OPUS_COPY(tmp, &X[tf_chan*N0 + (m->eBands[i]<<LM)], N);
      /* Just add the right channel if we're in stereo */
      /*if (C==2)
         for (j=0;j<N;j++)
            tmp[j] = ADD16(SHR16(tmp[j], 1),SHR16(X[N0+j+(m->eBands[i]<<LM)], 1));*/
      L1 = l1_metric(tmp, N, isTransient ? LM : 0, bias);
      best_L1 = L1;
      /* Check the -1 case for transients */
      if (isTransient && !narrow)
      {
         OPUS_COPY(tmp_1, tmp, N);
         haar1(tmp_1, N>>LM, 1<<LM);
         L1 = l1_metric(tmp_1, N, LM+1, bias);
         if (L1<best_L1)
         {
            best_L1 = L1;
            best_level = -1;
         }
      }
      /*printf ("%f ", L1);*/
      for (k=0;k<LM+!(isTransient||narrow);k++)
      {
         int B;

         if (isTransient)
            B = (LM-k-1);
         else
            B = k+1;

         haar1(tmp, N>>k, 1<<k);

         L1 = l1_metric(tmp, N, B, bias);

         if (L1 < best_L1)
         {
            best_L1 = L1;
            best_level = k+1;
         }
      }
      /*printf ("%d ", isTransient ? LM-best_level : best_level);*/
      /* metric is in Q1 to be able to select the mid-point (-0.5) for narrower bands */
      if (isTransient)
         metric[i] = 2*best_level;
      else
         metric[i] = -2*best_level;
      /* For bands that can't be split to -1, set the metric to the half-way point to avoid
         biasing the decision */
      if (narrow && (metric[i]==0 || metric[i]==-2*LM))
         metric[i]-=1;
      /*printf("%d ", metric[i]/2 + (!isTransient)*LM);*/
   }
   /*printf("\n");*/
   /* Search for the optimal tf resolution, including tf_select */
   tf_select = 0;
   for (sel=0;sel<2;sel++)
   {
      cost0 = importance[0]*abs(metric[0]-2*tf_select_table[LM][4*isTransient+2*sel+0]);
      cost1 = importance[0]*abs(metric[0]-2*tf_select_table[LM][4*isTransient+2*sel+1]) + (isTransient ? 0 : lambda);
      for (i=1;i<len;i++)
      {
         int curr0, curr1;
         curr0 = IMIN(cost0, cost1 + lambda);
         curr1 = IMIN(cost0 + lambda, cost1);
         cost0 = curr0 + importance[i]*abs(metric[i]-2*tf_select_table[LM][4*isTransient+2*sel+0]);
         cost1 = curr1 + importance[i]*abs(metric[i]-2*tf_select_table[LM][4*isTransient+2*sel+1]);
      }
      cost0 = IMIN(cost0, cost1);
      selcost[sel]=cost0;
   }
   /* For now, we're conservative and only allow tf_select=1 for transients.
    * If tests confirm it's useful for non-transients, we could allow it. */
   if (selcost[1]<selcost[0] && isTransient)
      tf_select=1;
   cost0 = importance[0]*abs(metric[0]-2*tf_select_table[LM][4*isTransient+2*tf_select+0]);
   cost1 = importance[0]*abs(metric[0]-2*tf_select_table[LM][4*isTransient+2*tf_select+1]) + (isTransient ? 0 : lambda);
   /* Viterbi forward pass */
   for (i=1;i<len;i++)
   {
      int curr0, curr1;
      int from0, from1;

      from0 = cost0;
      from1 = cost1 + lambda;
      if (from0 < from1)
      {
         curr0 = from0;
         path0[i]= 0;
      } else {
         curr0 = from1;
         path0[i]= 1;
      }

      from0 = cost0 + lambda;
      from1 = cost1;
      if (from0 < from1)
      {
         curr1 = from0;
         path1[i]= 0;
      } else {
         curr1 = from1;
         path1[i]= 1;
      }
      cost0 = curr0 + importance[i]*abs(metric[i]-2*tf_select_table[LM][4*isTransient+2*tf_select+0]);
      cost1 = curr1 + importance[i]*abs(metric[i]-2*tf_select_table[LM][4*isTransient+2*tf_select+1]);
   }
   tf_res[len-1] = cost0 < cost1 ? 0 : 1;
   /* Viterbi backward pass to check the decisions */
   for (i=len-2;i>=0;i--)
   {
      if (tf_res[i+1] == 1)
         tf_res[i] = path1[i+1];
      else
         tf_res[i] = path0[i+1];
   }
   /*printf("%d %f\n", *tf_sum, tf_estimate);*/

   FREE1(metric, len, int);
   FREE1(tmp, tmp_size, celt_norm);
   FREE1(tmp_1, tmp_size, celt_norm);
   FREE1(path0, len, int);
   FREE1(path1, len, int);
   RESTORE_STACK;
#ifdef FUZZING
   tf_select = rand()&0x1;
   tf_res[0] = rand()&0x1;
   for (i=1;i<len;i++)
      tf_res[i] = tf_res[i-1] ^ ((rand()&0xF) == 0);
#endif
   return tf_select;
}

static void tf_encode(int start, int end, int isTransient, int *tf_res, int LM, int tf_select, ec_enc *enc)
{
   int curr, i;
   int tf_select_rsv;
   int tf_changed;
   int logp;
   opus_uint32 budget;
   opus_uint32 tell;
   budget = enc->storage*8;
   tell = ec_tell(enc);
   logp = isTransient ? 2 : 4;
   /* Reserve space to code the tf_select decision. */
   tf_select_rsv = LM>0 && tell+logp+1 <= budget;
   budget -= tf_select_rsv;
   curr = tf_changed = 0;
   for (i=start;i<end;i++)
   {
      if (tell+logp<=budget)
      {
         ec_enc_bit_logp(enc, tf_res[i] ^ curr, logp);
         tell = ec_tell(enc);
         curr = tf_res[i];
         tf_changed |= curr;
      }
      else
         tf_res[i] = curr;
      logp = isTransient ? 4 : 5;
   }
   /* Only code tf_select if it would actually make a difference. */
   if (tf_select_rsv &&
         tf_select_table[LM][4*isTransient+0+tf_changed]!=
         tf_select_table[LM][4*isTransient+2+tf_changed])
      ec_enc_bit_logp(enc, tf_select, 1);
   else
      tf_select = 0;
   for (i=start;i<end;i++)
      tf_res[i] = tf_select_table[LM][4*isTransient+2*tf_select+tf_res[i]];
   /*for(i=0;i<end;i++)printf("%d ", isTransient ? tf_res[i] : LM+tf_res[i]);printf("\n");*/
}


static int alloc_trim_analysis(const CELTMode *m, const celt_norm *X,
      const opus_val16 *bandLogE, int end, int LM, int C, int N0,
      AnalysisInfo *analysis, opus_val16 *stereo_saving, opus_val16 tf_estimate,
      int intensity, opus_val16 surround_trim, opus_int32 equiv_rate, int arch)
{
   int i;
   opus_val32 diff=0;
   int c;
   int trim_index;
   opus_val16 trim = QCONST16(5.f, 8);
   opus_val16 logXC, logXC2;
   /* At low bitrate, reducing the trim seems to help. At higher bitrates, it's less
      clear what's best, so we're keeping it as it was before, at least for now. */
   if (equiv_rate < 64000) {
      trim = QCONST16(4.f, 8);
   } else if (equiv_rate < 80000) {
      opus_int32 frac = (equiv_rate-64000) >> 10;
      trim = QCONST16(4.f, 8) + QCONST16(1.f/16.f, 8)*frac;
   }
   if (C==2)
   {
      opus_val16 sum = 0; /* Q10 */
      opus_val16 minXC; /* Q10 */
      /* Compute inter-channel correlation for low frequencies */
      for (i=0;i<8;i++)
      {
         opus_val32 partial;
         partial = celt_inner_prod(&X[m->eBands[i]<<LM], &X[N0+(m->eBands[i]<<LM)],
               (m->eBands[i+1]-m->eBands[i])<<LM, arch);
         sum = ADD16(sum, EXTRACT16(SHR32(partial, 18)));
      }
      sum = MULT16_16_Q15(QCONST16(1.f/8, 15), sum);
      sum = MIN16(QCONST16(1.f, 10), ABS16(sum));
      minXC = sum;
      for (i=8;i<intensity;i++)
      {
         opus_val32 partial;
         partial = celt_inner_prod(&X[m->eBands[i]<<LM], &X[N0+(m->eBands[i]<<LM)],
               (m->eBands[i+1]-m->eBands[i])<<LM, arch);
         minXC = MIN16(minXC, ABS16(EXTRACT16(SHR32(partial, 18))));
      }
      minXC = MIN16(QCONST16(1.f, 10), ABS16(minXC));
      /*printf ("%f\n", sum);*/
      /* mid-side savings estimations based on the LF average*/
      logXC = celt_log2(QCONST32(1.001f, 20)-MULT16_16(sum, sum));
      /* mid-side savings estimations based on min correlation */
      logXC2 = MAX16(HALF16(logXC), celt_log2(QCONST32(1.001f, 20)-MULT16_16(minXC, minXC)));
#ifdef FIXED_POINT
      /* Compensate for Q20 vs Q14 input and convert output to Q8 */
      logXC = PSHR32(logXC-QCONST16(6.f, DB_SHIFT),DB_SHIFT-8);
      logXC2 = PSHR32(logXC2-QCONST16(6.f, DB_SHIFT),DB_SHIFT-8);
#endif

      trim += MAX16(-QCONST16(4.f, 8), MULT16_16_Q15(QCONST16(.75f,15),logXC));
      *stereo_saving = MIN16(*stereo_saving + QCONST16(0.25f, 8), -HALF16(logXC2));
   }

   /* Estimate spectral tilt */
   c=0; do {
      for (i=0;i<end-1;i++)
      {
         diff += bandLogE[i+c*m->nbEBands]*(opus_int32)(2+2*i-end);
      }
   } while (++c<C);
   diff /= C*(end-1);
   /*printf("%f\n", diff);*/
   trim -= MAX32(-QCONST16(2.f, 8), MIN32(QCONST16(2.f, 8), SHR32(diff+QCONST16(1.f, DB_SHIFT),DB_SHIFT-8)/6 ));
   trim -= SHR16(surround_trim, DB_SHIFT-8);
   trim -= 2*SHR16(tf_estimate, 14-8);
#ifndef DISABLE_FLOAT_API
   if (analysis->valid)
   {
      trim -= MAX16(-QCONST16(2.f, 8), MIN16(QCONST16(2.f, 8),
            (opus_val16)(QCONST16(2.f, 8)*(analysis->tonality_slope+.05f))));
   }
#else
   (void)analysis;
#endif

#ifdef FIXED_POINT
   trim_index = PSHR32(trim, 8);
#else
   trim_index = (int)floorf(.5f+trim);
#endif
   trim_index = IMAX(0, IMIN(10, trim_index));
   /*printf("%d\n", trim_index);*/
#ifdef FUZZING
   trim_index = rand()%11;
#endif
   return trim_index;
}

#define MSWAP(a,b) do {opus_val16 tmp = a;a=b;b=tmp;} while(0)
static opus_val16 median_of_5(const opus_val16 *x)
{
   opus_val16 t0, t1, t2, t3, t4;
   t2 = x[2];
   if (x[0] > x[1])
   {
      t0 = x[1];
      t1 = x[0];
   } else {
      t0 = x[0];
      t1 = x[1];
   }
   if (x[3] > x[4])
   {
      t3 = x[4];
      t4 = x[3];
   } else {
      t3 = x[3];
      t4 = x[4];
   }
   if (t0 > t3)
   {
      MSWAP(t0, t3);
      MSWAP(t1, t4);
   }
   if (t2 > t1)
   {
      if (t1 < t3)
         return MIN16(t2, t3);
      else
         return MIN16(t4, t1);
   } else {
      if (t2 < t3)
         return MIN16(t1, t3);
      else
         return MIN16(t2, t4);
   }
}

static opus_val16 median_of_3(const opus_val16 *x)
{
   opus_val16 t0, t1, t2;
   if (x[0] > x[1])
   {
      t0 = x[1];
      t1 = x[0];
   } else {
      t0 = x[0];
      t1 = x[1];
   }
   t2 = x[2];
   if (t1 < t2)
      return t1;
   else if (t0 < t2)
      return t2;
   else
      return t0;
}

static opus_val16 dynalloc_analysis(const opus_val16 *bandLogE, const opus_val16 *bandLogE2,
      int nbEBands, int start, int end, int C, int *offsets, int lsb_depth, const opus_int16 *logN,
      int isTransient, int vbr, int constrained_vbr, const opus_int16 *eBands, int LM,
      int effectiveBytes, opus_int32 *tot_boost_, int lfe, opus_val16 *surround_dynalloc,
      AnalysisInfo *analysis, int *importance, int *spread_weight)
{
   int i, c;
   opus_int32 tot_boost=0;
   opus_val16 maxDepth;
   VARDECL(opus_val16, follower);
   VARDECL(opus_val16, noise_floor);
   SAVE_STACK;
   ALLOC(follower, C*nbEBands, opus_val16);
   ALLOC(noise_floor, C*nbEBands, opus_val16);
   OPUS_CLEAR(offsets, nbEBands);
   /* Dynamic allocation code */
   maxDepth=-QCONST16(31.9f, DB_SHIFT);
   for (i=0;i<end;i++)
   {
      /* Noise floor must take into account eMeans, the depth, the width of the bands
         and the preemphasis filter (approx. square of bark band ID) */
      noise_floor[i] = MULT16_16(QCONST16(0.0625f, DB_SHIFT),logN[i])
            +QCONST16(.5f,DB_SHIFT)+SHL16(9-lsb_depth,DB_SHIFT)-SHL16(eMeans[i],6)
            +MULT16_16(QCONST16(.0062f,DB_SHIFT),(i+5)*(i+5));
   }
   c=0;do
   {
      for (i=0;i<end;i++)
         maxDepth = MAX16(maxDepth, bandLogE[c*nbEBands+i]-noise_floor[i]);
   } while (++c<C);
   {
      /* Compute a really simple masking model to avoid taking into account completely masked
         bands when computing the spreading decision. */
      VARDECL(opus_val16, mask);
      VARDECL(opus_val16, sig);
      ALLOC(mask, nbEBands, opus_val16);
      ALLOC(sig, nbEBands, opus_val16);
      for (i=0;i<end;i++)
         mask[i] = bandLogE[i]-noise_floor[i];
      if (C==2)
      {
         for (i=0;i<end;i++)
            mask[i] = MAX16(mask[i], bandLogE[nbEBands+i]-noise_floor[i]);
      }
      OPUS_COPY(sig, mask, end);
      for (i=1;i<end;i++)
         mask[i] = MAX16(mask[i], mask[i-1] - QCONST16(2.f, DB_SHIFT));
      for (i=end-2;i>=0;i--)
         mask[i] = MAX16(mask[i], mask[i+1] - QCONST16(3.f, DB_SHIFT));
      for (i=0;i<end;i++)
      {
         /* Compute SMR: Mask is never more than 72 dB below the peak and never below the noise floor.*/
         opus_val16 smr = sig[i]-MAX16(MAX16(0, maxDepth-QCONST16(12.f, DB_SHIFT)), mask[i]);
         /* Clamp SMR to make sure we're not shifting by something negative or too large. */
         smr = MAX16(-QCONST16(5.f, DB_SHIFT), MIN16(0, smr));
#ifdef FIXED_POINT
         /* FIXME: Use PSHR16() instead */
         spread_weight[i] = IMAX(1, 32 >> -PSHR32(smr, DB_SHIFT));
#else
         spread_weight[i] = IMAX(1, 32 >> -(int)floorf(.5f + smr));
#endif
      }
	  FREE1(sig, nbEBands, opus_val16);
	  FREE1(mask, nbEBands, opus_val16);
      /*for (i=0;i<end;i++)
         printf("%d ", spread_weight[i]);
      printf("\n");*/
   }
   /* Make sure that dynamic allocation can't make us bust the budget */
   if (effectiveBytes > 50 && LM>=1 && !lfe)
   {
      int last=0;
      c=0;do
      {
         opus_val16 offset;
         opus_val16 tmp;
         opus_val16 *f;
         f = &follower[c*nbEBands];
         f[0] = bandLogE2[c*nbEBands];
         for (i=1;i<end;i++)
         {
            /* The last band to be at least 3 dB higher than the previous one
               is the last we'll consider. Otherwise, we run into problems on
               bandlimited signals. */
            if (bandLogE2[c*nbEBands+i] > bandLogE2[c*nbEBands+i-1]+QCONST16(.5f,DB_SHIFT))
               last=i;
            f[i] = MIN16(f[i-1]+QCONST16(1.5f,DB_SHIFT), bandLogE2[c*nbEBands+i]);
         }
         for (i=last-1;i>=0;i--)
            f[i] = MIN16(f[i], MIN16(f[i+1]+QCONST16(2.f,DB_SHIFT), bandLogE2[c*nbEBands+i]));

         /* Combine with a median filter to avoid dynalloc triggering unnecessarily.
            The "offset" value controls how conservative we are -- a higher offset
            reduces the impact of the median filter and makes dynalloc use more bits. */
         offset = QCONST16(1.f, DB_SHIFT);
         for (i=2;i<end-2;i++)
            f[i] = MAX16(f[i], median_of_5(&bandLogE2[c*nbEBands+i-2])-offset);
         tmp = median_of_3(&bandLogE2[c*nbEBands])-offset;
         f[0] = MAX16(f[0], tmp);
         f[1] = MAX16(f[1], tmp);
         tmp = median_of_3(&bandLogE2[c*nbEBands+end-3])-offset;
         f[end-2] = MAX16(f[end-2], tmp);
         f[end-1] = MAX16(f[end-1], tmp);

         for (i=0;i<end;i++)
            f[i] = MAX16(f[i], noise_floor[i]);
      } while (++c<C);
      if (C==2)
      {
         for (i=start;i<end;i++)
         {
            /* Consider 24 dB "cross-talk" */
            follower[nbEBands+i] = MAX16(follower[nbEBands+i], follower[         i]-QCONST16(4.f,DB_SHIFT));
            follower[         i] = MAX16(follower[         i], follower[nbEBands+i]-QCONST16(4.f,DB_SHIFT));
            follower[i] = HALF16(MAX16(0, bandLogE[i]-follower[i]) + MAX16(0, bandLogE[nbEBands+i]-follower[nbEBands+i]));
         }
      } else {
         for (i=start;i<end;i++)
         {
            follower[i] = MAX16(0, bandLogE[i]-follower[i]);
         }
      }
      for (i=start;i<end;i++)
         follower[i] = MAX16(follower[i], surround_dynalloc[i]);
      for (i=start;i<end;i++)
      {
#ifdef FIXED_POINT
         importance[i] = PSHR32(13*celt_exp2(MIN16(follower[i], QCONST16(4.f, DB_SHIFT))), 16);
#else
         importance[i] = (int)floorf(.5f+13*celt_exp2(MIN16(follower[i], QCONST16(4.f, DB_SHIFT))));
#endif
      }
      /* For non-transient CBR/CVBR frames, halve the dynalloc contribution */
      if ((!vbr || constrained_vbr)&&!isTransient)
      {
         for (i=start;i<end;i++)
            follower[i] = HALF16(follower[i]);
      }
      for (i=start;i<end;i++)
      {
         if (i<8)
            follower[i] *= 2;
         if (i>=12)
            follower[i] = HALF16(follower[i]);
      }
#ifdef DISABLE_FLOAT_API
      (void)analysis;
#else
      if (analysis->valid)
      {
         for (i=start;i<IMIN(LEAK_BANDS, end);i++)
            follower[i] = follower[i] +  QCONST16(1.f/64.f, DB_SHIFT)*analysis->leak_boost[i];
      }
#endif
      for (i=start;i<end;i++)
      {
         int width;
         int boost;
         int boost_bits;

         follower[i] = MIN16(follower[i], QCONST16(4, DB_SHIFT));

         width = C*(eBands[i+1]-eBands[i])<<LM;
         if (width<6)
         {
            boost = (int)SHR32(EXTEND32(follower[i]),DB_SHIFT);
            boost_bits = boost*width<<BITRES;
         } else if (width > 48) {
            boost = (int)SHR32(EXTEND32(follower[i])*8,DB_SHIFT);
            boost_bits = (boost*width<<BITRES)/8;
         } else {
            boost = (int)SHR32(EXTEND32(follower[i])*width/6,DB_SHIFT);
            boost_bits = boost*6<<BITRES;
         }
         /* For CBR and non-transient CVBR frames, limit dynalloc to 2/3 of the bits */
         if ((!vbr || (constrained_vbr&&!isTransient))
               && (tot_boost+boost_bits)>>BITRES>>3 > 2*effectiveBytes/3)
         {
            opus_int32 cap = ((2*effectiveBytes/3)<<BITRES<<3);
            offsets[i] = cap-tot_boost;
            tot_boost = cap;
            break;
         } else {
            offsets[i] = boost;
            tot_boost += boost_bits;
         }
      }
   } else {
      for (i=start;i<end;i++)
         importance[i] = 13;
   }
   *tot_boost_ = tot_boost;
   FREE1(follower, C*nbEBands, opus_val16);
   FREE1(noise_floor, C*nbEBands, opus_val16);
   RESTORE_STACK;
   return maxDepth;
}


static int run_prefilter(CELTEncoder *st, celt_sig *in, celt_sig *prefilter_mem, int CC, int N,
      int prefilter_tapset, int *pitch, opus_val16 *gain, int *qgain, int enabled, int nbAvailableBytes, AnalysisInfo *analysis)
{
   int c;
   VARDECL(celt_sig, _pre);
   celt_sig *pre[2];
   const CELTMode *mode;
   int pitch_index;
   opus_val16 gain1;
   opus_val16 pf_threshold;
   int pf_on;
   int qg;
   int overlap;
   SAVE_STACK;

   mode = st->mode;
   overlap = mode->overlap;
   ALLOC(_pre, CC*(N+COMBFILTER_MAXPERIOD), celt_sig);

   pre[0] = _pre;
   pre[1] = _pre + (N+COMBFILTER_MAXPERIOD);


   c=0; do {
      OPUS_COPY(pre[c], prefilter_mem+c*COMBFILTER_MAXPERIOD, COMBFILTER_MAXPERIOD);
      OPUS_COPY(pre[c]+COMBFILTER_MAXPERIOD, in+c*(N+overlap)+overlap, N);
   } while (++c<CC);

   if (enabled)
   {
#if (QL_OPUS_ENC_COMPLEXITY >= 5)
      VARDECL(opus_val16, pitch_buf);
      ALLOC(pitch_buf, (COMBFILTER_MAXPERIOD+N)>>1, opus_val16);
      pitch_downsample(pre, pitch_buf, COMBFILTER_MAXPERIOD+N, CC, st->arch);
      /* Don't search for the fir last 1.5 octave of the range because
         there's too many false-positives due to short-term correlation */
      pitch_search(pitch_buf+(COMBFILTER_MAXPERIOD>>1), pitch_buf, N,
            COMBFILTER_MAXPERIOD-3*COMBFILTER_MINPERIOD, &pitch_index,
            st->arch);
      pitch_index = COMBFILTER_MAXPERIOD-pitch_index;

      gain1 = remove_doubling(pitch_buf, COMBFILTER_MAXPERIOD, COMBFILTER_MINPERIOD,
            N, &pitch_index, st->prefilter_period, st->prefilter_gain, st->arch);
      if (pitch_index > COMBFILTER_MAXPERIOD-2)
         pitch_index = COMBFILTER_MAXPERIOD-2;
      gain1 = MULT16_16_Q15(QCONST16(.7f,15),gain1);
      /*printf("%d %d %f %f\n", pitch_change, pitch_index, gain1, st->analysis.tonality);*/
      if (st->loss_rate>2)
         gain1 = HALF32(gain1);
      if (st->loss_rate>4)
         gain1 = HALF32(gain1);
      if (st->loss_rate>8)
         gain1 = 0;

	  FREE1(pitch_buf, (COMBFILTER_MAXPERIOD + N) >> 1, opus_val16);
#endif
   } else {
      gain1 = 0;
      pitch_index = COMBFILTER_MINPERIOD;
   }
#ifndef DISABLE_FLOAT_API
   if (analysis->valid)
      gain1 *= analysis->max_pitch_ratio;
#endif
   /* Gain threshold for enabling the prefilter/postfilter */
   pf_threshold = QCONST16(.2f,15);

   /* Adjusting the threshold based on rate and continuity */
   if (abs(pitch_index-st->prefilter_period)*10>pitch_index)
      pf_threshold += QCONST16(.2f,15);
   if (nbAvailableBytes<25)
      pf_threshold += QCONST16(.1f,15);
   if (nbAvailableBytes<35)
      pf_threshold += QCONST16(.1f,15);
   if (st->prefilter_gain > QCONST16(.4f,15))
      pf_threshold -= QCONST16(.1f,15);
   if (st->prefilter_gain > QCONST16(.55f,15))
      pf_threshold -= QCONST16(.1f,15);

   /* Hard threshold at 0.2 */
   pf_threshold = MAX16(pf_threshold, QCONST16(.2f,15));
   if (gain1<pf_threshold)
   {
      gain1 = 0;
      pf_on = 0;
      qg = 0;
   } else {
      /*This block is not gated by a total bits check only because
        of the nbAvailableBytes check above.*/
      if (ABS16(gain1-st->prefilter_gain)<QCONST16(.1f,15))
         gain1=st->prefilter_gain;

#ifdef FIXED_POINT
      qg = ((gain1+1536)>>10)/3-1;
#else
      qg = (int)floorf(.5f+gain1*32/3)-1;
#endif
      qg = IMAX(0, IMIN(7, qg));
      gain1 = QCONST16(0.09375f,15)*(qg+1);
      pf_on = 1;
   }
   /*printf("%d %f\n", pitch_index, gain1);*/

   c=0; do {
      int offset = mode->shortMdctSize-overlap;
      st->prefilter_period=IMAX(st->prefilter_period, COMBFILTER_MINPERIOD);
      OPUS_COPY(in+c*(N+overlap), st->in_mem+c*(overlap), overlap);
      if (offset)
         comb_filter(in+c*(N+overlap)+overlap, pre[c]+COMBFILTER_MAXPERIOD,
               st->prefilter_period, st->prefilter_period, offset, -st->prefilter_gain, -st->prefilter_gain,
               st->prefilter_tapset, st->prefilter_tapset, NULL, 0, st->arch);

      comb_filter(in+c*(N+overlap)+overlap+offset, pre[c]+COMBFILTER_MAXPERIOD+offset,
            st->prefilter_period, pitch_index, N-offset, -st->prefilter_gain, -gain1,
            st->prefilter_tapset, prefilter_tapset, mode->window, overlap, st->arch);
      OPUS_COPY(st->in_mem+c*(overlap), in+c*(N+overlap)+N, overlap);

      if (N>COMBFILTER_MAXPERIOD)
      {
         OPUS_COPY(prefilter_mem+c*COMBFILTER_MAXPERIOD, pre[c]+N, COMBFILTER_MAXPERIOD);
      } else {
         OPUS_MOVE(prefilter_mem+c*COMBFILTER_MAXPERIOD, prefilter_mem+c*COMBFILTER_MAXPERIOD+N, COMBFILTER_MAXPERIOD-N);
         OPUS_COPY(prefilter_mem+c*COMBFILTER_MAXPERIOD+COMBFILTER_MAXPERIOD-N, pre[c]+COMBFILTER_MAXPERIOD, N);
      }
   } while (++c<CC);
   FREE1(_pre, CC*(N + COMBFILTER_MAXPERIOD), celt_sig);
   RESTORE_STACK;
   *gain = gain1;
   *pitch = pitch_index;
   *qgain = qg;
   return pf_on;
}

int calc_band_nrg(CELTEncoder * OPUS_RESTRICT st, opus_val16* bandLogE2, celt_sig* in, celt_ener *bandE, opus_val16 *bandLogE, celt_sig *freq,  int window_len, int LM, int shortBlocks, int effEnd)
{

	int CC = st->channels;
	int C = st->stream_channels;
	const OpusCustomMode *mode = st->mode;

	int end = st->end;
	int secondMdct = 0;




	secondMdct = mode->shortMdctSize && st->complexity >= 8;
	if (secondMdct)
	{
#if (QL_OPUS_ENC_COMPLEXITY >= 8)
		compute_mdcts(mode, 0, in, freq, C, CC, LM, st->upsample, st->arch);
		compute_band_energies(mode, freq, bandE, effEnd, C, LM, st->arch);
		amp2Log2(mode, effEnd, end, bandE, bandLogE2, C);
		for (int i = 0; i<C*mode->nbEBands; i++)
			bandLogE2[i] += HALF16(SHL16(LM, DB_SHIFT));
#endif
	}

	compute_mdcts(mode, shortBlocks, in, freq, C, CC, LM, st->upsample, st->arch);

	compute_band_energies(mode, freq, bandE, effEnd, C, LM, st->arch);

	if (st->lfe)
	{
		for (int i = 2; i<end; i++)
		{
			bandE[i] = IMIN(bandE[i], MULT16_32_Q15(QCONST16(1e-4f, 15), bandE[0]));
			bandE[i] = MAX32(bandE[i], EPSILON);
		}
	}
	amp2Log2(mode, effEnd, end, bandE, bandLogE, C);

	return secondMdct;
}
int calc_detect_pitch_gain(CELTEncoder * OPUS_RESTRICT st, celt_sig* in, int nbAvailableBytes, int *pitch_index, opus_val16 *gain1, int *pf_on, int total_bits, int tell, ec_enc *enc, int N, int silence)
{
	int enabled;
	int qg;
	int pitch_change = 0;

	const OpusCustomMode *mode = st->mode;
	celt_sig *prefilter_mem = st->in_mem + st->channels * (mode->overlap);
	enabled = ((st->lfe&&nbAvailableBytes>3) || nbAvailableBytes>12 )  && !silence && !st->disable_pf
		&& st->complexity >= 5;

	int prefilter_tapset = st->tapset_decision;
	*pf_on = run_prefilter(st, in, prefilter_mem, st->channels, N, prefilter_tapset, pitch_index, gain1, &qg, enabled, nbAvailableBytes, &st->analysis);
	if ((*gain1 > QCONST16(.4f, 15) || st->prefilter_gain > QCONST16(.4f, 15)) && (!st->analysis.valid || st->analysis.tonality > .3)
		&& (*pitch_index > 1.26*st->prefilter_period || *pitch_index < .79*st->prefilter_period))
		pitch_change = 1;
	if (*pf_on == 0)
	{
		if (tell + 16 <= total_bits)
			ec_enc_bit_logp(enc, 0, 1);
	}
	else {
		/*This block is not gated by a total bits check only because
		of the nbAvailableBytes check above.*/
		int octave;
		ec_enc_bit_logp(enc, 1, 1);
		*pitch_index += 1;
		octave = EC_ILOG(*pitch_index) - 5;
		ec_enc_uint(enc, octave, 6);
		ec_enc_bits(enc, *pitch_index - (16 << octave), 4 + octave);
		*pitch_index -= 1;
		ec_enc_bits(enc, qg, 3);
		ec_enc_icdf(enc, prefilter_tapset, tapset_icdf, 2);
	}
	return pitch_change;
}
int calc_mask(CELTEncoder * OPUS_RESTRICT st, opus_val16 *surround_dynalloc, opus_val16 *surround_trim, opus_val16 *surround_masking)
{
	const OpusCustomMode *mode = st->mode;
	if (st->energy_mask && !st->lfe)
	{
		int mask_end;
		int midband;
		int count_dynalloc;
		opus_val32 mask_avg = 0;
		opus_val32 diff = 0;
		int count = 0;
		mask_end = IMAX(2, st->lastCodedBands);
		const opus_int16 *eBands = mode->eBands;

		for (int i = 0; i < mask_end; i++)
		{
			opus_val16 mask;
			mask = MAX16(MIN16(st->energy_mask[mode->nbEBands + i],
				QCONST16(.25f, DB_SHIFT)), -QCONST16(2.0f, DB_SHIFT));
			if (mask > 0)
				mask = HALF16(mask);
			mask_avg += MULT16_16(mask, eBands[i + 1] - eBands[i]);
			count += eBands[i + 1] - eBands[i];
			diff += MULT16_16(mask, 1 + 2 * i - mask_end);
		}

		celt_assert(count > 0);
		mask_avg = DIV32_16(mask_avg, count);
		mask_avg += QCONST16(.2f, DB_SHIFT);
		diff = diff * 6 / ((mask_end - 1)*(mask_end + 1)*mask_end);
		/* Again, being conservative */
		diff = HALF32(diff);
		diff = MAX32(MIN32(diff, QCONST32(.031f, DB_SHIFT)), -QCONST32(.031f, DB_SHIFT));
		/* Find the band that's in the middle of the coded spectrum */
		for (midband = 0; eBands[midband + 1] < eBands[mask_end] / 2; midband++);
		count_dynalloc = 0;
		for (int i = 0; i < mask_end; i++)
		{
			opus_val32 lin;
			opus_val16 unmask;
			lin = mask_avg + diff * (i - midband);

			unmask = st->energy_mask[i];

			unmask = MIN16(unmask, QCONST16(.0f, DB_SHIFT));
			unmask -= lin;
			if (unmask > QCONST16(.25f, DB_SHIFT))
			{
				surround_dynalloc[i] = unmask - QCONST16(.25f, DB_SHIFT);
				count_dynalloc++;
			}
		}
		if (count_dynalloc >= 3)
		{
			/* If we need dynalloc in many bands, it's probably because our
			initial masking rate was too low. */
			mask_avg += QCONST16(.25f, DB_SHIFT);
			if (mask_avg > 0)
			{
				/* Something went really wrong in the original calculations,
				disabling masking. */
				mask_avg = 0;
				diff = 0;
				OPUS_CLEAR(surround_dynalloc, mask_end);
			}
			else {
				for (int i = 0; i < mask_end; i++)
					surround_dynalloc[i] = MAX16(0, surround_dynalloc[i] - QCONST16(.25f, DB_SHIFT));
			}
		}
		mask_avg += QCONST16(.2f, DB_SHIFT);
		/* Convert to 1/64th units used for the trim */
		*surround_trim = 64 * diff;
		/*printf("%d %d ", mask_avg, surround_trim);*/
		*surround_masking = mask_avg;
	}
	return 0;
}
int time_freq_trasnform(CELTEncoder * OPUS_RESTRICT st,  int effEnd, int isTransient,
	int *tf_res,  celt_norm *X,  int LM, opus_val16 tf_estimate, int tf_chan, int *importance, int enable_tf_analysis, int effectiveBytes, int weak_transient, int N)
{
	const CELTMode *mode = st->mode;
	int tf_select = 0;
	int end = st->end;

	/* Disable variable tf resolution for hybrid and at very low bitrate */
	if (enable_tf_analysis)
	{
		int lambda;
		lambda = IMAX(80, 20480 / effectiveBytes + 2);
		tf_select = tf_analysis(mode, effEnd, isTransient, tf_res, lambda, X, N, LM, tf_estimate, tf_chan, importance);
		for (int i = effEnd; i<end; i++)
			tf_res[i] = tf_res[effEnd - 1];
	}
	else if (weak_transient)
	{
		/* For weak transients, we rely on the fact that improving time resolution using
		TF on a long window is imperfect and will not result in an energy collapse at
		low bitrate. */
		for (int i = 0; i<end; i++)
			tf_res[i] = 1;
		tf_select = 0;
	}
	else {
		for (int i = 0; i<end; i++)
			tf_res[i] = isTransient;
		tf_select = 0;
	}
	return tf_select;
}

int prep_quantize(CELTEncoder * OPUS_RESTRICT st, ec_enc *enc, int *cap, int *offsets, int total_bits, int LM, int *alloc_trim, celt_norm *X,
	opus_val16 *bandLogE, opus_val16 tf_estimate, opus_val16 surround_trim, int equiv_rate, int N)
{
	int dynalloc_logp = 6;
	total_bits <<= BITRES;
	int total_boost = 0;
	int tell = ec_tell_frac(enc);
	const CELTMode *mode = st->mode;
	for (int i = st->start; i<st->end; i++)
	{
		int width, quanta;
		int dynalloc_loop_logp;
		int boost;
		int j;
		width = (mode->eBands[i + 1] - mode->eBands[i]) << LM;
		/* quanta is 6 bits, but no more than 1 bit/sample
		and no less than 1/8 bit/sample */
		quanta = IMIN(width << BITRES, IMAX(6 << BITRES, width));
		dynalloc_loop_logp = dynalloc_logp;
		boost = 0;
		for (j = 0; tell + (dynalloc_loop_logp << BITRES) < total_bits - total_boost
			&& boost < cap[i]; j++)
		{
			int flag;
			flag = j<offsets[i];
			ec_enc_bit_logp(enc, flag, dynalloc_loop_logp);
			tell = ec_tell_frac(enc);
			if (!flag)
				break;
			boost += quanta;
			total_boost += quanta;
			dynalloc_loop_logp = 1;
		}
                
		/* Making dynalloc more likely */
#if 0           //This path has compiler code generation issue for optimization: "common subexpression elimination"
                //This can be solved be using "#pragma optimize=no_cse"  for this function alone or re-write code with as it is done in #else portion.
		if (j)
			dynalloc_logp = IMAX(2, dynalloc_logp - 1);
#else
		if (j)
                {
                        dynalloc_logp--;
                        if (dynalloc_logp <= 2)
                        {
                            dynalloc_logp = 2;
                        }
                }
#endif                
                
                
		offsets[i] = boost;
	}


	*alloc_trim = 5;
	if (tell + (6 << BITRES) <= total_bits - total_boost)
	{
		if (st->start > 0 || st->lfe)
		{
			st->stereo_saving = 0;
			*alloc_trim = 5;
		}
		else {
			*alloc_trim = alloc_trim_analysis(mode, X, bandLogE,
				st->end, LM, st->stream_channels, N, &st->analysis, &st->stereo_saving, tf_estimate,
				st->intensity, surround_trim, equiv_rate, st->arch);
		}
		ec_enc_icdf(enc, *alloc_trim, trim_icdf, 7);
		tell = ec_tell_frac(enc);
	}

	return total_bits;
}
int celt_encode_with_ec(CELTEncoder * OPUS_RESTRICT st, const opus_val16 * pcm, int frame_size, unsigned char *compressed, int nbCompressedBytes, ec_enc *enc)
{
	int i, c, N;
	opus_int32 bits;
	ec_enc _enc;

	opus_val16 *oldBandE, *oldLogE, *oldLogE2, *energyError;
	int shortBlocks = 0;
	int isTransient = 0;
	const int CC = st->channels;
	const int C = st->stream_channels;
	int LM, M;
	int tf_select;
	int nbFilledBytes, nbAvailableBytes;
	int start;
	int end;
	int effEnd;
	int codedBands;
	int alloc_trim = 0;
	int pitch_index = COMBFILTER_MINPERIOD;
	opus_val16 gain1 = 0;
	int dual_stereo = 0;
	int effectiveBytes;
	// int dynalloc_logp;
	opus_int32 total_bits;
	// opus_int32 total_boost;
	opus_int32 balance;
	opus_int32 tell;
	int prefilter_tapset = 0;
	int pf_on;
	int anti_collapse_rsv;
	int anti_collapse_on = 0;
	int silence = 0;
	int tf_chan = 0;
	opus_val16 tf_estimate;
	opus_int32 tot_boost;
	opus_val32 sample_max;
	const OpusCustomMode *mode;
	int nbEBands;
	int overlap;
	const opus_int16 *eBands;
	int secondMdct;
	int signalBandwidth;
	int transient_got_disabled = 0;
	opus_val16 surround_masking = 0;
	opus_val16 temporal_vbr = 0;
	opus_val16 surround_trim = 0;
	opus_int32 equiv_rate;
	int hybrid;
	int weak_transient = 0;
	int enable_tf_analysis;

	ALLOC_STACK;

	mode = st->mode;
	nbEBands = mode->nbEBands;
	overlap = mode->overlap;
	eBands = mode->eBands;
	start = st->start;
	end = st->end;
	hybrid = start != 0;
	tf_estimate = 0;
	if (nbCompressedBytes < 2 || pcm == NULL)
	{
		RESTORE_STACK;
		return OPUS_BAD_ARG;
	}

	frame_size *= st->upsample;
	for (LM = 0; LM <= mode->maxLM; LM++)
		if (mode->shortMdctSize << LM == frame_size)
			break;
	if (LM > mode->maxLM)
	{
		RESTORE_STACK;
		return OPUS_BAD_ARG;
	}
	M = 1 << LM;
	N = M * mode->shortMdctSize;

	oldBandE = (opus_val16*)(st->in_mem + CC * (overlap + COMBFILTER_MAXPERIOD));
	oldLogE = oldBandE + CC * nbEBands;
	oldLogE2 = oldLogE + CC * nbEBands;
	energyError = oldLogE2 + CC * nbEBands;

	if (enc == NULL)
	{
		tell = 1;
		nbFilledBytes = 0;
	}
	else {
		tell = ec_tell(enc);
		nbFilledBytes = (tell + 4) >> 3;
	}

	celt_assert(st->signalling == 0);

	/* Can't produce more than 1275 output bytes */
	nbCompressedBytes = IMIN(nbCompressedBytes, 1275);
	nbAvailableBytes = nbCompressedBytes - nbFilledBytes;


	opus_int32 tmp;
	tmp = st->bitrate*frame_size;
	if (tell > 1)
		tmp += tell;
	if (st->bitrate != OPUS_BITRATE_MAX)
		nbCompressedBytes = IMAX(2, IMIN(nbCompressedBytes,
		(tmp + 4 * mode->Fs) / (8 * mode->Fs) - !!st->signalling));
	effectiveBytes = nbCompressedBytes - nbFilledBytes;

	equiv_rate = ((opus_int32)nbCompressedBytes * 8 * 50 >> (3 - LM)) - (40 * C + 20)*((400 >> LM) - 50);
	if (st->bitrate != OPUS_BITRATE_MAX)
		equiv_rate = IMIN(equiv_rate, st->bitrate - (40 * C + 20)*((400 >> LM) - 50));

	if (enc == NULL)
	{
		ec_enc_init(&_enc, compressed, nbCompressedBytes);
		enc = &_enc;
	}


	total_bits = nbCompressedBytes * 8;
	{
		VARDECL(opus_val16, error);

		ALLOC(error, C*nbEBands, opus_val16);

		effEnd = end;
		if (effEnd > mode->effEBands)
			effEnd = mode->effEBands;

		{
			VARDECL(celt_norm, X);
			VARDECL(int, cap);
			VARDECL(int, offsets);
			VARDECL(celt_ener, bandE);
			VARDECL(opus_val16, bandLogE);
			VARDECL(int, fine_quant);
			VARDECL(int, fine_priority);
			VARDECL(int, tf_res);

			ALLOC(X, C*N, celt_norm);         /**< Interleaved normalised MDCTs */
			ALLOC(cap, nbEBands, int);
			ALLOC(offsets, nbEBands, int);
			ALLOC(bandE, nbEBands*CC, celt_ener);
			ALLOC(bandLogE, nbEBands*CC, opus_val16);
			ALLOC(fine_quant, nbEBands, int);
			ALLOC(fine_priority, nbEBands, int);
			ALLOC(tf_res, nbEBands, int);

			{
				VARDECL(opus_val16, surround_dynalloc);
				VARDECL(int, spread_weight);
				VARDECL(opus_val16, bandLogE2);
				VARDECL(int, importance);
				VARDECL(celt_sig, freq);

				ALLOC(surround_dynalloc, C*nbEBands, opus_val16);
				ALLOC(spread_weight, nbEBands, int);
				ALLOC(bandLogE2, C*nbEBands, opus_val16);
				ALLOC(importance, nbEBands, int); /// check 
				ALLOC(freq, CC*N, celt_sig); /**< Interleaved signal MDCTs */

				OPUS_CLEAR(surround_dynalloc, end);

				{
					VARDECL(celt_sig, in);
					ALLOC(in, CC*(N + overlap), celt_sig);
					OPUS_CLEAR(in, CC*(N + overlap));

					sample_max = MAX32(st->overlap_max, celt_maxabs16(pcm, C*(N - overlap) / st->upsample));
					st->overlap_max = celt_maxabs16(pcm + C * (N - overlap) / st->upsample, C*overlap / st->upsample);
					sample_max = MAX32(sample_max, st->overlap_max);

					silence = (sample_max <= (opus_val16)1 / (1 << st->lsb_depth));

					if (tell == 1)
						ec_enc_bit_logp(enc, silence, 15);
					else
						silence = 0;
					if (silence)
					{
						/* Pretend we've filled all the remaining bits with zeros
						(that's what the initialiser did anyway) */
						tell = nbCompressedBytes * 8;
						enc->nbits_total += tell - ec_tell(enc);
					}
					else { /* may lead to some ave mips saving - chalil */
						int need_clip = st->clip && sample_max > 65536.f, c = 0;
						celt_preemphasis(pcm + c, in + c * (N + overlap) + overlap, N, CC, st->upsample,
							mode->preemph, st->preemph_memE + c, need_clip);
					}
					/* Find pitch period and gain */
					calc_detect_pitch_gain(st, in, nbAvailableBytes, &pitch_index, &gain1, &pf_on, total_bits, tell, enc, N, silence);

					isTransient = 0;
					shortBlocks = 0;

#if (QL_OPUS_ENC_COMPLEXITY >= 1)
					if (st->complexity >= 1 && !st->lfe)
					{
						/* Reduces the likelihood of energy instability on fricatives at low bitrate
						in hybrid mode. It seems like we still want to have real transients on vowels
						though (small SILK quantization offset value). */
						int allow_weak_transients = hybrid && effectiveBytes < 15;//  && st->silk_info.signalType != 2;
						isTransient = transient_analysis(in, N + overlap, CC,
							&tf_estimate, &tf_chan, allow_weak_transients, &weak_transient);
					}
#endif
					if (LM > 0 && ec_tell(enc) + 3 <= total_bits)
					{
						if (isTransient)
							shortBlocks = M;
					}
					else {
						isTransient = 0;
						transient_got_disabled = 1;
					}

					secondMdct = calc_band_nrg(st, bandLogE2, in, bandE, bandLogE, freq, N, LM, shortBlocks,  effEnd);


					/* This computes how much masking takes place between surround channels */
					calc_mask(st, surround_dynalloc, &surround_trim, &surround_masking);

					/* Temporal VBR (but not for LFE) */
					if (!st->lfe)
					{
						opus_val16 follow = -QCONST16(10.0f, DB_SHIFT);
						opus_val32 frame_avg = 0;
						opus_val16 offset = shortBlocks ? HALF16(SHL16(LM, DB_SHIFT)) : 0;
						for (i = start; i < end; i++)
						{
							follow = MAX16(follow - QCONST16(1.f, DB_SHIFT), bandLogE[i] - offset);
							if (C == 2)
								follow = MAX16(follow, bandLogE[i + nbEBands] - offset);
							frame_avg += follow;
						}
						frame_avg /= (end - start);
						temporal_vbr = SUB16(frame_avg, st->spec_avg);
						temporal_vbr = MIN16(QCONST16(3.f, DB_SHIFT), MAX16(-QCONST16(1.5f, DB_SHIFT), temporal_vbr));
						st->spec_avg += MULT16_16_Q15(QCONST16(.02f, 15), temporal_vbr);
					}
					/*for (i=0;i<21;i++)
					printf("%f ", bandLogE[i]);
					printf("\n");*/

					if (!secondMdct)
					{
						OPUS_COPY(bandLogE2, bandLogE, C*nbEBands);
					}
#if (QL_OPUS_ENC_COMPLEXITY >= 5)
					/* Last chance to catch any transient we might have missed in the
					time-domain analysis */
					if (LM > 0 && ec_tell(enc) + 3 <= total_bits && !isTransient && st->complexity >= 5 && !st->lfe && !hybrid)
					{
						if (patch_transient_decision(bandLogE, oldBandE, nbEBands, start, end, C))
						{
							isTransient = 1;
							shortBlocks = M;
							compute_mdcts(mode, shortBlocks, in, freq, C, CC, LM, st->upsample, st->arch);
							compute_band_energies(mode, freq, bandE, effEnd, C, LM, st->arch);
							amp2Log2(mode, effEnd, end, bandE, bandLogE, C);
							/* Compensate for the scaling of short vs long mdcts */
							for (i = 0; i < C*nbEBands; i++)
								bandLogE2[i] += HALF16(SHL16(LM, DB_SHIFT));
							tf_estimate = QCONST16(.2f, 14);
						}
					}
#endif
					FREE1(in, CC*(N + overlap), celt_sig);
				}
				if (LM > 0 && ec_tell(enc) + 3 <= total_bits)
					ec_enc_bit_logp(enc, isTransient, 3);


				/* Band normalisation */
				normalise_bands(mode, freq, X, bandE, effEnd, C, M);

				enable_tf_analysis = effectiveBytes >= 15 * C && !hybrid && st->complexity >= 2 && !st->lfe;


				dynalloc_analysis(bandLogE, bandLogE2, nbEBands, start, end, C, offsets,
					st->lsb_depth, mode->logN, isTransient, st->vbr, st->constrained_vbr,
					eBands, LM, effectiveBytes, &tot_boost, st->lfe, surround_dynalloc, &st->analysis, importance, spread_weight);


				tf_select = time_freq_trasnform(st, effEnd, isTransient, tf_res, X, LM,
					tf_estimate, tf_chan, importance, enable_tf_analysis, effectiveBytes, weak_transient, N);

				c = 0;
				do {
					for (i = start; i < end; i++)
					{
						/* When the energy is stable, slightly bias energy quantization towards
						the previous error to make the gain more stable (a constant offset is
						better than fluctuations). */
						if (ABS32(SUB32(bandLogE[i + c * nbEBands], oldBandE[i + c * nbEBands])) < QCONST16(2.f, DB_SHIFT))
						{
							bandLogE[i + c * nbEBands] -= MULT16_16_Q15(energyError[i + c * nbEBands], QCONST16(0.25f, 15));
						}
					}
				} while (++c < C);
				quant_coarse_energy(mode, start, end, effEnd, bandLogE,
					oldBandE, total_bits, error, enc,
					C, LM, nbAvailableBytes, st->force_intra,
					&st->delayedIntra, st->complexity >= 4, st->loss_rate, st->lfe);

				tf_encode(start, end, isTransient, tf_res, LM, tf_select, enc);

				if (ec_tell(enc) + 4 <= total_bits)
				{
					if (shortBlocks || st->complexity < 3 || nbAvailableBytes < 10 * C)
					{
						if (st->complexity == 0)
							st->spread_decision = SPREAD_NONE;
						else
							st->spread_decision = SPREAD_NORMAL;
					}
					else {
						{
							st->spread_decision = spreading_decision(mode, X,
								&st->tonal_average, st->spread_decision, &st->hf_average,
								&st->tapset_decision, pf_on && !shortBlocks, effEnd, C, M, spread_weight);
						}
						/*printf("%d %d\n", st->tapset_decision, st->spread_decision);*/
						/*printf("%f %d %f %d\n\n", st->analysis.tonality, st->spread_decision, st->analysis.tonality_slope, st->tapset_decision);*/
					}
					ec_enc_icdf(enc, st->spread_decision, spread_icdf, 5);
				}

				init_caps(mode, cap, LM, C);

				/* Quantization */
				total_bits = prep_quantize(st, enc, cap, offsets, total_bits, LM, &alloc_trim, X, bandLogE, tf_estimate, surround_trim, equiv_rate, N);
				FREE1(freq, CC*N, celt_sig); /**< Interleaved signal MDCTs */
				FREE1(importance, nbEBands, int);
				FREE1(bandLogE2, C*nbEBands, opus_val16);
				FREE1(spread_weight, nbEBands, int);
				FREE1(surround_dynalloc, C*nbEBands, opus_val16);

			}
			/* Bit allocation */
			{
				VARDECL(int, pulses);
				VARDECL(unsigned char, collapse_masks);

				ALLOC(pulses, nbEBands, int);
				ALLOC(collapse_masks, C*nbEBands, unsigned char);

				/* bits =           packet size                    - where we are - safety*/
				bits = (((opus_int32)nbCompressedBytes * 8) << BITRES) - ec_tell_frac(enc) - 1;
				anti_collapse_rsv = isTransient && LM >= 2 && bits >= ((LM + 2) << BITRES) ? (1 << BITRES) : 0;
				bits -= anti_collapse_rsv;
				signalBandwidth = end - 1;
#ifndef DISABLE_FLOAT_API
				if (st->analysis.valid)
				{
					int min_bandwidth;
					if (equiv_rate < (opus_int32)32000 * C)
						min_bandwidth = 13;
					else if (equiv_rate < (opus_int32)48000 * C)
						min_bandwidth = 16;
					else if (equiv_rate < (opus_int32)60000 * C)
						min_bandwidth = 18;
					else  if (equiv_rate < (opus_int32)80000 * C)
						min_bandwidth = 19;
					else
						min_bandwidth = 20;
					signalBandwidth = IMAX(st->analysis.bandwidth, min_bandwidth);
				}
#endif
				if (st->lfe)
					signalBandwidth = 1;
				codedBands = compute_allocation(mode, start, end, offsets, cap,
					alloc_trim, &st->intensity, &dual_stereo, bits, &balance, pulses,
					fine_quant, fine_priority, C, LM, enc, 1, st->lastCodedBands, signalBandwidth);
				if (st->lastCodedBands)
					st->lastCodedBands = IMIN(st->lastCodedBands + 1, IMAX(st->lastCodedBands - 1, codedBands));
				else
					st->lastCodedBands = codedBands;
				quant_fine_energy(mode, start, end, oldBandE, error, fine_quant, enc, C);
				/* Residual quantisation */
				quant_all_bands(1, mode, start, end, X, C == 2 ? X + N : NULL, collapse_masks,
					bandE, pulses, shortBlocks, st->spread_decision,
					dual_stereo, st->intensity, tf_res, nbCompressedBytes*(8 << BITRES) - anti_collapse_rsv,
					balance, enc, LM, codedBands, &st->rng, st->complexity, st->arch, st->disable_inv);

				FREE1(collapse_masks, C*nbEBands, unsigned char);
				FREE1(pulses, nbEBands, int);
			}


			if (anti_collapse_rsv > 0)
			{
				anti_collapse_on = st->consec_transient < 2;
#ifdef FUZZING
				anti_collapse_on = rand() & 0x1;
#endif
				ec_enc_bits(enc, anti_collapse_on, 1);
			}

			quant_energy_finalise(mode, start, end, oldBandE, error, fine_quant, fine_priority, nbCompressedBytes * 8 - ec_tell(enc), enc, C);

			FREE1(X, C*N, celt_norm);         /**< Interleaved normalised MDCTs */
			FREE1(cap, nbEBands, int);
			FREE1(offsets, nbEBands, int);
			FREE1(bandE, nbEBands*CC, celt_ener);
			FREE1(bandLogE, nbEBands*CC, opus_val16);
			FREE1(fine_quant, nbEBands, int);
			FREE1(fine_priority, nbEBands, int);
			FREE1(tf_res, nbEBands, int);
		}

		OPUS_CLEAR(energyError, nbEBands*CC);
		c = 0;
		do {
			for (i = start; i < end; i++)
			{
				energyError[i + c * nbEBands] = MAX16(-QCONST16(0.5f, 15), MIN16(QCONST16(0.5f, 15), error[i + c * nbEBands]));
			}
		} while (++c < C);

		if (silence)
		{
			for (i = 0; i < C*nbEBands; i++)
				oldBandE[i] = -QCONST16(28.f, DB_SHIFT);
		}


		st->prefilter_period = pitch_index;
		st->prefilter_gain = gain1;
		st->prefilter_tapset = prefilter_tapset;


		if (CC == 2 && C == 1) {
			OPUS_COPY(&oldBandE[nbEBands], oldBandE, nbEBands);
		}

		if (!isTransient)
		{
			OPUS_COPY(oldLogE2, oldLogE, CC*nbEBands);
			OPUS_COPY(oldLogE, oldBandE, CC*nbEBands);
		}
		else {
			for (i = 0; i < CC*nbEBands; i++)
				oldLogE[i] = MIN16(oldLogE[i], oldBandE[i]);
		}
		/* In case start or end were to change */
		c = 0; do
		{
			for (i = 0; i < start; i++)
			{
				oldBandE[c*nbEBands + i] = 0;
				oldLogE[c*nbEBands + i] = oldLogE2[c*nbEBands + i] = -QCONST16(28.f, DB_SHIFT);
			}
			for (i = end; i < nbEBands; i++)
			{
				oldBandE[c*nbEBands + i] = 0;
				oldLogE[c*nbEBands + i] = oldLogE2[c*nbEBands + i] = -QCONST16(28.f, DB_SHIFT);
			}
		} while (++c < CC);

		if (isTransient || transient_got_disabled)
			st->consec_transient++;
		else
			st->consec_transient = 0;
		st->rng = enc->rng;

		/* If there's any room left (can only happen for very high rates),
		it's already filled with zeros */
		ec_enc_done(enc);

	}

	RESTORE_STACK;
	if (ec_get_error(enc))
		return OPUS_INTERNAL_ERROR;
	else
		return nbCompressedBytes;
}

int ql_celt_param_ctrl(CELTEncoder * OPUS_RESTRICT s, void **c, E_QL_OPUS_ENC_PARAM_T eParam)
{
	int ret = 0;
	switch (eParam)
	{
	case E_QL_OPUS_ENC_PARAM_CELT_GET_MODE:
		{
			CELTMode const **p = (CELTMode const**)c;
			*p = s->mode;
		}
		break;
	case E_QL_OPUS_ENC_PARAM_CELT_GET_ANALYSIS:
	{
		AnalysisInfo **p = (AnalysisInfo**)c;
		*p = &s->analysis;
	}
		break;
	case E_QL_OPUS_ENC_PARAM_GET_FINAL_RANGE:
	{
		opus_uint32 * *p = (opus_uint32**)c;
		*p = &s->rng;
	}
		break;
	case E_QL_OPUS_ENC_PARAM_SET_FINAL_COMPLEXITY:
	{
		opus_uint32 p = (opus_uint32)c;
		s->complexity = p;
	}
	break;
	case E_QL_OPUS_ENC_PARAM_SET_BITRATE:
	{
		opus_uint32 p = (opus_uint32)c;
		s->bitrate= p;
	}
		break;
	case E_QL_OPUS_ENC_PARAM_GET_FINAL_COMPLEXITY:
	{
		opus_uint32 * *p = (opus_uint32**)c;
		*p = (uint32_t*)s->complexity;
	}
	break;

	break;
	default:
		ret = -1;
		break;
	}
	return ret;
}
int opus_celt_resest_state(CELTEncoder * OPUS_RESTRICT st)
{
 
 
      {
         int i;
         opus_val16 *oldBandE, *oldLogE, *oldLogE2;
         oldBandE = (opus_val16*)(st->in_mem+st->channels*(st->mode->overlap+COMBFILTER_MAXPERIOD));
         oldLogE = oldBandE + st->channels*st->mode->nbEBands;
         oldLogE2 = oldLogE + st->channels*st->mode->nbEBands;
         OPUS_CLEAR((char*)&st->ENCODER_RESET_START,
               opus_custom_encoder_get_size(st->mode, st->channels)-
               ((char*)&st->ENCODER_RESET_START - (char*)st));
         for (i=0;i<st->channels*st->mode->nbEBands;i++)
            oldLogE[i]=oldLogE2[i]=-QCONST16(28.f,DB_SHIFT);
         st->vbr_offset = 0;
         st->delayedIntra = 1;
         st->spread_decision = SPREAD_NORMAL;
         st->tonal_average = 256;
         st->hf_average = 0;
         st->tapset_decision = 0;
      }
   return OPUS_UNIMPLEMENTED;
}
