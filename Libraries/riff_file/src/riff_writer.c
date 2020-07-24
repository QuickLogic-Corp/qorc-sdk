#include "Fw_global_config.h"

#include "stdio.h"
#include "ql_riff.h"
#include "riff_internal.h"
#include "string.h"
#include "dbg_uart.h"

int riff_json_indent_depth;
static int need_indent;

/* write N bytes into buffer at current write index */
int RIFF_write_vp( struct riff_object *pObj, const void *vpRd, size_t nbytes, int is_str )
{
    void *vpWr;
    size_t space;

    /* always copy the null byte for a string */
    if( is_str ){
        nbytes += 1;
    }
    RIFF_dma_wr_get( pObj, &vpWr, &space );
    if( space < nbytes ){
        /* non fatal */
        dbg_str("riff-vp-overflow\n");
        return EOF;
    }
    memcpy( vpWr, vpRd, nbytes );
    
    /* but don't include the null when we complete */
    if( is_str ){
        nbytes -= 1;
    }
    RIFF_dma_wr_complete( pObj, nbytes );
    return 0;
}

/* common strings used in JSON data */
static const char s_comma[] = ",";
static const char s_quote[] = "\"";
static const char s_quote_comma[] = "\",\n";
static const char s_nl[]= "\n";

int RIFF_write_indent( struct riff_object *pObject )
{
    int x;
    int r;
    r = 0;
    if( !need_indent ){
        return 0;
    }
    need_indent = 0;
    for( x = 0 ; x < riff_json_indent_depth ; x++ ){
        /* indent json text by 4 */
        r |= RIFF_write_str( pObject, "    ");
    }
    return r;
}

/* insert a new line */
int RIFF_write_newline(struct riff_object *pObject)
{
    need_indent = 1;
    return RIFF_write_vp( pObject, (const void *)(s_nl), 1, 1 );
}

/* open a json group, ie with {} or [] */
int RIFF_add_json_group_open( struct riff_object *pObject, int ch)
{
    int r;
    char buf[3];
    buf[0] = ch;
    buf[1] = '\n';
    buf[2] = 0;

    r = RIFF_write_indent(pObject);
    r |= RIFF_write_str( pObject, buf );
    need_indent = 1;
    return r;
}

/* open a named group in the JSON data */
int RIFF_add_json_tag_group_open(struct riff_object *pObject, const char *name, int ch)
{
    int r;
    r = RIFF_write_indent(pObject);
    r |= RIFF_write_quoted_string( pObject, name,0 );
    r |= RIFF_write_str( pObject, " : " );
    r |= RIFF_add_json_group_open( pObject, ch );
    return r;
}

/* close the group, and if needed add a comma at the end of this group */
int RIFF_add_json_group_close(struct riff_object *pObject, int ch, int need_comma )
{
    char buf[4];
    
    buf[0] = ch;
    if( need_comma ){
        buf[1] = ',';
        buf[2] = '\n';
        buf[3] = 0;
    } else {
        buf[1] = '\n';
        buf[2] = 0;
    }
    need_indent = 1;
    return RIFF_write_str( pObject, buf);
}

/* Write a quoted string to the JSON */
int RIFF_write_quoted_string( struct riff_object *pObject, const char *name, int need_comma )
{
    int r;
    RIFF_write_indent(pObject);
    r = 0;
    r |= RIFF_write_str( pObject, s_quote );
    r |= RIFF_write_str( pObject, name );
    r |= RIFF_write_str( pObject, need_comma ? s_quote_comma : s_quote);
    if( need_comma ){
        need_indent=1;
    }
    return r;
}

/* work horse for sprintf() into the riff data block */
int RIFF_vsprintf( struct riff_object *pObject, const char *fmt, va_list ap )
{
    int r;
    size_t size;
    int actual;
    void *wp;

    RIFF_dma_wr_get( pObject, &wp, &size );
    /* leave room for 1 byte + null */
    if( size < 2 ){
        /* no room */
        return -1;
    }
    size--;
    
    /* do the work
     * vsnprintf() returns number that *would have been* be written
     */
    /* FIXME: only need to support simple things get rid of sprintf()? */
    actual = vsnprintf( (char *)(wp), size, fmt, ap );
    if( actual < size ){
        r = 0;
    } else {
        actual = size;
        r = -1;
    }
    /* force null term */
    ((char *)(wp))[ actual ] = 0;
    RIFF_dma_wr_complete( pObject, actual );
    return r;
}

/* printf() into the riff block */
int RIFF_sprintf( struct riff_object *pObject, const char *fmt, ... )
{
    int r;
    va_list ap;
    va_start( ap, fmt );
    r = RIFF_vsprintf( pObject, fmt, ap );
    va_end(ap);
    return r;
}

/* insert a tag and a quoted or non-quoted value */
static int _do_tag( struct riff_object *pObject, const char *name, int isquote, int iscomma, const char *fmt_value, va_list ap )
{
    int r;
    r = RIFF_write_indent(pObject);
    r |= RIFF_write_quoted_string(pObject, name,0);
    r |= RIFF_write_str( pObject, " : " );
    
    /* value */
    if( isquote ){
        r |= RIFF_write_str( pObject, s_quote );
    }    
    r |= RIFF_vsprintf( pObject, fmt_value, ap );
    if( isquote ){
        r |= RIFF_write_str( pObject, s_quote );
    }    
    if( iscomma ){
        r |= RIFF_write_str( pObject, s_comma);
    }
    r |= RIFF_write_newline(pObject);
    need_indent = 1;

    return r;
}


/* write a tag with a quoted value */
int RIFF_add_json_tag_quoted( struct riff_object *pObject, const char *name, int iscomma, const char *value_fmt, ... )
{
    int r;
    va_list ap;
 
    va_start( ap, value_fmt );
    r = _do_tag( pObject, name, 1, iscomma, value_fmt, ap );
    va_end(ap);
    return r;
}

/* write a tag with a non-quoted value, note: value might be an open { or [ */
int RIFF_add_json_tag_plain( struct riff_object *pObject, const char *name, int iscomma, const char *value_fmt, ... )
{
    int r;
    va_list ap;
 
    va_start( ap, value_fmt );
    r = _do_tag( pObject, name, 0, iscomma, value_fmt, ap );
    va_end(ap);
    return r;
}

/* write null terminated string to riff object */
int RIFF_write_str( struct riff_object *pObject, const char *s )
{
    return RIFF_write_vp( pObject, (const void *)(s), strlen(s), 1 );
}


/* write array of u8 to riff object */
int RIFF_write_u8( struct riff_object *pObject, const uint8_t *p8, size_t n )
{
    return RIFF_write_vp( pObject, (const void *)(p8), n * sizeof(uint8_t),0);
}

/* write array of u16 to riff object */
int RIFF_write_u16( struct riff_object *pObject, const uint16_t *p16, size_t n )
{
    /* fixme: Endian in future */
    return RIFF_write_vp( pObject, (const void *)(p16), n * sizeof(uint16_t),0);
}

/* write array of u32 to riff object */
int RIFF_write_u32( struct riff_object *pObject, const uint32_t *p32, size_t n )
{
    /* fixme: Endian in future */
    return RIFF_write_vp( pObject, (const void *)(p32), n * sizeof(uint32_t),0);
}

/* write array of u64 to riff object */
int RIFF_write_u64( struct riff_object *pObject, const uint64_t *p64, size_t n )
{
    /* fixme: Endian in future */
    return RIFF_write_vp( pObject, (const void *)(p64), n * sizeof(uint64_t),0);
}

/* write single u8 to object */
int RIFF_write_one_u8( struct riff_object *pObject, uint8_t v )
{
    /* fixme: Endian in future */
    return RIFF_write_vp( pObject, (const void *)(&v), sizeof(v),0 );
}

/* write single u16 to object */
int RIFF_write_one_u16( struct riff_object *pObject, uint16_t v )
{
    /* fixme: Endian in future */
    return RIFF_write_vp( pObject, (const void *)(&v), sizeof(v),0 );
}

/* write single u32 to object */
int RIFF_write_one_u32( struct riff_object *pObject, uint32_t v )
{
    /* fixme: Endian in future */
    return RIFF_write_vp( pObject, (const void *)(&v), sizeof(v),0 );
}

/* write single u64 to object */
int RIFF_write_one_u64( struct riff_object *pObject, uint64_t v )
{
    /* fixme: Endian in future */
    return RIFF_write_vp( pObject, (const void *)(&v), sizeof(v),0 );
}
