#include "Fw_global_config.h"

#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "Fw_global_config.h"
#include "eoss3_dev.h"
#include "eoss3_hal_spi.h"
#include "spi_flash.h"

/* FreeRTOS+FAT includes. */
#include "ff_headers.h"
#include "ff_stdio.h"

#include "media_drv_spi.h"

#define DIVS_ONE_KB                    0xA  
#pragma data_alignment = 4
static FF_Disk_t spi_flash_disk_info={0};
SPI_DISK_PRIVATE_DATA spi_disk_priv={0};

#ifdef ENABLE_RAMDISK
char ramdisk[RAMDISK_SIZE];
#endif

BaseType_t FF_SPIDiskDelete( FF_Disk_t *pxDisk );
void dump_spi_sectors(uint32_t start,uint32_t end);
static int32_t SPI_Write_Sector( uint8_t *pucSource,
                                uint32_t ulSectorNumber,
                                uint32_t ulSectorCount,
                                FF_Disk_t *pxDisk );
FF_Disk_t *FF_SpiFlashDiskPointer(void)
{
    return &spi_flash_disk_info;
}

void notify_spi_write_done(void)
{

}

#if 0
void spi_flash_test(void)
{
	int addr,size=4096;

	memset(testbuf,0,sizeof(testbuf));

	for(int i=0;i<100;i++)
	{
		addr=0x100000+(i*size);

		spiflash_erase(addr,size);

		memset(testbuf,i,size);

		if(spiflash_write(addr, size, testbuf, notify_spi_write_done) != FlashOperationSuccess)
		{
			printf("\n Write fail %x ",addr);
		}

		memset(testbuf,0,size);

		if(spiflash_read_fast(addr, size, (void *)(testbuf), NULL) != FlashOperationSuccess)
		{
			printf("Error reading Flash memory into buf\r\n");
		}

		for(int j=0;j<size;j++)
		{
			if(testbuf[j]!=i)
			{
				printf("\n Failed at %d \n",j);
				while(1);
			}
		}

		printf("\n RW sector 0x%x pass\n",addr);
	}

	return;

}
#endif
//=============================================================================


#if 0
/* Partion SPI flash and formats it */

static FF_Error_t SPI_PartitionAndFormatDisk(void)
{
	FF_PartitionParameters_t xPartition;
	FF_Error_t xError;
	FF_Disk_t *pxDisk=FF_SpiFlashDiskPointer();

	/* Media cannot be used until it has been partitioned.  In this
case a single partition is to be created that fills all available space - so
	by clearing the xPartition structure to zero. */
	memset( &xPartition, 0x00, sizeof( xPartition ) );
	xPartition.ulSectorCount = pxDisk->ulNumberOfSectors;
	xPartition.ulHiddenSectors = SPI_HIDDEN_SECTOR_COUNT;
	xPartition.xPrimaryCount = SPI_PRIMARY_PARTITIONS;
	xPartition.eSizeType = eSizeIsQuota;

	/* Perform the partitioning. */
	xError = FF_Partition( pxDisk, &xPartition );

	/* Print out the result of the partition operation. */
	FF_PRINTF( "FF_Partition: FF_Format: %s\n", FF_GetErrMessage( xError ) );

	/* Was the disk partitioned successfully? */
	if( FF_isERR( xError ) == pdFALSE )
	{
		/* The disk was partitioned successfully.  Format the first partition. */
		xError = FF_Format( pxDisk, SPI_PARTITION_NUMBER, pdTRUE, pdTRUE );

		/* Print out the result of the format operation. */
		FF_PRINTF( "FF_SPIDiskInit: FF_Format: %s\n", FF_GetErrMessage( xError ) );
	}

	return xError;
}

int SPI_MountAndAdd()
{
	FF_Error_t ret;
	FF_Disk_t *pxDisk=FF_SpiFlashDiskPointer();

	ret=FF_Mount( pxDisk, SPI_PARTITION_NUMBER);
	if(ret!=FF_ERR_NONE)
	{
		printf("\nErr mounting FS of SPI flash\n");
		return ret;
	}

	BaseType_t ret2;

	ret2=FF_FS_Add( SPIFLASH_DISK_NAME, pxDisk);
	if(ret2!=1)
	{
		printf("\nErr adding SPI to VFS\n");
		return ret2;
	}
}
#endif

/*
* These modules add FAT Sector Cache ability to the FAT file system
* for 2 important reasons.
* 1. To remove the over head of updating the FAT info sector everytime
*    a Data sector is updated. So file write throughput is increased.
* 2. To the reduce the wear level on the same FAT file info sector 
 *    being updated for each Data Sector write
*
* Operation:
*    Write - 
* 1. Anytime a sector is being written, it is checked if it belongs to
*    FAT sector or Data Sector in the file system
* 2. If it belongs to Data Sector it is always written immediately
* 3. If it belongs to FAT sector, then it is checked whether it is cached
*    If it is cached - then the sector buffer is updated with new data
      and the number of times that specific sector buffer is updated.
*    If the number reaches a certain limit, then the write to the Physical
*    sector is enabled else the Physical sector is not updated.
*    Also, if previous write to the Flash exceeds user specified time-limit
*    then flush the cache to the physical sector
*    If cache sector is not updated frequently, the sector cache is invalidated.
* 4. If it is not cached, then the sector is cached in one of the sector
*    buffers if a free buffer is available. 
* 5. If a free buffer is not available, then it is passed for immediate
*    storage to the Flash chip
*
* . Read -
* 6. Every read operation, the Sector is checked if it belongs to the FAT
*    region.
*    If it is - then the cache is checked if it available.
*       If it is available in the cache, the data is copied and returned
*    For all other cases, the data is read from the Flash chip and returned
*
*   Close -
* 7. Every time a file is closed, all cache buffers are written to the
*    physical sectors on the chip always.
*
*/


#if (SPI_SECTOR_CACHE_BUFFER_SIZE_BYTES) // if zero or undefined choose a default value
// must have been defined in Fw_global_config.h
#else
#define SPI_SECTOR_CACHE_BUFFER_SIZE_BYTES (4*4096)
#define SPI_SECTOR_SIZE_BYTES              (SPIFLASH_DISK_SECTOR_SIZE)
#endif

#define SPI_CACHE_SECTOR_COUNT_MAX (      16 ) // ( (SPI_SECTOR_CACHE_BUFFER_SIZE_BYTES) / (SPI_SECTOR_SIZE_BYTES) )
#define SPI_CACHE_UPDATE_COUNT_MAX (     400 )
#define SPI_CACHE_UPDATE_TIME_MAX  (100*1000 )
#define SPI_CACHE_INVALIDATE_COUNT_MAX ( 20  )

typedef struct fat_sector_cache
{
  int sector_number ;  // Sector number being cached
  int sector_size ;    // sector size fixed for a given Flash drive
  int sector_update_count; // count the number of times cache buffer is updated
  uint32_t cache_last_flush_time; // Last time cache is flushed to physical sector
  uint8_t *psector_buffer; // buffer where the sector cache is stored
} fat_sector_cache_t;

typedef struct media_cache_info
{
 int cache_update_count_max; // = 100; // flush cache when update count exceeds this value
 int cache_flush_time_max_millisecs; // = 35*1000;  // flush cache when time held exceeds ths value
 int spi_flash_cache_enable; // = 0; // cache disabled by default
 int fat_first_sector;
 int fat_last_sector;
 int max_fat_cache_count;
 int max_cache_invalidate_count;
 fat_sector_cache_t spi_flash_cache_info[SPI_CACHE_SECTOR_COUNT_MAX];
} media_cache_info_t ;

media_cache_info_t spi_flash_media_info;

void set_fat_sectors ( void )
{
  FF_Disk_t* pxDisk = &spi_flash_disk_info;
  media_cache_info_t *pmedia = &spi_flash_media_info;
  int first_sector = pxDisk->pxIOManager->xPartition.ulFATBeginLBA;
  int last_sector = first_sector + pxDisk->pxIOManager->xPartition.ulSectorsPerFAT * pxDisk->pxIOManager->xPartition.ucNumFATS;
  pmedia->fat_first_sector = first_sector ;
  pmedia->fat_last_sector = last_sector;
    
}

int check_fat_sector( int sector_number )
{
    media_cache_info_t *pmedia = &spi_flash_media_info;
    FF_Disk_t* pxDisk = &spi_flash_disk_info;

    if (!pmedia->spi_flash_cache_enable)
      return 0;
    int first_sector = pxDisk->pxIOManager->xPartition.ulFATBeginLBA;
    int last_sector = first_sector + pxDisk->pxIOManager->xPartition.ulSectorsPerFAT * pxDisk->pxIOManager->xPartition.ucNumFATS;

    if (sector_number >= first_sector && sector_number < last_sector )
      return 1;
    else
      return 0;
}


int SPIFlashCacheInit(uint8_t *pcache_buffer, int buffer_size, int flash_type, int sector_size, 
                      int update_count_max, int flush_time_max)
{
    media_cache_info_t *pmedia = &spi_flash_media_info;
    fat_sector_cache_t *pcache = &pmedia->spi_flash_cache_info[0];

    if (update_count_max > 0) {
      pmedia->cache_update_count_max = update_count_max ;
    }
    if (pmedia->cache_update_count_max > SPI_CACHE_UPDATE_COUNT_MAX) {
      pmedia->cache_update_count_max = SPI_CACHE_UPDATE_COUNT_MAX;
    }
    if (flush_time_max > 0) {
      pmedia->cache_flush_time_max_millisecs = flush_time_max;
    }
    if (pmedia->cache_flush_time_max_millisecs > SPI_CACHE_UPDATE_TIME_MAX ) {
      pmedia->cache_flush_time_max_millisecs = SPI_CACHE_UPDATE_TIME_MAX;
    }
    
    // to be fixed later
    if (sector_size != SPI_SECTOR_SIZE_BYTES)
      sector_size = SPI_SECTOR_SIZE_BYTES;

    pmedia->max_fat_cache_count = buffer_size / sector_size ;
    if (pmedia->max_fat_cache_count > SPI_CACHE_SECTOR_COUNT_MAX)
    {
      pmedia->max_fat_cache_count = SPI_CACHE_SECTOR_COUNT_MAX;
    }
    for (int nsectors = 0; nsectors < pmedia->max_fat_cache_count; nsectors++)
    {
      pcache[nsectors].sector_number = 0;  // initialize to invalid sector number
      pcache[nsectors].sector_size = sector_size;
      pcache[nsectors].sector_update_count = 0;
      pcache[nsectors].cache_last_flush_time = 0;
      pcache[nsectors].psector_buffer = pcache_buffer;
      pcache_buffer += sector_size;
    }

    if (pdTRUE == spi_flash_disk_info.xStatus.bIsMounted)
    {
       set_fat_sectors();
    }
    pmedia->spi_flash_cache_enable = 1; // enable cache

    return pmedia->max_fat_cache_count;
}

/** @return 
 * 0 - do not update physical sector
 * 1 - must update physical sector
 */
int SPIFlashCacheUpdate(int sector_number, uint8_t *psector_data)
{
    media_cache_info_t *pmedia = &spi_flash_media_info;
    fat_sector_cache_t *pcache = &pmedia->spi_flash_cache_info[0];

   if (!pmedia->spi_flash_cache_enable)
     return 1;

    pmedia->max_cache_invalidate_count++;
    // check if sector_number belongs to the FAT table region
    // else return 
    if (!check_fat_sector(sector_number))
       return 1;

    // check if sector is cached
    int snumber = -1;
    int free_snumber = -1;
    for (int nsectors = 0; nsectors < pmedia->max_fat_cache_count; nsectors++)
    {
      if (sector_number == pcache[nsectors].sector_number)  // initialize to invalid sector number
      {
        snumber = nsectors;
        break;
      }
      if (0 == pcache[nsectors].sector_number)  // initialize to invalid sector number
      {
        if (free_snumber <= 0) {
          free_snumber = nsectors;
        }
      }
    }
    if (snumber == -1)
    {
      // didnt find a sector, assign next available cache info
      if (free_snumber > 0) {
          snumber = free_snumber;
          pcache[snumber].sector_number = sector_number;
          pcache[snumber].cache_last_flush_time = xTaskGetTickCount();
          pcache[snumber].sector_update_count = 0;
      } 
      else
      {
        return 1;
      }
    }
    // Update the cache sector
    memcpy(pcache[snumber].psector_buffer, psector_data, pcache[snumber].sector_size);
    pcache[snumber].sector_update_count++;
    if (pcache[snumber].sector_update_count >= pmedia->cache_update_count_max)
    {
      pcache[snumber].sector_update_count = 0;
      pcache[snumber].cache_last_flush_time = xTaskGetTickCount();
      return 1;
    }
    if (( xTaskGetTickCount() - pcache[snumber].cache_last_flush_time) >= pmedia->cache_flush_time_max_millisecs)
    {
      if (pcache[snumber].sector_update_count < 2) {
        // sector not being actively used
        // invalidate the sector
        pcache[snumber].sector_number = 0;  // initialize to invalid sector number
        pcache[snumber].sector_update_count = 0;
        pcache[snumber].cache_last_flush_time = 0;
        return 1;
      }
      pcache[snumber].sector_update_count = 0;
      pcache[snumber].cache_last_flush_time = xTaskGetTickCount();
      return 1;
    }
    return 0;
}


int SPIFlashCacheRead(int sector_number, uint8_t *psector_data)
{
    media_cache_info_t *pmedia = &spi_flash_media_info;
    fat_sector_cache_t *pcache = &pmedia->spi_flash_cache_info[0];

   if (!pmedia->spi_flash_cache_enable)
     return 1;
   // check if sector_number belongs to the FAT table region
   // else return 
   if (!check_fat_sector(sector_number))
      return 1;
    // check if sector is cached
    for (int nsectors = 0; nsectors < pmedia->max_fat_cache_count; nsectors++)
    {
      if (sector_number == pcache[nsectors].sector_number)
      {
        // sector found in cache
        // return data from the cached buffer
        memcpy(psector_data, pcache[nsectors].psector_buffer, pcache[nsectors].sector_size);
        return 0;
      }
    }
    // request from physical sector
    return 1;
}

// file close must call this function
void SPIFlashCacheFlush(void)
{
    media_cache_info_t *pmedia = &spi_flash_media_info;
    fat_sector_cache_t *pcache = &pmedia->spi_flash_cache_info[0];

    if (!pmedia->spi_flash_cache_enable)
     return ;

    // temporarily disable cache copying onto itself
    pmedia->spi_flash_cache_enable = 0;

    for (int nsectors = 0; nsectors < pmedia->max_fat_cache_count; nsectors++)
    {
      if (pcache[nsectors].sector_number > 0)  // write contents to physical sector
      {
        // invoke the write function
printf("sector:%d cache count: %d, ticks: %d\n", pcache[nsectors].sector_number, pcache[nsectors].sector_update_count, pcache[nsectors].cache_last_flush_time);
        pcache[nsectors].sector_update_count = pmedia->cache_update_count_max+1;
        SPI_Write_Sector( pcache[nsectors].psector_buffer,
                         pcache[nsectors].sector_number,
                         1,
                         NULL );
        pcache[nsectors].sector_number = 0;  // initialize to invalid sector number
        pcache[nsectors].sector_update_count = 0;
        pcache[nsectors].cache_last_flush_time = 0;
      }
    }
    // enable cache copying onto itself
    pmedia->spi_flash_cache_enable = 1;
    pmedia->max_cache_invalidate_count = 0;
    return;
}

// Invalidate least recently used cache entries to improve cache region availability
void SPIFlashCacheInvalidate(void)
{
    media_cache_info_t *pmedia = &spi_flash_media_info;
    fat_sector_cache_t *pcache = &pmedia->spi_flash_cache_info[0];

    if (!pmedia->spi_flash_cache_enable)
      return ;
    if (pmedia->max_cache_invalidate_count <  SPI_CACHE_INVALIDATE_COUNT_MAX)
      return;

    // temporarily disable cache copying onto itself
    pmedia->spi_flash_cache_enable = 0;
    uint32_t curr_ticks = xTaskGetTickCount();
    for (int nsectors = 0; nsectors < pmedia->max_fat_cache_count; nsectors++)
    {
        if (pcache[nsectors].sector_number <= 0)
          continue;

        // invalidate least recently used sector
        if ( ( pcache[nsectors].sector_update_count < 2 ) &&
             (curr_ticks - pcache[nsectors].cache_last_flush_time) >= pmedia->cache_flush_time_max_millisecs )
        {
            // sector not being actively used
            // invalidate the sector
            // invoke the write function
            pcache[nsectors].sector_update_count = 0;
            SPI_Write_Sector( pcache[nsectors].psector_buffer,
                              pcache[nsectors].sector_number,
                              1,
                            NULL );
            pcache[nsectors].sector_number = 0;  // initialize to invalid sector number
            pcache[nsectors].sector_update_count = 0;
            pcache[nsectors].cache_last_flush_time = 0;
        }
    }
    pmedia->max_cache_invalidate_count = 0;
    // reenable cache 
    pmedia->spi_flash_cache_enable = 1;

    return;
}

static void media_cache_init(void)
{
    media_cache_info_t *pmedia = &spi_flash_media_info;

    pmedia->spi_flash_cache_enable = 0; // cache disabled by default
    pmedia->cache_update_count_max = 100; // 100 sectors (4kB for SPI Flash)
    pmedia->cache_flush_time_max_millisecs = 35*1000; // 35 secs
    pmedia->fat_first_sector = 0 ;
    pmedia->fat_last_sector = 0;
    pmedia->max_fat_cache_count = 0;
    pmedia->max_cache_invalidate_count = 0;
    return;
}
    //Media init, spi for for flash

int SPI_MediaInit(void)
{
    media_cache_init();
    flash_boot_setup();

#ifdef ENABLE_RAMDISK
	memset(ramdisk,0,sizeof(ramdisk));
#endif

	return 1;
}

//Media Read API

static int32_t SPI_Read_Sector( uint8_t *pucDestination,
							   uint32_t ulSectorNumber,
							   uint32_t ulSectorCount,
							   FF_Disk_t *pxDisk )
{

	int ret=0;

	//printf("\nRead %d sectors from %d",ulSectorCount,ulSectorNumber);

	for(int i=0;i<ulSectorCount;i++)
	{
      ret = SPIFlashCacheRead( (ulSectorNumber+i), (void *)(pucDestination+(i*SPIFLASH_DISK_SECTOR_SIZE)));
      if (ret==0)
        continue;
#ifndef ENABLE_RAMDISK

		//printf("\nReading SPI addr 0x%x\n",SPI_FLASH_FS_BASE_ADDR+((ulSectorNumber+i)*SPIFLASH_DISK_SECTOR_SIZE));
#if 0   // this workaround not required. [2019-08-28]. see changes in spi_flash.c in same commit.		
        //Fix [2019-06-14]: add taskdelay for Read file CRC mismatch and hanging in SPI driver
        if(i > 0)  vTaskDelay(1);
#endif
		ret=spiflash_read_fast(SPI_FLASH_FS_BASE_ADDR+((ulSectorNumber+i)*SPIFLASH_DISK_SECTOR_SIZE), SPIFLASH_DISK_SECTOR_SIZE,\
			(void *)(pucDestination+(i*SPIFLASH_DISK_SECTOR_SIZE)), NULL);

		if(ret!= FlashOperationSuccess)
		{
			printf("Error reading sector %d \r\n",i);
			return FF_ERR_DEVICE_DRIVER_FAILED;
		}
#else
		memcpy(pucDestination+(i*SPIFLASH_DISK_SECTOR_SIZE),((char*)ramdisk+((ulSectorNumber+i)*SPIFLASH_DISK_SECTOR_SIZE)),SPIFLASH_DISK_SECTOR_SIZE);
#endif
	}

	return FF_ERR_NONE;
}


//Media Write -> Erase sector and write

#ifdef SPI_512B_SECTOR
char temp_sector[SPI_FLASH_ERASE_SIZE]={0};
#endif

static int32_t SPI_Write_Sector( uint8_t *pucSource,
								uint32_t ulSectorNumber,
								uint32_t ulSectorCount,
								FF_Disk_t *pxDisk )
{
	int ret;

	//printf("\nWrite %d sectors from %d\n",ulSectorCount,ulSectorNumber);



	for(int i=0;i<ulSectorCount;i++)
	{
      ret = SPIFlashCacheUpdate( (ulSectorNumber+i), (void *)(pucSource+(i*SPIFLASH_DISK_SECTOR_SIZE)));
      if (ret==0)
        continue;

#ifdef ENABLE_RAMDISK
		memcpy(((char*)ramdisk+((ulSectorNumber+i)*SPIFLASH_DISK_SECTOR_SIZE)),pucSource+(i*SPIFLASH_DISK_SECTOR_SIZE),SPIFLASH_DISK_SECTOR_SIZE);
#else

	   ret=spiflash_erase(SPI_FLASH_FS_BASE_ADDR+((ulSectorNumber+i)*SPIFLASH_DISK_SECTOR_SIZE),SPIFLASH_DISK_SECTOR_SIZE);

	   if(ret!= FlashOperationSuccess)
	   {
		   printf("Error erasing sector %d \r\n",i);
		   return FF_ERR_DEVICE_DRIVER_FAILED;
	   }

   	   //printf("\nWriting SPI addr 0x%x\n",SPI_FLASH_FS_BASE_ADDR+((ulSectorNumber+i)*SPIFLASH_DISK_SECTOR_SIZE));

	   ret=spiflash_write(SPI_FLASH_FS_BASE_ADDR+((ulSectorNumber+i)*SPIFLASH_DISK_SECTOR_SIZE), SPIFLASH_DISK_SECTOR_SIZE,\
		   (void *)(pucSource+(i*SPIFLASH_DISK_SECTOR_SIZE)), notify_spi_write_done);

	   if(ret!= FlashOperationSuccess)
	   {
		   printf("Error reading sector %d \r\n",i);
		   return FF_ERR_DEVICE_DRIVER_FAILED;
	   }

#endif

	}

	return FF_ERR_NONE;
}



static FF_Error_t SPIPartitionAndFormatDisk( FF_Disk_t *pxDisk )
{
	FF_PartitionParameters_t xPartition;
	FF_Error_t xError;

	/* Create a single partition that fills all available space on the disk. */
	memset( &xPartition, '\0', sizeof( xPartition ) );
	xPartition.ulSectorCount = pxDisk->ulNumberOfSectors;
	xPartition.ulHiddenSectors = SPI_HIDDEN_SECTOR_COUNT;

	xPartition.ulInterSpace=0;
	xPartition.xPrimaryCount = SPI_PRIMARY_PARTITIONS;

	xPartition.eSizeType = eSizeIsPercent;
	xPartition.xSizes[0]=100;

	/* Partition the disk */

	FF_PRINTF("\nSPI Flash Partitioning...\n");

	xError = FF_Partition( pxDisk, &xPartition );
	FF_PRINTF( "FF_Partition: %s\n", ( const char * ) FF_GetErrMessage( xError ) );

	if( FF_isERR( xError ) == pdFALSE )
	{
	/* Format the partition. */
	FF_PRINTF("\nSPI Flash Formatting...\n");

	xError = FF_Format( pxDisk, SPI_PARTITION_NUMBER, pdTRUE, pdTRUE );
	FF_PRINTF( "FF_SPIDiskInit: FF_Format: %s\n", ( const char * ) FF_GetErrMessage( xError ) );
	}

	return xError;

}

#if 1
char dump_buf[4096]={0};

void dump_spi_sectors(uint32_t start,uint32_t end)
{

	for( ;start<=end;start++)
	{

	if(spiflash_read_fast(SPI_FLASH_FS_BASE_ADDR+(start*SPIFLASH_DISK_SECTOR_SIZE), SPIFLASH_DISK_SECTOR_SIZE, (void *)(dump_buf), NULL) != FlashOperationSuccess)
	{
	printf("Error reading Flash memory into buf\r\n");
	}


	printf("\nSector %d\n",start);
	for(int j=0;j<SPIFLASH_DISK_SECTOR_SIZE;j++)
	{
	if(j%16==0)
	{
	printf("\n0x%04x",j);
	}

	printf(" 0x%02x",dump_buf[j]);

	}
	}
	printf("\n");

}
#endif


int8_t FF_SPIIOMAN_Init(FF_Disk_t* pxDisk,size_t xIOManagerCacheSize,void* priv_ptr,uint32_t ulSectorCount )
{
	FF_CreationParameters_t xParameters;
	FF_Error_t xError;

	/* Check the validity of the xIOManagerCacheSize parameter. */
	configASSERT( ( xIOManagerCacheSize % SPIFLASH_DISK_SECTOR_SIZE ) == 0 );
	configASSERT( ( xIOManagerCacheSize >= ( 2 * SPIFLASH_DISK_SECTOR_SIZE ) ) );


	/* Attempt to allocated the FF_Disk_t structure. */
	pxDisk = FF_SpiFlashDiskPointer();

		memset( pxDisk, 0, sizeof( FF_Disk_t ) );

	/* The pvTag member of the FF_Disk_t structure allows the structure to be
	extended to also include media specific parameters. */
	pxDisk->pvTag = ( void * ) priv_ptr;

	/* The signature is used by the disk read and disk write functions to
	ensure the disk being accessed is a SPI disk. */
	pxDisk->ulSignature = SPIDISK_SIGNATURE;

	/* The number of sectors is recorded for bounds checking in the read and
	write functions. */
	pxDisk->ulNumberOfSectors = ulSectorCount;

	/* Create the IO manager that will be used to control the SPI disk. */
	memset( &xParameters, '\0', sizeof( xParameters ) );
	xParameters.pucCacheMemory = NULL;
	xParameters.ulMemorySize = xIOManagerCacheSize;
	xParameters.ulSectorSize = SPIFLASH_DISK_SECTOR_SIZE;
	xParameters.fnWriteBlocks = SPI_Write_Sector;
	xParameters.fnReadBlocks = SPI_Read_Sector;
	xParameters.pxDisk = pxDisk;

	/* Driver is reentrant so xBlockDeviceIsReentrant can be set to pdTRUE.
	In this case the semaphore is only used to protect FAT data
	structures. */
	xParameters.pvSemaphore = ( void * ) xSemaphoreCreateRecursiveMutex();
	vQueueAddToRegistry( xParameters.pvSemaphore, "SpiFlash_Sem" );
	xParameters.xBlockDeviceIsReentrant = pdFALSE;

	pxDisk->pxIOManager = FF_CreateIOManger( &xParameters, &xError );

	if( ( pxDisk->pxIOManager != NULL ) && ( FF_isERR( xError ) == pdFALSE ) )
	{
	/* Record that the SPI disk has been initialised. */
	pxDisk->xStatus.bIsInitialised = pdTRUE;

	return 1;

	}

	FF_PRINTF( "FF_SPIDiskInit: FF_CreateIOManger: %s\n", ( const char * ) FF_GetErrMessage( xError ) );

	/* The disk structure was allocated, but the disk's IO manager could
	not be allocated, so free the disk again. */
	FF_SPIDiskDelete( pxDisk );
	pxDisk = NULL;

	return 0;
}


FF_Error_t FF_SPIMountandAdd(FF_Disk_t* pxDisk,const char*pcName)
{
	FF_Error_t xError;

	/* Record the partition number the FF_Disk_t structure is, then
	mount the partition. */
	pxDisk->xStatus.bPartitionNumber = SPI_PARTITION_NUMBER;

	/* Mount the partition. */

	FF_PRINTF("\nSPI Flash Mounting ...\n");

	xError = FF_Mount( pxDisk, SPI_PARTITION_NUMBER );
	FF_PRINTF( "%s: FF_Mount: %s\n",__func__,( const char * ) FF_GetErrMessage( xError ) );

	if( FF_isERR( xError ) == pdFALSE )
	{
	/* The partition mounted successfully, add it to the virtual
	file system - where it will appear as a directory off the file
	system's root directory. */

	FF_FS_Add( pcName, pxDisk );

	FF_PRINTF("\nSPI Disk added at path %s \n",pcName);

	spi_flash_disk_info.xStatus.bIsMounted=pdTRUE;

	}
	else
	{
		FF_PRINTF( "%s: Adding SPI Disk to FS failed,err %s\n",__func__,( const char * ) FF_GetErrMessage( xError ) );
	}

	return xError;

}

int FF_SPICreatePartition(void)
{
	FF_Error_t xError;
	int ret=0;

    FF_PRINTF("\n Formatting flash \n");

	/* Clear the disk */
#ifdef ENABLE_RAMDISK
	memset(ramdisk,0,sizeof(ramdisk));
#else
	spiflash_erase(SPI_FLASH_FS_BASE_ADDR,16*SPIFLASH_DISK_SECTOR_SIZE);	//Erase initial sectors to clear FAT table

	//Entire chip erase
	//spiflash_erase(SPI_FLASH_FS_BASE_ADDR,SPIFLASH_DISK_SECTORS*SPIFLASH_DISK_SECTOR_SIZE);

#endif

	FF_SPIDiskDelete(FF_SpiFlashDiskPointer());	//To handle multiple format sequentially

	//ioman init

	ret=FF_SPIIOMAN_Init(FF_SpiFlashDiskPointer(),mainIO_MANAGER_CACHE_SIZE,&spi_disk_priv,SPIFLASH_DISK_SECTORS);

	if(ret!=1)
	{
		FF_PRINTF("\n SPI IOMAN init failed \n");
		return 0;
	}

#if 0
	pxDisk = FF_SPIDiskInit( SPIFLASH_DISK_NAME, (uint8_t*)&spi_disk_priv, SPIFLASH_DISK_SECTORS, mainIO_MANAGER_CACHE_SIZE );
	configASSERT( pxDisk );

	dump_spi_sectors(0,3);

#else
	xError = SPIPartitionAndFormatDisk( FF_SpiFlashDiskPointer() );
	if( FF_isERR( xError ) == pdFALSE )
	{
		//BL_PRINTF("\nSPI Flash partitioned and formatted\n");
		return 1;
	}
#endif


	return 0;

}

#if 0
FF_Disk_t *FF_SPIDiskInit( char *pcName, uint8_t *pucDataBuffer, uint32_t ulSectorCount, size_t xIOManagerCacheSize )
{
	FF_Error_t xError;
	FF_Disk_t *pxDisk = NULL;
	FF_CreationParameters_t xParameters;

	/* Check the validity of the xIOManagerCacheSize parameter. */
	configASSERT( ( xIOManagerCacheSize % SPIFLASH_DISK_SECTOR_SIZE ) == 0 );
	configASSERT( ( xIOManagerCacheSize >= ( 2 * SPIFLASH_DISK_SECTOR_SIZE ) ) );

	/* Attempt to allocated the FF_Disk_t structure. */
	pxDisk = (FF_Disk_t*)(FF_SpiFlashDiskPointer());

	/* Clear the disk */
	#ifdef ENABLE_RAMDISK
		memset(ramdisk,0,sizeof(ramdisk));
	#else

		#ifdef	FORMAT_DISK_SPI

		spiflash_erase(SPI_FLASH_FS_BASE_ADDR,16*SPIFLASH_DISK_SECTOR_SIZE);	//Erase first 16 sectors

		//Entire chip erase
		//spiflash_erase(SPI_FLASH_FS_BASE_ADDR,SPIFLASH_DISK_SECTORS*SPIFLASH_DISK_SECTOR_SIZE);
	#endif
	#endif

	if( pxDisk != NULL )
	{
	/* Start with every member of the structure set to zero. */

	memset( pxDisk, 0, sizeof( FF_Disk_t ) );

	/* The pvTag member of the FF_Disk_t structure allows the structure to be
	extended to also include media specific parameters. */
	pxDisk->pvTag = ( void * ) pucDataBuffer;

	/* The signature is used by the disk read and disk write functions to
	ensure the disk being accessed is a SPI disk. */
	pxDisk->ulSignature = SPIDISK_SIGNATURE;

	/* The number of sectors is recorded for bounds checking in the read and
	write functions. */
	pxDisk->ulNumberOfSectors = ulSectorCount;

	/* Create the IO manager that will be used to control the SPI disk. */
	memset( &xParameters, '\0', sizeof( xParameters ) );
	xParameters.pucCacheMemory = NULL;
	xParameters.ulMemorySize = xIOManagerCacheSize;
	xParameters.ulSectorSize = SPIFLASH_DISK_SECTOR_SIZE;
	xParameters.fnWriteBlocks = SPI_Write_Sector;
	xParameters.fnReadBlocks = SPI_Read_Sector;
	xParameters.pxDisk = pxDisk;

	/* Driver is reentrant so xBlockDeviceIsReentrant can be set to pdTRUE.
	In this case the semaphore is only used to protect FAT data
	structures. */
	xParameters.pvSemaphore = ( void * ) xSemaphoreCreateRecursiveMutex();
	xParameters.xBlockDeviceIsReentrant = pdFALSE;

	pxDisk->pxIOManager = FF_CreateIOManger( &xParameters, &xError );

	if( ( pxDisk->pxIOManager != NULL ) && ( FF_isERR( xError ) == pdFALSE ) )
	{
	/* Record that the SPI disk has been initialised. */
	pxDisk->xStatus.bIsInitialised = pdTRUE;

	#ifdef FORMAT_DISK_SPI

	/* Create a partition on the device.  NOTE!  The disk is only
	being partitioned here because it is a new disk.  It is
	known that the disk has not been used before, and cannot already
	contain any partitions.  Most media drivers will not perform
	this step because the media will have already been partitioned. */
	xError = SPIPartitionAndFormatDisk( pxDisk );

	if( FF_isERR( xError ) == pdFALSE )
	#endif

	{
	/* Record the partition number the FF_Disk_t structure is, then
	mount the partition. */
	pxDisk->xStatus.bPartitionNumber = SPI_PARTITION_NUMBER;

	/* Mount the partition. */

	FF_PRINTF("\nSPI Flash Mounting ...\n");

	xError = FF_Mount( pxDisk, SPI_PARTITION_NUMBER );
	FF_PRINTF( "FF_SPIDiskInit: FF_Mount: %s\n", ( const char * ) FF_GetErrMessage( xError ) );
	}

	if( FF_isERR( xError ) == pdFALSE )
	{
	/* The partition mounted successfully, add it to the virtual
	file system - where it will appear as a directory off the file
	system's root directory. */

	FF_FS_Add( pcName, pxDisk );

	FF_PRINTF("\nSPI Disk added at path %s \n",pcName);

	}
	}
	else
	{
	FF_PRINTF( "FF_SPIDiskInit: FF_CreateIOManger: %s\n", ( const char * ) FF_GetErrMessage( xError ) );

	/* The disk structure was allocated, but the disk's IO manager could
	not be allocated, so free the disk again. */
	FF_SPIDiskDelete( pxDisk );
	pxDisk = NULL;
	}
	}
	else
	{
	FF_PRINTF( "FF_SPIDiskInit: Malloc failed\n" );
	}

	return pxDisk;
}
/*-----------------------------------------------------------*/

#endif

BaseType_t FF_SPIDiskDelete( FF_Disk_t *pxDisk )
{
	if( pxDisk != NULL )
	{
		pxDisk->ulSignature = 0;
		pxDisk->xStatus.bIsInitialised = 0;
		if( pxDisk->pxIOManager != NULL )
		{
			FF_DeleteIOManager( pxDisk->pxIOManager );
		}
	}

return pdPASS;
}

/*-----------------------------------------------------------*/

BaseType_t FF_SPIDiskShowPartition( FF_Disk_t *pxDisk )
{
	FF_Error_t xError;
	uint64_t ullFreeSectors;
	uint32_t ulTotalSizeB, ulFreeSizeB;
	int iPercentageFree;
	FF_IOManager_t *pxIOManager;
	const char *pcTypeName = "unknown type";
	BaseType_t xReturn = pdPASS;

	if( pxDisk == NULL )
	{
	xReturn = pdFAIL;
	}
	else
	{
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

	uint32_t freesize = FF_GetFreeSize( pxIOManager, &xError );
    FF_PRINTF( "FreeSize       %8lu Bytes\n", freesize );

	ullFreeSectors = pxIOManager->xPartition.ulFreeClusterCount * pxIOManager->xPartition.ulSectorsPerCluster;
	iPercentageFree = ( int ) ( ( HUNDRED_64_BIT * ullFreeSectors + pxIOManager->xPartition.ulDataSectors / 2 ) /
	( ( uint64_t )pxIOManager->xPartition.ulDataSectors ) );

	ulTotalSizeB = pxIOManager->xPartition.ulDataSectors * SPIFLASH_DISK_SECTOR_SIZE;
	ulFreeSizeB = ( uint32_t ) ( ullFreeSectors * SPIFLASH_DISK_SECTOR_SIZE );

	((SPI_DISK_PRIVATE_DATA*)pxDisk->pvTag)->total_size=ulTotalSizeB;
	((SPI_DISK_PRIVATE_DATA*)pxDisk->pvTag)->free_size=ulFreeSizeB;

	/* It is better not to use the 64-bit format such as %Lu because it
	might not be implemented. */
	FF_PRINTF( "Partition Nr   %8u\n", pxDisk->xStatus.bPartitionNumber );
	FF_PRINTF( "Type           %8u (%s)\n", pxIOManager->xPartition.ucType, pcTypeName );
	FF_PRINTF( "TotalSectors   %8lu\n", pxIOManager->xPartition.ulTotalSectors );
	//FF_PRINTF( "SecsPerCluster %8lu\n", pxIOManager->xPartition.ulSectorsPerCluster );
	FF_PRINTF( "Total Size     %8lu Bytes\n", ulTotalSizeB  );
	FF_PRINTF( "FreeSize       %8lu Bytes ( %d perc free )\n", ulFreeSizeB, iPercentageFree );


}

return xReturn;
}

int8_t FF_IsSPIMounted(void)
{
	return spi_flash_disk_info.xStatus.bIsMounted;
}


int FF_SPIFlashMount(void)
{
	FF_Error_t xError;
	int ret=0;

	//ioman init

	if(spi_flash_disk_info.xStatus.bIsInitialised!=pdTRUE)
	{

		ret=FF_SPIIOMAN_Init(FF_SpiFlashDiskPointer(),mainIO_MANAGER_CACHE_SIZE,&spi_disk_priv,SPIFLASH_DISK_SECTORS);

		if(ret!=1)
		{
			FF_PRINTF("\n SPI IOMAN init failed \n");
			return 0;
		}

	}

	ret=1;

	//mount and add

	xError=FF_SPIMountandAdd(FF_SpiFlashDiskPointer(),SPIFLASH_DISK_NAME);

	if( FF_isERR( xError ) != pdFALSE )
	{
		FF_PRINTF( "%s: SPI flash disk mounted failed : %s\n",__func__,( const char * ) FF_GetErrMessage( xError ) );

		return 0;
	}

	FF_PRINTF("\nSPI Flash mount passed\n");

	/* Print out information on the disk. */
	FF_SPIDiskShowPartition( FF_SpiFlashDiskPointer() );

	return 1;

}

int FF_SPIFlashUnmount(void)
{
    /* Unmount the partition. */
	FF_PRINTF("\nSPI Flash Unmounting ...\n");

    FF_Error_t error = FF_Unmount(FF_SpiFlashDiskPointer());
    if(error == FF_ERR_NONE)
    {
        FF_PRINTF("\nSPI Flash Unmount Successful \n");
    }
	else
    {
        FF_PRINTF("\nSPI Flash Unmount Unsuccessful \n");
    }
    
    /* The disk structure was allocated, after unmounting the partition, so free the disk again. */
	FF_SPIDiskDelete( FF_SpiFlashDiskPointer() );
    return error;
}

uint32_t FF_SPIFlashGetFreeDiskSize(void)
{
    FF_Error_t xError;
    FF_Disk_t *pxDisk = FF_SpiFlashDiskPointer();
    configASSERT(pxDisk != NULL);
    return FF_GetFreeSize( pxDisk->pxIOManager, &xError );
}

// Get flash disk usuage numbers numbers in KB
uint32_t FF_SPIFDUN(uint32_t *pTotalKbs, uint32_t *pTotalUsedKbs)
{
	uint64_t ullFreeSectors;
	uint32_t ulTotalSizeB, ulFreeSizeB;
    FF_Disk_t *pxDisk = FF_SpiFlashDiskPointer();
	FF_IOManager_t *pxIOManager;

	if( pxDisk == NULL )
	{
        *pTotalKbs = 0;
        *pTotalUsedKbs = 0;
        
        return 1;
	}
	else
	{
        pxIOManager = pxDisk->pxIOManager;

        ullFreeSectors = pxIOManager->xPartition.ulFreeClusterCount * pxIOManager->xPartition.ulSectorsPerCluster;

        ulTotalSizeB = pxIOManager->xPartition.ulDataSectors * SPIFLASH_DISK_SECTOR_SIZE;
        ulFreeSizeB = ( uint32_t ) ( ullFreeSectors * SPIFLASH_DISK_SECTOR_SIZE );

        *pTotalKbs = ulTotalSizeB >> DIVS_ONE_KB;
        *pTotalUsedKbs = (ulTotalSizeB - ulFreeSizeB)>>DIVS_ONE_KB; 
        
        return 0;
    }
}
