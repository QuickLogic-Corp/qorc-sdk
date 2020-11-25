/*
* Copyright (c) 2017, SensiML Corporation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __KB_H__
#define __KB_H__
#include "kb_typedefs.h"
#include "kb_defines.h"



//Total Models in this Knowledge Pack
#define SENSIML_NUMBER_OF_MODELS 1

//Model Indexes to use for calls
#define KB_MODEL_slide_rank_0_INDEX 0

#define MAX_VECTOR_SIZE 12

//FILL_SENSIML_SENSOR_USAGES

#ifdef __cplusplus
extern "C"
{
#endif


/**
* @brief Initialize the parameters of all the models, must be run first.
*/
void kb_model_init();

/**
* @brief Main Pipeline entry point.
*
* This is the main entry point into the pipeline. It takes a single
* timepoint of data as an array. Adds that sample to the internal ring buffer
* checks for a segment, generates features if there is a segment,
* produces a classification and returns the result.
* Classification results will be 0 if unkown through the classification numbers
* you have. This function returns -1 when a segment hasn't yet been identified.
* returns -2 when a segment has been filtered
*
*
* @param[in] pSample Pointer to a single time point of data accross senseors (ie. ax,ay,az)
* @param[in] nsesnors (unused currently)
* @param[in] model_index Model index to use.
*/
int kb_run_model(SENSOR_DATA_T *pSample, int nsensors, int model_index);

/**
* @brief Advance the model so it is ready for new sample data
* This should be used once you have recieved a
* classification and are done with the data in the buffer
*
* Inputs:
*		model index - index of the model to retrain
*/
int kb_reset_model(int model_index);

/**
* @brief Flush the internal data buffer of a model
*
* Inputs:
*		model index - index of the model to flush the buffer
*/
int kb_flush_model_buffer(int model_index);

/**
* @brief Run model with cascade feature resets.
*
* This performs the same as kb_run_model, but is meant for models using cascade
* feature generation where it will only classify after all of the feature banks
* in the cascade have been filled, then reset the number of feature banks to 0.
* This is different from run model, which treats the feature banks as a circular
* buffer and constantly classifiers
* Classification results will be 0 if unkown through the classification numbers
* you have. This function returns -1 when it is wainting for more data to create
* a classification.* returns -2 when features were generated for a feature bank
*
*
* @param[in] pSample Pointer to a single time point of data accross senseors (ie. ax,ay,az)
* @param[in] nsesnors (unused currently)
* @param[in] model_index Model index to use.
*/

int kb_run_model_with_cascade_reset(SENSOR_DATA_T *pSample, int nsensors, int model_index);

/**
* @brief Add a custom segment to the pipeline for classification.
*
* Setup the model ring buffer with the user provided
* buffer to be used as well as its size.
*
* @param[in] pSample Pointer to a buffer to use.
* @param[in] len length of the ring buffer.
* @param[in] nbuffs Number of buffers of length len to create in pBuffer pointer.
* @param[in] model_index Model index to use.
*/
void kb_add_segment(uint16_t *pBuffer, int len, int nbuffs, int model_index);

/**
* Runs the pipeline starting a segmentation through classification.
*
* @param[in] model_index Model index to use.
*/
int kb_run_segment(int model_index);

/**
* @brief This performs the same as kb_run_segment, but is meant for models using cascade
* feature generation where the user desires classifications only after all of the feature banks
* in the cascade have been filled. After classification all of the feature banks are emptied.
* This is different from run model, which treats the feature banks as a circular
* buffer and constantly classifiers popping the oldest and adding the newest.
* Classification results will be 0 if unkown through the classification numbers
* you have. This function returns -1 when it is wainting for more data to create
* returns -2 when features were generated for a feature bank
* a classification
*
* @param[in] model_index Model index to use.
*/
int kb_run_segment_with_cascade_reset(int model_index);

/**
* @brief Print the model name to map relationship
*
*/
void kb_print_model_map();

void kb_print_model_class_map(int model_index, char *output);

/**
* @brief Prints the current information for the model
*
* @param[in] model_index Model index to use.
* @param[in] result result from most recent classification
*/
void sml_print_model_result(int model_index, int result);
#define kb_print_model_result sml_print_model_result

/**
* @brief Prints the model  score for specified index
*
* @param[in] model_index Model index to use.
*/
int sml_print_model_score(int model_index);
#define kb_print_model_score sml_print_model_score

/**
* @brief Get the model header information for model index
*
* @param[in] model_index Model index to use.
* @param[in] pointer struct for the particular type of classifier (defined in kb_typdefs.h).
*
*  Returns:
*      1 if successful
*      0 if not supported for this classifier
*/

int get_model_header(int model_index, void *model_header);

/**
* @brief Prints the model class map
*
* @param[in] model_index Model index to use.
* @param[in] pattern_index Pattern index in the classifier to retrieve.
* @param[in] pointer struct for the particular type of classifier pattern (defined in kb_typdefs.h).
*
* Returns:
*      1 if successful
*      0 if not supported for this classifier, or pattern index is out of bounds
*/
int get_model_pattern(int model_index, int pattern_index, void *pattern);

/**
* @brief Set the number of stored patterns in a model to 0
*
* Inputs:
*		model index -index of the model to reset
*/
int flush_model(int model_index);

/**
* @brief Expand model with most recent pattern.
*
* After recieving a classification, you can tell the model to add this classifiaction as
*  a pattern with a specific category as well as an influence field. The larger the field
* the larger the area this pattern can be activiated in. This can be used to add
*
*/
int kb_add_last_pattern_to_model(int model_index, uint16_t category, uint16_t influence);

/**
* @brief Retrains the model by adding the a custom feature vector with a new label
*
*
* Inputs:
*		model index - number of axes in the data stream
*      category - category of the vector to add
*      influence - amount of weight we should give to this new feature vector when training the model
*
* Returns:
* 		0 if model does not support dynamic updates
* 		-1 if model can not be updated anymore
* 		1 if model was succesfully updated
*/
int kb_add_custom_pattern_to_model(int model_index, uint8_t *feature_vector, uint16_t category, uint16_t influence);

/***
* @brief scores the current model based on the input category
*
*
* Inputs:
*		model index - number of axes in the data stream
*      category - category of the vector to add
*
* Returns:
* 		0 if model does not support scoring
* 		-1 if model can not be scored anymore
* 		1 if model was succesfully scored
*/
int kb_score_model(int model_index, uint16_t category);

/***
* @brief retrain model based on scores
*
*
* Inputs:
*		model index - number of axes in the data stream
*
* Returns:
* 		0 if model does not support retraining
* 		1 if model was succesfully retrained
*/
int kb_retrain_model(int model_index);

/**
* @brief Gets the pointer to 16-byte UUID of model
*
* @param[in] model_index Model index to get UUID from
* @return uint8_t* Pointer to 16-byte UUID for model
*/
const uint8_t *sml_get_model_uuid_ptr(int model_index);

/**
* @brief Gets the Segment for debug printing, saving.
*
* @param[in] model_index Model index to use.
* @param[out] p_seg_len pointer to segment length
*/
void sml_get_segment_length(int model_index, int *p_seg_len);

/**
* @brief Gets segment sample data
*
* @param[in] model_index Model Index to use
* @param[in] sample_num Sample number to retrieve
* @param[out] p_sample_len pointer to sample length
* @param[out] p_sample_data pointer to use for sample data array
*/
void sml_get_segment_sample(int model_index, int sample_num, int *p_sample_len, SENSOR_DATA_T *p_sample_data);

/**
* @brief Get Debug logging level, if enabled
*
* @return int 1-4, or 0 if disabled.
*/
int sml_get_log_level();

/**
* @brief Gets feature vector for debug.
*
* @param[in] model_index Model index to use.
* @param[out] fv_arr Feature Vector to copy into
* @param[out] p_fv_len Feature vector length to copy
*/
void sml_get_feature_vector(int model_index, uint8_t *fv_arr, uint8_t *p_fv_len);
#define kb_get_feature_vector sml_get_feature_vector

#if SML_ADD_SEGMENT_DATA
/**
* @brief Gets the Segment for debug.
*
* @param[in] model_index Model index to use.
* @param[out] fv_arr Feature Vector to copy into
* @param[out] p_fv_len Feature vector length to copy
*/
void sml_get_segment_data(int model_index, SENSOR_DATA_T *seg_arr, uint16_t *p_seg_len);
#define kb_get_segment_data sml_get_segment_data
#endif


/**
* @brief Set the feature vector for a model 
*
* @param[int] model_index Model index to use.
* @param[uint8_t *] feature_vector to set the model input to
*
* @returns[int] the count of features that were set 
*/
int sml_set_feature_vector(int model_index, uint8_t * feature_vector);

/**
* @brief Recognize a feature vector that is in the model buffer
*
* @param[int] model_index Model index to use.
*
* @returns[uint16_t] classification result 
*/
uint16_t sml_recognize_feature_vector(int model_index);

/**
* @brief Gets the Segment for debug.
*
* @param[int] model_index Model index to use.
* @param[void *] model result object, specific to the model you are getting results for
*
* @returns[int] 1 if success, 0 if not applicatbale to this model type
*/
int sml_classification_result_info(int model_index, void * model_results);


#ifdef __cplusplus
}
#endif

#endif //__KB_H__
