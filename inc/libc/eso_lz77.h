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


//LZ77 ziped
es_int32_t eso_lz77_zip(es_ostream_t *outStream, es_istream_t *inStream);

//LZ77 unziped
es_int32_t eso_lz77_unzip(es_ostream_t *outStream, es_istream_t *inStream);

#ifdef __cplusplus
}
#endif

#endif /* __ESO_LZ77_H__ */
