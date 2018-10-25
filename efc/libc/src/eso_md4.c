/**
 * @file  eso_md4.c
 * @brief ES RSA Data Security
 */

/*
 * md4c.c	MD4 message-digest algorithm
 *
 * Version:	$Id: md4.c,v 1.5.2.2 2004/12/19 00:47:43 phampson Exp $
 *
 * This file is licensed under the LGPL, but is largely derived
 * from public domain source code.
 */

#include "eso_md4.h"
#include "eso_libc.h"

#define S11 3
#define S12 7
#define S13 11
#define S14 19
#define S21 3
#define S22 5
#define S23 9
#define S24 13
#define S31 3
#define S32 9
#define S33 11
#define S34 15

static const unsigned char PADDING[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// F, G and I are basic MD4 functions.
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))

// ROTATE_LEFT rotates x left n bits.
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

// FF, GG and HH are transformations for rounds 1, 2 and 3 */
// Rotation is separate from addition to prevent recomputation */
#define FF(a, b, c, d, x, s) { \
    (a) += F ((b), (c), (d)) + (x); \
    (a) = ROTATE_LEFT ((a), (s)); \
}
#define GG(a, b, c, d, x, s) { \
    (a) += G ((b), (c), (d)) + (x) + (es_uint32_t)0x5a827999; \
    (a) = ROTATE_LEFT ((a), (s)); \
}
#define HH(a, b, c, d, x, s) { \
    (a) += H ((b), (c), (d)) + (x) + (es_uint32_t)0x6ed9eba1; \
    (a) = ROTATE_LEFT ((a), (s)); \
}


static void decode(unsigned int* output, const unsigned char* input, unsigned int length) {
	unsigned int i, j;

	for (i = 0, j = 0; j < length; i++, j += 4)
		output[i] = ((unsigned int)input[j])             |
					(((unsigned int)input[j + 1]) << 8)  |
					(((unsigned int)input[j + 2]) << 16) |
					(((unsigned int)input[j + 3]) << 24);
}

static void encode(unsigned char* output, const unsigned int* input, unsigned int length) {
	unsigned int i, j;
	unsigned int k;

	for (i = 0, j = 0; j < length; i++, j += 4) {
		k = input[i];
		output[j]     = (unsigned char)(k & 0xff);
		output[j + 1] = (unsigned char)((k >> 8) & 0xff);
		output[j + 2] = (unsigned char)((k >> 16) & 0xff);
		output[j + 3] = (unsigned char)((k >> 24) & 0xff);
	}
}

// MD4 basic transformation. Transforms state based on block.
static void MD4Transform(unsigned int state[4], const unsigned char block[64]) {
	unsigned int a = state[0];
	unsigned int b = state[1];
	unsigned int c = state[2];
	unsigned int d = state[3];
	unsigned int x[MD4_DIGEST_LENGTH];

	decode(x, block, 64);

	/* Round 1 */
	FF (a, b, c, d, x[ 0], S11); /* 1 */
	FF (d, a, b, c, x[ 1], S12); /* 2 */
	FF (c, d, a, b, x[ 2], S13); /* 3 */
	FF (b, c, d, a, x[ 3], S14); /* 4 */
	FF (a, b, c, d, x[ 4], S11); /* 5 */
	FF (d, a, b, c, x[ 5], S12); /* 6 */
	FF (c, d, a, b, x[ 6], S13); /* 7 */
	FF (b, c, d, a, x[ 7], S14); /* 8 */
	FF (a, b, c, d, x[ 8], S11); /* 9 */
	FF (d, a, b, c, x[ 9], S12); /* 10 */
	FF (c, d, a, b, x[10], S13); /* 11 */
	FF (b, c, d, a, x[11], S14); /* 12 */
	FF (a, b, c, d, x[12], S11); /* 13 */
	FF (d, a, b, c, x[13], S12); /* 14 */
	FF (c, d, a, b, x[14], S13); /* 15 */
	FF (b, c, d, a, x[15], S14); /* 16 */

	/* Round 2 */
	GG (a, b, c, d, x[ 0], S21); /* 17 */
	GG (d, a, b, c, x[ 4], S22); /* 18 */
	GG (c, d, a, b, x[ 8], S23); /* 19 */
	GG (b, c, d, a, x[12], S24); /* 20 */
	GG (a, b, c, d, x[ 1], S21); /* 21 */
	GG (d, a, b, c, x[ 5], S22); /* 22 */
	GG (c, d, a, b, x[ 9], S23); /* 23 */
	GG (b, c, d, a, x[13], S24); /* 24 */
	GG (a, b, c, d, x[ 2], S21); /* 25 */
	GG (d, a, b, c, x[ 6], S22); /* 26 */
	GG (c, d, a, b, x[10], S23); /* 27 */
	GG (b, c, d, a, x[14], S24); /* 28 */
	GG (a, b, c, d, x[ 3], S21); /* 29 */
	GG (d, a, b, c, x[ 7], S22); /* 30 */
	GG (c, d, a, b, x[11], S23); /* 31 */
	GG (b, c, d, a, x[15], S24); /* 32 */

	/* Round 3 */
	HH (a, b, c, d, x[ 0], S31); /* 33 */
	HH (d, a, b, c, x[ 8], S32); /* 34 */
	HH (c, d, a, b, x[ 4], S33); /* 35 */
	HH (b, c, d, a, x[12], S34); /* 36 */
	HH (a, b, c, d, x[ 2], S31); /* 37 */
	HH (d, a, b, c, x[10], S32); /* 38 */
	HH (c, d, a, b, x[ 6], S33); /* 39 */
	HH (b, c, d, a, x[14], S34); /* 40 */
	HH (a, b, c, d, x[ 1], S31); /* 41 */
	HH (d, a, b, c, x[ 9], S32); /* 42 */
	HH (c, d, a, b, x[ 5], S33); /* 43 */
	HH (b, c, d, a, x[13], S34); /* 44 */
	HH (a, b, c, d, x[ 3], S31); /* 45 */
	HH (d, a, b, c, x[11], S32); /* 46 */
	HH (c, d, a, b, x[ 7], S33); /* 47 */
	HH (b, c, d, a, x[15], S34); /* 48 */

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;

	// clear any sensitive information.
	eso_memset(x, 0, sizeof(x));
}

static void reset(MD4_CTX *ctx) {
	ctx->count[0] = 0;
	ctx->count[1] = 0;
	ctx->state[0] = 0x67452301;
	ctx->state[1] = 0xefcdab89;
	ctx->state[2] = 0x98badcfe;
	ctx->state[3] = 0x10325476;

	eso_memset(ctx->buffer, 0, 64);
}

/*
 * Start MD4 accumulation.
 * Set bit count to 0 and buffer to mysterious initialization constants.
 */
void
MD4_Init(MD4_CTX *ctx)
{
	reset(ctx);
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
void
MD4_Update(MD4_CTX *ctx, const es_uint8_t *vinput, es_uint32_t inputLen)
{
	const unsigned char *input = vinput;
	int i;
	int idx;
	int partLen;

	// Compute number of bytes mod 64
	idx = (unsigned int)((ctx->count[0] >> 3) & 0x3F);

	// Update number of bits
	if ((ctx->count[0] += ((unsigned int)inputLen << 3)) < ((unsigned int)inputLen << 3)) {
		ctx->count[1]++;
	}

	ctx->count[1] += (unsigned int)inputLen >> 29;

	partLen = 64 - idx;

	// Transform as many times as possible.
	if (inputLen >= partLen) {
		eso_memcpy(&ctx->buffer[idx], input, partLen);
		MD4Transform(ctx->state, ctx->buffer);

		for (i = partLen; i + 63 < inputLen; i += 64) {
			MD4Transform(ctx->state, &input[i]);
		}

		idx = 0;
	} else {
		i = 0;
	}

	// Buffer remaining input
	eso_memcpy(&ctx->buffer[idx], &input[i], inputLen - i);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern 
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
void
MD4_Final(unsigned char digest[MD4_DIGEST_LENGTH], MD4_CTX *ctx)
{
	unsigned char bits[8];
	unsigned int idx;
	unsigned int padLen;

	// Save number of bits
	encode(bits, ctx->count, 8);

	// Pad out to 56 mod 64.
	idx = (unsigned int)((ctx->count[0] >> 3) & 0x3f);
	padLen = (idx < 56) ? (56 - idx) : (120 - idx);
	MD4_Update(ctx, PADDING, padLen);

	// Append length (before padding)
	MD4_Update(ctx, bits, 8);

	// Store state in digest
	encode(digest, ctx->state, MD4_DIGEST_LENGTH);

	reset(ctx);
}
