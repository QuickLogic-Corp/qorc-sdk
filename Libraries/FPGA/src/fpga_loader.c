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
*    File   : fpga_loader.c
*    Purpose: Contains functionality to load FPGA
*                                                          
*=========================================================*/

#include "Fw_global_config.h"

#include "stdio.h"
#include "eoss3_dev.h"
#include "s3x_clock_hal.h"
#include "s3x_clock.h"

#include "fpga_design.h"


#define REG1                            (*(volatile uint32_t *)(0x40004610))
#define REG2                            (*(volatile uint32_t *)(0x40004044))
#define REG3                            (*(volatile uint32_t *)(0x4000404C))
#define REG4                            (*(volatile uint32_t *)(0x40004064))

#define REG5                            (*(volatile uint32_t *)(0x40004070))
#define REG6                            (*(volatile uint32_t *)(0x4000411C))
#define REG7                            (*(volatile uint32_t *)(0x40004054))
#define REG8                            (*(volatile uint32_t *)(0x400047F8))

#define REG9                            (*(volatile uint32_t *)(0x40014000))

#define REG10                           (*(volatile uint32_t *)(0x400047F0))
#define REG11                           (*(volatile uint32_t *)(0x400047F4))
#define REG12                           (*(volatile uint32_t *)(0x40004088))
#define REG13                           (*(volatile uint32_t *)(0x40004094))
#define REG14                           (*(volatile uint32_t *)(0x400047F8))
#define REG15                           (*(volatile uint32_t *)(0x40004040))
#define REG16                           (*(volatile uint32_t *)(0x40004048))
#define REG17                           (*(volatile uint32_t *)(0x4000404C))
#define REG18                           (*(volatile uint32_t *)(0x40000300))

#define CFG_CTL_CFG_DATA                (*(volatile uint32_t *)(0x40014FFC))
#define CFG_CTL_CFG_CTL                 (*(volatile uint32_t *)(0x40014000))

/*************************************************************
 *
 *  Load FPGA from in memory description
 *
 *************************************************************/

// internal function prototype
int load_fpga_ram_content(uint32_t ram_content_size, uint32_t* ram_content_ptr);


int load_fpga(uint32_t image_size, uint32_t* image_ptr)
{
	unsigned int    i = 0;
	uint32_t        chunk_cnt=0;
	volatile uint32_t   *gFPGAPtr = (volatile uint32_t*)image_ptr;


	*((volatile unsigned int*) 0x40004c4c) = 0x00000180;    

	S3x_Clk_Enable(S3X_FB_02_CLK);
	S3x_Clk_Enable(S3X_A0_08_CLK);
	S3x_Clk_Enable(S3X_FB_16_CLK);	
	S3x_Clk_Enable(S3X_CLKGATE_FB);
	S3x_Clk_Enable(S3X_CLKGATE_PIF);
	*(volatile uint32_t*)(0x4000411c) = 0x00000006; // CRU C09_CLK_GATE enable FB (bit 2), PIF (bit 1)	


	// Configuration of CFG_CTRL for writes
	CFG_CTL_CFG_CTL = 0x0000bdff ;
	// wait some time for fpga to get reset pulse
	for (i=0;i<50; i++) {
		PMU->GEN_PURPOSE_1  = i << 4;
	}

#if 0 // this seems to be extra, not required, can be removed.
	REG8 = 0x10;
	REG8 = 0x20;
	REG8 = 0x30;
	REG8 = 0x40;
	REG8 = 0x50;
	REG8 = 0x60;
	REG8 = 0x70;
	REG8 = 0x80;

	REG9 = 0xBDFF;

	REG8 = 0x10;
	REG8 = 0x20;
	REG8 = 0x30;
	REG8 = 0x40;
	REG8 = 0x50;
	REG8 = 0x60;
	REG8 = 0x70;
	REG8 = 0x80;
#endif 

	for(chunk_cnt=0;chunk_cnt<(image_size/4);chunk_cnt++)
		CFG_CTL_CFG_DATA = gFPGAPtr[chunk_cnt];

	// wait some time for fpga to get reset pulse
	for (i=0;i<50; i++) {
		PMU->GEN_PURPOSE_1  = i << 4;
	}

	CFG_CTL_CFG_CTL = 0x0; // exit config mode
	
	// required wait time
	for (i=0;i<5000; i++) {
		PMU->GEN_PURPOSE_1  = i << 4;
	}


	REG18 = 0x1; // APB mode
	
	// required wait time 
	for (i=0;i<5000; i++) {
		PMU->GEN_PURPOSE_1  = i << 4;
	}
	
	if(load_fpga_ram_content(axFPGAMemInit_length, axFPGAMemInit) < 0)
	{
	    // indicate error: FPGA RAM initialization failed
	    printf("FPGA RAM Blocks init failed!\r\n");
	    return -1;
	}
	
	REG18 = 0x0; // exit APB mode
	
	// required wait time 
	for (i=0;i<500; i++) {
		PMU->GEN_PURPOSE_1  = i << 4;
	}
	
	
	REG10 = 0;
	REG11 = 0;
	REG12 = 0;
	REG13 = 0;
	REG14 = 0x90;


	PMU->GEN_PURPOSE_0 = 0x0; //set APB_FB_EN = 0 for normal mode

	// required wait time before releasing LTH_ENB
	for (i=0;i<500; i++) {
		PMU->GEN_PURPOSE_1  = i << 4;
	}

	//release isolation - LTH_ENB
	PMU->FB_ISOLATION = 0x0;
	*((volatile unsigned int*) 0x40004c4c) = 0x000009a0;

	//printf("FPGA is programmed\r\n");	
	

	return 1;
}


int load_fpga_ram_content(uint32_t ram_content_size, uint32_t* ram_content_ptr)
{
    uint32_t current_index = 0;
    uint32_t ram_content_size_in_words = 0;
    volatile uint32_t* current_block_addr = 0;
    uint32_t current_block_size_in_words = 0;
    uint32_t current_block_iterator = 0;
    uint8_t ram_block_num = 0;
    // the ram content initialization array is of the form:
    //    uint32_t   axFPGAMemInit[] = {
    //          [0]start_addr1, [1]size_in_bytes1,
    //          [2 ... ] values1 (size_in_bytes1/4)...
    //          start_addr2, size_in_bytes2,
    //          values2 (size_in_bytes2/4) ...
    // and so on.
    
    // so, we do:
    // each ram block is defined by : 1 word addr, 1 word size_in_words
    // and size_in_bytes next words of values
    // read the block address and block size
    // read the next "block_size_in_words" entries from the array and write to consecutive addresses
    // starting from the block address
    // check if we have reached end of the array, then done.
    // othewise, we process the next ram block in the same way as above.
    
    // check if there is any ram blocks to init
    if (ram_content_size == 0) 
    {
        //printf("no content to init ram blocks\r\n");
        return 0;
    }   
    
    
    // calculate the total number of entries in the array
    ram_content_size_in_words = ram_content_size/4;
    
    //printf("ram init content size: 0x%08x, %u\r\n", ram_content_size_in_words, ram_content_size_in_words);
        
    
    // process all ram blocks
    while(1)
    {
        // read block_addr and block_size_in_words
        current_block_addr = (uint32_t *)ram_content_ptr[current_index++];
        current_block_size_in_words = ram_content_ptr[current_index++];
        //printf("ram_block: [%d], start:[0x%08x], size: [0x%08x, %u]\r\n", ram_block_num,
        //                                                             current_block_addr,
        //                                                             current_block_size_in_words,
        //                                                             current_block_size_in_words);
                                                                     
        // read the next "block_size_in_words" entries and write value into consecutive addresses
        for(current_block_iterator = 0; current_block_iterator < current_block_size_in_words; current_block_iterator++)
        {
            *(current_block_addr) = ram_content_ptr[current_index + current_block_iterator];
            // readback-verify
            if(*current_block_addr != ram_content_ptr[current_index + current_block_iterator])
            {
                printf("addr: 0x%08x, read: 0x%08x, write: 0x%08x\r\n", current_block_addr,
                                                                        *current_block_addr,
                                                                        ram_content_ptr[current_index + current_block_iterator]);
                return -1;
                                                                        
            }
            current_block_addr++;
        }
        
        // increment the index with the current block size
        current_index += current_block_size_in_words;
        
       // check if any more ram blocks remaining:
       if(current_index >= ram_content_size_in_words) 
       {
            //printf("end of ram blocks initialization\r\n");
            break;
       }
       
       // otherwise, move to the next block, current_index is already at the right place.
       ram_block_num++;
    }
    
    // indicate all ok
    return 1;
}

