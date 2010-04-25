/* md5.c
 * Copyright (C) 2000-2010 Farooq Mela. All rights reserved.
 *
 * An implementation of the Message Digest 5 algorithm. See RFC1321 for
 * algorithm details.
 *
 * $Id$
 */

#include "md5.h"

#include <string.h>

#include "mp.h"
#include "mp_defs.h"

static void md5_step(unsigned int *md5, const unsigned char *block);
static void md5_encode(unsigned char *output, const unsigned int *input, unsigned len);
static void md5_decode(unsigned int *output, const unsigned char *input, unsigned len);

static const unsigned char padding[64] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const unsigned int T[64] = {
	0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
	0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
	0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
	0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
	0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
	0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
	0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
	0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
	0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
	0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
	0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
	0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
	0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
	0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
	0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
	0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

static const unsigned int A[4] = { 7, 12, 17, 22 };
static const unsigned int B[4] = { 5,  9, 14, 20 };
static const unsigned int C[4] = { 4, 11, 16, 23 };
static const unsigned int D[4] = { 6, 10, 15, 21 };

#define MD5_MAGIC0		0x67452301
#define MD5_MAGIC1		0xefcdab89
#define MD5_MAGIC2		0x98badcfe
#define MD5_MAGIC3		0x10325476

#define CALC1(x,y,z)	(((x) & (y)) | ((~x) & (z)))
#define CALC2(x,y,z)	(((x) & (z)) | ((y) & (~z)))
#define CALC3(x,y,z)	((x) ^ (y) ^ (z))
#define CALC4(x,y,z)	((y) ^ ((x) | (~z)))

#define ROT_L(x,n)		(((x) << (n)) | ((x) >> ((sizeof(x) << 3) - (n))))
#define ROT_R(x,n)		(((x) >> (n)) | ((x) << ((sizeof(x) << 3) - (n))))

#define RND1(a,b,c,d,r,s,t) {						\
	(a) += CALC1((b),(c),(d))+x[(r)]+T[(t)];		\
	(a) = ROT_L((a),A[(s)]);						\
	(a) += (b);										}

#define RND2(a,b,c,d,r,s,t) {						\
	(a) += CALC2((b),(c),(d))+x[(r)]+T[(t)];		\
	(a) = ROT_L((a),B[(s)]);						\
	(a) += (b);										}

#define RND3(a,b,c,d,r,s,t) {						\
	(a) += CALC3((b),(c),(d))+x[(r)]+T[(t)];		\
	(a) = ROT_L((a),C[(s)]);						\
	(a) += (b);										}

#define RND4(a,b,c,d,r,s,t) {						\
	(a) += CALC4((b),(c),(d))+x[(r)]+T[(t)];		\
	(a) = ROT_L((a),D[(s)]);						\
	(a) += (b);										}

void
md5_init(md5_context *ctx)
{
	ASSERT(ctx != NULL);

	ctx->len[0] = 0;
	ctx->len[1] = 0;
	ctx->md5[0] = MD5_MAGIC0;
	ctx->md5[1] = MD5_MAGIC1;
	ctx->md5[2] = MD5_MAGIC2;
	ctx->md5[3] = MD5_MAGIC3;
}

void
md5_update(md5_context *ctx, const void *data, unsigned len)
{
	const unsigned char *input;
	unsigned i, idx, plen;

	ASSERT(ctx != NULL);
	ASSERT(data != NULL);
	ASSERT(len > 0);

	input = (const unsigned char *)data;

	/* # of bytes, % 64 */
	idx = (ctx->len[0] >> 3) & 0x3F;

	/* number of bits */
	ctx->len[0] += (unsigned int)len << 3;
	if (ctx->len[0] < ((unsigned int)len << 3)) /* overflow */
		ctx->len[1]++;
	ctx->len[1] += (unsigned int)len >> 29; /* 29 b/c want bits not bytes */
	plen = 64 - idx;
	if (len >= plen) {
		// Do the calculation, 64 bytes at a time.
		memcpy(&ctx->buf[idx], input, plen);
		md5_step(ctx->md5, ctx->buf);
		for (i = plen; i + 63 < len; i += 64)
			md5_step(ctx->md5, &input[i]);
		idx = 0;
	} else {
		i = 0;
	}

	/* buffer rest */
	memcpy(&ctx->buf[idx], &input[i], len - i);
}

void
md5_final(md5_context *ctx, void *digest)
{
	unsigned char bits[8];
	unsigned idx, plen;

	ASSERT(ctx != NULL);
	ASSERT(digest != NULL);

	/* # of bits */
	md5_encode(bits, ctx->len, 8);

	/*
	 * Pad out to 56 mod 64.
	 * Shr by 3 to get back to bytes.
	 */
	idx = (unsigned)((ctx->len[0] >> 3) & 0x3f);
	plen = (idx < 56) ? (56 - idx) : (120 - idx);
	md5_update(ctx, padding, plen);

	/* add length */
	md5_update(ctx, bits, 8);

	/* md5 */
	md5_encode((unsigned char *)digest, ctx->md5, 16);

	/* zero possibly sensitive data */
	memset(ctx, 0, sizeof(md5_context));
}

static void
md5_step(unsigned int *md5, const unsigned char *block)
{
	unsigned int a, b, c, d, x[16];

	ASSERT(md5 != NULL);
	ASSERT(block != NULL);

	a = md5[0];
	b = md5[1];
	c = md5[2];
	d = md5[3];

	md5_decode(x, block, 64);

	/*
	 * Like to see something interesting? ;)
	 * cpp FX_MD5.C | more
	 */
	RND1(a,b,c,d, 0, 0, 0);
	RND1(d,a,b,c, 1, 1, 1);
	RND1(c,d,a,b, 2, 2, 2);
	RND1(b,c,d,a, 3, 3, 3);
	RND1(a,b,c,d, 4, 0, 4);
	RND1(d,a,b,c, 5, 1, 5);
	RND1(c,d,a,b, 6, 2, 6);
	RND1(b,c,d,a, 7, 3, 7);
	RND1(a,b,c,d, 8, 0, 8);
	RND1(d,a,b,c, 9, 1, 9);
	RND1(c,d,a,b,10, 2,10);
	RND1(b,c,d,a,11, 3,11);
	RND1(a,b,c,d,12, 0,12);
	RND1(d,a,b,c,13, 1,13);
	RND1(c,d,a,b,14, 2,14);
	RND1(b,c,d,a,15, 3,15);

	RND2(a,b,c,d, 1, 0,16);
	RND2(d,a,b,c, 6, 1,17);
	RND2(c,d,a,b,11, 2,18);
	RND2(b,c,d,a, 0, 3,19);
	RND2(a,b,c,d, 5, 0,20);
	RND2(d,a,b,c,10, 1,21);
	RND2(c,d,a,b,15, 2,22);
	RND2(b,c,d,a, 4, 3,23);
	RND2(a,b,c,d, 9, 0,24);
	RND2(d,a,b,c,14, 1,25);
	RND2(c,d,a,b, 3, 2,26);
	RND2(b,c,d,a, 8, 3,27);
	RND2(a,b,c,d,13, 0,28);
	RND2(d,a,b,c, 2, 1,29);
	RND2(c,d,a,b, 7, 2,30);
	RND2(b,c,d,a,12, 3,31);

	RND3(a,b,c,d, 5, 0,32);
	RND3(d,a,b,c, 8, 1,33);
	RND3(c,d,a,b,11, 2,34);
	RND3(b,c,d,a,14, 3,35);
	RND3(a,b,c,d, 1, 0,36);
	RND3(d,a,b,c, 4, 1,37);
	RND3(c,d,a,b, 7, 2,38);
	RND3(b,c,d,a,10, 3,39);
	RND3(a,b,c,d,13, 0,40);
	RND3(d,a,b,c, 0, 1,41);
	RND3(c,d,a,b, 3, 2,42);
	RND3(b,c,d,a, 6, 3,43);
	RND3(a,b,c,d, 9, 0,44);
	RND3(d,a,b,c,12, 1,45);
	RND3(c,d,a,b,15, 2,46);
	RND3(b,c,d,a, 2, 3,47);

	RND4(a,b,c,d, 0, 0,48);
	RND4(d,a,b,c, 7, 1,49);
	RND4(c,d,a,b, 14,2,50);
	RND4(b,c,d,a, 5, 3,51);
	RND4(a,b,c,d, 12,0,52);
	RND4(d,a,b,c, 3, 1,53);
	RND4(c,d,a,b, 10,2,54);
	RND4(b,c,d,a, 1, 3,55);
	RND4(a,b,c,d, 8, 0,56);
	RND4(d,a,b,c, 15,1,57);
	RND4(c,d,a,b, 6, 2,58);
	RND4(b,c,d,a, 13,3,59);
	RND4(a,b,c,d, 4, 0,60);
	RND4(d,a,b,c, 11,1,61);
	RND4(c,d,a,b, 2, 2,62);
	RND4(b,c,d,a, 9, 3,63);

	md5[0] += a;
	md5[1] += b;
	md5[2] += c;
	md5[3] += d;

	/* zero possibly sensitive data on stack.. not that im worried ;-) */
	memset(x, 0, sizeof(x));
}

static void
md5_encode(unsigned char *output, const unsigned int *input, unsigned len)
{
	unsigned i, j;

	ASSERT(output != NULL);
	ASSERT(input != NULL);
	ASSERT(len > 0);

	for (i = 0, j = 0; j < len; i++, j += 4) {
		output[j + 0] = (unsigned char)((input[i] >>  0) & 0xff);
		output[j + 1] = (unsigned char)((input[i] >>  8) & 0xff);
		output[j + 2] = (unsigned char)((input[i] >> 16) & 0xff);
		output[j + 3] = (unsigned char)((input[i] >> 24) & 0xff);
	}
}

static void
md5_decode(unsigned int *output, const unsigned char *input, unsigned len)
{
	unsigned i, j;

	ASSERT(output != NULL);
	ASSERT(input != NULL);
	ASSERT(len > 0);

	for (i = 0, j = 0; j < len; i++, j += 4)
		output[i] =
			(((unsigned int)input[j + 0]) <<  0) |
			(((unsigned int)input[j + 1]) <<  8) |
			(((unsigned int)input[j + 2]) << 16) |
			(((unsigned int)input[j + 3]) << 24);
}

void
md5_hash(const void *input, unsigned len, void *digest)
{
	md5_context ctx;

	ASSERT(input != NULL);
	ASSERT(len > 0);
	ASSERT(digest != NULL);

	md5_init(&ctx);
	md5_update(&ctx, input, len);
	md5_final(&ctx, digest);
}
