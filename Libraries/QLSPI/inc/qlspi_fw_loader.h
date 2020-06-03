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

#ifndef __QLSPI_FW_LOADER_H__
#define __QLSPI_FW_LOADER_H__
#include "qlspi_s3.h"
//#include "tlc_reg.h"

typedef struct {

	volatile unsigned char* m4_fw_addr;		//should be a pointer , unsigned char*
	
	volatile uint32_t m4_fw_size;
	volatile uint32_t m4_fw_dest_addr;
	volatile unsigned char* ffe_fw_addr;
	volatile uint32_t ffe_fw_size;
	volatile uint32_t ffe_fw_dest_addr;
	volatile unsigned char* fab_fw_addr;
	volatile uint32_t fab_fw_size;
	volatile uint32_t fab_fw_dest_addr;


	volatile unsigned char* fab_prg_fw_addr;
	volatile uint32_t fab_prg_fw_size;
	volatile uint32_t fab_prg_fw_dest_addr;

	
} SLAVE_DEV_FW_LOAD_T;

QL_Status QLSPI_fw_download (SLAVE_DEV_FW_LOAD_T * slave_dev_fw_load_info);

//#define FFE_LOADER

#define FFE_IMAGE_HAS_HEADER_AND_SM1_SUPPORT			// define this if FFE Image has signature header in the binary as well as SM1 support (Celeris 3.2.7 onwards)

//#define FABRIC_LOADER	

#ifdef FABRIC_LOADER

	#define	IR_FABRIC_DEV		

	#define	VERIFY_FABRIC_IMAGE			

	struct FABRIC_HEADER
	{
		uint32_t size;
	};

	/* Fabric_PRG.bin (FPGA programmer) address */
	
	#define FABRIC_PROG_FW_DEST_ADDR		(0x20000000)

	#define FABRIC_HEADER_ADDR				(FABRIC_PROG_FW_DEST_ADDR+(32*1024))

	/* Fabric program completion status written at this address*/
	
	#define FABRIC_PROGRAM_STS				(FABRIC_HEADER_ADDR+sizeof(struct FABRIC_HEADER))	//this is 4B
	
	/* Fabric.bin copied to this location */
	
	#define FABRIC_FW_DEST_ADDR			(0x20010000)

#endif

#endif	/* __QL_MCU_H__ */
