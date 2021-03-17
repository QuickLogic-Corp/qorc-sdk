#if !defined(QL_RIFF_FILE_H)
#define QL_RIFF_FILE_H

#if !defined(_IN_QL_RIFF_H_)
#error "include ql_riff.h" instead.
#endif


/**
 * @brief Open a queued write RIFF file we can save data to.
 * 
 * @param filename - preferably an 8.3 filename to save.
 * @param mode - string "w" for writing.
 *
 * @return NULL on failure.
 * Acts like fopen().
 */
struct riff_file *RIFF_file_open( const char *filename, const char *mode);

/**
 * @brief Writes this block to the specified RIFF data file via the queue
 *
 * @param pObject - the object to save
 * @param pFile - riff file pointer
 *
 * Note: Once 'saved' - the pObject is no longer valid.
 */
void RIFF_file_enqueue_object( struct riff_file *pFile, struct riff_object *pObject );

/**
 * @brief Writes the actual data to the file (used in the file task)
 *
 * @param pObject - the object to save
 *
 * used in the SD Card Task to actually write the data
 * Return 0 on success, -1 on error
 */
int RIFF_file_write_object( struct riff_object *pObject );

/**
 * @brief Close a riff queued write file.
 */
void RIFF_file_close( struct riff_file *pFile );

void riff_filename_format_init(char *basename, int ifilenum);
char *riff_get_newfilename(void);

#endif
