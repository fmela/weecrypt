/*
 * mp_divexact.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mp.h"
#include "mp_defs.h"

/* This routine implements Jebelean's algorithm for exact division. It will
 * divide u[usize] by d[dsize] and put the quotient in q[usize - dsize + 1]
 * when it is known in advance that u is a multiple of d. If u is NOT a
 * multiple of d, q's contents will not be anything meaningful. */
void
mp_divexact(const mp_digit *u, mp_size usize,
			const mp_digit *d, mp_size dsize, mp_digit *q)
{
	mp_digit *utmp, *dtmp, d0_inv, cy;
	mp_size k, K, size;
	unsigned ds, dd;

	ASSERT(u != NULL);
	ASSERT(u[usize - 1] != 0);
	ASSERT(d != NULL);
	ASSERT(dsize != 0);
	ASSERT(d[dsize - 1] != 0);
	ASSERT(q != NULL);

	mp_zero(q, usize - dsize + 1);
	usize = mp_rsize(u, usize);
	if (usize == 0) /* U is zero; we are done. */
		return;

	/* Shift U and D right until D's least significant bit is set. Before
	 * starting, we can check if the index of U's lowest set bit is lower than
	 * D's lowest set bit; if this is the case then U cannot possibly be a
	 * multiple of D and the algorithm terminates unsuccessfully. */
	ds = mp_odd_shift(d, dsize);
	if (mp_odd_shift(u, usize) < ds)
		return;
	dd = ds / MP_DIGIT_BITS;
	ds = ds % MP_DIGIT_BITS;
	u += dd, usize -= dd;
	d += dd, dsize -= dd;

	if (ds) {
		MP_TMP_ALLOC(utmp, usize);
		cy = mp_rshift(u, usize, ds, utmp);
		ASSERT(cy == 0);
	} else {
		MP_TMP_COPY(utmp, u, usize);
	}

	if (ds) {
		MP_TMP_ALLOC(dtmp, dsize);
		cy = mp_rshift(d, dsize, ds, dtmp);
		ASSERT(cy == 0);
		d = dtmp;
	} else {
		dtmp = NULL;
	}

	ASSERT((d[0] & 1) == 1);
	d0_inv = mp_digit_invert(d[0]);

	K = usize - dsize + 1;
	for (k = 0; k < K; k++) {
		q[k] = d0_inv * utmp[k];
		size = MIN(dsize, K - k);
		cy = mp_dmul_sub(d, size, q[k], &utmp[k]);
		if (cy) {
			ASSERT(usize > k + size);
			cy = mp_dsubi(&utmp[k + size], usize - (k + size), cy);
		}
		ASSERT(utmp[k] == 0);
		ASSERT(cy == 0);
	}
	if (dtmp != NULL)
		MP_TMP_FREE(dtmp);
	MP_TMP_FREE(utmp);
}
