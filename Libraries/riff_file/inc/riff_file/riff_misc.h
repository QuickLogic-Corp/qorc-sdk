#if !defined(RIFF_MISC_H)
#define RIFF_MISC_H

struct riff_object;
struct riff_file;
struct riff_block_pool;

#if !defined(_IN_QL_RIFF_H_)
#error "include ql_riff.h" instead.
#endif


/**
 * @brief Force override the size of the object
 *
 * @param pObject - the object 
 * @param size - the new forced size.
 *
 * By default, the size of the object is the number of items written.
 */
void RIFF_override_size( struct riff_object *pObject, uint32_t size );


#define RIFF_WAIT_FOREVER 0xFFFFFFFF

/**
 * @brief Initialize the RIFF data save filesystem.
 */
void RIFF_init(void);


/* Return the RAW data pointer for a RIFF object 
 * The RAW pointer points to the first byte in the data block.
 */
void* RIFF_raw_data_ptr( struct riff_object *pObject );

#endif
