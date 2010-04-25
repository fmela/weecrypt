/*
 * mp_composite.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mp.h"
#include "mp_defs.h"
#include <stdlib.h>

/* Knuth 4.5.4P: use a variant of the Miller-Rabin probabilistic procedure to
 * test the integer N for compositeness. It will never incorrectly identify a
 * prime as composite, but it may incorrectly identify a composite as prime.
 * For a given round there is a worst-case probability 1/4 that this
 * algorithm guesses wrong; thus for N rounds it guesses wrong with worst-case
 * probability (1/4)^N. With typical input the chances of wrong guess are far
 * more minute. It usually makes sense to first test N for divisibility by
 * small primes using mp_sieve() before calling this function.
 *
 * Return 1 if N is composite, otherwise 0 if N is "probably" prime. */
int
mp_composite(const mp_digit *n, mp_size nsize, unsigned rounds)
{
	unsigned r, k, j;
	mp_digit *x, *q, *y, cy;
	mp_size qsize, ysize, xsize;

	nsize = mp_rsize(n, nsize);
	if (nsize == 0)			/* Call 0 prime. */
		return 0;
	if ((n[0] & 1) == 0)	/* Even N. */
		return 1;
	if (rounds == 0)		/* Nuthin' doin' */
		return 1;

	/* Find K and Q such that N = (2^K) * Q + 1, Q odd.
	 * Assumes that it is safe to modify N. */
	q = (mp_digit *)n;
	--*q; /* Can't underflow: N odd. */
	k = mp_odd_shift(n, nsize);
	++*q;
	qsize = nsize - (j = k / MP_DIGIT_BITS);
	MP_TMP_COPY(q, n + j, qsize);
	if ((j = k % MP_DIGIT_BITS) != 0) {
		cy = mp_rshifti(q, qsize, j);
		ASSERT(cy == 1);	/* This is the trailing odd bit. */
	}

	MP_TMP_ALLOC(y, nsize * 3); /* x will store y^2 */
	x = y + nsize;
	for (r = 0; r < rounds; r++) {
		/* Generate X so 1 < X < N */
		xsize = nsize - (rand() % nsize);
		mp_rand(x, xsize);
		if (xsize == nsize && x[xsize - 1] >= n[xsize - 1])
			xsize -= ((x[xsize - 1] -= n[xsize - 1]) == 0);

		/* Y = X^Q mod N */
	//	mp_modexp(x, xsize, q, qsize, n, nsize, y);
	//	mp_modexp_pow2(x, xsize, q, qsize, n, nsize, y);
		mp_mexp(x, xsize, q, qsize, n, nsize, y);

		for (j = k; j != 0; j--) {
			ysize = mp_rsize(y, nsize);
			/* If J = 0 and Y = 1, prime. */
			if (j == 0 && ysize == 1 && y[0] == 1) {
				MP_TMP_FREE(y);
				MP_TMP_FREE(q);
				return 0;
			}

			/* If Y = N - 1 (Y + 1 == N), prime. */
			if (mp_inc(y, ysize))
				y[ysize++] = 1;
			if (ysize == nsize && mp_cmp_n(y, n, ysize) == 0) {
				MP_TMP_FREE(y);
				MP_TMP_FREE(q);
				return 0;
			}
			mp_dec(y, ysize);
			ysize -= (y[ysize - 1] == 0);

			/* Y = Y^2 mod N. */
			mp_sqr(y, ysize, x);
			mp_modi(x, ysize * 2, n, nsize);
			mp_copy(x, nsize, y);
		}
	}

	MP_TMP_FREE(y);
	MP_TMP_FREE(q);

	return 1;
}
