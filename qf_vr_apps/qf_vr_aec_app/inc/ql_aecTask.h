/*==========================================================
 * Copyright 2021 QuickLogic Corporation
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
 *    File   : ql_aecTask.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef _S3_QL_AECTASK_H_
#define _S3_QL_AECTASK_H_

#include "Fw_global_config.h"

#define SDMA_FRAME_SIZE_MS  (15)

#define SDMA_SINGLE_BUFFER_SIZE   (240)
#define SDMA_NUMBER_OF_BUFFERS    (30)
#define SDMA_BYTES_TO_READ        (SDMA_SINGLE_BUFFER_SIZE*2)
#define SDMA_WORDS_TO_READ        (SDMA_SINGLE_BUFFER_SIZE/2)


#define AEC_QUEUE_LENGTH      (10)
#define PRIORITY_TASK_AEC     (PRIORITY_HIGH + 2) //(PRIORITY_HIGH)//(18) //(4)
#define STACK_SIZE_TASK_AEC   (256)

//========================
enum AUDIO_AEC
{
    eCMD_AUDIO_DATA_READY_AEC = 1,
	eCMD_AUDIO_DATA_READY_I2S,
    eCMD_AUDIO_I2S_DATA_START,
    eCMD_AUDIO_I2S_DATA_STOP,
    eCMD_AUDIO_AEC_PROCESS,
    eCMD_AUDIO_CLK_SYNC_INVOKE
};
void cb_notify_i2sRx_intr_from_FPGA(void);
//========================


#if (AEC_ENABLED == 1)

#include "common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "eoss3_hal_fpga_i2s_slave.h"
#if 0

// Sampling Frequency
// AEC 2.0, FIR decimation is part fabric, Fabric I2S Slave Rx output is at 16KHz
#define SDMA_FS              (16000)

// Base Frame Size gives number of samples in 1 msec
#define SDMA_BASE_FRAME_SIZE ((SDMA_FS)/1000)  // 48 samples in 1ms

#define SDMA_BYTES_TO_READ		(SDMA_SINGLE_BUFFER_SIZE*2)
#define SDMA_WORDS_TO_READ		(SDMA_SINGLE_BUFFER_SIZE/2)

//#define SDMA_NUMBER_OF_BUFFERS_LEFT_CH 200
#define SDMA_NUMBER_OF_BUFFERS_LEFT_CH 10

// Sampling Frequency
#define SDMA_16KFS              (16000)

//  Frame Size gives number of samples in 1 msec
#define I2S_DECIMATE_FRAME_SIZE ((SDMA_16KFS)/1000)  // 16 samples in 1ms

#define DECIMATE_LEFT_CH (I2S_DECIMATE_FRAME_SIZE*SDMA_FRAME_SIZE_MS)

typedef struct sdma_buffer_s_left_ch {
  int16_t  mem[SDMA_NUMBER_OF_BUFFERS_LEFT_CH][DECIMATE_LEFT_CH];
  uint16_t count;

} sdma_buffer_t_left_ch;



/* Change the time according to the processing time taken by consilient library */
#define PTIME_AEC   10
/* to be aligned with DMA_SINGLE_BUFFER_SIZE in dma_buffer.h */
#define SAMPLE_15MS			(DECIMATE_LEFT_CH)
#define SAMPLE_30MS			(2*SAMPLE_15MS)

#endif

int FB_I2S_Slave_Rx_Enable(void);
int FB_I2S_Slave_Rx_Disable(void);

void cb_notify_i2sRx_intr_from_FPGA(void);
#endif //AEC_ENABLED

#endif  /* _S3_QL_AECTASK_H_ */