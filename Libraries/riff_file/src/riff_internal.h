#if !defined(RIFF_FILE_INTERNAL_H)
#define RIFF_FILE_INTERNAL_H

/* 
 * This header file is *NOT* part of the public api for the RIFF module.
 * That is why this file lives in the SRC directory.
 */
#include "FreeRTOS.h"
#include <queue.h>

#include "ql_fs.h"

/* this is only used internal to the riff file code
 * it is not a public api and should become public.
 */

#define RIFF_BLOCK_POOL_MAGIC (0x1072062b)
#define RIFF_OBJECT_MAGIC     (0xd111f164)
#define RIFF_FILE_MAGIC       (0xb1fb7056)

void riff_valid_busy_object(struct riff_object *pObject);
void riff_valid_free_object(struct riff_object *pObject);

/* this represents an open riff file */
struct riff_file {
  uint32_t magic;
  const QLFS_Handle *pFsHandle;
  QLFILE_Handle *pFile;
};

/* initialize different items */
void RIFF_file_init(void);
void RIFF_block_init(void);

/* this is a riff object
 *
 * basically - we start with an empty RIFF block
 * with no data in the block.
 *
 * As we add data, the "byte_wr_index" advances.
 * Eventually the block is full, or we otherwise
 * decide to write the file to the disk.
 */
struct riff_object {
  uint32_t magic;
  /* where it came from */
  struct riff_block_pool *pOwner;

  /* easier access then casting pointers all the time */
  union {
    uint32_t *pData32;
    uint32_t *pData16;
    uint8_t  *pData8;
    uint8_t  *pDataVp;
  } u;
  /* where are we writing in this block */
  uint32_t  byte_wr_index;

  /* are we *overriding the RIFF length field? If so this is non-zero */
  uint32_t  over_ride_length;

  /* when written what file will this goto? */
  struct riff_file *pFile;
};

/* this is a pool of RIFF blocks we manage */
struct riff_block_pool {
  uint32_t magic;

  /* where is the array of blocks */
  void     *block0;

  /* how big is each block */
  size_t    block_size;

  /* how many blocks are there */
  size_t    n_blocks;

  /* our queues holding the blocks */
  QueueHandle_t  free_queue;
  QueueHandle_t  busy_queue;

  /* our "open block" pointers */
  struct riff_object blocks[ RIFF_OBJECTS_MAX ];
};


/* internal check functions */
void riff_internal_error(void);
void riff_validate_object( const struct riff_object *p );
void riff_valid_file( struct riff_file *pFile );


#endif
