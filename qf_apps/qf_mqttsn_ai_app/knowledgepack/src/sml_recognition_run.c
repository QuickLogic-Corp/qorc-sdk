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

//SensiML Includes
#include "kb.h"
#include "sml_output.h"
#include "sml_recognition_run.h"
#include "dcl_commands.h"

//FILL_USE_TEST_DATA

#ifdef SML_USE_TEST_DATA
#include "testdata.h"
int td_index = 0;
#endif //SML_USE_TEST_DATA


int sml_recognition_run_batch(signed short *data_batch, int batch_sz, uint8_t num_sensors, uint32_t sensor_id)
{
	int ret;

	int batch_index = 0;
	signed short* data;
	for(batch_index=0; batch_index < batch_sz; batch_index++)
	{
	#ifdef SML_USE_TEST_DATA
	ret = kb_run_model((SENSOR_DATA_T*)&testdata[td_index++], TD_NUMCOLS, 0);
	if(td_index >= TD_NUMROWS)
	{
		td_index = 0;
	}
	if(ret >= 0)
	{
		//kb_print_model_result(0, ret);
		sml_output_results(0, ret);
		kb_reset_model(0);
	}
	#else
	data = &data_batch[batch_index*num_sensors];
	switch(sensor_id)
	{
                case SENSOR_ENG_VALUE_ACCEL_GYRO:
			ret = kb_run_model((SENSOR_DATA_T *)data, num_sensors, KB_MODEL_IMU_MODEL_INDEX);
		if (ret >= 0){
			//kb_print_model_result(KB_MODEL_IMU_MODEL_INDEX, ret);
			sml_output_results(KB_MODEL_IMU_MODEL_INDEX, ret);
			kb_reset_model(0);
		};
			break;
		case SENSOR_AUDIO:
					ret = kb_run_model((SENSOR_DATA_T *)data, num_sensors, KB_MODEL_AUDIO_MODEL_INDEX);
		if (ret >= 0){
			//kb_print_model_result(KB_MODEL_AUDIO_MODEL_INDEX, ret);
			sml_output_results(KB_MODEL_AUDIO_MODEL_INDEX, ret);
			kb_reset_model(1);
		};
			break;
		case SENSOR_ADC_LTC_1859_MAYHEW:
			//FILL_RUN_MODEL_MAYHEW_LTC1859
			break;
		default:
			break;
	}
	#endif //SML_USE_TEST_DATA
	}
	return ret;
}

int sml_recognition_run_single(signed short *data, uint32_t sensor_id)
{
	int ret;
	uint8_t num_sensors = 0;
	#ifdef SML_USE_TEST_DATA
	ret = kb_run_model((SENSOR_DATA_T*)&testdata[td_index++], TD_NUMCOLS, 0);
	if(td_index >= TD_NUMROWS)
	{
		td_index = 0;
	}
	if(ret >= 0)
	{
		//kb_print_model_result(0, ret);
		sml_output_results(0, ret);
		kb_reset_model(0);
	}
	#else
	switch(sensor_id)
	{
                case SENSOR_ENG_VALUE_ACCEL_GYRO:
			ret = kb_run_model((SENSOR_DATA_T *)data, num_sensors, KB_MODEL_IMU_MODEL_INDEX);
		if (ret >= 0){
			//kb_print_model_result(KB_MODEL_IMU_MODEL_INDEX, ret);
			sml_output_results(KB_MODEL_IMU_MODEL_INDEX, ret);
			kb_reset_model(0);
		};
			break;
		case SENSOR_AUDIO:
			break;
		case SENSOR_ADC_LTC_1859_MAYHEW:
			break;
		default:
			break;
	}
	#endif //SML_USE_TEST_DATA
	return ret;
}
