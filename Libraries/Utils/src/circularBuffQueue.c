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

/*! \file circularBuffQueue.c
 *
 *  \brief This file contains circular buffer utility APIs.
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "test_types.h"
#include "Fw_global_config.h"
#include "FreeRTOS.h"
#include "RtosTask.h"

#include "circularBuffQueue.h"

/*!
* \fn      void initCircularBuff(circularBuffQueue *node, UINT8_t *pBuffPtr, int chunkSize, int maxElements)
* \brief   API to intilaize circular buffer queue data structure
* \param   node     - pointer to circular queue structure
* \param   pBuffPtr - pointer to buffer
* \param   buffLen - length of the buffer
* \param   buffLen - chunk size
*/
void initCircularBuff(circularBuffQueue *node, UINT8_t *pBuffPtr, size_t buffLen, int chunkSize)
{
	node->bufferNodeArr = (bufferNode *)malloc(sizeof(bufferNode));
	configASSERT(node->bufferNodeArr);

    int maxElements = buffLen/chunkSize;
	for (int eleCnt = 0; eleCnt < maxElements; eleCnt++)
	{
		node->bufferNodeArr[eleCnt].pDataBuffPtr = pBuffPtr + eleCnt * chunkSize;
		node->bufferNodeArr[eleCnt].status = EMPTY;
	}
	node->readPtr = 0;
	node->writePtr = 0;
	node->maxElements = maxElements;
}

/*!
* \fn      UINT8_t *getNextWritePtr(circularBuffQueue *node)
* \brief   API to get the buffer pointer for the next empty buffer
* \param   node     - pointer to circular queue structure
* \return  pointer to empty buffer if there is any else NULL
*/
UINT8_t *getNextWritePtr(circularBuffQueue *node)
{
	// check if the next write buffer is available/free
	if (node->bufferNodeArr[node->writePtr].status != EMPTY)
	{
        // Just for testing added
        printQueue(node);
        // just for testing ends
        
		printf("\n Buffer overflow");
        
        //while(1);
		return NULL;
	}
	// get the buffer pointer
	unsigned char* buffPtr = node->bufferNodeArr[node->writePtr].pDataBuffPtr;

	//Mark the staus full
	node->bufferNodeArr[node->writePtr].status = FULL;

	// increment the pointer
	node->writePtr++;
	// take care of wrap around case
	if (node->writePtr >= node->maxElements)
	{
		node->writePtr -= node->maxElements;
	}

	// return the buffer pointer
	return buffPtr;
}

/*!
* \fn      UINT8_t *getReadBuffPtr(circularBuffQueue *node)
* \brief   API to get the buffer pointer for the full buffer for data read
* \param   node     - pointer to circular queue structure
* \return  pointer to full buffer if there is any else NULL
*/
UINT8_t *getReadBuffPtr(circularBuffQueue *node)
{
	// check if the next write buffer is available/free
	if (node->bufferNodeArr[node->readPtr].status != FULL)
	{
		// Something wrong either biffer underflow or same api is called without freeing the buffer
		printf("\n Buffer underflow/buffer free issue buffer status = %d ", node->bufferNodeArr[node->readPtr].status);
		return NULL;
	}

	// Mark the staus to be busy
	// Note if the same API is called second time without freeing the buffer, same buffer will be returned
	node->bufferNodeArr[node->readPtr].status = BUSY;

	// return the buffer pointer
	return node->bufferNodeArr[node->readPtr].pDataBuffPtr;
}

/*!
* \fn      freeReadBuffPtr(circularBuffQueue *node, unsigned char* pBuffPtr)
* \brief   API to get free up full buffer from the list of full buffer
* \        This APt suppose to be succedded by call to getReadBuffPtrcall() API
* \        after data has been processed by the calling task/API. 
* \param   node     - pointer to circular queue structure
* \param   pBuffPtr - pointer to the buffer queue to be free
* \return  None
*/
void freeReadBuffPtr(circularBuffQueue *node, unsigned char* pBuffPtr)
{
	bool found = false;
	// Find the buffer index in the buffer pool 
	for (int ptr = 0; ptr < node->maxElements; ptr++)
	{
		if (node->bufferNodeArr[ptr].pDataBuffPtr == pBuffPtr)
		{
			// Read pointer and pointer index should match
			if (node->readPtr != ptr)
				configASSERT(0);
			found = true;
			break;
		}
	}
	if (found == false)
	{
		printf("\nError pointer not found");
		return;
	}

	// mark the pointer status EMPTY
	node->bufferNodeArr[node->readPtr].status = EMPTY;
    
    printf("\n ####%x", pBuffPtr);

	// increment the pointer
	node->readPtr++;
	if (node->readPtr >= node->maxElements)
	{
		node->readPtr -= node->maxElements;
	}
}

/*!
* \fn      printQueue(circularBuffQueue *node)
* \brief   This API is for debugg purpose only. Print starting address of the 
* \        all buffer queue and their status empty(0)/full(1) 
* \param   node     - pointer to circular queue structure
* \return  None
*/
void printQueue(circularBuffQueue *node)
{
	for (int ptr = 0; ptr < node->maxElements; ptr++)
	{
		printf("\n elementCnt =%d,	 dataptr =%x,	 status =%d", ptr, (unsigned int)node->bufferNodeArr[ptr].pDataBuffPtr, node->bufferNodeArr[ptr].status);
	}
	printf("\n WritePtr =%d,	 ReadPtr=%d\n", node->writePtr, node->readPtr);
}
