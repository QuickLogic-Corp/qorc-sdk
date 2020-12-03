#include "ql_opus.h"
#include "opus.h"
#include "ql_mem.h"

#ifdef CMSIS_OPT    
#include <arm_math.h>    
#endif    

#define QL_OPUS_ENC_SAMPLING_RATE (16000)
#define QL_OPUS_ENC_CHANNELS      (1)
#define QL_OPUS_ENC_FRAME_SIZE_20MS    (320)
#define QL_OPUS_ENC_FRAME_SIZE_10MS    (160)
#define QL_OPUS_ENC_BITRATE (16000)

ql_opus_encoder_t o_ql_opus_encoder;
char* library_version = { "QL Opus Library Version: 1.2.1.0" };
// strings libopus.a | grep Version | cut -d " " -f 1-5

int ql_opus_param_ctrl(OpusEncoder *opus, void **c, E_QL_OPUS_ENC_PARAM_T eParam);

#define OSC_CTRL0 (0x40005480)
#define MAX_VALUE (80 *1024000)
int validate_platform(void)
{
  
  int *ptr;
  int ret = 0;
  
  ptr = (int *)OSC_CTRL0;
  
  if(!(*ptr++ & 1))
    ret = -1;
  else if (((*ptr & (0xFFF)) + 3) * 32768 > MAX_VALUE)
    ret = -1;
  
  return ret;
}


ql_opus_encoder_t *ql_opus_init(e_ql_opus_encoder_options_t option)
{
  OpusEncoder *enc=NULL;
  
  uint32_t fs = QL_OPUS_ENC_SAMPLING_RATE;
  uint32_t n_channels = QL_OPUS_ENC_CHANNELS;
  uint32_t framesamples = QL_OPUS_ENC_FRAME_SIZE_10MS;
  switch (option)
  {
  case E_QL_OPUS_ENCODER_OPTION1:
	  //o_ql_opus_encoder.n_bitrate = 16000;
	  o_ql_opus_encoder.n_bitrate = 64000;
	  break;
  case E_QL_OPUS_ENCODER_OPTION2:
	  o_ql_opus_encoder.n_bitrate = 32000;
	  break;
#if ((defined QL_FFT_480) | (defined USE_KISS_FFT))
  case E_QL_OPUS_ENCODER_OPTION3:
	  //o_ql_opus_encoder.n_bitrate = 16000;
	  o_ql_opus_encoder.n_bitrate = 64000;
	  framesamples = QL_OPUS_ENC_FRAME_SIZE_20MS;
	  break;
  case E_QL_OPUS_ENCODER_OPTION4:
	  o_ql_opus_encoder.n_bitrate = 32000;
	  framesamples = QL_OPUS_ENC_FRAME_SIZE_20MS;
	  break;
#else
  case E_QL_OPUS_ENCODER_OPTION3:
  case E_QL_OPUS_ENCODER_OPTION4:
     o_ql_opus_encoder.e_status = E_QL_OPUS_ENCODER_STATUS_RRR_ENC_CREATE_FAILED;
      return &o_ql_opus_encoder;

    break;
#endif
  }
  
  o_ql_opus_encoder.n_samplingrate = fs;
  o_ql_opus_encoder.n_samples_per_frame = framesamples;
 // o_ql_opus_encoder.n_max_enc_bytes = 1500; // todo
  o_ql_opus_encoder.n_header_size = 8; // 
  int err = 0;
  
  //enc = opus_encoder_create(fs, n_channels, OPUS_APPLICATION_VOIP, &err);
  enc = opus_encoder_create(fs, n_channels, OPUS_APPLICATION_AUDIO, &err);
 
   if (err != OPUS_OK)
   {
     o_ql_opus_encoder.e_status = E_QL_OPUS_ENCODER_STATUS_RRR_ENC_CREATE_FAILED;
      return &o_ql_opus_encoder;
   }
#if 1
#ifndef _WIN32
   if(validate_platform() < 0)
   {
     o_ql_opus_encoder.h_opus_encoder = NULL;
     o_ql_opus_encoder.e_status = E_QL_OPUS_ENCODER_STATUS_RRR_ENC_CREATE_FAILED;
     return &o_ql_opus_encoder;
   }
#endif
#endif
   ql_opus_param_ctrl(enc, (void*)o_ql_opus_encoder.n_bitrate, E_QL_OPUS_ENC_PARAM_SET_BITRATE);
   
 //  opus_encoder_ctl(enc, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_WIDEBAND));
//   opus_encoder_ctl(enc, OPUS_SET_VBR(0));
//   opus_encoder_ctl(enc, OPUS_SET_VBR_CONSTRAINT(0));
//   opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(8));
 //  opus_encoder_ctl(enc, OPUS_SET_INBAND_FEC(0));
 //  opus_encoder_ctl(enc, OPUS_SET_FORCE_CHANNELS(OPUS_AUTO));
  // opus_encoder_ctl(enc, OPUS_SET_DTX(0));
 //  opus_encoder_ctl(enc, OPUS_SET_PACKET_LOSS_PERC(0));

//   opus_encoder_ctl(enc, OPUS_GET_LOOKAHEAD(&skip));
//   opus_encoder_ctl(enc, OPUS_SET_LSB_DEPTH(16));
//   opus_encoder_ctl(enc, OPUS_SET_EXPERT_FRAME_DURATION(OPUS_FRAMESIZE_5_MS));
   //opus_encoder_ctl(enc, OPUS_SET_EXPERT_FRAME_DURATION(OPUS_FRAMESIZE_20_MS));    //MQD
   
   
   /* int math should be fine here for the supported values */
   o_ql_opus_encoder.n_encoded_framesize = o_ql_opus_encoder.n_header_size + ((o_ql_opus_encoder.n_bitrate / o_ql_opus_encoder.n_samplingrate)*framesamples/8);
   

  o_ql_opus_encoder.h_opus_encoder = enc;
  o_ql_opus_encoder.e_status = E_QL_OPUS_ENCODER_STATUS_OK;
  
  return &o_ql_opus_encoder;
}


static void int_to_char(int i, unsigned char ch[4])
{
    ch[0] = i>>24;
    ch[1] = (i>>16)&0xFF;
    ch[2] = (i>>8)&0xFF;
    ch[3] = i&0xFF;
}


e_ql_opus_encoder_status_t ql_opus_encode_configure(ql_opus_encoder_t *p_ql_opus_encoder, ql_opus_parameters_t *p_ql_opus_parameters, e_ql_opus_parameter_index_t param_index)
{
  e_ql_opus_encoder_status_t ret = E_QL_OPUS_ENCODER_STATUS_OK;
  switch(param_index)
  {
    
  case e_ql_opus_param_set_complexity  :
    //   ql_opus_param_ctrl((OpusEncoder*)p_ql_opus_encoder->h_opus_encoder, OPUS_SET_COMPLEXITY(p_ql_opus_parameters->complexity));
	  ql_opus_param_ctrl((OpusEncoder*)p_ql_opus_encoder->h_opus_encoder, (void**)p_ql_opus_parameters->complexity, E_QL_OPUS_ENC_PARAM_SET_FINAL_COMPLEXITY);
    break;

  case e_ql_opus_param_set_bitrate    :

  default:
    
      ret = E_QL_OPUS_ENCODER_STATUS_RRR;
      break;
    
  }
  
  return ret;
}
#ifndef QL_OPUS_FLOAT_SAMPLES
e_ql_opus_encoder_status_t ql_opus_encode(ql_opus_encoder_t *p_ql_opus_encoder, int16_t *p_samples, uint32_t n_count, uint32_t stride, uint8_t *p_encoded_buffer, int32_t *n_encoded_bytes, int32_t *samples_consumed)
#else
e_ql_opus_encoder_status_t ql_opus_encode(ql_opus_encoder_t *p_ql_opus_encoder, float *p_samples, uint32_t n_count, uint32_t stride, uint8_t *p_encoded_buffer, int32_t *n_encoded_bytes, int32_t *samples_consumed)
#endif
{
	int32_t nb_encoded = 0;
	int ret = 0;
	opus_uint32 enc_final_range;
	int hdr_size = o_ql_opus_encoder.n_header_size;
    if(p_ql_opus_encoder->e_status != E_QL_OPUS_ENCODER_STATUS_OK)
    {
      return  p_ql_opus_encoder->e_status;
    }
	MALLOC2_RESET();

	ret = opus_encode((OpusEncoder*)p_ql_opus_encoder->h_opus_encoder,
		p_samples,
		n_count, // p_ql_opus_encoder->n_framesize,
		p_encoded_buffer + hdr_size,
		1800 //  p_ql_opus_encoder->n_max_enc_bytes
	);
	if (ret > 0) {
		*n_encoded_bytes = ret;
		if (hdr_size >= 4)
		{
			int_to_char(*n_encoded_bytes, p_encoded_buffer);
			*n_encoded_bytes += hdr_size; // hdr size 
		}
		ql_opus_param_ctrl((OpusEncoder*)p_ql_opus_encoder->h_opus_encoder, (void**)&enc_final_range, E_QL_OPUS_ENC_PARAM_GET_FINAL_RANGE);

		// opus_encoder_ctl((OpusEncoder*)p_ql_opus_encoder->h_opus_encoder, &enc_final_range);
		//enc_final_range = 0x1234567;
		if (hdr_size >= 8)
		{
			int_to_char(enc_final_range, p_encoded_buffer + 4);
		}
		nb_encoded = opus_packet_get_samples_per_frame(p_encoded_buffer + hdr_size, p_ql_opus_encoder->n_samplingrate)*opus_packet_get_nb_frames(p_encoded_buffer, *n_encoded_bytes);
		*samples_consumed = nb_encoded;
	}
	else
	{
		return E_QL_OPUS_ENCODER_STATUS_RRR;
	}
  return E_QL_OPUS_ENCODER_STATUS_OK;
}
