/**
 * @file  eso_md5.h
 * @brief ES RSA Data Security
 */


/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

/*
 Note: Renamed the functions to avoid having same symbols in
 the linked-in frameworks.
 It is a hack to work around the problem.
 */

#ifndef __ESO_MD5_H__
#define __ESO_MD5_H__

#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ES_MD5_DIGEST_LEN     16

/* MD5 context. */
typedef struct {
  es_uint32_t i[2];                   /* number of _bits_ handled mod 2^64 */
  es_uint32_t buf[4];                                    /* scratch buffer */
  es_uint8_t in[64];                              /* input buffer */
  es_uint8_t digest[16];     /* actual digest after MD5Final call */
} MD5_CTX;

void MD5_Init(MD5_CTX *context);
void MD5_Update(MD5_CTX *mdContext, const es_uint8_t *inBuf, es_uint32_t inLen);
void MD5_Final(es_uint8_t hash[], MD5_CTX *mdContext);

#ifdef __cplusplus
}
#endif

#endif /* __ESO_MD5_H__ */
