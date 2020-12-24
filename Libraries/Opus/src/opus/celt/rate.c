/* Copyright (c) 2007-2008 CSIRO
   Copyright (c) 2007-2009 Xiph.Org Foundation
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include "modes.h"
#include "cwrs.h"
#include "arch.h"
#include "os_support.h"

#include "entcode.h"
#include "rate.h"

static const unsigned char LOG2_FRAC_TABLE[24]={
   0,
   8,13,
  16,19,21,23,
  24,26,27,28,29,30,31,32,
  32,33,34,34,35,36,36,37,37
};

#define ALLOC_STEPS 6

static OPUS_INLINE int interp_bits2pulses(const CELTMode *m, int start, int end, int skip_start,
      const int *bits1, const int *bits2, const int *thresh, const int *cap, opus_int32 total, opus_int32 *_balance,
      int skip_rsv, int *intensity, int intensity_rsv, int *dual_stereo, int dual_stereo_rsv, int *bits,
      int *ebits, int *fine_priority, int C, int LM, ec_ctx *ec, int encode, int prev, int signalBandwidth)
{
   opus_int32 psum;
   int lo, hi;
   int i, j;
   int logM;
   int stereo;
   int codedBands=-1;
   int alloc_floor;
   opus_int32 left, percoeff;
   int done;
   opus_int32 balance;
   SAVE_STACK;

   alloc_floor = C<<BITRES;
   stereo = C>1;

   logM = LM<<BITRES;
   lo = 0;
   hi = 1<<ALLOC_STEPS;
   for (i=0;i<ALLOC_STEPS;i++)
   {
      int mid = (lo+hi)>>1;
      psum = 0;
      done = 0;
      for (j=end;j-->start;)
      {
         int tmp = bits1[j] + (mid*(opus_int32)bits2[j]>>ALLOC_STEPS);
         if (tmp >= thresh[j] || done)
         {
            done = 1;
            /* Don't allocate more than we can actually use */
            psum += IMIN(tmp, cap[j]);
         } else {
            if (tmp >= alloc_floor)
               psum += alloc_floor;
         }
      }
      if (psum > total)
         hi = mid;
      else
         lo = mid;
   }
   psum = 0;
   /*printf ("interp bisection gave %d\n", lo);*/
   done = 0;
   for (j=end;j-->start;)
   {
      int tmp = bits1[j] + ((opus_int32)lo*bits2[j]>>ALLOC_STEPS);
      if (tmp < thresh[j] && !done)
      {
         if (tmp >= alloc_floor)
            tmp = alloc_floor;
         else
            tmp = 0;
      } else
         done = 1;
      /* Don't allocate more than we can actually use */
      tmp = IMIN(tmp, cap[j]);
      bits[j] = tmp;
      psum += tmp;
   }

   /* Decide which bands to skip, working backwards from the end. */
   for (codedBands=end;;codedBands--)
   {
      int band_width;
      int band_bits;
      int rem;
      j = codedBands-1;
      /* Never skip the first band, nor a band that has been boosted by
          dynalloc.
         In the first case, we'd be coding a bit to signal we're going to waste
          all the other bits.
         In the second case, we'd be coding a bit to redistribute all the bits
          we just signaled should be cocentrated in this band. */
      if (j<=skip_start)
      {
         /* Give the bit we reserved to end skipping back. */
         total += skip_rsv;
         break;
      }
      /*Figure out how many left-over bits we would be adding to this band.
        This can include bits we've stolen back from higher, skipped bands.*/
      left = total-psum;
      percoeff = celt_udiv(left, m->eBands[codedBands]-m->eBands[start]);
      left -= (m->eBands[codedBands]-m->eBands[start])*percoeff;
      rem = IMAX(left-(m->eBands[j]-m->eBands[start]),0);
      band_width = m->eBands[codedBands]-m->eBands[j];
      band_bits = (int)(bits[j] + percoeff*band_width + rem);
      /*Only code a skip decision if we're above the threshold for this band.
        Otherwise it is force-skipped.
        This ensures that we have enough bits to code the skip flag.*/
      if (band_bits >= IMAX(thresh[j], alloc_floor+(1<<BITRES)))
      {
         if (encode)
         {
            /*This if() block is the only part of the allocation function that
               is not a mandatory part of the bitstream: any bands we choose to
               skip here must be explicitly signaled.*/
            int depth_threshold;
            /*We choose a threshold with some hysteresis to keep bands from
               fluctuating in and out, but we try not to fold below a certain point. */
            if (codedBands > 17)
               depth_threshold = j<prev ? 7 : 9;
            else
               depth_threshold = 0;
#ifdef FUZZING
            if ((rand()&0x1) == 0)
#else
            if (codedBands<=start+2 || (band_bits > (depth_threshold*band_width<<LM<<BITRES)>>4 && j<=signalBandwidth))
#endif
            {
               ec_enc_bit_logp(ec, 1, 1);
               break;
            }
            ec_enc_bit_logp(ec, 0, 1);
         }
#ifdef ENABLE_DECODE
         else if (ec_dec_bit_logp(ec, 1)) {
            break;
         }
#endif
         /*We used a bit to skip this band.*/
         psum += 1<<BITRES;
         band_bits -= 1<<BITRES;
      }
      /*Reclaim the bits originally allocated to this band.*/
      psum -= bits[j]+intensity_rsv;
      if (intensity_rsv > 0)
         intensity_rsv = LOG2_FRAC_TABLE[j-start];
      psum += intensity_rsv;
      if (band_bits >= alloc_floor)
      {
         /*If we have enough for a fine energy bit per channel, use it.*/
         psum += alloc_floor;
         bits[j] = alloc_floor;
      } else {
         /*Otherwise this band gets nothing at all.*/
         bits[j] = 0;
      }
   }

   celt_assert(codedBands > start);
   /* Code the intensity and dual stereo parameters. */
   if (intensity_rsv > 0)
   {
      if (encode)
      {
         *intensity = IMIN(*intensity, codedBands);
         ec_enc_uint(ec, *intensity-start, codedBands+1-start);
      }
#ifdef ENABLE_DECODE
      else
         *intensity = start+ec_dec_uint(ec, codedBands+1-start);
#endif // ENABLE_DECODE
   }
   else
      *intensity = 0;
   if (*intensity <= start)
   {
      total += dual_stereo_rsv;
      dual_stereo_rsv = 0;
   }
   if (dual_stereo_rsv > 0)
   {
      if (encode)
         ec_enc_bit_logp(ec, *dual_stereo, 1);
#ifdef      ENABLE_DECODE
      else
         *dual_stereo = ec_dec_bit_logp(ec, 1);
#endif 
   }
   else
      *dual_stereo = 0;

   /* Allocate the remaining bits */
   left = total-psum;
   percoeff = celt_udiv(left, m->eBands[codedBands]-m->eBands[start]);
   left -= (m->eBands[codedBands]-m->eBands[start])*percoeff;
   for (j=start;j<codedBands;j++)
      bits[j] += ((int)percoeff*(m->eBands[j+1]-m->eBands[j]));
   for (j=start;j<codedBands;j++)
   {
      int tmp = (int)IMIN(left, m->eBands[j+1]-m->eBands[j]);
      bits[j] += tmp;
      left -= tmp;
   }
   /*for (j=0;j<end;j++)printf("%d ", bits[j]);printf("\n");*/

   balance = 0;
   for (j=start;j<codedBands;j++)
   {
      int N0, N, den;
      int offset;
      int NClogN;
      opus_int32 excess, bit;

      celt_assert(bits[j] >= 0);
      N0 = m->eBands[j+1]-m->eBands[j];
      N=N0<<LM;
      bit = (opus_int32)bits[j]+balance;

      if (N>1)
      {
         excess = MAX32(bit-cap[j],0);
         bits[j] = bit-excess;

         /* Compensate for the extra DoF in stereo */
         den=(C*N+ ((C==2 && N>2 && !*dual_stereo && j<*intensity) ? 1 : 0));

         NClogN = den*(m->logN[j] + logM);

         /* Offset for the number of fine bits by log2(N)/2 + FINE_OFFSET
            compared to their "fair share" of total/N */
         offset = (NClogN>>1)-den*FINE_OFFSET;

         /* N=2 is the only point that doesn't match the curve */
         if (N==2)
            offset += den<<BITRES>>2;

         /* Changing the offset for allocating the second and third
             fine energy bit */
         if (bits[j] + offset < den*2<<BITRES)
            offset += NClogN>>2;
         else if (bits[j] + offset < den*3<<BITRES)
            offset += NClogN>>3;

         /* Divide with rounding */
         ebits[j] = IMAX(0, (bits[j] + offset + (den<<(BITRES-1))));
         ebits[j] = celt_udiv(ebits[j], den)>>BITRES;

         /* Make sure not to bust */
         if (C*ebits[j] > (bits[j]>>BITRES))
            ebits[j] = bits[j] >> stereo >> BITRES;

         /* More than that is useless because that's about as far as PVQ can go */
         ebits[j] = IMIN(ebits[j], MAX_FINE_BITS);

         /* If we rounded down or capped this band, make it a candidate for the
             final fine energy pass */
         fine_priority[j] = ebits[j]*(den<<BITRES) >= bits[j]+offset;

         /* Remove the allocated fine bits; the rest are assigned to PVQ */
         bits[j] -= C*ebits[j]<<BITRES;

      } else {
         /* For N=1, all bits go to fine energy except for a single sign bit */
         excess = MAX32(0,bit-(C<<BITRES));
         bits[j] = bit-excess;
         ebits[j] = 0;
         fine_priority[j] = 1;
      }

      /* Fine energy can't take advantage of the re-balancing in
          quant_all_bands().
         Instead, do the re-balancing here.*/
      if(excess > 0)
      {
         int extra_fine;
         int extra_bits;
         extra_fine = IMIN(excess>>(stereo+BITRES),MAX_FINE_BITS-ebits[j]);
         ebits[j] += extra_fine;
         extra_bits = extra_fine*C<<BITRES;
         fine_priority[j] = extra_bits >= excess-balance;
         excess -= extra_bits;
      }
      balance = excess;

      celt_assert(bits[j] >= 0);
      celt_assert(ebits[j] >= 0);
   }
   /* Save any remaining bits over the cap for the rebalancing in
       quant_all_bands(). */
   *_balance = balance;

   /* The skipped bands use all their bits for fine energy. */
   for (;j<end;j++)
   {
      ebits[j] = bits[j] >> stereo >> BITRES;
      celt_assert(C*ebits[j]<<BITRES == bits[j]);
      bits[j] = 0;
      fine_priority[j] = ebits[j]<1;
   }
   RESTORE_STACK;
   return codedBands;
}

int compute_allocation(const CELTMode *m, int start, int end, const int *offsets, const int *cap, int alloc_trim, int *intensity, int *dual_stereo,
      opus_int32 total, opus_int32 *balance, int *pulses, int *ebits, int *fine_priority, int C, int LM, ec_ctx *ec, int encode, int prev, int signalBandwidth)
{
   int lo, hi, len, j;
   int codedBands;
   int skip_start;
   int skip_rsv;
   int intensity_rsv;
   int dual_stereo_rsv;
   VARDECL(int, bits1);
   VARDECL(int, bits2);
   VARDECL(int, thresh);
   VARDECL(int, trim_offset);
   SAVE_STACK;

   total = IMAX(total, 0);
   len = m->nbEBands;
   skip_start = start;
   /* Reserve a bit to signal the end of manually skipped bands. */
   skip_rsv = total >= 1<<BITRES ? 1<<BITRES : 0;
   total -= skip_rsv;
   /* Reserve bits for the intensity and dual stereo parameters. */
   intensity_rsv = dual_stereo_rsv = 0;
   if (C==2)
   {
      intensity_rsv = LOG2_FRAC_TABLE[end-start];
      if (intensity_rsv>total)
         intensity_rsv = 0;
      else
      {
         total -= intensity_rsv;
         dual_stereo_rsv = total>=1<<BITRES ? 1<<BITRES : 0;
         total -= dual_stereo_rsv;
      }
   }
   ALLOC(bits1, len, int);
   ALLOC(bits2, len, int);
   ALLOC(thresh, len, int);
   ALLOC(trim_offset, len, int);

   for (j=start;j<end;j++)
   {
      /* Below this threshold, we're sure not to allocate any PVQ bits */
      thresh[j] = IMAX((C)<<BITRES, (3*(m->eBands[j+1]-m->eBands[j])<<LM<<BITRES)>>4);
      /* Tilt of the allocation curve */
      trim_offset[j] = C*(m->eBands[j+1]-m->eBands[j])*(alloc_trim-5-LM)*(end-j-1)
            *(1<<(LM+BITRES))>>6;
      /* Giving less resolution to single-coefficient bands because they get
         more benefit from having one coarse value per coefficient*/
      if ((m->eBands[j+1]-m->eBands[j])<<LM==1)
         trim_offset[j] -= C<<BITRES;
   }
   lo = 1;
   hi = m->nbAllocVectors - 1;
   do
   {
      int done = 0;
      int psum = 0;
      int mid = (lo+hi) >> 1;
      for (j=end;j-->start;)
      {
         int bitsj;
         int N = m->eBands[j+1]-m->eBands[j];
         bitsj = C*N*m->allocVectors[mid*len+j]<<LM>>2;
         if (bitsj > 0)
            bitsj = IMAX(0, bitsj + trim_offset[j]);
         bitsj += offsets[j];
         if (bitsj >= thresh[j] || done)
         {
            done = 1;
            /* Don't allocate more than we can actually use */
            psum += IMIN(bitsj, cap[j]);
         } else {
            if (bitsj >= C<<BITRES)
               psum += C<<BITRES;
         }
      }
      if (psum > total)
         hi = mid - 1;
      else
         lo = mid + 1;
      /*printf ("lo = %d, hi = %d\n", lo, hi);*/
   }
   while (lo <= hi);
   hi = lo--;
   /*printf ("interp between %d and %d\n", lo, hi);*/
   for (j=start;j<end;j++)
   {
      int bits1j, bits2j;
      int N = m->eBands[j+1]-m->eBands[j];
      bits1j = C*N*m->allocVectors[lo*len+j]<<LM>>2;
      bits2j = hi>=m->nbAllocVectors ?
            cap[j] : C*N*m->allocVectors[hi*len+j]<<LM>>2;
      if (bits1j > 0)
         bits1j = IMAX(0, bits1j + trim_offset[j]);
      if (bits2j > 0)
         bits2j = IMAX(0, bits2j + trim_offset[j]);
      if (lo > 0)
         bits1j += offsets[j];
      bits2j += offsets[j];
      if (offsets[j]>0)
         skip_start = j;
      bits2j = IMAX(0,bits2j-bits1j);
      bits1[j] = bits1j;
      bits2[j] = bits2j;
   }
   codedBands = interp_bits2pulses(m, start, end, skip_start, bits1, bits2, thresh, cap,
         total, balance, skip_rsv, intensity, intensity_rsv, dual_stereo, dual_stereo_rsv,
         pulses, ebits, fine_priority, C, LM, ec, encode, prev, signalBandwidth);

   FREE1(bits1, len, int);
   FREE1(bits2, len, int);
   FREE1(thresh, len, int);
   FREE1(trim_offset, len, int);
   RESTORE_STACK;
   return codedBands;
}

