#include "Fw_global_config.h"
#include "stdio.h"
#include "ql_riff.h"
#include "riff_internal.h"
#include "string.h"
#include "dbg_uart.h"

static struct riff_block_pool da_common_block_pool;

/* validate a block pool pointer with its magic number */
void riff_valid_block_pool( struct riff_block_pool *pPool )
{
    if( pPool->magic != RIFF_BLOCK_POOL_MAGIC ){
        dbg_fatal_error("riff-magic-err1");
    }
}

/* validate this pointer is a busy riff object */
void riff_valid_busy_object( struct riff_object *pObject )
{
    if( pObject->magic != RIFF_OBJECT_MAGIC ){
        dbg_fatal_error("riff-magic-err2");
    }
}

/* validate this pointer is a free riff object */
void riff_valid_free_object( struct riff_object *pObject )
{
    if( pObject->magic != (~RIFF_OBJECT_MAGIC)){
        dbg_fatal_error("riff-magic-err3");
    }
}

/* return pointer to the single common RIFF pool
 * Future, we might have multiple RIFF pools.
 */
struct riff_block_pool *RIFF_block_pool_common(void)
{
    return &da_common_block_pool;
}


/* initialize the RIFF block pool */
struct riff_block_pool *
RIFF_block_pool_init( struct riff_block_pool *raw_pool,
                     void *pBlockMemory,
                     size_t nBytes,
                     size_t blockSize )
{
    int x;
    struct riff_block_pool *pResult;
    pResult = raw_pool;
    
    memset( (void *)(pResult), 0, sizeof(pResult) );
    pResult->magic = RIFF_BLOCK_POOL_MAGIC;
    
    /* some reasonable size */
    if( nBytes < 8192 ){
        /* die */
        dbg_fatal_error("riff-size-small");
    }
    
    /* chunk must be power of 2 */
    if( (blockSize != 0) && ((blockSize-1) & blockSize) ){
        /* die */
        dbg_fatal_error("riff: bad blk size");
    }
    
    pResult->n_blocks = nBytes / blockSize;
    if( pResult->n_blocks >  RIFF_OBJECTS_MAX ){
        dbg_fatal_error("riff: too-many-blocks");
    }
    
    pResult->block_size = blockSize;
    pResult->block0     = pBlockMemory;
    pResult->free_queue = xQueueCreate( pResult->n_blocks,
                                       sizeof( void * ) );
    vQueueAddToRegistry( pResult->free_queue,"Riff_Free");
    pResult->busy_queue = xQueueCreate( pResult->n_blocks,
                                       sizeof( void * ) );
    vQueueAddToRegistry( pResult->busy_queue,"Riff_Busy");
    
    for( x = 0 ; x < pResult->n_blocks ; x++ ){
        struct riff_object *pObject;
        
        pObject = &(pResult->blocks[ x ]);
        char *cp;

	/* for now pretend they are busy... */
        pObject->magic            = RIFF_OBJECT_MAGIC;
        pObject->pOwner           = pResult;
        cp = pResult->block0;
        cp = cp + (x * pResult->block_size);
        pObject->u.pDataVp        = ((void *)(cp));
        pObject->byte_wr_index    = 0;
        pObject->over_ride_length = 0;

	/* insert FREE blocks by discarding them */
        RIFF_block_pool_discard( pObject );
    }
    return pResult;
}


/* put this object back into its owning pool */
void RIFF_block_pool_discard( struct riff_object *pObject )
{
    int r;
    
    riff_valid_busy_object(pObject);
    
    /* mark as not busy */
    pObject->magic ^= 0xffffffff;
    r = xQueueSend( pObject->pOwner->free_queue, (void *)(&pObject), 0 );
    if( r != pdPASS ){
        dbg_fatal_error("riff-discard-error");
    }
}


/* Allocate a new object from the specified block pool */
struct riff_object *
RIFF_blockpool_allocate( struct riff_block_pool *pPool,
                        uint32_t tag,
                        uint32_t timeout_mSecs )
{
    int r;
    struct riff_object *pResult;
    
    riff_valid_block_pool( pPool );
    
    r = xQueueReceive( pPool->free_queue, (void *)(&(pResult)), timeout_mSecs );
    if( r != pdPASS ){
        dbg_str("ERROR - Drop Data\n");
        return NULL;
    }
    
    /* mark as busy */
    pResult->magic ^= 0xffffffff;
    pResult->byte_wr_index = 0;
    pResult->over_ride_length = 0;
    /* for debug reasons */
    memset( pResult->u.pDataVp, 0, pPool->block_size );
    
    /* write element 0, the tag */
    RIFF_write_one_u32( pResult, tag );
    /* and the length */
    RIFF_write_one_u32( pResult, 0   );
    return pResult;
}


/* pull top item off of the busy queue 
 * If queue is empty return EOF
 *
 * Otherwise write that item to the file
 * And release that item into the free queue.
 */
int RIFF_busy_queue_service( struct riff_block_pool *pPool, int *had_error )
{
    int r;
    struct riff_object *pObject;
    
    *had_error = 0;
    
    pObject = NULL;

    /* Take one off the front... */
    r = xQueueReceive( pPool->busy_queue, (&pObject), 0 );
    if( r==0 ){
        /* nothing to remove we are done at end of pseudo-file */
        return EOF;
    }
    if( DBG_flags & DBG_FLAG_datasave_debug ){
        dbg_str_ptr( "data-save-ptr", (void *)(pObject) );
    }
    //dbg_ch('W');
    /* Write the object to the file */
    r = RIFF_file_write_object( pObject );
    //dbg_ch('w'); dbg_nl();
    if( r != 0 ){
        /* this is *NOT* fatal */
        dbg_str("wr-err\n");
        *had_error = 1;
    }

    /* free the block */
    RIFF_block_pool_discard( pObject );

    /* not an EOF we did something 
     * Caller might call us again, or decide to do that later.
     */
    return 0;
}









