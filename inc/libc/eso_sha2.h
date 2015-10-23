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
typedef struct _SHA256_CTX {
        es_uint32_t    state[8];
        es_uint64_t    bitcount;
        es_byte_t      buffer[SHA256_BLOCK_LENGTH];
} SHA256_CTX;
typedef struct _SHA512_CTX {
        es_uint64_t    state[8];
        es_uint64_t    bitcount[2];
        es_byte_t      buffer[SHA512_BLOCK_LENGTH];
} SHA512_CTX;

typedef SHA512_CTX SHA384_CTX;


/*** SHA-256/384/512 Function Prototypes ******************************/
void SHA256_Init(SHA256_CTX *);
void SHA256_Update(SHA256_CTX *, const es_byte_t *, es_size_t);
void SHA256_Final(es_byte_t [SHA256_DIGEST_LENGTH], SHA256_CTX *);
char* SHA256_End(SHA256_CTX *, char [SHA256_DIGEST_STRING_LENGTH]);
char* SHA256_Data(const es_byte_t *, es_size_t,
                  char [SHA256_DIGEST_STRING_LENGTH]);

void SHA384_Init(SHA384_CTX *);
void SHA384_Update(SHA384_CTX *, const es_byte_t *, es_size_t);
void SHA384_Final(es_byte_t [SHA384_DIGEST_LENGTH], SHA384_CTX *);
char* SHA384_End(SHA384_CTX *, char [SHA384_DIGEST_STRING_LENGTH]);
char* SHA384_Data(const es_byte_t *, es_size_t,
                  char [SHA384_DIGEST_STRING_LENGTH]);

void SHA512_Init(SHA512_CTX *);
void SHA512_Update(SHA512_CTX *, const es_byte_t *, es_size_t);
void SHA512_Final(es_byte_t [SHA512_DIGEST_LENGTH], SHA512_CTX *);
char* SHA512_End(SHA512_CTX *, char [SHA512_DIGEST_STRING_LENGTH]);
char* SHA512_Data(const es_byte_t *, es_size_t,
                  char [SHA512_DIGEST_STRING_LENGTH]);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __ESO_SHA2_H__ */

