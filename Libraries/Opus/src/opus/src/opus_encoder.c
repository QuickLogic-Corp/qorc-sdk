/* Copyright (c) 2010-2011 Xiph.Org Foundation, Skype Limited
   Written by Jean-Marc Valin and Koen Vos */
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

#include "ql_opus.h" //  not really needed here... can be removed when the reconfig is removed.

#include <stdarg.h>
#include "celt.h"
#include "entenc.h"
#include "modes.h"
#ifdef ENABLE_SILK
#include "API.h"
#endif
#include "stack_alloc.h"
#include "float_cast.h"
#include "opus.h"
#include "arch.h"
#include "pitch.h"
#include "opus_private.h"
#include "os_support.h"
#include "cpu_support.h"
#include "analysis.h"
#include "mathops.h"
#ifdef ENABLE_SILK
#include "tuning_parameters.h"
#ifdef FIXED_POINT
#include "fixed/structs_FIX.h"
#else
#include "float/structs_FLP.h"
#endif
#endif

#define MAX_ENCODER_BUFFER 480

#ifndef DISABLE_FLOAT_API
#define PSEUDO_SNR_THRESHOLD 316.23f    /* 10^(25/10) */
#endif

#ifdef CMSIS_OPT
#include <arm_math.h>
#endif


typedef struct {
   opus_val32 XX, XY, YY;
   opus_val16 smoothed_width;
   opus_val16 max_follower;
} StereoWidthState;

struct OpusEncoder {
    int          celt_enc_offset;
    int          silk_enc_offset;
#ifdef ENABLE_SILK
    silk_EncControlStruct silk_mode;
#endif
    int          application;
    int          channels;
    int          delay_compensation;
    int          force_channels;
    int          signal_type;
    int          user_bandwidth;
    int          max_bandwidth;
    int          user_forced_mode;
    int          voice_ratio;
    opus_int32   Fs;
    int          use_vbr;
    int          vbr_constraint;
    int          variable_duration;
    opus_int32   bitrate_bps;
    opus_int32   user_bitrate_bps;
    int          lsb_depth;
    int          encoder_buffer;
    int          lfe;
    int          arch;
    int          use_dtx;                 /* general DTX for both SILK and CELT */
#ifndef DISABLE_FLOAT_API
    TonalityAnalysisState analysis;
#endif

#define OPUS_ENCODER_RESET_START stream_channels
    int          stream_channels;
    opus_int16   hybrid_stereo_width_Q14;
    opus_int32   variable_HP_smth2_Q15;
 //   opus_val16   prev_HB_gain;
    opus_val32   hp_mem[4];
    int          mode;
    int          prev_mode;
    int          prev_channels;
    int          prev_framesize;
    int          bandwidth;
    /* Bandwidth determined automatically from the rate (before any other adjustment) */
    int          auto_bandwidth;
    int          silk_bw_switch;
    /* Sampling rate (at the API level) */
    int          first;
    opus_val16 * energy_masking;
    StereoWidthState width_mem;
    opus_val16   delay_buffer[MAX_ENCODER_BUFFER*2];
#ifndef DISABLE_FLOAT_API
    int          detected_bandwidth;
    int          nb_no_activity_frames;
    opus_val32   peak_signal_energy;
#endif
    int          nonfinal_frame; /* current frame is not the final in a packet */
    opus_uint32  rangeFinal;
};

/* Transition tables for the voice and music. First column is the
   middle (memoriless) threshold. The second column is the hysteresis
   (difference with the middle) */
static const opus_int32 mono_voice_bandwidth_thresholds[8] = {
        10000, 1000, /* NB<->MB */
        11000, 1000, /* MB<->WB */
        13500, 1000, /* WB<->SWB */
        14000, 2000, /* SWB<->FB */
};
static const opus_int32 mono_music_bandwidth_thresholds[8] = {
         9000, 1000, /* NB<->MB */
        10000, 1000, /* MB<->WB */
        11000, 1000, /* WB<->SWB */
        12000, 2000, /* SWB<->FB */
};
static const opus_int32 stereo_voice_bandwidth_thresholds[8] = {
        10000, 1000, /* NB<->MB */
        11000, 1000, /* MB<->WB */
        13500, 1000, /* WB<->SWB */
        14000, 2000, /* SWB<->FB */
};
static const opus_int32 stereo_music_bandwidth_thresholds[8] = {
         9000, 1000, /* NB<->MB */
        10000, 1000, /* MB<->WB */
        11000, 1000, /* WB<->SWB */
        12000, 2000, /* SWB<->FB */
};
/* Threshold bit-rates for switching between mono and stereo */
//static const opus_int32 stereo_voice_threshold = 19000;
//static const opus_int32 stereo_music_threshold = 17000;
#ifdef ENABLE_SILK
/* Threshold bit-rate for switching between SILK/hybrid and CELT-only */
static const opus_int32 mode_thresholds[2][2] = {
      /* voice */ /* music */
      {  64000,      10000}, /* mono */
      {  44000,      10000}, /* stereo */
};

static const opus_int32 fec_thresholds[] = {
        12000, 1000, /* NB */
        14000, 1000, /* MB */
        16000, 1000, /* WB */
        20000, 1000, /* SWB */
        22000, 1000, /* FB */
};
#endif

int ql_opus_param_ctrl(OpusEncoder *opus, void **c, E_QL_OPUS_ENC_PARAM_T eParam);

static opus_int32 silk_lin2log(
	const opus_int32            inLin               /* I  input in linear scale                                         */
);

int opus_encoder_get_size(int channels)
{
    int silkEncSizeBytes = 0, celtEncSizeBytes;
//    int ret;
    if (channels<1 || channels > 2)
        return 0;
#ifdef ENABLE_SILK
    int ret = silk_Get_Encoder_Size( &silkEncSizeBytes );
    if (ret)
        return 0;
    silkEncSizeBytes = align(silkEncSizeBytes);
#endif
	celtEncSizeBytes = celt_encoder_get_size(channels);
    return align(sizeof(OpusEncoder))+silkEncSizeBytes+celtEncSizeBytes;
}

int opus_encoder_init(OpusEncoder* st, opus_int32 Fs, int channels, int application)
{
    CELTEncoder *celt_enc;
    int err;
	int silkEncSizeBytes = 0;

   if((Fs!=48000&&Fs!=24000&&Fs!=16000&&Fs!=12000&&Fs!=8000)||(channels!=1&&channels!=2)||
        (application != OPUS_APPLICATION_VOIP && application != OPUS_APPLICATION_AUDIO
        && application != OPUS_APPLICATION_RESTRICTED_LOWDELAY))
        return OPUS_BAD_ARG;

    OPUS_CLEAR((char*)st, opus_encoder_get_size(channels));

	st->silk_enc_offset = 0;
	st->celt_enc_offset = align(sizeof(OpusEncoder)) + st->silk_enc_offset + silkEncSizeBytes;
    celt_enc = (CELTEncoder*)((char*)st+st->celt_enc_offset);
    st->stream_channels = st->channels = channels;
    st->Fs = Fs;
    st->arch = opus_select_arch();

    /* Create CELT encoder */
    /* Initialize CELT encoder */
    err = celt_encoder_init(celt_enc, Fs, channels, st->arch);
    if(err!=OPUS_OK)return OPUS_INTERNAL_ERROR;

 //   celt_encoder_ctl(celt_enc, CELT_SET_SIGNALLING(0));

//	celt_encoder_ctl(celt_enc, OPUS_SET_COMPLEXITY(QL_OPUS_ENC_COMPLEXITY));

    st->use_vbr = 0;
    /* Makes constrained VBR the default (safer for real-time use) */
    st->vbr_constraint = 1;
    st->user_bitrate_bps = OPUS_AUTO;
    st->bitrate_bps = 3000+Fs*channels;
    st->application = application;
    st->signal_type = OPUS_AUTO;
    st->user_bandwidth = OPUS_AUTO;
    st->max_bandwidth = OPUS_BANDWIDTH_FULLBAND;
    st->force_channels = OPUS_AUTO;
    st->user_forced_mode = MODE_CELT_ONLY;
    st->voice_ratio = -1;
    st->encoder_buffer = st->Fs/100;
    st->lsb_depth = 16;
    st->variable_duration = OPUS_FRAMESIZE_ARG;


	
    /* Delay compensation of 4 ms (2.5 ms for SILK's extra look-ahead
       + 1.5 ms for SILK resamplers and stereo prediction) */
    st->delay_compensation = st->Fs/250;

    st->hybrid_stereo_width_Q14 = 1 << 14;
 //   st->prev_HB_gain = Q15ONE;
#if 1 //def ENABLE_SILK
#define VARIABLE_HP_MIN_CUTOFF_HZ                       60
#define silk_LSHIFT32(a, shift)             ((opus_int32)((opus_uint32)(a)<<(shift)))       /* shift >= 0, shift < 32 */

#define silk_LSHIFT(a, shift)               silk_LSHIFT32(a, shift)                         /* shift >= 0, shift < 32 */

    st->variable_HP_smth2_Q15 = silk_LSHIFT( silk_lin2log( VARIABLE_HP_MIN_CUTOFF_HZ ), 8 );
#endif
    st->first = 1;
    st->mode = MODE_CELT_ONLY;
    st->bandwidth = OPUS_BANDWIDTH_FULLBAND;

#ifndef DISABLE_FLOAT_API
    tonality_analysis_init(&st->analysis, st->Fs);
    st->analysis.application = st->application;
#endif

    return OPUS_OK;
}

static unsigned char gen_toc(int mode, int framerate, int bandwidth, int channels)
{
   int period;
   unsigned char toc;
   period = 0;
   while (framerate < 400)
   {
       framerate <<= 1;
       period++;
   }
   if (mode == MODE_SILK_ONLY)
   {
       toc = (bandwidth-OPUS_BANDWIDTH_NARROWBAND)<<5;
       toc |= (period-2)<<3;
   } else if (mode == MODE_CELT_ONLY)
   {
       int tmp = bandwidth-OPUS_BANDWIDTH_MEDIUMBAND;
       if (tmp < 0)
           tmp = 0;
       toc = 0x80;
       toc |= tmp << 5;
       toc |= period<<3;
   } else /* Hybrid */
   {
       toc = 0x60;
       toc |= (bandwidth-OPUS_BANDWIDTH_SUPERWIDEBAND)<<4;
       toc |= (period-2)<<3;
   }
   toc |= (channels==2)<<2;
   return toc;
}

#ifndef FIXED_POINT
static void silk_biquad_float(
    const opus_val16      *in,            /* I:    Input signal                   */
    const opus_int32      *B_Q28,         /* I:    MA coefficients [3]            */
    const opus_int32      *A_Q28,         /* I:    AR coefficients [2]            */
    opus_val32            *S,             /* I/O:  State vector [2]               */
    opus_val16            *out,           /* O:    Output signal                  */
    const opus_int32      len,            /* I:    Signal length (must be even)   */
    int stride
)
{
    /* DIRECT FORM II TRANSPOSED (uses 2 element state vector) */
    opus_int   k;
    opus_val32 vout;
    opus_val32 inval;
    opus_val32 A[2], B[3];

    A[0] = (opus_val32)(A_Q28[0] * (1.f/((opus_int32)1<<28)));
    A[1] = (opus_val32)(A_Q28[1] * (1.f/((opus_int32)1<<28)));
    B[0] = (opus_val32)(B_Q28[0] * (1.f/((opus_int32)1<<28)));
    B[1] = (opus_val32)(B_Q28[1] * (1.f/((opus_int32)1<<28)));
    B[2] = (opus_val32)(B_Q28[2] * (1.f/((opus_int32)1<<28)));

    /* Negate A_Q28 values and split in two parts */

    for( k = 0; k < len; k++ ) {
        /* S[ 0 ], S[ 1 ]: Q12 */
        inval = in[ k*stride ];
        vout = S[ 0 ] + B[0]*inval;

        S[ 0 ] = S[1] - vout*A[0] + B[1]*inval;

        S[ 1 ] = - vout*A[1] + B[2]*inval + VERY_SMALL;

        /* Scale back to Q0 and saturate */
        out[ k*stride ] = vout;
    }
}
#endif


#ifndef ENABLE_SILK
#define silk_int64_MAX   ((opus_int64)0x7FFFFFFFFFFFFFFFLL)   /*  2^63 - 1 */
#define silk_int64_MIN   ((opus_int64)0x8000000000000000LL)   /* -2^63 */
#define silk_int32_MAX   0x7FFFFFFF                           /*  2^31 - 1 =  2147483647 */
#define silk_int32_MIN   ((opus_int32)0x80000000)             /* -2^31     = -2147483648 */
#define silk_int16_MAX   0x7FFF                               /*  2^15 - 1 =  32767 */
#define silk_int16_MIN   ((opus_int16)0x8000)                 /* -2^15     = -32768 */
#define silk_int8_MAX    0x7F                                 /*  2^7 - 1  =  127 */
#define silk_int8_MIN    ((opus_int8)0x80)                    /* -2^7      = -128 */
#define silk_uint8_MAX   0xFF                                 /*  2^8 - 1 = 255 */


/* DTX settings */
#define NB_SPEECH_FRAMES_BEFORE_DTX             10      /* eq 200 ms */
#define MAX_CONSECUTIVE_DTX                     20      /* eq 400 ms */
#define DTX_ACTIVITY_THRESHOLD                  0.1f

#define VARIABLE_HP_MIN_CUTOFF_HZ                       60
#define VARIABLE_HP_SMTH_COEF2                          0.015f

#  define silk_assert(COND) {if (!(COND)) {printf("assertion failed: %s" #COND);}}
#define SILK_FIX_CONST( C, Q )              ((opus_int32)((C) * ((opus_int64)1 << (Q)) + 0.5))

#define silk_DIV32_16(a32, b16)             ((opus_int32)((a32) / (b16)))
/* (opus_int32)((opus_int16)(a3))) * (opus_int32)((opus_int16)(b32)) output have to be 32bit int */
#define silk_SMULBB(a32, b32)            ((opus_int32)((opus_int16)(a32)) * (opus_int32)((opus_int16)(b32)))
#define silk_MUL(a32, b32)                  ((a32) * (b32))
#define silk_LSHIFT(a, shift)               silk_LSHIFT32(a, shift)                         /* shift >= 0, shift < 32 */
#define silk_RSHIFT(a, shift)               silk_RSHIFT32(a, shift)                         /* shift >= 0, shift < 32 */
#define silk_ADD_LSHIFT32(a, b, shift)      silk_ADD32((a), silk_LSHIFT32((b), (shift)))    /* shift >= 0 */
#define silk_LSHIFT32(a, shift)             ((opus_int32)((opus_uint32)(a)<<(shift)))       /* shift >= 0, shift < 32 */
#define silk_RSHIFT32(a, shift)             ((a)>>(shift))                                  /* shift >= 0, shift < 32 */
/* a32 + (b32 * c32) output have to be 32bit int */
#define silk_MLA(a32, b32, c32)             silk_ADD32((a32),((b32) * (c32)))

/* (a32 * b32) >> 16 */
#if OPUS_FAST_INT64
#define silk_SMULWW(a32, b32)            ((opus_int32)(((opus_int64)(a32) * (b32)) >> 16))
#else
#define silk_SMULWW(a32, b32)            silk_MLA(silk_SMULWB((a32), (b32)), (a32), silk_RSHIFT_ROUND((b32), 16))
#endif


/* These macros enables checking for overflow in silk_API_Debug.h*/
#define silk_ADD16(a, b)                    ((a) + (b))
#define silk_ADD32(a, b)                    ((a) + (b))
#define silk_ADD64(a, b)                    ((a) + (b))

#define silk_SUB16(a, b)                    ((a) - (b))
#define silk_SUB32(a, b)                    ((a) - (b))
#define silk_SUB64(a, b)                    ((a) - (b))

/* (a32 * (opus_int32)((opus_int16)(b32))) >> 16 output have to be 32bit int */
#if OPUS_FAST_INT64
#define silk_SMULWB(a32, b32)            ((opus_int32)(((a32) * (opus_int64)((opus_int16)(b32))) >> 16))
#else
#define silk_SMULWB(a32, b32)            ((((a32) >> 16) * (opus_int32)((opus_int16)(b32))) + ((((a32) & 0x0000FFFF) * (opus_int32)((opus_int16)(b32))) >> 16))
#endif
/* Requires that shift > 0 */
#define silk_RSHIFT_ROUND(a, shift)         ((shift) == 1 ? ((a) >> 1) + ((a) & 1) : (((a) >> ((shift) - 1)) + 1) >> 1)
#define silk_RSHIFT_ROUND64(a, shift)       ((shift) == 1 ? ((a) >> 1) + ((a) & 1) : (((a) >> ((shift) - 1)) + 1) >> 1)


/* a32 + (b32 * (opus_int32)((opus_int16)(c32))) >> 16 output have to be 32bit int */
#if OPUS_FAST_INT64
#define silk_SMLAWB(a32, b32, c32)       ((opus_int32)((a32) + (((b32) * (opus_int64)((opus_int16)(c32))) >> 16)))
#else
#define silk_SMLAWB(a32, b32, c32)       ((a32) + ((((b32) >> 16) * (opus_int32)((opus_int16)(c32))) + ((((b32) & 0x0000FFFF) * (opus_int32)((opus_int16)(c32))) >> 16)))
#endif

#define silk_ADD_RSHIFT32(a, b, shift)      silk_ADD32((a), silk_RSHIFT32((b), (shift)))    /* shift >= 0 */


#define silk_min(a, b)                      (((a) < (b)) ? (a) : (b))
#define silk_max(a, b)                      (((a) > (b)) ? (a) : (b))

#ifndef OVERRIDE_silk_CLZ32
static OPUS_INLINE opus_int32 silk_CLZ32(opus_int32 in32)
{
	return in32 ? 32 - EC_ILOG(in32) : 32;
}
#endif

/********************************************************************/
/*                                MACROS                            */
/********************************************************************/

/* Rotate a32 right by 'rot' bits. Negative rot values result in rotating
left. Output is 32bit int.
Note: contemporary compilers recognize the C expression below and
compile it into a 'ror' instruction if available. No need for OPUS_INLINE ASM! */
static OPUS_INLINE opus_int32 silk_ROR32(opus_int32 a32, opus_int rot)
{
	opus_uint32 x = (opus_uint32)a32;
	opus_uint32 r = (opus_uint32)rot;
	opus_uint32 m = (opus_uint32)-rot;
	if (rot == 0) {
		return a32;
	}
	else if (rot < 0) {
		return (opus_int32)((x << m) | (x >> (32 - m)));
	}
	else {
		return (opus_int32)((x << (32 - r)) | (x >> r));
	}
}

/* get number of leading zeros and fractional part (the bits right after the leading one */
static OPUS_INLINE void silk_CLZ_FRAC(
	opus_int32 in,            /* I  input                               */
	opus_int32 *lz,           /* O  number of leading zeros             */
	opus_int32 *frac_Q7       /* O  the 7 bits right after the leading one */
)
{
	opus_int32 lzeros = silk_CLZ32(in);

	*lz = lzeros;
	*frac_Q7 = silk_ROR32(in, 24 - lzeros) & 0x7f;
}


/* Approximation of 128 * log2() (very close inverse of silk_log2lin()) */
/* Convert input to a log scale    */
static opus_int32 silk_lin2log(
	const opus_int32            inLin               /* I  input in linear scale                                         */
)
{
	opus_int32 lz, frac_Q7;

	silk_CLZ_FRAC(inLin, &lz, &frac_Q7);

	/* Piece-wise parabolic approximation */
	return silk_ADD_LSHIFT32(silk_SMLAWB(frac_Q7, silk_MUL(frac_Q7, 128 - frac_Q7), 179), 31 - lz, 7);
}


/* Approximation of 2^() (very close inverse of silk_lin2log()) */
/* Convert input to a linear scale    */
static opus_int32 silk_log2lin(
	const opus_int32            inLog_Q7            /* I  input on log scale                                            */
)
{
	opus_int32 out, frac_Q7;

	if (inLog_Q7 < 0) {
		return 0;
	}
	else if (inLog_Q7 >= 3967) {
		return silk_int32_MAX;
	}

	out = silk_LSHIFT(1, silk_RSHIFT(inLog_Q7, 7));
	frac_Q7 = inLog_Q7 & 0x7F;
	if (inLog_Q7 < 2048) {
		/* Piece-wise parabolic approximation */
		out = silk_ADD_RSHIFT32(out, silk_MUL(out, silk_SMLAWB(frac_Q7, silk_SMULBB(frac_Q7, 128 - frac_Q7), -174)), 7);
	}
	else {
		/* Piece-wise parabolic approximation */
		out = silk_MLA(out, silk_RSHIFT(out, 7), silk_SMLAWB(frac_Q7, silk_SMULBB(frac_Q7, 128 - frac_Q7), -174));
	}
	return out;
}

#endif  /* !ENABLE_SILK */
static void hp_cutoff(const opus_val16 *in, opus_int32 cutoff_Hz, opus_val16 *out, opus_val32 *hp_mem, int len, int channels, opus_int32 Fs, int arch)
{
   opus_int32 B_Q28[ 3 ], A_Q28[ 2 ];
   opus_int32 Fc_Q19, r_Q28, r_Q22;
   (void)arch;

//   silk_assert( cutoff_Hz <= silk_int32_MAX / SILK_FIX_CONST( 1.5 * 3.14159 / 1000, 19 ) );
   Fc_Q19 = silk_DIV32_16( silk_SMULBB( SILK_FIX_CONST( 1.5 * 3.14159 / 1000, 19 ), cutoff_Hz ), Fs/1000 );
//   silk_assert( Fc_Q19 > 0 && Fc_Q19 < 32768 );

   r_Q28 = SILK_FIX_CONST( 1.0, 28 ) - silk_MUL( SILK_FIX_CONST( 0.92, 9 ), Fc_Q19 );

   /* b = r * [ 1; -2; 1 ]; */
   /* a = [ 1; -2 * r * ( 1 - 0.5 * Fc^2 ); r^2 ]; */
   B_Q28[ 0 ] = r_Q28;
   B_Q28[ 1 ] = silk_LSHIFT( -r_Q28, 1 );
   B_Q28[ 2 ] = r_Q28;

   /* -r * ( 2 - Fc * Fc ); */
   r_Q22  = silk_RSHIFT( r_Q28, 6 );
   A_Q28[ 0 ] = silk_SMULWW( r_Q22, silk_SMULWW( Fc_Q19, Fc_Q19 ) - SILK_FIX_CONST( 2.0,  22 ) );
   A_Q28[ 1 ] = silk_SMULWW( r_Q22, r_Q22 );

#ifdef FIXED_POINT
   if( channels == 1 ) {
      silk_biquad_alt_stride1( in, B_Q28, A_Q28, hp_mem, out, len );
   } else {
      silk_biquad_alt_stride2( in, B_Q28, A_Q28, hp_mem, out, len, arch );
   }
#else
   silk_biquad_float( in, B_Q28, A_Q28, hp_mem, out, len, channels );
   if( channels == 2 ) {
       silk_biquad_float( in+1, B_Q28, A_Q28, hp_mem+2, out+1, len, channels );
   }
#endif
}

#ifdef FIXED_POINT
static void dc_reject(const opus_val16 *in, opus_int32 cutoff_Hz, opus_val16 *out, opus_val32 *hp_mem, int len, int channels, opus_int32 Fs)
{
   int c, i;
   int shift;

   /* Approximates -round(log2(6.3*cutoff_Hz/Fs)) */
   shift=celt_ilog2(Fs/(cutoff_Hz*4));
   for (c=0;c<channels;c++)
   {
      for (i=0;i<len;i++)
      {
         opus_val32 x, y;
         x = SHL32(EXTEND32(in[channels*i+c]), 14);
         y = x-hp_mem[2*c];
         hp_mem[2*c] = hp_mem[2*c] + PSHR32(x - hp_mem[2*c], shift);
         out[channels*i+c] = EXTRACT16(SATURATE(PSHR32(y, 14), 32767));
      }
   }
}

#else
static void dc_reject(const opus_val16 *in, opus_int32 cutoff_Hz, opus_val16 *out, opus_val32 *hp_mem, int len, int channels, opus_int32 Fs)
{
   int i;
   float coef, coef2;
   coef = 6.3f*cutoff_Hz/Fs;
   coef2 = 1-coef;
   if (channels==2)
   {
      float m0, m2;
      m0 = hp_mem[0];
      m2 = hp_mem[2];
      for (i=0;i<len;i++)
      {
         opus_val32 x0, x1, out0, out1;
         x0 = in[2*i+0];
         x1 = in[2*i+1];
         out0 = x0-m0;
         out1 = x1-m2;
         m0 = coef*x0 + VERY_SMALL + coef2*m0;
         m2 = coef*x1 + VERY_SMALL + coef2*m2;
         out[2*i+0] = out0;
         out[2*i+1] = out1;
      }
      hp_mem[0] = m0;
      hp_mem[2] = m2;
   } else {
      float m0;
      m0 = hp_mem[0];
      for (i=0;i<len;i++)
      {
         opus_val32 x, y;
         x = in[i];
         y = x-m0;
         m0 = coef*x + VERY_SMALL + coef2*m0;
         out[i] = y;
      }
      hp_mem[0] = m0;
   }
}
#endif

OpusEncoder *opus_encoder_create(opus_int32 Fs, int channels, int application, int *error)
{
   int ret;
   OpusEncoder *st;
   if((Fs!=48000&&Fs!=24000&&Fs!=16000&&Fs!=12000&&Fs!=8000)||(channels!=1&&channels!=2)||
       (application != OPUS_APPLICATION_VOIP && application != OPUS_APPLICATION_AUDIO
       && application != OPUS_APPLICATION_RESTRICTED_LOWDELAY))
   {
      if (error)
         *error = OPUS_BAD_ARG;
      return NULL;
   }
   st = (OpusEncoder *)opus_alloc(opus_encoder_get_size(channels));
   if (st == NULL)
   {
      if (error)
         *error = OPUS_ALLOC_FAIL;
      return NULL;
   }
   ret = opus_encoder_init(st, Fs, channels, application);
   if (error)
      *error = ret;
   if (ret != OPUS_OK)
   {
      opus_free(st);
      st = NULL;
   }
   st->user_bandwidth = OPUS_BANDWIDTH_WIDEBAND;
   st->use_vbr = 0;
   st->vbr_constraint = 0;
   st->force_channels = OPUS_AUTO;
   st->use_dtx = 0;
   st->variable_duration = OPUS_FRAMESIZE_5_MS;

 //  st->bitrate_bps = QL_OPUS_ENC_BITRATE;
   st->force_channels = OPUS_AUTO;
   st->use_dtx = 0;
   st->lsb_depth = 16;
   
   return st;
}

static opus_int32 user_bitrate_to_bitrate(OpusEncoder *st, int frame_size, int max_data_bytes)
{
  if(!frame_size)frame_size=st->Fs/400;
  if (st->user_bitrate_bps==OPUS_AUTO)
    return 60*st->Fs/frame_size + st->Fs*st->channels;
  else if (st->user_bitrate_bps==OPUS_BITRATE_MAX)
    return max_data_bytes*8*st->Fs/frame_size;
  else
    return st->user_bitrate_bps;
}

#ifndef DISABLE_FLOAT_API
#ifdef FIXED_POINT
#define PCM2VAL(x) FLOAT2INT16(x)
#else
#define PCM2VAL(x) SCALEIN(x)
#endif

#endif

void downmix_int(const void *_x, opus_val32 *y, int subframe, int offset, int c1, int c2, int C)
{
   const opus_int16 *x;
   int j;

   x = (const opus_int16 *)_x;
   for (j=0;j<subframe;j++)
      y[j] = x[(j+offset)*C+c1];
   if (c2>-1)
   {
      for (j=0;j<subframe;j++)
         y[j] += x[(j+offset)*C+c2];
   } else if (c2==-2)
   {
      int c;
      for (c=1;c<C;c++)
      {
         for (j=0;j<subframe;j++)
            y[j] += x[(j+offset)*C+c];
      }
   }
}

opus_int32 frame_size_select(opus_int32 frame_size, int variable_duration, opus_int32 Fs)
{
#ifndef QL_ENC_OPTION1
	int new_size = frame_size;
#else
   int new_size;
   if (frame_size<Fs/400)
      return -1;
   if (variable_duration == OPUS_FRAMESIZE_ARG)
      new_size = frame_size;
   else if (variable_duration >= OPUS_FRAMESIZE_2_5_MS && variable_duration <= OPUS_FRAMESIZE_120_MS)
   {
      if (variable_duration <= OPUS_FRAMESIZE_40_MS)
         new_size = (Fs/400)<<(variable_duration-OPUS_FRAMESIZE_2_5_MS);
      else
         new_size = (variable_duration-OPUS_FRAMESIZE_2_5_MS-2)*Fs/50;
   }
   else
      return -1;
   if (new_size>frame_size)
      return -1;
   if (400*new_size!=Fs   && 200*new_size!=Fs   && 100*new_size!=Fs   &&
        50*new_size!=Fs   &&  25*new_size!=Fs   &&  50*new_size!=3*Fs &&
        50*new_size!=4*Fs &&  50*new_size!=5*Fs &&  50*new_size!=6*Fs)
      return -1;
#endif
   return new_size;
}

/* Returns the equivalent bitrate corresponding to 20 ms frames,
   complexity 10 VBR operation. */
static opus_int32 compute_equiv_rate(opus_int32 bitrate, int channels,
      int frame_rate, int vbr, int mode, int complexity, int loss)
{
   opus_int32 equiv;
   equiv = bitrate;
   /* Take into account overhead from smaller frames. */
   equiv -= (40*channels+20)*(frame_rate - 50);
   /* CBR is about a 8% penalty for both SILK and CELT. */
   if (!vbr)
      equiv -= equiv/12;
   /* Complexity makes about 10% difference (from 0 to 10) in general. */
   equiv = equiv * (90+complexity)/100;
   if (mode == MODE_SILK_ONLY || mode == MODE_HYBRID)
   {
      /* SILK complexity 0-1 uses the non-delayed-decision NSQ, which
         costs about 20%. */
      if (complexity<2)
         equiv = equiv*4/5;
      equiv -= equiv*loss/(6*loss + 10);
   } else if (mode == MODE_CELT_ONLY) {
      /* CELT complexity 0-4 doesn't have the pitch filter, which costs
         about 10%. */
      if (complexity<5)
         equiv = equiv*9/10;
   } else {
      /* Mode not known yet */
      /* Half the SILK loss*/
      equiv -= equiv*loss/(12*loss + 20);
   }
   return equiv;
}

#ifndef DISABLE_FLOAT_API
#if (QL_OPUS_ENC_COMPLEXITY >= 5)
static int is_digital_silence(const opus_val16* pcm, int frame_size, int channels, int lsb_depth)
{
   int silence = 0;
   opus_val32 sample_max = 0;
#ifdef MLP_TRAINING
   return 0;
#endif
   sample_max = celt_maxabs16(pcm, frame_size*channels);

#ifdef FIXED_POINT
   silence = (sample_max == 0);
   (void)lsb_depth;
#else
   silence = (sample_max <= (opus_val16) 1 / (1 << lsb_depth));
#endif

   return silence;
}

static opus_val32 compute_frame_energy(const opus_val16 *pcm, int frame_size, int channels, int arch)
{
   int len = frame_size*channels;
   return celt_inner_prod(pcm, pcm, len, arch)/len;
}
#endif 

#endif

static int compute_redundancy_bytes(opus_int32 max_data_bytes, opus_int32 bitrate_bps, int frame_rate, int channels)
{
   int redundancy_bytes_cap;
   int redundancy_bytes;
   opus_int32 redundancy_rate;
   int base_bits;
   opus_int32 available_bits;
   base_bits = (40*channels+20);

   /* Equivalent rate for 5 ms frames. */
   redundancy_rate = bitrate_bps + base_bits*(200 - frame_rate);
   /* For VBR, further increase the bitrate if we can afford it. It's pretty short
      and we'll avoid artefacts. */
   redundancy_rate = 3*redundancy_rate/2;
   redundancy_bytes = redundancy_rate/1600;

   /* Compute the max rate we can use given CBR or VBR with cap. */
   available_bits = max_data_bytes*8 - 2*base_bits;
   redundancy_bytes_cap = (available_bits*240/(240+48000/frame_rate) + base_bits)/8;
   redundancy_bytes = IMIN(redundancy_bytes, redundancy_bytes_cap);
   /* It we can't get enough bits for redundancy to be worth it, rely on the decoder PLC. */
   if (redundancy_bytes > 4 + 8*channels)
      redundancy_bytes = IMIN(257, redundancy_bytes);
   else
      redundancy_bytes = 0;
   return redundancy_bytes;
}
#include "ql_opus.h"

int adapt_parameters(OpusEncoder *st, int is_silence, AnalysisInfo *p_analysis_info, int frame_size, opus_int32 out_data_bytes, opus_int32 *p_max_data_bytes)
{
	opus_int32 max_rate;
	int voice_est;
	opus_int32 equiv_rate;
	opus_int32 max_data_bytes = *p_max_data_bytes;

	if (frame_size <= 0 || max_data_bytes <= 0)
	{
		return -1;
	}

	/* Cannot encode 100 ms in 1 byte */
	if (max_data_bytes == 1 && st->Fs == (frame_size * 10))
	{
		return -1;
	}
#ifndef DISABLE_FLOAT_API
	/* Reset voice_ratio if this frame is not silent or if analysis is disabled.
	* Otherwise, preserve voice_ratio from the last non-silent frame */
	if (!is_silence)
		st->voice_ratio = -1;

	st->detected_bandwidth = 0;
	if (p_analysis_info->valid)
	{
		int analysis_bandwidth;
		if (st->signal_type == OPUS_AUTO)
		{
			float prob;
			if (st->prev_mode == 0)
				prob = p_analysis_info->music_prob;
			else if (st->prev_mode == MODE_CELT_ONLY)
				prob = p_analysis_info->music_prob_max;
			else
				prob = p_analysis_info->music_prob_min;
			st->voice_ratio = (int)floorf(.5f + 100 * (1 - prob));
		}

		analysis_bandwidth = p_analysis_info->bandwidth;
		if (analysis_bandwidth <= 12)
			st->detected_bandwidth = OPUS_BANDWIDTH_NARROWBAND;
		else if (analysis_bandwidth <= 14)
			st->detected_bandwidth = OPUS_BANDWIDTH_MEDIUMBAND;
		else if (analysis_bandwidth <= 16)
			st->detected_bandwidth = OPUS_BANDWIDTH_WIDEBAND;
		else if (analysis_bandwidth <= 18)
			st->detected_bandwidth = OPUS_BANDWIDTH_SUPERWIDEBAND;
		else
			st->detected_bandwidth = OPUS_BANDWIDTH_FULLBAND;
	}
#else
	st->voice_ratio = -1;
#endif


	st->bitrate_bps = user_bitrate_to_bitrate(st, frame_size, max_data_bytes);

	int frame_rate = st->Fs / frame_size;
	if (!st->use_vbr)
	{
		int cbrBytes;
		/* Multiply by 12 to make sure the division is exact. */
		int frame_rate12 = 12 * st->Fs / frame_size;
		/* We need to make sure that "int" values always fit in 16 bits. */
		cbrBytes = IMIN((12 * st->bitrate_bps / 8 + frame_rate12 / 2) / frame_rate12, max_data_bytes);
		st->bitrate_bps = cbrBytes * (opus_int32)frame_rate12 * 8 / 12;
		/* Make sure we provide at least one byte to avoid failing. */
		max_data_bytes = IMAX(1, cbrBytes);
	}

	max_rate = frame_rate * max_data_bytes * 8;
#ifdef ENABLE_SILK
	/* Equivalent 20-ms rate for mode/channel/bandwidth decisions */
	equiv_rate = compute_equiv_rate(st->bitrate_bps, st->channels, st->Fs / frame_size,
		st->use_vbr, 0, st->silk_mode.complexity, st->silk_mode.packetLossPercentage);
#else
#define   QL_OPUS_ENC_PACKETLOSSPERCENTAGE (0)
	int complexity = 0;
	ql_opus_param_ctrl(st, (void**)&complexity, E_QL_OPUS_ENC_PARAM_GET_FINAL_COMPLEXITY);
	equiv_rate = compute_equiv_rate(st->bitrate_bps, st->channels, st->Fs / frame_size,
		st->use_vbr, st->mode, complexity, QL_OPUS_ENC_PACKETLOSSPERCENTAGE);
#endif
	if (st->signal_type == OPUS_SIGNAL_VOICE)
		voice_est = 127;
	else if (st->signal_type == OPUS_SIGNAL_MUSIC)
		voice_est = 0;
	else if (st->voice_ratio >= 0)
	{
		voice_est = st->voice_ratio * 327 >> 8;
		/* For AUDIO, never be more than 90% confident of having speech */
		if (st->application == OPUS_APPLICATION_AUDIO)
			voice_est = IMIN(voice_est, 115);
	}
	else if (st->application == OPUS_APPLICATION_VOIP)
		voice_est = 115;
	else
		voice_est = 48;



	st->mode = MODE_CELT_ONLY;
	if (st->mode == MODE_CELT_ONLY || st->first)
	{
		const opus_int32 *voice_bandwidth_thresholds, *music_bandwidth_thresholds;
		opus_int32 bandwidth_thresholds[8];
		int bandwidth = OPUS_BANDWIDTH_FULLBAND;

		if (st->channels == 2 && st->force_channels != 1)
		{
			voice_bandwidth_thresholds = stereo_voice_bandwidth_thresholds;
			music_bandwidth_thresholds = stereo_music_bandwidth_thresholds;
		}
		else {
			voice_bandwidth_thresholds = mono_voice_bandwidth_thresholds;
			music_bandwidth_thresholds = mono_music_bandwidth_thresholds;
		}
		/* Interpolate bandwidth thresholds depending on voice estimation */
		for (int i = 0; i<8; i++)
		{
			bandwidth_thresholds[i] = music_bandwidth_thresholds[i]
				+ ((voice_est*voice_est*(voice_bandwidth_thresholds[i] - music_bandwidth_thresholds[i])) >> 14);
		}
		do {
			int threshold, hysteresis;
			threshold = bandwidth_thresholds[2 * (bandwidth - OPUS_BANDWIDTH_MEDIUMBAND)];
			hysteresis = bandwidth_thresholds[2 * (bandwidth - OPUS_BANDWIDTH_MEDIUMBAND) + 1];
			if (!st->first)
			{
				if (st->auto_bandwidth >= bandwidth)
					threshold -= hysteresis;
				else
					threshold += hysteresis;
			}
			if (equiv_rate >= threshold)
				break;
		} while (--bandwidth>OPUS_BANDWIDTH_NARROWBAND);
		st->bandwidth = st->auto_bandwidth = bandwidth;
		/* Prevents any transition to SWB/FB until the SILK layer has fully
		switched to WB mode and turned the variable LP filter off */
		if (!st->first && st->mode != MODE_CELT_ONLY && st->bandwidth > OPUS_BANDWIDTH_WIDEBAND)
			st->bandwidth = OPUS_BANDWIDTH_WIDEBAND;
	}

	if (st->bandwidth>st->max_bandwidth)
		st->bandwidth = st->max_bandwidth;

	if (st->user_bandwidth != OPUS_AUTO)
		st->bandwidth = st->user_bandwidth;

	/* This prevents us from using hybrid at unsafe CBR/max rates */
	if (st->mode != MODE_CELT_ONLY && max_rate < 15000)
	{
		st->bandwidth = IMIN(st->bandwidth, OPUS_BANDWIDTH_WIDEBAND);
	}

	/* Prevents Opus from wasting bits on frequencies that are above
	the Nyquist rate of the input signal */
	if (st->Fs <= 24000 && st->bandwidth > OPUS_BANDWIDTH_SUPERWIDEBAND)
		st->bandwidth = OPUS_BANDWIDTH_SUPERWIDEBAND;
	if (st->Fs <= 16000 && st->bandwidth > OPUS_BANDWIDTH_WIDEBAND)
		st->bandwidth = OPUS_BANDWIDTH_WIDEBAND;
	if (st->Fs <= 12000 && st->bandwidth > OPUS_BANDWIDTH_MEDIUMBAND)
		st->bandwidth = OPUS_BANDWIDTH_MEDIUMBAND;
	if (st->Fs <= 8000 && st->bandwidth > OPUS_BANDWIDTH_NARROWBAND)
		st->bandwidth = OPUS_BANDWIDTH_NARROWBAND;

	/* CELT mode doesn't support mediumband, use wideband instead */
	if (st->mode == MODE_CELT_ONLY && st->bandwidth == OPUS_BANDWIDTH_MEDIUMBAND)
		st->bandwidth = OPUS_BANDWIDTH_WIDEBAND;
	if (st->lfe)
		st->bandwidth = OPUS_BANDWIDTH_NARROWBAND;

	*p_max_data_bytes = max_data_bytes;
	return st->bandwidth;
}
#ifndef DISABLE_FLOAT_API
#if (QL_OPUS_ENC_COMPLEXITY >=7)
int perform_analysis(OpusEncoder *st, AnalysisInfo *p_analysis_info, CELTMode *celt_mode, const opus_val16 *pcm, int frame_size, int bits, 
	const void *analysis_pcm, opus_int32 analysis_size)
{
	int is_silence = 0;
	int c1 = 0, c2 = -2;

	p_analysis_info->valid = 0;
	{
		if (is_digital_silence(pcm, frame_size, st->channels, bits))
		{
			is_silence = 1;
		}
		else {
			run_analysis(&st->analysis, celt_mode, analysis_pcm, analysis_size, frame_size,
				c1, c2, 1, st->Fs,
				bits, downmix_int, p_analysis_info);
		}

		/* Track the peak signal energy */
		if (!is_silence && p_analysis_info->activity_probability > DTX_ACTIVITY_THRESHOLD)
			st->peak_signal_energy = MAX32(MULT16_32_Q15(QCONST16(0.999f, 15), st->peak_signal_energy),
				compute_frame_energy(pcm, frame_size, st->channels, st->arch));
	}

	return is_silence;
}
#endif
#endif
int ql_pre_filter(OpusEncoder *st, const opus_val16 *pcm, opus_val16 *pcm_buf, int total_buffer, int frame_size)
{
	int cutoff_Hz, hp_freq_smth1;
	OPUS_COPY(pcm_buf, &st->delay_buffer[(st->encoder_buffer - total_buffer)*st->channels], total_buffer*st->channels);

	hp_freq_smth1 = silk_LSHIFT(silk_lin2log(VARIABLE_HP_MIN_CUTOFF_HZ), 8);

	st->variable_HP_smth2_Q15 = silk_SMLAWB(st->variable_HP_smth2_Q15, hp_freq_smth1 - st->variable_HP_smth2_Q15, SILK_FIX_CONST(VARIABLE_HP_SMTH_COEF2, 16));

	/* convert from log scale to Hertz */
	cutoff_Hz = silk_log2lin(silk_RSHIFT(st->variable_HP_smth2_Q15, 8));

	if (st->application == OPUS_APPLICATION_VOIP)
	{
		hp_cutoff(pcm, cutoff_Hz, &pcm_buf[total_buffer*st->channels], st->hp_mem, frame_size, st->channels, st->Fs, st->arch);
	}
	else {
		dc_reject(pcm, 3, &pcm_buf[total_buffer*st->channels], st->hp_mem, frame_size, st->channels, st->Fs);
	}

	if (st->channels*(st->encoder_buffer - (frame_size + total_buffer)) > 0)
	{
		OPUS_MOVE(st->delay_buffer, &st->delay_buffer[st->channels*frame_size], st->channels*(st->encoder_buffer - frame_size - total_buffer));
		OPUS_COPY(&st->delay_buffer[st->channels*(st->encoder_buffer - frame_size - total_buffer)],
			&pcm_buf[0],
			(frame_size + total_buffer)*st->channels);
	}
	else {
		OPUS_COPY(st->delay_buffer, &pcm_buf[(frame_size + total_buffer - st->encoder_buffer)*st->channels], st->encoder_buffer*st->channels);
	}
        return 0;
}
int ql_celt_param_ctrl(CELTEncoder * OPUS_RESTRICT s, void **c, E_QL_OPUS_ENC_PARAM_T eParam);

opus_int32 opus_encode_core(OpusEncoder *st, const opus_val16 *pcm, int frame_size,
                unsigned char *data, opus_int32 out_data_bytes, 
                const void *analysis_pcm, opus_int32 analysis_size, int analysis_channels)
{
    CELTEncoder *celt_enc;
    int ret=0;
    ec_enc enc;
    int redundancy = 0;
    int redundancy_bytes = 0; /* Number of bytes to use for redundancy frame */
    VARDECL(opus_val16, pcm_buf);
    int nb_compr_bytes;
    opus_uint32 redundant_rng = 0;
    
    int curr_bandwidth;
    opus_int32 max_data_bytes = IMIN(1276, out_data_bytes); /* Max number of bytes we're allowed to use */
    int total_buffer;
    const CELTMode *celt_mode;
	AnalysisInfo *p_analysis_info = NULL;
    int is_silence = 0;

    ALLOC_STACK;
 
    st->rangeFinal = 0;

	celt_enc = (CELTEncoder*)((char*)st+st->celt_enc_offset);

	ql_celt_param_ctrl(celt_enc, (void*)&celt_mode, E_QL_OPUS_ENC_PARAM_CELT_GET_MODE);
	ql_celt_param_ctrl(celt_enc, (void*)&p_analysis_info, E_QL_OPUS_ENC_PARAM_CELT_GET_ANALYSIS);
#ifndef DISABLE_FLOAT_API
#if (QL_OPUS_ENC_COMPLEXITY >= 7)
	is_silence = perform_analysis(st, p_analysis_info, (void*)celt_mode, pcm, frame_size, st->lsb_depth, analysis_pcm, analysis_size);
#endif
#endif
    /* Automatic (rate-dependent) bandwidth selection */
	curr_bandwidth = adapt_parameters(st, is_silence, p_analysis_info,  frame_size,  out_data_bytes, &max_data_bytes);

    if (redundancy)
    {
		int frame_rate = st->Fs / frame_size;
       redundancy_bytes = compute_redundancy_bytes(max_data_bytes, st->bitrate_bps, frame_rate, st->stream_channels);
       if (redundancy_bytes == 0)
          redundancy = 0;
    }

  //   printf("%d %d %d %d\n", st->bitrate_bps, st->stream_channels, st->mode, curr_bandwidth); 

    data += 1;

    ec_enc_init(&enc, data, max_data_bytes-1);

	total_buffer = st->delay_compensation;
	ALLOC(pcm_buf, (total_buffer+frame_size)*st->channels, opus_val16);

	ql_pre_filter(st, pcm, pcm_buf, total_buffer, frame_size);
  
    /* CELT processing */
     redundancy = 0;
	 st->silk_bw_switch = 0;
     redundancy_bytes = 0;
     nb_compr_bytes = (max_data_bytes-1)-redundancy_bytes;
     ec_enc_shrink(&enc, nb_compr_bytes);
  
	if (ec_tell(&enc) <= 8 * nb_compr_bytes)
	{
		ret = celt_encode_with_ec(celt_enc, pcm_buf, frame_size, NULL, nb_compr_bytes, &enc);
		if (ret < 0)
		{
			RESTORE_STACK;
			return OPUS_INTERNAL_ERROR;
		}
	}

    /* Signalling the mode in the first byte */
    data--;
    data[0] = gen_toc(st->mode, st->Fs/frame_size, curr_bandwidth, st->stream_channels);

    st->rangeFinal = enc.rng ^ redundant_rng;

    st->prev_channels = st->stream_channels;
    st->prev_framesize = frame_size;

    st->first = 0;

    /* Count ToC and redundancy */
    ret += 1+redundancy_bytes;
	if (opus_packet_pad(data, ret, max_data_bytes) != OPUS_OK)
	{
		RESTORE_STACK;
		return OPUS_INTERNAL_ERROR;
	}
	ret = max_data_bytes;
    RESTORE_STACK;
    return ret;
    
}
#ifndef QL_OPUS_FLOAT_SAMPLES
opus_int32 opus_encode(OpusEncoder *st, const opus_int16 *pcm, int analysis_frame_size,  unsigned char *data, opus_int32 max_data_bytes)
#else
opus_int32 opus_encode(OpusEncoder *st, const float *pcm, int analysis_frame_size, unsigned char *data, opus_int32 max_data_bytes)
#endif
{
   int ret = 0;
   int frame_size;
   float* in_samples;  // scratch mem
   ALLOC_STACK;

   frame_size = frame_size_select(analysis_frame_size, st->variable_duration, st->Fs);
   if (frame_size <= 0)
   {
      RESTORE_STACK;
      return OPUS_BAD_ARG;
   }
#ifndef QL_OPUS_FLOAT_SAMPLES
   ALLOC(in_samples, frame_size*st->channels, float);
#ifndef CMSIS_OPT
   for (int i=0;i<frame_size*st->channels;i++)
      in_samples[i] = (1.0f/32768)*pcm[i];
#else
   arm_q15_to_float((q15_t*)pcm, in_samples, frame_size);
#endif
#else
	 in_samples = pcm;
#endif
   ret = opus_encode_core(st, in_samples, frame_size, data, max_data_bytes, 
                            pcm, analysis_frame_size, st->channels);
   if (ret < 0)
   {
	   return -1;
   }
   RESTORE_STACK;
   return ret;
}


int ql_opus_param_ctrl(OpusEncoder *opus, void **c, E_QL_OPUS_ENC_PARAM_T eParam)
{

	CELTEncoder * OPUS_RESTRICT s = (CELTEncoder*)((char*)opus + opus->celt_enc_offset);
	int ret = 0;
	switch (eParam)
	{
	case E_QL_OPUS_ENC_PARAM_SET_BITRATE:
	{
		opus_uint32 p = (opus_uint32)c;
		opus->user_bitrate_bps = p;
	}
	break;
        
        case E_QL_OPUS_ENC_PARAM_GET_FINAL_RANGE:
          {uint32_t *p = (uint32_t *)c;
          *p = opus->rangeFinal;
          }
          break;
		case E_QL_OPUS_ENC_PARAM_SET_FINAL_COMPLEXITY:
		{
			
			ql_celt_param_ctrl(s, c, E_QL_OPUS_ENC_PARAM_SET_FINAL_COMPLEXITY);
		}
			break;
		case E_QL_OPUS_ENC_PARAM_GET_FINAL_COMPLEXITY:
		{

			ql_celt_param_ctrl(s, c, E_QL_OPUS_ENC_PARAM_GET_FINAL_COMPLEXITY);
		}
		break;
	default:
		ret = -1;
		break;
	}
	return ret;
}


void opus_encoder_destroy(OpusEncoder *st)
{
    opus_free(st);
}
