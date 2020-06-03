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
 *    File   : dma_buffer.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef HAL_INC_DMA_BUFFER_H_
#define HAL_INC_DMA_BUFFER_H_

#include <stdbool.h>

// Sampling Frequency
#define FS              (16000)

// Base Frame Size gives number of samples in 1 msec
#define BASE_FRAME_SIZE ((FS)/1000)  // 16 samples in 1ms

// Frame size in msec
#define FRAME_SIZE_MS   (15)   // 1 Frame duration = 15 ms

// Frame size gives number of samples in one frame
#define FRAME_SIZE      ((FRAME_SIZE_MS)*(BASE_FRAME_SIZE)) // 1 Frame size = 15*16 = 240 (samples = 480 bytes)


// DMA block size of single buffer size for BLOCKINT to be fired
#define DMA_SINGLE_BUFFER_SIZE (FRAME_SIZE)

// # of DMA buffers

#define DMA_NUMBER_OF_BUFFERS  (15) // 200 bricks =3s of audio data

// DMA multi buffer size for WRAPINT to be fired
#define DMA_MULTI_BUFFER_SIZE ((DMA_NUMBER_OF_BUFFERS) * (DMA_SINGLE_BUFFER_SIZE))

/*
 * DMA buffer where audio data will be captured.
 */
typedef struct dma_buffer_s {
   uint16_t head;
   uint16_t head2;
   int16_t  mem[DMA_NUMBER_OF_BUFFERS][DMA_SINGLE_BUFFER_SIZE];
   uint16_t tail;
   uint16_t tail2;
   uint16_t count;
   bool     overflow;
   uint8_t  spare1;
   uint16_t spare2;
} dma_buffer_t;


extern dma_buffer_t gDmaBuffer;


void init_dmaBuffer();


#endif /* HAL_INC_DMA_BUFFER_H_ */
