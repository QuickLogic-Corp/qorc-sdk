/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/


#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/examples/person_detection/model_settings.h"
#include "tensorflow/lite/micro/examples/person_detection/no_person_image_data.h"
#include "tensorflow/lite/micro/examples/person_detection/person_detect_model_data.h"
#include "tensorflow/lite/micro/examples/person_detection/person_image_data.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
// #include "tensorflow/lite/micro/micro_optional_debug_tools.h"
#include "tensorflow/lite/micro/testing/micro_test.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"



#include "tensorflow/lite/micro/all_ops_resolver.h"


// Setup C linkages
extern "C" void dbg_ch(const char);
extern "C" void dbg_str(const char*);
extern "C" void dbg_hex32(int32_t);
extern "C" void dbg_str_int(const char*, int32_t);
extern "C" void dbg_str_hex32(const char*, int32_t);

extern "C" uint32_t xTaskGetTickCount(void);

namespace tflite {
	
}

// Globals, used for compatibility with Arduino-style sketches.
//namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
int inference_count = 0;

// Create an area of memory to use for input, output, and intermediate arrays.
// Minimum arena size, at the time of writing. After allocating tensors
// you can retrieve this value by invoking interpreter.arena_used_bytes().

constexpr int kTensorArenaSize = 95 * 1024;
uint8_t tensor_arena[kTensorArenaSize] __attribute__ ((aligned (16))) ;
//}  // namespace

// The name of this function is important for Arduino compatibility.
extern "C" void setup() {
  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_person_detect_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

#if 0
  // This pulls in all the operation implementations we need.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::AllOpsResolver resolver;
    // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
	  
#else	  
  // tflite::AllOpsResolver resolver;
  static tflite::MicroMutableOpResolver<3> micro_op_resolver;
  micro_op_resolver.AddAveragePool2D();
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddDepthwiseConv2D();

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(model, micro_op_resolver, tensor_arena,
                                       kTensorArenaSize, error_reporter);
#endif


  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
    return;
  }

  // Obtain pointers to the model's input and output tensors.
  input = interpreter->input(0);
  
   // Make sure the input has the properties we expect.
  if (nullptr == input) TF_LITE_REPORT_ERROR(error_reporter, "nullptr == input");
  if (4 != input->dims->size) TF_LITE_REPORT_ERROR(error_reporter, "4 != input->dims->size");
  if (1 != input->dims->data[0]) TF_LITE_REPORT_ERROR(error_reporter, "1 != input->dims->data[0]");
  if (kNumRows != input->dims->data[1]) TF_LITE_REPORT_ERROR(error_reporter, "96 != input->dims->data[1]");
  if (kNumCols != input->dims->data[2]) TF_LITE_REPORT_ERROR(error_reporter, "96 != input->dims->data[2]");
  if (kNumChannels != input->dims->data[3]) TF_LITE_REPORT_ERROR(error_reporter, "1 != input->dims->data[3]");
  if (kTfLiteUInt8 != input->type) TF_LITE_REPORT_ERROR(error_reporter, "kTfLiteUInt8 != input->type");
  
  output = interpreter->output(0);

  // Keep track of how many inferences we have performed.
  inference_count = 0;
}

 void RunSingleIterationCustomInput(const uint8_t* custom_input) {
    //Populate input tensor with an image with no person.
    const uint8_t*	pucSrc = custom_input;
	uint8_t*	pucDst = (uint8_t*)input->data.data;
	for (int i = 0 ; i != input->bytes; i++) {
		*pucDst++ = *pucSrc++;
	}

    //Run the model on this input and make sure it succeeds.
	volatile uint32_t* CycCnt = (volatile uint32_t*)0xE0001004;
	volatile uint32_t* OscCtrl = (volatile uint32_t*)0x40005484;
	uint32_t	freq = 32768*(*OscCtrl+3);
	uint32_t	tStart = *CycCnt;
    TfLiteStatus invoke_status = interpreter->Invoke();
	uint32_t	tStop = *CycCnt;
    if (invoke_status != kTfLiteOk) {
      TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
    }
	
	// Get the output from the model, and make sure it's the expected size and
  // type.
  
  TfLiteTensor* output = interpreter->output(0);
  if (4 != output->dims->size) TF_LITE_REPORT_ERROR(error_reporter, "4 != output->dims->size");
  if (1 != output->dims->data[0]) TF_LITE_REPORT_ERROR(error_reporter, "1 != output->dims->data[0]");
  if (1 != output->dims->data[1]) TF_LITE_REPORT_ERROR(error_reporter, "1 != output->dims->data[1]");
  if (1 != output->dims->data[2]) TF_LITE_REPORT_ERROR(error_reporter, "1 != output->dims->data[2]");
  if (kCategoryCount != output->dims->data[3]) TF_LITE_REPORT_ERROR(error_reporter, "kCategoryCount != output->dims->data[3]");
  if (kTfLiteUInt8 != output->type) TF_LITE_REPORT_ERROR(error_reporter, "kTfLiteUInt8 != output->type");

 // Make sure that the expected "Person" score is higher than the other class.
  uint8_t person_score = output->data.uint8[kPersonIndex];
  uint8_t no_person_score = output->data.uint8[kNotAPersonIndex];
  TF_LITE_REPORT_ERROR(error_reporter,
                       "person data.  person score: %d, no person score: %d\n",
                       person_score, no_person_score);
  //TF_LITE_MICRO_EXPECT_GT(person_score, no_person_score);
  }

// The name of this function is important for Arduino compatibility.
extern "C" void loop() {
    TF_LITE_REPORT_ERROR(error_reporter, "\n***********************\nRunning image with person");                            
	RunSingleIterationCustomInput(g_person_data); 
	TF_LITE_REPORT_ERROR(error_reporter, "\n***********************\nRunning image without person");                            
	RunSingleIterationCustomInput(g_no_person_data);	

	// // Increment the inference_counter, and reset it if we have reached
	// // the total number per cycle
	// inference_count += 1;
	// if (inference_count >= kInferencesPerCycle) inference_count = 0;
}
