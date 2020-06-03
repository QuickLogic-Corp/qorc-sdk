#ifndef MEDIA_SPI_H
#define MEDIA_SPI_H

#if !defined( _EnD_Of_Fw_global_config_h )
#error "Include Fw_global_config.h first"
#endif

#include "FreeRTOS.h"
#include "task.h"
//#define ENABLE_RAMDISK

//#define SPI_512B_SECTOR


#ifdef ENABLE_RAMDISK

	#define RAMDISK_SIZE		(128*1024)

	extern char ramdisk[RAMDISK_SIZE];

	#define SPI_FLASH_FS_BASE_ADDR		(0)
	#define SPIFLASH_DISK_SECTOR_SIZE	(512)
	#define SPIFLASH_DISK_SECTORS		((( sizeof(ramdisk)) / SPIFLASH_DISK_SECTOR_SIZE ))

#else

	#ifdef SPI_512B_SECTOR

		#define SPIFLASH_DISK_SECTOR_SIZE	(512)
	#else
		#define SPIFLASH_DISK_SECTOR_SIZE	(4096)
	#endif

	#define SPI_FLASH_FS_BASE_ADDR		((1*1024*1024))			//FS starts from 1 MB
	#define SPIFLASH_DISK_SECTORS		(( 3 * 1024 *1024 ) / SPIFLASH_DISK_SECTOR_SIZE )

#endif

#define SPI_FLASH_ERASE_SIZE		(4096)

typedef struct
{
	uint32_t total_size;
	uint32_t free_size;
}SPI_DISK_PRIVATE_DATA;

//extern FF_Disk_t spi_flash_disk_info;

/* Where the SPIFLASH disk is mounted. */
#define SPIFLASH_DISK_NAME			"/SPIFLASH"

#define mainIO_MANAGER_CACHE_SIZE	( 2 * SPIFLASH_DISK_SECTOR_SIZE )

#define HUNDRED_64_BIT				100ULL
#define BYTES_PER_MB				( 1024ull * 1024ull )
#define SECTORS_PER_MB				( BYTES_PER_MB / SPIFLASH_DISK_SECTOR_SIZE )

#define SPI_HIDDEN_SECTOR_COUNT     1
#define SPI_PRIMARY_PARTITIONS      1
#define SPI_PARTITION_NUMBER        0

static TaskHandle_t xFlashTaskHandle = NULL;
#define FLASH_TASK_STACK_SIZE	( configMINIMAL_STACK_SIZE )
#define TASK_FLASH_HANDLER      5


/* Used as a magic number to indicate that an FF_Disk_t structure is a RAM
disk. */
#define SPIDISK_SIGNATURE				(0x01020304)


extern uint32_t Get_SPI_FS_Free_Space(void);
extern int8_t FF_IsSPIMounted(void);
extern int FF_SPIFlashMount(void);
extern int FF_SPICreatePartition(void);
extern int FF_SPIFlashUnmount(void);
extern FF_Disk_t *FF_SpiFlashDiskPointer(void);


extern int SPIFlashCacheInit(uint8_t *pcache_buffer, int buffer_size, 
                             int flash_type, int sector_size, 
                             int update_count_max, int flush_time_max);
extern void SPIFlashCacheFlush(void);
extern void SPIFlashCacheInvalidate(void);

#endif
