#ifndef QL_OPUS__H
#define QL_OPUS__H


#include <stdint.h> 
/*
typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
*/
#ifndef NULL
#define NULL ((void*)(0))
#endif

typedef enum
{
	E_QL_OPUS_ENC_PARAM_CELT_GET_MODE,
	E_QL_OPUS_ENC_PARAM_CELT_GET_ANALYSIS,
	E_QL_OPUS_ENC_PARAM_GET_FINAL_RANGE, 
	E_QL_OPUS_ENC_PARAM_SET_FINAL_COMPLEXITY,
	E_QL_OPUS_ENC_PARAM_GET_FINAL_COMPLEXITY,
	E_QL_OPUS_ENC_PARAM_SET_BITRATE,
	E_QL_OPUS_ENC_PARAM_SET_FRAMESIZE,
	E_QL_OPUS_ENC_PARAM_SET_BANDWIDTH
}E_QL_OPUS_ENC_PARAM_T;

typedef enum 
{
  E_QL_OPUS_ENCODER_OPTION1 = 1,
  E_QL_OPUS_ENCODER_OPTION2 = 2,
  E_QL_OPUS_ENCODER_OPTION3 = 3,
  E_QL_OPUS_ENCODER_OPTION4 = 4
}e_ql_opus_encoder_options_t;

typedef enum
{
  E_QL_OPUS_ENCODER_STATUS_OK = 0,
  E_QL_OPUS_ENCODER_STATUS_RRR = -1,
  E_QL_OPUS_ENCODER_STATUS_RRR_ENC_CREATE_FAILED = -2,
  E_QL_OPUS_ENCODER_STATUS_INVALID_CONFIG_INIT_FAILED = -3
}e_ql_opus_encoder_status_t;
typedef void* h_opus_encoder_t;

typedef struct {
  e_ql_opus_encoder_options_t e_encoder_option;
  h_opus_encoder_t h_opus_encoder;
  e_ql_opus_encoder_status_t e_status;
  uint16_t n_bitrate;
  uint16_t n_samplingrate;
  uint16_t n_encoded_framesize;
  uint16_t n_samples_per_frame; 
  uint16_t n_header_size;
}ql_opus_encoder_t;


typedef enum{
  e_ql_opus_param_set_bitrate    = 1,
  e_ql_opus_param_set_complexity = 2,
  e_ql_opus_encoder_get_framesize = 4
}e_ql_opus_parameter_index_t;

typedef struct {
  uint16_t complexity   ;
  uint16_t bitrate       ;
  uint16_t bandwidth   ;
  
} ql_opus_parameters_t;

ql_opus_encoder_t *ql_opus_init(e_ql_opus_encoder_options_t option);
e_ql_opus_encoder_status_t ql_opus_encode(ql_opus_encoder_t *p_ql_opus_encoder, 
#ifndef QL_OPUS_FLOAT_SAMPLES
	int16_t *p_samples,
#else
	float *p_samples,
#endif
	uint32_t n_count, 
	uint32_t stride, uint8_t *p_encoded_buffer, 
	int32_t *n_encoded_bytes, 
	int32_t *samples_consumed);
e_ql_opus_encoder_status_t ql_opus_encode_configure(ql_opus_encoder_t *p_ql_opus_encoder, ql_opus_parameters_t *p_ql_opus_parameters, e_ql_opus_parameter_index_t param_index);

void ql_opus_set_mem(char*p1, char *p2, int size1, int size2);


#endif /* QL_OPUS__H */
