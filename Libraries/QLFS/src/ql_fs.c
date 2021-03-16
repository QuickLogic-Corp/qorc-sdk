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
#include "Fw_global_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
//#include "eoss3_dev.h"
//#include "eoss3_hal_spi.h"
//#include "spi_flash.h"

/* FreeRTOS+FAT includes. */
//#include "ff_headers.h"
//#include "ff_stdio.h"

#include "ql_fs.h"
#include "dbg_uart.h"

#include "micro_tick64.h"
#include "ql_time.h"
#if (USE_FATFS == 1)

#include "ff.h"

//these are not defined for FatFs, but used by QLFS_
#define FF_SEEK_SET	0
#define FF_SEEK_CUR	1
#define FF_SEEK_END	2

#define SPISD_DISK_SECTOR_SIZE           (512)        

extern FATFS *mount_sensortile_SD_card(void);

//forward declarations to remove warnings
int FATFS_SPISDGetFreeDiskSize(void);
int FATFS_SPISDGetDiskSpaceInfo (uint32_t *pTotalSz, uint32_t *pInuseSz);

static uint32_t sector_buff[SPISD_DISK_SECTOR_SIZE/4];//need uint32_t buff for ST Drivers SD_read() and SD_write()
#endif

QLFS_Handle *QLFS_DEFAULT_FILESYTEM;
/* these should NOT have a trailing / */
const char QLFS_sdcard_prefix[] = "/SPISD";
const char QLFS_spiflash_prefix[] = "/SPIFLASH";

QLFS_Handle qlfsHandle_sdcard;
QLFS_Handle qlfsHandle_spiflash;

#if (USE_FREERTOS_FAT == 1)
#include "media_drv_spi_sd.h"
#include "media_drv_spi.h"
#include "ff_stdio.h"
static FF_FindData_t *pxFindStruct = NULL;
static struct QLFS_file_find_info find_info;

#endif

static char file_path[QLFS_MAX_ABS_PATHLEN] = "";    // place holder to storage file path
static void _make_filename( const QLFS_Handle *handle, const char *filename )
{
        memset(file_path, 0,  sizeof(file_path));
        memcpy(file_path, handle->mountVolume, handle->pathLen);
        strcat(file_path, "/");
        strcat(file_path, filename);
        if( file_path[QLFS_MAX_ABS_PATHLEN-1] ){
            dbg_fatal_error("filename overflow");
        }
}

QLFS_Handle *QLFS_mount_as_default( QL_FSStorageMedia fsType )
{
    
    if( fsType == FREERTOS_NONE_MOUNTED ){
        /* We want nothing mounted */
        QLFS_DEFAULT_FILESYTEM  = NULL;
    } else {
        QLFS_DEFAULT_FILESYTEM = QLFS_mount_this( fsType );
    }
    return QLFS_DEFAULT_FILESYTEM;
}


QLFS_Handle *QLFS_type2handle( QL_FSStorageMedia mountid )
{
    QLFS_Handle *result;
    result = NULL;

    switch( mountid ){
    default:
    case FREERTOS_NONE_MOUNTED:
        result = NULL;
        break;
    case FREERTOS_SPI_FLASH:
        result = &qlfsHandle_spiflash;
        break;
    case FREERTOS_SPI_SD:
        result = &qlfsHandle_sdcard;
        break;
    }
    return result;
}

const char *QLFS_get_default_mount_prefix(void)
{
    if( QLFS_DEFAULT_FILESYTEM == NULL ){
        return NULL;
    }

    return QLFS_get_mount_prefix( QLFS_DEFAULT_FILESYTEM->fsType );
}

const char *QLFS_get_mount_prefix( QL_FSStorageMedia mountid )
{
    const char *cp;
    switch( mountid ){
    default:
    case FREERTOS_NONE_MOUNTED:
        cp = NULL;
        break;
    case FREERTOS_SPI_FLASH:
        cp = QLFS_spiflash_prefix;
        break;
    case FREERTOS_SPI_SD:
        cp = QLFS_sdcard_prefix;
        break;
    }
    return cp;
}

uint32_t QLFS_getFreeDiskSpace(const QLFS_Handle *pHandle)
{
    uint32_t diskFreeSpace = 0xFFFFFFFF;
    switch( pHandle->fsType )
    {
        case FREERTOS_SPI_FLASH:
#if( USE_FREERTOS_FAT == 1)
          diskFreeSpace = FF_SPIFlashGetFreeDiskSize();
#elif ( USE_FATFS == 1)
          //diskFreeSpace = FATFS_SPIFlashGetFreeDiskSize();
          diskFreeSpace = 0; //For FATFS needs to be done
#endif
            break;
        case FREERTOS_SPI_SD:
#if( USE_FREERTOS_FAT == 1)
          diskFreeSpace = FF_SPISDGetFreeDiskSize();
#elif ( USE_FATFS == 1)
          diskFreeSpace = FATFS_SPISDGetFreeDiskSize();
#endif
            break;
        default:
            break;
    }
    return diskFreeSpace;
}

uint32_t QLFS_getDiskSpaceInfo(const QLFS_Handle *pHandle,
                               uint32_t *pTotalSz, uint32_t *pInuseSz)
{
    uint32_t result = 0;
    
    switch( pHandle->fsType )
    {
        case FREERTOS_SPI_FLASH:
#if( USE_FREERTOS_FAT == 1)         
        result = FF_SPIFDUN(pTotalSz, pInuseSz);
#elif ( USE_FATFS == 1)
        result = 0; //For FATFS needs to be done 
#endif 
        break;
        case FREERTOS_SPI_SD:
#if( USE_FREERTOS_FAT == 1)         
          result = FF_SPISDGetDiskSpaceInfo(pTotalSz, pInuseSz);
#elif ( USE_FATFS == 1)
          result = FATFS_SPISDGetDiskSpaceInfo(pTotalSz, pInuseSz);
#endif
        break;
        default:
        break;
    }
    
    return result;
}


QLFS_Handle *QLFS_mount_this( QL_FSStorageMedia what )
{
    const char *cp;
    if( what == FREERTOS_NONE_MOUNTED ){
        return NULL;
    } 
    cp = QLFS_get_mount_prefix( what );
    return QLFS_mount( what, cp );
}

#if (USE_FATFS == 1)
FATFS qlfs_fatfs_obj;
#endif

/*
 * @fn        QLFS_mount
 *
 * @brief   This api will enable mounting of the volume on desired storage media SPI flash or SD card. 
            Note- Currently this implementaion only supports SD card over SPI bus. SPI flash support will be added later.
 * @param fsType - Storage media either SPI flash or SD card.
 * @param volumeName - Mount volume name/path.
 * @return - on success pointer to handle otherwise NULL.
 */
QLFS_Handle *QLFS_mount(QL_FSStorageMedia fsType, const char* path)
{
    QLFS_Handle *pHandle;
    
    pHandle = QLFS_type2handle(fsType);
    if( pHandle == NULL ){
        return pHandle;
    }

    if( pHandle->isMounted )
    {
        //dbg_str_str("Already mounted", pHandle->mountVolume );
        return pHandle;
    }
    dbg_str_str("First Mount", pHandle->mountVolume );
printf("Mount Type %d\n",fsType);    
    //FF_Disk_t *pDisk = NULL;
    void *pDisk = NULL;
    
    if(fsType == FREERTOS_SPI_SD)
    {
#if (USE_FREERTOS_FAT == 1)      
        pDisk = FF_SPISDMount(path);
#elif (USE_FATFS == 1)
        FRESULT fr = f_mount(&qlfs_fatfs_obj, path, 1);
        if (fr == FR_OK)
           pDisk = &qlfs_fatfs_obj;
#endif
        if(pDisk != NULL)
        {
            pHandle->isMounted = 1;
            strcpy( pHandle->mountVolume, path );
            pHandle->pathLen = strlen(pHandle->mountVolume);
            pHandle->pDisk = pDisk;
            pHandle->fsType = fsType;
#ifdef SD_TEST
            prvCreateExampleFiles(SPISD_DISK_NAME);
#endif
            return pHandle;
        }
    }
    
#if (USE_FREERTOS_FAT == 1)
    if(fsType == FREERTOS_SPI_FLASH)
    {
        int r;
        r = FF_SPIFlashMount();
        if( r == 0 ){
            /* mount failed */
            return NULL;
        }
        pHandle->isMounted = 1;
        strcpy( pHandle->mountVolume, path );
        pHandle->pathLen = strlen(pHandle->mountVolume);
        pHandle->pDisk = FF_SpiFlashDiskPointer();
        pHandle->fsType = fsType;
        return pHandle;
    }
#elif (USE_FATFS == 1)
    //implement if needed
#endif        
    
    /* fixme? What about spi flash? */
    return NULL;
}
#if (USE_FATFS == 1)
//Reference : Adopted FF_GetModeBits() from FreeRTOS FAT for FatFS Mode
static BYTE get_fatfs_mode(const char *pcMode)
{
    BYTE ucModeBits = 0;
    while( *pcMode != '\0' )
    {
        switch( *pcMode )
        {
            case 'r':                       /* Allow Read. */
            case 'R':
            ucModeBits |= FA_READ;
            break;
            
            case 'w':                       /* Allow Write. */
            case 'W':
            ucModeBits |= FA_WRITE;
            ucModeBits |= FA_CREATE_ALWAYS; /* Create if not exists and truncate if exists. */
            break;

            case 'a':                       /* Append new writes to the end of the file. */
            case 'A':
            ucModeBits |= FA_WRITE;
            ucModeBits |= FA_OPEN_APPEND; /* Create if not exist. */
            break;
            
            case '+':                       /* Allow Read and Write */
            ucModeBits |= FA_READ;
            ucModeBits |= FA_WRITE; /* RW Mode. */
            break;
            
            case 'b':
            case 'B':
            /* b|B flags not supported (Binary mode is native anyway). */
            break;
            
            default:
            break;
        }
        
        pcMode++;
    }
    
    return ucModeBits;
}

#endif
/*
 * @fn        QLFS_fopen
 * @brief   This api will be called to open the file on the mounted volume passed in the file system handle.
 * @param   handle- QL file system handle.
 * @param   pcFile - File name.
 * @param   pcMode- file open mode/attribute.
 * @return  on success pointer to file handle otherwise NULL.
 */
QLFILE_Handle *QLFS_fopen( const QLFS_Handle *handle, const char *pcFile, const char *pcMode)
{
    if(handle->isMounted)
    {

        _make_filename( handle, pcFile );
        QLFILE_Handle *file;
//printf("fopen: %s\n", file_path);
//printf("fopen: %s\n", pcFile);      
#if (USE_FREERTOS_FAT == 1)
        file = ff_fopen( file_path, pcMode); 
#elif (USE_FATFS == 1)
        strcpy(file_path,"/");
        strcat(file_path, pcFile);
printf("fopen: %s\n", file_path);      
        file = ff_malloc(sizeof(FIL));
        FRESULT result;
        if(file != NULL)
        {
            //if(f_open( file, pcFile, get_fatfs_mode(pcMode)) != FR_OK)
            result = f_open( file, file_path, get_fatfs_mode(pcMode));
            if(result != FR_OK)
            {
                ff_free(file);
                file =  NULL;
printf("fopen: failed %d\n", result);
            }
        }
#endif
        // printf("QLFS_fopen:: Time taken in file openig = %d\n", (xTaskGetTickCount()- tickCnt) );
        return file;
    }
    FF_PRINTF("QLFS_fopen:: Error- Volume not mounted\n");
    return NULL;
}

/*
 * @fn      QLFS_fclose
 * @brief   This api will be called to open the file on the mounted volume passed in the file system handle.
 * @param   handle- QL file system handle.
 * @param   pxStream - File handle.
 * @return  0 - on success else error.
 */
int QLFS_fclose(const QLFS_Handle *handle, QLFILE_Handle *file)
{
    if(handle->isMounted)
    {
#if (USE_FREERTOS_FAT == 1)
       int ret = ff_fclose(file);
       if (FREERTOS_SPI_FLASH == handle->fsType)
       {
          SPIFlashCacheFlush();
       }
       return ret;
#elif (USE_FATFS == 1)
        int fatfs_error = (int) f_close(file);
        ff_free(file);
printf("fclose: %d \n", fatfs_error);
        return fatfs_error;
#endif       
    }
    
    FF_PRINTF("QLFS_fclose:: Error- Volume not mounted\n");
    return QLFS_ERR_VL_NOT_MOUNT;
}

/*
 * @fn      QLFS_fwrite
 * @brief   This api will be called to write data from a give file handle..
 * @param   handle- QL file system handle.
 * @param   pvBuffer- pointer to the data buffer.
 * @param   xSize- size of data item
 * @param   xItems- numeber of data items
 * @return  no of xItems written, else error (-1)
 */
size_t QLFS_fwrite(const QLFS_Handle *handle, QLFILE_Handle *file, const void *pvBuffer, size_t xSize, size_t xItems)
{
    if(handle->isMounted)
    {
#if (USE_FREERTOS_FAT == 1)            
        if (FREERTOS_SPI_FLASH == handle->fsType)
        {
           SPIFlashCacheInvalidate();
        }
        return ff_fwrite(pvBuffer, xSize, xItems, file);
#elif (USE_FATFS == 1)
        UINT bytes_written = 0;
        f_write(file, pvBuffer, xSize*xItems, &bytes_written);
printf("fwrite:%d,%d\n", xSize*xItems, bytes_written);
        return bytes_written;
#endif        
    }
    
    FF_PRINTF("QLFS_fwrite:: Error- Volume not mounted\n");
    return QLFS_ERR_VL_NOT_MOUNT;
}

/*
 * @fn      QLFS_fread
 * @brief   This api will be called to read data from a give file handle.
 * @param   handle- QL file system handle.
 * @param   pvBuffer- pointer to the data buffer.
 * @param   xSize- size of data item
 * @param   xItems- numeber of data items
 * @return  number of data items read, else error (-1) 
 */
size_t QLFS_fread(const QLFS_Handle *handle, QLFILE_Handle *file, void *pvBuffer, size_t xSize, size_t xItems)
{
    if(handle->isMounted)
    {
        // Note this hack for the time being as HAL SPI is not able to read more than 512 byte at time
        // need to remove when it is fixed
#if (USE_FREERTOS_FAT == 1)
        if(xSize*xItems <= SPISD_DISK_SECTOR_SIZE)
        {
            return ff_fread(pvBuffer, xSize, xItems, file);
        }
        else
        {
            uint32_t byteToRd = xSize*xItems;
            uint32_t bytesRemaining = byteToRd, currByteRd = SPISD_DISK_SECTOR_SIZE, byteCnt = 0;
            while(bytesRemaining > 0)
            {
                if( ff_fread(((uint8_t *)pvBuffer + byteCnt), currByteRd, 1, file) == 0)
                    return 0;
                byteCnt += currByteRd;
                bytesRemaining -= currByteRd;
                currByteRd = (bytesRemaining > SPISD_DISK_SECTOR_SIZE)?(SPISD_DISK_SECTOR_SIZE):bytesRemaining;
            }
            return byteToRd;
            
        }
#elif (USE_FATFS == 1 )

        uint32_t byteToRd = xSize*xItems;
        uint32_t bytesRemaining = byteToRd, currByteRd = SPISD_DISK_SECTOR_SIZE, byteCnt = 0;
        uint32_t bytes_read;
        if(byteToRd <= currByteRd)
            currByteRd = byteToRd;
        while(bytesRemaining > 0)
        {
            bytes_read = 0;
            //Note: the SD_read() assumes the buffer tyep uint32_t. So, first read 
            // the data into unit32_t buffer then copy it into given buffer
            //if( f_read(file, ((uint8_t *)pvBuffer + byteCnt), currByteRd, &bytes_read) != FR_OK)
            if( f_read(file, ((uint8_t *)&sector_buff[0]), currByteRd, &bytes_read) != FR_OK)
                return 0;
            memcpy(((uint8_t *)pvBuffer + byteCnt), ((uint8_t *)&sector_buff[0]), currByteRd); 
            byteCnt += currByteRd;
            bytesRemaining -= currByteRd;
            currByteRd = (bytesRemaining > SPISD_DISK_SECTOR_SIZE)?(SPISD_DISK_SECTOR_SIZE):bytesRemaining;
        }
        return byteToRd;
#endif        
    }
    
    FF_PRINTF("QLFS_fread:: Error- Volume not mounted\n");
    return QLFS_ERR_VL_NOT_MOUNT;
}

int QLFS_Ffseek( const QLFS_Handle *pHandle, void *pFilePtr, long lOffset, int iWhence )
{
    if(pHandle->isMounted)
    {
#if (USE_FREERTOS_FAT == 1) 
        return ff_fseek( (FF_FILE *)pFilePtr, lOffset, iWhence );       
#elif (USE_FATFS == 1)
        if ( iWhence == FF_SEEK_SET)
        {
          return f_lseek((FIL *)pFilePtr,lOffset);  
        }
        //else //@TODO implement FF_SEEK_CUR for FatFS
        //else //@TODO implement FF_SEEK_END for FatFS
#endif        
    }
    return QLFS_ERR_VL_NOT_MOUNT;
}
/*
 * @fn      QLFS_RmFile
 * @brief   This api will be called to read data from a give file handle.
 * @param   handle- QL file system handle.
 * @param   pcPath - file name to be deleted
 * @return  0 on success, else -1  
 */
int32_t QLFS_RmFile(const QLFS_Handle *handle, const char *pcPath)
{
    if(handle->isMounted)
    {
        _make_filename( handle, pcPath );
#if (USE_FREERTOS_FAT == 1)        
        return ff_remove(file_path);
#elif (USE_FATFS == 1)
strcpy(file_path,"/");
strcat(file_path, pcPath);
printf("Del: %s\n", file_path);
vTaskDelay(2);
//        return f_unlink(file_path); //restrictions apply
int err = f_unlink(file_path); 
if (err != 0)
{
  printf("cannot del %d - %s \n", err, file_path);
}
          return err;
#endif        
    }
    
    FF_PRINTF("QLFS_RmFile:: Error- Volume not mounted\n");
    return QLFS_ERR_VL_NOT_MOUNT;
}

/*
* @fn      QLFS_IsFilePresent
* @brief   This api can be called to if a file is present.
* @param   handle- QL file system handle.
* @param   file name to be checked
* @return  1 if file is present, 0 if not
*/
int QLFS_IsFilePresent(const QLFS_Handle *pHandle, const char *pFileName)
{
    int ret = 0;
    if(pHandle->isMounted)
    {
        _make_filename( pHandle, pFileName );
        
#if (USE_FREERTOS_FAT == 1)        
        FF_FILE *fp;
        fp = ff_fopen(file_path, "r");
        
        if(fp != NULL)
        {
            ret = 1;
        } 
        ff_fclose(fp);
        
#elif (USE_FATFS == 1)
        strcpy(file_path,"/");
        strcat(file_path, pFileName);
printf("fpresent: %s\n", file_path);      
        FIL *fp = ff_malloc(sizeof(FIL));
        if(fp != NULL)
        {
            if(f_open( fp, file_path, get_fatfs_mode("r")) == FR_OK)
            {
              ret = 1;
              f_close(fp);
            }
            ff_free(fp);
        }
#endif        
    }
    return ret;
}

#if (USE_FREERTOS_FAT == 1)
/*
 * @fn      QLFS_DiskUnmount
 * @brief   This api will enable un-mounting of file system. Internally will deselect the SPI slave select to
 *          release the SPI bus.
 * @param   handle- QL file system handle.
 * @return  returns - 0 one success, else -1 
 */
int32_t QLFS_DiskUnmount(QLFS_Handle *handle)
{
    int32_t ret = QLFS_ERROR;
    if( handle==NULL){
        return QLFS_SUCCESS;
    }
	FF_Error_t error = FF_Unmount(handle->pDisk);
    if(error == FF_ERR_NONE)
    {
        handle->isMounted = 0;
        handle->pDisk = NULL;
        ret = QLFS_SUCCESS;
    }
    
    FF_PRINTF("QLFS_DiskUnmount:: Error- DiskUnMount failed\n");
    return ret;    
}

/*
 * @fn       QLFS_ShowPartition
 * @brief    This api will display partition information UART console. 
             Note- Currently this implementation only supports SD card over SPI bus. SPI flash support will be added later.
 * @return - '0' on success otherwise nonzero.
 */
int32_t QLFS_ShowPartition(QLFS_Handle *handle)
{
    if((handle->isMounted) && (handle->fsType == FREERTOS_SPI_SD))
    {
        FF_SPISDDiskShowPartition(handle->pDisk);
        return QLFS_SUCCESS;
    }
    //@TBD - SPI flash support
    // if(handle->isMounted == 0)
    FF_PRINTF("qlfs_ShowPartition:: Volume not mounted\n");
    return QLFS_ERR_VL_NOT_MOUNT;
}

/*
 * @fn      QLFS_GetSectorNClustorSize
 * @brief   This api will return the sector and cluster  size of the mounted volume
 * @param   handle - QL file system handle@param handle- QL file system handle.
 * @param   sectorSize - pointer to sector size variable
 * @param   clustorSize - pointer to cluster size variable
 * @return  None.
 */
void QLFS_GetSectorNClusterSize(QLFS_Handle *handle, uint32_t *sectorSize, uint32_t *clusterSize)
{
    if(handle->isMounted)
    {
        *sectorSize = ((FF_Disk_t *)(handle->pDisk))->pxIOManager->usSectorSize;
        *clusterSize = (*sectorSize)*(((FF_Disk_t *)(handle->pDisk))->pxIOManager->xPartition.ulSectorsPerCluster);
    }
    //@TBD - SPI flash support
    else // if(handle->isMounted == 0)
    {
        *sectorSize = 0;
        *clusterSize = 0;                  
    }  
    FF_PRINTF("qlfs_ShowPartition:: Volume not mounted\n");
    return;
}

/*!
 *\fn static int prvCreateFileInfoString(FF_FindData_t *pxFindStruct)
 *\param file_path -- FF_FindData_t data strcture
 *\brief Prints the information conatined in the data structure
*/
static void prvCreateFileInfoString( FF_FindData_t *pxFindStruct )
{
    const char * pcWritableFile = "writable file", *pcReadOnlyFile = "read only file", *pcDirectory = "directory";
    const char * pcAttrib;

	/* Point pcAttrib to a string that describes the file. */
	if( ( pxFindStruct->ucAttributes & FF_FAT_ATTR_DIR ) != 0 )
	{
		pcAttrib = pcDirectory;
	}
    else if( pxFindStruct->ucAttributes & FF_FAT_ATTR_READONLY )
	{
		pcAttrib = pcReadOnlyFile;
	}
	else
	{
		pcAttrib = pcWritableFile;
	}

	/* Create a string that includes the file name, the file size and the
	attributes string. */
	printf("\n%s [%s] [size=%d]", pxFindStruct->pcFileName, pcAttrib, ( int ) pxFindStruct->ulFileSize );
}

/*
 * @fn      QLFS_DIRCommand
 * @brief   This api will implements the dir command
 * @param   pcPath- path to directory.
 * @return  pdTRUE on success else pdFALSE.
 */
BaseType_t QLFS_DIRCommand(const char *pcPath)
{
    static FF_FindData_t *pxFindStruct = NULL;
    int iReturned;
    static uint32_t fileEntryCnt = 0;
printf(":DIR ::");
    BaseType_t xReturn = pdFALSE;
	if( pxFindStruct == NULL )
	{

		/* This is the first time this function has been executed since the Dir
		command was run.  Create the find structure. */
		pxFindStruct = ( FF_FindData_t * ) pvPortMalloc( sizeof( FF_FindData_t ) );
		if( pxFindStruct != NULL )
		{         
			memset( pxFindStruct, 0x00, sizeof( FF_FindData_t ) );
			iReturned = ff_findfirst( pcPath, pxFindStruct );
			if( iReturned == FF_ERR_NONE )
			{
				prvCreateFileInfoString( pxFindStruct );
                fileEntryCnt++;
                while( iReturned == FF_ERR_NONE)
                {
                    memset( pxFindStruct->xDirectoryEntry.pcFileName, 0x00, ffconfigMAX_FILENAME );
                    iReturned =  ff_findnext( pxFindStruct );
                    prvCreateFileInfoString( pxFindStruct );
                    fileEntryCnt++;
                }
                xReturn = pdPASS;
                
                //free this after printing all the information
                vPortFree( pxFindStruct );
                pxFindStruct = NULL;
            }
            else
            {
                printf( "Error: ff_findfirst() failed." );
                vPortFree( pxFindStruct );
                pxFindStruct = NULL;
            }
        }
        else
        {
            printf( "Failed to allocate RAM (using heap_4.c will prevent fragmentation)." );
        }
    }
    else
    {
        vPortFree( pxFindStruct );
        pxFindStruct = NULL;
    }
    printf("\n");
    return xReturn;
}

BaseType_t QLFS_RemoveUserFiles(const char *dirPath)
{
    static FF_FindData_t *pxFindStruct = NULL;
    int iReturned;
    
    BaseType_t xReturn = pdFALSE;
    if( pxFindStruct == NULL )
    {
        
        /* This is the first time this function has been executed since the Dir
        command was run.  Create the find structure. */
        pxFindStruct = ( FF_FindData_t * ) pvPortMalloc( sizeof( FF_FindData_t ) );
        if( pxFindStruct != NULL )
        {
            memset( pxFindStruct, 0x00, sizeof( FF_FindData_t ) );
            iReturned = ff_findfirst( dirPath, pxFindStruct );
            if( iReturned == FF_ERR_NONE )
            {
                prvCreateFileInfoString( pxFindStruct );
                while( iReturned == FF_ERR_NONE)
                {
                    printf("File name: %s\n", pxFindStruct->pcFileName);
                    /* Point pcAttrib to a string that describes the file. */
                    if( ( pxFindStruct->ucAttributes & FF_FAT_ATTR_DIR ) != 0
                       || ( pxFindStruct->ucAttributes & FF_FAT_ATTR_READONLY )
                           || (!strcmp("mcu_ffe.bin", pxFindStruct->pcFileName)) )
                    {
                        //pcAttrib = pcReadOnlyFile;
                        printf("Dir of readOnly file: %s\n", pxFindStruct->pcFileName);
                    }
                    else
                    {
                        _make_filename(QLFS_DEFAULT_FILESYTEM, pxFindStruct->pcFileName);
                        xReturn = ff_remove(file_path);
                    }
                    iReturned =  ff_findnext( pxFindStruct );
                }
				xReturn = pdPASS;
			}
			else
			{
				printf( "Error: ff_findfirst() failed." );
				pxFindStruct = NULL;
			}
		}
		else
		{
			printf( "Failed to allocate RAM (using heap_4.c will prevent fragmentation)." );
		}
	}
	else
	{
		vPortFree( pxFindStruct );
		pxFindStruct = NULL;
	}
	return xReturn;
}

static void verify_magic( QLFS_FILEFINDINFO *pFI )
{
    if( (pFI->magic != (intptr_t)(&verify_magic)) || (pFI->internal_use==0) ){
        for(;;){
            dbg_str("bad-ffi-magic\n");
        }
    }
}

/* see ql_fs.h */
void QLFS_FindFirst_Init( QLFS_FILEFINDINFO *pInitMe, QLFS_Handle *handle )
{
    memset( (void *)(pInitMe), 0, sizeof(*pInitMe) );
    pInitMe->fs_handle = handle;
    
    pInitMe->internal_use = ( intptr_t) pvPortMalloc( sizeof( FF_FindData_t ) );
    if( pInitMe->internal_use != 0 ){
        memset( (void *)(pInitMe->internal_use), 0, sizeof( FF_FindData_t )  );
        pInitMe->magic = (intptr_t)(&verify_magic);
    }
}

static void
_extract_file_details(  QLFS_FILEFINDINFO *pFI, FF_FindData_t *pF )
{
    struct tm tm;
    pFI->dir_entry.msdos_attr = pF->ucAttributes;
    pFI->dir_entry.filesize_bytes = pF->ulFileSize;
    tm.tm_year = pF->xDirectoryEntry.xModifiedTime.Year - 1900;
    tm.tm_mon  = pF->xDirectoryEntry.xModifiedTime.Month -1;
    tm.tm_mday = pF->xDirectoryEntry.xModifiedTime.Day;
    tm.tm_hour = pF->xDirectoryEntry.xModifiedTime.Hour;
    tm.tm_min  = pF->xDirectoryEntry.xModifiedTime.Minute;
    tm.tm_sec  = pF->xDirectoryEntry.xModifiedTime.Second;
    pFI->dir_entry.unix_time_filetime = mktime( &tm );
}

/* see ql_fs.h */
int QLFS_FindFirst( QLFS_FILEFINDINFO *pFI, const char *pattern )
{
    int r;
    FF_FindData_t *pF;
printf(":fFF ::");
    verify_magic( pFI );
    pF = (FF_FindData_t *)(pFI->internal_use);

    memset( &(pFI->dir_entry), 0, sizeof( pFI->dir_entry ) );
            
    _make_filename( pFI->fs_handle, pattern );
    r = ff_findfirst( file_path, pF );
    if( r != FF_ERR_NONE ){
        pFI->eof = 1;
        return r;
    }
    
    _extract_file_details( pFI, pF );
    return r;
}

/* see ql_fs.h */
int QLFS_FindNext( QLFS_FILEFINDINFO *pFI )
{
    int r;
    FF_FindData_t *pF;
printf(":fFN ::");    
    verify_magic( pFI );
    pF = (FF_FindData_t *)(pFI->internal_use);

    memset( &(pFI->dir_entry), 0, sizeof( pFI->dir_entry ) );
            
    r = ff_findnext(pF) ;
    if( r != FF_ERR_NONE ){
        pFI->eof = 1;
        return r;
    }
    
    _extract_file_details( pFI, pF );
    return r;
}

/* see ql_fs.h */
void QLFS_FindEnd( QLFS_FILEFINDINFO *pFI )
{
    FF_FindData_t *pF;
    
    verify_magic( pFI );
    pF = (FF_FindData_t *)(pFI->internal_use);
    /* make structure unusable, but don't wack other parts */
    pFI->internal_use = 0;
    pFI->magic = 0;
    if( pF ){
        vPortFree( (void *)(pF) );
    }
}    

#if (USE_FREERTOS_FAT == 1)
/*
* @fn      QLFS_DIRFindFirst
* @brief   This api will implements the dir command
* @param   pcPath- path to directory.
* @return  pdTRUE on success else pdFALSE.
*/
BaseType_t QLFS_DIRFindFirst(const QLFS_Handle *pHandle, uint32_t *pFileSz, uint32_t *pDateTime, uint8_t *pFileName)
{
    //static FF_FindData_t *pxFindStruct = NULL;
    int iReturned;
    const char *pcPath = QLFS_get_mount_prefix(pHandle->fsType);
printf(":dFF %s::",pcPath);
    BaseType_t xReturn = pdFALSE;
    if( pxFindStruct == NULL )
    {
        
        /* This is the first time this function has been executed since the Dir
        command was run.  Create the find structure. */
        pxFindStruct = ( FF_FindData_t * ) pvPortMalloc( sizeof( FF_FindData_t ) );
        if( pxFindStruct != NULL )
        {         
            memset( pxFindStruct, 0x00, sizeof( FF_FindData_t ) );
            iReturned = ff_findfirst( pcPath, pxFindStruct );
            if( iReturned == FF_ERR_NONE )
            {
                _extract_file_details( &find_info, pxFindStruct );
                *pFileSz = pxFindStruct->ulFileSize;
                *pDateTime = find_info.dir_entry.unix_time_filetime;
                strcpy((char *)pFileName, pxFindStruct->pcFileName);
                xReturn = pdPASS;
printf(":fD %s::",pFileName);
            }
            else
            {
                printf( "Error: ff_findfirst() failed." );
                vPortFree( pxFindStruct );
                pxFindStruct = NULL;
            }
        }
        else
        {
            printf( "Failed to allocate RAM (using heap_4.c will prevent fragmentation)." );
        }
    }
    
    //printf("\n");
    return xReturn;
}

/*
* @fn      QLFS_DIRFindNext
* @brief   This api first file of file system
* @param   pcPath- path to directory.
* @return  pdTRUE on success else pdFALSE.
*/
BaseType_t QLFS_DIRFindNext(const QLFS_Handle *pHandle, uint32_t *pFileSz, uint32_t *pDateTime, uint8_t *pFileName)
{
    //static FF_FindData_t *pxFindStruct = NULL;
    int iReturned;
    BaseType_t xReturn = pdFALSE;
    const char *pcPath = QLFS_get_mount_prefix(pHandle->fsType);
printf(":dFN ::");
    if( pxFindStruct != NULL )
    {         
        memset( pxFindStruct->xDirectoryEntry.pcFileName, 0x00, ffconfigMAX_FILENAME );
        iReturned =  ff_findnext( pxFindStruct );
        
        while(((!strcmp(pxFindStruct->pcFileName, ".\0"))||(!strcmp(pxFindStruct->pcFileName, "..\0")))&&
              ( FF_GETERROR( iReturned ) != FF_ERR_DIR_END_OF_DIR ))
        {
            memset( pxFindStruct->xDirectoryEntry.pcFileName, 0x00, ffconfigMAX_FILENAME );
            iReturned =  ff_findnext( pxFindStruct );
        }
        
        if( iReturned == FF_ERR_NONE)
        {
            _extract_file_details( &find_info, pxFindStruct );
            *pFileSz = pxFindStruct->ulFileSize;
            *pDateTime = find_info.dir_entry.unix_time_filetime;
            strcpy((char *)pFileName, pxFindStruct->pcFileName);
            xReturn = pdPASS;
printf(":fN %s::",pFileName);
        }
        else
        {
            if(FF_GETERROR( iReturned ) == FF_ERR_DIR_END_OF_DIR)
            {
                //not an error, only the end of the dir
                strcpy((char *)pFileName, pxFindStruct->pcFileName);
                xReturn = pdPASS;
printf(":fN End %s::",pFileName);
            }
            else
            {
                printf( "Error: ff_findnext() failed." );
            }
            
            vPortFree( pxFindStruct );
            pxFindStruct = NULL;
        }
    }
    //printf("\n");
    return xReturn;
}
#endif

/* goal:  equal to: "man 2 stat" on linux */
int QLFS_stat( const char *filename, QLFS_FILEFINDINFO *pFI, QLFS_Handle *fsHandle  )
{
    FF_Stat_t statinfo;
    int r;
    
    memset( (void *)(pFI), 0, sizeof(*pFI) );
    _make_filename( fsHandle, filename );
    r = ff_stat( file_path, &statinfo );
    if( r != 0 ){
        pFI->eof = 1;
    } else {
        pFI->eof = 1;
        pFI->dir_entry.valid = 1;
        pFI->dir_entry.filesize_bytes = statinfo.st_size;
#if (FF_FA_RDONLY != QLFS_FILE_ATTR_RDONLY) || (FF_FA_DIREC!= QLFS_FILE_ATTR_DIRECTORY)
#error FIX ME
        /* note we only check a couple.. which is good enough */
        /* this code assumes the attribute bits are the same */
#else
        pFI->dir_entry.msdos_attr = statinfo.st_mode;
#endif
          pFI->dir_entry.unix_time_filetime = statinfo.st_mtime;
    }
    if( !pFI->dir_entry.valid ){
        return -1;
    } else {
        return 0;
    }
}
#endif
                       
int QLFS_formatFlashDisk(void)
{
#if( USE_FREERTOS_FAT == 1)  
    return (FF_SPICreatePartition() == 1)? 0 : 1;
#elif ( USE_FATFS == 1)
    return 0; //for FATFS don't do anything
#endif
}

#if (USE_FATFS == 1)
/* Equivalent of time() : returns a 32 bit number in the format
   ((DWORD)(YEAR - 1980) << 25 | (MONTH << 21) | (DAY << 16) | 
          (HOURS << 11) | (MINS << 5) | (SECS/2) )
   Return Value

   Currnet local time shall be returned as bit-fields packed into a DWORD value.
   The bit fields are as follows:

   bit31:25
     Year origin from the 1980 (0..127, e.g. 37 for 2017)
   bit24:21
    Month (1..12)
   bit20:16
    Day of the month (1..31)
   bit15:11
    Hour (0..23)
   bit10:5
    Minute (0..59)
   bit4:0
    Second / 2 (0..29, e.g. 25 for 50)
*/
DWORD get_fattime (void)
{
    uint64_t t_secs = xTaskGet_uSecCount()/(1000ULL*1000ULL);
    time_t  t_32 = (time_t)t_secs; 
    struct tm t_data;
    ql_gmtime_r((const time_t *)&t_32, &t_data);
    t_data.tm_mon += 1; //should start with 1
    t_data.tm_year -= (80); //should be with relative to 1980 not 1900
    
    DWORD t2 = t_data.tm_sec + (t_data.tm_min *60) + (t_data.tm_hour *3600);;
    t2  = t2 >> 1; //seconds/2
    t2 |= (t_data.tm_mday << 16);
    t2 |= (t_data.tm_mon << 21);
    t2 |= (t_data.tm_year << 25);
    return t2;
}

int GetFatFsFileSize(QLFILE_Handle *pFileHandle)
{
    return f_size(pFileHandle);
}

FRESULT FATFS_GetDiskSectors(const TCHAR* path, uint32_t *pTotalSectors, uint32_t *pFreeSectors )
{

    FATFS *fs;
    DWORD fre_clust, fre_sect, tot_sect;
    FRESULT res;
    
    /* Get volume information and free clusters of drive 1 */
    res = f_getfree(path, &fre_clust, &fs);
    //if (res) die(res);

    /* Get total sectors and free sectors */
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;
    
    *pTotalSectors = tot_sect;
    *pFreeSectors = fre_sect;
      
    /* Print the free space (assuming 512 bytes/sector) */
    //printf("%10lu KiB total drive space.\n%10lu KiB available.\n", tot_sect / 2, fre_sect / 2);
    
    return res;
}

int FATFS_SPIFlashGetFreeDiskSize(void)
{
  uint32_t pTotalSectors, pFreeSectors;
  FRESULT res;
  res = FATFS_GetDiskSectors("2:", &pTotalSectors, &pFreeSectors );
  if(res)
    return 0;
  else
    return pFreeSectors*4096; //assuming 4096byte per sector for Flash drive
}

int FATFS_SPISDGetFreeDiskSize(void)
{
  uint32_t pTotalSectors, pFreeSectors;
  FRESULT res;
  res = FATFS_GetDiskSectors("1:", &pTotalSectors, &pFreeSectors );
  if(res)
    return 0;
  else
    return pFreeSectors*512; //assuming 512byte per sector

}

int FATFS_SPISDGetDiskSpaceInfo (uint32_t *pTotalSz, uint32_t *pInuseSz)
{
    uint32_t pTotalSectors, pFreeSectors;
    FRESULT res;
    
    res = FATFS_GetDiskSectors("1:", &pTotalSectors, &pFreeSectors);
    if(res)
    {
      *pTotalSz = 0;
      *pInuseSz = 0;
    }
    else
    {
      *pTotalSz = pTotalSectors*512; //assuming 512byte per sector
      *pInuseSz = (pTotalSectors - pFreeSectors)*512; //assuming 512byte per sector
    }
    return res; //0= no error
}
/*
fdate
    The date when the file was modified or the directory was created.

    bit15:9
        Year origin from 1980 (0..127)
    bit8:5
        Month (1..12)
    bit4:0
        Day (1..31)

ftime
    The time when the file was modified or the directory was created.

    bit15:11
        Hour (0..23)
    bit10:5
        Minute (0..59)
    bit4:0
        Second / 2 (0..29)

*/
static uint32_t convert_fatfs_time(FILINFO *pFI)
{
    //return ((uint32_t)pFI->fdate << 16) | (uint32_t)pFI->ftime;
    
    struct tm tm;
    tm.tm_year = ((pFI->fdate >> 9) & 0x7F) + 1980 - 1900;
    tm.tm_mon  = ((pFI->fdate >> 5) & 0x0F)  -1;
    tm.tm_mday = ((pFI->fdate >> 0) & 0x1F);
    tm.tm_hour = ((pFI->ftime >> 11) & 0x1F);
    tm.tm_min  = ((pFI->ftime >> 5) & 0x3F);
    tm.tm_sec  = ((pFI->ftime >> 0) & 0x1F) *2;
    uint32_t unix_time = mktime( &tm );
    return unix_time;
}

static DIR fatfsDir; //used to get info
static FILINFO fatfsFileinfo; //used to get info
static int fatfsFindfirst = 0;
BaseType_t QLFS_DIRFindFirst(const QLFS_Handle *pHandle, uint32_t *pFileSz, uint32_t *pDateTime, uint8_t *pFileName)
{
  BaseType_t xReturn = pdFALSE;
  const char *pcPath = QLFS_get_mount_prefix(pHandle->fsType);
printf(":dFF %s::",pcPath);

  FRESULT result;
//  if (fatfsFindfirst == 1)
//    return pdPASS;
  memset((void *)&fatfsDir, 0,  sizeof(fatfsDir));
  memset((void *)&fatfsFileinfo, 0,  sizeof(fatfsFileinfo));
  result = f_findfirst(&fatfsDir,&fatfsFileinfo, "/", "*.*");
  //result = f_findfirst(&fatfsDir,&fatfsFileinfo, pcPath, "*.*");
  if (result == FR_OK) {
    *pFileSz  = fatfsFileinfo.fsize; 
    *pDateTime = convert_fatfs_time(&fatfsFileinfo);
    strcpy((char *)pFileName, fatfsFileinfo.fname);
    xReturn = pdPASS;
    fatfsFindfirst = 1;
printf(":fD %s::\n",pFileName);
  }
  return xReturn; //not impemented yet
}

BaseType_t QLFS_DIRFindNext(const QLFS_Handle *pHandle, uint32_t *pFileSz, uint32_t *pDateTime, uint8_t *pFileName)
{
  BaseType_t xReturn = pdFALSE;
  FRESULT result;
printf(":dFN ::");  
  //need to call findfist once
//  if (fatfsFindfirst == 0)  {
//    if(QLFS_DIRFindFirst(pHandle, pFileSz, pDateTime, pFileName) == pdFALSE)
//      return pdFALSE;
//  }
  if(fatfsFindfirst == 0)
  {
    printf("Find First is not called \n");
  }
  memset((void *)&fatfsFileinfo, 0,  sizeof(fatfsFileinfo));
  result = f_findnext(&fatfsDir,&fatfsFileinfo);
  if (result == FR_OK) {
    *pFileSz  = fatfsFileinfo.fsize; 
    *pDateTime = convert_fatfs_time(&fatfsFileinfo);
    strcpy((char *)pFileName, fatfsFileinfo.fname);
    xReturn = pdPASS;
    //if lastfile then need to start again
    if(pFileName[0] == '\0') 
    {
      fatfsFindfirst = 0;
    }
printf(":fN %s, %d ::\n",pFileName, *pFileSz );
  }
  else
  {
    xReturn = pdFALSE;
    printf("QLFS -- Error %d \n", result);
    
    // erros is end end of dir ??
    fatfsFindfirst = 0;
    pFileName[0] = '\0';
    xReturn = pdPASS;
  }

  return xReturn; 
}


#endif
