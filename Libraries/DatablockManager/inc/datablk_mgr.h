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
 *    File   : datablk_mgr.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

/** @file datablk_mgr.h
 *
 * @brief Data block manager defines interfaces for creating data
 * blocks. This module manages use and re-use of these data
 * blocks differt tasks.
 *
 */

#ifndef __DATABLK_MGR_H__
#define __DATABLK_MGR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "Fw_global_config.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
/** @addtogroup DATABLK_MGR data block manager module
 *
 * This module provides interfaces for managing an array of data blocks
 * FreeRTOS Queue mechanism is used to populate and retrieve free data
 * blocks. \ref Fig1 "Figure 1" below depicts the association between
 * DataBlockFreeQ and the data blocks.
 *
 * A non-empty DataBlockFreeQ element points to an available data block
 * for use. Multiple tasks may need access to this same data block.
 * To enable such re-use, an numUseCount variable is added to the data 
 * block. This variable numUseCount will indicate if the data block is
 * in use. Only data blocks with an numUseCount of 0 will be added
 * to the DataBlockFreeQ queue.
 *
 * \anchor Fig1 \diafile Fig1.dia "Figure 1. DataBlockFreeQ and block association (Use case for Audio brick Management)" width=\textwidth
 *
 * \ref Fig2 "Figure 2" below depicts the API usage to acquire and release
 * data blocks using the APIs provided by this data block manager module
 *
 * \anchor Fig2 \diafile Fig2.dia "Figure 2. Datablock Manager API Usage" width=\textwidth
 *
 *  @{
 */

#define DATABLK_MGR_API_VERSION_MAJOR (0)      ///< API Version number Major
#define DATABLK_MGR_API_VERSION_MINOR (1)      ///< API Version number Minor

#define MAX_DBM_ARRAY_SIZE              (1 << 4)  ///< maximum number of DBM pools are limited to 16
#define DBM_HDR_BUFMODE_DBM_INDEX_MASK  (MAX_DBM_ARRAY_SIZE -1)   
#define DBM_HDR_BUFMODE_RESVD1_MASK     (1 << 4)  ///< reserved for future use
#define DBM_HDR_BUFMODE_RESVD2_MASK     (1 << 5)  ///< reserved for future use
#define DBM_HDR_BUFMODE_RESVD3_MASK     (1 << 6)  ///< reserved for future use
#define DBM_HDR_BUFMODE_SEGMENT_MASK    (1 << 7)  ///< segment mask
   
/** @brief Generic data block header    */
typedef struct st_QAI_DataBlockHeader {
    union {
      struct {
         uint8_t  numUseCount;       ///< reserved for data block queue management.
                  ///< user application should not modify this value
                  ///< 0 indicates data block is available for use
         uint8_t  numDropCount;     ///< reset to 0 when buffer is allocated, 
                  ///< otherwise, this count incremented when buffer is reused
                  ///< by the ISR. The count increment is limited to 255
         uint8_t  pid;             ///< ID of the processing element that filled this buffer
         uint8_t  numDataChannels;   ///< number of data channels, 
                  ///< example for audio, mono = 1, stereo = 2
                  ///< for accel xyz, numDataChannels = 3
         uint16_t numDataElements;   ///< actual number of data elements
         uint8_t  dataElementSize; ///< size in bytes of each data element
         uint8_t  bufModeBits;     ///< bit-field indicating buffer mode and DBM array index
                  ///< Bit7 = 0 => interleaved channel data elements, 
                  ///< Bit7 = 1 => segmented channel data elements
                  ///< Bits3:0 = Dbm Index
         uint32_t Tstart; ///< time stamp when the buffer was allocated by the PE or ISR (in mill-secs)
         uint32_t Tend;   ///< time stamp when the next buffer was allocated (in mill-secs)
       };
       uint32_t    block_header[4]; ///< for aligning this structure to 4-byte boundary
   };
} QAI_DataBlockHeader_t;

/** @brief Generic data block definition
 *         A placeholder, individual modules may use definitions specific to
 *         their use case and typecast to this placeholder structure when
 *         using the APIs from this module
 */
typedef struct st_QAI_DataBlock {
   QAI_DataBlockHeader_t dbHeader; ///< data block header
   uint8_t               p_data[]; ///< user defined data block, flexible array
} QAI_DataBlock_t ;

/** @brief Data block manager structure, holds state of data block manager
 */
typedef struct {
    QueueHandle_t    dataBlockFreeQ    ; ///< Handle to the FreeQ 
    int              max_data_blocks   ; ///< maximum number of data blocks handled by this queue
    size_t           size_data_block   ; ///< Size in bytes of each data block
    int              maxDataElements   ; ///< maximum number of samples assuming 1 channel only
    int              dataElementSize   ; ///< data element size in bytes
    int              refIndex          ; ///< Reference index into the global DBM array
    void            *pmem_buf_start    ; ///< Start address of the datablock memory pool
    void            *pmem_buf_end      ; ///< Start address of the last datablock in the pool
} QAI_DataBlockMgr_t;

extern QAI_DataBlockMgr_t *gdbmArray[MAX_DBM_ARRAY_SIZE]; ///< Global DBM Array holding pointers to Datablock manager handles

/** @brief Event notifier prototype. Processing elements will invoke this prototype 
 *         to indicate the event and the associated data.
 *
 *  @param[in] pid            processing element's ID that generated the event
 *  @param[in] event_type     event type specific to the processing element
 *  @param[in] p_event_data   pointer to event data
 *  @param[in] num_data_bytes number of bytes contained in event data
 *
 */
typedef void datablk_pe_event_notifier_t(int pid, int event_type, void *p_event_data, int num_data_bytes);

/** @brief processing element function prototype
 *
 *  @param[in] pIn   pointer to input block
 *  @param[in] pOut  pointer to intermediate output block
 *  @param[in] pRet  double pointer to data block containing output
 *  @param[in] p_event_notifier event notifier function for indicating error or events
 */
typedef void datablk_pe_process_func_t(
                QAI_DataBlock_t *pIn,
                QAI_DataBlock_t *pOut,
                QAI_DataBlock_t **pRet,
                datablk_pe_event_notifier_t *pevent_notifier
             );

/** @brief structure for outputting data buffer to multiple Qs */
typedef struct st_outQ_processor
{
  void (*process_func)(void) ; ///< ISR handler or process function to invoke
  QAI_DataBlockMgr_t *p_dbm ;  ///< datablock manager handle
  int in_pid;                  ///< input processing element's process ID
  int outQ_num;                ///< number of output queues
  QueueHandle_t  **outQ;       ///< pointer to an array of output queue handle pointers
  datablk_pe_event_notifier_t *p_event_notifier;
                               ///< event notifier function associated with the processing element
} outQ_processor_t ;

/** @brief 
 *
 * @return Returns 32-bit version info for this module.
 *       |            |                    |
 *       |  ---:      | ---:               |
 *       | Bit[31-24] | API major version  |
 *       | Bit[23-16] | API minor version  |
 *       | Bit[15- 8] | code major version |
 *       | Bit[ 7- 0] | code minor version |
 */
unsigned long datablk_mgr_get_version(void);

/** @brief data block manager initialization function
 *
 * Creates a dataBlockFreeQ free queue and populates with available data block pointers.
 *
 * @param[in,out] pdatablk_mgr_handle pointer QAI_DataBlockMgr_t. Newly created
 *                          queues will be populated here
 * @param[in] pmem pointer to memory for use as datablocks
 * @param[in] mem_size size of the memory block for use by datablock manager
 * @param[in] item_count number of maximum samples per datablock
 * @param[in] item_size_bytes size of each sample in bytes (2 for 16-bit samples)
 *
 * @return returns 0 on success, an error code otherwise
 */
int datablk_mgr_init(QAI_DataBlockMgr_t *pdatablk_mgr_handle, 
                     void *pmem, 
                     int mem_size, 
                     int item_count, 
                     int item_size_bytes);


/** @brief Get datablock manager handle associated with given datablock
 * 
 * @param[in] pdata_block pointer to data block
 *
 * @return datablock manager handle associated with the pdata_block
 */
QAI_DataBlockMgr_t * datablk_mgr_get_handle(QAI_DataBlock_t *pdata_block);

/** @brief Acquire a free data block from dataBlockFreeQ (with numUseCount=0)
 * 
 * @param[in] pdatablk_mgr_handle pointer to QAI_DataBlockMgr_t initialized
 *                                using datablk_mgr_init
 * @param[in,out] pdata_block pointer to an available data block will be
 *                                returned here
 * @param[in] xTicksToWait The maximum amount of time to wait blocking the task
 *                                to acquire an data block
 *
 * @return return 0 on success, an error code otherwise
 *
 */
int datablk_mgr_acquire(QAI_DataBlockMgr_t *pdatablk_mgr_handle, QAI_DataBlock_t **pdata_block, TickType_t xTicksToWait);

/** @brief Acquire a free data block from dataBlockFreeQ (with numUseCount=0).
 *         This is intended for calling from within an ISR
 * 
 * @param[in] pdatablk_mgr_handle pointer to QAI_DataBlockMgr_t initialized
 *                                using datablk_mgr_init
 * @param[in,out] pdata_block pointer to an available data block will be
 *                                returned here
 * @param[in] xTicksToWait The maximum amount of time to wait blocking the task
 *                                to acquire an data block
 *
 * @return return 0 on success, an error code otherwise
 *
 * @note function name TBD
 */
int datablk_mgr_acquireFromISR(QAI_DataBlockMgr_t *pdatablk_mgr_handle, QAI_DataBlock_t **pdata_block);

/** @brief Discard the data block, decrements numUseCount, pushes the data block
 * to data block free queue when numUseCount reaches 0
 *
 * @param[in] pdatablk_mgr_handle pointer to QAI_DataBlockMgr_t initialized
 *                                using datablk_mgr_init
 * @param[in] pdata_block pointer to data block to be discarded
 *
 */
void datablk_mgr_release(QAI_DataBlockMgr_t *pdatablk_mgr_handle, QAI_DataBlock_t *pdata_block);

/** @brief Discard the data block, decrements numUseCount, pushes the data block
 * to data block free queue when numUseCount reaches 0
 *         This is intended for calling from within an ISR
 *
 * @param[in] pdatablk_mgr_handle pointer to QAI_DataBlockMgr_t initialized
 *                                using datablk_mgr_init
 * @param[in] pdata_block pointer to data block to be discarded
 *
 */
void datablk_mgr_releaseFromISR(QAI_DataBlockMgr_t *pdatablk_mgr_handle, QAI_DataBlock_t *pdata_block);

/** @brief Retrieve the datablock manager handle associated with the
 *   given datablock and discard the data block, decrements numUseCount, 
 *   pushes the data block to data block free queue when numUseCount reaches 0
 *
 * @param[in] pdata_block pointer to data block to be discarded
 *
 */
void datablk_mgr_release_generic(QAI_DataBlock_t *pdata_block);

/** @brief Retrieve the datablock manager handle associated with the
 *   given datablock and discard the data block, decrements numUseCount, 
 *   pushes the data block to data block free queue when numUseCount reaches 0
 *         This is intended for calling from within an ISR
 *
 * @param[in] pdata_block pointer to data block to be discarded
 *
 */
void datablk_mgr_releaseFromISR_generic(QAI_DataBlock_t *pdata_block);

/** @brief Get number of data blocks that are acquired so far
 *  
 * @param[in] pdatablk_mgr_handle pointer to QAI_DataBlockMgr_t initialized
 *                                using datablk_mgr_init
 *
 * @return number of data blocks that have been acquired
 */
int datablk_mgr_num_acquired(QAI_DataBlockMgr_t *pdatablk_mgr_handle);

/** @brief Get number of data blocks that are available for acquiring 
 *  
 * @param[in] pdatablk_mgr_handle pointer to QAI_DataBlockMgr_t initialized
 *                                using datablk_mgr_init
 *
 * @return number of data blocks available for acquiring
 */
int datablk_mgr_num_available(QAI_DataBlockMgr_t *pdatablk_mgr_handle);

/** @brief Write data block to Output Queues
 *
 * @param[in] poutq_processor pointer to output Queue processor structure
 *  @param[in] pdata_block  pointer to data block.
 *                          This data block is written to output queues
 *
 *  @return 0 on success, an error code otherwise
 */
int datablk_mgr_WriteDataBufferToQueues
          (outQ_processor_t *poutq_processor,
           QueueHandle_t     inQ,
           QAI_DataBlock_t  *pdata_block
          );

/** @brief Write data block to Output Queues from an ISR
 *
 *  @param[in] poutq_processor pointer to output Queue processor structure
 *  @param[in] pdata_block  pointer to data block.
 *                          This data block is written to output queues
 *
 *  @return 0 on success, an error code otherwise
 */
int datablk_mgr_WriteDataBufferToQueuesFromISR
          (outQ_processor_t *poutq_processor,
           QAI_DataBlock_t  *pdata_block
          );

/** @brief Datablock validity check
 *
 *  @param[in] pdata_block pointer to output Queue processor structure
 *  @param[in] pdbm        datablock manger handle
 *
 *  @return 1 if valid datablock, 0 otherwise
 */
int datablk_mgr_validity_check
          (QAI_DataBlockMgr_t *pdatablk_mgr_handle,
           QAI_DataBlock_t *pdata_block
          );

/** @brief Update datablock use count
 *
 *  @param[in] pdata_block pointer to output Queue processor structure
 *  @param[in] icount      increment the use count by "icount"
 *
 *  @return update use count for the datablock
 */
uint8_t datablk_mgr_usecount_increment
          (QAI_DataBlock_t *pdata_block, 
           uint8_t icount
          );

/** @brief Update datablock use count
 *
 *  @param[in] pdata_block pointer to output Queue processor structure
 *  @param[in] icount      increment the use count by "icount"
 *
 *  @return update use count for the datablock
 */
uint8_t datablk_mgr_usecount_incrementFromISR
          (QAI_DataBlock_t *pdata_block, 
           uint8_t icount
          );

/** @brief Reset data block manager, release all datablocks that are in use.
 *
 * @param[in] pdatablk_mgr_handle pointer to QAI_DataBlockMgr_t initialized
 *                                using datablk_mgr_init
 */
int datablk_mgr_reset(QAI_DataBlockMgr_t *pdatablk_mgr_handle);

/** @brief Get the number (in sequence) of the current datablock in the buffer
 *
 * @param[in] pdata_block pointer to data block whose number is needed
 *
 */
int get_datablk_index(QAI_DataBlock_t *pdata_block);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DATABLK_MGR_H__ */
