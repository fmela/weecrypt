/*
 * mp_gcdext.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mp.h"
#include "mp_defs.h"

/* Binary extended GCD algorithm, as given by Knuth in 4.5.2 exercise 39
 * (vol.2 3rd ed. p.646)
 *
 * Given two natural numbers U and V, U >= V, V != 0, this algorithm will find
 * integers U1, U2 and U3 such that U*U1 + V*V2 = U3 = gcd(U,V).
 *
 * Y1. [Find power of 2] Set K = 0, and then repeatedly set K = K + 1,
 *     V = V / 2, U = U / 2 until U and V are both not even.
 * Y2. [Initialize] Set (U1,U2,U3) = (1,0,U) and (V1,V2,V3) = (V,1-U,V).
 *     If U is odd, set (T1,T2,T3) = (0,-1,-V) and go to Y4. Otherwise, set
 *     (T1,T2,T3) = (1,0,U).
 * Y3. [Halve T3] If T1 and T2 are both even, set (T1,T2,T3) = (T1,T2,T3) / 2;
 *     otherwise set (T1,T2,T3) = (T1+V,T2-U,T3) / 2.
 * Y4. [Is T3 even?] If T3 is even, go back to Y3.
 * Y5. [Reset max(U3,V3)] If T3 is positive, set (U1,U2,U3) = (T1,T2,T3);
 *     otherwise set (V1,V2,V3) = (V-T1,-U-T2,-T3).
 * Y6. [Subtract] Set (T1,T2,T3) = (U1,U2,U3) - (V1,V2,V3). Then if T1 <= 0,
 *     set (T1,T2) = (T1+V,T2-U). If T3 != 0, go back to Y3. Otherwise, the
 *     algorithm terminates with (U1,U2,U3*2^K) as the answer.
 *
 * Algorithm invariants:
 *
 *  0 <= U1,V1,T1 <= V
 * -U <= U2,V2,T2 <= 0
 *  0 < U3 <= U, 0 < V3 <= V
 *
 * Thus U1 must point to an area at least as large as V, U2 must point to an
 * area at least as large as U, and U3 must point to an area at least as large
 * as U.
 */

void
mp_gcdext(const mp_digit *u, mp_size ulen, const mp_digit *v, mp_size vlen,
		  mp_digit *u1, mp_digit *u2, mp_digit *u3)
{
	unsigned ki, ks;
	mp_digit tt;
#if 0
	mp_digit *v1, *v2, *v3;
	mp_size v1len, v2len, v3len;
	mp_digit *t1, *t2, *t3;
	mp_size t1len, t2len, t3len;
#endif

	mp_zero(u1, vlen);
	mp_zero(u2, ulen);
	mp_zero(u3, ulen);

	ki = 0;
	while ((tt = u[ki] | v[ki]) == 0)
		ki++;
	if ((tt & 1) == 0) {
		ks = 1;
		while (((tt >>= 1) & 1) == 0)
			ks++;
	} else {
		ks = 0;
	}

	u += ki; ulen -= ki;
	v += ki; vlen -= ki;
	u3 += ki;

	u1[0] = 1;
	u2[0] = 0;
	mp_copy(u, ulen, u3);
}
