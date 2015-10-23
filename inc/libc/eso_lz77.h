/**
 * @file  eso_lz77.h
 * @brief ES lz77 zip api.
 */

#ifndef __ESO_LZ77_H__
#define __ESO_LZ77_H__

#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define ES_ZIP_BUF_SIZE   2048

// basic unsigned integer types
typedef unsigned char  WORD8;   // unsigned 8bit integer, prefix "b"
typedef unsigned short WORD16;  // unsigned 8bit integer, prefix "w"
typedef unsigned int   WORD32;  // unsigned 8bit integer, prefix "l"

// boolean definitions
typedef signed char BYTEBOOL;  // 8bit boolean, prefix "bl"

#define BOOL_FALSE  0
#define BOOL_TRUE   1

/*
 * LZ77
 */
typedef struct
{
	es_size_t (*write)(void *p, const void *buf, es_size_t size);
	/* Returns: result - the number of actually written bytes.
       (result < size) means error */
} es_lz77_ostream_t;

typedef struct
{
	es_int32_t (*read)(void *p, void *buf, es_size_t *size);
	/* if (input(*size) != 0 && output(*size) == 0) means end_of_stream.
       (output(*size) < input(*size)) is allowed. 
       Returns: 0-suceess, other-failure*/
} es_lz77_istream_t;

//LZ77 ziped
es_int32_t eso_lz77_zip(es_lz77_ostream_t *outStream, es_lz77_istream_t *inStream);

//LZ77 unziped
es_int32_t eso_lz77_unzip(es_lz77_ostream_t *outStream, es_lz77_istream_t *inStream);

#ifdef __cplusplus
}
#endif

#endif /* __ESO_LZ77_H__ */
