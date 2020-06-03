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

#define CFG_CTL_CFG_DATA                (*(volatile uint32_t *)(0x40014FFC))
#define CFG_CTL_CFG_CTL                 (*(volatile uint32_t *)(0x40014000))

/*************************************************************
 *
 *  Load FPGA from in memory description
 *
 *************************************************************/

int load_fpga(uint32_t img_size,uint32_t* image_ptr)
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


	// Configuration of CFG_CTRL for writes
	CFG_CTL_CFG_CTL = 0x0000bdff ;
	// wait some time for fpga to get reset pulse
	for (i=0;i<50; i++) {
		PMU->GEN_PURPOSE_1  = i << 4;
	}

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


	for(chunk_cnt=0;chunk_cnt<(img_size/4);chunk_cnt++)
		CFG_CTL_CFG_DATA = gFPGAPtr[chunk_cnt];

	// wait some time for fpga to get reset pulse
	for (i=0;i<50; i++) {
		PMU->GEN_PURPOSE_1  = i << 4;
	}

	CFG_CTL_CFG_CTL = 0x0; // exit config mode
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

	printf("FPGA is programmed\r\n");

	return 1;

}
