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

 /*  qlspi_fw_loader.c
  *  Copyrighted (C) 2016 QuickLogic Corp
  *
  *  This file contains source to download the firmware
  *  into S3 Memory. It provides an API exposed to application
  *  which will download the firmware into S3 Memory.
  *  This file provides API which uses QLSPI API to download 
  *  the firmware.
  *  This API does the below things required to download the 
  *  firmware into S3 Memory. 
  *   	- Reset S3 MCU 
  *     - Load firmware into S3 Memory
  *     - Release S3 MCU from reset
  */
#include <stdint.h>
#include <stdio.h>
#include "qlspi_fw_loader.h"
#include "tlc_reg.h"
#include "qlspi_s3.h"
//#include "qltrace.h"



#define VALIDATE_FFE_MEM_WRITE  0

#if (VALIDATE_FFE_MEM_WRITE == 1)
#define READBACK_FFE_CFGMEM	1
#define READBACK_FFE_DMMEM	1
#define READBACK_FFE_SM0MEM	1
#define READBACK_FFE_SM1MEM	1
#define READBACK_FFE_CMMEM	1
#else
#define READBACK_FFE_CFGMEM	0
#define READBACK_FFE_DMMEM	0
#define READBACK_FFE_SM0MEM	0
#define READBACK_FFE_SM1MEM	0
#define READBACK_FFE_CMMEM	0
#endif  /* VALIDATE_FFE_MEM_WRITE */

static uint8_t ucRxBuf[10];
static volatile uint32_t *FFE_CFG = (volatile uint32_t*)0x2007FE80;
static uint32_t * const FFE_DM0 = (uint32_t*)0x40040000;
static uint32_t * const FFE_CM  = (uint32_t*)0x40050000;
static uint32_t * const FFE_SM0  = (uint32_t*)0x40044000;
static uint32_t * const FFE_SM1  = (uint32_t*)0x40048000;


/*Function Declarations*/
static int load_fw_s3_mem(unsigned char *rawData, uint32_t size, uint32_t DestAddr);
static int release_slave_mcu(void);
static int reset_slave_mcu(void);
void readback_slave_image(uint32_t rawdata_size);


/*Function Definitions*/

int32_t disp_reset_reg(uint32_t* slave_por_addr)
{
	QL_Status ql_status;
	
	*slave_por_addr =0;
	
	ql_status = QLSPI_Read_S3_Mem(MISC_POR_0_ADDR,(uint8_t*)slave_por_addr, 4);

	printf(" *** Reset reg %x \n",*slave_por_addr);

	if (ql_status != QL_STATUS_OK)
	{
		printf(" ### Failure reading RESET reg \n");
		return -1;
	}

	return 0;
}


static int reset_slave_mcu(void)
{
	int ql_status = QL_STATUS_OK;
	uint32_t slave_por_addr = 0x00UL; /* Keep the MCU in reset mode */

	int32_t sts;

	uint8_t aucBuffer;
	int rty_count = 2;

	/* Access control write only once optimization */		
	RETRY:
		aucBuffer = 0x3;
	
		ql_status = tlc_reg_write( SPITLC_AHB_ACCESS_CTL, &aucBuffer, ONE_BYTE);	
	
		if(ql_status < 0){
	
			if(rty_count-- > 0)
				goto RETRY;
	
			printf("Failed to write SPITLC_AHB_ACCESS_CTL\n");
			return QL_STATUS_ERROR;
	
		}	
	
	//printf("  Access control write \n");

	printf(" Reset reg before RESET \n");

	sts=disp_reset_reg(&slave_por_addr);

	if(sts<0)
	{
		printf(" Failure reading reset reg MISC_POR_0_ADDR \n");
		return QL_STATUS_ERROR;
	}

	if(slave_por_addr)
	{	
		printf(" Resetting M4 \n");

		slave_por_addr = 0x00UL;
	
		ql_status = QLSPI_Write_S3_Mem(MISC_POR_0_ADDR,(uint8_t *)&slave_por_addr, 4);
		if (ql_status != QL_STATUS_OK)
		{
			printf("Failure in resetting mcu \n");
			return QL_STATUS_ERROR;
		}

		printf(" Reset reg after RESET \n");

		slave_por_addr=0;

		disp_reset_reg(&slave_por_addr);

		return QL_STATUS_OK;
	}

	printf(" M4 already in reset state %x \n",slave_por_addr);


	
	return QL_STATUS_OK;
}

static int release_slave_mcu(void)
{
	int ql_status = QL_STATUS_OK;
	uint32_t slave_por_addr = 0x01UL; /* Keep the MCU in reset mode */

	int32_t sts;

	slave_por_addr=0;

	printf(" Reset reg before release \n");

	sts=disp_reset_reg(&slave_por_addr);

	if(sts<0)
	{
		printf(" Failure reading reset reg MISC_POR_0_ADDR \n");
		return QL_STATUS_ERROR;
	}

	if(!slave_por_addr)
	{
		slave_por_addr=1;
	
		ql_status = QLSPI_Write_S3_Mem(MISC_POR_0_ADDR,(uint8_t *)&slave_por_addr, 4);
		if (ql_status != QL_STATUS_OK)
		{
			printf("Failure in releasing mcu from reset \n");
			return QL_STATUS_ERROR;
		}

		printf(" Reset reg after release \n");

		slave_por_addr=0;

		disp_reset_reg(&slave_por_addr);
	
	}
	else
	{
		printf(" M4 RESET already released\n");
	}

	return QL_STATUS_OK;
}

static int load_fw_s3_mem(unsigned char *rawData, uint32_t size, uint32_t DestAddr)
{
	int ql_status = QL_STATUS_OK;
	volatile uint32_t ulOrigSlaveImgSize;
	uint32_t ulRemImgSize;
	uint32_t ulTxImgSize;
	volatile uint32_t ulRemAlign4ByteWords;
	volatile uint32_t ulRemImageData;
	uint32_t index;

	printf(" Loading FW size %d , destaddr 0x%x \n", size,DestAddr);
        
	ql_status = QLSPI_Read_S3_Mem(MISC_POR_0_ADDR,&ucRxBuf[0],4);
	if (ql_status != QL_STATUS_OK)
	{
		printf("QLSPI_Read_S3_Mem Failure \n");
		return QL_STATUS_ERROR;
	}

	printf("POR content after reset - 0x%08x \n", *(uint32_t *)ucRxBuf);
	
	ulOrigSlaveImgSize = size;
	printf("\n Raw Image Size - %u", ulOrigSlaveImgSize);
	ulRemImgSize = ulOrigSlaveImgSize;
	ulTxImgSize = 0x00UL;
	
	do {
		
		if (ulRemImgSize >= QL_SPI_WRITE_MAX_LEN ) {
			
			/* Write some bytes of data from slave image binary */

			ql_status = QLSPI_Write_S3_Mem(DestAddr + ulTxImgSize, &rawData[ulTxImgSize], QL_SPI_WRITE_MAX_LEN);

			if (ql_status != QL_STATUS_OK)
			{
				printf("QLSPI_Write_S3_Mem Failure \n");
				return QL_STATUS_ERROR;
			}
			ulTxImgSize += QL_SPI_WRITE_MAX_LEN;
			ulRemImgSize -= QL_SPI_WRITE_MAX_LEN;
		}

		
		else {
			
			ulRemAlign4ByteWords = ulRemImgSize / 4;
			ulRemAlign4ByteWords *= 4;

			ql_status = QLSPI_Write_S3_Mem(DestAddr + ulTxImgSize, &rawData[ulTxImgSize], ulRemAlign4ByteWords);
			if (ql_status != QL_STATUS_OK)
			{
				printf("QLSPI_Write_S3_Mem Failure \n");
				return QL_STATUS_ERROR;
			}
			ulTxImgSize += ulRemAlign4ByteWords;
			ulRemImgSize -= ulRemAlign4ByteWords;

			ulRemAlign4ByteWords = ulRemImgSize % 4;
			if (ulRemAlign4ByteWords) {
				ulRemImageData = 0x00UL;
				index = 0x00UL;
				while (ulRemAlign4ByteWords) {
					ulRemImageData |= rawData[ulTxImgSize] << (index*8);
					++ulTxImgSize;
					++index;
					--ulRemAlign4ByteWords;
				}
				ulRemAlign4ByteWords = ulRemImgSize % 4;
				
				ql_status = QLSPI_Write_S3_Mem(DestAddr + ulTxImgSize - ulRemAlign4ByteWords, (uint8_t *)&ulRemImageData, 4);
				if (ql_status != QL_STATUS_OK)
				{
					printf("QLSPI_Write_S3_Mem Failure \n");
					return QL_STATUS_ERROR;
				}
				ulRemImgSize -= ulRemAlign4ByteWords;
			}
		}
				
	} while (ulRemImgSize); 

	return QL_STATUS_OK;
}

#ifdef FABRIC_LOADER

QL_Status read_s3_fw_mem (uint32_t addr, uint8_t *data, uint32_t len) 
{
	uint32_t noc=0,c=0, now=0;
	uint32_t tempbuff;

	if(len>=QL_SPI_READ_MAX_LEN)
	{
		printf(" Read size greater than QLSPI size");	

		noc=len/QL_SPI_READ_MAX_LEN;

		printf(" noc %x \n",noc);

		if(noc)
		{		
			for(c=0;c<noc;c++)		// no of QL_SPI_READ_MAX_LEN chunks
			{
				if(QLSPI_Read_S3_Mem(addr,data,QL_SPI_READ_MAX_LEN)!=QL_STATUS_OK)
				{
					printf(" Error reading S3 data at 0x%x \n",addr);

					return QL_STATUS_ERROR;
				}
				
				addr+=QL_SPI_READ_MAX_LEN;
				
				data+=QL_SPI_READ_MAX_LEN;
			}
		}
	}
	
	noc=len%QL_SPI_READ_MAX_LEN;

//	QL_TRACE_MCU_DEBUG(" Read nob %d , addr %x  \n",noc, addr);

	if(noc)
	{
		now=(noc/4);		//no of dwords

		if(now)
		{			
			printf(" Read now (4b)  %d , addr %x  \n",now, addr);			
		
			if(QLSPI_Read_S3_Mem(addr,data,(now*4))!=QL_STATUS_OK)
			{
				printf(" Error reading S3 data at 0x%x , size %d Bytes \n",addr,(now*4));

				return QL_STATUS_ERROR;
			}

			addr+=(now*4);

			data+=(now*4);

			now=noc%4;		// no of bytes remaining

			printf(" Read nob (1b) %d , addr %x	\n",now, addr);

			if(now)
			{
				if(QLSPI_Read_S3_Mem(addr,(uint8_t*)&tempbuff,4)!=QL_STATUS_OK)
				{
					printf(" Error reading fw \n");
	
					return QL_STATUS_ERROR;
				}

				memcpy(data,(uint8_t*)tempbuff,now);

			}
		}
		
	}

	return QL_STATUS_OK;

}

QL_Status QLFAB_Fw_Download (unsigned char *fab_fw_raw_data, uint32_t size,unsigned char *fab_prg_fw_raw_data, uint32_t prg_size) 
{
	int ql_status = QL_STATUS_OK; 

	volatile uint32_t prog_sts;

	uint32_t counter=0;

	struct FABRIC_HEADER hdr;

#ifdef VERIFY_FABRIC_IMAGE

	uint8_t* fw_buf_v=NULL;
	uint32_t i=0;
	
#endif	
	
	/* Do and Hold the mcu in reset mode */
	
	ql_status = reset_slave_mcu();

	if (ql_status != QL_STATUS_OK)
	{
		printf(" Reset Slave MCU for FPGA programmer Failed \n");
	
		return QL_STATUS_ERROR;
	}

//	QL_TRACE_MCU_DEBUG("Reset Slave MCU for FPGA programmer DONE\n");

	prog_sts=0x00000000;

	QLSPI_Write_S3_Mem(FABRIC_PROGRAM_STS,(uint8_t*)&prog_sts,4);

	QLSPI_Write_S3_Mem(FABRIC_HEADER_ADDR,(uint8_t*)&prog_sts,4);

	if (fab_fw_raw_data && size) 
	{

		printf(" Loading Fabric image  \n");

		ql_status = load_fw_s3_mem(fab_fw_raw_data, size, FABRIC_FW_DEST_ADDR);


		if (ql_status != QL_STATUS_OK)
		{
			printf("Loading Fabric image FAILED\n");

			return QL_STATUS_ERROR;
		}

#ifdef VERIFY_FABRIC_IMAGE

		fw_buf_v=malloc(size);

		if(NULL==fw_buf_v)
		{
			printf("\n Error allocating verify firware buffer \n");

			return QL_STATUS_ERROR;
		}

		printf(" Reading Firmware Back\n");

		ql_status=read_s3_fw_mem(FABRIC_FW_DEST_ADDR,fw_buf_v,size);

		if(ql_status!=QL_STATUS_OK)
		{
			printf(" Reading Firmware Back\n");
			free(fw_buf_v);
			return QL_STATUS_ERROR;
		}

		printf(" Reading Firmware Back DONE , ret %d \n",ql_status);

		for(i=0;i<size;i++)
		{
			if(fw_buf_v[i]!=fab_fw_raw_data[i])
			{
				printf(" Fabric Firmware Verify FAILED, %d , %x %x \n",i,fw_buf_v[i],fab_fw_raw_data[i]);
				free(fw_buf_v);		
				return QL_STATUS_ERROR;
			}
		}

		printf(" Fabric Firmware Verify Passed \n");

		free(fw_buf_v); 	

#endif
	
		printf(" Writing Fabric header \n");

		hdr.size=size;

		load_fw_s3_mem((unsigned char*)&hdr, sizeof(struct FABRIC_HEADER), FABRIC_HEADER_ADDR);	

		printf(" Loading Fabric programmer \n");

		ql_status = load_fw_s3_mem(fab_prg_fw_raw_data, prg_size, FABRIC_PROG_FW_DEST_ADDR);

		if (ql_status != QL_STATUS_OK)
		{
			printf("Loading Fabric programmer FAILED\n");
			return QL_STATUS_ERROR;
		}
	
		printf(" Loading Fabric programmer DONE \n");
	}	

	/* Release MCU from reset state */

	ql_status = release_slave_mcu();
	
	if (ql_status != QL_STATUS_OK)
	{
		printf("Releasing RESET to FPGA programmer FAILED\n");
		return QL_STATUS_ERROR;
	}

	printf("Releasing RESET to FPGA programmer DONE\n");

	printf(" Waiting for program completion \n ");	

	counter=0;

	do
	{
		prog_sts=0;
		QLSPI_Read_S3_Mem(FABRIC_PROGRAM_STS,(uint8_t*)&prog_sts,4);


		if(counter++>2000)
			{
				printf(" Timeout waiting for fabric program completion \n ");					
				return QL_STATUS_ERROR;
			}

	
	}while(prog_sts!=1);	

	printf(" Fabric FW download completed %d ",prog_sts);	

	return ql_status;

}

#endif

QL_Status QLFFE_Fw_Download (unsigned char *ffe_fw_raw_data, uint32_t size) 
{
	int ql_status = QL_STATUS_OK;
	uint32_t *buf;
	
#if VALIDATE_FFE_MEM_WRITE	
	uint8_t *buf_cmp;
#endif

	volatile uint32_t *exportVariableTab;
	volatile uint32_t *dmTab;
	volatile uint32_t *sm0Tab;
#ifdef FFE_IMAGE_HAS_HEADER_AND_SM1_SUPPORT
	volatile uint32_t *sm1Tab;
#endif // FFE_IMAGE_HAS_HEADER_AND_SM1_SUPPORT
	volatile uint32_t *cmTab;
	uint32_t exportVariableTabSzInWord = 0;
	uint32_t dmTabSzInWord = 0;
	uint32_t sm0TabSzInWord = 0;
#ifdef FFE_IMAGE_HAS_HEADER_AND_SM1_SUPPORT
	uint32_t sm1TabSzInWord = 0;
#endif // FFE_IMAGE_HAS_HEADER_AND_SM1_SUPPORT
	uint32_t cmTabSzInWord = 0;
	uint32_t S3_Reg_Addr = 0x0UL;
	volatile uint32_t Reg_Value = 0x00UL;
#ifdef FFE_IMAGE_HAS_HEADER_AND_SM1_SUPPORT
	uint8_t* raw_data_ptr;
#endif // FFE_IMAGE_HAS_HEADER_AND_SM1_SUPPORT

	printf("  FFE Fw size %d bytes \n",size);

	
	/* Do and Hold the mcu in reset mode */
	ql_status = reset_slave_mcu();
	if (ql_status != QL_STATUS_OK)
	{
		printf("Reset Slave MCU Failed \n");
		return QL_STATUS_ERROR;
	}

#ifdef FFE_IMAGE_HAS_HEADER_AND_SM1_SUPPORT
	raw_data_ptr = (uint8_t *)ffe_fw_raw_data;
	while(*(raw_data_ptr++) != '$');	// raw_data_ptr is now at character after $
	while(*(raw_data_ptr++) != '\n');	// raw_data_ptr is now at character after \n, header parsing code is TODO

	buf = (uint32_t *)raw_data_ptr;
#endif // FFE_IMAGE_HAS_HEADER_AND_SM1_SUPPORT


#if VALIDATE_FFE_MEM_WRITE
	buf_cmp = (uint8_t *)os_malloc(256);
	if (!buf_cmp) {
		printf("Memory Allocation has failed \n");
		return QL_STATUS_ERROR;
	}
#endif

	exportVariableTabSzInWord = buf[0] >> 2;
	dmTabSzInWord = buf[1] >> 2;
	sm0TabSzInWord = buf[2] >> 2;
#ifdef FFE_IMAGE_HAS_HEADER_AND_SM1_SUPPORT
	sm1TabSzInWord = buf[3] >> 2;
	cmTabSzInWord = buf[4] >> 2;
#else // NOT FFE_IMAGE_HAS_HEADER_AND_SM1_SUPPORT
	cmTabSzInWord = buf[3] >> 2;
#endif // FFE_IMAGE_HAS_HEADER_AND_SM1_SUPPORT

#ifdef FFE_IMAGE_HAS_HEADER_AND_SM1_SUPPORT
	exportVariableTab = &buf[5];
#else // NOT FFE_IMAGE_HAS_HEADER_AND_SM1_SUPPORT
	exportVariableTab = &buf[4];
#endif // FFE_IMAGE_HAS_HEADER_AND_SM1_SUPPORT
	dmTab = exportVariableTab + exportVariableTabSzInWord;
	sm0Tab = dmTab + dmTabSzInWord;
#ifdef FFE_IMAGE_HAS_HEADER_AND_SM1_SUPPORT
	sm1Tab = sm0Tab + sm0TabSzInWord;
	cmTab = sm1Tab + sm1TabSzInWord;
#else // NOT FFE_IMAGE_HAS_HEADER_AND_SM1_SUPPORT
	cmTab = sm0Tab + sm0TabSzInWord;
#endif // FFE_IMAGE_HAS_HEADER_AND_SM1_SUPPORT

	printf("\nValidating FFE image data and sizes have been proceed correctly or not");
	printf("\nExport section size = %d, address = 0x%08x, starting data = 0x%08x", exportVariableTabSzInWord << 2, (uint8_t *)exportVariableTab - (uint8_t *)raw_data_ptr, *exportVariableTab);
	printf("\nData Memory section size = %d, address = 0x%08x, starting data = 0x%08x", dmTabSzInWord << 2, (uint8_t *)dmTab- (uint8_t *)raw_data_ptr, *dmTab);
	printf("\nSection Memory0 section size = %d, address = 0x%08x, starting data = 0x%08x", sm0TabSzInWord << 2, (uint8_t *)sm0Tab-(uint8_t *)raw_data_ptr, *sm0Tab);
	printf("\nSection Memory1 section size = %d, address = 0x%08x, starting data = 0x%08x", sm1TabSzInWord << 2, (uint8_t *)sm1Tab-(uint8_t *)raw_data_ptr, *sm0Tab);
	printf("\nCode Memory section size = %d, address = 0x%08x, starting data = 0x%08x", cmTabSzInWord << 2, (uint8_t *)cmTab-(uint8_t *)raw_data_ptr, *cmTab);
	
	//enable FFE power & clock domain
	S3_Reg_Addr = 0x40004494; /* FFE_PWR_MODE_CFG */
	ql_status = QLSPI_Write_S3_Mem(S3_Reg_Addr, (uint8_t *)&Reg_Value, 4);
	if (ql_status != QL_STATUS_OK)
	{
		printf("QLSPI_Write_S3_Mem Failure \n");
		return QL_STATUS_ERROR;
	}
	
	S3_Reg_Addr = 0x40004498; /* FFE_PD_SRC_MASK_N */
	ql_status = QLSPI_Write_S3_Mem(S3_Reg_Addr, (uint8_t *)&Reg_Value, 4);
	if (ql_status != QL_STATUS_OK)
	{
		printf("QLSPI_Write_S3_Mem Failure \n");
		return QL_STATUS_ERROR;
	}
	
	S3_Reg_Addr = 0x4000449C; /* FFE_WU_SRC_MASK_N */
	ql_status = QLSPI_Write_S3_Mem(S3_Reg_Addr, (uint8_t *)&Reg_Value, 4);
	if (ql_status != QL_STATUS_OK)
	{
		printf("QLSPI_Write_S3_Mem Failure \n");
		return QL_STATUS_ERROR;
	}

	/* Wake up FFE */
	S3_Reg_Addr = 0x40004610; /* FFE_FB_PF_SW_WU */
	Reg_Value = 0x01UL;
	ql_status = QLSPI_Write_S3_Mem(S3_Reg_Addr, (uint8_t *)&Reg_Value, 4);
	if (ql_status != QL_STATUS_OK)
	{
		printf("QLSPI_Write_S3_Mem Failure \n");
		return QL_STATUS_ERROR;
	}
	

	/* check if FFE is in Active mode */
	S3_Reg_Addr = 0x40004490; /* FFE_STATUS */
	do {
		ql_status =  QLSPI_Read_S3_Mem(S3_Reg_Addr, (uint8_t *)&Reg_Value, 4);	
		if (ql_status != QL_STATUS_OK)
		{
			printf("QLSPI_Write_S3_Mem Failure \n");
			Reg_Value = 0x00; /* Don't return continue in loop */
		}
	} while(!(Reg_Value& 0x1));
	
	/* Enable C08-X4 clock */
	S3_Reg_Addr = 0x40004010;	/* CLK_CTRL_C_0 */
	Reg_Value = 0x204;
	ql_status = QLSPI_Write_S3_Mem(S3_Reg_Addr, (uint8_t *)&Reg_Value, 4);
	if (ql_status != QL_STATUS_OK)
	{
		printf("QLSPI_Write_S3_Mem Failure \n");
		return QL_STATUS_ERROR;
	}

	S3_Reg_Addr = 0x40004134;  /* CLK_SWITCH_FOR_C */
	Reg_Value = 0x00UL;
	ql_status = QLSPI_Write_S3_Mem(S3_Reg_Addr, (uint8_t *)&Reg_Value, 4);
	if (ql_status != QL_STATUS_OK)
	{
		printf("QLSPI_Write_S3_Mem Failure \n");
		return QL_STATUS_ERROR;
	}

	S3_Reg_Addr = 0x40004040;	/* C01_CLK_GATE */
	Reg_Value = 0x29F;
	ql_status = QLSPI_Write_S3_Mem(S3_Reg_Addr, (uint8_t *)&Reg_Value, 4);
	if (ql_status != QL_STATUS_OK)
	{
		printf("QLSPI_Write_S3_Mem Failure \n");
		return QL_STATUS_ERROR;
	}
	
	
	/* Enable C08-X4 clock */
	S3_Reg_Addr =0x40004048; /* C08_X4_CLK_GATE */
	Reg_Value = 0x01;
	ql_status = QLSPI_Write_S3_Mem(S3_Reg_Addr, (uint8_t *)&Reg_Value, 4);
	if (ql_status != QL_STATUS_OK)
	{
		printf("QLSPI_Write_S3_Mem Failure \n");
		return QL_STATUS_ERROR;
	}

	/* Enable C08-X1 clock */
	S3_Reg_Addr =0x4000404C; /* C08_X1_CLK_GATE */
	Reg_Value = 0x0F;
	ql_status = QLSPI_Write_S3_Mem(S3_Reg_Addr, (uint8_t *)&Reg_Value, 4);
	if (ql_status != QL_STATUS_OK)
	{
		printf("QLSPI_Write_S3_Mem Failure \n");
		return QL_STATUS_ERROR;
	}		

	S3_Reg_Addr =0x40004084; /* FFE_SW_RESET */
	Reg_Value = 0x03;
	ql_status = QLSPI_Write_S3_Mem(S3_Reg_Addr, (uint8_t *)&Reg_Value, 4);
	if (ql_status != QL_STATUS_OK)
	{
		printf("QLSPI_Write_S3_Mem Failure \n");
		return QL_STATUS_ERROR;
	}

	S3_Reg_Addr =0x40004084; /* FFE_SW_RESET */
	Reg_Value = 0x00;
	ql_status = QLSPI_Write_S3_Mem(S3_Reg_Addr, (uint8_t *)&Reg_Value, 4);
	if (ql_status != QL_STATUS_OK)
	{
		printf("QLSPI_Write_S3_Mem Failure \n");
		return QL_STATUS_ERROR;
	}
	
	/* Load FFE Export Variable Memory */
	if (exportVariableTabSzInWord) { 
		ql_status = load_fw_s3_mem((uint8_t *)exportVariableTab, exportVariableTabSzInWord * 4, (uint32_t)FFE_DM0);
		if (ql_status != QL_STATUS_OK)
		{
			printf("Load of Firwmare in S3 Memory Failed \n");
			return QL_STATUS_ERROR;
		}
		
#if (READBACK_FFE_CFGMEM == 1)
		/* Read back Export region of FFE and compare to check data integrity */
		ulRemImgSize= exportVariableTabSzInWord << 2;
		ulRxImgSize = 0;
		S3_Reg_Addr = (uint32_t)FFE_CFG; /* Export Region Beginning */
		temp = 4;
		
		do {

			ql_status =  ql_spi_read_s3_mem_ahb(S3_Reg_Addr + ulRxImgSize, (uint8_t *)(buf_cmp), temp);	
			if (ql_status != QL_STATUS_OK)
			{
				printf("ql_spi_read_s3_mem_ahb Failure \n");

					configASSERT(0);
				
			}
		
			if (*(uint32_t *)buf_cmp != *(uint32_t *)(exportVariableTab + ulRxImgSize)) {
				printf("FFE Section memory has Failure \n");

					configASSERT(0);
				
			}

			else {
				ulRemImgSize -= temp;
				ulRxImgSize += temp;
			}
		} while(ulRemImgSize);
#endif		/* READBACK_FFE_CFGMEM */ 		
	}

	/* Load FFE Data Memory */
	if (dmTabSzInWord) {
		ql_status = load_fw_s3_mem((uint8_t *)dmTab, dmTabSzInWord* 4, (uint32_t)FFE_DM0);
		if (ql_status != QL_STATUS_OK)
		{
			printf("Load of Firwmare in S3 Memory Failed \n");
			return QL_STATUS_ERROR;
		}
#if (READBACK_FFE_DMMEM == 1)
		/* Read back Export region of FFE and compare to check data integrity */
		ulRemImgSize= dmTabSzInWord << 2;
		ulRxImgSize = 0;
		S3_Reg_Addr = (uint32_t)FFE_DM0; /* Export Region Beginning */
		temp = 4;
		
		do {

			ql_status =  ql_spi_read_s3_mem_ahb(S3_Reg_Addr + ulRxImgSize, (uint8_t *)(buf_cmp), temp);	
			if (ql_status != QL_STATUS_OK)
			{
				printf("ql_spi_read_s3_mem_ahb Failure \n");

					configASSERT(0);
			}
		
			if (*(uint32_t *)buf_cmp != *(uint32_t *)((uint8_t *)dmTab + ulRxImgSize)) {
				printf("FFE Section memory has Failure \n");

					configASSERT(0);
				
			}

			else {
				ulRemImgSize -= temp;
				ulRxImgSize += temp;
			}
		} while(ulRemImgSize);
#endif /* READBACK_FFE_DMMEM */		
	}

	/* Load FFE sensor SM0 Memory */
	if (sm0TabSzInWord) {
		ql_status = load_fw_s3_mem((uint8_t *)sm0Tab, sm0TabSzInWord* 4, (uint32_t)FFE_SM0);
		if (ql_status != QL_STATUS_OK)
		{
			printf("Load of Firwmare in S3 Memory Failed \n");
			return QL_STATUS_ERROR;
		}
#if (READBACK_FFE_SM0MEM == 1)
		/* Read back Export region of FFE and compare to check data integrity */
		ulRemImgSize= sm0TabSzInWord<< 2;
		ulRxImgSize = 0;
		S3_Reg_Addr = (uint32_t)FFE_SM0; /* Export Region Beginning */
		temp = 4;
		
		do {

			ql_status =  ql_spi_read_s3_mem_ahb(S3_Reg_Addr + ulRxImgSize, (uint8_t *)(buf_cmp), temp);	
			if (ql_status != QL_STATUS_OK)
			{
				printf("ql_spi_read_s3_mem_ahb Failure \n");

					configASSERT(0);
			}
		
			if (*(uint32_t *)buf_cmp != *(uint32_t *)((uint8_t *)sm0Tab+ ulRxImgSize)) {
				printf("FFE Section memory has Failure \n");

					configASSERT(0);
				
			}

			else {
				ulRemImgSize -= temp;
				ulRxImgSize += temp;
			}
		} while(ulRemImgSize);
#endif  /* READBACK_FFE_SM0MEM */
	}

#ifdef FFE_IMAGE_HAS_HEADER_AND_SM1_SUPPORT
	/* Load FFE sensor SM1 Memory */
	if (sm1TabSzInWord) {
		ql_status = load_fw_s3_mem((uint8_t *)sm1Tab, sm1TabSzInWord* 4, (uint32_t)FFE_SM1);
		if (ql_status != QL_STATUS_OK)
		{
			printf("Load of Firwmare in S3 Memory Failed \n");
			return QL_STATUS_ERROR;
		}
#if (READBACK_FFE_SM1MEM == 1)
		/* Read back Export region of FFE and compare to check data integrity */
		ulRemImgSize= sm1TabSzInWord<< 2;
		ulRxImgSize = 0;
		S3_Reg_Addr = (uint32_t)FFE_SM1; /* Export Region Beginning */
		temp = 4;

		do {

			ql_status =  ql_spi_read_s3_mem_ahb(S3_Reg_Addr + ulRxImgSize, (uint8_t *)(buf_cmp), temp);
			if (ql_status != QL_STATUS_OK)
			{
				printf("ql_spi_read_s3_mem_ahb Failure \n");

					configASSERT(0);
			}

			if (*(uint32_t *)buf_cmp != *(uint32_t *)((uint8_t *)sm1Tab+ ulRxImgSize)) {
				printf("FFE Section memory has Failure \n");

					configASSERT(0);

			}

			else {
				ulRemImgSize -= temp;
				ulRxImgSize += temp;
			}
		} while(ulRemImgSize);
#endif  /* READBACK_FFE_SM1MEM */
	}
#endif // FFE_IMAGE_HAS_HEADER_AND_SM1_SUPPORT

	/* Load FFE code Memory */
	if (cmTabSzInWord) {
		ql_status = load_fw_s3_mem((uint8_t *)cmTab, cmTabSzInWord* 4, (uint32_t)FFE_CM);
		if (ql_status != QL_STATUS_OK)
		{
			printf("Load of Firwmare in S3 Memory Failed \n");
			return QL_STATUS_ERROR;
		}
#if (READBACK_FFE_CMMEM == 1)
		/* Read back Export region of FFE and compare to check data integrity */
		ulRemImgSize= cmTabSzInWord<< 2;
		ulRxImgSize = 0;
		S3_Reg_Addr = (uint32_t)FFE_CM; /* Export Region Beginning */
		temp = 4;
		
		do {

			ql_status =  ql_spi_read_s3_mem_ahb(S3_Reg_Addr + ulRxImgSize, (uint8_t *)(buf_cmp), temp);	
			if (ql_status != QL_STATUS_OK)
			{
				printf("ql_spi_read_s3_mem_ahb Failure \n");

					configASSERT(0);
			}
		
			if (*(uint32_t *)buf_cmp != *(uint32_t *)((uint8_t *)cmTab+ ulRxImgSize)) {
				printf("FFE Section memory has Failure \n");

					configASSERT(0);
				
			}

			else {
				ulRemImgSize -= temp;
				ulRxImgSize += temp;
			}
		} while(ulRemImgSize);
#endif /* READBACK_FFE_CMMEM */		
	}

#if VALIDATE_FFE_MEM_WRITE
	free(buf_cmp);
#endif

	return QL_STATUS_OK;

}

/* Download the Firmware using QLSPI */
QL_Status QLSPI_fw_download (SLAVE_DEV_FW_LOAD_T *slave_dev_fw_load_info) 
{
	int ql_status = QL_STATUS_OK;

#ifdef FABRIC_LOADER
			
	printf(" Fabric programmer  size %d bytes \n",slave_dev_fw_load_info->fab_prg_fw_size);

	printf(" Fabric size %d bytes \n",slave_dev_fw_load_info->fab_fw_size);

	if (slave_dev_fw_load_info->fab_prg_fw_addr && slave_dev_fw_load_info->fab_prg_fw_size) {

		ql_status = QLFAB_Fw_Download((uint8_t *)slave_dev_fw_load_info->fab_fw_addr, slave_dev_fw_load_info->fab_fw_size,(uint8_t *)slave_dev_fw_load_info->fab_prg_fw_addr, slave_dev_fw_load_info->fab_prg_fw_size);

		if (ql_status != QL_STATUS_OK)
		{
			printf("Load Fab Firwmare in S3 Memory Failed \n");
			return QL_STATUS_ERROR;
		}
	}
		
#endif

	/* Do and Hold the mcu in reset mode */
	ql_status = reset_slave_mcu();
	if (ql_status != QL_STATUS_OK)
	{
		printf("Reset Slave MCU Failed \n");
		return QL_STATUS_ERROR;
	}

	if (slave_dev_fw_load_info->ffe_fw_addr && slave_dev_fw_load_info->ffe_fw_size) {
		/* Load FFE firmware into FFE Memory */
		ql_status = QLFFE_Fw_Download((uint8_t *)slave_dev_fw_load_info->ffe_fw_addr, slave_dev_fw_load_info->ffe_fw_size);
		if (ql_status != QL_STATUS_OK)
		{
			printf("Load of FFE Firwmare in S3 Memory Failed \n");
			return QL_STATUS_ERROR;
		}
	}
	if (slave_dev_fw_load_info->m4_fw_addr && slave_dev_fw_load_info->m4_fw_size) {
		/* Load M4 firmware into S3 Memory */
		ql_status = load_fw_s3_mem((uint8_t *)slave_dev_fw_load_info->m4_fw_addr, slave_dev_fw_load_info->m4_fw_size, BOOTL1_START_ADDR);
		if (ql_status != QL_STATUS_OK)
		{
			printf("Load of M4 Firwmare in S3 Memory Failed \n");
			return QL_STATUS_ERROR;
		}
	}
        
	/* Release MCU from reset state */
	ql_status = release_slave_mcu();
	if (ql_status != QL_STATUS_OK)
	{
		printf("Load of Firwmare in S3 Memory Failed \n");
		return QL_STATUS_ERROR;
	}

	QLSPI_Read_S3_Mem(MISC_POR_0_ADDR,&ucRxBuf[0],4 );
	printf("POR content after release - 0x%08x \n", *(uint32_t *)ucRxBuf);
	 

	return QL_STATUS_OK;
}



void readback_slave_image(uint32_t rawdata_size)
{

	uint32_t index;
	uint32_t ulRemImgSize;
	uint32_t ulTxImgSize;
	uint32_t ulRemAlign4ByteWords;

	uint8_t ucRxBuf[5];
	uint32_t ulOrigSlaveImgSize;
	
	/* Checking the loaded image content by reading back before releasing
	the CPU */
	ulOrigSlaveImgSize = rawdata_size;
	ulRemImgSize = ulOrigSlaveImgSize;
	ulRemAlign4ByteWords = ulRemImgSize % 4;
	ulTxImgSize = 0x00UL;
	index = 0;
	if (ulRemAlign4ByteWords) {
		ulRemImgSize += (4 - ulRemAlign4ByteWords); 
	}
	printf("\nReading Loaded Image Content\n");
	while (ulRemImgSize) {
		QLSPI_Read_S3_Mem(BOOTL1_START_ADDR + ulTxImgSize,&ucRxBuf[0],4 );
		printf("0x%02x, 0x%02x, 0x%02x, 0x%02x, ", ucRxBuf[0], ucRxBuf[1], ucRxBuf[2], ucRxBuf[3]);		
		
		ulTxImgSize += 4;
		ulRemImgSize -= 4;
		
		if (index >= 2) {
			printf("\n");
			index = 0;
		}
		else {
			index++;
		}		
	}
}
