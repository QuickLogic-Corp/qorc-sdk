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
 *    File   : spi_flash.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

#ifndef __SPI_FLASH_H_
#define __SPI_FLASH_H_

#include "eoss3_hal_spi.h"

/*** MX25 series command hex code definition ***/
//ID comands
#define    FLASH_CMD_RDID      0x9F    //RDID (Read Identification)
#define    FLASH_CMD_RES       0xAB    //RES (Read Electronic ID)
#define    FLASH_CMD_REMS      0x90    //REMS (Read Electronic & Device ID)
#define    FLASH_CMD_REMS2     0xEF    //REMS2 (Read ID for 2 x I/O mode)
#define    FLASH_CMD_REMS4     0xDF    //REMS4 (Read ID for 4 x I/O mode)

//Register comands
#define    FLASH_CMD_WRSR      0x01    //WRSR (Write Status Register)
#define    FLASH_CMD_RDSR      0x05    //RDSR (Read Status Register)
#define    FLASH_CMD_WRSCUR    0x2F    //WRSCUR (Write Security Register)
#define    FLASH_CMD_RDSCUR    0x2B    //RDSCUR (Read Security Register)

//READ comands
#define    FLASH_CMD_READ        0x03    //READ (1 x I/O)
#define    FLASH_CMD_2READ       0xBB    //2READ (2 x I/O)
#define    FLASH_CMD_4READ       0xEB    //4READ (4 x I/O)
#define    FLASH_CMD_FASTREAD    0x0B    //FAST READ (Fast read data)

//Program comands
#define    FLASH_CMD_WREN     0x06    //WREN (Write Enable)
#define    FLASH_CMD_WRDI     0x04    //WRDI (Write Disable)
#define    FLASH_CMD_PP       0x02    //PP (page program)
#define    FLASH_CMD_4PP      0x38    //4PP (Quad page program)
#define    FLASH_CMD_CP       0xAD    //CP (Continously program)

//Erase comands
#define    FLASH_CMD_SE       0x20    //SE (Sector Erase)
#define    FLASH_CMD_BE32K    0x52    //BE32K (Block Erase 32kb)
#define    FLASH_CMD_BE64K    0xD8    //BE (Block Erase)
#define    FLASH_CMD_CE       0x60    //CE (Chip Erase) hex code: 60 or C7

//Mode setting comands
#define    FLASH_CMD_DP       0xB9    //DP (Deep Power Down)
#define    FLASH_CMD_RDP      0xAB    //RDP (Release form Deep Power Down)
#define    FLASH_CMD_ENSO     0xB1    //ENSO (Enter Secured OTP)
#define    FLASH_CMD_EXSO     0xC1    //EXSO  (Exit Secured OTP)
#define    FLASH_CMD_ESRY     0x70    //ESRY (Enable SO to output RY/BY)
#define    FLASH_CMD_DSRY     0x80    //DSRY (Enable SO to output RY/BY)
#define    FLASH_CMD_HDE      0xAA    //HDE (Hold# enable)

// Flash control register mask define
// status register
#define    FLASH_WIP_MASK         0x01
#define    FLASH_WEL_MASK	      0x02
#define    FLASH_QE_MASK          0x80

#define    FLASH_PROG_FAIL_MASK    0x20
#define    FLASH_ERASE_FAIL_MASK   0x40

#define SPI_FLASH_PROG_TIMEOUT  7000000

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define SPI_FLASH_64KBBLOCK_SIZE	65536
#define SPI_FLASH_32KBBLOCK_SIZE	32768
#define SPI_FLASH_SECTOR_SIZE		4096

#define FLASH_PAGE_SIZE                 256
#define ALIGN_2_PAGESIZE(X)             (X = X + (FLASH_PAGE_SIZE - (X % FLASH_PAGE_SIZE)))

#define MIN(a,b) (((a) < (b)) ? (a):(b))
#define MAX(a,b) (((a) > (b)) ? (a):(b))

typedef enum
{
    SECTOR_ERASE = 0,
    BLOCK_32K_ERASE,
    BLOCK_64K_ERASE,
    CHIP_ERASE,    
}Flash_Erase_Type;

typedef struct
{	
	uint8_t  *pTxBuffer;						/*! Pointer to SPI Tx transfer Buffer */
	uint8_t  *pRxBuffer;						/*! Pointer to SPI Rx transfer Buffer */
	uint32_t  ulTotalXferLen;    				/*! Total transfer count */
	uint32_t  ulXferSize;					    /*! Transfer size */
	uint32_t  ulOffset;							/*! Offset */
	void 	  (*SPI_TxRxComplCallback)(void);
	
}SPI_Flash_Xfer;

//set this to for any Flash utilities
extern SPI_HandleTypeDef spiFlashHandle;

void notify_dma_complete(void);
uint32_t spi_flash_cmd(uint8_t *cmd, const unsigned int cmd_len, void *response, const size_t len, uint32_t flags);
uint32_t spi_flash_cmd_read(uint8_t *cmd, size_t cmd_len, void *data, size_t data_len, uint32_t flags, void (*ptrFunc)(void));
uint8_t IsFlashBusy(uint8_t bitmask);
uint8_t IsProg_EraseFail(uint8_t bitmask);
//uint32_t macronix_wait_ready(unsigned long timeout,uint8_t bitmask);
//uint32_t macronix_prog_erase_status(unsigned long timeout, uint8_t bitmask);
uint32_t spi_flash_program(uint8_t *cmd, size_t cmd_len, void *data, size_t data_len, void (*ptrFunc)(void));
uint32_t SPI_flash_probe(void);
// FBR
uint8_t IsSPIFlashBusy(uint8_t bitmask);
void unlock_flash(void);
void lock_flash(void);
uint32_t spiflash_wait_ready(unsigned long timeout,uint8_t bitmask);
uint32_t spiflash_prog_erase_status(unsigned long timeout, uint8_t bitmask);
uint32_t spiflash_read_fast(uint32_t offset, size_t len, void *buf, void (*ptrFunc)(void));
uint32_t spiflash_erase(uint32_t ulOffset, size_t len);
uint32_t spiflash_write(uint32_t offset, size_t len, void *buf, void (*ptrFunc)(void));
uint32_t spiflash_chiperase(void);
uint32_t flash_boot_setup(void);
uint8_t read_flash_id(void);
#endif /* __SPI_FLASH_H_ */
