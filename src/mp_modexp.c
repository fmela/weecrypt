/*
 * mp_modexp.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mp.h"
#include "mp_defs.h"

void
mp_modexp(const mp_digit *u, mp_size usize,
		  const mp_digit *p, mp_size psize,
		  const mp_digit *m, mp_size msize, mp_digit *w)
{
	mp_digit *umod, *t, k;
	mp_size umsize, i;

	ASSERT(u != NULL);
	ASSERT(p != NULL);
	ASSERT(m != NULL);
	ASSERT(w != NULL);

	mp_zero(w, msize);
	msize = mp_rsize(m, msize);
	ASSERT(msize != 0);

	if (msize == 1 && m[0] == 1)	/* Anything mod 1 is zero. */
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
			MP_TMP_ALLOC(t, usize * 2);
			mp_sqr(u, usize, t);
			mp_mod(t, usize * 2, m, msize, w);
			MP_TMP_FREE(t);
			return;
		}
	}

	/* Precompute U mod M. */
	mp_mod(u, usize, m, msize, w);
	umsize = mp_rsize(w, msize);
	if (umsize == 0) {
		/* If U is congruent to 0 mod M, the final answer is zero. */
		mp_zero(w, msize);
		return;
	}

	MP_TMP_ALLOC(t, umsize + (msize * 2));
	umod = t + (msize * 2);
	mp_copy(w, umsize, umod);

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
		mp_mod(t, msize * 2, m, msize, w);
		if (p[i] & k) {
			mp_mul(w, msize, umod, umsize, t);
			mp_mod(t, msize + umsize, m, msize, w);
		}
	}

	MP_TMP_FREE(t);
}

void
mp_modexp_ul(const mp_digit *u, mp_size usize, unsigned long power,
			 const mp_digit *m, mp_size msize, mp_digit *w)
{
	mp_digit *umod, *t;
	mp_size umsize;
	unsigned long k;
	unsigned i;

	ASSERT(u != NULL);
	ASSERT(m != NULL);
	ASSERT(w != NULL);

	mp_zero(w, msize);
	msize = mp_rsize(m, msize);
	ASSERT(msize != 0);

	if (msize == 1 && m[0] == 1)	/* Anything mod 1 is zero. */
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
		MP_TMP_ALLOC(t, usize * 2);
		mp_sqr(u, usize, t);
		mp_mod(t, usize * 2, m, msize, w);
		MP_TMP_FREE(t);
		return;
	}

	/* Precompute U mod M. */
	mp_mod(u, usize, m, msize, w);
	umsize = mp_rsize(w, msize);
	if (umsize == 0) {
		/* If U is congruent to 0 mod M, the final answer is zero. */
		return;
	}

	if (power & (power - 1)) {
		MP_TMP_ALLOC(t, umsize + (msize * 2));
		umod = t + (msize * 2);
		mp_copy(w, umsize, umod);
	} else {
		/* With power of the form 2^K, we never have to multiply by U mod M. */
		MP_TMP_ALLOC(t, msize * 2);
		umod = NULL;
	}

	k = power;
	for (i = 0; k != 1; i++)
		k >>= 1;
	k <<= i;

	while (k >>= 1) {
		mp_sqr(w, msize, t);
		mp_mod(t, msize * 2, m, msize, w);
		if (power & k) {
			mp_mul(w, msize, umod, umsize, t);
			mp_mod(t, msize + umsize, m, msize, w);
		}
	}

	MP_TMP_FREE(t);
}
