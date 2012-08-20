/* mp_divrem.c
 * Copyright (C) 2002-2012 Farooq Mela. All rights reserved. */

#include "mp.h"
#include "mp_defs.h"

/* Knuth's 4.3.1-D, vol.2, 3rd ed, pp.270-275 */
void
mp_norm_div(mp_digit *u, mp_size usize,
			const mp_digit *v, mp_size vsize, mp_digit *q)
{
	ASSERT(vsize >= 2);
	const mp_digit vd = v[vsize - 1];
	ASSERT((vd & MP_DIGIT_MSB) != 0);

	/* D2: Initialize j. */
	mp_size j = usize - vsize;
	mp_digit *u_j = &u[j]; /* u_j will point to u[j] throughout loop. */
	do {
		mp_digit qhat;
		/* D3: Calculate qhat. */
		if (u_j[vsize] == vd) {
			qhat = ~(mp_digit)0;	/* largest value for mp_digit */
		} else {
			mp_digit q1, q0, rhat, qq;

			digit_div(u_j[vsize], u_j[vsize - 1], vd, qhat, rhat);
			/* Test if q*v_{vsize-2} > B*r + u_{j+vsize-2}; if so, decrement q
			 * and increase r by v_{vsize-1}, and repeat this test if r < B. */
			digit_mul(qhat, v[vsize - 2], q1, q0);
			while ((q1 > rhat) ||
				   (q1 == rhat && q0 > u_j[vsize - 2])) {
				--qhat;
				if ((rhat += vd) < vd)
					break;
				qq = q0;
				q1 -= ((q0 -= v[vsize - 2]) > qq);
			}
		}
		/* D4: Multiply and subtract. */
		mp_digit borrow = mp_dmul_sub(v, vsize, qhat, u_j);
		/* D5: Test remainder. */
		if (u_j[vsize] < borrow) {
			/* D6: Add back. */
			--qhat;
			mp_addi_n(u_j, v, vsize);
		}
		/* D7: Loop on j. */
		if (q != NULL)
			q[j] = qhat;
		--u_j;
	} while (j-- != 0);
}

/* Divide u[usize] by v[vsize], storing the result in q[usize - vsize + 1], and
 * remainder in r[vsize]. v[vsize - 1] must NOT be zero if q != NULL. */
void
mp_divrem(const mp_digit *u, mp_size usize,
		  const mp_digit *v, mp_size vsize, mp_digit *q, mp_digit *r)
{
	ASSERT(u);
	ASSERT(usize > 0);
	ASSERT(v);
	ASSERT(vsize > 0);
	ASSERT(v[vsize - 1] != 0);
	ASSERT(usize >= vsize);

	if (!q && !r) /* Nothing to do. */
		return;

	/* Initialize quotient and remainder to zero. */
	if (r != NULL)
		mp_zero(r, vsize);
	if (q != NULL)
		mp_zero(q, usize - vsize + 1);
	MP_NORMALIZE(u, usize);

	if (usize < vsize) {
		ASSERT(q == NULL);
		if (r != NULL)
			mp_copy(u, usize, r);
	}

	/* Compare numbers. */
	const int cmp = mp_cmp(u, usize, v, vsize);
	if (cmp == 0) {
		/* If equal, set quotient to 1; finished. */
		if (q != NULL)
			q[0] = 1;
		return;
	} else if (cmp < 0) {
		/* If U < V, set remainder to U; finished. */
		if (r != NULL)
			mp_copy(u, usize, r);
		return;
	}

	if (vsize == 1) {
		/* If VLEN == 1 we can divide in linear time (see exercise 16). */
		if (q != NULL) {
			mp_digit remainder = mp_ddiv(u, usize, v[0], q);
			if (r != NULL)
				r[0] = remainder;
		} else {
			ASSERT(r != NULL);
			r[0] = mp_dmod(u, usize, v[0]);
		}
		return;
	}
	/* TODO: add a special case handler for VLEN == 2? */

	/* D1: Normalize. */
	/* Find number of leading zero bits in most significant digit of V. */
	unsigned scale = mp_digit_msb_shift(v[vsize - 1]);
	/* Allocate space for U << SCALE. */
	mp_digit *utmp = MP_TMP_ALLOC(usize + 1);
	mp_copy(u, usize, utmp);
	if (scale == 0) {
		/* Divisor already normalized. */
		utmp[usize] = 0;
		/* D2 - D7. */
		mp_norm_div(utmp, usize, v, vsize, q);
	} else {
		/* XXX Assumes V is writable. */
		/* Shift them left SCALE positions; the size of V will not change. */
		utmp[usize] = mp_lshifti(utmp, usize, scale);
		ASSERT(mp_lshifti((mp_digit *)v, vsize, scale) == 0);
		/* D2 - D7. */
		mp_norm_div(utmp, usize, v, vsize, q);
		ASSERT(mp_rshifti((mp_digit *)v, vsize, scale) == 0);
	}

	/* D8: Store remainder. */
	if (r != NULL) {
		mp_copy(utmp, vsize, r);
		/* If needed, unnormalize. */
		if (scale)
			mp_rshifti(r, vsize, scale);
	}

	/* Release space for U. */
	MP_TMP_FREE(utmp);
}
