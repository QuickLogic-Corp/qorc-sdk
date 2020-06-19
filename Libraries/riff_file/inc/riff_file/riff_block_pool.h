#if !defined(RIFF_BLOCKPOOL_H)
#define RIFF_BLOCKPOOL_H

#if !defined(_IN_QL_RIFF_H_)
#error "include ql_riff.h instead"
#endif

/*
 * Return pointer to the common block pool.
 * Today we support a single block pool
 * but the code can support multiples.
 */
struct riff_block_pool *RIFF_block_pool_common(void);

/**
 * @brief - Initialize/Create a RIFF block pool
 *
 * @param pPool - memory for the pool header control blocks.
 * @param pBlockMemory - memory for the blocks.
 * @param nBytes - how many bytes big is the block memory=
 * @param chunkSize - how big should each block be? 
 */
struct riff_block_pool *RIFF_block_pool_init( struct riff_block_pool *pPool,
					      void *pBlockMemory,
					      size_t nBytes,
					      size_t chunkSize );


/** 
 * @brief Discards a RIFF block by not saving it.
 * 
 * @param pObject - object to discard.
 */
void RIFF_block_pool_discard( struct riff_object *pObject );


/**
 * @brief Allocate a block from the free pool 
 *
 * @param pPool pool to allocate from
 * @param tag value to initialize the RIFF tag with.
 * @param timeout_mSecs - 0 do not wait, RIFF_WAIT_FOREVER, or some number of milliseconds
 *
 * Returns NULL if no data is available.
 */
struct riff_object *RIFF_blockpool_allocate( struct riff_block_pool *pPool,
					 uint32_t tag, uint32_t timeout_mSecs );


 
/* take top item from the busy queue and write to the riff file. 
 * return EOF if nothing to do */
int RIFF_busy_queue_service( struct riff_block_pool *pPool, int *had_error );
#endif
