/**
 * @file  eso_zlib.h
 * @brief ES zlib zip api.
 */

#ifndef __ESO_ZLIB_H__
#define __ESO_ZLIB_H__

#include "es_types.h"
#include "es_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//==============================================================================
// zlib
//==============================================================================

#define Z_OK            0
#define Z_STREAM_END    1
#define Z_NEED_DICT     2
#define Z_ERRNO        (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR   (-3)
#define Z_MEM_ERROR    (-4)
#define Z_BUF_ERROR    (-5)
#define Z_VERSION_ERROR (-6)
/* Return codes for the compression/decompression functions. Negative values
 * are errors, positive values are used for special but normal events.
 */

/*
 * ZLIB
 */
typedef struct
{
	es_size_t (*write)(void *p, const void *buf, es_size_t size);
    /* Returns: result - the number of actually written bytes.
       (result < size) means error */
} es_zlib_ostream_t;

typedef struct
{
	es_int32_t (*read)(void *p, void *buf, es_size_t *size);
    /* if (input(*size) != 0 && output(*size) == 0) means end_of_stream.
       (output(*size) < input(*size)) is allowed. 
       Returns: 0-suceess, other-failure*/
} es_zlib_istream_t;

//==============================================================================

//ZLIB ziped
es_int32_t eso_zlib_deflate(es_zlib_ostream_t *outStream, 
                            es_zlib_istream_t *inStream);

//ZLIB unzipd
es_int32_t eso_zlib_inflate(es_zlib_ostream_t *outStream, 
                            es_zlib_istream_t *inStream);

#ifdef __cplusplus
}
#endif

#endif /* __ESO_ZLIB_H__ */
