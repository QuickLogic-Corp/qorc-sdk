#include "Fw_global_config.h"

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
    
    QLFS_fclose( pFile->pFsHandle, pFile->pFile );
    pFile->pFsHandle = NULL;
    pFile->pFile = NULL;
	pFile->magic = 0;
}

/*
* Open a RIFF file for writing.
*/
struct riff_file *RIFF_file_open( const char *filename, const char *mode )
{
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
    
    pFile->pFsHandle = QLFS_DEFAULT_FILESYTEM;
    
    /* always delete the old file */
    if( DBG_flags & DBG_FLAG_datasave_debug ){
        dbg_str_str("riff-rm", filename );
    }
    QLFS_RmFile( pFile->pFsHandle, filename );
	
    if( DBG_flags & DBG_FLAG_datasave_debug ){
        dbg_str_str("riff-open", filename );
    }

    pFile->pFile = QLFS_fopen( pFile->pFsHandle, filename, "wb" );
    if( DBG_flags & DBG_FLAG_datasave_debug ){
        dbg_str_ptr("riff-handle", (void *)(pFile->pFile) );
    }
    if( pFile->pFile == NULL ){
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
int RIFF_file_write_object( struct riff_object *pObject )
{
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
    /* update the actual bytes in the riff header */
    pObject->u.pData32[1] = nBytes;
    
    /* add the header size 8 */
    /* align to 32bit boundary*/
    
    nBytes = (nBytes + 8 + 3) & ~0x03;

    if( DBG_flags & DBG_FLAG_datasave_debug ){
        dbg_str("riff-wr-START\n");
    }
	actual = QLFS_fwrite( pObject->pFile->pFsHandle, 
                          pObject->pFile->pFile, 
                          pObject->u.pDataVp,
                          1, 
                          nBytes );
    if( DBG_flags & DBG_FLAG_datasave_debug ){
        dbg_str("riff-wr-DONE\n");
    }
    if( actual == nBytes ){
		/* All is well */
        return 0;
	} else {
		return -1;
	}    
}

