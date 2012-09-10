/* mp_modi.c
 * Copyright (C) 2002-2012 Farooq Mela. All rights reserved. */

#include "mp.h"
#include "mp_internal.h"

/* Knuth's 4.3.1D, but ignoring quotient */
void
mp_modi(mp_digit *u, mp_size usize, const mp_digit *v, mp_size vsize)
{
    ASSERT(u != NULL);
    ASSERT(v != NULL);

    /* V cannot be zero. */
    MP_NORMALIZE(v, vsize);
    ASSERT(vsize != 0);

    /* Find U's real size. */
    MP_NORMALIZE(u, usize);
    if (usize == 0)
	return;

    int cmp = mp_cmp(u, usize, v, vsize);
    if (cmp <= 0) {
	/* If U < V, nothing to do. */
	/* If U == V, remainder is zero. */
	if (cmp == 0)
	    mp_zero(u, vsize);
	return;
    }

    if (vsize == 1) {
	u[0] = mp_dmod(u, usize, v[0]);
	mp_zero(u + 1, usize - 1);
	return;
    }

    /* Normalize operands. XXX assumes V is writable. */
    const unsigned vshift = mp_msb_normalize((mp_digit *)v, vsize);
    mp_digit u_high = vshift ? mp_lshifti(u, usize, vshift) : 0;

    mp_digit *u_j = &u[usize - vsize];
    const mp_digit vd = v[vsize - 1];

    for (;;) {
	mp_digit qhat;
	if (u_high == vd) {
	    qhat = ~(mp_digit)0;	/* largest value for an mp_digit */
	} else {
	    ASSERT(u_high < vd);

	    mp_digit rhat;
	    digit_div(u_high, u_j[vsize - 1], vd, qhat, rhat);
	    mp_digit q1, q0;
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

    if (vshift) {
	mp_rshifti((mp_digit *)v, vsize, vshift);
	mp_rshifti(u, vsize, vshift);
    }
}
