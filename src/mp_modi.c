/*
 * mp_modi.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mp.h"
#include "mp_defs.h"

/* Knuth's 4.3.1D, but ignoring quotient */
void
mp_modi(mp_digit *u, mp_size usize, const mp_digit *v, mp_size vsize)
{
	mp_digit u_high, *u_j;
	mp_digit q1, q0, rhat, qhat, vd;
	unsigned vs;
	int rv;

	ASSERT(u != NULL);
	ASSERT(v != NULL);

	/* V cannot be zero. */
	vsize = mp_rsize(v, vsize);
	ASSERT(vsize != 0);

	/* Find U's real size. */
	usize = mp_rsize(u, usize);
	if (usize == 0)
		return;

	if ((rv = mp_cmp(u, usize, v, vsize)) <= 0) {
		/* If U < V, nothing to do. */
		/* If U == V, remainder is zero. */
		if (rv == 0)
			mp_zero(u, vsize);
		return;
	}

	if (vsize == 1) {
		u[0] = mp_dmod(u, usize, v[0]);
		mp_zero(u + 1, usize - 1);
		return;
	}

	/* Normalize operands. XXX assumes V is writable. */
	vs = mp_msb_normalize((mp_digit *)v, vsize);
	u_high = vs ? mp_lshifti(u, usize, vs) : 0;

	u_j = &u[usize - vsize];
	vd = v[vsize - 1];

	for (;;) {
		if (u_high == vd) {
			qhat = ~(mp_digit)0;	/* largest value for an mp_digit */
		} else {
			ASSERT(u_high < vd);

			digit_div(u_high, u_j[vsize - 1], vd, qhat, rhat);
			digit_mul(qhat, v[vsize - 2], q1, q0);
			while ((q1 > rhat) ||
				   (q1 == rhat && q0 > u_j[vsize - 2])) {
				qhat--;
				if ((rhat += vd) < vd)
					break;
				q1 -= q0 < v[vsize - 2];
				q0 -= v[vsize - 2];
			}
		}
		if (u_high < mp_dmul_sub(v, vsize, qhat, u_j))
			mp_addi_n(u_j, v, vsize);
		if (--u_j < u)
			break;
		u_high = u_j[vsize];
	}

	if (vs) {
		mp_rshifti((mp_digit *)v, vsize, vs);
		mp_rshifti(u, vsize, vs);
	}
}
