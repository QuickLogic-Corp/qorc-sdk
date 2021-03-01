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

#include "vr_engine_api.h"
#include "ql_vr_amazon.h"
   
// These 3 APIs need to be provided by every VR engine
void vr_engine_init(void)
{
  // initialize the VR engine
  init_amazon_wwe();
  return;
}

int vr_engine_get_samples_per_frame(void)
{
  // return samples per frame size used by the VR engine
  return amazon_samples_per_frame();
}

int vr_engine_process(short *samples)
{
  // process input samples and return if wakeword is detected
  // 0 = no wakeword detected, 1 = wakeword detected
  return amazon_vr_process(samples);
}
