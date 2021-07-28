#include "Fw_global_config.h"
#if (USE_FATFS_APIS == 1)
#include "ql_riff.h"
#include "riff_internal.h"
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "dbg_uart.h"

#if (RIFF_AUTO_SEQUENCE_FILENAMES == 1)
void riff_delete_oldest_qlsm(void);
#endif

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
        //dbg_str_int("riff file size limit reached", fileSize);
        uint32_t sticks, eticks;
        sticks = xTaskGetTickCount();
        f_close(pObject->pFile->pFile);

        char *nextfilename = riff_get_newfilename();
        fresult = f_open(pObject->pFile->pFile, nextfilename, FA_WRITE | FA_CREATE_ALWAYS );
        if (fresult == FR_OK)
        {
            dbg_str_str("datafile", nextfilename);
        }
        // Now delete the oldest file
        riff_delete_oldest_qlsm();
        eticks = xTaskGetTickCount();
        //dbg_str_int("f_close + f_open took", eticks-sticks);
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
#include <stdbool.h>
#define RIFF_FILE_COUNT_MAX  (10)
#define RIFF_FILE_BASENAME   ("data_")
/* Hold the file numbers (in sorted order) that are present on the card
 * (data_<file number>.qlsm). A 0 indicates there is no file with that number
 */
static int riff_file_num_buff[RIFF_FILE_COUNT_MAX] = {0};
static int sfilenumHead = -1; // new/next file number to be created
static int sfilenumTail = -1; // oldest file number to be deleted

static bool riff_file_num_buff_init_done = false;
static int riff_file_count = 0;
static char riff_file_basename[64]= RIFF_FILE_BASENAME;
static char riff_file_nextname[64];
static char riff_file_oldname[64];
/* File number management */
void riff_filenum_print_buf(void)
{
    char tmpb[16];
    dbg_str("File numbers: ");
    for (int k = 0; k < RIFF_FILE_COUNT_MAX; k++)
    {
        dbg_str(itoa(riff_file_num_buff[k], tmpb, 10));
        dbg_str(", ");
    }
    dbg_str_int_noln("Head", sfilenumHead);
    dbg_str_int_noln(", Tail", sfilenumTail);
    dbg_str("\n");
}

/** @brief Get head and tail of file numbers
 *
 * riff_file_num_buff[] contains the file numbers currently on the
 * SD card. Scan for a gap to identify the new filename to be created
 * and the oldest file that needs to be deleted.
 *
 * If scan identifies following file numbers
 * 001, 002, 003, 004, 005,           008, 009, 010
 * then delete 008 and create 006 as in the below
 * 001, 002, 003, 004, 005, 006,           009, 010
 *
 */
int riff_filenum_get_head_tail(int *pfilenumTail)
{
    int filenumHead, filenumTail;
    filenumHead = -1;
    filenumTail = -1;

    for (int k = 1; k <= RIFF_FILE_COUNT_MAX; k++)
    {
        if (riff_file_num_buff[k-1] == 0)
        {  // first break
            if (riff_file_num_buff[(k%RIFF_FILE_COUNT_MAX)] == 0)
            {
                // we have a break
                filenumHead = k;   // new file name will be k
                filenumTail = (k+1)%(RIFF_FILE_COUNT_MAX) + 1; // old filename to delete
            }
            else if (k==1)
            {
                 filenumHead = RIFF_FILE_COUNT_MAX;
                 filenumTail = 2;
            }
            else
            {   // there is an error, force a break
                filenumHead = k;   // new file name will be k
                // delete the file (k+1)
                sfilenumTail = k%(RIFF_FILE_COUNT_MAX) + 1; // delete the extra file
                riff_delete_oldest_qlsm();
                filenumTail = (k+1)%(RIFF_FILE_COUNT_MAX) + 1; // old filename to delete
            }
            break;
        }
    }

    *pfilenumTail = filenumTail;
    return filenumHead;
}

/** @brief Search a directory for to identify new file number to be created and
 * oldest *.qlsm file to be deleted. The new file number is returned
 * and the oldest file number is stored at the address `pfilenumTail`
 *
 * @param pdir directory to be searched
 * @param pfilenumTail address to store the oldest file number to be deleted
 *
 * @return new file number to be created
 */
int riff_file_scan_qlsm_files (TCHAR *pdir, int *pfilenumTail)
{
    FRESULT fr;     /* Return value */
    DIR dj;         /* Directory object */
    FILINFO fno;    /* File information */
    FILINFO oldest_fno;
    int ifilenum, tot_count, nitems;
    int filenumHead, filenumTail, filenumOffset;

    if (riff_file_num_buff_init_done == false)
    {
        for (int k = 0; k < RIFF_FILE_COUNT_MAX; k++)
            riff_file_num_buff[k] = 0;
        tot_count = 0;
        fr = f_findfirst(&dj, &fno, pdir, "?*.qlsm");   /* Start to search for QLSM files */

        while (fr == FR_OK && fno.fname[0]) {           /* Repeat while an item is found */
            ifilenum = 0;
            nitems = 0;
            filenumOffset = strlen(riff_file_basename);
            if (strncmp (fno.fname, riff_file_basename, filenumOffset) == 0)
            {
                nitems = sscanf(fno.fname+filenumOffset, "%d.qlsm", &ifilenum); /* scan for the file number */
            }
            if (nitems <= 0)
                continue;
            if ((ifilenum > 0) && (ifilenum <= RIFF_FILE_COUNT_MAX)) {
                tot_count++;
                riff_file_num_buff[ifilenum-1] = ifilenum;
            }
            fr = f_findnext(&dj, &fno);               /* Search for next item */
        }

        f_closedir(&dj);
    }

    filenumHead = riff_filenum_get_head_tail(&filenumTail);
    *pfilenumTail = filenumTail;
    return filenumHead;
}

void riff_filenum_init(void)
{
    sfilenumHead = riff_file_scan_qlsm_files("", &sfilenumTail);
    return;
}

/** @brief Delete oldest file identified by the file number in sfilenumTail
 *  static variable
 */
void riff_delete_oldest_qlsm(void)
{
    int filenum;

    filenum = sfilenumTail;
    snprintf(riff_file_oldname, sizeof(riff_file_oldname), "%s%08d.qlsm", riff_file_basename, filenum);
    dbg_str_str_nonl("Deleting", riff_file_oldname);
    if (f_unlink(riff_file_oldname) == FR_OK)
    {
        dbg_str(" Successful\n");
    }
    else
    {
        dbg_str(" Failed\n");
    }
    riff_file_num_buff[filenum-1] = 0;

}

/** @brief RIFF file format initialization function using the
 *  user provided basename and the initial file number integer
 *  New riff filenames will be constructed in the format
 *  basenameIIIIIIII.qlsm where IIIIIIII is an integer
 *
 *  @param[in] basename pointer a char array not exceeding 50 bytes
 *  @param[in] ifilenum initial integer to be used in filename format
 */
void riff_filename_format_init(char *basename, int ifilenum)
{
    extern int riff_file_scan_qlsm_files (TCHAR *pdir, int *pfilenumTail);
    if (riff_file_num_buff_init_done == true)
        return;

    strncpy(riff_file_basename, basename, sizeof(riff_file_basename));
    riff_filenum_init();
    riff_file_num_buff_init_done = true;
}

/** @brief Get a new RIFF filename for storing the sensor data
 *  A new filename of the format: <basename><NNNNNNNN>.qlsm where
 *  <basename> is the file basename specified in riff_file_basename
 *  and <NNNNNNNN> is the new file number
 *
 *  @return newfilename pointer to a char array of atleast 64 bytes
 *
 */
char *riff_get_newfilename(void)
{
    extern int riff_file_scan_qlsm_files (TCHAR *pdir, int *pfilenumTail);

    int k, filenumHead, filenumTail;
    if (riff_file_num_buff_init_done == false)
        riff_filename_format_init(riff_file_basename, 0);

    sfilenumHead = riff_file_scan_qlsm_files("", &sfilenumTail);
    riff_filenum_print_buf();
    riff_file_count = sfilenumHead;
    snprintf(riff_file_nextname, sizeof(riff_file_nextname), "%s%08d.qlsm", riff_file_basename, riff_file_count);
    dbg_str_str_nonl("Next filename", riff_file_nextname);
    riff_file_num_buff[riff_file_count-1] = riff_file_count;
    dbg_str("\n");

    return riff_file_nextname;
}

size_t RIFF_get_filesize(void)
{

}


#endif /* RIFF_AUTO_SEQUENCE_FILENAMES */
#endif /* USE_FATFS_APIS */
