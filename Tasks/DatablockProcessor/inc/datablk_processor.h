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
 *    File   : datablk_processor.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

/** @file datablk_processor.h
 *
 * @brief Thread handler to process data block available and waiting
 *   on the input queues. Processed data is output to the specified output
 *   queues or chained to the next processing element (PE)
 */

#ifndef __DATABLK_PROCESSOR_H__
#define __DATABLK_PROCESSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "Fw_global_config.h"
#include "datablk_mgr.h"
#include "process_ids.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
/** @addtogroup DATABLK_PROCESSOR thread handler module
 * @brief This thread handler module defines a generic input/output/configuration
 * to handle a block of data using user supplied processing element functions.
 *
 * This module provides APIs to specify the input connections, and
 * output connections. \ref Fig7 "Figure 7" below depicts this general
 * structure for the thread handler and a simple flowchart that shows
 * processing of data
 *
 * \anchor Fig7 \diafile Fig7.dia "Figure 3. Thread handler Overview" width=\textwidth
 *
 * @{
 */

#define DATABLK_PROCESSOR_API_VERSION_MAJOR (0)      ///< API Version number Major
#define DATABLK_PROCESSOR_API_VERSION_MINOR (1)      ///< API Version number Minor

#define DATABLK_PE_SEMAPHORE_WAITTIME   (200)      ///< 10ms wait time for acquiring semaphore

typedef enum e_datablk_pe_active_states {
  DATABLK_PE_ACTIVE_FALSE,   ///< Active false ==> processing function downward chain removed
  DATABLK_PE_ACTIVE_TRUE,    ///< Active true ==> actively processing datablocks
  DATABLK_PE_ACTIVE_BYPASS   ///< Active bypass ==> simulates processing element by copying or user-defined function
} datablk_pe_active_states_t ;

/** @brief structure for a thread handler queue element */
typedef struct st_QAI_ThreadHandlerQElement {
    QAI_DataBlock_t *pdata_block; ///< pointer to a data block structure
} QAI_ThreadHandlerQElement_t;

/** @brief Get API and code version information
 *
 * @return Returns 32-bit version info for this module.
 *       |            |                    |
 *       |  ---:      | ---:               |
 *       | Bit[31-24] | API major version  |
 *       | Bit[23-16] | API minor version  |
 *       | Bit[15- 8] | code major version |
 *       | Bit[ 7- 0] | code minor version |
 */
unsigned long thread_handler_get_version(void);

typedef struct st_datablk_pe_funcs
{
  /** @brief thread handler processing element configuration function
   *
   *  @param[in] pdatablk_pe_cfg pointer to configuration parameters
   *
   */
  void (*pconfig)(void *p_pe_object);
  
  /** processing function to invoke for the current datablock */
  datablk_pe_process_func_t  *pprocess;

  /** @brief start this processing element */
  int  (*pstart)(void);

  /** @brief stop this processing element */
  int  (*pstop)(void);

  /** @brief processing element specific object handle 
  *  Example: Nuance VR initialization and configuration 
  *  might use this variable to access language model, heap 
  *  and other parameters specific to the Nuance VR engine.
  */
  void                *p_pe_object;
} datablk_pe_funcs_t;

/** @brief Thread handler processing element configuration structure 
 *  To setup a thread handler processing element, the application
 *  should fill in these fields for all processing elements grouped
 *  and then invoke the thread_handler_setup function.
 *
 *  Thread handler processing element (\ref THPE) functions. Implemented 
 *  processing elements and specified in the thread handler configuration
 *  are invoked by the thread handler.
 */
typedef struct st_datablk_pe_descriptor
{
  process_id_t              in_id;           ///< processing element input source ID
  process_id_t              out_id;          ///< processing element ID
  int                       fActive;         ///< indicates whether the processing 
  int                       fSupplyOut;      ///< whether to supply output buffer
  int                       fReleaseIn;      ///< whether to release input buffer
                            ///< element processses data or skips processing data
  outQ_processor_t          *poutq_processor; ///< Output Queue processor for this processing element
  datablk_pe_funcs_t         *pe_funcs;        ///< processing element functions
  datablk_pe_process_func_t *pe_bypass_func;  ///< bypass function to use when the fActive indicates BYPASS mode
  SemaphoreHandle_t         *p_pe_sem;        ///< pointer to Semaphore handle for the processing element
} datablk_pe_descriptor_t ;

/** brief thread configuration */
typedef struct st_datablk_processor_params
{
  int                      dbp_task_priority;  ///< desired task priority
  QueueHandle_t           *p_dbp_task_inQ;     ///< input queue handle
  int                      num_pes;           ///< number of PEs for this thread
  datablk_pe_descriptor_t  *p_descr;          ///< array of thread PE configurations
  int                      stack_depth;      ///< depth of stack needed for this thread
  char                    *dbp_task_name;    ///< datablock processor task name string
  xTaskHandle              datablk_pe_handle; ///< Task handle for use with task control 
                           ///< functions such as vTaskResume
} datablk_processor_params_t ;

/** @brief Datablock processor task processing elements setup. Table of processing element IDs
 *         and functions should have been setup in `p_dbp_params` before invoking this function.
 *         Input and output queues should have been allocated before invoking this function.
 *
 *  @param[in] p_dbp_params pointer to datablock processor parameters structure. 
 *                  Members of this structure are used to create the task for 
 *                  the datablock processor.
 *
 *  @return 0 on success, an error code otherwise
 */
int datablk_processor_task_setup(datablk_processor_params_t *p_dbp_params);

/* @param[in]   pdatablk_processor_params is a pointer to the PE descriptor block
*  @param[in]   outqid is the id of the output q from the PE since output queues are 1:1 with producing PE
 * @param[i]]   factive is the value to set in factive
*/

void datablk_set_factive(datablk_processor_params_t* pdatablk_processor_params, int outqid, bool factive);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DATABLK_PROCESSOR_H__ */
