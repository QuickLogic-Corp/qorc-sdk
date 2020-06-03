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

/*
 *
 *  Convert stereo audio blocks into mono
 *
 */
#include "audio_stereo2mono.h"
     
static QAI_DataBlock_t* pdbPartial = NULL;     // Pointer to block where we are assembling the mono data, if NULL no assembly has started
     
 void datablk_pe_config_stereo2mono(void *p_pe_object) {
 }

void datablk_pe_process_stereo2mono(

       QAI_DataBlock_t *pIn,
       QAI_DataBlock_t *pOut,
       QAI_DataBlock_t **pRet,
       datablk_pe_event_notifier_t *pevent_notifier
     )
{
    // Check for block in progress.  If none, then use this one; else fill in tail end and release new block
    if (pdbPartial == NULL) {
        pdbPartial = pIn;
        pdbPartial->dbHeader.numDataChannels = 1;       // Indicate mono
        *pRet = NULL;                                   // Indicate no output this time
    } else {
        memcpy(pdbPartial->p_data+(pIn->dbHeader.numDataElements/2) * pIn->dbHeader.dataElementSize, pIn->p_data, (pIn->dbHeader.numDataElements/2) * pIn->dbHeader.dataElementSize);
        *pRet = pdbPartial;
        datablk_mgr_usecount_increment(pdbPartial, -1); // Indicate that we are no longer using the block
        pdbPartial = NULL;                              // Indicates that process needs to grab a new block next time
        datablk_mgr_release_generic(pIn);               // Release the second block
    }
}