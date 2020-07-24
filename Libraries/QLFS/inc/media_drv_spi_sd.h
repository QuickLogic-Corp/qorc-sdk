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
 *    File   : media_drv_spi_sd.h
 *    Purpose: This file contains driver definition code for Micro SD card.
 *                                                          
 *=========================================================*/

#ifndef MEDIA_DRV_SPI_SD_H
#define MEDIA_DRV_SPI_SD_H

/* FreeRTOS+FAT includes. */
#include "ff_headers.h"
#include "ff_stdio.h"

/*******************************************************************************
 *                                          Constants
 */

#define SPI_FLASH_ERASE_SIZE	         (4096)
#define SPISD_DISK_SECTOR_SIZE	         (512)
#define SPISD_DISK_SECTOR_FACTOR	     (SPISD_DISK_SECTOR_SIZE/512) // = 1 for 512, ==8 for 4096
#define SPISD_DISK_NAME			        "/SPISD"  /* Where the SPIFLASH disk is mounted. */
#define SPI_SD_IO_MANAGER_CACHE_SIZE	 (4096)   /* @TBD this may need fine tuning */
 
#define SPI_HIDDEN_SECTOR_COUNT           1
#define SPI_SD_PRIMARY_PARTITIONS         1
#define SPI_SD_PARTITION_NUMBER           0

/* Used as a magic number to indicate that an FF_Disk_t structure is a SPI SD disk. */
#define SPISD_SIGNATURE			          0x41404342UL

/*********************************************************************
 * TYPEDEFS
 */

typedef struct xSD_Handle
{
  uint32_t                     CardType;         /*!< SD card type                                   */
  uint32_t                     RCA;              /*!< SD relative card address                       */
  uint32_t                     CSD[4];           /*!< SD card specific data table                    */
  uint32_t                     CID[4];           /*!< SD card identification number table            */
  uint64_t                     CardCapacity;     /*!< Card capacity                                  */  
  uint32_t                     CardBlockSize;    /*!< Card block size                                */
  uint32_t                     sectorCount;      /*!< Card sector                                     */
}SD_HandleTypeDef;

/*******************************************************************************
 * LOCAL FUNCTIONS
 */

extern FF_Disk_t* FF_SPISDMount(const char* pathName);
extern int8_t FF_IsSPISDMounted(void);
extern int32_t FF_SPISDDiskShowPartition(FF_Disk_t *pxDisk);
extern uint32_t FF_SPISDGetFreeDiskSize( void );
uint32_t FF_SPISDGetDiskSpaceInfo(uint32_t *pTotalKbs, uint32_t *pTotalUsedKbs);

#endif  // MEDIA_DRV_SPI_SD_H
