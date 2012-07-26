/*
 * mp_perfsqr.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mp.h"
#include "mp_defs.h"

/* Adapted from Algorithm 1.7.3 from Cohen, "A Course in Computational
 * Algebraic Number Theory" */

/* q11[N mod 11] = 0 if N non-perfect
 * for (k = 0; k < 11; k++) q11[k] = 0;
 * for (k = 0; k <  6; k++) q11[(k * k) % 11] = 1; */
static const uint8_t q11[11] = {
	1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0
};

/* q63[N mod 63] = 0 if N non-perfect
 * for (k = 0; k < 63; k++) q63[k] = 0;
 * for (k = 0; k < 32; k++) q63[(k * k) % 63] = 1; */
static const uint8_t q63[63] = {
	1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1,
	0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0,
	1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0
};

/* q64[N mod 64] = 0 if N non-perfect
 * for (k = 0; k < 64; k++) q64[k] = 0;
 * for (k = 0; k < 32; k++) q64[(k * k) % 64] = 1; */
static const uint8_t q64[64] = {
	1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0
};

/* q11[N mod 11] = 0 if N non-perfect
 * for (k = 0; k < 65; k++) q65[k] = 0;
 * for (k = 0; k < 33; k++) q65[(k * k) % 65] = 1; */
static const uint8_t q65[65] = {
	1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1
};

int
mp_perfsqr(const mp_digit *u, mp_size usize)
{
	int rv;
	mp_digit *rem;
#if MP_DIGIT_SIZE != 1
	mp_digit r;
#endif

	ASSERT(u != NULL);

	usize = mp_rsize(u, usize);
	if (usize == 0)
		return 1;

	if (q64[u[0] & 63] == 0)
		return 0;
#if MP_DIGIT_SIZE == 1
	if (q63[mp_dmod(u, usize, 63)] == 0 ||
		q65[mp_dmod(u, usize, 65)] == 0 ||
		q11[mp_dmod(u, usize, 11)] == 0)
		return 0;
#else
	/* U (mod m1) == (U (mod m1*m2)) (mod m1) since
	 * (U (mod m1*m2)) = U - m1*m2*floor(U/(m1*m2)) and
	 * any multiple of m1 == 0 (mod m1). */
	r = mp_dmod(u, usize, 45045U); /* 63 * 65 * 11 */
	if (q63[r % 63] == 0 ||
		q65[r % 65] == 0 ||
		q11[r % 11] == 0)
		return 0;
#endif
	/* Previous fast tests filter out 709/715 = ~99.16% of numbers.  If that
	 * didn't work, calculate square root and remainder. U is perfect square
	 * iff remainder is zero. */
	MP_TMP_ALLOC(rem, usize);
	mp_sqrtrem(u, usize, NULL, rem);
	rv = (mp_rsize(rem, usize) == 0);
	MP_TMP_FREE(rem);
	return rv;
}
