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
 *    File   : media_drv_spi_sd.c
 *    Purpose: This file contains driver source code for Micro SD card.
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "eoss3_dev.h"
#include "eoss3_hal_spi.h"
#include "spi_flash.h"

#include "mmc_s3.h"
#include "media_drv_spi_sd.h"
#include "media_drv_defines.h"
#include "dbg_uart.h"
#pragma data_alignment = 4
static FF_Disk_t spi_sd_disk_info={0};
static SD_HandleTypeDef hsd = {0};
SPI_DISK_PRIVATE_DATA spisd_priv={0};

#ifdef ENABLE_RAMDISK
char ramdisk[RAMDISK_SIZE];
#endif

/*********************************************************************
 * FUNCTIONS
 */ 

BaseType_t FF_SPIDiskDelete(FF_Disk_t *pxDisk);

static int32_t spi_sdDiskRead(uint8_t  *buff, uint32_t sectorNo,
               uint32_t sectorCount, FF_Disk_t *pxDisk);

static int32_t spi_sdDiskWrite(uint8_t *pucSource, uint32_t ulSectorNumber,
		  uint32_t ulSectorCount, FF_Disk_t *pxDisk);

/*
 * @fn      FF_SPISDIOMAN_Init
 *
 * @brief   API intializes the disk driver IO manager
 * @param pxDisk - pointer to disk structure.
 * @param xIOManagerCacheSize - IO manager cache size.
 * @param priv_ptr - Pointer to disk's private data.
 * @param ulSectorCount - sector counts.
 * @return - pdTRUE on success otherwise pdFALSE.
 */
static int8_t FF_SPISDIOMAN_Init(FF_Disk_t* pxDisk, size_t xIOManagerCacheSize, void* priv_ptr, uint32_t ulSectorCount )
{
	FF_CreationParameters_t xParameters;
	FF_Error_t xError;

	/* Check the validity of the xIOManagerCacheSize parameter. */
	//configASSERT( ( xIOManagerCacheSize % SPIFLASH_DISK_SECTOR_SIZE ) == 0 );
	//configASSERT( ( xIOManagerCacheSize >= ( 2 * SPIFLASH_DISK_SECTOR_SIZE ) ) );


    dbg_str("SD-ioman-init\n");
	/* Attempt to allocated the FF_Disk_t structure. */
	pxDisk = (FF_Disk_t*)(&spi_sd_disk_info);

	memset( pxDisk, 0, sizeof( FF_Disk_t ) );

	/* The pvTag member of the FF_Disk_t structure allows the structure to be
	extended to also include media specific parameters. */
	pxDisk->pvTag = ( void * ) priv_ptr;

	/* The signature is used by the disk read and disk write functions to
	ensure the disk being accessed is a SPI disk. */
	pxDisk->ulSignature = SPISD_SIGNATURE;

	/* The number of sectors is recorded for bounds checking in the read and
	write functions. */
	pxDisk->ulNumberOfSectors = ulSectorCount;

	/* Create the IO manager that will be used to control the SPI disk. */
	memset( &xParameters, '\0', sizeof( xParameters ) );
	xParameters.pucCacheMemory = NULL;
	xParameters.ulMemorySize = xIOManagerCacheSize;
	xParameters.ulSectorSize = SPISD_DISK_SECTOR_SIZE;
	xParameters.fnWriteBlocks = spi_sdDiskWrite;
	xParameters.fnReadBlocks = spi_sdDiskRead;
	xParameters.pxDisk = pxDisk;

	/* Driver is reentrant so xBlockDeviceIsReentrant can be set to pdTRUE.
	In this case the semaphore is only used to protect FAT data
	structures. */
	xParameters.pvSemaphore = ( void * ) xSemaphoreCreateRecursiveMutex();
    vQueueAddToRegistry( xParameters.pvSemaphore, "SdCard_Sem" );
	xParameters.xBlockDeviceIsReentrant = pdFALSE;

	pxDisk->pxIOManager = FF_CreateIOManger( &xParameters, &xError );

	if( ( pxDisk->pxIOManager != NULL ) && ( FF_isERR( xError ) == pdFALSE ) )
	{
	    /* Record that the SPI disk has been initialised. */
        dbg_str("SD-OPEN\n");
	    pxDisk->xStatus.bIsInitialised = pdTRUE;
	    return pdTRUE;
	}
    dbg_str_str("SD-ioman-init: ERROR FAIL", ( const char * ) FF_GetErrMessage( xError ) ); 

	FF_PRINTF( "FF_SPIDiskInit: FF_CreateIOManger: %s\n", ( const char * ) FF_GetErrMessage( xError ) );

	/* The disk structure was allocated, but the disk's IO manager could
	not be allocated, so free the disk again. */
	FF_SPIDiskDelete( pxDisk );
	pxDisk = NULL;
	return pdFALSE;
}

/*
 * @fn      FF_SDDiskInit
 *
 * @brief   This API initializes the SD card physical media via SPI bus and 
 *   initializes the disk IO manager      
 * @param   pxDisk - pointer to disk structure.
 * @param   xIOManagerCacheSize - IO manager cache size.
 * @param   priv_ptr - Pointer to disk's private data.
 * @param   ulSectorCount - sector counts.
 * @return  pdTRUE on success otherwise pdFALSE.
 */
FF_Disk_t *FF_SDDiskInit(void)
{   
    // Check if disk already initialized    
    if(spi_sd_disk_info.xStatus.bIsInitialised == pdTRUE)
    {
        dbg_str("SD-DiskInit - Already Init\n");
        return &spi_sd_disk_info;
    }
    
    dbg_str("SD-DiskInit - Require init\n");
    if(SDdisk_initialize (0, &hsd) == STA_NOINIT)
    {
        dbg_str("no-card-present1\n");
        //FF_PRINTF("\nFF_SDDiskInit:: SPI IOMAN init failed \n");
	    return NULL;
    }

    if(FF_SPISDIOMAN_Init(&spi_sd_disk_info, SPI_SD_IO_MANAGER_CACHE_SIZE, &spisd_priv, hsd.sectorCount) == pdFALSE)
    {
        dbg_str("card-init-failed\n");
	    return NULL;
    }    
    return &spi_sd_disk_info;
}

/*
 * @fn      FF_SPISDMountandAdd
 *
 * @brief   This API mounts the disk volume to the given path name and adds to 
 *  virtual file structure
 * @param   pxDisk - pointer to disk structure.
 * @param   pathName - Volume name
 * @return  0 on success otherwise non zero.
 */
FF_Error_t FF_SPISDMountandAdd(FF_Disk_t* pxDisk, const char* pathName)
{
	FF_Error_t xError;

	/* Record the partition number the FF_Disk_t structure is, then
	mount the partition. */
	pxDisk->xStatus.bPartitionNumber = SPI_SD_PARTITION_NUMBER;

	/* Mount the partition. */
    dbg_str("SD MountAdd\n");
	FF_PRINTF("\nSPI SD Mounting ...\n");

	xError = FF_Mount( pxDisk, SPI_SD_PARTITION_NUMBER );
	FF_PRINTF( "%s: FF_Mount: %s\n",__func__,( const char * ) FF_GetErrMessage( xError ) );

	if( FF_isERR( xError ) == pdFALSE )
	{
	    /* The partition mounted successfully, add it to the virtual
	     file system - where it will appear as a directory off the file
	     system's root directory. */

	    FF_FS_Add( pathName, pxDisk );
	    FF_PRINTF("\nSPI Disk added at path %s \n", pathName);
        dbg_str_str("SD-Mount succss at", pathName );
	    spi_sd_disk_info.xStatus.bIsMounted = pdTRUE;
	}
	else
	{
        dbg_str_str("SD-Mount FAIL",FF_GetErrMessage( xError ));
		FF_PRINTF( "%s: Adding SPI Disk to FS failed,err %s\n",__func__,( const char * ) FF_GetErrMessage( xError ) );
	}

	return xError;
}

/*
 * @fn      FF_SPISDDiskDelete
 *
 * @brief   This API delete the disk volume.
 * @param   pxDisk - pointer to disk structure.
 * @return  0 on success otherwise non zero.
 */
FF_Error_t FF_SPISDDiskDelete( FF_Disk_t *pxDisk )
{
    FF_Error_t xError = FF_ERR_NONE; 
    
	if( pxDisk != NULL )
	{
        dbg_str("SD-Delete\n");
		pxDisk->ulSignature = 0;
		pxDisk->xStatus.bIsInitialised = 0;
		if( pxDisk->pxIOManager != NULL )
		{
			xError = FF_DeleteIOManager( pxDisk->pxIOManager );
		}
	} else {
        dbg_str("SDK Disk Delete NULL\n");
	}
    return xError;
}

/*
 * @fn      FF_SPISDDiskShowPartition
 *
 * @brief   This API dispalys the disk volume information.
 * @param   pxDisk - pointer to disk structure.
 * @return  0 on success otherwise non zero.
 */
INT32_t FF_SPISDDiskShowPartition( FF_Disk_t *pxDisk )
{
	uint64_t ullFreeSectors;
	uint32_t ulTotalSizeB, ulFreeSizeB;
	int iPercentageFree;
	FF_IOManager_t *pxIOManager;
	const char *pcTypeName = "unknown type";
	FF_Error_t xError = FF_ERR_NONE;

	if(pxDisk == NULL)
	{
	   return FF_ERR_NULL_POINTER;
	}
	
	pxIOManager = pxDisk->pxIOManager;

	FF_PRINTF( "Reading FAT and calculating Free Space\n" );

	switch( pxIOManager->xPartition.ucType )
	{
	case FF_T_FAT12:
	    pcTypeName = "FAT12";
	    break;

	case FF_T_FAT16:
	    pcTypeName = "FAT16";
	    break;

	case FF_T_FAT32:
	    pcTypeName = "FAT32";
	    break;

	default:
	    pcTypeName = "UNKOWN";
	    break;
	}

	FF_GetFreeSize( pxIOManager, &xError );
	ullFreeSectors = pxIOManager->xPartition.ulFreeClusterCount * pxIOManager->xPartition.ulSectorsPerCluster;
	iPercentageFree = ( int ) ( ( HUNDRED_64_BIT * ullFreeSectors + pxIOManager->xPartition.ulDataSectors / 2 ) /
	( ( uint64_t )pxIOManager->xPartition.ulDataSectors ) );

	ulTotalSizeB = pxIOManager->xPartition.ulDataSectors * hsd.CardBlockSize;
	ulFreeSizeB = ( uint32_t ) ( ullFreeSectors * hsd.CardBlockSize );

    FF_PRINTF( "Sector size    %8u\n", pxDisk->pxIOManager->usSectorSize );
	FF_PRINTF( "Partition Nr   %8u\n", pxDisk->pxIOManager->xPartition.ucPartitionMounted );
    FF_PRINTF( "Partition Mounted   %8u\n", pxDisk->xStatus.bPartitionNumber );
	FF_PRINTF( "Type           %8u (%s)\n", pxIOManager->xPartition.ucType, pcTypeName );
	FF_PRINTF( "TotalSectors   %8lu\n", pxIOManager->xPartition.ulTotalSectors );
	FF_PRINTF( "SecsPerCluster %8lu\n", pxIOManager->xPartition.ulSectorsPerCluster );
	FF_PRINTF( "Total Size     %8lu Bytes\n", ulTotalSizeB  );
	FF_PRINTF( "FreeSize       %8lu Bytes ( %d perc free )\n", ulFreeSizeB, iPercentageFree);
    return FF_ERR_NONE;
}

uint32_t FF_SPISDGetFreeDiskSize( void )
{
    FF_Error_t xError;
    FF_Disk_t *pxDisk =&spi_sd_disk_info;
    configASSERT(pxDisk != NULL);
    return FF_GetFreeSize( pxDisk->pxIOManager, &xError );
}

uint32_t FF_SPISDGetDiskSpaceInfo(uint32_t *pTotalKbs, uint32_t *pTotalUsedKbs)
{
    uint64_t ullFreeSectors;
	uint64_t ulTotalSizeB, ulFreeSizeB;
    FF_Disk_t *pxDisk = &spi_sd_disk_info;
	FF_IOManager_t *pxIOManager;
    FF_Error_t xError = FF_ERR_NONE;
    
	if( pxDisk == NULL )
	{
        *pTotalKbs = 0;
        *pTotalUsedKbs = 0;
        
        return 1;
	}
	else
	{
        pxIOManager = pxDisk->pxIOManager;
        
      	FF_GetFreeSize( pxIOManager, &xError );
        
        ullFreeSectors = (uint64_t)pxIOManager->xPartition.ulFreeClusterCount * (uint64_t)pxIOManager->xPartition.ulSectorsPerCluster;
        
        ulTotalSizeB = (uint64_t)(pxIOManager->xPartition.ulDataSectors) * (uint64_t)hsd.CardBlockSize;
        ulFreeSizeB =  ( ullFreeSectors * hsd.CardBlockSize );
        
        //get space in kBs
        *pTotalKbs = ulTotalSizeB >> 10;
        *pTotalUsedKbs = (ulTotalSizeB - ulFreeSizeB)>>10; 
        
        return 0;
    }
}

/*
 * @fn      FF_SPISDMount
 *
 * @brief   This API mounts the disk volume.
 * @param   pathName - disk volume name.
 * @return  pointer to disk structure on success els failure.
 */
FF_Disk_t* FF_SPISDMount(const char* pathName)
{
	FF_Error_t xError;
    
    FF_Disk_t *diskInfo = FF_SDDiskInit();        
    dbg_str_str("SPSDI Mount", pathName);
    if(diskInfo == NULL)
    {
        FF_PRINTF("\n FF_SPISDMount:: SPIFF_SDDiskInit failed \n");
        return NULL;
    }
    
	xError=FF_SPISDMountandAdd(&spi_sd_disk_info, pathName);
	if( FF_isERR( xError ) != pdFALSE )
	{
	    FF_PRINTF( "FF_SPISDMount::%s: SPI SD disk mounted failed : %s\n",__func__,( const char * ) FF_GetErrMessage( xError ) );
	    return NULL;
	}

	FF_PRINTF("\nFF_SPISDMount:: SPI SD mount passed\n");
	
#if DEBUG  /* Print out information on the disk. */
	FF_SPISDDiskShowPartition( &spi_sd_disk_info );
#endif
	return diskInfo;
}

#ifdef PARTITION_SD_DISK
/*
 * @fn      SPISDPartitionAndFormatDisk
 *
 * @brief   This API partitons and formats the disk.
 * @param   pxDisk - pointer to disk structure.
 * @return  FF_ERR_NONE(0) on success else to error failure.
 */
static FF_Error_t SPISDPartitionAndFormatDisk( FF_Disk_t *pxDisk )
{
	FF_PartitionParameters_t xPartition;
	FF_Error_t xError;

	/* Create a single partition that fills all available space on the disk. */
	memset( &xPartition, '\0', sizeof( xPartition ) );
	xPartition.ulSectorCount = pxDisk->ulNumberOfSectors;
	xPartition.ulHiddenSectors = SPI_HIDDEN_SECTOR_COUNT;

	xPartition.ulInterSpace=0;
	xPartition.xPrimaryCount = SPI_SD_PRIMARY_PARTITIONS;

	xPartition.eSizeType = eSizeIsPercent;
	xPartition.xSizes[0]=100;

	/* Partition the disk */

	FF_PRINTF("\nSPI SD Partitioning...\n");

	xError = FF_Partition( pxDisk, &xPartition );
	FF_PRINTF( "FF_Partition: %s\n", ( const char * ) FF_GetErrMessage( xError ) );

	if( FF_isERR( xError ) == pdFALSE )
	{
	    /* Format the partition. */
	    FF_PRINTF("\nSPI SD Formatting...\n");

	    xError = FF_Format( pxDisk, SPI_SD_PARTITION_NUMBER, pdTRUE, pdTRUE );
	    FF_PRINTF( "FF_SPIDiskInit: FF_Format: %s\n", ( const char * ) FF_GetErrMessage( xError ) );
	}

	return xError;
}
#endif

/*
 * @fn      spi_sdDiskRead
 *
 * @brief   This is adaptor api to use mmc_s3.c driver.
 * @param   buff - Pointer to the data buffer to store read data.
 * @param   sectorNo - Start sector number (LBA).
 * @param   sectorCount - Sector count (1..128) ).
 * @param   pointer to disk structure- not used.
 * @return  FF_ERR_NONE(0) on success.
 */
static int32_t spi_sdDiskRead (
	uint8_t  *buff,		/* Pointer to the data buffer to store read data */
	uint32_t sectorNo,	/* Start sector number (LBA) */
	uint32_t sectorCount,	/* Sector count (1..128) */
        FF_Disk_t *pxDisk)
{
	DRESULT status = (DRESULT)FF_ERR_NONE;
    
    while((sectorCount) && (status == FF_ERR_NONE))
    {
        status = disk_read(
                           0,		/* Physical drive nmuber (0) */
                           (UINT8_t *)buff,		/* Pointer to the data buffer to store read data */
                           (UINT32_t )sectorNo*SPISD_DISK_SECTOR_FACTOR,	/* Start sector number (LBA) */
                           SPISD_DISK_SECTOR_FACTOR);		/* Sector count (1..128) */
        buff += SPISD_DISK_SECTOR_SIZE;
        sectorNo++;
        sectorCount--;
    }   
    return status;
}

/*
 * @fn      spi_sdDiskWrite
 *
 * @brief   This is adaptor api to use mmc_s3.c driver.
 * @param   pucSource - Pointer to source data buffer.
 * @param   sectorNo - Start sector number (LBA).
 * @param   sectorCount - Sector count (1..128) ).
 * @param   pointer to disk structure- not used.
 * @return  FF_ERR_NONE(0) on success.
 */
int32_t spi_sdDiskWrite(uint8_t *pucSource,
		       uint32_t ulSectorNumber,
		       uint32_t ulSectorCount,
		       FF_Disk_t *pxDisk)
{
    disk_write (
	0,				/* Physical drive nmuber (0) */
	(const BYTE*)pucSource,		/* Pointer to the data to be written */
	ulSectorNumber*SPISD_DISK_SECTOR_FACTOR,			/* Start sector number (LBA) */
	ulSectorCount*SPISD_DISK_SECTOR_FACTOR);			/* Sector count (1..128) */

    return FF_ERR_NONE;
}




