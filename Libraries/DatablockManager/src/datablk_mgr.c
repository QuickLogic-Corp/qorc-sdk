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
 *    File   : datablk_mgr.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

/** @file datablk_mgr.c
 */

#include "datablk_mgr.h"
#include <string.h>
#include "ql_util.h"

/** @addtogroup DATABLK_MGR data block manager module
 *  @{
 */

#define DATABLK_MGR_CODE_VERSION_MAJOR (0)    ///< Code Version number Major
#define DATABLK_MGR_CODE_VERSION_MINOR (1)    ///< Code Version number Minor

QAI_DataBlockMgr_t *gdbmArray[MAX_DBM_ARRAY_SIZE]; ///< Global DBM Array holding pointers to Datablock manager handles
static int gdbmArrayCount = 0;   ///< holds the count of datablock manager handles index for gdbmArray

unsigned long datablk_mgr_get_version(void)
{
  unsigned long datablk_mgr_version ;
  datablk_mgr_version = (DATABLK_MGR_API_VERSION_MAJOR  << 24) |
                        (DATABLK_MGR_API_VERSION_MINOR  << 16) |
                        (DATABLK_MGR_CODE_VERSION_MAJOR <<  8) |
                        (DATABLK_MGR_CODE_VERSION_MINOR <<  0);

  return datablk_mgr_version;
}

//TIM
QueueHandle_t  DataBlockFreeQ;

int datablk_mgr_init(QAI_DataBlockMgr_t *pdatablk_mgr_handle, 
                     void *pmem, 
                     int mem_size, 
                     int item_count, 
                     int item_size_bytes)
{
    QAI_DataBlock_t *pnext_item;
    int size_data_block ;
    int num_data_blocks ;
    
/** Detailed description */
    // reached maximum number of datablock manager handles
    configASSERT(gdbmArrayCount < MAX_DBM_ARRAY_SIZE); 
    
    // compute and adjust each datablock size to be multiple 4-bytes
    size_data_block = (item_count * item_size_bytes) + sizeof(QAI_DataBlockHeader_t);
    size_data_block = ( size_data_block + 3 ) & ~0x03 ;
    
    num_data_blocks = (mem_size) / size_data_block;
    /** Create dataBlockFreeQ queue of size num_data_blocks with each element */
    pdatablk_mgr_handle->max_data_blocks = num_data_blocks;
    //TIM
    DataBlockFreeQ = 
    pdatablk_mgr_handle->dataBlockFreeQ = xQueueCreate(num_data_blocks, sizeof(QAI_DataBlock_t *));
    configASSERT(pdatablk_mgr_handle->dataBlockFreeQ);
    
    pdatablk_mgr_handle->size_data_block = size_data_block;
    pdatablk_mgr_handle->maxDataElements = item_count;
    pdatablk_mgr_handle->dataElementSize = item_size_bytes;
    pdatablk_mgr_handle->refIndex = gdbmArrayCount;
    gdbmArray[gdbmArrayCount] = pdatablk_mgr_handle;
    /** Populate dataBlockFreeQ with available data blocks from pdata_blocks[] */
    
    // adjust the start address to 4-byte boundary
    pnext_item = (QAI_DataBlock_t *)( ((uint32_t)pmem + 3) & ~0x03 ); 
    for (int k = 0; k < num_data_blocks; k++) {
      xQueueSend(pdatablk_mgr_handle->dataBlockFreeQ, (void *)&pnext_item, (TickType_t)0);
      pnext_item = (QAI_DataBlock_t *)((uint32_t)pnext_item + size_data_block);
    }

    /* Save datablock memory start and end addresses for later use */    
    pdatablk_mgr_handle->pmem_buf_start = pmem ;
    pnext_item = (QAI_DataBlock_t *)((uint32_t)pnext_item - size_data_block);
    pdatablk_mgr_handle->pmem_buf_end   = pnext_item;
   
    gdbmArrayCount++;
    /** return success code */
    return 0;
}

static int datablk_mgr_acquire_common(QAI_DataBlockMgr_t *pdatablk_mgr_handle, QAI_DataBlock_t **pdata_block, TickType_t xTicksToWait, int context)
{
    BaseType_t ret;
    QAI_DataBlock_t *qItem;
    TickType_t tickCount;
    configASSERT(pdatablk_mgr_handle); // Make sure a valid handle is provided

    *pdata_block = NULL;
    if (context)
    {
      ret = xQueueReceiveFromISR(pdatablk_mgr_handle->dataBlockFreeQ, &qItem, NULL);
      tickCount = xTaskGetTickCountFromISR();
    }
    else
    {
      ret = xQueueReceive(pdatablk_mgr_handle->dataBlockFreeQ, &qItem, xTicksToWait);
      tickCount = xTaskGetTickCount();
    }
    
    if (pdTRUE == ret) 
    {
      *pdata_block = qItem;
      
    //  configASSERT((*pdata_block)->dbHeader.numUseCount == 0);
    //  configASSERT((*pdata_block)->dbHeader.numDropCount == 0);
      
      (*pdata_block)->dbHeader.numUseCount = 0; // set use count to 0
      (*pdata_block)->dbHeader.numDropCount = 0; // reset numDropCount
      (*pdata_block)->dbHeader.bufModeBits &= DBM_HDR_BUFMODE_DBM_INDEX_MASK;
      (*pdata_block)->dbHeader.bufModeBits |= pdatablk_mgr_handle->refIndex;

        
        // update Tstart timestamp
      (*pdata_block)->dbHeader.Tstart = tickCount;
        // fill with default values
      (*pdata_block)->dbHeader.numDataElements = pdatablk_mgr_handle->maxDataElements;
      (*pdata_block)->dbHeader.numDataChannels = 1;
      (*pdata_block)->dbHeader.dataElementSize = pdatablk_mgr_handle->dataElementSize;
      return 0;
    } 
    else 
    {
      return -1;
    }
}

int datablk_mgr_acquire(QAI_DataBlockMgr_t *pdatablk_mgr_handle, QAI_DataBlock_t **pdata_block, TickType_t xTicksToWait)
{
   return datablk_mgr_acquire_common(pdatablk_mgr_handle, pdata_block, xTicksToWait, 0);
}

int datablk_mgr_acquireFromISR(QAI_DataBlockMgr_t *pdatablk_mgr_handle, QAI_DataBlock_t **pdata_block)
{
    return datablk_mgr_acquire_common(pdatablk_mgr_handle, pdata_block, 0, 1);
}


int datablk_mgr_num_acquired(QAI_DataBlockMgr_t *pdatablk_mgr_handle)
{
  return uxQueueSpacesAvailable( pdatablk_mgr_handle -> dataBlockFreeQ );
}

int datablk_mgr_num_available(QAI_DataBlockMgr_t *pdatablk_mgr_handle)
{
  return uxQueueMessagesWaiting( pdatablk_mgr_handle -> dataBlockFreeQ );
}

static uint8_t datablk_mgr_usecount_decrement(QAI_DataBlock_t *pdata_block)
{
  if(pdata_block -> dbHeader.numUseCount != 0)
  {
    taskENTER_CRITICAL();
    pdata_block -> dbHeader.numUseCount--;
    taskEXIT_CRITICAL();
  }
    return pdata_block -> dbHeader.numUseCount ;
}

static uint8_t datablk_mgr_usecount_decrementFromISR(QAI_DataBlock_t *pdata_block)
{
  if(pdata_block -> dbHeader.numUseCount != 0)
  {    
    pdata_block -> dbHeader.numUseCount--;
  }
    return pdata_block -> dbHeader.numUseCount ;
}

uint8_t datablk_mgr_usecount_increment(QAI_DataBlock_t *pdata_block, uint8_t count)
{
    taskENTER_CRITICAL();
    pdata_block -> dbHeader.numUseCount += count;
    taskEXIT_CRITICAL();
    return pdata_block -> dbHeader.numUseCount ;
}

uint8_t datablk_mgr_usecount_incrementFromISR(QAI_DataBlock_t *pdata_block, uint8_t count)
{
    pdata_block -> dbHeader.numUseCount += count;
    return pdata_block -> dbHeader.numUseCount ;
}

int datablk_mgr_validity_check
          (QAI_DataBlockMgr_t *p_dbm,
           QAI_DataBlock_t *pdata_block
          )
{
    if ( (!p_dbm) || (!pdata_block) )
       return 0;

    if ( ((void*)pdata_block >= p_dbm->pmem_buf_start) &&
         ((void*)pdata_block <= p_dbm->pmem_buf_end)
       )
    {
      return 1;       /* Valid datablock */
    }
    else
    {
      return 0;        /* Invalid datablock */
    }
}

void datablk_mgr_release_common(QAI_DataBlockMgr_t *pdatablk_mgr_handle, QAI_DataBlock_t *pdata_block, int context)
{
    QueueHandle_t free_q ;
    BaseType_t ret = -1;
    uint8_t numUseCount;

    // check for valid handle
    if (!datablk_mgr_validity_check(pdatablk_mgr_handle, pdata_block))
      return;
    if(pdata_block -> dbHeader.numUseCount > 0) 
    {
    free_q = pdatablk_mgr_handle -> dataBlockFreeQ;
    if (context)
    {
      numUseCount = datablk_mgr_usecount_decrementFromISR(pdata_block);
    }
    else
    {
      numUseCount = datablk_mgr_usecount_decrement(pdata_block);
    }
    
    configASSERT(numUseCount != 0xFF);
    
    if (0 == numUseCount)
    {
      /** If dbHeader.numUseCount reaches 0, add this block to freeQ */
      if (context) ret = xQueueSendFromISR(free_q, &pdata_block, NULL);
      else         ret = xQueueSend(free_q, &pdata_block, 0);
      configASSERT(pdTRUE == ret);
      }
    }
   
    return ;
}

void datablk_mgr_release(QAI_DataBlockMgr_t *pdatablk_mgr_handle, QAI_DataBlock_t *pdata_block)
{
    datablk_mgr_release_common(pdatablk_mgr_handle, pdata_block, 0);
}

void datablk_mgr_releaseFromISR(QAI_DataBlockMgr_t *pdatablk_mgr_handle, QAI_DataBlock_t *pdata_block)
{
    datablk_mgr_release_common(pdatablk_mgr_handle, pdata_block, 1);
}

QAI_DataBlockMgr_t * datablk_mgr_get_handle(QAI_DataBlock_t *pdata_block)
{
    QAI_DataBlockMgr_t *p_dbm = NULL;
    if (pdata_block) {
       // Get the datablock manager referencing into the gdbmArray[]
       p_dbm = gdbmArray[pdata_block->dbHeader.bufModeBits & 0x0F] ;

       // Validate if the datablock is in the address range
       // indicated by the datablock manager handle
       if (!datablk_mgr_validity_check(p_dbm, pdata_block))
         p_dbm = NULL;
    }
    return p_dbm;
}

void datablk_mgr_release_generic(QAI_DataBlock_t *pdata_block)
{
  QAI_DataBlockMgr_t *pdatablk_mgr_handle = datablk_mgr_get_handle(pdata_block);
  datablk_mgr_release_common(pdatablk_mgr_handle, pdata_block, 0);
}

void datablk_mgr_releaseFromISR_generic(QAI_DataBlock_t *pdata_block)
{
  QAI_DataBlockMgr_t *pdatablk_mgr_handle = datablk_mgr_get_handle(pdata_block);
  datablk_mgr_release_common(pdatablk_mgr_handle, pdata_block, 1);
}

int datablk_mgr_WriteDataBufferToQueues
    (outQ_processor_t *poutq_processor,
     QueueHandle_t     inQ,
     QAI_DataBlock_t  *pdata_block
    )
{
    size_t k = 0;

    int              datablk_pe_id = poutq_processor->in_pid;
    QueueHandle_t  **outQ_array = poutq_processor->outQ;
    size_t           outQ_array_len = poutq_processor->outQ_num;
    BaseType_t       ret;

    // update datablock header process id field
    if ((pdata_block) && outQ_array_len) {
        pdata_block->dbHeader.pid = datablk_pe_id;
    }

    taskENTER_CRITICAL();
    pdata_block -> dbHeader.numUseCount += outQ_array_len ;
    taskEXIT_CRITICAL();

    for (k = 0; k < outQ_array_len; k++) {

        if (inQ == *outQ_array[k]) {
           // when adding to input queue, send to the front of the queue
           ret = xQueueSendToFront(*outQ_array[k], (void *)&pdata_block, 0);
        } else {
           ret = xQueueSendToBack(*outQ_array[k], (void *)&pdata_block, 0);
        }
        if (pdTRUE != ret) {
           // send an error message
           QL_LOG_INFO("Insufficient Queue\n");
        }
    }
    return 0;
}

int datablk_mgr_WriteDataBufferToQueuesFromISR
    (outQ_processor_t *poutq_processor,
     QAI_DataBlock_t  *pdata_block
    )
{
    int   k = 0;
    int              datablk_pe_id = poutq_processor->in_pid;
    QueueHandle_t  **outQ_array = poutq_processor->outQ;
    int              outQ_array_len = poutq_processor->outQ_num;

    // update process id field
    if ((pdata_block) && outQ_array_len) {
        pdata_block->dbHeader.pid = datablk_pe_id;
    }

    pdata_block -> dbHeader.numUseCount += outQ_array_len;
    for (k = 0; k < outQ_array_len; k++) {
        // always send to back of the queue
        BaseType_t ret = xQueueSendToBackFromISR(*outQ_array[k], (void *)&pdata_block, NULL);
        configASSERT(ret == pdTRUE);

    }
    return 0;
}

int datablk_mgr_reset(QAI_DataBlockMgr_t *pdatablk_mgr_handle)
{
    QAI_DataBlock_t *pnext_item;
    int size_data_block ;
    int num_data_blocks ;
    int num_data_blocks_acquired;

/** Detailed description */  
    num_data_blocks_acquired = datablk_mgr_num_acquired(pdatablk_mgr_handle);
    // retrieve datablock size
    size_data_block = pdatablk_mgr_handle->size_data_block;

    // retrieve number of datablocks handled by this datablock manager    
    num_data_blocks = pdatablk_mgr_handle->max_data_blocks ;
    
    /* Enter critical section */    
    taskENTER_CRITICAL();

    // Reset dataBlockFreeQ datablock manager's free queue
    xQueueReset( pdatablk_mgr_handle->dataBlockFreeQ );

    /** Populate dataBlockFreeQ with available data blocks from pdata_blocks[] */

    uint8_t *pmem = pdatablk_mgr_handle->pmem_buf_start ;
    pnext_item = (QAI_DataBlock_t *)( ((uint32_t)pmem + 3) & ~0x03 ); 
    for (int k = 0; k < num_data_blocks; k++) {
      memset(&pnext_item->dbHeader, 0, sizeof(pnext_item->dbHeader));
      xQueueSend(pdatablk_mgr_handle->dataBlockFreeQ, (void *)&pnext_item, (TickType_t)0);
      pnext_item = (QAI_DataBlock_t *)((uint32_t)pnext_item + size_data_block);
    }
    taskEXIT_CRITICAL();
    /* Exit critical section */    

    /** return success code */
  
    return num_data_blocks_acquired;
}

int get_datablk_index(QAI_DataBlock_t *pdata_block)
{
  QAI_DataBlockMgr_t *pdatablk_mgr_handle = datablk_mgr_get_handle(pdata_block);
  uint8_t *buf_start    = pdatablk_mgr_handle->pmem_buf_start;
  uint8_t *block_start  = (uint8_t *)pdata_block;
  
  int diff = block_start - buf_start;
  
  int block_number = diff/pdatablk_mgr_handle->size_data_block;
  
  return block_number;
}

/** @} */
