/**
 * @file  eso_sha2.h
 * @brief ES SHA2 Library
 */

/*
 * FILE:        sha2.h
 * AUTHOR:      Aaron D. Gifford <me@aarongifford.com>
 * 
 * A licence was granted to the ASF by Aaron on 4 November 2003.
 */

#ifndef __ESO_SHA2_H__
#define __ESO_SHA2_H__

#include "es_comm.h"
#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*** SHA-256/384/512 Various Length Definitions ***********************/
#define SHA256_BLOCK_LENGTH             64
#define SHA256_DIGEST_LENGTH            32
#define SHA256_DIGEST_STRING_LENGTH     (SHA256_DIGEST_LENGTH * 2 + 1)
#define SHA384_BLOCK_LENGTH             128
#define SHA384_DIGEST_LENGTH            48
#define SHA384_DIGEST_STRING_LENGTH     (SHA384_DIGEST_LENGTH * 2 + 1)
#define SHA512_BLOCK_LENGTH             128
#define SHA512_DIGEST_LENGTH            64
#define SHA512_DIGEST_STRING_LENGTH     (SHA512_DIGEST_LENGTH * 2 + 1)


/*** SHA-256/384/512 Context Structures *******************************/
typedef struct {
        es_uint32_t    state[8];
        es_uint64_t    bitcount;
        es_byte_t      buffer[SHA256_BLOCK_LENGTH];
} es_sha256_ctx_t;
typedef struct {
        es_uint64_t    state[8];
        es_uint64_t    bitcount[2];
        es_byte_t      buffer[SHA512_BLOCK_LENGTH];
} es_sha512_ctx_t;

typedef es_sha512_ctx_t es_sha384_ctx_t;


/*** SHA-256/384/512 Function Prototypes ******************************/
void eso_sha256_init(es_sha256_ctx_t *);
void eso_sha256_update(es_sha256_ctx_t *, const es_byte_t *, es_size_t);
void eso_sha256_final(es_byte_t [SHA256_DIGEST_LENGTH], es_sha256_ctx_t *);
char* eso_sha256_end(es_sha256_ctx_t *, char [SHA256_DIGEST_STRING_LENGTH]);
char* eso_sha256_data(const es_byte_t *, es_size_t,
                  char [SHA256_DIGEST_STRING_LENGTH]);

void eso_sha384_init(es_sha384_ctx_t *);
void eso_sha384_update(es_sha384_ctx_t *, const es_byte_t *, es_size_t);
void eso_sha384_final(es_byte_t [SHA384_DIGEST_LENGTH], es_sha384_ctx_t *);
char* eso_sha384_end(es_sha384_ctx_t *, char [SHA384_DIGEST_STRING_LENGTH]);
char* eso_sha384_data(const es_byte_t *, es_size_t,
                  char [SHA384_DIGEST_STRING_LENGTH]);

void eso_sha512_init(es_sha512_ctx_t *);
void eso_sha512_update(es_sha512_ctx_t *, const es_byte_t *, es_size_t);
void eso_sha512_final(es_byte_t [SHA512_DIGEST_LENGTH], es_sha512_ctx_t *);
char* eso_sha512_end(es_sha512_ctx_t *, char [SHA512_DIGEST_STRING_LENGTH]);
char* eso_sha512_data(const es_byte_t *, es_size_t,
                  char [SHA512_DIGEST_STRING_LENGTH]);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __ESO_SHA2_H__ */

