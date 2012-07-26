/*
 * mp_rand.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include <string.h>
#include <time.h>

#include "mp.h"
#include "mp_defs.h"

/* mersenne twister */
#define MT_N			624
#define MT_M			397
#define MT_MATRIX_A		0x9908b0dfUL /* constant vector a */
#define MT_UPPER_MASK	0x80000000UL /* most significant w-r bits */
#define MT_LOWER_MASK	0x7fffffffUL /* least significant r bits */
// #define MT_DEF_SEED		0x87654321UL
#define MT_DEF_SEED		(unsigned)time(0)

static void
mt_init32(mp_rand_ctx *ctx, uint32_t s)
{
	int mti;

    ctx->mt[0] = s;
    for (mti = 1; mti < MT_N; mti++)
        ctx->mt[mti] =
			(1812433253UL * (ctx->mt[mti-1] ^ (ctx->mt[mti-1] >> 30)) + mti);
	ctx->mti = mti;
}

static uint32_t
mt_rand32(mp_rand_ctx *ctx)
{
	int mti;
    uint32_t y, mag01[2] = { 0, MT_MATRIX_A };

	mti = ctx->mti;
    if (mti >= MT_N) { /* generate MT_N words at one time */
        int k;

        if (mti == MT_N + 1)   /* if mt_init32() has not been called, */
            mt_init32(ctx, MT_DEF_SEED); /* a default initial seed is used */

        for (k = 0; k < MT_N - MT_M; k++) {
            y = (ctx->mt[k] & MT_UPPER_MASK) |
				(ctx->mt[k + 1] & MT_LOWER_MASK);
            ctx->mt[k] = ctx->mt[k + MT_M] ^ (y >> 1) ^ mag01[y & 1];
        }
        for (; k < MT_N - 1; k++) {
            y = (ctx->mt[k] & MT_UPPER_MASK) |
				(ctx->mt[k + 1] & MT_LOWER_MASK);
            ctx->mt[k] = ctx->mt[k + (MT_M - MT_N)] ^ (y >> 1) ^ mag01[y & 1];
        }
        y = (ctx->mt[MT_N - 1] & MT_UPPER_MASK) |
			(ctx->mt[0] & MT_LOWER_MASK);
        ctx->mt[MT_N - 1] = ctx->mt[MT_M - 1] ^ (y >> 1) ^ mag01[y & 1];

        mti = 0;
    }

    y = ctx->mt[mti];
	ctx->mti = ++mti;

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

void
mp_rand_ctx_init(mp_rand_ctx *ctx)
{
	ASSERT(ctx != NULL);

	mt_init32(ctx, MT_DEF_SEED);
}

void
mp_rand_ctx_init_seed(mp_rand_ctx *ctx, uint32_t seed)
{
	ASSERT(ctx != NULL);

	mt_init32(ctx, seed);
}

void
mp_rand_ctx_clear(mp_rand_ctx *ctx)
{
	ASSERT(ctx != NULL);

	memset(ctx, 0, sizeof(*ctx));
}

/* Try to use all bits output by RNG. */
void
mp_rand_digits(mp_rand_ctx *ctx, mp_digit *u, mp_size size)
{
	unsigned char *ptr;
	static mp_rand_ctx stat_ctx = { MT_N + 1, { 0 } };
	uint32_t k;
	mp_size l;

	if (ctx == NULL)
		ctx = &stat_ctx;

	ptr = (unsigned char *)u;
	size *= MP_DIGIT_SIZE;
	l = size / 4;
	while (l--) {
		k = mt_rand32(ctx);
		*ptr++ = (mp_digit)k; k >>= 8;
		*ptr++ = (mp_digit)k; k >>= 8;
		*ptr++ = (mp_digit)k; k >>= 8;
		*ptr++ = (mp_digit)k;
	}
	if (size &= 3) {
		k = mt_rand32(ctx);
		do {
			*ptr++ = (mp_digit)k;
			k >>= 8;
		} while (--size);
	}
}
