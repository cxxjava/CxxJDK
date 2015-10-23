/**
 * @file  eso_sha1.h
 * @brief ES SHA1 Library
 */

/* NIST Secure Hash Algorithm
 * 	heavily modified by Uwe Hollerbach uh@alumni.caltech edu
 * 	from Peter C. Gutmann's implementation as found in
 * 	Applied Cryptography by Bruce Schneier
 * 	This code is hereby placed in the public domain
 */

#ifndef __ESO_SHA1_H__
#define __ESO_SHA1_H__

#include "es_comm.h"
#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif


/** size of the SHA1 DIGEST */
#define ES_SHA1_DIGESTSIZE 20

/**
 * Define the Magic String prefix that identifies a password as being
 * hashed using our algorithm.
 */
#define ES_SHA1PW_ID "{SHA}"

/** length of the SHA Password */
#define ES_SHA1PW_IDLEN 5

/** @see es_sha1_ctx_t */
typedef struct es_sha1_ctx_t es_sha1_ctx_t;

/** 
 * SHA1 context structure
 */
struct es_sha1_ctx_t {
    /** message digest */
    es_uint32_t digest[5];
    /** 64-bit bit counts */
    es_uint32_t count_lo, count_hi;
    /** SHA data buffer */
    es_uint32_t data[16];
    /** unprocessed amount in data */
    int local;
};

/**
 * Provide a means to SHA1 crypt/encode a plaintext password in a way which
 * makes password file compatible with those commonly use in netscape web
 * and ldap installations.
 * @param clear The plaintext password
 * @param len The length of the plaintext password
 * @param out The encrypted/encoded password
 * @note SHA1 support is useful for migration purposes, but is less
 *     secure than Apache's password format, since Apache's (MD5)
 *     password format uses a random eight character salt to generate
 *     one of many possible hashes for the same password.  Netscape
 *     uses plain SHA1 without a salt, so the same password
 *     will always generate the same hash, making it easier
 *     to break since the search space is smaller.
 */
void eso_sha1_base64(const char *clear, int len, char out[ES_SHA1_DIGESTSIZE]);

/**
 * Initialize the SHA digest
 * @param context The SHA context to initialize
 */
void es_sha1_init(es_sha1_ctx_t *context);

/**
 * Update the SHA digest
 * @param context The SHA1 context to update
 * @param input The buffer to add to the SHA digest
 * @param inputLen The length of the input buffer
 */
void es_sha1_update(es_sha1_ctx_t *context, const char *input,
                                unsigned int inputLen);

/**
 * Update the SHA digest with binary data
 * @param context The SHA1 context to update
 * @param input The buffer to add to the SHA digest
 * @param inputLen The length of the input buffer
 */
void es_sha1_update_binary(es_sha1_ctx_t *context,
                                       const unsigned char *input,
                                       unsigned int inputLen);

/**
 * Finish computing the SHA digest
 * @param digest the output buffer in which to store the digest
 * @param context The context to finalize
 */
void eso_sha1_final(unsigned char digest[ES_SHA1_DIGESTSIZE],
                               es_sha1_ctx_t *context);

#ifdef __cplusplus
}
#endif

#endif	/* __ESO_SHA1_H__ */
