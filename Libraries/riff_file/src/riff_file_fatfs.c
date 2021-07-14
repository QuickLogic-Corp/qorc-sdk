#include "Fw_global_config.h"
#if (USE_FATFS_APIS == 1)
#include "ql_riff.h"
#include "riff_internal.h"
#include <string.h>
#include "dbg_uart.h"

/* list of all known RIFF files */
static struct riff_file all_riff_files[ RIFF_MAX_FILES ];

/* is this a valid RIFF file pointer? */
void riff_valid_file( struct riff_file *pFile )
{
    if( pFile->magic != RIFF_FILE_MAGIC ){
        dbg_fatal_error("riff-bad-magic");
    }
}


/* init the file component of the RIFF package */
void RIFF_file_init(void)
{
    memset( (void *)(all_riff_files), 0, sizeof(all_riff_files) );
}

/* 
* Close a RIFF file.
*/
void RIFF_file_close( struct riff_file *pFile )
{
    riff_valid_file(pFile);
    
    f_close( pFile->pFile );

    pFile->pFile = NULL;
	pFile->magic = 0;
}

/*
* Open a RIFF file for writing.
*/
struct riff_file *RIFF_file_open( const char *filename, const char *mode )
{
    FRESULT fresult;
    struct riff_file *pFile;
    int x;
    
    for( x = 0 ; x < RIFF_MAX_FILES ; x++ ){
        pFile = all_riff_files + x;
        if( pFile->magic == 0 ){
            break;
        }
    }
    
    if( x >= RIFF_MAX_FILES ){
        dbg_fatal_error("riff-toomany-files");
    }
    
    /* always delete the old file */
    if( DBG_flags & DBG_FLAG_datasave_debug ){
        dbg_str_str("riff-rm", filename );
    }
    f_unlink( filename );
	
    if( DBG_flags & DBG_FLAG_datasave_debug ){
        dbg_str_str("riff-open", filename );
    }
    pFile->pFile = &pFile->FileDescriptor;
    fresult = f_open( pFile->pFile, filename, FA_WRITE | FA_CREATE_ALWAYS );
    if( DBG_flags & DBG_FLAG_datasave_debug ){
        dbg_str_ptr("riff-handle", (void *)(pFile->pFile) );
    }
    if( fresult != FR_OK ){
      pFile = NULL;
    } else {
      pFile->magic = RIFF_FILE_MAGIC;
    }
    return pFile;
}

/*
* Enqueue this block into the write queue for later writing.
*/
void RIFF_file_enqueue_object(  struct riff_file *pFile, struct riff_object *pObject )
{
    int r;
    
    riff_valid_busy_object( pObject );
    riff_valid_file( pFile );
    pObject->pFile = pFile;
    
    r = xQueueSend( pObject->pOwner->busy_queue, (&pObject), 0 );
    
    if( r != pdPASS ){
        dbg_fatal_error("riff-busy-full?");
    }
}

/*
* This function performs the actual write to the file
* it is called from the sd card task.
*/
static size_t riffFileSizeSinceLastSync = 0;
int RIFF_file_write_object( struct riff_object *pObject )
{
    FRESULT fresult;
    riff_valid_busy_object(pObject);
    riff_valid_file( pObject->pFile );
    size_t actual;
    size_t nBytes;
    
    if( pObject->over_ride_length ){
        nBytes = pObject->over_ride_length;
    } else {
        /* RIFF length is value 1 */
        nBytes = pObject->byte_wr_index;
    }
#define RIFF_FILE_SYNC_SIZE  (1024*4*4)   // file sync every 16kB
#if ((RIFF_FILE_SIZE_MAX > 0) && (RIFF_AUTO_SEQUENCE_FILENAMES == 1))
    if (riffFileSizeSinceLastSync > RIFF_FILE_SYNC_SIZE)
    {
        //dbg_str_int_noln("riff sync size limit reached", riffFileSizeSinceLastSync);
        uint32_t sticks, eticks;
        sticks = xTaskGetTickCount();
        f_sync(pObject->pFile->pFile);
        eticks = xTaskGetTickCount();
        //dbg_str_int_noln("f_sync took", eticks-sticks);
        //dbg_str("ms \n");
        riffFileSizeSinceLastSync = 0;
    }

    size_t fileSize = f_size(pObject->pFile->pFile);
    if (fileSize > RIFF_FILE_SIZE_MAX)
    {
        dbg_str_int("riff file size limit reached", fileSize);
        uint32_t sticks, eticks;
        sticks = xTaskGetTickCount();
        f_close(pObject->pFile->pFile);

        char *nextfilename = riff_get_newfilename();
        fresult = f_open(pObject->pFile->pFile, nextfilename, FA_WRITE | FA_CREATE_ALWAYS );
        if (fresult == FR_OK)
        {
            dbg_str_str("datafile", nextfilename);
        }
        eticks = xTaskGetTickCount();
        dbg_str_int("f_close + f_open took", eticks-sticks);
    }
#endif
    /* update the actual bytes in the riff header */
    pObject->u.pData32[1] = nBytes;
    
    /* add the header size 8 */
    /* align to 32bit boundary*/
    
    nBytes = (nBytes + 8 + 3) & ~0x03;
    actual = 0;
    if( DBG_flags & DBG_FLAG_datasave_debug ){
        dbg_str("riff-wr-START\n");
    }
    fresult = f_write(pObject->pFile->pFile,
                     pObject->u.pDataVp,
                     nBytes,
                     &actual );
    if( DBG_flags & DBG_FLAG_datasave_debug ){
        dbg_str("riff-wr-DONE\n");
    }
    if( actual == nBytes ){
		/* All is well */
        riffFileSizeSinceLastSync += actual;
        return 0;
	} else {
		return -1;
	}    
}

#if (RIFF_AUTO_SEQUENCE_FILENAMES == 1)
static int riff_file_count = 0;
static char riff_file_basename[64]= "data_";
static char riff_file_nextname[64];

/** @brief RIFF fileformat initialization function using the
 *  user provided basename and the initial file number integer
 *  New riff filenames will be constructed in the format
 *  basenameIIIIIIII.qlsm where IIIIIIII is an integer
 *
 *  @param[in] basename pointer a char array not exceeding 50 bytes
 *  @param[in] ifilenum initial integer to be used in filename format
 */
void riff_filename_format_init(char *basename, int ifilenum)
{
	strncpy(riff_file_basename, basename, sizeof(riff_file_basename));
	riff_file_count = ifilenum;
}

/** @brief Get a new RIFF filename for storing the sensor data
 *  A new filename of the format
 *  @param[out] newfilename pointer a char array of atleast 64 bytes
 */
char *riff_get_newfilename(void)
{
	riff_file_count++;
	snprintf(riff_file_nextname, sizeof(riff_file_nextname), "%s%08d.qlsm", riff_file_basename, riff_file_count);
	return riff_file_nextname;
}

size_t RIFF_get_filesize(void)
{

}
#endif /* RIFF_AUTO_SEQUENCE_FILENAMES */
#endif /* USE_FATFS_APIS */
