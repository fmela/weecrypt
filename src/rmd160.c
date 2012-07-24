#include "rmd160.h"

#include "mp.h"
#include "mp_defs.h"

#include <string.h> /* for memset */

static void rmd160_step(rmd160_context *ctx, const void *source);

void
rmd160_init(rmd160_context *ctx)
{
	ASSERT(ctx != NULL);

	memset(ctx, 0, sizeof(*ctx));

	/* Magic numbers. */
	ctx->rmd[0] = 0x67452301U;
	ctx->rmd[1] = 0xefcdab89U;
	ctx->rmd[2] = 0x98badcfeU;
	ctx->rmd[3] = 0x10325476U;
	ctx->rmd[4] = 0xc3d2e1f0U;

	/* Buffer is empty. */
	ctx->nbuf = 0;
}

void
rmd160_update(rmd160_context *ctx, const void *data, unsigned len)
{
	const mp8_t *source = data;

	ASSERT(ctx != NULL);
	ASSERT(data != NULL);
	ASSERT(len > 0);

	/* Check to see if we already have some data buffered. */
	if (ctx->nbuf) {
		/* Add data to the end of the buffer (if possible). */
		unsigned nleft = sizeof(ctx->buf) - ctx->nbuf;
		if (nleft > len) {
			/* We don't have enough data to make 64 bytes. Just buffer it. */
			memcpy(ctx->buf + ctx->nbuf, source, len);
			ctx->nbuf += len;
			return;
		}

		/* We can fill all 64 bytes. */
		memcpy(ctx->buf + ctx->nbuf, source, nleft);
		source += nleft;
		len -= nleft;

		/* Process those 64 bytes. */
		rmd160_step(ctx, ctx->buf);

		/* Set the buffered data size back to zero. */
		ctx->nbuf = 0;
	}

	/* Process data 64 bytes at a time. */
	while (len >= 64) {
		rmd160_step(ctx, source);
		source += 64;
		len -= 64;
	}

	/* Buffer any remaining data. */
	ASSERT(len < 64);
	if (len) {
		memcpy(ctx->buf, source, len);
		ctx->nbuf = len;
	}
}

void
rmd160_final(rmd160_context *ctx, void *digest)
{
	ASSERT(ctx != NULL);
	ASSERT(digest != NULL);
}

void
rmd160_hash(const void *input, unsigned len, void *digest) // digest must be 20 bytes
{
	ASSERT(input != NULL);
	ASSERT(len > 0);
	ASSERT(digest != NULL);
}

/* ROL(x, n) cyclically rotates x over n bits to the left.
 * x must be of an unsigned 32 bits type and 0 <= n < 32. */
#define ROL(x, n)        (((x) << (n)) | ((x) >> (32-(n))))

/* The five basic functions F(), G() and H() */
#define F(x, y, z)        ((x) ^ (y) ^ (z))
#define G(x, y, z)        (((x) & (y)) | (~(x) & (z)))
#define H(x, y, z)        (((x) | ~(y)) ^ (z))
#define I(x, y, z)        (((x) & (z)) | ((y) & ~(z)))
#define J(x, y, z)        ((x) ^ ((y) | ~(z)))

/* The ten basic operations FF() through III() */
#define FF(a, b, c, d, e, x, s) \
	      (a) += F((b), (c), (d)) + (x); \
	      (a) = ROL((a), (s)) + (e); \
	      (c) = ROL((c), 10);

#define GG(a, b, c, d, e, x, s) \
	      (a) += G((b), (c), (d)) + (x) + 0x5a827999UL; \
	      (a) = ROL((a), (s)) + (e); \
	      (c) = ROL((c), 10);

#define HH(a, b, c, d, e, x, s) \
	      (a) += H((b), (c), (d)) + (x) + 0x6ed9eba1UL; \
	      (a) = ROL((a), (s)) + (e); \
	      (c) = ROL((c), 10)

#define II(a, b, c, d, e, x, s) \
	      (a) += I((b), (c), (d)) + (x) + 0x8f1bbcdcUL; \
	      (a) = ROL((a), (s)) + (e); \
	      (c) = ROL((c), 10);

#define JJ(a, b, c, d, e, x, s) \
	      (a) += J((b), (c), (d)) + (x) + 0xa953fd4eUL; \
	      (a) = ROL((a), (s)) + (e); \
	      (c) = ROL((c), 10);

#define FFF(a, b, c, d, e, x, s) \
	      (a) += F((b), (c), (d)) + (x); \
	      (a) = ROL((a), (s)) + (e); \
	      (c) = ROL((c), 10);

#define GGG(a, b, c, d, e, x, s) \
	      (a) += G((b), (c), (d)) + (x) + 0x7a6d76e9UL; \
	      (a) = ROL((a), (s)) + (e); \
	      (c) = ROL((c), 10);

#define HHH(a, b, c, d, e, x, s) \
	      (a) += H((b), (c), (d)) + (x) + 0x6d703ef3UL; \
	      (a) = ROL((a), (s)) + (e); \
	      (c) = ROL((c), 10);

#define III(a, b, c, d, e, x, s) \
	      (a) += I((b), (c), (d)) + (x) + 0x5c4dd124UL; \
	      (a) = ROL((a), (s)) + (e); \
	      (c) = ROL((c), 10);

#define JJJ(a, b, c, d, e, x, s) \
	      (a) += J((b), (c), (d)) + (x) + 0x50a28be6UL; \
	      (a) = ROL((a), (s)) + (e); \
	      (c) = ROL((c), 10);

static void
rmd160_step(rmd160_context *ctx, const void *source)
{
	const unsigned int *input = source;

	unsigned int aa, aaa, bb, bbb, cc, ccc, dd, ddd, ee, eee;
	aa = aaa = ctx->rmd[0];
	bb = bbb = ctx->rmd[1];
	cc = ccc = ctx->rmd[2];
	dd = ddd = ctx->rmd[3];
	ee = eee = ctx->rmd[4];

	/* round 1 */
	FF(aa, bb, cc, dd, ee, input[ 0], 11);
	FF(ee, aa, bb, cc, dd, input[ 1], 14);
	FF(dd, ee, aa, bb, cc, input[ 2], 15);
	FF(cc, dd, ee, aa, bb, input[ 3], 12);
	FF(bb, cc, dd, ee, aa, input[ 4],  5);
	FF(aa, bb, cc, dd, ee, input[ 5],  8);
	FF(ee, aa, bb, cc, dd, input[ 6],  7);
	FF(dd, ee, aa, bb, cc, input[ 7],  9);
	FF(cc, dd, ee, aa, bb, input[ 8], 11);
	FF(bb, cc, dd, ee, aa, input[ 9], 13);
	FF(aa, bb, cc, dd, ee, input[10], 14);
	FF(ee, aa, bb, cc, dd, input[11], 15);
	FF(dd, ee, aa, bb, cc, input[12],  6);
	FF(cc, dd, ee, aa, bb, input[13],  7);
	FF(bb, cc, dd, ee, aa, input[14],  9);
	FF(aa, bb, cc, dd, ee, input[15],  8);

	/* round 2 */
	GG(ee, aa, bb, cc, dd, input[ 7],  7);
	GG(dd, ee, aa, bb, cc, input[ 4],  6);
	GG(cc, dd, ee, aa, bb, input[13],  8);
	GG(bb, cc, dd, ee, aa, input[ 1], 13);
	GG(aa, bb, cc, dd, ee, input[10], 11);
	GG(ee, aa, bb, cc, dd, input[ 6],  9);
	GG(dd, ee, aa, bb, cc, input[15],  7);
	GG(cc, dd, ee, aa, bb, input[ 3], 15);
	GG(bb, cc, dd, ee, aa, input[12],  7);
	GG(aa, bb, cc, dd, ee, input[ 0], 12);
	GG(ee, aa, bb, cc, dd, input[ 9], 15);
	GG(dd, ee, aa, bb, cc, input[ 5],  9);
	GG(cc, dd, ee, aa, bb, input[ 2], 11);
	GG(bb, cc, dd, ee, aa, input[14],  7);
	GG(aa, bb, cc, dd, ee, input[11], 13);
	GG(ee, aa, bb, cc, dd, input[ 8], 12);

	/* round 3 */
	HH(dd, ee, aa, bb, cc, input[ 3], 11);
	HH(cc, dd, ee, aa, bb, input[10], 13);
	HH(bb, cc, dd, ee, aa, input[14],  6);
	HH(aa, bb, cc, dd, ee, input[ 4],  7);
	HH(ee, aa, bb, cc, dd, input[ 9], 14);
	HH(dd, ee, aa, bb, cc, input[15],  9);
	HH(cc, dd, ee, aa, bb, input[ 8], 13);
	HH(bb, cc, dd, ee, aa, input[ 1], 15);
	HH(aa, bb, cc, dd, ee, input[ 2], 14);
	HH(ee, aa, bb, cc, dd, input[ 7],  8);
	HH(dd, ee, aa, bb, cc, input[ 0], 13);
	HH(cc, dd, ee, aa, bb, input[ 6],  6);
	HH(bb, cc, dd, ee, aa, input[13],  5);
	HH(aa, bb, cc, dd, ee, input[11], 12);
	HH(ee, aa, bb, cc, dd, input[ 5],  7);
	HH(dd, ee, aa, bb, cc, input[12],  5);

	/* round 4 */
	II(cc, dd, ee, aa, bb, input[ 1], 11);
	II(bb, cc, dd, ee, aa, input[ 9], 12);
	II(aa, bb, cc, dd, ee, input[11], 14);
	II(ee, aa, bb, cc, dd, input[10], 15);
	II(dd, ee, aa, bb, cc, input[ 0], 14);
	II(cc, dd, ee, aa, bb, input[ 8], 15);
	II(bb, cc, dd, ee, aa, input[12],  9);
	II(aa, bb, cc, dd, ee, input[ 4],  8);
	II(ee, aa, bb, cc, dd, input[13],  9);
	II(dd, ee, aa, bb, cc, input[ 3], 14);
	II(cc, dd, ee, aa, bb, input[ 7],  5);
	II(bb, cc, dd, ee, aa, input[15],  6);
	II(aa, bb, cc, dd, ee, input[14],  8);
	II(ee, aa, bb, cc, dd, input[ 5],  6);
	II(dd, ee, aa, bb, cc, input[ 6],  5);
	II(cc, dd, ee, aa, bb, input[ 2], 12);

	/* round 5 */
	JJ(bb, cc, dd, ee, aa, input[ 4],  9);
	JJ(aa, bb, cc, dd, ee, input[ 0], 15);
	JJ(ee, aa, bb, cc, dd, input[ 5],  5);
	JJ(dd, ee, aa, bb, cc, input[ 9], 11);
	JJ(cc, dd, ee, aa, bb, input[ 7],  6);
	JJ(bb, cc, dd, ee, aa, input[12],  8);
	JJ(aa, bb, cc, dd, ee, input[ 2], 13);
	JJ(ee, aa, bb, cc, dd, input[10], 12);
	JJ(dd, ee, aa, bb, cc, input[14],  5);
	JJ(cc, dd, ee, aa, bb, input[ 1], 12);
	JJ(bb, cc, dd, ee, aa, input[ 3], 13);
	JJ(aa, bb, cc, dd, ee, input[ 8], 14);
	JJ(ee, aa, bb, cc, dd, input[11], 11);
	JJ(dd, ee, aa, bb, cc, input[ 6],  8);
	JJ(cc, dd, ee, aa, bb, input[15],  5);
	JJ(bb, cc, dd, ee, aa, input[13],  6);

	/* parallel round 1 */
	JJJ(aaa, bbb, ccc, ddd, eee, input[ 5],  8);
	JJJ(eee, aaa, bbb, ccc, ddd, input[14],  9);
	JJJ(ddd, eee, aaa, bbb, ccc, input[ 7],  9);
	JJJ(ccc, ddd, eee, aaa, bbb, input[ 0], 11);
	JJJ(bbb, ccc, ddd, eee, aaa, input[ 9], 13);
	JJJ(aaa, bbb, ccc, ddd, eee, input[ 2], 15);
	JJJ(eee, aaa, bbb, ccc, ddd, input[11], 15);
	JJJ(ddd, eee, aaa, bbb, ccc, input[ 4],  5);
	JJJ(ccc, ddd, eee, aaa, bbb, input[13],  7);
	JJJ(bbb, ccc, ddd, eee, aaa, input[ 6],  7);
	JJJ(aaa, bbb, ccc, ddd, eee, input[15],  8);
	JJJ(eee, aaa, bbb, ccc, ddd, input[ 8], 11);
	JJJ(ddd, eee, aaa, bbb, ccc, input[ 1], 14);
	JJJ(ccc, ddd, eee, aaa, bbb, input[10], 14);
	JJJ(bbb, ccc, ddd, eee, aaa, input[ 3], 12);
	JJJ(aaa, bbb, ccc, ddd, eee, input[12],  6);

	/* parallel round 2 */
	III(eee, aaa, bbb, ccc, ddd, input[ 6],  9);
	III(ddd, eee, aaa, bbb, ccc, input[11], 13);
	III(ccc, ddd, eee, aaa, bbb, input[ 3], 15);
	III(bbb, ccc, ddd, eee, aaa, input[ 7],  7);
	III(aaa, bbb, ccc, ddd, eee, input[ 0], 12);
	III(eee, aaa, bbb, ccc, ddd, input[13],  8);
	III(ddd, eee, aaa, bbb, ccc, input[ 5],  9);
	III(ccc, ddd, eee, aaa, bbb, input[10], 11);
	III(bbb, ccc, ddd, eee, aaa, input[14],  7);
	III(aaa, bbb, ccc, ddd, eee, input[15],  7);
	III(eee, aaa, bbb, ccc, ddd, input[ 8], 12);
	III(ddd, eee, aaa, bbb, ccc, input[12],  7);
	III(ccc, ddd, eee, aaa, bbb, input[ 4],  6);
	III(bbb, ccc, ddd, eee, aaa, input[ 9], 15);
	III(aaa, bbb, ccc, ddd, eee, input[ 1], 13);
	III(eee, aaa, bbb, ccc, ddd, input[ 2], 11);

	/* parallel round 3 */
	HHH(ddd, eee, aaa, bbb, ccc, input[15],  9);
	HHH(ccc, ddd, eee, aaa, bbb, input[ 5],  7);
	HHH(bbb, ccc, ddd, eee, aaa, input[ 1], 15);
	HHH(aaa, bbb, ccc, ddd, eee, input[ 3], 11);
	HHH(eee, aaa, bbb, ccc, ddd, input[ 7],  8);
	HHH(ddd, eee, aaa, bbb, ccc, input[14],  6);
	HHH(ccc, ddd, eee, aaa, bbb, input[ 6],  6);
	HHH(bbb, ccc, ddd, eee, aaa, input[ 9], 14);
	HHH(aaa, bbb, ccc, ddd, eee, input[11], 12);
	HHH(eee, aaa, bbb, ccc, ddd, input[ 8], 13);
	HHH(ddd, eee, aaa, bbb, ccc, input[12],  5);
	HHH(ccc, ddd, eee, aaa, bbb, input[ 2], 14);
	HHH(bbb, ccc, ddd, eee, aaa, input[10], 13);
	HHH(aaa, bbb, ccc, ddd, eee, input[ 0], 13);
	HHH(eee, aaa, bbb, ccc, ddd, input[ 4],  7);
	HHH(ddd, eee, aaa, bbb, ccc, input[13],  5);

	/* parallel round 4 */
	GGG(ccc, ddd, eee, aaa, bbb, input[ 8], 15);
	GGG(bbb, ccc, ddd, eee, aaa, input[ 6],  5);
	GGG(aaa, bbb, ccc, ddd, eee, input[ 4],  8);
	GGG(eee, aaa, bbb, ccc, ddd, input[ 1], 11);
	GGG(ddd, eee, aaa, bbb, ccc, input[ 3], 14);
	GGG(ccc, ddd, eee, aaa, bbb, input[11], 14);
	GGG(bbb, ccc, ddd, eee, aaa, input[15],  6);
	GGG(aaa, bbb, ccc, ddd, eee, input[ 0], 14);
	GGG(eee, aaa, bbb, ccc, ddd, input[ 5],  6);
	GGG(ddd, eee, aaa, bbb, ccc, input[12],  9);
	GGG(ccc, ddd, eee, aaa, bbb, input[ 2], 12);
	GGG(bbb, ccc, ddd, eee, aaa, input[13],  9);
	GGG(aaa, bbb, ccc, ddd, eee, input[ 9], 12);
	GGG(eee, aaa, bbb, ccc, ddd, input[ 7],  5);
	GGG(ddd, eee, aaa, bbb, ccc, input[10], 15);
	GGG(ccc, ddd, eee, aaa, bbb, input[14],  8);

	/* parallel round 5 */
	FFF(bbb, ccc, ddd, eee, aaa, input[12] ,  8);
	FFF(aaa, bbb, ccc, ddd, eee, input[15] ,  5);
	FFF(eee, aaa, bbb, ccc, ddd, input[10] , 12);
	FFF(ddd, eee, aaa, bbb, ccc, input[ 4] ,  9);
	FFF(ccc, ddd, eee, aaa, bbb, input[ 1] , 12);
	FFF(bbb, ccc, ddd, eee, aaa, input[ 5] ,  5);
	FFF(aaa, bbb, ccc, ddd, eee, input[ 8] , 14);
	FFF(eee, aaa, bbb, ccc, ddd, input[ 7] ,  6);
	FFF(ddd, eee, aaa, bbb, ccc, input[ 6] ,  8);
	FFF(ccc, ddd, eee, aaa, bbb, input[ 2] , 13);
	FFF(bbb, ccc, ddd, eee, aaa, input[13] ,  6);
	FFF(aaa, bbb, ccc, ddd, eee, input[14] ,  5);
	FFF(eee, aaa, bbb, ccc, ddd, input[ 0] , 15);
	FFF(ddd, eee, aaa, bbb, ccc, input[ 3] , 13);
	FFF(ccc, ddd, eee, aaa, bbb, input[ 9] , 11);
	FFF(bbb, ccc, ddd, eee, aaa, input[11] , 11);

	/* combine results */
	ddd += cc + ctx->rmd[1];               /* final result for ctx->rmd[0] */
	ctx->rmd[1] = ctx->rmd[2] + dd + eee;
	ctx->rmd[2] = ctx->rmd[3] + ee + aaa;
	ctx->rmd[3] = ctx->rmd[4] + aa + bbb;
	ctx->rmd[4] = ctx->rmd[0] + bb + ccc;
	ctx->rmd[0] = ddd;
}
