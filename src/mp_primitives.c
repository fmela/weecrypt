/* mp_primitives.c
 * Copyright (C) 2002-2012 Farooq Mela. All rights reserved. */

#include "mp.h"
#include "mp_internal.h"

#define LOHALF(u)	((u) & MP_DIGIT_LMASK)
#define HIHALF(u)	((u) >> MP_DIGIT_HSHIFT)
#define COMBINE(hi,lo)	(((hi) << MP_DIGIT_HSHIFT) | (lo))

void
mp_digit_mul(mp_digit u, mp_digit v, mp_digit *h, mp_digit *l)
{
    mp_digit u1 = HIHALF(u);
    mp_digit u0 = LOHALF(u);
    mp_digit v1 = HIHALF(v);
    mp_digit v0 = LOHALF(v);

    mp_digit lo = u0 * v0;
    mp_digit hi = u1 * v1;

    u1 *= v0; v0 = u1 << MP_DIGIT_HSHIFT; u1 >>= MP_DIGIT_HSHIFT;
    hi += u1 + ((lo += v0) < v0);

    v1 *= u0; u0 = v1 << MP_DIGIT_HSHIFT; v1 >>= MP_DIGIT_HSHIFT;
    hi += v1 + ((lo += u0) < u0);

    *h = hi;
    *l = lo;
}

void
mp_digit_sqr(mp_digit u, mp_digit *h, mp_digit *l)
{
    mp_digit u1 = HIHALF(u);
    mp_digit u0 = LOHALF(u);

    mp_digit lo = u0 * u0;
    mp_digit hi = u1 * u1;

    u1 *= u0; u0 = u1 << (MP_DIGIT_HSHIFT + 1); u1 >>= (MP_DIGIT_HSHIFT - 1);
    hi += u1 + ((lo += u0) < u0);

    *h = hi;
    *l = lo;
}

void
mp_digit_div(mp_digit n1, mp_digit n0, mp_digit d,
	      mp_digit *q, mp_digit *r)
{
    ASSERT(d != 0);
    ASSERT(n1 < d);	/* This must be true for single-digit quotient. */

    if (n1 == 0) {
	/* Simple case. */
	if (n0 < d) {
	    *q = 0;
	    *r = n0;
	} else {
	    *q = n0 / d;
	    *r = n0 % d;
	}
	return;
    }

    const unsigned msb_shift = mp_digit_msb_shift(d);
    if (msb_shift != 0) {	/* Normalize divisor. */
	d  <<= msb_shift;
	n1 = (n1 << msb_shift) | (n0 >> (MP_DIGIT_BITS - msb_shift));
	n0 <<= msb_shift;
    }

    mp_digit d1 = HIHALF(d);
    mp_digit d0 = LOHALF(d);

    mp_digit q1 = n1 / d1;
    mp_digit r1 = n1 % d1;
    mp_digit m = q1 * d0;
    r1 = COMBINE(r1, HIHALF(n0));
    if (r1 < m) {
	--q1, r1 += d;
	if (r1 >= d && r1 < m)
	    --q1, r1 += d;
    }
    r1 -= m;

    mp_digit q0 = r1 / d1;
    mp_digit r0 = r1 % d1;
    m = q0 * d0;
    r0 = COMBINE(r0, LOHALF(n0));
    if (r0 < m) {
	--q0, r0 += d;
	if (r0 >= d && r0 < m)
	    --q0, r0 += d;
    }
    r0 -= m;

    *q = COMBINE(q1, q0);
    *r = r0 >> msb_shift;
}
