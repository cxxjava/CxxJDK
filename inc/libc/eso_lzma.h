/**
 * @file  eso_lzma.h
 * @brief ES lzma zip api.
 */

#ifndef __ESO_LZMA_H__
#define __ESO_LZMA_H__

#include "es_types.h"
#include "es_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//==============================================================================
// lzmz
//==============================================================================

#define ES_SZ_OK 0
#define ES_SZ_ERROR_DATA 1
#define ES_SZ_ERROR_MEM 2
#define ES_SZ_ERROR_CRC 3
#define ES_SZ_ERROR_UNSUPPORTED 4
#define ES_SZ_ERROR_PARAM 5
#define ES_SZ_ERROR_INPUT_EOF 6
#define ES_SZ_ERROR_OUTPUT_EOF 7
#define ES_SZ_ERROR_READ 8
#define ES_SZ_ERROR_WRITE 9
#define ES_SZ_ERROR_PROGRESS 10
#define ES_SZ_ERROR_FAIL 11
#define ES_SZ_ERROR_THREAD 12
#define ES_SZ_ERROR_ARCHIVE 16
#define ES_SZ_ERROR_NO_ARCHIVE 17


//==============================================================================

//LZMA ziped
es_int32_t eso_lzma_zip(es_ostream_t *outStream,
                        es_istream_t *inStream,
                        es_size_t dataSize);

//LZMA unzipd
es_int32_t eso_lzma_unzip(es_ostream_t *outStream, 
                          es_istream_t *inStream);

//LZMA with directory decompression, Note: opath must be last with '/'.
es_int32_t eso_lzma_unzip_arfile(const char *ifname, 
                                 es_size_t offset, 
                                 const char *opath, 
                                 es_bool_t overwrite);

es_int32_t eso_lzma_unzip_arbuff(const char *ibuff, 
                                 es_size_t size, 
                                 const char *opath, 
                                 es_bool_t overwrite);

#ifdef __cplusplus
}
#endif

#endif /* __ESO_LZMA_H__ */
