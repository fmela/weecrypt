/*
 * mp_barrett.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mp.h"
#include "mp_defs.h"

static void barrett_reduce(const mp_digit *x,
						   const mp_barrett_ctx *ctx, mp_digit *r);

/* Compute MU = floor(B^2K / M) (K = MLEN) */
void
mp_barrett_ctx_init(mp_barrett_ctx *ctx, const mp_digit *m, mp_size msize)
{
	mp_digit *s;

	ASSERT(ctx != NULL);
	ASSERT(ctx->m == NULL);
	ASSERT(ctx->mu == NULL);
	ASSERT(ctx->k == 0);

	msize = mp_rsize(m, msize);
	ASSERT(msize != 0);

	MP_TMP_ALLOC0(s, msize * 2 + 1);
	s[msize * 2] = 1;

	ctx->m = m;
	ctx->k = msize;
	ctx->mu = mp_new(msize + 2);
	mp_div(s, msize * 2 + 1, m, msize, ctx->mu);
	ASSERT(mp_rsize(ctx->mu, msize + 2) == msize + 1);

	MP_TMP_FREE(s);
}

void
mp_barrett_ctx_clear(mp_barrett_ctx *ctx)
{
	ASSERT(ctx != NULL);
	ASSERT(ctx->m != NULL);
	ASSERT(ctx->mu != NULL);
	ASSERT(ctx->k != 0);

	ctx->m = NULL;
	mp_free(ctx->mu);
	ctx->mu = NULL;
	ctx->k = 0;
}

/* Compute W = (U ** P) mod M using Barrett modular reduction. */
void
mp_barrett(const mp_digit *u, mp_size usize,
		   const mp_digit *p, mp_size psize,
		   const mp_barrett_ctx *ctx, mp_digit *w)
{
	mp_digit k;
	mp_digit *t, *umod;
	mp_size msize, tsize, umod_size, i;
	const mp_digit *m;

	ASSERT(u != NULL);
	ASSERT(p != NULL);
	ASSERT(ctx != NULL);
	ASSERT(ctx->m != NULL);
	ASSERT(ctx->mu != NULL);
	ASSERT(ctx->k != 0);
	ASSERT(w != NULL);

	m = ctx->m;
	msize = ctx->k;
	mp_zero(w, msize);
	msize = mp_rsize(m, msize);
	ASSERT(msize == ctx->k);

	if (msize == 1 && m[0] == 1) /* Anything mod 1 is zero. */
		return;

	usize = mp_rsize(u, usize);
	psize = mp_rsize(p, psize);
	if (usize == 0 || psize == 0) {
		w[0] = (psize != 0);
		return;
	}

	if (psize == 1) {
		if (p[0] == 1) {
			mp_mod(u, usize, m, msize, w);
			return;
		}
		if (p[0] == 2) {
			tsize = usize * 2;
			MP_TMP_ALLOC(t, tsize);
			mp_sqr(u, usize, t);
			mp_mod(t, tsize, m, msize, w);
			MP_TMP_FREE(t);
			return;
		}
	}

	/* Precompute W = U mod M */
	mp_mod(u, usize, m, msize, w);
	umod_size = mp_rsize(w, msize);
	if (umod_size == 0) {
		/* If U is congruent to 0 mod M, the final answer is zero. */
		return;
	}

	tsize = msize * 2;
	MP_TMP_ALLOC(t, tsize + umod_size);
	umod = t + tsize;
	mp_copy(w, umod_size, umod);

	k = p[psize - 1];
	for (i = 0; k != 1; i++)
		k >>= 1;
	k <<= i;
	i = psize - 1;

	for (;;) {
		if ((k >>= 1) == 0) {
			if (i == 0)
				break;
			--i;
			k = MP_DIGIT_MSB;
		}
		mp_sqr(w, msize, t);
		barrett_reduce(t, ctx, w);
		if (p[i] & k) {
			mp_mul(w, msize, umod, umod_size, t);
			/* Barrett reduction requires T be twice as large as M, so clear
			 * high digits if needed. */
			if (msize - umod_size)
				mp_zero(t + msize + umod_size, msize - umod_size);
			barrett_reduce(t, ctx, w);
		}
	}

	MP_TMP_FREE(t);
}

void
mp_barrett_ul(const mp_digit *u, mp_size usize, unsigned long power,
			  const mp_barrett_ctx *ctx, mp_digit *w)
{
	const mp_digit *m;
	mp_digit *t, *umod;
	mp_size msize, tsize, umod_size;
	unsigned long k;
	unsigned i;

	ASSERT(u != NULL);
	ASSERT(ctx != NULL);
	ASSERT(ctx->m != NULL);
	ASSERT(ctx->mu != NULL);
	ASSERT(ctx->k != 0);
	ASSERT(w != NULL);

	m = ctx->m;
	msize = ctx->k;
	mp_zero(w, msize);
	msize = mp_rsize(m, msize);
	ASSERT(msize == ctx->k);

	if (msize == 1 && m[0] == 1) /* Anything mod 1 is zero. */
		return;

	usize = mp_rsize(u, usize);
	if (usize == 0 || power == 0) {
		w[0] = (usize != 0);
		return;
	}

	if (power == 1) {
		mp_mod(u, usize, m, msize, w);
		return;
	}

	if (power == 2) {
		tsize = usize * 2;
		MP_TMP_ALLOC(t, tsize);
		mp_sqr(u, usize, t);
		mp_mod(t, tsize, m, msize, w);
		MP_TMP_FREE(t);
		return;
	}

	/* Precompute W = U mod M */
	mp_mod(u, usize, m, msize, w);
	umod_size = mp_rsize(w, msize);
	if (umod_size == 0) {
		/* If U is congruent to 0 mod M, the final answer is zero. */
		return;
	}

	/* With power of the form 2^K, we never have to multiply by U mod M. */
	tsize = msize * 2;
	if (power & (power - 1)) {
		MP_TMP_ALLOC(t, tsize + umod_size);
		umod = t + tsize;
		mp_copy(w, umod_size, umod);
	} else {
		MP_TMP_ALLOC(t, tsize);
		umod = NULL;
	}

	k = power;
	for (i = 0; k != 1; i++)
		k >>= 1;
	k <<= i;		/* power mask. */

	k = power;
	for (i = 0; k != 1; i++)
		k >>= 1;
	k <<= i;

	while (k >>= 1) {
		mp_sqr(w, msize, t);
		barrett_reduce(t, ctx, w);
		if (power & k) {
			mp_mul(w, msize, umod, umod_size, t);
			/* Barrett reduction requires T be twice as large as M, so clear
			 * high digits if needed. */
			if (msize - umod_size)
				mp_zero(t + msize + umod_size, msize - umod_size);
			barrett_reduce(t, ctx, w);
		}
	}

	MP_TMP_FREE(t);
}


/* Barrett modular reduction.  This implementation follows from algorithm 14.42
 * in "Handbook of Applied Cryptography" pp.603-604
 *
 * Input: x[0..2k-1], m[0..k-1], and mu[0..k] = (b^(2k))/m
 * Output: r[0..k-1] = x mod m */
static void
barrett_reduce(const mp_digit *x, const mp_barrett_ctx *ctx, mp_digit *r)
{
	const mp_digit *q1, *m;
	mp_digit *q2, *q3, *r2, cy;
	mp_size q2size, rsize;
	mp_size k;

	k = ctx->k;
	m = ctx->m;

	/* Step 1. */
	/* q1 = x / b^(k-1); k + 1 digits */
	q1 = &x[k - 1];
	/* q2 = q1 * mu */
	q2size = 2 * (k + 1);
	MP_TMP_ALLOC(q2, q2size);
	mp_mul_n(q1, ctx->mu, k + 1, q2);
	/* q3 = q2 / b^(k+1); k + 1 digits */
	q3 = &q2[k + 1];

	/* Step 2. */
	/* r1 = x mod b^(k+1) */
	/* r2 = q3 * m mod b^(k+1) */
	MP_TMP_ALLOC(r2, k + 1);
	mp_mul_mod_powb(q3, k + 1, m, k, r2, k + 1);
	MP_TMP_FREE(q2); /* done with q's */
	/* r = r1 - r2, combined with Step 3: if r < 0, r += b^(k+1) */
	/* it's 2's complement, so r1 if < r2 the "+= b^(k+1)" is taken care of */
	mp_sub_n(x, r2, k + 1, r2);
	rsize = mp_rsize(r2, k + 1);

	/* Step 4. */
	/* While r >= m, r -= m (will repeat at most twice) */
	if (mp_cmp(r2, rsize, m, k) > 0) {
		cy = mp_subi(r2, rsize, m, k);
		ASSERT(cy == 0);
		rsize = mp_rsize(r2, rsize);
		if (mp_cmp(r2, rsize, m, k) > 0) {
			cy = mp_subi(r2, rsize, m, k);
			ASSERT(cy == 0);
		}
	}

	/* Step 5. Return r. */
	mp_copy(r2, k, r);
	MP_TMP_FREE(r2);
}
