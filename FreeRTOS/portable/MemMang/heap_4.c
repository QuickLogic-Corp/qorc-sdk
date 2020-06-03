/*
 * FreeRTOS Kernel V10.1.1
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/*
 * A sample implementation of pvPortMalloc() and vPortFree() that combines
 * (coalescences) adjacent memory blocks as they are freed, and in so doing
 * limits memory fragmentation.
 *
 * See heap_1.c, heap_2.c and heap_3.c for alternative implementations, and the
 * memory management pages of http://www.FreeRTOS.org for more information.
 */
#include <stdlib.h>
#include <stdio.h>

#if( configHEAP4_DEBUG == 1 )
/*if need full heap debug checking information, enable PRINT_HEAP_DEBUG_MORE */
/*#define PRINT_HEAP_DEBUG_MORE 1 */
#endif

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "FreeRTOS.h"
#include "task.h"

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#if( configSUPPORT_DYNAMIC_ALLOCATION == 0 )
	#error This file must not be used if configSUPPORT_DYNAMIC_ALLOCATION is 0
#endif

/* Block sizes must not get too small. */
#define heapMINIMUM_BLOCK_SIZE	( ( size_t ) ( xHeapStructSize << 1 ) )

/* Assumes 8bit bytes! */
#define heapBITS_PER_BYTE		( ( size_t ) 8 )

/* Allocate the memory for the heap. */
#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
	/* The application writer has already defined the array used for the RTOS
	heap - probably so it can be placed in a special segment or address. */
	extern uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#else
	static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif /* configAPPLICATION_ALLOCATED_HEAP */

/* Add information to detect heap4 errors
    BlockLint_t->pxNextFreeBlock = 
     [ OVERFLOW_DETECTION (12 bit) | CALLER (20 bit)          ]  - Allocated block 
     [ OVERFLOW_DETECTION (12 bit) | pxNextFreeBlock (20 bit) ]  - Freed block
    BlockLint_t->xBlockSize = 
     [ A |  TASK_NUM  (11 bit)     | SIZE (20 bit)            ]  - Allocated block 
     [ A |  should be 0  (11 bit)  | SIZE (20 bit)            ]  - Freed block */
#if( configHEAP4_DEBUG == 1 )
	#ifndef SRAM_START
		#define SRAM_START 0x20000000
	#endif
	#define HEAP4_OVERFLOW_DETECTION ( 0xABC)
	#define HEAP4_OVERFLOW_OFFSET	( ( size_t ) 20 )
	#define HEAP4_TASK_NUM_OFFSET	( ( size_t ) 20 )
	#define HEAP4_CALLER_MASK ( 0xFFFFF )
	#define HEAP4_NEXT_FREEBLOCK_MASK ( 0xFFFFF )
	#define HEAP4_TASK_NUM_MASK ( 0xFFF )
	#define HEAP4_SIZE_MASK ( 0xFFFFF )
#endif

/* Define the linked list structure.  This is used to link free blocks in order
of their memory address. */
typedef struct A_BLOCK_LINK
{
	struct A_BLOCK_LINK *pxNextFreeBlock;	/*<< The next free block in the list. */
	size_t xBlockSize;						/*<< The size of the free block. */
} BlockLink_t;

/*-----------------------------------------------------------*/

/*
 * Inserts a block of memory that is being freed into the correct position in
 * the list of free memory blocks.  The block being freed will be merged with
 * the block in front it and/or the block behind it if the memory blocks are
 * adjacent to each other.
 */
static void prvInsertBlockIntoFreeList( BlockLink_t *pxBlockToInsert );

/*
 * Called automatically to setup the required heap structures the first time
 * pvPortMalloc() is called.
 */
static void prvHeapInit( void );

/*-----------------------------------------------------------*/

/* The size of the structure placed at the beginning of each allocated memory
block must by correctly byte aligned. */
static const size_t xHeapStructSize	= ( sizeof( BlockLink_t ) + ( ( size_t ) ( portBYTE_ALIGNMENT - 1 ) ) ) & ~( ( size_t ) portBYTE_ALIGNMENT_MASK );

/* Create a couple of list links to mark the start and end of the list. */
static BlockLink_t xStart, *pxEnd = NULL;

/* Keeps track of the number of free bytes remaining, but says nothing about
fragmentation. */
static size_t xFreeBytesRemaining = 0U;
static size_t xMinimumEverFreeBytesRemaining = 0U;

/* Gets set to the top bit of an size_t type.  When this bit in the xBlockSize
member of an BlockLink_t structure is set then the block belongs to the
application.  When the bit is free the block is still part of the free heap
space. */
static size_t xBlockAllocatedBit = 0;

/*-----------------------------------------------------------*/

void *pvPortMalloc( size_t xWantedSize )
{
BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
void *pvReturn = NULL;
#if( configHEAP4_DEBUG == 1 )
long * volatile lr_address = NULL;
	__asm volatile
	(
	     "mov %0, LR\n\t" : "=r"(lr_address)
	);
#endif

	vTaskSuspendAll();
	{
		/* If this is the first call to malloc then the heap will require
		initialisation to setup the list of free blocks. */
		if( pxEnd == NULL )
		{
			prvHeapInit();
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}

		/* Check the requested block size is not so large that the top bit is
		set.  The top bit of the block size member of the BlockLink_t structure
		is used to determine who owns the block - the application or the
		kernel, so it must be free. */
		if( ( xWantedSize & xBlockAllocatedBit ) == 0 )
		{
			/* The wanted size is increased so it can contain a BlockLink_t
			structure in addition to the requested amount of bytes. */
			if( xWantedSize > 0 )
			{
				xWantedSize += xHeapStructSize;

				/* Ensure that blocks are always aligned to the required number
				of bytes. */
				if( ( xWantedSize & portBYTE_ALIGNMENT_MASK ) != 0x00 )
				{
					/* Byte alignment required. */
					xWantedSize += ( portBYTE_ALIGNMENT - ( xWantedSize & portBYTE_ALIGNMENT_MASK ) );
					configASSERT( ( xWantedSize & portBYTE_ALIGNMENT_MASK ) == 0 );
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}

			if( ( xWantedSize > 0 ) && ( xWantedSize <= xFreeBytesRemaining ) )
			{
				/* Traverse the list from the start	(lowest address) block until
				one	of adequate size is found. */
				pxPreviousBlock = &xStart;
#if( configHEAP4_DEBUG == 1 )
				pxBlock = ( BlockLink_t * ) ( ( ( ( size_t ) xStart.pxNextFreeBlock ) & HEAP4_NEXT_FREEBLOCK_MASK ) + SRAM_START );
				while( ( (pxBlock->xBlockSize & HEAP4_SIZE_MASK) < xWantedSize ) && ( pxBlock->pxNextFreeBlock != NULL ) )
				{
					pxPreviousBlock = pxBlock;
					pxBlock = ( BlockLink_t * ) ( ( ( ( size_t ) pxBlock->pxNextFreeBlock ) & HEAP4_NEXT_FREEBLOCK_MASK ) + SRAM_START );
				}

				/* If the end marker was reached then a block of adequate size
				was	not found. */
				if( pxBlock != pxEnd )
				{
					/* Return the memory space pointed to - jumping over the
					BlockLink_t structure at its start. */
					pvReturn = ( void * ) ( ( ( uint8_t * ) ( ( ( size_t ) pxPreviousBlock->pxNextFreeBlock ) & HEAP4_NEXT_FREEBLOCK_MASK) + SRAM_START ) + xHeapStructSize );

					/* This block is being returned for use so must be taken out
					of the list of free blocks. */
					pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

					/* If the block is larger than required it can be split into
					two. */
					if( ( ( pxBlock->xBlockSize & HEAP4_SIZE_MASK ) - xWantedSize ) > heapMINIMUM_BLOCK_SIZE )
					{
						/* This block is to be split into two.  Create a new
						block following the number of bytes requested. The void
						cast is used to prevent byte alignment warnings from the
						compiler. */
						pxNewBlockLink = ( void * ) ( ( ( uint8_t * ) pxBlock ) + xWantedSize );
						configASSERT( ( ( ( size_t ) pxNewBlockLink ) & portBYTE_ALIGNMENT_MASK ) == 0 );

						/* Calculate the sizes of two blocks split from the
						single block. */
						pxNewBlockLink->xBlockSize = ( pxBlock->xBlockSize & HEAP4_SIZE_MASK ) - xWantedSize;
						pxBlock->xBlockSize = xWantedSize;

						/* Insert the new block into the list of free blocks. */
						prvInsertBlockIntoFreeList( ( pxNewBlockLink ) );
					}
#else
				pxBlock = xStart.pxNextFreeBlock;
				while( ( pxBlock->xBlockSize < xWantedSize ) && ( pxBlock->pxNextFreeBlock != NULL ) )
				{
					pxPreviousBlock = pxBlock;
					pxBlock = pxBlock->pxNextFreeBlock;
				}

				/* If the end marker was reached then a block of adequate size
				was	not found. */
				if( pxBlock != pxEnd )
				{
					/* Return the memory space pointed to - jumping over the
					BlockLink_t structure at its start. */
					pvReturn = ( void * ) ( ( ( uint8_t * ) pxPreviousBlock->pxNextFreeBlock ) + xHeapStructSize );

					/* This block is being returned for use so must be taken out
					of the list of free blocks. */
					pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

					/* If the block is larger than required it can be split into
					two. */
					if( ( pxBlock->xBlockSize - xWantedSize ) > heapMINIMUM_BLOCK_SIZE )
					{
						/* This block is to be split into two.  Create a new
						block following the number of bytes requested. The void
						cast is used to prevent byte alignment warnings from the
						compiler. */
						pxNewBlockLink = ( void * ) ( ( ( uint8_t * ) pxBlock ) + xWantedSize );
						configASSERT( ( ( ( size_t ) pxNewBlockLink ) & portBYTE_ALIGNMENT_MASK ) == 0 );

						/* Calculate the sizes of two blocks split from the
						single block. */
						pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
						pxBlock->xBlockSize = xWantedSize;

						/* Insert the new block into the list of free blocks. */
						prvInsertBlockIntoFreeList( pxNewBlockLink );
					}
#endif
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}

					xFreeBytesRemaining -= pxBlock->xBlockSize;

					if( xFreeBytesRemaining < xMinimumEverFreeBytesRemaining )
					{
						xMinimumEverFreeBytesRemaining = xFreeBytesRemaining;
					}
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}

					/* The block is being returned - it is allocated and owned
					by the application and has no "next" block. */
#if( configHEAP4_DEBUG == 1 )
					/* Add TCBNumber, OVERFLOW_DETECTION Magic Value and Caller information */
					pxBlock->xBlockSize |= xBlockAllocatedBit;
					pxBlock->xBlockSize |= ( ( uxTaskGetTCBNumber( xTaskGetCurrentTaskHandle() ) ) & HEAP4_TASK_NUM_MASK ) << HEAP4_TASK_NUM_OFFSET;
					pxBlock->pxNextFreeBlock = ( struct A_BLOCK_LINK * ) ( ( ( size_t ) ( HEAP4_OVERFLOW_DETECTION << HEAP4_OVERFLOW_OFFSET ) ) | ( ( size_t ) lr_address & HEAP4_CALLER_MASK ) );
#else
					pxBlock->xBlockSize |= xBlockAllocatedBit;
					pxBlock->pxNextFreeBlock = NULL;
#endif
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}

		traceMALLOC( pvReturn, xWantedSize );
	}
	( void ) xTaskResumeAll();

	#if( configUSE_MALLOC_FAILED_HOOK == 1 )
	{
		if( pvReturn == NULL )
		{
			extern void vApplicationMallocFailedHook( void );
			vApplicationMallocFailedHook();
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
	#endif

	configASSERT( ( ( ( size_t ) pvReturn ) & ( size_t ) portBYTE_ALIGNMENT_MASK ) == 0 );
	return pvReturn;
}
/*-----------------------------------------------------------*/

void vPortFree( void *pv )
{
uint8_t *puc = ( uint8_t * ) pv;
BlockLink_t *pxLink;

	if( pv != NULL )
	{
		/* The memory being freed will have an BlockLink_t structure immediately
		before it. */
		puc -= xHeapStructSize;

		/* This casting is to keep the compiler from issuing warnings. */
		pxLink = ( void * ) puc;

		/* Check the block is actually allocated. */
		configASSERT( ( pxLink->xBlockSize & xBlockAllocatedBit ) != 0 );
#ifndef configHEAP4_DEBUG
		configASSERT( pxLink->pxNextFreeBlock == NULL );
#endif

		if( ( pxLink->xBlockSize & xBlockAllocatedBit ) != 0 )
		{
#if( configHEAP4_DEBUG == 1 )
			/* The block is being returned to the heap - it is no longer
			allocated. */
			pxLink->xBlockSize &= ~xBlockAllocatedBit;
			pxLink->xBlockSize &= ~( HEAP4_TASK_NUM_MASK << HEAP4_TASK_NUM_OFFSET );

			/* If the next block is not pxEnd and if this block breaked next block */
			if( ( ( size_t ) pxLink + ( pxLink->xBlockSize ) != ( size_t ) pxEnd ) && ( ( ( size_t ) ( ( ( struct A_BLOCK_LINK * ) ( ( size_t ) pxLink + ( pxLink->xBlockSize ) ) )->pxNextFreeBlock ) >> HEAP4_OVERFLOW_OFFSET ) != HEAP4_OVERFLOW_DETECTION ) )
			{
				printf("The Next Heap 4 block corrupted by this block.[%8x]\n", (void*) pxLink );
			}
			vTaskSuspendAll();
			{
				/* Add this block to the list of free blocks. */
				xFreeBytesRemaining += pxLink->xBlockSize;
				traceFREE( pv, pxLink->xBlockSize );
				prvInsertBlockIntoFreeList( ( ( BlockLink_t * ) pxLink ) );
			}
			( void ) xTaskResumeAll();

#else
			if( pxLink->pxNextFreeBlock == NULL )
			{
				/* The block is being returned to the heap - it is no longer
				allocated. */
				pxLink->xBlockSize &= ~xBlockAllocatedBit;

				vTaskSuspendAll();
				{
					/* Add this block to the list of free blocks. */
					xFreeBytesRemaining += pxLink->xBlockSize;
					traceFREE( pv, pxLink->xBlockSize );
					prvInsertBlockIntoFreeList( ( ( BlockLink_t * ) pxLink ) );
				}
				( void ) xTaskResumeAll();
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
#endif
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
}
/*-----------------------------------------------------------*/

size_t xPortGetFreeHeapSize( void )
{
	return xFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

size_t xPortGetMinimumEverFreeHeapSize( void )
{
	return xMinimumEverFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

void vPortInitialiseBlocks( void )
{
	/* This just exists to keep the linker quiet. */
}
/*-----------------------------------------------------------*/

static void prvHeapInit( void )
{
BlockLink_t *pxFirstFreeBlock;
uint8_t *pucAlignedHeap;
size_t uxAddress;
size_t xTotalHeapSize = configTOTAL_HEAP_SIZE;

	/* Ensure the heap starts on a correctly aligned boundary. */
	uxAddress = ( size_t ) ucHeap;

	if( ( uxAddress & portBYTE_ALIGNMENT_MASK ) != 0 )
	{
		uxAddress += ( portBYTE_ALIGNMENT - 1 );
		uxAddress &= ~( ( size_t ) portBYTE_ALIGNMENT_MASK );
		xTotalHeapSize -= uxAddress - ( size_t ) ucHeap;
	}

	pucAlignedHeap = ( uint8_t * ) uxAddress;

	/* xStart is used to hold a pointer to the first item in the list of free
	blocks.  The void cast is used to prevent compiler warnings. */
#if( configHEAP4_DEBUG == 1 )
          xStart.pxNextFreeBlock = ( struct A_BLOCK_LINK * ) ( ( size_t )pucAlignedHeap | (size_t) ( HEAP4_OVERFLOW_DETECTION << HEAP4_OVERFLOW_OFFSET ) );
#else
          xStart.pxNextFreeBlock = ( void * ) pucAlignedHeap;
#endif
	xStart.xBlockSize = ( size_t ) 0;

	/* pxEnd is used to mark the end of the list of free blocks and is inserted
	at the end of the heap space. */
	uxAddress = ( ( size_t ) pucAlignedHeap ) + xTotalHeapSize;
	uxAddress -= xHeapStructSize;
	uxAddress &= ~( ( size_t ) portBYTE_ALIGNMENT_MASK );
	pxEnd = ( void * ) uxAddress;
	pxEnd->xBlockSize = 0;
	pxEnd->pxNextFreeBlock = NULL;

	/* To start with there is a single free block that is sized to take up the
	entire heap space, minus the space taken by pxEnd. */
	pxFirstFreeBlock = ( void * ) pucAlignedHeap;
	pxFirstFreeBlock->xBlockSize = uxAddress - ( size_t ) pxFirstFreeBlock;
#if( configHEAP4_DEBUG == 1 )
	pxFirstFreeBlock->pxNextFreeBlock = ( struct A_BLOCK_LINK * ) ( ( size_t )pxEnd | (size_t) ( HEAP4_OVERFLOW_DETECTION << HEAP4_OVERFLOW_OFFSET ) );
#else
	pxFirstFreeBlock->pxNextFreeBlock = pxEnd;
#endif

	/* Only one block exists - and it covers the entire usable heap space. */
	xMinimumEverFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
	xFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;

	/* Work out the position of the top bit in a size_t variable. */
	xBlockAllocatedBit = ( ( size_t ) 1 ) << ( ( sizeof( size_t ) * heapBITS_PER_BYTE ) - 1 );
}
/*-----------------------------------------------------------*/

static void prvInsertBlockIntoFreeList( BlockLink_t *pxBlockToInsert )
{
BlockLink_t *pxIterator;
uint8_t *puc;

	/* Iterate through the list until a block is found that has a higher address
	than the block being inserted. */
#if( configHEAP4_DEBUG == 1 )
	for( pxIterator = &xStart; ( BlockLink_t* ) ( ( ( ( size_t ) pxIterator->pxNextFreeBlock ) & HEAP4_NEXT_FREEBLOCK_MASK ) + SRAM_START ) < pxBlockToInsert; pxIterator = ( BlockLink_t * ) ( ( ( (size_t)  pxIterator->pxNextFreeBlock ) & HEAP4_NEXT_FREEBLOCK_MASK ) + SRAM_START ) )
	{
		/* Nothing to do here, just iterate to the right position. */
	}
#else
	for( pxIterator = &xStart; pxIterator->pxNextFreeBlock < pxBlockToInsert; pxIterator = pxIterator->pxNextFreeBlock )
	{
		/* Nothing to do here, just iterate to the right position. */
	}
#endif

	/* Do the block being inserted, and the block it is being inserted after
	make a contiguous block of memory? */
	puc = ( uint8_t * ) pxIterator;
#if( configHEAP4_DEBUG == 1 )
	if( ( puc + ( pxIterator->xBlockSize & HEAP4_SIZE_MASK ) ) == ( uint8_t * ) pxBlockToInsert )
	{
		pxIterator->xBlockSize = ( pxIterator->xBlockSize & HEAP4_SIZE_MASK ) + ( pxBlockToInsert->xBlockSize & HEAP4_SIZE_MASK);
		pxBlockToInsert = pxIterator;
	}
#else
	if( ( puc + pxIterator->xBlockSize ) == ( uint8_t * ) pxBlockToInsert )
	{
		pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
		pxBlockToInsert = pxIterator;
	}
#endif
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}

	/* Do the block being inserted, and the block it is being inserted before
	make a contiguous block of memory? */
	puc = ( uint8_t * ) pxBlockToInsert;
#if( configHEAP4_DEBUG == 1 )
	if( ( puc + ( pxBlockToInsert->xBlockSize & HEAP4_SIZE_MASK ) ) == ( uint8_t * ) ( ( ( size_t ) pxIterator->pxNextFreeBlock ) & HEAP4_NEXT_FREEBLOCK_MASK ) + SRAM_START )
	{
		if( pxIterator->pxNextFreeBlock != ( struct A_BLOCK_LINK * ) ( ( size_t )pxEnd | (size_t) ( HEAP4_OVERFLOW_DETECTION << HEAP4_OVERFLOW_OFFSET ) ) )
		{
			/* Form one big block from the two blocks. */
			pxBlockToInsert->xBlockSize = ( ( ( size_t ) pxBlockToInsert->xBlockSize ) & HEAP4_SIZE_MASK ) + ( ( ( struct A_BLOCK_LINK * ) ( ( ( size_t ) pxIterator->pxNextFreeBlock & HEAP4_NEXT_FREEBLOCK_MASK ) + SRAM_START ) )->xBlockSize ) & HEAP4_SIZE_MASK;
			pxBlockToInsert->pxNextFreeBlock = ( ( struct A_BLOCK_LINK * ) ( ( ( size_t ) pxIterator->pxNextFreeBlock ) & HEAP4_NEXT_FREEBLOCK_MASK ) + SRAM_START )->pxNextFreeBlock;
		}
		else
		{
			pxBlockToInsert->pxNextFreeBlock = ( struct A_BLOCK_LINK * ) ( ( size_t )pxEnd | (size_t) ( HEAP4_OVERFLOW_DETECTION << HEAP4_OVERFLOW_OFFSET ) );
		}
	}
#else
	if( ( puc + pxBlockToInsert->xBlockSize ) == ( uint8_t * ) pxIterator->pxNextFreeBlock )
	{
		if( pxIterator->pxNextFreeBlock != pxEnd )
		{
			/* Form one big block from the two blocks. */
			pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
			pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
		}
		else
		{
			pxBlockToInsert->pxNextFreeBlock = pxEnd;
		}
	}
#endif
	else
	{
		pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
	}

	/* If the block being inserted plugged a gab, so was merged with the block
	before and the block after, then it's pxNextFreeBlock pointer will have
	already been set, and should not be set here as that would make it point
	to itself. */
	if( pxIterator != pxBlockToInsert )
	{
#if( configHEAP4_DEBUG == 1 )
		pxIterator->pxNextFreeBlock = ( struct A_BLOCK_LINK * ) ( ( size_t ) pxBlockToInsert | ( size_t ) ( HEAP4_OVERFLOW_DETECTION << HEAP4_OVERFLOW_OFFSET ) );
#else
		pxIterator->pxNextFreeBlock = pxBlockToInsert;
#endif
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}
}

#if( configHEAP4_DEBUG == 1 )
void printHeapSize( void ) 
{
	printf("Free / MinEverFree / Total = ( %d / %d / %d byte )\r\n", 
		xPortGetFreeHeapSize() ,xPortGetMinimumEverFreeHeapSize(), sizeof(ucHeap) );
	return;
}

void printFreeList( void )
{
BlockLink_t *pxBlock;
unsigned int p = 1;
    (void)(p);
	printf("[FreeBlock List] \r\n");

	pxBlock = ( BlockLink_t * ) ( ( ( ( size_t ) xStart.pxNextFreeBlock ) & HEAP4_NEXT_FREEBLOCK_MASK ) + SRAM_START );
	while( pxBlock->pxNextFreeBlock != NULL )
	{
		pxBlock = ( BlockLink_t * ) ( ( ( ( size_t ) pxBlock->pxNextFreeBlock ) & HEAP4_NEXT_FREEBLOCK_MASK ) + SRAM_START );
		printf ("  (%d). [0x%p]  Size : %d byte \r\n", p++, pxBlock, pxBlock->xBlockSize);
	}
}

void printAllocList( void )
{
size_t uxAddress;
BlockLink_t *pxBlock;
unsigned int p = 1;
    (void)(p);
	/* Ensure the heap starts on a correctly aligned boundary. */
	uxAddress = ( size_t ) ucHeap;

	if( ( uxAddress & portBYTE_ALIGNMENT_MASK ) != 0 )
	{
		uxAddress += ( portBYTE_ALIGNMENT - 1 );
		uxAddress &= ~( ( size_t ) portBYTE_ALIGNMENT_MASK );
	}

	pxBlock = ( BlockLink_t * ) ( uxAddress );

	printf("[AllocatedBlock List] \r\n");
	while( pxBlock->pxNextFreeBlock != NULL )
	{
		if( pxBlock->xBlockSize & xBlockAllocatedBit )
		{
			printf ("  (%d). [0x%p]  Size : %d byte, Caller = [0x%p] \r\n",p++ ,pxBlock ,( (size_t) pxBlock->xBlockSize ) & HEAP4_SIZE_MASK, ( void * ) ( ( ( ( size_t ) pxBlock->pxNextFreeBlock ) & HEAP4_CALLER_MASK ) + SRAM_START ) );
		}
		pxBlock = ( BlockLink_t * ) ( ( size_t ) pxBlock + ( ( ( size_t ) pxBlock->xBlockSize ) & HEAP4_NEXT_FREEBLOCK_MASK ) );
	}
}

/* return 1 : if there's heap corruption
   return 0 : if there's no corruption. */
int checkHeapCorruption( void )
{
size_t uxAddress;
size_t uxendAddress;
BlockLink_t *pxBlock;
int errorReturn = 0;
unsigned int p = 1;
size_t xTotalHeapSize = configTOTAL_HEAP_SIZE;
#if( PRINT_HEAP_DEBUG_MORE == 1 )
unsigned int i = 0;
#endif

	/* Ensure the heap starts on a correctly aligned boundary. */
	uxAddress = ( size_t ) ucHeap;

	if( ( uxAddress & portBYTE_ALIGNMENT_MASK ) != 0 )
	{
		uxAddress += ( portBYTE_ALIGNMENT - 1 );
		uxAddress &= ~( ( size_t ) portBYTE_ALIGNMENT_MASK );
	}
	
	uxendAddress = uxAddress + xTotalHeapSize;
	uxendAddress -= xHeapStructSize;
	uxendAddress &= ~( ( size_t ) portBYTE_ALIGNMENT_MASK );

	printf("Checking Heap Corruption \r\n");

	pxBlock = ( BlockLink_t * ) ( uxAddress );

	while( pxBlock->pxNextFreeBlock != NULL )
	{
#if( PRINT_HEAP_DEBUG_MORE == 1 )
		printf ("[HEAP CHECK NO %d] pxBlock = [0x%08x], pxBlock->pxNextFreeBlock = [0x%08x], pxBlock->xBlockSize = [0x%08x]\r\n",i++ ,( unsigned int )pxBlock, ( unsigned int ) pxBlock->pxNextFreeBlock, ( unsigned int )pxBlock->xBlockSize );
#endif
		/* if next block does not have overflow mark */	
		if( ( ( size_t ) ( ( BlockLink_t * ) ( ( size_t ) pxBlock + ( ( ( size_t ) pxBlock->xBlockSize ) & HEAP4_SIZE_MASK ) ) )->pxNextFreeBlock ) >> HEAP4_OVERFLOW_OFFSET != HEAP4_OVERFLOW_DETECTION )
		{

#if( PRINT_HEAP_DEBUG_MORE == 1 )
			printf ("End of Heap Location = [0x%08x]\r\n ",uxendAddress );
#endif
			if(uxendAddress == ( ( size_t ) pxBlock + ( ( ( size_t ) pxBlock->xBlockSize ) & HEAP4_SIZE_MASK ) ) )
			{
				/* reached end of the heap and passed properly. */
				break; 
			}

			errorReturn = 1;

			/* Begin to print Corrupted RAMDUMP */
			printf ("[HEAP CORRUPTED]\r\n"  );
			printf ("OVERFLOW_MARK Corrupt location : 0x%08x  value : %08x \r\n", ( ( BlockLink_t * ) ( ( size_t ) pxBlock + ( ( ( size_t ) pxBlock->xBlockSize ) & HEAP4_SIZE_MASK ) ) ), ( ( size_t ) ( ( BlockLink_t * ) ( ( size_t ) pxBlock + ( ( ( size_t ) pxBlock->xBlockSize ) & HEAP4_SIZE_MASK ) ) )->pxNextFreeBlock ) );
			printf ("[Broken Heap Area]\r\n"  );

			if( pxBlock->xBlockSize & xBlockAllocatedBit )
			{
				printf (" The block before broken block was an alloced block!!!\r\n");
				printf (" before block : [0x%08x]  Size : %08x byte, Caller = %08x \r\n", ( unsigned int ) pxBlock, ( unsigned int ) (pxBlock->xBlockSize), ( void * ) ( ( ( ( size_t ) pxBlock->pxNextFreeBlock ) & HEAP4_CALLER_MASK ) + SRAM_START ) );
			}
			else
			{
				printf (" The block before broken block was a free block!!!\r\n");
				printf (" before block : [0x%08x]  Size : %d byte \r\n", ( unsigned int ) pxBlock, ( unsigned int ) (pxBlock->xBlockSize) );
			}

			printf ("***** near before block.*****\r\n");
			printf("  [0x%08x] = 0x%08x 0x%08x 0x%08x 0x%08x\n", (unsigned int) pxBlock, *(unsigned int *)pxBlock, *(unsigned int *)(pxBlock + 4), *(unsigned int *)( pxBlock + 8 ), *(unsigned int *)( pxBlock + 12 ) );
			
			printf ("***** near broken block.*****\r\n");
			for(p = ( unsigned int )pxBlock + ( ( ( size_t ) pxBlock->xBlockSize ) & HEAP4_SIZE_MASK ) - 256; p < ( unsigned int)pxBlock + ( ( ( size_t ) pxBlock->xBlockSize ) & HEAP4_SIZE_MASK ) ; p+=16 )
			{
				printf("  [0x%08x] = 0x%08x 0x%08x 0x%08x 0x%08x\n", (unsigned int) p, *(unsigned int *)p, *(unsigned int *)(p + 4), *(unsigned int *)( p + 8 ), *(unsigned int *)( p + 12 ) );
			}
			printf(" -> broken checking area \n");
			for(p = ( unsigned int )pxBlock + ( ( ( size_t ) pxBlock->xBlockSize ) & HEAP4_SIZE_MASK ); p < ( unsigned int)pxBlock + ( ( ( size_t ) pxBlock->xBlockSize ) & HEAP4_SIZE_MASK ) + 256; p+=16 )
			{
				printf("  [0x%08x] = 0x%08x 0x%08x 0x%08x 0x%08x\n", (unsigned int) p, *(unsigned int *)p, *(unsigned int *)(p + 4), *(unsigned int *)( p + 8 ), *(unsigned int *)( p + 12 ) );
			}
		}
		pxBlock = ( BlockLink_t * ) ( ( size_t ) pxBlock + ( ( ( size_t ) pxBlock->xBlockSize ) & HEAP4_SIZE_MASK ) );
	}

	if( errorReturn == 0 )
	{
		printf("[PASS] Heap Corruption Check passed. \r\n");
	}
	else
	{
		printf("[Failed] Heap Corruption Check failed. \r\n");
	}

	return errorReturn;
}
#endif
