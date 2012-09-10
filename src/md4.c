/* md4.c
 * Copyright (C) 2000-2012 Farooq Mela. All rights reserved.
 *
 * An implementation of Message Digest 4, as described in RFC1320. */

#include "md4.h"

#include <string.h>

#include "mp.h"
#include "mp_internal.h"

static void md4_step(const uint8_t *block, uint32_t *md4);
static void md4_encode(const uint32_t *input, unsigned len, uint8_t *output);
static void md4_decode(const uint8_t *input, unsigned len, uint32_t *output);

static const uint8_t padding[64] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const uint32_t A[4] = { 3, 7, 11, 19 };
static const uint32_t B[4] = { 3, 5,  9, 13 };
static const uint32_t C[4] = { 3, 9, 11, 15 };

#define MD4_MAGIC0		0x67452301
#define MD4_MAGIC1		0xefcdab89
#define MD4_MAGIC2		0x98badcfe
#define MD4_MAGIC3		0x10325476

#define CALC1(x,y,z)	(((x) & (y)) | ((~x) & (z)))
#define CALC2(x,y,z)	(((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define CALC3(x,y,z)	((x) ^ (y) ^ (z))

#define ROT_L(x,n)		(((x) << (n)) | ((x) >> ((sizeof(x) << 3) - (n))))
#define ROT_R(x,n)		(((x) >> (n)) | ((x) << ((sizeof(x) << 3) - (n))))

#define RND1(a,b,c,d,r,s) {								\
	(a) += CALC1((b), (c), (d)) + x[(r)];				\
	(a) = ROT_L((a), A[(s) - 1]);						}

#define RND2(a,b,c,d,r,s) {								\
	(a) += CALC2((b), (c), (d)) + x[(r)] + 0x5a827999;	\
	(a) = ROT_L((a), B[(s) - 1]);						}

#define RND3(a,b,c,d,r,s) {								\
	(a) += CALC3((b), (c), (d)) + x[(r)] + 0x6ed9eba1;	\
	(a) = ROT_L((a), C[(s) - 1]);						}

void
md4_init(md4_context *ctx)
{
	ASSERT(ctx != NULL);

	ctx->len[0] = ctx->len[1] = 0;
	ctx->md4[0] = MD4_MAGIC0;
	ctx->md4[1] = MD4_MAGIC1;
	ctx->md4[2] = MD4_MAGIC2;
	ctx->md4[3] = MD4_MAGIC3;
}

void
md4_update(md4_context *ctx, const void *data, unsigned len)
{
	ASSERT(ctx != NULL);
	ASSERT(data != NULL);
	ASSERT(len > 0);


	/* # of bytes, % 64 */
	unsigned idx = (ctx->len[0] >> 3) & 0x3f;

	/* number of bits */
	ctx->len[1] += ((ctx->len[0] += len << 3) < (len << 3));
	ctx->len[1] += len >> 29;
	unsigned plen = 64 - idx;
	const uint8_t *input = data;
	unsigned i = 0;
	if (len >= plen) {
		memcpy(&ctx->buf[idx], input, plen);
		md4_step(ctx->buf, ctx->md4);
		for (i = plen; i + 63 < len; i += 64)
			md4_step(&input[i], ctx->md4);
		idx = 0;
	}

	/* buffer rest */
	memcpy(&ctx->buf[idx], &input[i], len - i);
}

void
md4_final(md4_context *ctx, void *digest)
{
	ASSERT(ctx != NULL);
	ASSERT(digest != NULL);

	/* # of bits */
	uint8_t bits[8];
	md4_encode(ctx->len, 8, bits);

	/* Pad out to 56 mod 64. */
	unsigned idx = (ctx->len[0] >> 3) & 0x3f;
	unsigned plen = (idx < 56) ? (56 - idx) : (120 - idx);
	md4_update(ctx, padding, plen);

	/* add length */
	md4_update(ctx, bits, 8);

	/* md4 */
	md4_encode(ctx->md4, 16, digest);

	memset(ctx, 0, sizeof(md4_context));
}

static void
md4_step(const uint8_t *block, uint32_t *md4)
{
	ASSERT(md4 != NULL);
	ASSERT(block != NULL);

	uint32_t a = md4[0];
	uint32_t b = md4[1];
	uint32_t c = md4[2];
	uint32_t d = md4[3];

	uint32_t x[16];
	md4_decode(block, 64, x);

	RND1(a, b, c, d,  0, 1);
	RND1(d, a, b, c,  1, 2);
	RND1(c, d, a, b,  2, 3);
	RND1(b, c, d, a,  3, 4);
	RND1(a, b, c, d,  4, 1);
	RND1(d, a, b, c,  5, 2);
	RND1(c, d, a, b,  6, 3);
	RND1(b, c, d, a,  7, 4);
	RND1(a, b, c, d,  8, 1);
	RND1(d, a, b, c,  9, 2);
	RND1(c, d, a, b, 10, 3);
	RND1(b, c, d, a, 11, 4);
	RND1(a, b, c, d, 12, 1);
	RND1(d, a, b, c, 13, 2);
	RND1(c, d, a, b, 14, 3);
	RND1(b, c, d, a, 15, 4);

	RND2(a, b, c, d,  0, 1);
	RND2(d, a, b, c,  4, 2);
	RND2(c, d, a, b,  8, 3);
	RND2(b, c, d, a, 12, 4);
	RND2(a, b, c, d,  1, 1);
	RND2(d, a, b, c,  5, 2);
	RND2(c, d, a, b,  9, 3);
	RND2(b, c, d, a, 13, 4);
	RND2(a, b, c, d,  2, 1);
	RND2(d, a, b, c,  6, 2);
	RND2(c, d, a, b, 10, 3);
	RND2(b, c, d, a, 14, 4);
	RND2(a, b, c, d,  3, 1);
	RND2(d, a, b, c,  7, 2);
	RND2(c, d, a, b, 11, 3);
	RND2(b, c, d, a, 15, 4);

	RND3(a, b, c, d,  0, 1);
	RND3(d, a, b, c,  8, 2);
	RND3(c, d, a, b,  4, 3);
	RND3(b, c, d, a, 12, 4);
	RND3(a, b, c, d,  2, 1);
	RND3(d, a, b, c, 10, 2);
	RND3(c, d, a, b,  6, 3);
	RND3(b, c, d, a, 14, 4);
	RND3(a, b, c, d,  1, 1);
	RND3(d, a, b, c,  9, 2);
	RND3(c, d, a, b,  5, 3);
	RND3(b, c, d, a, 13, 4);
	RND3(a, b, c, d,  3, 1);
	RND3(d, a, b, c, 11, 2);
	RND3(c, d, a, b,  7, 3);
	RND3(b, c, d, a, 15, 4);

	md4[0] += a;
	md4[1] += b;
	md4[2] += c;
	md4[3] += d;

	memset(x, 0, sizeof(x));
}

static void
md4_encode(const uint32_t *input, unsigned len, uint8_t *output)
{
	ASSERT(output != NULL);
	ASSERT(input != NULL);
	ASSERT(len > 0);

	for (unsigned i = 0, j = 0; j < len; i++, j += 4) {
		output[j + 0] = (uint8_t)((input[i] >>  0) & 0xff);
		output[j + 1] = (uint8_t)((input[i] >>  8) & 0xff);
		output[j + 2] = (uint8_t)((input[i] >> 16) & 0xff);
		output[j + 3] = (uint8_t)((input[i] >> 24) & 0xff);
	}
}

static void
md4_decode(const uint8_t *input, unsigned len, uint32_t *output)
{
	ASSERT(output != NULL);
	ASSERT(input != NULL);
	ASSERT(len > 0);

	for (unsigned i = 0, j = 0; j < len; i++, j += 4)
		output[i] =
			(((uint32_t)input[j + 0]) <<  0) |
			(((uint32_t)input[j + 1]) <<  8) |
			(((uint32_t)input[j + 2]) << 16) |
			(((uint32_t)input[j + 3]) << 24);
}


void
md4_hash(const void *input, unsigned len, void *digest)
{
	ASSERT(input != NULL);
	ASSERT(len > 0);
	ASSERT(digest != NULL);

	md4_context ctx;
	md4_init(&ctx);
	md4_update(&ctx, input, len);
	md4_final(&ctx, digest);
	memset(&ctx, 0, sizeof(ctx));
}
