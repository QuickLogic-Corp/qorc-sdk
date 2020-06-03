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

/*==========================================================
 *
 *    File   : datablk_processor.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

/** @file datablk_processor.c 
 */

#include "datablk_mgr.h"
#include "datablk_processor.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "ql_util.h"

/** @addtogroup DATABLK_PROCESSOR
 *  @{
 */
   
#define DATABLK_PROCESSOR_CODE_VERSION_MAJOR (0)      ///< Code Version number Major
#define DATABLK_PROCESSOR_CODE_VERSION_MINOR (1)      ///< Code Version number Minor

unsigned long thread_handler_get_version(void)
{
  unsigned long datablk_pe_version ;
  datablk_pe_version =  (DATABLK_PROCESSOR_API_VERSION_MAJOR  << 24) |
                       (DATABLK_PROCESSOR_API_VERSION_MINOR  << 16) |
                       (DATABLK_PROCESSOR_CODE_VERSION_MAJOR <<  8) |
                       (DATABLK_PROCESSOR_CODE_VERSION_MINOR <<  0);

  return datablk_pe_version;
}


/** @brief thread handler configuration function */
static void datablk_pe_config(void *pvParams)
{
     datablk_processor_params_t *p_params = (datablk_processor_params_t *) pvParams;
     void (*pconfig)(void *p_pe_object);
     datablk_pe_funcs_t *p_pe_funcs;
    
     for (int k = 0; k < p_params->num_pes; k++) {
       p_pe_funcs = p_params->p_descr[k].pe_funcs;
       pconfig = p_pe_funcs->pconfig;
     /** configuration the audio subtask */
         if (pconfig) {
            (*pconfig)(p_pe_funcs->p_pe_object);
         }
     }
}

static void datablk_pe_process_bypass_func(
                QAI_DataBlock_t *pIn,
                QAI_DataBlock_t *pOut,
                QAI_DataBlock_t **pRet,
                datablk_pe_event_notifier_t *pevent_notifier
             )
{
     int nbytes ;
     *pRet = pOut;
     if (pOut) 
     {
        nbytes = pIn->dbHeader.dataElementSize * pIn->dbHeader.numDataElements ;
        memcpy(pOut, pIn, sizeof(QAI_DataBlockHeader_t)+nbytes);
     }
     return;
}

/** @brief Datablock processor task */
static void datablk_processor_task(void *pvParameters)
{
  datablk_processor_params_t *p_params = (datablk_processor_params_t *) pvParameters;
  QueueHandle_t   dbp_inQ;       ///< input Queue to wait for incoming datablock
  QAI_DataBlock_t    *pIn, *pOut, *pRet;
  //TIM BaseType_t         xResult = pdFAIL;
  QAI_DataBlockMgr_t *pdatablk_mgr_handle;
  datablk_pe_descriptor_t *p_pe_descr;
  datablk_pe_process_func_t      *pprocess_func;
  datablk_pe_event_notifier_t    *pevent_notifier;
  int ret;

  dbp_inQ  = *(p_params->p_dbp_task_inQ);
   
  
  dbg_str_str("Datablock processor task name",  p_params->dbp_task_name);
  dbg_str_hex32("inQ         ", (uint32_t)dbp_inQ);

  datablk_pe_config(p_params);

  while (1) {
      /** Wait for input data */
      //TIM xResult = xQueueReceive( dbp_inQ, &pIn, portMAX_DELAY);
      xQueueReceive( dbp_inQ, &pIn, portMAX_DELAY);
      int fReleaseInCount = 0;   
      for (int k = 0; k < p_params->num_pes; k++) {
          p_pe_descr = &p_params->p_descr[k];
          if (DATABLK_PE_ACTIVE_FALSE == p_pe_descr->fActive)
          {
            continue;
          }
          pdatablk_mgr_handle = p_pe_descr->poutq_processor->p_dbm;
          if (pIn->dbHeader.pid == p_pe_descr->in_id) {
              /** allocate pOut data block if fSupplyOut is specified as true */
              pOut = NULL;
              pprocess_func = p_pe_descr->pe_funcs->pprocess;
              if (p_pe_descr->fSupplyOut) {
                 ret = datablk_mgr_acquire(pdatablk_mgr_handle, &pOut, 0);
                 if (ret != 0) {
                   // @todo send error message and continue
                   continue;
                 }
              }
              if (DATABLK_PE_ACTIVE_BYPASS == p_pe_descr->fActive)
              {
                  if (p_pe_descr->pe_bypass_func)
                     pprocess_func = p_pe_descr->pe_bypass_func;
                  else
                     pprocess_func = datablk_pe_process_bypass_func;
              }
              /* increment the count if current PE is holding the datablock */
              fReleaseInCount += ( p_pe_descr->fReleaseIn == false ) ? 1 : 0; 
              /** process input data */
              pRet = NULL;
              pevent_notifier =  p_pe_descr->poutq_processor->p_event_notifier;
              if (p_pe_descr->p_pe_sem)
              {
                xSemaphoreTake( *p_pe_descr->p_pe_sem, ( TickType_t ) (DATABLK_PE_SEMAPHORE_WAITTIME) );
              }
              
              (*pprocess_func)(pIn, pOut, &pRet, pevent_notifier);
              
              if (p_pe_descr->p_pe_sem)
              {
                xSemaphoreGive(*p_pe_descr->p_pe_sem);
              }
              
              if (pRet) {
                  /** push the output to output queues */
                  datablk_mgr_WriteDataBufferToQueues(p_pe_descr->poutq_processor, dbp_inQ, pRet);
              }

            }
        } 
        /** Discard the input data block */
        if (fReleaseInCount == 0) {
           /* no body is holding the block, go ahead and release */
           datablk_mgr_release(pdatablk_mgr_handle, (QAI_DataBlock_t *)pIn);
        } else if (fReleaseInCount > 1) {
          /* more than one PE is holding the block, so increment the usecount to allow them independed release */
          datablk_mgr_usecount_increment((QAI_DataBlock_t *)pIn, (uint8_t)(fReleaseInCount-1));
        }
  }
  //vTaskDelete(NULL);

}

/* @brief Datablock processor task handler setup
 *
 * Creates a new task using the datablock processing element functions
 * specified in the structure pdatablk_pe_config
 *
 */
int datablk_processor_task_setup(datablk_processor_params_t *p_dbp_params)
{
    xTaskCreate ( datablk_processor_task,
                  p_dbp_params->dbp_task_name,
                  p_dbp_params->stack_depth,  
                  p_dbp_params,
                  p_dbp_params->dbp_task_priority, 
                  &p_dbp_params->datablk_pe_handle
                );
    
    configASSERT( p_dbp_params->datablk_pe_handle );
    return 0;
}

void datablk_set_factive(datablk_processor_params_t* pdatablk_processor_params, int outqid, bool fActive) {
    datablk_processor_params_t *p_params = pdatablk_processor_params;
    datablk_pe_descriptor_t *p_pe_descr;
    
    for (int k = 0; k < p_params->num_pes; k++) {
        p_pe_descr = &p_params->p_descr[k];
        if (outqid == p_pe_descr->out_id) {
            p_pe_descr->fActive = fActive;
        } 
    }
}

/** @} */
