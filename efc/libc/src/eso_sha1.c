/**
 * @file  eso_sha1.c
 * @brief ES SHA1 Library
 */
 
/*
 * provides a means to SHA1 crypt/encode a plaintext password in
 * a way which makes password files compatible with those commonly
 * used in netscape web and ldap installations. It was put together
 * by Clinton Wong <clintdw@netcom.com>, who also notes that:
 *
 * Note: SHA1 support is useful for migration purposes, but is less
 *     secure than Apache's password format, since Apache's (MD5)
 *     password format uses a random eight character salt to generate
 *     one of many possible hashes for the same password.  Netscape
 *     uses plain SHA1 without a salt, so the same password
 *     will always generate the same hash, making it easier
 *     to break since the search space is smaller.
 *
 * See also the documentation in support/SHA1 as to hints on how to
 * migrate an existing netscape installation and other supplied utitlites.
 *
 * This software also makes use of the following component:
 *
 * NIST Secure Hash Algorithm
 *  	heavily modified by Uwe Hollerbach uh@alumni.caltech edu
 *	from Peter C. Gutmann's implementation as found in
 *	Applied Cryptography by Bruce Schneier
 *	This code is hereby placed in the public domain
 */

#include "eso_sha1.h"
#include "eso_base64.h"
#include "eso_libc.h"

/* a bit faster & bigger, if defined */
#define UNROLL_LOOPS

/* NIST's proposed modification to SHA, 7/11/94 */
#define USE_MODIFIED_SHA

/* SHA f()-functions */
#define f1(x,y,z)	((x & y) | (~x & z))
#define f2(x,y,z)	(x ^ y ^ z)
#define f3(x,y,z)	((x & y) | (x & z) | (y & z))
#define f4(x,y,z)	(x ^ y ^ z)

/* SHA constants */
#define CONST1		0x5a827999L
#define CONST2		0x6ed9eba1L
#define CONST3		0x8f1bbcdcL
#define CONST4		0xca62c1d6L

/* 32-bit rotate */

#define ROT32(x,n)	((x << n) | (x >> (32 - n)))

#define FUNC(n,i)						\
    temp = ROT32(A,5) + f##n(B,C,D) + E + W[i] + CONST##n;	\
    E = D; D = C; C = ROT32(B,30); B = A; A = temp

#define SHA_BLOCKSIZE           64


/* do SHA transformation */
static void sha_transform(es_sha1_ctx_t *sha_info)
{
    int i;
    es_uint32_t temp, A, B, C, D, E, W[80];

    for (i = 0; i < 16; ++i) {
	W[i] = sha_info->data[i];
    }
    for (i = 16; i < 80; ++i) {
	W[i] = W[i-3] ^ W[i-8] ^ W[i-14] ^ W[i-16];
#ifdef USE_MODIFIED_SHA
	W[i] = ROT32(W[i], 1);
#endif /* USE_MODIFIED_SHA */
    }
    A = sha_info->digest[0];
    B = sha_info->digest[1];
    C = sha_info->digest[2];
    D = sha_info->digest[3];
    E = sha_info->digest[4];
#ifdef UNROLL_LOOPS
    FUNC(1, 0);  FUNC(1, 1);  FUNC(1, 2);  FUNC(1, 3);  FUNC(1, 4);
    FUNC(1, 5);  FUNC(1, 6);  FUNC(1, 7);  FUNC(1, 8);  FUNC(1, 9);
    FUNC(1,10);  FUNC(1,11);  FUNC(1,12);  FUNC(1,13);  FUNC(1,14);
    FUNC(1,15);  FUNC(1,16);  FUNC(1,17);  FUNC(1,18);  FUNC(1,19);

    FUNC(2,20);  FUNC(2,21);  FUNC(2,22);  FUNC(2,23);  FUNC(2,24);
    FUNC(2,25);  FUNC(2,26);  FUNC(2,27);  FUNC(2,28);  FUNC(2,29);
    FUNC(2,30);  FUNC(2,31);  FUNC(2,32);  FUNC(2,33);  FUNC(2,34);
    FUNC(2,35);  FUNC(2,36);  FUNC(2,37);  FUNC(2,38);  FUNC(2,39);

    FUNC(3,40);  FUNC(3,41);  FUNC(3,42);  FUNC(3,43);  FUNC(3,44);
    FUNC(3,45);  FUNC(3,46);  FUNC(3,47);  FUNC(3,48);  FUNC(3,49);
    FUNC(3,50);  FUNC(3,51);  FUNC(3,52);  FUNC(3,53);  FUNC(3,54);
    FUNC(3,55);  FUNC(3,56);  FUNC(3,57);  FUNC(3,58);  FUNC(3,59);

    FUNC(4,60);  FUNC(4,61);  FUNC(4,62);  FUNC(4,63);  FUNC(4,64);
    FUNC(4,65);  FUNC(4,66);  FUNC(4,67);  FUNC(4,68);  FUNC(4,69);
    FUNC(4,70);  FUNC(4,71);  FUNC(4,72);  FUNC(4,73);  FUNC(4,74);
    FUNC(4,75);  FUNC(4,76);  FUNC(4,77);  FUNC(4,78);  FUNC(4,79);
#else /* !UNROLL_LOOPS */
    for (i = 0; i < 20; ++i) {
	FUNC(1,i);
    }
    for (i = 20; i < 40; ++i) {
	FUNC(2,i);
    }
    for (i = 40; i < 60; ++i) {
	FUNC(3,i);
    }
    for (i = 60; i < 80; ++i) {
	FUNC(4,i);
    }
#endif /* !UNROLL_LOOPS */
    sha_info->digest[0] += A;
    sha_info->digest[1] += B;
    sha_info->digest[2] += C;
    sha_info->digest[3] += D;
    sha_info->digest[4] += E;
}

union endianTest {
    long Long;
    char Char[sizeof(long)];
};

static char isLittleEndian(void)
{
    static union endianTest u;
    u.Long = 1;
    return (u.Char[0] == 1);
}

/* change endianness of data */

/* count is the number of bytes to do an endian flip */
static void maybe_byte_reverse(es_uint32_t *buffer, int count)
{
    int i;
    es_byte_t ct[4], *cp;

    if (isLittleEndian()) {	/* do the swap only if it is little endian */
	count /= sizeof(es_uint32_t);
	cp = (es_byte_t *) buffer;
	for (i = 0; i < count; ++i) {
	    ct[0] = cp[0];
	    ct[1] = cp[1];
	    ct[2] = cp[2];
	    ct[3] = cp[3];
	    cp[0] = ct[3];
	    cp[1] = ct[2];
	    cp[2] = ct[1];
	    cp[3] = ct[0];
	    cp += sizeof(es_uint32_t);
	}
    }
}

/* initialize the SHA digest */

void eso_sha1_init(es_sha1_ctx_t *sha_info)
{
    sha_info->digest[0] = 0x67452301L;
    sha_info->digest[1] = 0xefcdab89L;
    sha_info->digest[2] = 0x98badcfeL;
    sha_info->digest[3] = 0x10325476L;
    sha_info->digest[4] = 0xc3d2e1f0L;
    sha_info->count_lo = 0L;
    sha_info->count_hi = 0L;
    sha_info->local = 0;
}

/* update the SHA digest */

void eso_sha1_update_binary(es_sha1_ctx_t *sha_info,
                                     const unsigned char *buffer,
                                     unsigned int count)
 {
	unsigned int i;

	if ((sha_info->count_lo + ((es_uint32_t) count << 3))
			< sha_info->count_lo) {
		++sha_info->count_hi;
	}
	sha_info->count_lo += (es_uint32_t) count << 3;
	sha_info->count_hi += (es_uint32_t) count >> 29;
	if (sha_info->local) {
		i = SHA_BLOCKSIZE - sha_info->local;
		if (i > count) {
			i = count;
		}
		eso_memcpy(((es_byte_t *) sha_info->data) + sha_info->local, buffer, i);
		count -= i;
		buffer += i;
		sha_info->local += i;
		if (sha_info->local == SHA_BLOCKSIZE) {
			maybe_byte_reverse(sha_info->data, SHA_BLOCKSIZE);
			sha_transform(sha_info);
		} else {
			return;
		}
	}
	while (count >= SHA_BLOCKSIZE) {
		memcpy(sha_info->data, buffer, SHA_BLOCKSIZE);
		buffer += SHA_BLOCKSIZE;
		count -= SHA_BLOCKSIZE;
		maybe_byte_reverse(sha_info->data, SHA_BLOCKSIZE);
		sha_transform(sha_info);
	}
	memcpy(sha_info->data, buffer, count);
	sha_info->local = count;
}

void eso_sha1_update(es_sha1_ctx_t *sha_info, const char *buf)
{
    eso_sha1_update_binary(sha_info, (const unsigned char *) buf, strlen(buf));
}

/* finish computing the SHA digest */

void eso_sha1_final(unsigned char digest[ES_SHA1_DIGESTSIZE],
                             es_sha1_ctx_t *sha_info)
 {
	int count, i, j;
	es_uint32_t lo_bit_count, hi_bit_count, k;

	lo_bit_count = sha_info->count_lo;
	hi_bit_count = sha_info->count_hi;
	count = (int) ((lo_bit_count >> 3) & 0x3f);
	((es_byte_t *) sha_info->data)[count++] = 0x80;
	if (count > SHA_BLOCKSIZE - 8) {
		memset(((es_byte_t *) sha_info->data) + count, 0,
				SHA_BLOCKSIZE - count);
		maybe_byte_reverse(sha_info->data, SHA_BLOCKSIZE);
		sha_transform(sha_info);
		memset((es_byte_t *) sha_info->data, 0, SHA_BLOCKSIZE - 8);
	} else {
		memset(((es_byte_t *) sha_info->data) + count, 0,
				SHA_BLOCKSIZE - 8 - count);
	}
	maybe_byte_reverse(sha_info->data, SHA_BLOCKSIZE);
	sha_info->data[14] = hi_bit_count;
	sha_info->data[15] = lo_bit_count;
	sha_transform(sha_info);

	for (i = 0, j = 0; j < ES_SHA1_DIGESTSIZE; i++) {
		k = sha_info->digest[i];
		digest[j++] = (unsigned char) ((k >> 24) & 0xff);
		digest[j++] = (unsigned char) ((k >> 16) & 0xff);
		digest[j++] = (unsigned char) ((k >> 8) & 0xff);
		digest[j++] = (unsigned char) (k & 0xff);
	}
}

void eso_sha1_base64(const char *clear, int len, char out[ES_SHA1_BASE64SIZE])
{
    int l;
    es_sha1_ctx_t context;
    es_byte_t digest[ES_SHA1_DIGESTSIZE];

    if (eso_strncmp(clear, ES_SHA1PW_ID, ES_SHA1PW_IDLEN) == 0) {
		clear += ES_SHA1PW_IDLEN;
    }

    eso_sha1_init(&context);
    eso_sha1_update_binary(&context, clear, len);
    eso_sha1_final(digest, &context);

    /* SHA1 hash is always 20 chars */
    l = eso_base64_encode((es_uint8_t*)digest,
		                  sizeof(digest), 
		                  (es_uint8_t*)(out),
		                  ES_SHA1_BASE64SIZE);
    out[l] = '\0';

    /*
     * output of base64 encoded SHA1 is always 28 chars.
     */
}
