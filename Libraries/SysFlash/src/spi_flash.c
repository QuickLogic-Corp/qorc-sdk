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
 *    File   : spi_flash.c
 *    Purpose: This file contains driver source code for Macronix MX25U3235FM2I SPI flash.
 *                                                          
 *=========================================================*/

/*
* Note: This code is taken by splitting eoss3_hal_spiflash_test.c file into 2 parts
 *      Part 1: this code whick deals with all SPI Flash related modules
 *      Part 2: Test code in eoss3_hal_spiflash_test.c
 */

/* Standard includes. */
#include "Fw_global_config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "eoss3_dev.h"
#include "FreeRTOS.h"
#include "task.h"
#include <semphr.h>
#include "eoss3_hal_spi.h"
#include "spi_flash.h"
#include "eoss3_hal_uart.h"
#include "s3x_clock_hal.h"
#include "s3x_clock.h"

#include "dbg_uart.h"

/// @cond SPI_FLASH_TEST_GLOBAL_VAR
SPI_HandleTypeDef spiFlashHandle;
SemaphoreHandle_t xLockFlash = NULL;
SPI_Flash_Xfer spiXfer;
uint8_t spiflag = 1;

volatile UINT8_t	  read_complete = 0, write_complete = 0;
static const struct spi_flash_params *spiFlashParams;
uint8_t flashWRCmd[260];

static const struct spi_flash_params {
    UINT16_t idcode;
    UINT16_t page_size;
    UINT16_t pages_per_sector;
    UINT16_t sectors_per_block;
    UINT16_t nr_blocks;
    const char *name;

}spi_flash_table[6+1] = {
		{0x2534,256,16,16,16,"MX25U8035"},
		{0x2536, 256, 16, 16, 64, "MX25U3235"},
        {0x2816, 256, 16, 16, 64, "MX25R3235"},
        {0x2817, 256, 16, 16, 128, "MX25R6435"}, //8MB
		{0x6019, 256, 16, 16, 512, "GD25LQ256C"},
		{0x6016, 256, 16, 16, 64, "GD25LQ32C"},
		{0, 0, 0, 0, 0, 0},
};
/// @endcond

/// @cond SPI_FLASH_DRIVER_FUNS


void flash_write_cb(void)
{
	write_complete = 1;
}

void flash_read_cb(void)
{
	read_complete = 1;
}
void dma_complete(void)
{
	UINT8_t cmd[4];
	unsigned long page_addr;
	size_t chunk_len;

	//check if any pending transfer
	spiXfer.ulTotalXferLen -= spiXfer.ulXferSize;
	spiXfer.ulOffset 	   += spiXfer.ulXferSize;
	chunk_len   		    = (spiXfer.ulTotalXferLen > SPI_FLASH_64KBBLOCK_SIZE) ? SPI_FLASH_64KBBLOCK_SIZE : spiXfer.ulTotalXferLen;

	page_addr = spiXfer.ulOffset / (spiFlashParams->page_size);

	cmd[0] = FLASH_CMD_READ; // FLASH_CMD_FASTREAD;
	cmd[1] = page_addr >> 8;
	cmd[2] = page_addr;
	cmd[3] = spiXfer.ulOffset % (spiFlashParams->page_size);
	// cmd[4] = 0x00; // dummy byte needed only for fast reads

	if(spiXfer.ulTotalXferLen > 0)
	{
        spi_flash_cmd_read(cmd, sizeof(cmd), (UINT8_t*)spiXfer.pRxBuffer+spiXfer.ulXferSize, chunk_len, READ_CMD, NULL);
        spiXfer.pRxBuffer+=spiXfer.ulXferSize;
        // @todo: take care of read failures
	}
}

void flash_write_complete(void)
{
	UINT8_t cmd[4];
	int ret;
	unsigned long page_addr;
	unsigned long byte_addr;
	size_t chunk_len;

	//check if the previous page program is successful or not
	if(spiflash_wait_ready(SPI_FLASH_PROG_TIMEOUT,FLASH_WIP_MASK))
	{
	  if(spiflash_wait_ready(SPI_FLASH_PROG_TIMEOUT,FLASH_WEL_MASK))
	  {
		  if(spiflash_prog_erase_status(SPI_FLASH_PROG_TIMEOUT,FLASH_PROG_FAIL_MASK))
		  {
			  cmd[0] = FLASH_CMD_WREN;

			  ret = spi_flash_cmd(cmd, 1, NULL, 0,CMD_NoResponse);
			  if(ret == FlashOperationSuccess)
			  {
	  				if(IsSPIFlashBusy(FLASH_WEL_MASK) == TRUE)
					{
	   					  //check if any pending transfer
						if(spiXfer.ulTotalXferLen > spiXfer.ulXferSize)
						{
							chunk_len = spiFlashParams->page_size - (spiXfer.ulOffset % spiFlashParams->page_size);
							chunk_len = MIN((spiXfer.ulTotalXferLen - spiXfer.ulXferSize), chunk_len);

							if((chunk_len > (spiFlashParams->page_size/2)) && chunk_len < spiFlashParams->page_size /*&& (chunk_len % 2)*/)
								chunk_len = (spiFlashParams->page_size/2);

						  	page_addr	= (spiXfer.ulOffset / spiFlashParams->page_size);
						  	byte_addr 	= spiXfer.ulOffset % spiFlashParams->page_size;

						  	cmd[0] = FLASH_CMD_PP;
						    cmd[1] = page_addr  >> 8;
						    cmd[2] = page_addr;
						    cmd[3] = byte_addr;

                            ret = spi_flash_program(cmd, sizeof(cmd), (UINT8_t*)spiXfer.pTxBuffer+spiXfer.ulXferSize, chunk_len, NULL);
							if(ret == FlashOperationSuccess)
							{
								spiXfer.ulXferSize += chunk_len;
								spiXfer.ulOffset   += chunk_len;
							}
						}
					}
				}
			}
		}
	}
	else
	{
		printf("Page Programming timed out\n");
	}
}

/*!
 * \fn		static UINT32_t spi_flash_cmd(UINT8_t *cmd, unsigned int cmd_len, void *response, size_t len, FlashCmdType flags)
 * \brief	This function will send SPI flash command to HAL layer
 * \param       *cmd        --- pointer to SPI command buffer
 * \param       cmd_len     --- SPI flash command length
 * \param       *response   --- Pointer to read buffer
 * \param       len         --- number of bytes
 * \param       flags       --- SPI Transfer flag
 * \return      Status
 */
UINT32_t spi_flash_cmd(UINT8_t *cmd, const unsigned int cmd_len, void *response, const size_t len, UINT32_t flags)
{
	int ret = FlashCmdFailed;
	UINT8_t *resp = (UINT8_t *)response;

	spiFlashHandle.Init.ucCmdType = (FlashCmdType)flags;
	if(flags == CMD_NoResponse)
	{
		ret = HAL_SPI_Transmit(&spiFlashHandle,cmd,cmd_len,NULL);
		if(ret != HAL_OK)
		{
			printf("Failed to send command %02x: %d\n",*cmd,ret);
			ret = FlashCmdFailed;
		}
		else
			ret = FlashOperationSuccess;
	}
	else if(flags == CMD_WithResponse || flags == READ_CMD)
	{
		ret = HAL_SPI_TransmitReceive(&spiFlashHandle,cmd,cmd_len,resp,len,NULL);
		if(ret != HAL_OK)
		{
			printf("Failed to send command %02x: %d\n", *cmd, ret);
			ret = FlashCmdFailed;
		}
		else
			ret = FlashOperationSuccess;
	}
	return ret;
}

/*!
 * \fn		static UINT32_t spi_flash_program(UINT8_t *cmd, size_t cmd_len, void *data, size_t data_len)
 * \brief	This function will program SPI flash
 * \param       *cmd        --- pointer to SPI command buffer
 * \param       cmd_len     --- SPI flash command length
 * \param       *data      --- Pointer to buffer
 * \param       len         --- number of bytes
 * \return      Status
 */
UINT32_t spi_flash_program(UINT8_t *cmd, size_t cmd_len, void *data, size_t data_len, void (*ptrFunc)(void))
{
	int ret = FlashCmdFailed;
	FlashCmdType flags = PROGRAM_CMD;

	//Add 4 bytes of CMD before the data bytes
	memcpy(&flashWRCmd[0],cmd,cmd_len);
	memcpy(&flashWRCmd[cmd_len],data,(data_len));

    if(!((cmd_len + data_len) % 2))
		flags = PROGRAM_CMD;
	else
		flags = CMD_NoResponse;

	if(data_len != 0)
	{
		spiFlashHandle.Init.ucCmdType = (FlashCmdType)flags;
        if(ptrFunc)
			ret = HAL_SPI_Transmit(&spiFlashHandle,flashWRCmd,((data_len) + cmd_len), ptrFunc);
		else
			ret = HAL_SPI_Transmit(&spiFlashHandle,flashWRCmd,((data_len) + cmd_len),NULL);

		if(ret != HAL_OK)
		{
			printf("Failed to send command (%u bytes): %d\n",
					(data_len + 2), ret);

			ret = FlashCmdFailed;
		}
		else
			ret = FlashOperationSuccess;
	}
	return ret;
}

/*!
 * \fn		static UINT32_t spi_flash_probe_macronix(UINT8_t *idcode)
 * \brief	This function will compute SPI flash size, page size, sector size
 * \param       *id_code    --- SPI Flash Id
 * \return      Status
 */
static UINT32_t flash_probe(UINT8_t *idcode)
{
	unsigned int i;
	unsigned int flash_size;
	unsigned long sector_size;
	int ret = FlashNotFound;
	UINT16_t id = (idcode[2] | (idcode[1] << 8));

	for(i = 0; i < ARRAY_SIZE(spi_flash_table); i++) {
		spiFlashParams = &spi_flash_table[i];
		if(spiFlashParams->idcode == id)
		{
			ret = FlashMatchFound;
			break;
		}
	}

	if(i == ARRAY_SIZE(spi_flash_table)) {
		//printf("Unsupported Macronix ID %04x\n", id);
		return FlashNotFound;
	}

	sector_size = spiFlashParams->page_size * spiFlashParams->pages_per_sector
						* spiFlashParams->sectors_per_block;

    flash_size = sector_size * spiFlashParams->nr_blocks;

    (void)(flash_size);
    //printf("Detected %s with page size %u, sector size %lu, total %u bytes\r\n",
	//		spiFlashParams->name,spiFlashParams->page_size,sector_size,flash_size);

	return ret;
}

/*!
 * \fn		static UINT32_t spi_flash_cmd_read(UINT8_t *cmd, size_t cmd_len, void *data, size_t data_len, FlashCmdType flags)
 * \brief	This function will send command to read from SPI flash
 * \param       *cmd       --- pointer to SPI command buffer
 * \param       cmd_len    --- SPI flash command length
 * \param       *data      --- Pointer to buffer
 * \param       data_len   --- number of bytes
 * \param       flags      --- SPI Transfer Flags
 * \return      Status
 */
UINT32_t spi_flash_cmd_read(UINT8_t *cmd, size_t cmd_len, void *data, size_t data_len, UINT32_t flags, void (*ptrFunc)(void))
{
	int ret = FlashOperationSuccess;

	spiFlashHandle.Init.ucCmdType = (FlashCmdType)flags;
	ret = HAL_SPI_TransmitReceive(&spiFlashHandle,cmd,cmd_len,data,data_len,ptrFunc);
	if(ret != HAL_OK)
	{
		printf("Failed to send read command (%u bytes): %d\n",
				cmd_len, ret);
		ret = FlashCmdFailed;
	}
	else
		ret = FlashOperationSuccess;

	return ret;
}

/*!
 * \fn		static UINT8_t IsSPIFlashBusy(UINT8_t bitmask)
 * \brief	This function will read the SPI flash status
 * \param       bitmask       --- bit mask to check
 * \return      Status
 */
UINT8_t IsSPIFlashBusy(UINT8_t bitmask)
{
	uint8_t ucDataBuffer;
	int ret;
	uint8_t ucCmd = FLASH_CMD_RDSR;

	ret = spi_flash_cmd(&ucCmd, sizeof(ucCmd), &ucDataBuffer, 1, CMD_WithResponse);
	if(ret == FlashOperationSuccess)
	{
		if( (ucDataBuffer & bitmask) == bitmask )
			ret = TRUE;
		else
			ret = FALSE;
	}
	return ret;
}

UINT8_t IsProg_EraseFail(UINT8_t bitmask)
{
    UINT8_t ucDataBuffer;
    int ret;
    UINT8_t cmd = FLASH_CMD_RDSCUR;

    ret = spi_flash_cmd(&cmd, sizeof(cmd), &ucDataBuffer, 1 , CMD_WithResponse);
    if(ret == FlashOperationSuccess)
    {
        if((ucDataBuffer & bitmask) == bitmask)
            ret = TRUE;
        else
            ret = FALSE;
    }

    return ret;
}

/*!
 * \fn		static UINT32_t macronix_wakeup(void)
 * \brief	This function will send command to wake up SPI flash
 * \return      Status
 */
static UINT32_t spiflash_wakeup(void)
{
	int ret;
	int i = 1024;
	uint8_t cmd = FLASH_CMD_RDP;

	//release from deep power down
	ret = spi_flash_cmd(&cmd, sizeof(cmd), NULL,0, CMD_NoResponse);
	if(ret == FlashOperationSuccess)
	{
		//Keep the CS at least 8us to wake up from deep power down
		do
		{
			i--;
		}while(i>0);

		return FALSE;
	}

	return TRUE;
}

/*!
 * \fn		static UINT32_t macronix_wait_ready(unsigned long timeout)
 * \brief	This function will read SPI flash status to poll for SPI write operation completion
 * \return      Status
 */
UINT32_t spiflash_wait_ready(unsigned long timeout,UINT8_t bitmask)
{
	uint32_t temp = 0;

	while( IsSPIFlashBusy(bitmask) )
	{
		if(temp > timeout)
        {
            printf("timeout occurre\r\n");
			return FALSE;
        }

		temp = temp + 1;
	}

	return TRUE;
}

UINT32_t spiflash_prog_erase_status(unsigned long timeout, UINT8_t bitmask)
{
      UINT32_t temp = 0;

      while(IsProg_EraseFail(bitmask))
      {
          if(temp > timeout)
          {
              printf("time out occured\r\n");
              return FALSE;
          }
          temp = temp + 1;
      }

      return TRUE;
}

/*!
 * \fn		static UINT32_t macronix_chiperase(void)
 * \brief	This function will erase the whole SPI flash
 * \param       None
 * \return      Status
 */

UINT32_t spiflash_chiperase(void)
{
  	int ret = 0;
	uint8_t cmd[4];

	cmd[1] = 0;
	cmd[2] = 0;
	cmd[3] = 0;

	cmd[0] = FLASH_CMD_WREN;
	ret = spi_flash_cmd(cmd, 1, NULL, 0, CMD_NoResponse);
	if(ret != FlashOperationSuccess) {
		printf("Enabling Write Enable Latch bit failed\n");
		return FlashCmdFailed;
	}

	if(IsSPIFlashBusy(FLASH_WEL_MASK) == FALSE)
		return 0;

	cmd[0] = FLASH_CMD_CE;

	ret = spi_flash_cmd(cmd, 1, NULL, 0, CMD_NoResponse);
	if(ret != FlashOperationSuccess) {
		printf("Chip Erase Failed\n");
		return FlashCmdFailed;
	}

	if(spiflash_wait_ready(SPI_FLASH_PROG_TIMEOUT,FLASH_WIP_MASK))
	{
	    if(!(spiflash_prog_erase_status(SPI_FLASH_PROG_TIMEOUT,FLASH_ERASE_FAIL_MASK)))
	    {
		ret = FlashCmdFailed;
		return ret;
	    }
	}
	else
	{
		printf("Page Programming timed out\n");
		ret = FlashTimeOut;
	}

	cmd[0] = FLASH_CMD_WRDI;
	ret = spi_flash_cmd(cmd, 1, NULL, 0, CMD_NoResponse);
	if(ret != FlashOperationSuccess) {
		printf("Resetting Write Enable Latch bit\n");
	}

	return ret;
}

/*!
 * \fn		static UINT32_t spiflash_SE(UINT32_t offset, size_t len)
 * \brief	This function will send sector erase command to SPI flash
 * \param       offset   --- Offset
 * \param       len      --- number of bytes
 * \return      Status
 */
static UINT32_t spiflash_SE(UINT32_t offset, UINT8_t ucNumOfBlk)
{
	size_t actual;
	size_t len;
	int ret = 0;
	UINT8_t cmd[4];

	len = ucNumOfBlk * SPI_FLASH_SECTOR_SIZE;

	if(len % SPI_FLASH_SECTOR_SIZE) {
		printf("Erase offset/length not multiple of sector size\r\n");
		return FlashInvalidParams;
	}

	len /= SPI_FLASH_SECTOR_SIZE;
	cmd[2] = 0;
	cmd[3] = 0;

	cmd[0] = FLASH_CMD_WREN;
	ret = spi_flash_cmd(cmd, 1, NULL, 0, CMD_NoResponse);
	if(ret != FlashOperationSuccess) {
		printf("Enabling Write Enable Latch bit failed\n");
		return FlashCmdFailed;
	}

	if(IsSPIFlashBusy(FLASH_WEL_MASK) == FALSE)
		return 0;

	for(actual = 0; actual < len; actual++)
	{
        offset += actual;
		cmd[0] = FLASH_CMD_SE;
	  	//cmd[1] = (offset / SPI_FLASH_SECTOR_SIZE) + actual;
	  	cmd[1] = offset >> 16;
		cmd[2] = offset >> 8;
		cmd[3] = offset;

		ret = spi_flash_cmd(cmd, 4, NULL, 0, CMD_NoResponse);
		if(ret != FlashOperationSuccess) {
			printf("Block Erase Failed\n");
			break;
		}

		if(spiflash_wait_ready(SPI_FLASH_PROG_TIMEOUT,FLASH_WIP_MASK))
        {
            if(!(spiflash_prog_erase_status(SPI_FLASH_PROG_TIMEOUT,FLASH_ERASE_FAIL_MASK)))
            {
                ret = FlashCmdFailed;
                return ret;
            }
        }
        else
		{
			printf("Page Programming timed out\n");
			ret = FlashTimeOut;
			break;
		}
	}

	cmd[0] = FLASH_CMD_WRDI;
	ret = spi_flash_cmd(cmd, 1, NULL, 0, CMD_NoResponse);
	if(ret != FlashOperationSuccess) {
		printf("Resetting Write Enable Latch bit\n");
	}

	return ret;
}

/*!
 * \fn		static UINT32_t spiflash_64KBlkErase(UINT32_t offset, UINT8_t ucNumOfBlk)
 * \brief	This function will erase 64KB block on SPI flash
 * \param       offset   --- Offset
 * \param       ucNumOfBlk --- number of blocks
 * \return      Status
 */
static UINT32_t spiflash_64KBlkErase(UINT32_t offset, UINT8_t ucNumOfBlk)
{
	size_t actual;
	size_t len;
	int ret = 0;
	UINT8_t cmd[4];

	//printf("ucNumBlks = %d\r\n",ucNumOfBlk);

	len = ucNumOfBlk * SPI_FLASH_64KBBLOCK_SIZE;

	if(len % SPI_FLASH_64KBBLOCK_SIZE) {
		printf("Erase offset/length not multiple of sector size\r\n");
		return FlashInvalidParams;
	}

	len /= SPI_FLASH_64KBBLOCK_SIZE;
	cmd[2] = 0;
	cmd[3] = 0;

	cmd[0] = FLASH_CMD_WREN;
	ret = spi_flash_cmd(cmd, 1, NULL, 0, CMD_NoResponse);
	if(ret != FlashOperationSuccess) {
		printf("Enabling Write Enable Latch bit failed\n");
		return FlashCmdFailed;
	}

	if(IsSPIFlashBusy(FLASH_WEL_MASK) == FALSE)
		return 0;

	for(actual = 0; actual < len; actual++)
	{
        offset += actual;
		cmd[0] = FLASH_CMD_BE64K;
	  	//cmd[1] = (offset / SPI_FLASH_64KBBLOCK_SIZE) + actual;
	  	cmd[1] = offset >> 16;
		cmd[2] = offset >> 8;
		cmd[3] = offset;

		ret = spi_flash_cmd(cmd, 4, NULL, 0, CMD_NoResponse);
		if(ret != FlashOperationSuccess) {
			printf("Block Erase Failed\n");
			break;
		}

		if(spiflash_wait_ready(SPI_FLASH_PROG_TIMEOUT,FLASH_WIP_MASK))
                {
                    if(!(spiflash_prog_erase_status(SPI_FLASH_PROG_TIMEOUT,FLASH_ERASE_FAIL_MASK)))
                    {
                        ret = FlashCmdFailed;
                        return ret;
                    }
                }
                else
		{
			printf("Page Programming timed out\n");
			ret = FlashTimeOut;
			break;
		}
	}

	cmd[0] = FLASH_CMD_WRDI;
	ret = spi_flash_cmd(cmd, 1, NULL, 0, CMD_NoResponse);
	if(ret != FlashOperationSuccess) {
		printf("Resetting Write Enable Latch bit\n");
	}

	return ret;
}

/*!
 * \fn		static UINT32_t spiflash_32KBlkErase(UINT32_t offset, UINT8_t ucNumOfBlk)
 * \brief	This function will erase 32KB block on SPI flash
 * \param       offset   --- Offset
 * \param       ucNumOfBlk --- number of blocks
 * \return      Status
 */
static UINT32_t spiflash_32KBlkErase(UINT32_t offset, UINT8_t ucNumOfBlk)
{
	size_t actual;
	size_t len;
	int ret = 0;
	UINT8_t cmd[4];

	//printf("ucNumBlks = %d\r\n",ucNumOfBlk);

	len = ucNumOfBlk * SPI_FLASH_32KBBLOCK_SIZE;

	if(len % SPI_FLASH_32KBBLOCK_SIZE) {
		printf("Erase offset/length not multiple of sector size\r\n");
		return FlashInvalidParams;
	}

	len /= SPI_FLASH_32KBBLOCK_SIZE;
	cmd[2] = 0;
	cmd[3] = 0;

	cmd[0] = FLASH_CMD_WREN;
	ret = spi_flash_cmd(cmd, 1, NULL, 0, CMD_NoResponse);
	if(ret != FlashOperationSuccess) {
		printf("Enabling Write Enable Latch bit failed\n");
		return FlashCmdFailed;
	}

	if(IsSPIFlashBusy(FLASH_WEL_MASK) == FALSE)
		return 0;

	for(actual = 0; actual < len; actual++)
	{
        offset += actual;
		cmd[0] = FLASH_CMD_BE32K;
	  	//cmd[1] = (offset / SPI_FLASH_32KBBLOCK_SIZE) + actual;
	  	cmd[1] = offset >> 16;
		cmd[2] = offset >> 8;
		cmd[3] = offset;

		ret = spi_flash_cmd(cmd, 4, NULL, 0, CMD_NoResponse);
		if(ret != FlashOperationSuccess) {
			printf("Block Erase Failed\n");
			break;
		}

		if(spiflash_wait_ready(SPI_FLASH_PROG_TIMEOUT,FLASH_WIP_MASK))
                {
                    if(!(spiflash_prog_erase_status(SPI_FLASH_PROG_TIMEOUT,FLASH_ERASE_FAIL_MASK)))
                    {
                        ret = FlashCmdFailed;
                        return ret;
                    }
                }
                else
		{
			printf("Page Programming timed out\n");
			ret = FlashTimeOut;
			break;
		}
	}

	cmd[0] = FLASH_CMD_WRDI;
	ret = spi_flash_cmd(cmd, 1, NULL, 0, CMD_NoResponse);
	if(ret != FlashOperationSuccess) {
		printf("Resetting Write Enable Latch bit\n");
	}

	return ret;
}

UINT32_t spiflash_erase(UINT32_t ulOffset, size_t len)
{
	UINT32_t ucNumBlks;
	UINT32_t erase_offset = ulOffset;
	int ret = 0;
	ucNumBlks = (len % SPI_FLASH_SECTOR_SIZE) ? ((len / SPI_FLASH_SECTOR_SIZE) +1) : (len / SPI_FLASH_SECTOR_SIZE);

	lock_flash();

	while(ucNumBlks)
	{
		if(ucNumBlks >= 16)
		{
			ret = spiflash_64KBlkErase(erase_offset, 1);
			if(ret != FlashOperationSuccess)
			{
				unlock_flash();
                printf("Erase 64KB Block is failed\r\n");
				return ret;
			}
			//else
			//	printf("Erase 64KB Block is successful\r\n");

			ucNumBlks -= 16;
			erase_offset += (16*SPI_FLASH_SECTOR_SIZE);
		}
		else if(ucNumBlks >= 8)
		{
			ret = spiflash_32KBlkErase(erase_offset, 1);
			if(ret != FlashOperationSuccess)
			{
				unlock_flash();
                printf("Erase 32KB Block is failed\r\n");
				return ret;
			}
			//else
			//	printf("Erase 32KB Block is successful\r\n");

			ucNumBlks -= 8;
			erase_offset += (8*SPI_FLASH_SECTOR_SIZE);
		}
		else
		{
			ret = spiflash_SE(erase_offset, 1);
			if(ret != FlashOperationSuccess)
			{
				unlock_flash();
                printf("Erase 4k Sector is failed\r\n");
				return ret;
			}
			//else
			//	printf("Erase 4k Sector is successful\r\n");

			ucNumBlks -= 1;
			erase_offset += (1*SPI_FLASH_SECTOR_SIZE);
		}
	}

	unlock_flash();

	return FlashOperationSuccess;
}


/*!
 * \fn		static UINT32_t spiflash_write(UINT32_t offset, size_t len, void *buf)
 * \brief	This function will send write command to SPI flash
 * \param       offset   --- Offset
 * \param       len      --- number of bytes
 * \param       *buf     --- pointer to buffer
 * \return      Status
 */
UINT32_t spiflash_write(UINT32_t offset, size_t len, void *buf, void (*ptrFunc)(void))
{
	unsigned long page_addr;
	unsigned long byte_addr;
	size_t chunk_len;
	size_t actual;
	int ret = 0;
	uint8_t cmd[4];

    if(len ==0 || buf == NULL)
	{
		printf("%s: Invalid Parameters received, len %d\r\n",__func__,len);
		return FlashInvalidParams;
	}

	lock_flash();

	//wake up the flash
	if(spiflash_wakeup())
	{
		printf("Unable to wake up SPI Flash !!!\r\n");
		unlock_flash();
		return 0;
	}
	//check flash is busy or not
	if(IsSPIFlashBusy(FLASH_WIP_MASK))
    {
        printf("FLASH_WIP_MASK is set\r\n");
		unlock_flash();
		return 0;
	}

   	//synopsys TX FIFO is 16-bits wide and 133 (16-bits) words depth
	for(actual = 0; actual < len; actual += chunk_len)
	{
		page_addr = offset / spiFlashParams->page_size;
		byte_addr = offset % spiFlashParams->page_size;

		chunk_len = spiFlashParams->page_size - (offset%spiFlashParams->page_size);
		chunk_len = MIN(len, chunk_len);

		if((chunk_len > (spiFlashParams->page_size/2)) && chunk_len < spiFlashParams->page_size)
			chunk_len = (spiFlashParams->page_size/2);

		cmd[0] = FLASH_CMD_PP;
		cmd[1] = (page_addr >> 8);

		cmd[2] = page_addr;
		cmd[3] = byte_addr;

		cmd[0] = FLASH_CMD_WREN;

		ret = spi_flash_cmd(cmd, 1, NULL, 0, CMD_NoResponse);
		if(ret != FlashOperationSuccess) {
			printf("Enabling Write Enable Latch bit failed\n");
			unlock_flash();
			break;
		}

		if(IsSPIFlashBusy(FLASH_WEL_MASK) == FALSE)
		{
			printf("FLASH_WEL_MASK is NOT set\r\n");
			unlock_flash();
			return 0;
		}

	    cmd[0] = FLASH_CMD_PP;

        if(ptrFunc)
		{
			spiXfer.pTxBuffer = buf;
			spiXfer.ulXferSize = chunk_len;
			spiXfer.ulTotalXferLen = len;
			spiXfer.SPI_TxRxComplCallback = ptrFunc;
			spiXfer.ulOffset = (offset + chunk_len);

            ret = spi_flash_program(cmd, 4, (UINT8_t*)buf, chunk_len, NULL);
			if(ret == FlashOperationSuccess)
			{
				do
				{
					write_complete = 0;

					if(spiXfer.ulTotalXferLen > spiXfer.ulXferSize)
						flash_write_complete();

				}while(spiXfer.ulTotalXferLen != spiXfer.ulXferSize);
			}

			if(spiflash_wait_ready(SPI_FLASH_PROG_TIMEOUT,FLASH_WIP_MASK))
			{
				if(spiflash_wait_ready(SPI_FLASH_PROG_TIMEOUT,FLASH_WEL_MASK))
				{
					if(spiflash_prog_erase_status(SPI_FLASH_PROG_TIMEOUT,FLASH_PROG_FAIL_MASK))
					{
						cmd[0] = FLASH_CMD_WRDI;
						ret = spi_flash_cmd(cmd, 1, NULL, 0, CMD_NoResponse);
						if(ret != FlashOperationSuccess) {
							printf("Resetting Write Enable Latch bit\n");
						}
					}
				}
			}

			unlock_flash();
			if(ptrFunc)
				ptrFunc();

			return ret;
		}
		else
		{
    		ret = spi_flash_program(cmd, 4, (UINT8_t*)buf + actual, chunk_len, NULL);
			if(ret != FlashOperationSuccess) {
				printf("Page Program Failed\n");
				unlock_flash();
				break;
			}

			if(spiflash_wait_ready(SPI_FLASH_PROG_TIMEOUT,FLASH_WIP_MASK))
			{
				if(spiflash_wait_ready(SPI_FLASH_PROG_TIMEOUT,FLASH_WEL_MASK))
				{
					if(spiflash_prog_erase_status(SPI_FLASH_PROG_TIMEOUT,FLASH_PROG_FAIL_MASK))
					{
						ret = FlashOperationSuccess;
						actual += chunk_len;
						offset += chunk_len;
					}
				}
			}
			else
			{
				printf("Page Programming timed out\n");
				ret = FlashTimeOut;
				unlock_flash();
				break;
			}
		}
	}

	cmd[0] = FLASH_CMD_WRDI;
	ret = spi_flash_cmd(cmd, 1, NULL, 0, CMD_NoResponse);
	if(ret != FlashOperationSuccess) {
		printf("Resetting Write Enable Latch bit\n");
	}

	unlock_flash();

	return ret;
}
//to make the read fn use interrupts
static void temp_read_complete(void)
{
}

/*!
 * \fn		static UINT32_t spiflash_read_fast(UINT32_t offset, size_t len, void *buf, void (*ptrFunc)(void))
 * \brief	This function will send read command to SPI flash
 * \param       offset   --- Offset
 * \param       len      --- number of bytes
 * \param       *buf     --- pointer to buffer
 * \return      Status
 */
UINT32_t spiflash_read_fast(UINT32_t offset, size_t len, void *buf, void (*ptrFunc)(void))
{
	unsigned long page_addr;
	unsigned long page_size;
	uint8_t cmd[4];
    int ret = 0,temp;
	uint32_t chunk_len;
	uint32_t actual = 0;

	if(len ==0 || buf == NULL)
	{
		printf("%s: Invalid len %d\r\n",__func__,len);
		return FlashInvalidParams;
	}

	lock_flash();

    //to use the interrupt instead of polling loop
    if(!ptrFunc)
    {
       ptrFunc = temp_read_complete;   	//?? Should it be uncommented 
    }

	do
	{
		chunk_len = (len > SPI_FLASH_64KBBLOCK_SIZE) ? SPI_FLASH_64KBBLOCK_SIZE : len;

    	page_size = (spiFlashParams->page_size);
    	page_addr = offset / spiFlashParams->page_size;

    	cmd[0] = FLASH_CMD_READ; // FLASH_CMD_FASTREAD;
    	cmd[1] = page_addr >> 8;
    	cmd[2] = page_addr;
    	cmd[3] = offset % page_size;
    	// cmd[4] = 0x00; // dummy byte needed only for fast reads

        if(ptrFunc)
		{
			spiXfer.ulTotalXferLen = len;
			spiXfer.ulXferSize = chunk_len;
			spiXfer.SPI_TxRxComplCallback = ptrFunc;
			spiXfer.pRxBuffer = buf;
			spiXfer.ulOffset = offset;

            ret = spi_flash_cmd_read(cmd, sizeof(cmd), (UINT8_t*)buf, chunk_len, READ_CMD, NULL );

			do{
				read_complete = 0;

                dma_complete();
			}while(spiXfer.ulTotalXferLen > 0);

			unlock_flash();
			if(ptrFunc)
				ptrFunc();

			return ret;
		}
		else
		{
			ret = spi_flash_cmd_read(cmd, sizeof(cmd), (UINT8_t*)buf+actual, chunk_len, READ_CMD, NULL);
			if(ret == FlashOperationSuccess)
			{
				//wait for previous SPI flash read to complete
				do
				{
					temp = 30000;
					do
					{
						temp--;
					}while(temp>0 );

					if(DMA_SPI_MS->DMA_CTRL & DMA_CTRL_STOP_BIT)
					{
						SPI_DMA_Complete();
						break;
					}

				}while(1);

				len -= chunk_len;
				offset += chunk_len;
				actual += chunk_len;
			}
		}
	}while(len > 0);

	unlock_flash();

	return ret;
}

UINT32_t SPI_flash_probe(void)
{
	uint8_t idcode[3];
	uint8_t cmd = FLASH_CMD_RDID;
	int ret;

	ret = spi_flash_cmd(&cmd, 1, &idcode, sizeof(idcode), CMD_WithResponse);
	if(ret != FlashOperationSuccess)
		goto err_read_id;

	//printf("SPI Flash idcode %02x %02x %02x \r\n", idcode[0],idcode[1], idcode[2]);

	switch(idcode[0]) {
	case 0xc2:
    case 0xc8:
        ret = flash_probe(idcode);
        break;

    default:
    	printf("In compatible SPI Flash ID\r\n");
    	ret = FlashNotFound;
    	break;
	}

	return ret;

err_read_id:
	printf("Failed SPI Flash ID command\r\n");
	return FlashCmdFailed;

}
void unlock_flash(void)
{
	if(xLockFlash)
	{
		if (xSemaphoreGive(xLockFlash) != pdTRUE)
		{
			printf("[SPI Flash Driver] : Error : unable to release lock of flash\n");
		}
	}
}

void lock_flash(void)
{
	if(xLockFlash)
	{
		if (xSemaphoreTake(xLockFlash, portMAX_DELAY) != pdTRUE)
		{
			printf("[SPI Flash Driver] : Error unable to take lock of flash \n");
			return;
		}
	}
}

uint32_t flash_boot_setup(void)
{
    uint32_t    ret = 0;

    //SPI master init for SPI flash
    spiFlashHandle.Init.ucFreq       = SPI_BAUDRATE_10MHZ;
    spiFlashHandle.Init.ucSPIInf     = SPI_4_WIRE_MODE;
 //   SpiHandle.Init.ucSPIMode    = SPI_MODE_0;
    spiFlashHandle.Init.ucSSn        = SPI_SLAVE_1_SELECT;
    spiFlashHandle.Init.ulCLKPhase   = SPI_PHASE_1EDGE;
    spiFlashHandle.Init.ulCLKPolarity = SPI_POLARITY_LOW;
    spiFlashHandle.Init.ulDataSize   = SPI_DATASIZE_8BIT;
    spiFlashHandle.Init.ulFirstBit   = SPI_FIRSTBIT_MSB;
    spiFlashHandle.ucSPIx            = SPI1_MASTER_SEL;

	//S3x_Clk_Enable(S3X_A1_CLK);							?? Should it be umcommented
	//S3x_Clk_Enable(S3X_CFG_DMA_A1_CLK);

    if(ret=HAL_SPI_Init(&spiFlashHandle) != HAL_OK)
    {
        printf("HAL_SPI1_Init failed\r\n");
        return 1;
    }

    if (ret=SPI_flash_probe() != FlashMatchFound)
    {
        printf("SPI_flash_probe failed\r\n");
        return 1;
    }

	return ret;
}

/*
* Check if we can communicate with Flash chip
* Note: We do not validate the chip ID. Just assume it is 256byte pages
* and alleast 1MB size for BL0 purpose. And set some Flash params.
*/
uint8_t read_flash_id(void) {
  uint8_t idcode[3];
  uint8_t cmd = FLASH_CMD_RDID;
  int ret;

  idcode[0] = 0;
  ret = spi_flash_cmd(&cmd, 1, &idcode, sizeof(idcode), CMD_WithResponse);
  if(ret != FlashOperationSuccess)   {
    printf("Failed SPI Flash ID command\r\n");
  }
  //set it to any params, since only page size required for BL0
  spiFlashParams = &spi_flash_table[0];
  return idcode[0];
}

/// @endcondw
