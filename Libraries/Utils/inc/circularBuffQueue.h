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
 *    File   : circularBuffQueue.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __CIRCULAR_QUEUE_H_
#define __CIRCULAR_QUEUE_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef enum
{
    EMPTY = 0,
    FULL = 1,
    BUSY = 2
}BUFF_STATUS;

// Structure to define buffer node in the queue
typedef struct
{
	UINT8_t * pDataBuffPtr; // pointer to buffer
	int status;                  // status whether it is busy or free
} bufferNode;

typedef struct 
{
	int readPtr;       // pointer to read buffer
	int writePtr;      // pointer to write buffer
	int maxElements;   // maximum nu ber of elements in the buffer
	bufferNode	*bufferNodeArr;
}circularBuffQueue;

extern void initCircularBuff(circularBuffQueue *node, UINT8_t *pBuffPtr, size_t buffLen, int chunkSize);
extern UINT8_t* getNextWritePtr(circularBuffQueue *node);
extern UINT8_t* getReadBuffPtr(circularBuffQueue *node);
extern void freeReadBuffPtr(circularBuffQueue *node, unsigned char* pBuffPtr);
extern void printQueue(circularBuffQueue *node);

#endif //__CIRCULAR_QUEUE_H_
