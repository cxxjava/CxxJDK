/**
 * @file  eso_md4.h
 * @brief ES RSA Data Security
 */


/*
 * This code implements the MD4 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 * Todd C. Miller modified the MD5 code to do MD4 based on RFC 1186.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 */

#ifndef __ESO_MD4_H__
#define __ESO_MD4_H__

#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MD4_BLOCK_LENGTH     64
#define	MD4_DIGEST_LENGTH		16
#define	MD4_DIGEST_STRING_LENGTH	(MD4_DIGEST_LENGTH * 2 + 1)

/* MD5 context. */
typedef struct {
	es_uint32_t state[4];			/* state */
	es_uint32_t count[2];			/* number of bits, mod 2^64 */
	unsigned char buffer[MD4_BLOCK_LENGTH];	/* input buffer */
} MD4_CTX;

void MD4_Init(MD4_CTX *context);
void MD4_Update(MD4_CTX *mdContext, const es_uint8_t *inBuf, es_uint32_t inLen);
void MD4_Final(es_uint8_t hash[], MD4_CTX *mdContext);

#ifdef __cplusplus
}
#endif

#endif /* __ESO_MD4_H__ */
