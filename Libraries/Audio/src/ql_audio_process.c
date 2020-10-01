/*==========================================================
 * Copyright 2020 QuickLogic Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *==========================================================*/

/* Free RTOS Task specific to Audio processing.                               */
/* File Name : ql_audio_process.c                                             */
/* This file contains audio processing high-level function, performs:         */
/*      - agc                                                                 */
/*      - noise reduction with or without beamforming                         */
/*      - send buffer to opus encoder and also to I2S renderer                */
/******************************************************************************/

#include "Fw_global_config.h"
#include "RtosTask.h"
#include "eoss3_hal_i2s.h"
#include "ql_audioTask.h"
#include "dma_buffer.h"
#include "ql_vsrc_api.h"
#include "output_i2s.h"
#include "eoss3_hal_leds.h"
#ifdef OPUS_ENCODER
#include "Opus_app.h"
#endif

#include "ql_audio_preproc.h"

volatile int16_t Out_buf[NO_OF_OP_BUFFERS][INPUT_FRAME_SIZE] __attribute__ ((aligned (16)));
int16_t *pI2SBuf;

#ifdef ENABLE_I2S_CWWV
static inline int get_pretext_start_offset(tAudioTaskState *pAudioTaskState );
#endif

static e_ql_vr_status ql_audio_process_impl(tAudioTaskState *pAudioTaskState, t_ql_audio_stream *p_mic, t_ql_audio_stream *p_ref);

e_ql_vr_status ql_audio_process(tAudioTaskState *pAudioTaskState, t_ql_audio_stream *p_mic, t_ql_audio_stream *p_ref)
{
  return ql_audio_process_impl(pAudioTaskState, p_mic, p_ref);
}

#ifdef ENABLE_I2S_SLAVE_ASSP_TX
static uint32_t i2s_tx_buffer( void* bufptr)
{
    uint32_t ret_val=HAL_I2S_SUCCESS;
    i2sBufInfo_t i2sBufInfo;
    i2sBufInfo.pBuf = (uint16_t*)bufptr;
    i2sBufInfo.size = sizeof(Out_buf[0]);
    i2sBufInfo.index = 0;
    i2sBufInfo.numBuf = NO_OF_OP_BUFFERS;
    output_i2s_TxBuf(&i2sBufInfo);
    return ret_val;
}
#endif

#ifdef ENABLE_I2S_CWWV
/* Get index from current buf - wakeword len - pretext */
#define GET_PREV_INDEX_DSPC_OUTBUF(cur_pos,offset)
    ((cur_pos-offset)>=0)?(cur_pos-offset):((NO_OF_OP_BUFFERS)+(cur_pos-offset))

static inline int get_pretext_start_offset(tAudioTaskState *pAudioTaskState )
{
    uint16_t index = pAudioTaskState->i2s_streaming_index;
    /* i2s index is -1 as , it will be pointing to next Out_buf */
    return GET_PREV_INDEX_DSPC_OUTBUF((index-1),
        ((PRETEXT_LEN_SAMPLES+pAudioTaskState->ww_len_samples)/AWE_FRAME_SIZE));
}
#endif

#ifdef CWWV_PRETEXT_VERIFY
void verify_cwwv_pretext_data(tAudioTaskState *pAudioTaskState)
{
    if(pAudioTaskState->debug_index == 0)
    {
        int pre_text = get_pretext_start_offset(pAudioTaskState);
        int kpt_start_index = GET_PREV_INDEX_DSPC_OUTBUF((pAudioTaskState->i2s_streaming_index - 1), ((pAudioTaskState->ww_len_samples)/AWE_FRAME_SIZE));

        memset((void*)Out_buf[pre_text], 0xA5, sizeof(Out_buf[0]));
        memset((void*)Out_buf[kpt_start_index-1], 0xA5, sizeof(Out_buf[0]));
        pAudioTaskState->debug_index = 1;
    }
}
#define VERIFY_CWWV_PRETEXT verify_cwwv_pretext_data
#else
#define VERIFY_CWWV_PRETEXT
#endif

static e_ql_vr_status ql_audio_process_impl(tAudioTaskState *pAudioTaskState, t_ql_audio_stream *p_mic, t_ql_audio_stream *p_ref)
{
    VoiceSysMode  sys_mode = pAudioTaskState->pVoiceSystemInfo->sys_mode;
    VoiceSysState sys_state = pAudioTaskState->pVoiceSystemInfo->sys_state;
    t_ql_audio_stream o_output_stream;
    int16_t *outBuf;
    e_ql_vr_status res = e_ql_vr_status_ok;

#if (defined (ENABLE_I2S_SLAVE_ASSP_TX) || defined(QD_ALGO_ON_FFE))
    outBuf = (int16_t *)Out_buf[pAudioTaskState->i2s_buf_write_idx];
#else
#if defined(OPUS_ENCODER)
    outBuf = (int16_t *)Out_buf[pAudioTaskState->opus_tick_idx];
#endif
#endif
    o_output_stream.p_buffer = (uint16_t *)outBuf;
    o_output_stream.n_channels = OUTPUT_CHANNEL_COUNT;
    o_output_stream.n_frame_sz = INPUT_FRAME_SIZE * OUTPUT_CHANNEL_COUNT;

    ql_audio_preproc_process(p_mic, p_ref, &o_output_stream);

#if defined(OPUS_ENCODER) && defined(OPUS_ON_NS)    //send data to OPUS if required
    pAudioTaskState->opus_tick_idx++;
    if (!(pAudioTaskState->opus_tick_idx % TICKS_IN_OPUS_FRAME))
    {
        ql_send_msg_to_opus_enc(QlTaskIsr, eCMD_AUDIO_DATA_READY, ( uint32_t)(pAudioTaskState->opus_tick_idx - TICKS_IN_OPUS_FRAME), 0);
    }

    if (NO_OF_OP_BUFFERS == pAudioTaskState->opus_tick_idx)
        pAudioTaskState->opus_tick_idx = 0;
#endif

#if (defined (ENABLE_I2S_SLAVE_ASSP_TX) || defined(QD_ALGO_ON_FFE))
    outBuf = (int16_t *)Out_buf[pAudioTaskState->i2s_buf_write_idx];
#endif

    pAudioTaskState->i2s_buf_write_idx++;

    if (NO_OF_OP_BUFFERS == pAudioTaskState->i2s_buf_write_idx)
    pAudioTaskState->i2s_buf_write_idx = 0;

    switch(sys_state)
    {
    case WAIT_FOR_AUDIO:
#ifdef ENABLE_I2S_SLAVE_ASSP_TX

#ifdef ENABLE_I2S_CWWV
        if(KP_STREAM_OK == pAudioTaskState->KP_stream)
        {
            VERIFY_CWWV_PRETEXT(pAudioTaskState);
            // start from current o/p buf index - Alexa len - Pretext len
            outBuf = (int16_t *)Out_buf[get_pretext_start_offset(pAudioTaskState)];
            pAudioTaskState->i2s_streaming_index++;

            if (NO_OF_OP_BUFFERS == pAudioTaskState->i2s_streaming_index)
            pAudioTaskState->i2s_streaming_index = 0;
        }
#endif
        i2s_tx_buffer(outBuf);
#endif

        break;

#ifdef KPDCMD
    case WAIT_FOR_CMD:
#endif
    case WAIT_FOR_KPD:
        pAudioTaskState->debug_index = 0;
        pAudioTaskState->KP_stream = KP_STREAM_NOT_OK;

#if (defined(HOST_VOICE) && defined(AW_TUNING)) || defined (AEC_ENABLED)
  if(e_output_stream_pdm == pAudioTaskState->op_opt_to_stream)
      pI2SBuf = (int16_t *)p_mic->p_buffer;
  else if(e_output_stream_refI2S == pAudioTaskState->op_opt_to_stream)
      pI2SBuf = (int16_t *)p_ref->p_buffer;
  else if(e_output_stream_processed == pAudioTaskState->op_opt_to_stream)
      pI2SBuf = outBuf;
  else
      pI2SBuf = p_mic->p_buffer;

  i2s_tx_buffer(pI2SBuf);
#endif

#ifdef QL_SENSORY
        if (pAudioTaskState->s_AudioRunning)
        {
            pAudioTaskState->vr_tick_idx++;
            if (pAudioTaskState->vr_tick_idx == VR_FRAME_CNT)
            {
                outBuf -= (AWE_FRAME_SIZE * VR_FRAME_CNT);
                res = ql_vr_process(outBuf, sys_mode);
                if (res == e_ql_vr_status_detection_ok)
                {
                    pAudioTaskState->KP_stream = KP_STREAM_OK;
                    pAudioTaskState->i2s_streaming_index = pAudioTaskState->i2s_buf_write_idx;
                    pAudioTaskState->kpd_score++;
                    LedOrangeOn_AutoOff(); // TODO : Move out hal functions from applications.
                }
                pAudioTaskState->vr_tick_idx = 0;
          }
        }
        break;
#endif

    default:
        break;

    } /* end of switch case */

    return res;
}
