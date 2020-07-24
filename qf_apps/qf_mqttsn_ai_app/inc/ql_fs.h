#if !defined( QL_FS_H )
#define QL_FS_H 1
/*----------------------------------------------------------------------------/
/  FatFs - Generic FAT Filesystem Module  R0.13b                              /
/-----------------------------------------------------------------------------/
/
/ Copyright (C) 2018, ChaN, all right reserved.
/
/ FatFs module is an open source software. Redistribution and use of FatFs in
/ source and binary forms, with or without modification, are permitted provided
/ that the following condition is met:
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/
/----------------------------------------------------------------------------*/

#if !defined( _EnD_Of_Fw_global_config_h )
#error "Include Fw_global_config.h first"
#endif

// Include files

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "media_drv_spi.h"
//#include "media_drv_spi_sd.h"
#include <time.h>

#if (USE_FREERTOS_FAT)
#include "ff_headers.h"
#endif
#if (USE_FATFS)
//use FatFS definitions
#include "ffconf.h"
#include "ff.h"

#define FF_ERR_NONE		FR_OK 	/* No Error */

//#define FS_DBG				//Enable FS prints
#ifdef FS_DBG
#define FF_PRINTF							printf
#endif

#if !defined(portINLINE)
#define portINLINE							inline
#endif

#if !defined( FF_PRINTF )
	#define	FF_PRINTF FF_PRINTF
	static portINLINE void FF_PRINTF( const char *pcFormat, ... )
	{
		( void ) pcFormat;
	}
#endif

#endif

#if 1 //not required for SensorTile

#include "media_drv_spi.h"
#include "media_drv_spi_sd.h"

/**
  * @brief SD Commands Index
  */
#define SD_CMD_GO_IDLE_STATE                       ((uint8_t)0)   /*!< Resets the SD memory card.                                                               */
#define SD_CMD_SEND_OP_COND                        ((uint8_t)1)   /*!< Sends host capacity support information and activates the card's initialization process. */
#define SD_CMD_ALL_SEND_CID                        ((uint8_t)2)   /*!< Asks any card connected to the host to send the CID numbers on the CMD line.             */
#define SD_CMD_SET_REL_ADDR                        ((uint8_t)3)   /*!< Asks the card to publish a new relative address (RCA).                                   */
#define SD_CMD_SET_DSR                             ((uint8_t)4)   /*!< Programs the DSR of all cards.                                                           */
#define SD_CMD_SDIO_SEN_OP_COND                    ((uint8_t)5)   /*!< Sends host capacity support information (HCS) and asks the accessed card to send its
                                                                       operating condition register (OCR) content in the response on the CMD line.              */
#define SD_CMD_HS_SWITCH                           ((uint8_t)6)   /*!< Checks switchable function (mode 0) and switch card function (mode 1).                   */
#define SD_CMD_SEL_DESEL_CARD                      ((uint8_t)7)   /*!< Selects the card by its own relative address and gets deselected by any other address    */
#define SD_CMD_HS_SEND_EXT_CSD                     ((uint8_t)8)   /*!< Sends SD Memory Card interface condition, which includes host supply voltage information
                                                                       and asks the card whether card supports voltage.                                         */
#define SD_CMD_SEND_CSD                            ((uint8_t)9)   /*!< Addressed card sends its card specific data (CSD) on the CMD line.                       */
#define SD_CMD_SEND_CID                            ((uint8_t)10)  /*!< Addressed card sends its card identification (CID) on the CMD line.                      */
#define SD_CMD_READ_DAT_UNTIL_STOP                 ((uint8_t)11)  /*!< SD card doesn't support it.                                                              */
#define SD_CMD_STOP_TRANSMISSION                   ((uint8_t)12)  /*!< Forces the card to stop transmission.                                                    */
#define SD_CMD_SEND_STATUS                         ((uint8_t)13)  /*!< Addressed card sends its status register.                                                */
#define SD_CMD_HS_BUSTEST_READ                     ((uint8_t)14)
#define SD_CMD_GO_INACTIVE_STATE                   ((uint8_t)15)  /*!< Sends an addressed card into the inactive state.                                         */
#define SD_CMD_SET_BLOCKLEN                        ((uint8_t)16)  /*!< Sets the block length (in bytes for SDSC) for all following block commands
                                                                       (read, write, lock). Default block length is fixed to 512 Bytes. Not effective
                                                                       for SDHS and SDXC.                                                                       */
#define SD_CMD_READ_SINGLE_BLOCK                   ((uint8_t)17)  /*!< Reads single block of size selected by SET_BLOCKLEN in case of SDSC, and a block of
                                                                       fixed 512 bytes in case of SDHC and SDXC.                                                */
#define SD_CMD_READ_MULT_BLOCK                     ((uint8_t)18)  /*!< Continuously transfers data blocks from card to host until interrupted by
                                                                       STOP_TRANSMISSION command.                                                               */
#define SD_CMD_HS_BUSTEST_WRITE                    ((uint8_t)19)  /*!< 64 bytes tuning pattern is sent for SDR50 and SDR104.                                    */
#define SD_CMD_WRITE_DAT_UNTIL_STOP                ((uint8_t)20)  /*!< Speed class control command.                                                             */
#define SD_CMD_SET_BLOCK_COUNT                     ((uint8_t)23)  /*!< Specify block count for CMD18 and CMD25.                                                 */
#define SD_CMD_WRITE_SINGLE_BLOCK                  ((uint8_t)24)  /*!< Writes single block of size selected by SET_BLOCKLEN in case of SDSC, and a block of
                                                                       fixed 512 bytes in case of SDHC and SDXC.                                                */
#define SD_CMD_WRITE_MULT_BLOCK                    ((uint8_t)25)  /*!< Continuously writes blocks of data until a STOP_TRANSMISSION follows.                    */
#define SD_CMD_PROG_CID                            ((uint8_t)26)  /*!< Reserved for manufacturers.                                                              */
#define SD_CMD_PROG_CSD                            ((uint8_t)27)  /*!< Programming of the programmable bits of the CSD.                                         */
#define SD_CMD_SET_WRITE_PROT                      ((uint8_t)28)  /*!< Sets the write protection bit of the addressed group.                                    */
#define SD_CMD_CLR_WRITE_PROT                      ((uint8_t)29)  /*!< Clears the write protection bit of the addressed group.                                  */
#define SD_CMD_SEND_WRITE_PROT                     ((uint8_t)30)  /*!< Asks the card to send the status of the write protection bits.                           */
#define SD_CMD_SD_ERASE_GRP_START                  ((uint8_t)32)  /*!< Sets the address of the first write block to be erased. (For SD card only).              */
#define SD_CMD_SD_ERASE_GRP_END                    ((uint8_t)33)  /*!< Sets the address of the last write block of the continuous range to be erased.           */
#define SD_CMD_ERASE_GRP_START                     ((uint8_t)35)  /*!< Sets the address of the first write block to be erased. Reserved for each command
                                                                       system set by switch function command (CMD6).                                            */
#define SD_CMD_ERASE_GRP_END                       ((uint8_t)36)  /*!< Sets the address of the last write block of the continuous range to be erased.
                                                                       Reserved for each command system set by switch function command (CMD6).                  */
#define SD_CMD_ERASE                               ((uint8_t)38)  /*!< Reserved for SD security applications.                                                   */
#define SD_CMD_FAST_IO                             ((uint8_t)39)  /*!< SD card doesn't support it (Reserved).                                                   */
#define SD_CMD_GO_IRQ_STATE                        ((uint8_t)40)  /*!< SD card doesn't support it (Reserved).                                                   */
#define SD_CMD_LOCK_UNLOCK                         ((uint8_t)42)  /*!< Sets/resets the password or lock/unlock the card. The size of the data block is set by
                                                                       the SET_BLOCK_LEN command.                                                               */
#define SD_CMD_APP_CMD                             ((uint8_t)55)  /*!< Indicates to the card that the next command is an application specific command rather
                                                                       than a standard command.                                                                 */
#define SD_CMD_GEN_CMD                             ((uint8_t)56)  /*!< Used either to transfer a data block to the card or to get a data block from the card
                                                                       for general purpose/application specific commands.                                       */
#define SD_CMD_NO_CMD                              ((uint8_t)64)

/**
  * @brief Following commands are SD Card Specific commands.
  *        SDIO_APP_CMD should be sent before sending these commands.
  */
#define SD_CMD_APP_SD_SET_BUSWIDTH                 ((uint8_t)6)   /*!< (ACMD6) Defines the data bus width to be used for data transfer. The allowed data bus
                                                                       widths are given in SCR register.                                                          */
#define SD_CMD_SD_APP_STATUS                       ((uint8_t)13)  /*!< (ACMD13) Sends the SD status.                                                              */
#define SD_CMD_SD_APP_SEND_NUM_WRITE_BLOCKS        ((uint8_t)22)  /*!< (ACMD22) Sends the number of the written (without errors) write blocks. Responds with
                                                                       32bit+CRC data block.                                                                      */
#define SD_CMD_SD_APP_OP_COND                      ((uint8_t)41)  /*!< (ACMD41) Sends host capacity support information (HCS) and asks the accessed card to
                                                                       send its operating condition register (OCR) content in the response on the CMD line.       */
#define SD_CMD_SD_APP_SET_CLR_CARD_DETECT          ((uint8_t)42)  /*!< (ACMD42) Connects/Disconnects the 50 KOhm pull-up resistor on CD/DAT3 (pin 1) of the card. */
#define SD_CMD_SD_APP_SEND_SCR                     ((uint8_t)51)  /*!< Reads the SD Configuration Register (SCR).                                                 */
#define SD_CMD_SDIO_RW_DIRECT                      ((uint8_t)52)  /*!< For SD I/O card only, reserved for security specification.                                 */
#define SD_CMD_SDIO_RW_EXTENDED                    ((uint8_t)53)  /*!< For SD I/O card only, reserved for security specification.                                 */

/**
  * @brief Following commands are SD Card Specific security commands.
  *        SD_CMD_APP_CMD should be sent before sending these commands.
  */
#define SD_CMD_SD_APP_GET_MKB                      ((uint8_t)43)  /*!< For SD card only */
#define SD_CMD_SD_APP_GET_MID                      ((uint8_t)44)  /*!< For SD card only */
#define SD_CMD_SD_APP_SET_CER_RN1                  ((uint8_t)45)  /*!< For SD card only */
#define SD_CMD_SD_APP_GET_CER_RN2                  ((uint8_t)46)  /*!< For SD card only */
#define SD_CMD_SD_APP_SET_CER_RES2                 ((uint8_t)47)  /*!< For SD card only */
#define SD_CMD_SD_APP_GET_CER_RES1                 ((uint8_t)48)  /*!< For SD card only */
#define SD_CMD_SD_APP_SECURE_READ_MULTIPLE_BLOCK   ((uint8_t)18)  /*!< For SD card only */
#define SD_CMD_SD_APP_SECURE_WRITE_MULTIPLE_BLOCK  ((uint8_t)25)  /*!< For SD card only */
#define SD_CMD_SD_APP_SECURE_ERASE                 ((uint8_t)38)  /*!< For SD card only */
#define SD_CMD_SD_APP_CHANGE_SECURE_AREA           ((uint8_t)49)  /*!< For SD card only */
#define SD_CMD_SD_APP_SECURE_WRITE_MKB             ((uint8_t)48)  /*!< For SD card only */

/**
  * @brief Supported SD Memory Cards
  */
#define STD_CAPACITY_SD_CARD_V1_1             ((uint32_t)0x00000000)
#define STD_CAPACITY_SD_CARD_V2_0             ((uint32_t)0x00000001)
#define HIGH_CAPACITY_SD_CARD                 ((uint32_t)0x00000002)
#define MULTIMEDIA_CARD                       ((uint32_t)0x00000003)
#define SECURE_DIGITAL_IO_CARD                ((uint32_t)0x00000004)
#define HIGH_SPEED_MULTIMEDIA_CARD            ((uint32_t)0x00000005)
#define SECURE_DIGITAL_IO_COMBO_CARD          ((uint32_t)0x00000006)
#define HIGH_CAPACITY_MMC_CARD                ((uint32_t)0x00000007)

#endif // SD card related definitions

// QL_FS File system error codes
#define QLFS_SUCCESS                          0
#define QLFS_ERR_VL_NOT_MOUNT                 -1
#define QLFS_ERROR                            -2
#define QLFS_INVALID_FILE_HANDLE              -3 
#define QLFS_FOPEN_FAIL                       -4
                                                                                                                                                      
/* File system and storage media */
typedef enum
{
    FREERTOS_NONE_MOUNTED = 0,
    FREERTOS_SPI_FLASH = 1,
    FREERTOS_SPI_SD = 2,
}QL_FSStorageMedia;       

/* Entire filename, volume + path + filename */
#define QLFS_MAX_ABS_PATHLEN (200+1)
/* no component of the filename can be larger then this */
#define QLFS_MAX_FILENAME_COMPONENT 50

// QL file media storage handle
typedef struct
{
    char mountVolume[QLFS_MAX_FILENAME_COMPONENT];
    uint8_t pathLen;
    uint8_t isMounted;
    //FF_Disk_t *pDisk;
    void *pDisk; //will be casted to FF_Disk_t or FATFS
    QL_FSStorageMedia fsType;
}QLFS_Handle;

typedef struct
{
	uint32_t total_size;
	uint32_t free_size;
}DISK_PRIVATE_DATA;

// QL file handle
#if USE_FREERTOS_FAT
#define QLFILE_Handle FF_FILE
#endif

#if USE_FATFS
#define QLFILE_Handle FIL
#endif


extern QLFS_Handle* QLFS_mount(QL_FSStorageMedia fsType, const char* path);
extern int32_t QLFS_DiskUnmount(QLFS_Handle *handle);
extern QLFILE_Handle* QLFS_fopen(const QLFS_Handle *handle, const char *pcFile, const char *pcMode);
extern int QLFS_fclose(const QLFS_Handle *handle, QLFILE_Handle *file);
extern size_t QLFS_fwrite(const QLFS_Handle *handle, QLFILE_Handle *file, const void *pvBuffer, size_t xSize, size_t xItems);
extern size_t QLFS_fread(const QLFS_Handle *handle, QLFILE_Handle *file, void *pvBuffer, size_t xSize, size_t xItems);
extern int32_t  QLFS_RmFile(const QLFS_Handle *handle, const char *pcPath);
extern int QLFS_isFilePresent(const QLFS_Handle *handle,const char * file_path);
extern BaseType_t QLFS_DIRCommand(const char *pcPath);
extern uint32_t QLFS_getFreeDiskSpace(const QLFS_Handle *pHandle);
extern int QLFS_formatFlashDisk(void);

extern QLFS_Handle *QLFS_DEFAULT_FILESYTEM;

extern const char QLFS_sdcard_prefix[];
extern const char QLFS_spiflash_prefix[];

extern QLFS_Handle qlfsHandle_sdcard;
extern QLFS_Handle qlfsHandle_spiflash;

extern QLFS_Handle *QLFS_mount_as_default( QL_FSStorageMedia fsType );
/* what text appears at the front of files on the SDCard? */
extern const char QLFS_sdcard_prefix[];
/* what text appears at the front of the files on the SPI memory */
extern const char QLFS_spiflash_prefix[];

/* Given the default filesystem, what text appears at the front of the files in this filesystem */
const char *QLFS_get_default_mount_prefix(void);

/* for THIS file system, what is the prefix text for files, ie: /sdcard, or /spiflash */
const char *QLFS_get_mount_prefix( QL_FSStorageMedia fsType );

/* convert typeid to a handle */
extern QLFS_Handle *QLFS_type2handle( QL_FSStorageMedia fsType );

/* mount this filesystem, what ever "fsType" is */
extern QLFS_Handle *QLFS_mount_this( QL_FSStorageMedia fsType );

typedef struct QLFS_file_find_info QLFS_FILEFINDINFO;

/* generic STAT and directory search info for filesystems. */
struct QLFS_file_find_info {
  intptr_t  magic;
  intptr_t  internal_use;
  void      *fs_handle;
  int       eof; /* also "error" */
    
  struct qlfs_ffi_direntry{
    /* validity of below depends on valid above */
    int       valid;
    uint64_t  filesize_bytes;
    char      filename[255];
    time_t  unix_time_filetime;
#define QLFS_FILE_ATTR_DIRECTORY  0x10 /* msdos defines this bit */
#define QLFS_FILE_ATTR_HIDDEN     0x02 /* msdos defines this bit */
#define QLFS_FILE_ATTR_SYSTEM     0x04 /* msdos defines this bit */
#define QLFS_FILE_ATTR_RDONLY     0x01 /* msdos defines this bit */
#define QLFS_FILE_ATTR_ARCHIVE    0x20 /* msdos defines this bit */
    uint8_t  msdos_attr;
  } dir_entry;
};


/* Initialize a find structure */
extern void QLFS_FindFirst_Init( QLFS_FILEFINDINFO *pInitMe, QLFS_Handle *handle );

/* find the first item */
extern int QLFS_FindFirst( QLFS_FILEFINDINFO *pInitMe, const char *path );

/* find the next item */
extern int QLFS_FindNext( QLFS_FILEFINDINFO *pInitMe );

/* done finding... release resources */
extern void QLFS_FindEnd( QLFS_FILEFINDINFO *pInitMe );

/* return NON-ZERO if the filesystem is mounted or not */
extern int QLFS_isMounted( QLFS_Handle *pHandleToTest );

/* return details about specified file, see: "man 2 stat" for the concept */
extern int QLFS_stat( const char *filename, QLFS_FILEFINDINFO *pInfo, QLFS_Handle *handle );
	
/* checks for presence of specified file */
extern int QLFS_IsFilePresent( const QLFS_Handle *pHandle, const char *pFileName );

extern int QLFS_Ffseek( const QLFS_Handle *pHandle, void *pFilePtr, long lOffset, int iWhence );

extern BaseType_t QLFS_DIRFindFirst(const QLFS_Handle *pHandle, uint32_t *pFileSz, uint32_t *pDateTime, uint8_t *pFileName);

extern BaseType_t QLFS_DIRFindNext(const QLFS_Handle *pHandle, uint32_t *pFileSz, uint32_t *pDateTime, uint8_t *pFileName);

BaseType_t QLFS_RemoveUserFiles(const char *dirPath);

uint32_t QLFS_getDiskSpaceInfo(const QLFS_Handle *pHandle, uint32_t *pTotalSz, uint32_t *pInuseSz);

#if (USE_FATFS)
extern int GetFatFsFileSize(QLFILE_Handle *pFileHandle);
#endif
#endif
