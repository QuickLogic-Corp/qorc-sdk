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

#ifndef WW_METADATA_H 
#define WW_METADATA_H

#define MAX_KEPHRASE_SCORES (16)
typedef struct t_ql_audio_meta_data_
{
  int32_t n_keyphrase_count; // number of key phrases supported - default is 0
  int32_t n_keyphrase_triggered_index; // Index in the array of keyphrases
  int32_t n_keyphrase_start_index; // Start of Key Phrase in number of samples from start of buffer
  int32_t n_keyphrase_end_index;  // End of Key Phrase in number of samples from start of buffer 
  int32_t a_keyphrase_score;
  int32_t n_rdsp_length_estimate; // RDSP WW Length estimate 
} t_ql_audio_meta_data;

#endif /* WW_METADATA_H */
