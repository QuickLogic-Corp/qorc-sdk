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

#ifndef __QL_OPUS_ENCODE__
#define __QL_OPUS_ENCODE__

extern void init_opus_encoder(void);
extern int32_t get_opus_encoded_data(int16_t *p_pcm_data, int32_t n_samples_in, uint8_t *p_encoded_buffer);

#endif /* __QL_OPUS_ENCODE__ */
