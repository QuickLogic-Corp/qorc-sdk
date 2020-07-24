#if !defined(QL_RIFF_H)
#define QL_RIFF_H

#if !defined( _EnD_Of_Fw_global_config_h )
#error "Include Fw_global_config.h first"
#endif

#include <stddef.h> /* size_t etc */
#include <stdint.h>
#include <stdarg.h>

/* forward declorations */
struct riff_object;
struct riff_file;
struct riff_block_pool;

#define _IN_QL_RIFF_H_ 1
#define RIFF_OBJECTS_MAX 64
#define RIFF_MAX_FILES   8

/* init, setup etc */
#include "riff_file/riff_misc.h"

/* Writing data to the RIFF data block */
#include "riff_file/riff_writer.h"

/* File activity */
#include "riff_file/riff_file.h"

/* Block Pool memory management */
#include "riff_file/riff_block_pool.h"

/* convert these 4bytes into a 32bit RIFF value */
#define MAKE_RIFF_ID( D, C, B, A )\
    ( ( (A) << 24 ) | ( (B) << 16 ) | ( (C) << 8 ) | ( (D) << 0 ) )
/* File write queue for riff */



#undef _IN_QL_RIFF_H_

#endif
