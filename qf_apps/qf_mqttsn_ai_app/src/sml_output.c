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

#include "Fw_global_config.h"
#include "sml_output.h"
#include "kb.h"
#include "ble_pme_defs.h"
#include "iop_messages.h"
#include "Recognition.h"
#include "dbg_uart.h"
#include "ql_time.h"

#define SERIAL_OUT_CHARS_MAX 512

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

static ble_pme_result_w_fv_t recent_fv_result;

static char serial_out_buf[SERIAL_OUT_CHARS_MAX];

void SendLastRecognition()
{
    send_recognition_results(&recent_fv_result);
}

static void sml_output_ble(uint16_t model, uint16_t classification)
{
    recognition_state_t current_state = GetRecognitionCurrentState();
    
    switch(current_state){
    default:
        break;
    case RECOG_STATE_RUN:
    case RECOG_STATE_RUN_W_FV:
        SendLastRecognition();
        break;
    }
}

static void sml_output_led(uint16_t model, uint16_t classification)
{
    //Unused for right now.
}

static void sml_output_serial(uint16_t model, uint16_t classification)
{
    if( !(DBG_flags & DBG_FLAG_recog_result) ){
        return;
    }
    snprintf(serial_out_buf, sizeof(serial_out_buf)-1,
             "{\"ModelNumber\":%d,\"Classification\":%d,\"FeatureLength\":%d,\"FeatureVector\":[",model,classification, recent_fv_result.fv_len);
    dbg_str(serial_out_buf);
    for(int j=0; j < recent_fv_result.fv_len; j++)
    {
        dbg_int(recent_fv_result.feature_vector[j]);
        if(j < recent_fv_result.fv_len -1)
        {
            dbg_ch(',');
        }
    }
    dbg_ch(']');
    dbg_ch('}');
    dbg_ch('\n');
	//fflush(stdout);
}

static intptr_t last_output;

uint32_t sml_output_results(uint16_t model, uint16_t classification)
{
    recent_fv_result.context = model;
    recent_fv_result.classification = classification;
    
    kb_get_feature_vector(model, recent_fv_result.feature_vector, &recent_fv_result.fv_len);
    
    /* LIMIT output to 100hz */
    
    if( last_output == 0 ){
        last_output = ql_lw_timer_start();
    }
    
    if( ql_lw_timer_is_expired( last_output, 10 ) ){
        last_output = ql_lw_timer_start();
    	sml_output_ble(model, classification);
    	sml_output_serial(model, classification);
    }
    return 0;
}

uint32_t sml_output_init(void * p_module)
{
	//unused for now
    return 0;
}