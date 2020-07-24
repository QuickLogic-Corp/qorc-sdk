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
*    File   : MinorTopicDispatch_Storage.c
*    Purpose: File transfer topic dispatch
*                                                          
*=========================================================*/
#include "Fw_global_config.h"
#include "RtosTask.h"
#include "iop_messages.h"
#include "DataCollection.h"
#include "Mqttsn_MessageHandler.h"
#include "Mqttsn_Topics.h"
#include "ql_fs.h"
#include "crc16.h"
#include "dbg_uart.h"
#include <string.h>

#define FILE_TRANSFER_DEBUG 0
#define FILENAME_LEN_MAX 60
#define INVALID_SEQUENCE 0xFFFFFFFF
#define FILE_TRANSFER_GET_SIZE_BYTES    1024
#define FILE_TRANSFER_PUT_SIZE_BYTES    1024

/* dispatch entry */
struct storage_cmd_dispatch_entry {
    uint32_t value;
    void (*handler)(Mqttsn_IOMsgData_t *);
    uint32_t expPayldLength;
    uint8_t isVariableLen;
};

typedef struct{
    uint8_t filename[FILENAME_LEN_MAX];
    uint32_t numFrags;
    uint32_t blockSize;
    uint32_t transId;
    uint32_t fileSz;
    uint16_t fileCrc;
}FileTransferInfo_t;

typedef struct{
    FileTransferInfo_t ftInfo;
    QLFS_Handle *pStorageHandle;
    QLFILE_Handle *    pFileHandle;        // file pointer
    uint32_t           seqNo;           // sequence number of current block/fragment accessed
    uint16_t           prevCrc;
}FileStroageInfo_t;

uint32_t dirLastFile = 0;

FileStroageInfo_t currentStorageGet;   // Place holder for current
FileStroageInfo_t currentStoragePut;

#define MAX_PATH_LEN 10

extern char *pSupportedPaths[NUM_SUPPORTED_PATHS];


#define GET_FILE_NAME(x) ((strrchr((const char *)(x),'/'))+1)

static QLFS_Handle *GetHandle(uint8_t *pPath)
{
    if(strncmp((char const *)pPath, "/default/", strlen("/default/")) == 0)
    {
        return QLFS_DEFAULT_FILESYTEM;
    }
    else if(strncmp((char const *)pPath, "/SPIFLASH/", strlen("/SPIFLASH/")) == 0)
    {
#if (QLFS_NO_SPI_FLASH == 1)     
        return NULL;
#else
        //if SPIFlash is not mounted, mount here
        if(qlfsHandle_spiflash.isMounted == 0)
        {
            QLFS_mount_this(FREERTOS_SPI_FLASH);
        }
        
        return &qlfsHandle_spiflash;
#endif        
    }
    else
    {
        return NULL;
    }
}

sys_error_code_t GetNextFileInfo(uint8_t *pPath, uint32_t firstReq, uint16_t *pFileIndex,
                                uint32_t *pFileSz, uint32_t *pDateTime, uint8_t *pFileName)
{
    //int retVal = FF_ERR_NONE;
    BaseType_t retVal = pdPASS;
    static uint16_t fileIndex = 0;

    if(strcmp((char const *)pPath, "/*.*") == 0)
    {
        static uint32_t pathIdx = 0;
        
        if(firstReq)
        {
            pathIdx = 0;
        }
            
        if(pathIdx<NUM_SUPPORTED_PATHS)
        {
            strcpy((char *)pFileName, pSupportedPaths[pathIdx++]);
        }
        else
        {
            pFileName[0] = '\0';
        }
        retVal = pdPASS;
    }
    else
    {
        QLFS_Handle *pQlfsHandle = GetHandle(pPath);
        
        if(pQlfsHandle == NULL)
        {
            //request is not supported
            return SYS_ERR_ENOTSUP;
        }
        
         if(firstReq)
        {
            retVal = QLFS_DIRFindFirst(pQlfsHandle, pFileSz, pDateTime, pFileName);
        }
        else
        {
            retVal = QLFS_DIRFindNext(pQlfsHandle, pFileSz, pDateTime, pFileName);   
        }
    }
    
    if(firstReq)
    {
        fileIndex = 0;
    }
    else
    {
        fileIndex += 1;
    }
    
    if(retVal == pdPASS)
    {
        *pFileIndex = fileIndex;
        
        return SYS_ERR_NO_ERROR;
    }
    else
    {
        *pFileIndex = 0xffff;
        *pFileSz = 0xffffffff;
        *pDateTime = 0xffffffff;
        
        return SYS_ERR_EINVAL;
    }

}

sys_error_code_t Delete(uint8_t *pFileName)
{
    int32_t err;
    QLFS_Handle *pHandle = GetHandle(pFileName);
    
    if(pHandle != NULL)
    {
        uint8_t *pName =  (uint8_t *)GET_FILE_NAME(pFileName);
                
        err = QLFS_RmFile( pHandle, (char const *)pName );
        
        if(err == 0)
            return SYS_ERR_NO_ERROR;
        else
            return SYS_ERR_EINVAL;   //invalid
    }
    else
    {
        //request is not supported
        return SYS_ERR_ENOTSUP;
    }
}


sys_error_code_t GetStorageSpace(uint8_t *pPath, uint32_t *pTotalKbs, uint32_t *pTotalUsedKbs, 
                                uint32_t *pPutSizeBytes, uint32_t *pGetSizeBytes)
{
    QLFS_Handle *pHandle = GetHandle(pPath);
    
    if(pHandle != NULL)
    {
        uint32_t status = QLFS_getDiskSpaceInfo(pHandle, pTotalKbs, pTotalUsedKbs);

        if(status == 0)
        {
            return SYS_ERR_NO_ERROR;
        }
        else
        {
            return SYS_ERR_ENODEV;
        }
    }
    else
    {
        return SYS_ERR_ENOTSUP;
    }
}

sys_error_code_t FileTransferGetInit(FileTransferInfo_t *pFTInfo)
{
    QLFS_Handle *pHandle = GetHandle(pFTInfo->filename);
    
    if(pHandle != NULL)
    {
        uint8_t *pFilename =  (uint8_t *)GET_FILE_NAME(pFTInfo->filename);
        
        if(QLFS_IsFilePresent(pHandle, (char const *)pFilename))
        {
            QLFILE_Handle *pFileHandle = currentStorageGet.pFileHandle = QLFS_fopen( pHandle, (char const *)pFilename, "rb" );
            if(pFileHandle != NULL)
            {
                memset(&currentStorageGet, 0, sizeof(FileTransferInfo_t));
                memcpy(&currentStorageGet.ftInfo.filename, pFilename, strlen((char const *)pFilename));
                currentStorageGet.ftInfo.blockSize = pFTInfo->blockSize;
                currentStorageGet.ftInfo.transId = pFTInfo->transId;
                currentStorageGet.pStorageHandle = pHandle;
                currentStorageGet.pFileHandle = pFileHandle;
                currentStorageGet.seqNo = INVALID_SEQUENCE;
                return SYS_ERR_NO_ERROR;
            }
            else
            {
                return SYS_ERR_EINVAL;
            }
        }
        else
        {
            return SYS_ERR_ENOENT;
        }
    }
    else
    {
        //error not supported
        return SYS_ERR_ENOTSUP;
    }
}

sys_error_code_t FileTransferPutInit(FileTransferInfo_t *pFTInfo)
{
    QLFS_Handle *pHandle = GetHandle(pFTInfo->filename);
    
    if(pHandle != NULL)
    {
        uint8_t *pFilename =  (uint8_t *)GET_FILE_NAME(pFTInfo->filename);
        
        if(QLFS_getFreeDiskSpace(pHandle) > pFTInfo->fileSz)
        {
            QLFILE_Handle *pFileHandle = currentStoragePut.pFileHandle = QLFS_fopen( pHandle, (char const *)pFilename, "wb" );
            if(pHandle != NULL)
            {
                memset(&currentStoragePut, 0, sizeof(FileTransferInfo_t));
                memcpy(&currentStoragePut.ftInfo.filename, pFilename, strlen((char const *)pFilename));
                currentStoragePut.ftInfo.transId = pFTInfo->transId;
                currentStoragePut.ftInfo.fileSz = pFTInfo->fileSz;
                currentStoragePut.ftInfo.fileCrc = pFTInfo->fileCrc;
                currentStoragePut.ftInfo.blockSize = pFTInfo->blockSize;
                currentStoragePut.pStorageHandle = pHandle;
                currentStoragePut.pFileHandle = pFileHandle;
                currentStoragePut.seqNo = INVALID_SEQUENCE;
                return SYS_ERR_NO_ERROR;
            }
            else
            {
                return SYS_ERR_EINVAL;
            }
        }
        else
        {
            return SYS_ERR_EFBIG;
        }
    }
    else
    {
        //error not supported
        return SYS_ERR_ENOTSUP;
    }
}

sys_error_code_t FileTransferGet(uint32_t transId, uint32_t fragSeqNum, uint8_t *pBuf, uint16_t *pFragSz, uint16_t *pFragCrc)
{
    sys_error_code_t ftErr = SYS_ERR_NO_ERROR;
    uint32_t offset = currentStorageGet.ftInfo.blockSize*fragSeqNum;
    
    if((currentStorageGet.ftInfo.transId != transId) && (transId != 0))
    {
        ftErr = SYS_ERR_EINVAL;
    }
    
    else
    {
#if( USE_FREERTOS_FAT == 1)
        int fileSize = currentStorageGet.pFileHandle->ulFileSize;
        int seek_set = FF_SEEK_SET;
#endif      
#if( USE_FATFS == 1)
        int fileSize = GetFatFsFileSize(currentStorageGet.pFileHandle);
        int seek_set = 0;
#endif

        if(offset >= fileSize)
        {
            ftErr = SYS_ERR_EINVAL;
        }
        else if(QLFS_Ffseek(currentStorageGet.pStorageHandle, currentStorageGet.pFileHandle, offset, seek_set) != 0 )
        {
            ftErr = SYS_ERR_EINVAL;
        }
        
    }
    
    if(ftErr == SYS_ERR_NO_ERROR) 
    {
        size_t bytesToRead;
#if( USE_FREERTOS_FAT == 1)        
        int fileSize = currentStorageGet.pFileHandle->ulFileSize;
#endif
#if( USE_FATFS == 1)        
        int fileSize = GetFatFsFileSize(currentStorageGet.pFileHandle);
#endif
        //get the bytes to read
        if((fileSize - offset) < currentStorageGet.ftInfo.blockSize)
            bytesToRead = (fileSize - offset);
        else
            bytesToRead = currentStorageGet.ftInfo.blockSize;
        
        
        //read file
        int bytes = QLFS_fread(currentStorageGet.pStorageHandle, currentStorageGet.pFileHandle, pBuf, 1, bytesToRead);
        
        if(bytes)
        {
            *pFragSz = bytes;
            // calc CRC
            *pFragCrc = crc16_ccitt((const unsigned char *)pBuf, bytes, 0);
            currentStorageGet.seqNo = fragSeqNum; 
        }
        else
        {
            ftErr = SYS_ERR_EINVAL;
        }
    }
    return ftErr; 
}

sys_error_code_t FileTransferPut(uint32_t transId, uint32_t fragSeqNum, uint8_t *pInBuff, uint32_t fragSz, uint16_t fragCrc)
{
    sys_error_code_t ftErr = SYS_ERR_NO_ERROR;
    uint32_t offset = currentStoragePut.ftInfo.blockSize*fragSeqNum;
    
    if((currentStoragePut.ftInfo.transId != transId) && (transId != 0))
    {
        ftErr = SYS_ERR_EINVAL;
    }
    else 
    {
#if( USE_FREERTOS_FAT == 1)         
      int seek_set = FF_SEEK_SET;
#endif
#if( USE_FATFS == 1)         
      int seek_set = 0;
#endif
        if(offset >= currentStoragePut.ftInfo.fileSz)
        {
            ftErr = SYS_ERR_EINVAL;
        }
        else if(QLFS_Ffseek(currentStoragePut.pStorageHandle, currentStoragePut.pFileHandle, offset, seek_set) != 0)
        {
            ftErr = SYS_ERR_EINVAL;
        }
    }
    
    if(ftErr == SYS_ERR_NO_ERROR) 
    {
        //verify CRC before writing
        
        uint16_t blockCrc = crc16_ccitt((const unsigned char *)pInBuff, fragSz, 0);
        
        if(blockCrc == fragCrc)
        {
            //write file
            int bytes = QLFS_fwrite(currentStoragePut.pStorageHandle, currentStoragePut.pFileHandle, pInBuff, 1, fragSz);
        }
        else
        {
            ftErr = SYS_ERR_EINVAL;
        }
    }
    return ftErr; 
}

sys_error_code_t FileTransferGetDone(uint32_t transId)
{
    if(currentStorageGet.ftInfo.transId == transId)
    {
        QLFS_fclose(QLFS_DEFAULT_FILESYTEM, currentStorageGet.pFileHandle);
        memset((void *)&currentStorageGet, 0, sizeof(currentStorageGet));   
        
        return SYS_ERR_NO_ERROR;
    }
    else
    {
        return SYS_ERR_EINVAL;
    }
}

sys_error_code_t FileTransferPutDone(uint32_t transId)
{
    if(currentStoragePut.ftInfo.transId == transId)
    {
        QLFS_fclose(QLFS_DEFAULT_FILESYTEM, currentStoragePut.pFileHandle);
        memset((void *)&currentStoragePut, 0, sizeof(currentStoragePut));
        
        return SYS_ERR_NO_ERROR;
    }
    else
    {
        return SYS_ERR_EINVAL;
    }
}

static void do_storage_dir(Mqttsn_IOMsgData_t *pIoMsgData)
{
    //nothing to add to Ack
    
    //Response
    sys_error_code_t fTErr;
    uint8_t fileName[FILENAME_LEN_MAX];
    uint32_t dateTime = 0;
    uint32_t fileSz = 0;
    uint16_t fileIndex = 0;
    static uint8_t pPath[30];
    
    if(pIoMsgData->incomingMsg.msgType == MQTTSN_PUBLISH)
    {
        //we've received the request
        dirLastFile = 0;
        strcpy((char *)pPath, (const char *)pIoMsgData->incomingMsg.pMsgPayldBuf);
    }
    else
    {
        //We've received an Ack, send the next file
        if(dirLastFile)
        {
             *pPath = '\0';   
            //we've sent the last file info with all 1s in the last PUBLISH
            //so nothing to do here, just return
            return;
        }
    }
    
    fTErr = GetNextFileInfo(pPath, 
                            (pIoMsgData->incomingMsg.msgType == MQTTSN_PUBLISH)?1:0,
                            &fileIndex, &fileSz, &dateTime, fileName);
    
    if(fileName[0] == '\0')  
    {
        //null terminated file name means we are at the end of file list
        dirLastFile = 1;
        fileIndex = 0xffff;
        fileSz = 0xffffffff;
        dateTime = 0xffffffff;
    }
    
    if(fTErr == SYS_ERR_NO_ERROR)
    {
        int result;
        Mqttsn_MsgData_t *pOutMsgData = &pIoMsgData->outgoingResponse;
        uint8_t *pPayld;
        result = Mqttsn_AllocPublishPayload(MQTTSN_BUFFER_LARGE, &pOutMsgData->pMsgPayldBuf,
                                      &pOutMsgData->allocLen);
        
        pPayld = pOutMsgData->pMsgPayldBuf;
        pOutMsgData->topicId = TOPIC_STORAGE_DIR_RSP;
        pOutMsgData->payldLen = 0;
        
        if(result)
        {
            pOutMsgData->payldLen += Mqttsn_BuffWr_u16(&pPayld, fileIndex);
            pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, fileSz);
            pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, dateTime);
            
            configASSERT(strlen((const char *)fileName) <= FILENAME_LEN_MAX);
            
            strcpy((char *)pPayld, (const char *)fileName);
            pOutMsgData->payldLen += strlen((const char *)fileName);
        }
    }
    else
    {
        PopulateRejectMsg(pIoMsgData, fTErr);
    }
}

static void do_storage_del(Mqttsn_IOMsgData_t *pIoMsgData)
{
    sys_error_code_t fTErr = Delete(pIoMsgData->incomingMsg.pMsgPayldBuf);
    
    if(fTErr != SYS_ERR_NO_ERROR)
    {
        PopulateRejectMsg(pIoMsgData, fTErr);
    }
}

static void do_storage_space(Mqttsn_IOMsgData_t *pIoMsgData)
{
    //nothing to add to Ack
    
    //Response
    int result = 0;
    Mqttsn_MsgData_t *pOutMsgData = &pIoMsgData->outgoingResponse;
    
    result = Mqttsn_AllocPublishPayload(MQTTSN_BUFFER_SMALL, &pOutMsgData->pMsgPayldBuf,
                                  &pOutMsgData->allocLen);
    
    if(result)
    {
        uint32_t totalKbs;
        uint32_t totalUsedKbs;
        uint32_t putSizeBytes;
        uint32_t getSizeBytes;
        uint8_t *pPayld = pOutMsgData->pMsgPayldBuf;
        pOutMsgData->payldLen = 0;
        
        sys_error_code_t fTErr =GetStorageSpace(pIoMsgData->incomingMsg.pMsgPayldBuf, 
                        &totalKbs, &totalUsedKbs, &putSizeBytes,
                        &getSizeBytes);
        
        if(fTErr == SYS_ERR_NO_ERROR)
        {
            memset(pPayld, 0, pOutMsgData->allocLen);
            
            pOutMsgData->topicId = TOPIC_STORAGE_SPACE_RSP;
            
            pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, totalKbs);
            pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, totalUsedKbs);
            pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, FILE_TRANSFER_PUT_SIZE_BYTES);
            pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, FILE_TRANSFER_GET_SIZE_BYTES);
        }
        else
        {
            //release the allocated buffer first
            Mqttsn_FreePublishPayload(&pOutMsgData->pMsgPayldBuf, pOutMsgData->allocLen);
            
            PopulateRejectMsg(pIoMsgData, fTErr);
        }
    }
}

static void do_storage_get_start(Mqttsn_IOMsgData_t *pIoMsgData)
{
    FileTransferInfo_t fTInfo;
    uint8_t *pInBuff = pIoMsgData->incomingMsg.pMsgPayldBuf;
    
    if(pInBuff)
    {
        sys_error_code_t ftErr;
        uint32_t fNameIdx = 0;
        
        fTInfo.transId = Mqttsn_BuffRead_u32(&pInBuff);
        fTInfo.blockSize = Mqttsn_BuffRead_u32(&pInBuff);
        
        while((*pInBuff != '\0') &&(fNameIdx<(FILENAME_LEN_MAX-1)))
        {
            fTInfo.filename[fNameIdx++] = *pInBuff++;
        }
        
        fTInfo.filename[fNameIdx++] = '\0';
        
        ftErr = FileTransferGetInit(&fTInfo);
        
        if(ftErr != SYS_ERR_NO_ERROR)
        {
            PopulateRejectMsg(pIoMsgData, ftErr);
        }
    }
}

#define GET_MESSAGE_DATA_OFFSET 12
static void do_storage_get(Mqttsn_IOMsgData_t *pIoMsgData)
{
    uint8_t *pInBuff = pIoMsgData->incomingMsg.pMsgPayldBuf;
    
    if(pInBuff)
    {
        sys_error_code_t ftErr;
        uint32_t transId;
        uint32_t blockNumber;
        uint16_t blockSize;
        uint16_t fragCrc;
        
        transId = Mqttsn_BuffRead_u32(&pInBuff);
        blockNumber = Mqttsn_BuffRead_u32(&pInBuff);
     
        //dont modify the ack
        
        //PUBLISH the fragment
        int result = 0;
        Mqttsn_MsgData_t *pOutMsgData = &pIoMsgData->outgoingResponse;
        uint8_t *pPayld;
        pOutMsgData->payldLen = 0;
        
        pOutMsgData->topicId = TOPIC_STORAGE_GET_DATA_RSP;
        result = Mqttsn_AllocPublishPayload(MQTTSN_BUFFER_LARGE, &pOutMsgData->pMsgPayldBuf,
                                            &pOutMsgData->allocLen);
        
        if(result)
        {
            
            ftErr = FileTransferGet(transId, blockNumber, 
                                    pOutMsgData->pMsgPayldBuf+GET_MESSAGE_DATA_OFFSET,
                                    &blockSize, &fragCrc);
            
            if(ftErr == SYS_ERR_NO_ERROR)
            {
                pPayld = pOutMsgData->pMsgPayldBuf;    
                pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, transId);
                pOutMsgData->payldLen += Mqttsn_BuffWr_u32(&pPayld, blockNumber);
                pOutMsgData->payldLen += Mqttsn_BuffWr_u16(&pPayld, blockSize);
                pOutMsgData->payldLen += Mqttsn_BuffWr_u16(&pPayld, fragCrc);
                
                pOutMsgData->payldLen += currentStorageGet.ftInfo.blockSize;
                
            }
            else   //there is an error
            {
                //release the allocated buffer first
                Mqttsn_FreePublishPayload(&pOutMsgData->pMsgPayldBuf, pOutMsgData->allocLen);
                
                PopulateRejectMsg(pIoMsgData, ftErr);
            }
        }
        else
        {
            printf("Error: do_storage_get - buffer could not be allocated\n");
        }
    }
}

static void do_storage_get_done(Mqttsn_IOMsgData_t *pIoMsgData)
{
    uint8_t *pInBuff = pIoMsgData->incomingMsg.pMsgPayldBuf;
    
    if(pInBuff)
    {
        uint32_t transId;
        
        transId = Mqttsn_BuffRead_u32(&pInBuff);
        
        sys_error_code_t ftErr = FileTransferGetDone(transId);
        
        if(ftErr != SYS_ERR_NO_ERROR)
        {
            PopulateRejectMsg(pIoMsgData, ftErr);
        }
    }    
}

static void do_storage_put_start(Mqttsn_IOMsgData_t *pIoMsgData)
{
    FileTransferInfo_t fTInfo;
    uint8_t *pInBuff = pIoMsgData->incomingMsg.pMsgPayldBuf;
    
    if(pInBuff)
    {
        sys_error_code_t ftErr;
        uint32_t fNameIdx = 0;
        
        fTInfo.transId = Mqttsn_BuffRead_u32(&pInBuff);
        fTInfo.fileSz = Mqttsn_BuffRead_u32(&pInBuff);
        fTInfo.blockSize = Mqttsn_BuffRead_u32(&pInBuff);
        fTInfo.fileCrc = Mqttsn_BuffRead_u16(&pInBuff);
        
        while((*pInBuff != '\0') &&(fNameIdx<(FILENAME_LEN_MAX-1)))
        {
            fTInfo.filename[fNameIdx++] = *pInBuff++;
        }
        
        fTInfo.filename[fNameIdx++] = '\0';
        
        ftErr = FileTransferPutInit(&fTInfo);
        
        if(ftErr != SYS_ERR_NO_ERROR)
        {
            PopulateRejectMsg(pIoMsgData, ftErr);
        }
    }
}

static void do_storage_put(Mqttsn_IOMsgData_t *pIoMsgData)
{
    uint8_t *pInBuff = pIoMsgData->incomingMsg.pMsgPayldBuf;
    
    if(pInBuff)
    {
        sys_error_code_t ftErr;
        uint32_t transId;
        uint32_t blockNumber;
        uint32_t blockSize;
        uint16_t blockCrc;
        
        transId = Mqttsn_BuffRead_u32(&pInBuff);
        blockNumber = Mqttsn_BuffRead_u32(&pInBuff);
        blockSize = Mqttsn_BuffRead_u16(&pInBuff);
        blockCrc = Mqttsn_BuffRead_u16(&pInBuff);
        
        ftErr = FileTransferPut(transId, blockNumber, pInBuff, blockSize, blockCrc);
        
        if(ftErr != SYS_ERR_NO_ERROR)
        {
            PopulateRejectMsg(pIoMsgData, ftErr);
        }
    }
}

static void do_storage_put_done(Mqttsn_IOMsgData_t *pIoMsgData)
{
    uint8_t *pInBuff = pIoMsgData->incomingMsg.pMsgPayldBuf;
    
    if(pInBuff)
    {
        uint32_t transId;
        
        transId = Mqttsn_BuffRead_u32(&pInBuff);
        
        sys_error_code_t ftErr = FileTransferPutDone(transId);
        
        if(ftErr != SYS_ERR_NO_ERROR)
        {
            PopulateRejectMsg(pIoMsgData, ftErr);
        }
    }
}

void Storage_AllStop(void)
{
    FileTransferGetDone(currentStorageGet.ftInfo.transId);
    FileTransferPutDone(currentStoragePut.ftInfo.transId);
}

#define FILENAME_MIN_LENGTH 3

//storage commands we support - in a table
static struct storage_cmd_dispatch_entry const scde_table[] = {
    { .value = TOPIC_STORAGE_DIR_REQ, do_storage_dir, .expPayldLength = FILENAME_MIN_LENGTH, .isVariableLen = 1 },
    { .value = TOPIC_STORAGE_DIR_RSP, do_storage_dir, .expPayldLength = 0, .isVariableLen = 0 },
    { .value = TOPIC_STORAGE_SPACE_REQ, do_storage_space, .expPayldLength = FILENAME_MIN_LENGTH, .isVariableLen = 1 },
    { .value = TOPIC_STORAGE_DEL, do_storage_del, .expPayldLength = FILENAME_MIN_LENGTH, .isVariableLen = 1 },
    { .value = TOPIC_STORAGE_GET_START, do_storage_get_start, .expPayldLength = 8+FILENAME_MIN_LENGTH, .isVariableLen = 1 },
    { .value = TOPIC_STORAGE_GET_DATA_REQ, do_storage_get, .expPayldLength = 8, .isVariableLen = 0 },
    { .value = TOPIC_STORAGE_GET_STOP, do_storage_get_done, .expPayldLength = 4, .isVariableLen = 0 },
    { .value = TOPIC_STORAGE_PUT_START, do_storage_put_start, .expPayldLength = 14+FILENAME_MIN_LENGTH, .isVariableLen = 1 },
    { .value = TOPIC_STORAGE_PUT_DATA_REQ, do_storage_put, .expPayldLength = 12+FILENAME_MIN_LENGTH, .isVariableLen = 1 },
    { .value = TOPIC_STORAGE_PUT_STOP, do_storage_put_done, .expPayldLength = 4, .isVariableLen = 0 },
    /* terminate */
    { .value = -1, .handler = NULL }
};


//process collection minor command
void run_storage_cmd(Mqttsn_IOMsgData_t *pIoMsgData )
{
    Mqttsn_MsgData_t *pInMsg = &pIoMsgData->incomingMsg;
    
    const struct storage_cmd_dispatch_entry *pSCDE;
    
    //FILE transfer is not supported if collection, livestream or 
    //classification is active
    if((iop_globals.cur_status.bit_flags & (COLLECT_ACTIVE | 
                                            LIVESTREAM_ACTIVE |
                                            IOP_STATUS_BIT_reco | 
                                            IOP_STATUS_BIT_reco_f)))
    {
         PopulateRejectMsg(pIoMsgData, SYS_ERR_ENOTSUP);
         return;
    }
    
    
    if((pInMsg->msgType == MQTTSN_PUBACK) &&
       (pInMsg->topicId != TOPIC_STORAGE_DIR_RSP))
    {
        return;
    }

    
    for( pSCDE = scde_table; pSCDE->handler != NULL ; pSCDE++ )
    {
        if( pSCDE->value == pInMsg->topicId )
        {
            uint32_t payldErr = CheckPayloadValidity(&pIoMsgData->incomingMsg, 
                                                     pSCDE->expPayldLength, 
                                                     pSCDE->isVariableLen);
            
            if(!payldErr)
            {            
                (*(pSCDE->handler))(pIoMsgData);
                return;
            }
            else
            {
                //there is a payload error, send an error message back
                PopulateRejectMsg(pIoMsgData, SYS_ERR_EINVAL);
                //configASSERT(0);
            }             
        }
    }
    /* unknown */
    dbg_str("err-storage-subcmd\n");
    set_sys_error( SYS_ERR_ENOTSUP, pInMsg->topicId);
}
