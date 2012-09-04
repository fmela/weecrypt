/* mp_divexact.c
 * Copyright (C) 2002-2012 Farooq Mela. All rights reserved. */

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
    ASSERT(u);
    ASSERT(usize > 0);
    ASSERT(d);
    ASSERT(dsize > 0);
    ASSERT(d[dsize - 1] != 0);
    ASSERT(q);

    mp_zero(q, usize - dsize + 1);
    MP_NORMALIZE(u, usize);
    if (!usize) /* U is zero; we are done. */
	return;

#if 0
    if (usize == 1) {
	ASSERT(dsize == 1);
	ASSERT(u[0] % d[0] == 0);
	q[0] = u[0] / d[0];
	return;
    }
#endif

    /* Shift U and D right until D's least significant bit is set. Before
     * starting, we can check if the index of U's lowest set bit is lower than
     * D's lowest set bit; if this is the case then U cannot possibly be a
     * multiple of D and the algorithm terminates unsuccessfully. */
    const unsigned d_shift = mp_odd_shift(d, dsize);
    if (mp_odd_shift(u, usize) < d_shift)
	return;
    const unsigned dd = d_shift / MP_DIGIT_BITS;
    const unsigned ds = d_shift % MP_DIGIT_BITS;
    u += dd, usize -= dd;
    d += dd, dsize -= dd;

    mp_digit *utmp = NULL, *dtmp = NULL;
    if (ds) {
	utmp = MP_TMP_ALLOC(usize);
	ASSERT(mp_rshift(u, usize, ds, utmp) == 0);
	dtmp = MP_TMP_ALLOC(dsize);
	ASSERT(mp_rshift(d, dsize, ds, dtmp) == 0);
	d = dtmp;
    } else {
	utmp = MP_TMP_COPY(u, usize);
    }

    ASSERT((d[0] & 1) == 1);
    const mp_digit d0_inv = mp_digit_invert(d[0]);

    const mp_size q_size = usize - dsize + 1;
    for (mp_size q_i = 0; q_i < q_size; q_i++) {
	q[q_i] = d0_inv * utmp[q_i];
	const mp_size size = MIN(dsize, q_size - q_i);
	mp_digit cy = mp_dmul_sub(d, size, q[q_i], &utmp[q_i]);
	if (cy) {
	    ASSERT(usize > q_i + size);
	    /* FIXME: this assert will fail if U is not a multiple of D. */
	    ASSERT(mp_dsubi(&utmp[q_i + size], usize - (q_i + size), cy) == 0);
	}
	ASSERT(utmp[q_i] == 0);
    }
    if (dtmp != NULL)
	MP_TMP_FREE(dtmp);
    MP_TMP_FREE(utmp);
}
