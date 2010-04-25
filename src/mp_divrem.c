/*
 * mp_divrem.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mp.h"
#include "mp_defs.h"

void
mp_norm_div(mp_digit *u, mp_size usize,
			const mp_digit *v, mp_size vsize, mp_digit *q)
{
	mp_size j;
	mp_digit qhat, vd, borrow;
	mp_digit *u_j;

	/* Knuth's 4.3.1-D, vol.2, 3rd ed, pp.270-275 */

	ASSERT(vsize >= 2);
	vd = v[vsize - 1];
	ASSERT((vd & MP_DIGIT_MSB) != 0);

	/* D2: Initialize j. */
	u_j = &u[j = usize - vsize];	/* u_j will point to u[j] throughout loop. */
	do {
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
				qhat--;
				if ((rhat += vd) < vd)
					break;
				qq = q0;
				q1 -= ((q0 -= v[vsize - 2]) > qq);
			}
		}
		/* D4: Multiply and subtract. */
		borrow = mp_dmul_sub(v, vsize, qhat, u_j);
		/* D5: Test remainder. */
		if (u_j[vsize] < borrow) {
			/* D6: Add back. */
			qhat--;
			mp_addi_n(u_j, v, vsize);
		}
		/* D7: Loop on j. */
		if (q != NULL)
			q[j] = qhat;
		u_j--;
	} while (j-- != 0);
}

/* Divide u[usize] by v[vsize], storing the result in q[usize - vsize + 1], and
 * remainder in r[vsize]. v[vsize - 1] must NOT be zero if q != NULL. */
int
mp_divrem(const mp_digit *u, mp_size usize,
		  const mp_digit *v, mp_size vsize, mp_digit *q, mp_digit *r)
{
	int rv;
	mp_digit scale, cy, *utmp;

	ASSERT(u != NULL);
	ASSERT(usize != 0);
//	ASSERT(u[usize - 1] != 0);
	ASSERT(v != NULL);
	ASSERT(vsize != 0);
	ASSERT(v[vsize - 1] != 0);
//	ASSERT(usize >= vsize);


	if (q == NULL && r == NULL) /* Nothing to do. */
		return 0;

	/* Initialize quotient and remainder to zero. */
	if (r != NULL)
		mp_zero(r, vsize);
	if (q != NULL)
		mp_zero(q, usize - vsize + 1);

	if (usize < vsize) {
		ASSERT(q == NULL);
		if (r != NULL)
			mp_copy(u, usize, r);
	}

	/* Compare numbers. */
	rv = mp_cmp(u, usize, v, vsize);
	if (rv == 0) {
		/* If equal, set quotient to 1; finished. */
		if (q != NULL)
			q[0] = 1;
		return 0;
	} else if (rv < 0) {
		/* If U < V, set remainder to U; finished. */
		if (r != NULL)
			mp_copy(u, usize, r);
		return 0;
	}

	if (vsize == 1) {
		/* If VLEN == 1 we can divide in linear time (see exercise 16). */
		if (q != NULL) {
			cy = mp_ddiv(u, usize, v[0], q);
			if (r != NULL)
				r[0] = cy;
		} else {
			ASSERT(r != NULL);
			r[0] = mp_dmod(u, usize, v[0]);
		}
		return 0;
	}
	/* TODO: add a special case handler for VLEN == 2? */

	/* D1: Normalize. */
	/* Find number of leading zero bits in most significant digit of V. */
	scale = mp_msb_shift(v[vsize - 1]);
	/* Allocate space for U << SCALE. */
	MP_TMP_ALLOC(utmp, usize + 1);
	mp_copy(u, usize, utmp);
	if (scale == 0) {
		/* Divisor already normalized. */
		utmp[usize] = 0;
		/* D2 - D7. */
		mp_norm_div(utmp, usize, v, vsize, q);
	} else {
		mp_digit *vp;

		/* XXX Assume V is writable. */
		vp = (mp_digit *)v;
		/* Shift them left SCALE positions; the size of V will not change. */
		utmp[usize] = mp_lshifti(utmp, usize, scale);
		cy = mp_lshifti(vp, vsize, scale);
		ASSERT(cy == 0);
		/* D2 - D7. */
		mp_norm_div(utmp, usize, vp, vsize, q);
		mp_rshifti(vp, vsize, scale);
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
	return 0;
}
