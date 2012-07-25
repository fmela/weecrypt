/*
 * mp_sqrtrem.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mp.h"
#include "mp_defs.h"

static mp_digit digit_sqrt(mp_digit n);

/* Find the integer part of the square root of U and store it in V; V must be
 * at least as large as ceil(ULEN / 2). */
void
mp_sqrtrem(const mp_digit *u, mp_size usize, mp_digit *v, mp_digit *r)
{
	mp_digit *y, *x, cy;
	mp_size vsize, xsize, y_len, ysize, x_len;

	ASSERT(u != NULL);
	ASSERT(v != NULL);

	/* Newton iteration begins with some guess x_{0} and computes the sequence
	 *
	 * x_{i+1} = x_{i} - f(x_{i})/f'(x_{i})
	 *
	 * to find the zero of some function f(x). Since we are trying to find the
	 * square root of number a, we use the function f(x) = x^2 - a, so we have
	 *
	 * x_{i+1} = x_{i} - (x_{i}^2 - a)/(2x_{i})
	 *         = x_{i} - (x_{i} - a/x_{i})/2
	 *         = (x_{i} + a/x_{i})/2
	 *
	 * with x_{0} = 2^(floor(lg(N))/2 + 1)
	 */

	/* TODO: Implement a higher-order iteration for faster convergence? */

	if (!v && !r)
		return;	/* Nothing to do. */
	if (v)
		mp_zero(v, (usize + 1) / 2);
	if (r)
		mp_zero(r, usize);
	MP_NORMALIZE(u, usize);
	if (!usize)
		return;
	if (usize == 1) {
		cy = digit_sqrt(u[0]);
		if (v != NULL)
			v[0] = cy;
		if (r != NULL)
			r[0] = u[0] - (cy * cy);
		return;
	}
	vsize = (usize + 1) / 2;

	/* FIXME: This is very "generous" in terms of amount of space allocated for
	 * X and Y. Come up with something better? */
	xsize = ysize = usize;
	MP_TMP_ALLOC0(x, xsize + ysize);
	y = x + xsize;
	mp_setbit(x, xsize, (mp_significant_bits(u, usize) / 2) + 1);
	x_len = xsize;

	for (;;) {
		/* Y = U / X */
		x_len = mp_rsize(x, x_len);
		y_len = usize - x_len + 1;
		mp_zero(y + y_len, ysize - y_len);
		mp_div(u, usize, x, x_len, y);
		/* Y += X */
		cy = mp_addi(y, ysize, x, x_len);
		ASSERT(cy == 0);
		y_len = mp_rsize(y, ysize);
		/* Y /= 2 */
		mp_rshifti(y, y_len, 1);
		y_len -= (y[y_len - 1] == 0);
		/* If Y >= X, terminate with X as answer. */
		if (y_len > x_len ||
			(y_len == x_len && mp_cmp_n(y, x, y_len) >= 0))
			break;
		/* X = Y */
		mp_copy(y, y_len, x);
		mp_zero(x + y_len, xsize - y_len);
		x_len = y_len;
	}
	ASSERT(x_len == vsize);
	if (v != NULL)
		mp_copy(x, x_len, v);
	if (r != NULL) {
		mp_sqr(v, vsize, x);
		cy = mp_sub(u, usize, x, vsize * 2, r);
		ASSERT(cy == 0);
	}
	MP_TMP_FREE(x);
}

/* This is so much simpler it's not funny. */
static mp_digit
digit_sqrt(mp_digit n)
{
	mp_digit x, y;

	if (n <= 1)
		return n;

	x = (mp_digit)1 << ((mp_digit_log2(n) >> 1) + 1);
	for (;;) {
		y = (x + n / x) >> 1;
		if (y >= x)
			return x;
		x = y;
	}
}
