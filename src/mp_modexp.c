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
	ASSERT(u != NULL);
	ASSERT(p != NULL);
	ASSERT(m != NULL);
	ASSERT(w != NULL);

	mp_zero(w, msize);
	MP_NORMALIZE(m, msize);
	ASSERT(msize != 0);

	if (msize == 1 && m[0] == 1)	/* Anything mod 1 is zero. */
		return;

	MP_NORMALIZE(u, usize);
	MP_NORMALIZE(p, psize);
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
			mp_digit *tmp;
			MP_TMP_ALLOC(tmp, usize * 2);
			mp_sqr(u, usize, tmp);
			mp_mod(tmp, usize * 2, m, msize, w);
			MP_TMP_FREE(tmp);
			return;
		}
	}

	/* Precompute U mod M. */
	mp_mod(u, usize, m, msize, w);
	const mp_size umsize = mp_rsize(w, msize);
	if (umsize == 0) {
		/* If U is congruent to 0 mod M, the final answer is zero. */
		mp_zero(w, msize);
		return;
	}

	mp_digit *tmp;
	MP_TMP_ALLOC(tmp, umsize + (msize * 2));
	mp_digit *umod = tmp + (msize * 2);
	mp_copy(w, umsize, umod);

	mp_digit k = p[psize - 1];
	unsigned i = 0;
	for (; k != 1; i++)
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
		mp_sqr(w, msize, tmp);
		mp_mod(tmp, msize * 2, m, msize, w);
		if (p[i] & k) {
			mp_mul(w, msize, umod, umsize, tmp);
			mp_mod(tmp, msize + umsize, m, msize, w);
		}
	}

	MP_TMP_FREE(tmp);
}

void
mp_modexp_ul(const mp_digit *u, mp_size usize, unsigned long power,
			 const mp_digit *m, mp_size msize, mp_digit *w)
{
	ASSERT(u != NULL);
	ASSERT(m != NULL);
	ASSERT(w != NULL);

	mp_zero(w, msize);
	MP_NORMALIZE(m, msize);
	ASSERT(msize != 0);

	if (msize == 1 && m[0] == 1)	/* Anything mod 1 is zero. */
		return;

	MP_NORMALIZE(u, usize);
	if (usize == 0 || power == 0) {
		w[0] = (usize != 0);
		return;
	}

	if (power == 1) {
		mp_mod(u, usize, m, msize, w);
		return;
	}

	if (power == 2) {
		mp_digit *tmp;
		MP_TMP_ALLOC(tmp, usize * 2);
		mp_sqr(u, usize, tmp);
		mp_mod(tmp, usize * 2, m, msize, w);
		MP_TMP_FREE(tmp);
		return;
	}

	/* Precompute U mod M. */
	mp_mod(u, usize, m, msize, w);
	const mp_size umsize = mp_rsize(w, msize);
	if (umsize == 0) {
		/* If U is congruent to 0 mod M, the final answer is zero. */
		return;
	}

	mp_digit *tmp, *umod;
	if (power & (power - 1)) {
		MP_TMP_ALLOC(tmp, umsize + (msize * 2));
		umod = tmp + (msize * 2);
		mp_copy(w, umsize, umod);
	} else {
		/* With power of the form 2^K, we never have to multiply by U mod M. */
		MP_TMP_ALLOC(tmp, msize * 2);
		umod = NULL;
	}

	unsigned long k = power;
	unsigned i = 0;
	for (; k != 1; i++)
		k >>= 1;
	k <<= i;

	while (k >>= 1) {
		mp_sqr(w, msize, tmp);
		mp_mod(tmp, msize * 2, m, msize, w);
		if (power & k) {
			mp_mul(w, msize, umod, umsize, tmp);
			mp_mod(tmp, msize + umsize, m, msize, w);
		}
	}

	MP_TMP_FREE(t);
}
