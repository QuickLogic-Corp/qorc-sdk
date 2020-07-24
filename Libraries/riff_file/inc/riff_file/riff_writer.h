#if !defined(RIFF_WRITER_H)
#define RIFF_WRITER_H

/*
 * These are functions that WRITE data to the RIFF block.
 */

#if !defined(_IN_QL_RIFF_H_)
#error "include ql_riff.h" instead.
#endif

/* used to help indent JSON text */
extern int riff_json_indent_depth;

/**
 * Indent json text based upon the ident depth 
 */
int RIFF_write_ident( struct riff_object *pObject );

/**
 * @brief return space(in bytes) available in this riff block
 * 
 * @param pObject - the riff object
 * @param n    - how many bytes are available..
 *
 */
size_t RIFF_space_avail( struct riff_object *pObject );

/**
 * @brief Get a pointer into the buffer to copy data via dma like operations.
 *
 * @param pObject - the object in question
 * @param ppBuf - where to put the write pointer
 * @param size  - how many bytes are available at this pointer position.
 *
 */
void RIFF_dma_wr_get( struct riff_object *pObject, void **pBuf, size_t *size );

/**RIFF_dma_wr_get
 * @brief Completes a RIFF write operation via DMA by supplying the actual byte count.
 *
 * @param pObject - the riff object in question.
 * @param actual - the actual used number of bytes
 */
void RIFF_dma_wr_complete( struct riff_object *pObject, size_t actual );


/**
 * @brief sprintf() core function to write to the riff block.
 *
 * @param pObject - the riff object being writen to
 * @param fmt - printf() format string.
 * @param ap  - va_arg for the vprintf() purposes.
 */
int RIFF_vsprintf( struct riff_object *pObject, const char *fmt, va_list ap );


/**
 * @brief sprintf() to the riff block.
 *
 * @param pObject - the riff object being writen to
 * @param fmt - printf() format string.
 */
int RIFF_sprintf( struct riff_object *pObject, const char *fmt, ... );

/*
 * Start a group, with either { or [
 */
int RIFF_add_json_group_open( struct riff_object *pObject, int ch );

/*
 * close a group, with either } or ], with comma/newline or not
 */
int RIFF_add_json_group_close( struct riff_object *pObject, int ch, int needcomma );

/* open a named group with either { or [ */
int RIFF_add_json_tag_group_open( struct riff_object *pObject, const char *name, int ch );

/* output a named item non-quoted value with or without a comma */
int RIFF_add_json_tag_plain( struct riff_object *pObject, const char *name, int iscomma, const char *value_fmt, ... );

/* output a named item with quoted value with or without a comma */
int RIFF_add_json_tag_quoted( struct riff_object *pObject, const char *name, int iscomma, const char *value_fmt, ... );

/** write a string with quotes, optional comma at end */
int RIFF_write_quoted_string( struct riff_object *pObject, const char *name, int iscomma );

/** 
 * @brief write a null terminated string to the object.
 *
 * @param pObject - the object to write
 * @param s       - pointer to string
 */
int RIFF_write_str( struct riff_object *pObject, const char *s );

/**
 * @brief Write "generic" to the RIFF block.
 *
 * @param pObject - the object to write
 * @param pBuf - pointer to buffer of bytes
 * @param nBytes - number of bytes to write.
 * @param is_string - if true, force null terminate
 */
int RIFF_write_vp( struct riff_object *pObject, const void *pBuf, size_t nBytes, int is_string );

/**
 * @brief Write bytes to the RIFF block.
 *
 * @param pObject - the object to write
 * @param pBuf - pointer to buffer of bytes
 * @param nValues - number of values (u8) to write.
 */
int RIFF_write_u8( struct riff_object *pObject, const uint8_t *pBuf, size_t nValues );

/**
 * @brief Write bytes to the RIFF block.
 *
 * @param pObject - the object to write
 * @param pBuf - pointer to buffer of bytes
 * @param nValues - number of values (u16) to write.
 */
int RIFF_write_u16( struct riff_object *pObject, const uint16_t *pBuf, size_t nValues );

/**
 * @brief Write bytes to the RIFF block.
 *
 * @param pObject - the object to write
 * @param pBuf - pointer to buffer of bytes
 * @param nValues - number of values (u32) to write.
 */
int RIFF_write_u32( struct riff_object *pObject, const uint32_t *pBuf, size_t nValues );

/**
 * @brief Write bytes to the RIFF block.
 *
 * @param pObject - the object to write
 * @param pBuf - pointer to buffer of bytes
 * @param nValues - number of values (u64) to write.
 */
int RIFF_write_u64( struct riff_object *pObject, const uint64_t *pBuf, size_t nValues );


/**
 * @brief Write single U8 to the object.
 *
 * @param pObject - the riff object
 * @para  value   -  value to write
 */
int RIFF_write_one_u8( struct riff_object *pObject, uint8_t value );

/**
 * @brief Write single U8 to the object.
 *
 * @param pObject - the riff object
 * @para  value   -  value to write
 */
int RIFF_write_one_u16( struct riff_object *pObject, uint16_t value );

/**
 * @brief Write single U32 to the object.
 *
 * @param pObject - the riff object
 * @para  value   -  value to write
 */
int RIFF_write_one_u32( struct riff_object *pObject, uint32_t value );
/**
 * @brief Write single U64 to the object.
 *
 * @param pObject - the riff object
 * @para  value   -  value to write
 */
int RIFF_write_one_u64( struct riff_object *pObject, uint64_t value );

#endif /*   !defined(RIFF_WRITER_H) */
