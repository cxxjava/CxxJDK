/**
 * @file  eso_sha2.c
 * @brief ES SHA2 Library
 */

/*
 * FILE:        sha2.c
 * AUTHOR:      Aaron D. Gifford <me@aarongifford.com>
 *
 * A licence was granted to the ASF by Aaron on 4 November 2003.
 */

#include "eso_sha2.h"
#include "eso_libc.h"

/*
 * ASSERT NOTE:
 * Some sanity checking code is included using assert().  On my FreeBSD
 * system, this additional code can be removed by compiling with NDEBUG
 * defined.  Check your own systems manpage on assert() to see how to
 * compile WITHOUT the sanity checking code on your system.
 *
 * UNROLLED TRANSFORM LOOP NOTE:
 * You can define SHA2_UNROLL_TRANSFORM to use the unrolled transform
 * loop version for the hash transform rounds (defined using macros
 * later in this file).  Either define on the command line, for example:
 *
 *   cc -DSHA2_UNROLL_TRANSFORM -o sha2 sha2.c sha2prog.c
 *
 * or define below:
 *
 *   #define SHA2_UNROLL_TRANSFORM
 *
 */

/*** SHA-256/384/512 Machine Architecture Definitions *****************/
typedef es_byte_t   sha2_byte;         /* Exactly 1 byte */
typedef es_uint32_t sha2_word32;       /* Exactly 4 bytes */
typedef es_uint64_t sha2_word64;       /* Exactly 8 bytes */

/*** SHA-256/384/512 Various Length Definitions ***********************/
/* NOTE: Most of these are in sha2.h */
#define SHA256_SHORT_BLOCK_LENGTH       (SHA256_BLOCK_LENGTH - 8)
#define SHA384_SHORT_BLOCK_LENGTH       (SHA384_BLOCK_LENGTH - 16)
#define SHA512_SHORT_BLOCK_LENGTH       (SHA512_BLOCK_LENGTH - 16)


/*** ENDIAN REVERSAL MACROS *******************************************/
#ifndef __BIG_ENDIAN__
#define REVERSE32(w,x)  { \
        sha2_word32 tmp = (w); \
        tmp = (tmp >> 16) | (tmp << 16); \
        (x) = ((tmp & 0xff00ff00UL) >> 8) | ((tmp & 0x00ff00ffUL) << 8); \
}
#define REVERSE64(w,x)  { \
        sha2_word64 tmp = (w); \
        tmp = (tmp >> 32) | (tmp << 32); \
        tmp = ((tmp & ULLONG(0xff00ff00ff00ff00)) >> 8) | \
              ((tmp & ULLONG(0x00ff00ff00ff00ff)) << 8); \
        (x) = ((tmp & ULLONG(0xffff0000ffff0000)) >> 16) | \
              ((tmp & ULLONG(0x0000ffff0000ffff)) << 16); \
}
#endif /* __BIG_ENDIAN__ */

/*
 * Macro for incrementally adding the unsigned 64-bit integer n to the
 * unsigned 128-bit integer (represented using a two-element array of
 * 64-bit words):
 */
#define ADDINC128(w,n)  { \
        (w)[0] += (sha2_word64)(n); \
        if ((w)[0] < (n)) { \
                (w)[1]++; \
        } \
}

/*
 * Macros for copying blocks of memory and for zeroing out ranges
 * of memory.  Using these macros makes it easy to switch from
 * using memset()/memcpy() and using bzero()/bcopy().
 *
 * Please define either SHA2_USE_MEMSET_MEMCPY or define
 * SHA2_USE_BZERO_BCOPY depending on which function set you
 * choose to use:
 */
#if !defined(SHA2_USE_MEMSET_MEMCPY) && !defined(SHA2_USE_BZERO_BCOPY)
/* Default to memset()/memcpy() if no option is specified */
#define SHA2_USE_MEMSET_MEMCPY  1
#endif
#if defined(SHA2_USE_MEMSET_MEMCPY) && defined(SHA2_USE_BZERO_BCOPY)
/* Abort with an error if BOTH options are defined */
#error Define either SHA2_USE_MEMSET_MEMCPY or SHA2_USE_BZERO_BCOPY, not both!
#endif

#ifdef SHA2_USE_MEMSET_MEMCPY
#define MEMSET_BZERO(p,l)       eso_memset((p), 0, (l))
#define MEMCPY_BCOPY(d,s,l)     eso_memcpy((d), (s), (l))
#endif
#ifdef SHA2_USE_BZERO_BCOPY
#define MEMSET_BZERO(p,l)       bzero((p), (l))
#define MEMCPY_BCOPY(d,s,l)     bcopy((s), (d), (l))
#endif


/*** THE SIX LOGICAL FUNCTIONS ****************************************/
/*
 * Bit shifting and rotation (used by the six SHA-XYZ logical functions:
 *
 *   NOTE:  The naming of R and S appears backwards here (R is a SHIFT and
 *   S is a ROTATION) because the SHA-256/384/512 description document
 *   (see http://csrc.nist.gov/cryptval/shs/sha256-384-512.pdf) uses this
 *   same "backwards" definition.
 */
/* Shift-right (used in SHA-256, SHA-384, and SHA-512): */
#define R(b,x)          ((x) >> (b))
/* 32-bit Rotate-right (used in SHA-256): */
#define S32(b,x)        (((x) >> (b)) | ((x) << (32 - (b))))
/* 64-bit Rotate-right (used in SHA-384 and SHA-512): */
#define S64(b,x)        (((x) >> (b)) | ((x) << (64 - (b))))

/* Two of six logical functions used in SHA-256, SHA-384, and SHA-512: */
#define Ch(x,y,z)       (((x) & (y)) ^ ((~(x)) & (z)))
#define Maj(x,y,z)      (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

/* Four of six logical functions used in SHA-256: */
#define Sigma0_256(x)   (S32(2,  (x)) ^ S32(13, (x)) ^ S32(22, (x)))
#define Sigma1_256(x)   (S32(6,  (x)) ^ S32(11, (x)) ^ S32(25, (x)))
#define sigma0_256(x)   (S32(7,  (x)) ^ S32(18, (x)) ^ R(3 ,   (x)))
#define sigma1_256(x)   (S32(17, (x)) ^ S32(19, (x)) ^ R(10,   (x)))

/* Four of six logical functions used in SHA-384 and SHA-512: */
#define Sigma0_512(x)   (S64(28, (x)) ^ S64(34, (x)) ^ S64(39, (x)))
#define Sigma1_512(x)   (S64(14, (x)) ^ S64(18, (x)) ^ S64(41, (x)))
#define sigma0_512(x)   (S64( 1, (x)) ^ S64( 8, (x)) ^ R( 7,   (x)))
#define sigma1_512(x)   (S64(19, (x)) ^ S64(61, (x)) ^ R( 6,   (x)))

/*** INTERNAL FUNCTION PROTOTYPES *************************************/
/* NOTE: These should not be accessed directly from outside this
 * library -- they are intended for private internal visibility/use
 * only.
 */
static void SHA512_Last(es_sha512_ctx_t*);
static void SHA256_Transform(es_sha256_ctx_t*, const sha2_word32*);
static void SHA512_Transform(es_sha512_ctx_t*, const sha2_word64*);


/*** SHA-XYZ INITIAL HASH VALUES AND CONSTANTS ************************/
/* Hash constant words K for SHA-256: */
static const sha2_word32 K256[64] = {
        0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL,
        0x3956c25bUL, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL,
        0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL,
        0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL,
        0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
        0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL,
        0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL,
        0xc6e00bf3UL, 0xd5a79147UL, 0x06ca6351UL, 0x14292967UL,
        0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL,
        0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
        0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL,
        0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL,
        0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL,
        0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL,
        0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
        0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};

/* Initial hash value H for SHA-256: */
static const sha2_word32 sha256_initial_hash_value[8] = {
        0x6a09e667UL,
        0xbb67ae85UL,
        0x3c6ef372UL,
        0xa54ff53aUL,
        0x510e527fUL,
        0x9b05688cUL,
        0x1f83d9abUL,
        0x5be0cd19UL
};

/* Hash constant words K for SHA-384 and SHA-512: */
static const sha2_word64 K512[80] = {
    ULLONG(0x428a2f98d728ae22), ULLONG(0x7137449123ef65cd),
    ULLONG(0xb5c0fbcfec4d3b2f), ULLONG(0xe9b5dba58189dbbc),
    ULLONG(0x3956c25bf348b538), ULLONG(0x59f111f1b605d019),
    ULLONG(0x923f82a4af194f9b), ULLONG(0xab1c5ed5da6d8118),
    ULLONG(0xd807aa98a3030242), ULLONG(0x12835b0145706fbe),
    ULLONG(0x243185be4ee4b28c), ULLONG(0x550c7dc3d5ffb4e2),
    ULLONG(0x72be5d74f27b896f), ULLONG(0x80deb1fe3b1696b1),
    ULLONG(0x9bdc06a725c71235), ULLONG(0xc19bf174cf692694),
    ULLONG(0xe49b69c19ef14ad2), ULLONG(0xefbe4786384f25e3),
    ULLONG(0x0fc19dc68b8cd5b5), ULLONG(0x240ca1cc77ac9c65),
    ULLONG(0x2de92c6f592b0275), ULLONG(0x4a7484aa6ea6e483),
    ULLONG(0x5cb0a9dcbd41fbd4), ULLONG(0x76f988da831153b5),
    ULLONG(0x983e5152ee66dfab), ULLONG(0xa831c66d2db43210),
    ULLONG(0xb00327c898fb213f), ULLONG(0xbf597fc7beef0ee4),
    ULLONG(0xc6e00bf33da88fc2), ULLONG(0xd5a79147930aa725),
    ULLONG(0x06ca6351e003826f), ULLONG(0x142929670a0e6e70),
    ULLONG(0x27b70a8546d22ffc), ULLONG(0x2e1b21385c26c926),
    ULLONG(0x4d2c6dfc5ac42aed), ULLONG(0x53380d139d95b3df),
    ULLONG(0x650a73548baf63de), ULLONG(0x766a0abb3c77b2a8),
    ULLONG(0x81c2c92e47edaee6), ULLONG(0x92722c851482353b),
    ULLONG(0xa2bfe8a14cf10364), ULLONG(0xa81a664bbc423001),
    ULLONG(0xc24b8b70d0f89791), ULLONG(0xc76c51a30654be30),
    ULLONG(0xd192e819d6ef5218), ULLONG(0xd69906245565a910),
    ULLONG(0xf40e35855771202a), ULLONG(0x106aa07032bbd1b8),
    ULLONG(0x19a4c116b8d2d0c8), ULLONG(0x1e376c085141ab53),
    ULLONG(0x2748774cdf8eeb99), ULLONG(0x34b0bcb5e19b48a8),
    ULLONG(0x391c0cb3c5c95a63), ULLONG(0x4ed8aa4ae3418acb),
    ULLONG(0x5b9cca4f7763e373), ULLONG(0x682e6ff3d6b2b8a3),
    ULLONG(0x748f82ee5defb2fc), ULLONG(0x78a5636f43172f60),
    ULLONG(0x84c87814a1f0ab72), ULLONG(0x8cc702081a6439ec),
    ULLONG(0x90befffa23631e28), ULLONG(0xa4506cebde82bde9),
    ULLONG(0xbef9a3f7b2c67915), ULLONG(0xc67178f2e372532b),
    ULLONG(0xca273eceea26619c), ULLONG(0xd186b8c721c0c207),
    ULLONG(0xeada7dd6cde0eb1e), ULLONG(0xf57d4f7fee6ed178),
    ULLONG(0x06f067aa72176fba), ULLONG(0x0a637dc5a2c898a6),
    ULLONG(0x113f9804bef90dae), ULLONG(0x1b710b35131c471b),
    ULLONG(0x28db77f523047d84), ULLONG(0x32caab7b40c72493),
    ULLONG(0x3c9ebe0a15c9bebc), ULLONG(0x431d67c49c100d4c),
    ULLONG(0x4cc5d4becb3e42b6), ULLONG(0x597f299cfc657e2a),
    ULLONG(0x5fcb6fab3ad6faec), ULLONG(0x6c44198c4a475817)
};

/* Initial hash value H for SHA-384 */
static const sha2_word64 sha384_initial_hash_value[8] = {
    ULLONG(0xcbbb9d5dc1059ed8),
    ULLONG(0x629a292a367cd507),
    ULLONG(0x9159015a3070dd17),
    ULLONG(0x152fecd8f70e5939),
    ULLONG(0x67332667ffc00b31),
    ULLONG(0x8eb44a8768581511),
    ULLONG(0xdb0c2e0d64f98fa7),
    ULLONG(0x47b5481dbefa4fa4)
};

/* Initial hash value H for SHA-512 */
static const sha2_word64 sha512_initial_hash_value[8] = {
    ULLONG(0x6a09e667f3bcc908),
    ULLONG(0xbb67ae8584caa73b),
    ULLONG(0x3c6ef372fe94f82b),
    ULLONG(0xa54ff53a5f1d36f1),
    ULLONG(0x510e527fade682d1),
    ULLONG(0x9b05688c2b3e6c1f),
    ULLONG(0x1f83d9abfb41bd6b),
    ULLONG(0x5be0cd19137e2179)
};

/*
 * Constant used by SHA256/384/512_End() functions for converting the
 * digest to a readable hexadecimal character string:
 */
static const char *sha2_hex_digits = "0123456789abcdef";


/*** SHA-256: *********************************************************/
void eso_sha256_init(es_sha256_ctx_t* context) {
        if (context == (es_sha256_ctx_t*)0) {
                return;
        }
        MEMCPY_BCOPY(context->state, sha256_initial_hash_value, SHA256_DIGEST_LENGTH);
        MEMSET_BZERO(context->buffer, SHA256_BLOCK_LENGTH);
        context->bitcount = 0;
}

#ifdef SHA2_UNROLL_TRANSFORM

/* Unrolled SHA-256 round macros: */

#ifndef __BIG_ENDIAN__

#define ROUND256_0_TO_15(a,b,c,d,e,f,g,h)       \
        REVERSE32(*data++, W256[j]); \
        T1 = (h) + Sigma1_256(e) + Ch((e), (f), (g)) + \
             K256[j] + W256[j]; \
        (d) += T1; \
        (h) = T1 + Sigma0_256(a) + Maj((a), (b), (c)); \
        j++


#else /* __BIG_ENDIAN__ */

#define ROUND256_0_TO_15(a,b,c,d,e,f,g,h)       \
        T1 = (h) + Sigma1_256(e) + Ch((e), (f), (g)) + \
             K256[j] + (W256[j] = *data++); \
        (d) += T1; \
        (h) = T1 + Sigma0_256(a) + Maj((a), (b), (c)); \
        j++

#endif /* __BIG_ENDIAN__ */

#define ROUND256(a,b,c,d,e,f,g,h)       \
        s0 = W256[(j+1)&0x0f]; \
        s0 = sigma0_256(s0); \
        s1 = W256[(j+14)&0x0f]; \
        s1 = sigma1_256(s1); \
        T1 = (h) + Sigma1_256(e) + Ch((e), (f), (g)) + K256[j] + \
             (W256[j&0x0f] += s1 + W256[(j+9)&0x0f] + s0); \
        (d) += T1; \
        (h) = T1 + Sigma0_256(a) + Maj((a), (b), (c)); \
        j++

void SHA256_Transform(es_sha256_ctx_t* context, const sha2_word32* data) {
        sha2_word32     a, b, c, d, e, f, g, h, s0, s1;
        sha2_word32     T1, *W256;
        int             j;

        W256 = (sha2_word32*)context->buffer;

        /* Initialize registers with the prev. intermediate value */
        a = context->state[0];
        b = context->state[1];
        c = context->state[2];
        d = context->state[3];
        e = context->state[4];
        f = context->state[5];
        g = context->state[6];
        h = context->state[7];

        j = 0;
        do {
                /* Rounds 0 to 15 (unrolled): */
                ROUND256_0_TO_15(a,b,c,d,e,f,g,h);
                ROUND256_0_TO_15(h,a,b,c,d,e,f,g);
                ROUND256_0_TO_15(g,h,a,b,c,d,e,f);
                ROUND256_0_TO_15(f,g,h,a,b,c,d,e);
                ROUND256_0_TO_15(e,f,g,h,a,b,c,d);
                ROUND256_0_TO_15(d,e,f,g,h,a,b,c);
                ROUND256_0_TO_15(c,d,e,f,g,h,a,b);
                ROUND256_0_TO_15(b,c,d,e,f,g,h,a);
        } while (j < 16);

        /* Now for the remaining rounds to 64: */
        do {
                ROUND256(a,b,c,d,e,f,g,h);
                ROUND256(h,a,b,c,d,e,f,g);
                ROUND256(g,h,a,b,c,d,e,f);
                ROUND256(f,g,h,a,b,c,d,e);
                ROUND256(e,f,g,h,a,b,c,d);
                ROUND256(d,e,f,g,h,a,b,c);
                ROUND256(c,d,e,f,g,h,a,b);
                ROUND256(b,c,d,e,f,g,h,a);
        } while (j < 64);

        /* Compute the current intermediate hash value */
        context->state[0] += a;
        context->state[1] += b;
        context->state[2] += c;
        context->state[3] += d;
        context->state[4] += e;
        context->state[5] += f;
        context->state[6] += g;
        context->state[7] += h;

        /* Clean up */
        a = b = c = d = e = f = g = h = T1 = 0;
}

#else /* SHA2_UNROLL_TRANSFORM */

void SHA256_Transform(es_sha256_ctx_t* context, const sha2_word32* data) {
        sha2_word32     a, b, c, d, e, f, g, h, s0, s1;
        sha2_word32     T1, T2, *W256;
        int             j;

        W256 = (sha2_word32*)context->buffer;

        /* Initialize registers with the prev. intermediate value */
        a = context->state[0];
        b = context->state[1];
        c = context->state[2];
        d = context->state[3];
        e = context->state[4];
        f = context->state[5];
        g = context->state[6];
        h = context->state[7];

        j = 0;
        do {
#ifndef __BIG_ENDIAN__
                /* Copy data while converting to host byte order */
                REVERSE32(*data++,W256[j]);
                /* Apply the SHA-256 compression function to update a..h */
                T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] + W256[j];
#else /* __BIG_ENDIAN__ */
                /* Apply the SHA-256 compression function to update a..h with copy */
                T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] + (W256[j] = *data++);
#endif /* __BIG_ENDIAN__ */
                T2 = Sigma0_256(a) + Maj(a, b, c);
                h = g;
                g = f;
                f = e;
                e = d + T1;
                d = c;
                c = b;
                b = a;
                a = T1 + T2;

                j++;
        } while (j < 16);

        do {
                /* Part of the message block expansion: */
                s0 = W256[(j+1)&0x0f];
                s0 = sigma0_256(s0);
                s1 = W256[(j+14)&0x0f]; 
                s1 = sigma1_256(s1);

                /* Apply the SHA-256 compression function to update a..h */
                T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] + 
                     (W256[j&0x0f] += s1 + W256[(j+9)&0x0f] + s0);
                T2 = Sigma0_256(a) + Maj(a, b, c);
                h = g;
                g = f;
                f = e;
                e = d + T1;
                d = c;
                c = b;
                b = a;
                a = T1 + T2;

                j++;
        } while (j < 64);

        /* Compute the current intermediate hash value */
        context->state[0] += a;
        context->state[1] += b;
        context->state[2] += c;
        context->state[3] += d;
        context->state[4] += e;
        context->state[5] += f;
        context->state[6] += g;
        context->state[7] += h;

        /* Clean up */
        a = b = c = d = e = f = g = h = T1 = T2 = 0;
}

#endif /* SHA2_UNROLL_TRANSFORM */

void eso_sha256_update(es_sha256_ctx_t* context, const sha2_byte *data, es_size_t len) {
        unsigned int    freespace, usedspace;

        if (len == 0) {
                /* Calling with no data is valid - we do nothing */
                return;
        }

        /* Sanity check: */
        ES_ASSERT(context != (es_sha256_ctx_t*)0 && data != (sha2_byte*)0);

        usedspace = (unsigned int)((context->bitcount >> 3) 
                                 % SHA256_BLOCK_LENGTH);
        if (usedspace > 0) {
                /* Calculate how much free space is available in the buffer */
                freespace = SHA256_BLOCK_LENGTH - usedspace;

                if (len >= freespace) {
                        /* Fill the buffer completely and process it */
                        MEMCPY_BCOPY(&context->buffer[usedspace], data, freespace);
                        context->bitcount += freespace << 3;
                        len -= freespace;
                        data += freespace;
                        SHA256_Transform(context, (sha2_word32*)context->buffer);
                } else {
                        /* The buffer is not yet full */
                        MEMCPY_BCOPY(&context->buffer[usedspace], data, len);
                        context->bitcount += len << 3;
                        /* Clean up: */
                        usedspace = freespace = 0;
                        return;
                }
        }
        while (len >= SHA256_BLOCK_LENGTH) {
                /* Process as many complete blocks as we can */
                SHA256_Transform(context, (sha2_word32*)data);
                context->bitcount += SHA256_BLOCK_LENGTH << 3;
                len -= SHA256_BLOCK_LENGTH;
                data += SHA256_BLOCK_LENGTH;
        }
        if (len > 0) {
                /* There's left-overs, so save 'em */
                MEMCPY_BCOPY(context->buffer, data, len);
                context->bitcount += len << 3;
        }
        /* Clean up: */
        usedspace = freespace = 0;
}

void eso_sha256_final(sha2_byte digest[], es_sha256_ctx_t* context) {
        sha2_word32     *d = (sha2_word32*)digest;
        unsigned int    usedspace;

        /* Sanity check: */
        ES_ASSERT(context != (es_sha256_ctx_t*)0);

        /* If no digest buffer is passed, we don't bother doing this: */
        if (digest != (sha2_byte*)0) {
                usedspace = (unsigned int)((context->bitcount >> 3) 
                                         % SHA256_BLOCK_LENGTH);
#ifndef __BIG_ENDIAN__
                /* Convert FROM host byte order */
                REVERSE64(context->bitcount,context->bitcount);
#endif
                if (usedspace > 0) {
                        /* Begin padding with a 1 bit: */
                        context->buffer[usedspace++] = 0x80;

                        if (usedspace <= SHA256_SHORT_BLOCK_LENGTH) {
                                /* Set-up for the last transform: */
                                MEMSET_BZERO(&context->buffer[usedspace], SHA256_SHORT_BLOCK_LENGTH - usedspace);
                        } else {
                                if (usedspace < SHA256_BLOCK_LENGTH) {
                                        MEMSET_BZERO(&context->buffer[usedspace], SHA256_BLOCK_LENGTH - usedspace);
                                }
                                /* Do second-to-last transform: */
                                SHA256_Transform(context, (sha2_word32*)context->buffer);

                                /* And set-up for the last transform: */
                                MEMSET_BZERO(context->buffer, SHA256_SHORT_BLOCK_LENGTH);
                        }
                } else {
                        /* Set-up for the last transform: */
                        MEMSET_BZERO(context->buffer, SHA256_SHORT_BLOCK_LENGTH);

                        /* Begin padding with a 1 bit: */
                        *context->buffer = 0x80;
                }
                /* Set the bit count: */
                *(sha2_word64*)&context->buffer[SHA256_SHORT_BLOCK_LENGTH] = context->bitcount;

                /* Final transform: */
                SHA256_Transform(context, (sha2_word32*)context->buffer);

#ifndef __BIG_ENDIAN__
                {
                        /* Convert TO host byte order */
                        int     j;
                        for (j = 0; j < 8; j++) {
                                REVERSE32(context->state[j],context->state[j]);
                                *d++ = context->state[j];
                        }
                }
#else
                MEMCPY_BCOPY(d, context->state, SHA256_DIGEST_LENGTH);
#endif
        }

        /* Clean up state data: */
        MEMSET_BZERO(context, sizeof(context));
        usedspace = 0;
}

char *eso_sha256_end(es_sha256_ctx_t* context, char buffer[]) {
        sha2_byte       digest[SHA256_DIGEST_LENGTH], *d = digest;
        int             i;

        /* Sanity check: */
        ES_ASSERT(context != (es_sha256_ctx_t*)0);

        if (buffer != (char*)0) {
                eso_sha256_final(digest, context);

                for (i = 0; i < SHA256_DIGEST_LENGTH; i++) {
                        *buffer++ = sha2_hex_digits[(*d & 0xf0) >> 4];
                        *buffer++ = sha2_hex_digits[*d & 0x0f];
                        d++;
                }
                *buffer = (char)0;
        } else {
                MEMSET_BZERO(context, sizeof(context));
        }
        MEMSET_BZERO(digest, SHA256_DIGEST_LENGTH);
        return buffer;
}

char* eso_sha256_data(const sha2_byte* data, es_size_t len, char digest[SHA256_DIGEST_STRING_LENGTH]) {
        es_sha256_ctx_t      context;

        eso_sha256_init(&context);
        eso_sha256_update(&context, data, len);
        return eso_sha256_end(&context, digest);
}


/*** SHA-512: *********************************************************/
void eso_sha512_init(es_sha512_ctx_t* context) {
        if (context == (es_sha512_ctx_t*)0) {
                return;
        }
        MEMCPY_BCOPY(context->state, sha512_initial_hash_value, SHA512_DIGEST_LENGTH);
        MEMSET_BZERO(context->buffer, SHA512_BLOCK_LENGTH);
        context->bitcount[0] = context->bitcount[1] =  0;
}

#ifdef SHA2_UNROLL_TRANSFORM

/* Unrolled SHA-512 round macros: */
#ifndef __BIG_ENDIAN__

#define ROUND512_0_TO_15(a,b,c,d,e,f,g,h)       \
        REVERSE64(*data++, W512[j]); \
        T1 = (h) + Sigma1_512(e) + Ch((e), (f), (g)) + \
             K512[j] + W512[j]; \
        (d) += T1, \
        (h) = T1 + Sigma0_512(a) + Maj((a), (b), (c)), \
        j++


#else /* __BIG_ENDIAN__ */

#define ROUND512_0_TO_15(a,b,c,d,e,f,g,h)       \
        T1 = (h) + Sigma1_512(e) + Ch((e), (f), (g)) + \
             K512[j] + (W512[j] = *data++); \
        (d) += T1; \
        (h) = T1 + Sigma0_512(a) + Maj((a), (b), (c)); \
        j++

#endif /* __BIG_ENDIAN__ */

#define ROUND512(a,b,c,d,e,f,g,h)       \
        s0 = W512[(j+1)&0x0f]; \
        s0 = sigma0_512(s0); \
        s1 = W512[(j+14)&0x0f]; \
        s1 = sigma1_512(s1); \
        T1 = (h) + Sigma1_512(e) + Ch((e), (f), (g)) + K512[j] + \
             (W512[j&0x0f] += s1 + W512[(j+9)&0x0f] + s0); \
        (d) += T1; \
        (h) = T1 + Sigma0_512(a) + Maj((a), (b), (c)); \
        j++

void SHA512_Transform(es_sha512_ctx_t* context, const sha2_word64* data) {
        sha2_word64     a, b, c, d, e, f, g, h, s0, s1;
        sha2_word64     T1, *W512 = (sha2_word64*)context->buffer;
        int             j;

        /* Initialize registers with the prev. intermediate value */
        a = context->state[0];
        b = context->state[1];
        c = context->state[2];
        d = context->state[3];
        e = context->state[4];
        f = context->state[5];
        g = context->state[6];
        h = context->state[7];

        j = 0;
        do {
                ROUND512_0_TO_15(a,b,c,d,e,f,g,h);
                ROUND512_0_TO_15(h,a,b,c,d,e,f,g);
                ROUND512_0_TO_15(g,h,a,b,c,d,e,f);
                ROUND512_0_TO_15(f,g,h,a,b,c,d,e);
                ROUND512_0_TO_15(e,f,g,h,a,b,c,d);
                ROUND512_0_TO_15(d,e,f,g,h,a,b,c);
                ROUND512_0_TO_15(c,d,e,f,g,h,a,b);
                ROUND512_0_TO_15(b,c,d,e,f,g,h,a);
        } while (j < 16);

        /* Now for the remaining rounds up to 79: */
        do {
                ROUND512(a,b,c,d,e,f,g,h);
                ROUND512(h,a,b,c,d,e,f,g);
                ROUND512(g,h,a,b,c,d,e,f);
                ROUND512(f,g,h,a,b,c,d,e);
                ROUND512(e,f,g,h,a,b,c,d);
                ROUND512(d,e,f,g,h,a,b,c);
                ROUND512(c,d,e,f,g,h,a,b);
                ROUND512(b,c,d,e,f,g,h,a);
        } while (j < 80);

        /* Compute the current intermediate hash value */
        context->state[0] += a;
        context->state[1] += b;
        context->state[2] += c;
        context->state[3] += d;
        context->state[4] += e;
        context->state[5] += f;
        context->state[6] += g;
        context->state[7] += h;

        /* Clean up */
        a = b = c = d = e = f = g = h = T1 = 0;
}

#else /* SHA2_UNROLL_TRANSFORM */

void SHA512_Transform(es_sha512_ctx_t* context, const sha2_word64* data) {
        sha2_word64     a, b, c, d, e, f, g, h, s0, s1;
        sha2_word64     T1, T2, *W512 = (sha2_word64*)context->buffer;
        int             j;

        /* Initialize registers with the prev. intermediate value */
        a = context->state[0];
        b = context->state[1];
        c = context->state[2];
        d = context->state[3];
        e = context->state[4];
        f = context->state[5];
        g = context->state[6];
        h = context->state[7];

        j = 0;
        do {
#ifndef __BIG_ENDIAN__
                /* Convert TO host byte order */
                REVERSE64(*data++, W512[j]);
                /* Apply the SHA-512 compression function to update a..h */
                T1 = h + Sigma1_512(e) + Ch(e, f, g) + K512[j] + W512[j];
#else /* __BIG_ENDIAN__ */
                /* Apply the SHA-512 compression function to update a..h with copy */
                T1 = h + Sigma1_512(e) + Ch(e, f, g) + K512[j] + (W512[j] = *data++);
#endif /* __BIG_ENDIAN__ */
                T2 = Sigma0_512(a) + Maj(a, b, c);
                h = g;
                g = f;
                f = e;
                e = d + T1;
                d = c;
                c = b;
                b = a;
                a = T1 + T2;

                j++;
        } while (j < 16);

        do {
                /* Part of the message block expansion: */
                s0 = W512[(j+1)&0x0f];
                s0 = sigma0_512(s0);
                s1 = W512[(j+14)&0x0f];
                s1 =  sigma1_512(s1);

                /* Apply the SHA-512 compression function to update a..h */
                T1 = h + Sigma1_512(e) + Ch(e, f, g) + K512[j] +
                     (W512[j&0x0f] += s1 + W512[(j+9)&0x0f] + s0);
                T2 = Sigma0_512(a) + Maj(a, b, c);
                h = g;
                g = f;
                f = e;
                e = d + T1;
                d = c;
                c = b;
                b = a;
                a = T1 + T2;

                j++;
        } while (j < 80);

        /* Compute the current intermediate hash value */
        context->state[0] += a;
        context->state[1] += b;
        context->state[2] += c;
        context->state[3] += d;
        context->state[4] += e;
        context->state[5] += f;
        context->state[6] += g;
        context->state[7] += h;

        /* Clean up */
        a = b = c = d = e = f = g = h = T1 = T2 = 0;
}

#endif /* SHA2_UNROLL_TRANSFORM */

void eso_sha512_update(es_sha512_ctx_t* context, const sha2_byte *data, es_size_t len) {
        unsigned int    freespace, usedspace;

        if (len == 0) {
                /* Calling with no data is valid - we do nothing */
                return;
        }

        /* Sanity check: */
        ES_ASSERT(context != (es_sha512_ctx_t*)0 && data != (sha2_byte*)0);

        usedspace = (unsigned int)((context->bitcount[0] >> 3) 
                                 % SHA512_BLOCK_LENGTH);
        if (usedspace > 0) {
                /* Calculate how much free space is available in the buffer */
                freespace = SHA512_BLOCK_LENGTH - usedspace;

                if (len >= freespace) {
                        /* Fill the buffer completely and process it */
                        MEMCPY_BCOPY(&context->buffer[usedspace], data, freespace);
                        ADDINC128(context->bitcount, freespace << 3);
                        len -= freespace;
                        data += freespace;
                        SHA512_Transform(context, (sha2_word64*)context->buffer);
                } else {
                        /* The buffer is not yet full */
                        MEMCPY_BCOPY(&context->buffer[usedspace], data, len);
                        ADDINC128(context->bitcount, len << 3);
                        /* Clean up: */
                        usedspace = freespace = 0;
                        return;
                }
        }
        while (len >= SHA512_BLOCK_LENGTH) {
                /* Process as many complete blocks as we can */
                SHA512_Transform(context, (sha2_word64*)data);
                ADDINC128(context->bitcount, SHA512_BLOCK_LENGTH << 3);
                len -= SHA512_BLOCK_LENGTH;
                data += SHA512_BLOCK_LENGTH;
        }
        if (len > 0) {
                /* There's left-overs, so save 'em */
                MEMCPY_BCOPY(context->buffer, data, len);
                ADDINC128(context->bitcount, len << 3);
        }
        /* Clean up: */
        usedspace = freespace = 0;
}

void SHA512_Last(es_sha512_ctx_t* context) {
        unsigned int    usedspace;

        usedspace = (unsigned int)((context->bitcount[0] >> 3) 
                                 % SHA512_BLOCK_LENGTH);
#ifndef __BIG_ENDIAN__
        /* Convert FROM host byte order */
        REVERSE64(context->bitcount[0],context->bitcount[0]);
        REVERSE64(context->bitcount[1],context->bitcount[1]);
#endif
        if (usedspace > 0) {
                /* Begin padding with a 1 bit: */
                context->buffer[usedspace++] = 0x80;

                if (usedspace <= SHA512_SHORT_BLOCK_LENGTH) {
                        /* Set-up for the last transform: */
                        MEMSET_BZERO(&context->buffer[usedspace], SHA512_SHORT_BLOCK_LENGTH - usedspace);
                } else {
                        if (usedspace < SHA512_BLOCK_LENGTH) {
                                MEMSET_BZERO(&context->buffer[usedspace], SHA512_BLOCK_LENGTH - usedspace);
                        }
                        /* Do second-to-last transform: */
                        SHA512_Transform(context, (sha2_word64*)context->buffer);

                        /* And set-up for the last transform: */
                        MEMSET_BZERO(context->buffer, SHA512_BLOCK_LENGTH - 2);
                }
        } else {
                /* Prepare for final transform: */
                MEMSET_BZERO(context->buffer, SHA512_SHORT_BLOCK_LENGTH);

                /* Begin padding with a 1 bit: */
                *context->buffer = 0x80;
        }
        /* Store the length of input data (in bits): */
        *(sha2_word64*)&context->buffer[SHA512_SHORT_BLOCK_LENGTH] = context->bitcount[1];
        *(sha2_word64*)&context->buffer[SHA512_SHORT_BLOCK_LENGTH+8] = context->bitcount[0];

        /* Final transform: */
        SHA512_Transform(context, (sha2_word64*)context->buffer);
}

void eso_sha512_final(sha2_byte digest[], es_sha512_ctx_t* context) {
        sha2_word64     *d = (sha2_word64*)digest;

        /* Sanity check: */
        ES_ASSERT(context != (es_sha512_ctx_t*)0);

        /* If no digest buffer is passed, we don't bother doing this: */
        if (digest != (sha2_byte*)0) {
                SHA512_Last(context);

                /* Save the hash data for output: */
#ifndef __BIG_ENDIAN__
                {
                        /* Convert TO host byte order */
                        int     j;
                        for (j = 0; j < 8; j++) {
                                REVERSE64(context->state[j],context->state[j]);
                                *d++ = context->state[j];
                        }
                }
#else /* __BIG_ENDIAN__ */
                MEMCPY_BCOPY(d, context->state, SHA512_DIGEST_LENGTH);
#endif /* __BIG_ENDIAN__ */
        }

        /* Zero out state data */
        MEMSET_BZERO(context, sizeof(context));
}

char *eso_sha512_end(es_sha512_ctx_t* context, char buffer[]) {
        sha2_byte       digest[SHA512_DIGEST_LENGTH], *d = digest;
        int             i;

        /* Sanity check: */
        ES_ASSERT(context != (es_sha512_ctx_t*)0);

        if (buffer != (char*)0) {
                eso_sha512_final(digest, context);

                for (i = 0; i < SHA512_DIGEST_LENGTH; i++) {
                        *buffer++ = sha2_hex_digits[(*d & 0xf0) >> 4];
                        *buffer++ = sha2_hex_digits[*d & 0x0f];
                        d++;
                }
                *buffer = (char)0;
        } else {
                MEMSET_BZERO(context, sizeof(context));
        }
        MEMSET_BZERO(digest, SHA512_DIGEST_LENGTH);
        return buffer;
}

char* eso_sha512_data(const sha2_byte* data, es_size_t len, char digest[SHA512_DIGEST_STRING_LENGTH]) {
        es_sha512_ctx_t      context;

        eso_sha512_init(&context);
        eso_sha512_update(&context, data, len);
        return eso_sha512_end(&context, digest);
}


/*** SHA-384: *********************************************************/
void eso_sha384_init(es_sha384_ctx_t* context) {
        if (context == (es_sha384_ctx_t*)0) {
                return;
        }
        MEMCPY_BCOPY(context->state, sha384_initial_hash_value, SHA512_DIGEST_LENGTH);
        MEMSET_BZERO(context->buffer, SHA384_BLOCK_LENGTH);
        context->bitcount[0] = context->bitcount[1] = 0;
}

void eso_sha384_update(es_sha384_ctx_t* context, const sha2_byte* data, es_size_t len) {
        eso_sha512_update((es_sha512_ctx_t*)context, data, len);
}

void eso_sha384_final(sha2_byte digest[], es_sha384_ctx_t* context) {
        sha2_word64     *d = (sha2_word64*)digest;

        /* Sanity check: */
        ES_ASSERT(context != (es_sha384_ctx_t*)0);

        /* If no digest buffer is passed, we don't bother doing this: */
        if (digest != (sha2_byte*)0) {
                SHA512_Last((es_sha512_ctx_t*)context);

                /* Save the hash data for output: */
#ifndef __BIG_ENDIAN__
                {
                        /* Convert TO host byte order */
                        int     j;
                        for (j = 0; j < 6; j++) {
                                REVERSE64(context->state[j],context->state[j]);
                                *d++ = context->state[j];
                        }
                }
#else /* __BIG_ENDIAN__ */
                MEMCPY_BCOPY(d, context->state, SHA384_DIGEST_LENGTH);
#endif /* __BIG_ENDIAN__ */
        }

        /* Zero out state data */
        MEMSET_BZERO(context, sizeof(context));
}

char *eso_sha384_end(es_sha384_ctx_t* context, char buffer[]) {
        sha2_byte       digest[SHA384_DIGEST_LENGTH], *d = digest;
        int             i;

        /* Sanity check: */
        ES_ASSERT(context != (es_sha384_ctx_t*)0);

        if (buffer != (char*)0) {
                eso_sha384_final(digest, context);

                for (i = 0; i < SHA384_DIGEST_LENGTH; i++) {
                        *buffer++ = sha2_hex_digits[(*d & 0xf0) >> 4];
                        *buffer++ = sha2_hex_digits[*d & 0x0f];
                        d++;
                }
                *buffer = (char)0;
        } else {
                MEMSET_BZERO(context, sizeof(context));
        }
        MEMSET_BZERO(digest, SHA384_DIGEST_LENGTH);
        return buffer;
}

char* eso_sha384_data(const sha2_byte* data, es_size_t len, char digest[SHA384_DIGEST_STRING_LENGTH]) {
        es_sha384_ctx_t      context;

        eso_sha384_init(&context);
        eso_sha384_update(&context, data, len);
        return eso_sha384_end(&context, digest);
}
