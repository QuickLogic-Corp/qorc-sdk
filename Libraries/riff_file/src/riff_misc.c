#include "Fw_global_config.h"

#include "ql_riff.h"
#include "riff_internal.h"

/* initialize */
void RIFF_init(void)
{
    /* nothing yet */
    RIFF_file_init();
    RIFF_block_init();
}

/* we want to overide the length element with this value */
void RIFF_override_size( struct riff_object *pObject, uint32_t size )
{
    riff_valid_busy_object(pObject);
    
    pObject->over_ride_length = size;
}



/* get pointer to the raw data in the riff block */
void* RIFF_raw_data_ptr( struct riff_object *pObject )
{
    riff_valid_busy_object( pObject );
    
    return (void *)(pObject->u.pDataVp);
}


/* return number of bytes available in the riff block */
size_t RIFF_space_avail( struct riff_object *pObj )
{
    size_t size;
    RIFF_dma_wr_get( pObj, NULL, &size );
    
    return size;
}

/* Get location (and length) where we can insert data into the RIFF block. */
void RIFF_dma_wr_get( struct riff_object *pObj, void **vpp, size_t *pSize)
{
    int r;
    if( vpp ){
        *vpp = &(pObj->u.pData8[ pObj->byte_wr_index]);
    }
    
    r = pObj->pOwner->block_size;
    r = r - pObj->byte_wr_index;
    if( r < 0 ){
        r = 0;
    }
    if( pSize ){
        *pSize = r;
    }
}

/* That external code has completed writing data into the RIFF block
 * we need to update the write index with the actual amount written.
 */
void RIFF_dma_wr_complete( struct riff_object *pObj, size_t nActual )
{
    riff_valid_busy_object( pObj );
    pObj->byte_wr_index += nActual;
    if( pObj->byte_wr_index > pObj->pOwner->block_size ){
        pObj->byte_wr_index = pObj->pOwner->block_size;
    }
}
