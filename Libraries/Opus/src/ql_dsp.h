
#ifndef QL_DSP_H
#define QL_DSP_H

#define ENABLE_FIXEDPOINT

#ifdef ENABLE_FIXEDPOINT
#define USE_16BIT_TABLES
#define USE_QL_MDCT_TWIDDLE

/* not supported now as the win table is being used in another fn also - stake of about 320B*/
//#define USE_QL_WIN   
#endif /* ENABLE_FIXEDPOINT */

#ifndef ENABLE_FIXEDPOINT

typedef  float ql_sample_in_t;
typedef  float rota_coeff_t;
#else
typedef  int ql_sample_in_t;

#ifdef USE_16BIT_TABLES
typedef  short rota_coeff_t;
#ifdef WIN32
typedef  __int64 int_acc_t;
#else
typedef  long long int_acc_t;
#endif

#define QL_OPUS_FIXEDPOINT_BITS (20)
#define QL_OPUS_ROTA_BITS (15)
#else  /* ! USE_16BIT_TABLES */
typedef  int rota_coeff_t;
#ifdef WIN32
typedef  __int64 int_acc_t;
#else
l
typedef  long long int_acc_t;
#endif
#define QL_OPUS_FIXEDPOINT_BITS (20)
#define QL_OPUS_ROTA_BITS (20)
#endif
#endif



#ifdef USE_QL_MDCT_TWIDDLE
#define QL_MDCT_TWIDDLE_BITS (15)

   typedef short ql_twiddle_t;
#else
   typedef float ql_twiddle_t;
#endif

#ifdef USE_QL_WIN
#define QL_WIN_BITS    (15)

   typedef short ql_window_t;
#else
   typedef float ql_window_t;
#endif

typedef float ql_sample_mdct_in_t;

typedef   struct {
	float re;
	float im;
} ql_complex_t;

typedef   struct {
    ql_sample_in_t re;
    ql_sample_in_t im;
} ql_complex_fix_t;

#ifdef __cplusplus
extern "C" {
#endif

void ql_fft(int length, ql_sample_in_t *pInput, int *pScalefactor);
  
#ifdef __cplusplus
}
#endif

#endif /* QL_DSP_H */
