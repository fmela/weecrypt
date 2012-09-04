/* mp_lehmer.c
 * Copyright (C) 2002-2012 Farooq Mela. All rights reserved. */

#include "mp.h"
#include "mp_defs.h"

/* Compute g[min(usize,vsize)] = gcd(u,v) using Lehmer's algorithm.
 * Neither A or B may be 0 */
void
mp_lehmer(const mp_digit *u, mp_size usize,
	  const mp_digit *v, mp_size vsize, mp_digit *g)
{
    int rv;
    mp_digit uhat, vhat, A, B, C, D, T, q, q2;
    mp_digit *utmp, *vtmp, *t = NULL, *w = NULL;

    ASSERT(usize != 0);
    ASSERT(u[usize - 1] != 0);
    ASSERT(vsize != 0);
    ASSERT(v[vsize - 1] != 0);

    /* Arrange A >= B */
    rv = mp_cmp(u, usize, v, vsize);
    if (rv == 0) {
	mp_copy(u, MIN(usize, vsize), g);
	return;
    }
    if (rv < 0) {
	SWAP(u, v, const mp_digit *);
	SWAP(usize, vsize, mp_size);
    }

    utmp = MP_TMP_ALLOC(usize + 1);
    mp_copy(u, usize, utmp);
    utmp[usize] = 0;
    u = NULL;

    vtmp = MP_TMP_ALLOC(vsize + 1);
    mp_copy(v, vsize, vtmp);
    vtmp[vsize] = 0;
    v = NULL;

    for (;;) {
	mp_digit uu, vv, dummy;

	MP_NORMALIZE(utmp, usize);
	MP_NORMALIZE(vtmp, vsize);
	rv = mp_cmp(utmp, usize, vtmp, vsize);
	if (rv < 0) {
	    SWAP(utmp, vtmp, mp_digit *);
	    SWAP(usize, vsize, mp_size);
	}

	if (vsize <= 1) {
	    /* Handle this case with single-precision Euclidean. */
	    mp_digit r;

	    vv = vtmp[0];
	    if (vv == 0) {	/* gcd(u,0) = u */
		mp_copy(utmp, usize, g);
		MP_TMP_FREE(utmp);
		MP_TMP_FREE(vtmp);
		return;
	    }

	    if ((r = mp_dmod(utmp, usize, vv)) == 0) {
		g[0] = vv;
		MP_TMP_FREE(utmp);
		MP_TMP_FREE(vtmp);
		return;
	    }
	    uu = vv; vv = r;
	    for (;;) {
		if ((r = uu % vv) == 0)
		    break;
		uu = vv;
		vv = r;
	    }
	    g[0] = vv;
	    MP_TMP_FREE(utmp);
	    MP_TMP_FREE(vtmp);
	    return;
	}

	uhat = utmp[usize - 1];
	vhat = (usize == vsize) ? vtmp[vsize - 1] : 0;
	A = 1;
	B = 0;
	C = 0;
	D = 1;

	for (;;) {
	    printf("ul=%u vl=%u uhat=" MP_FORMAT
		   " vhat=" MP_FORMAT" A=" MP_FORMAT " B=" MP_FORMAT
		   " C=" MP_FORMAT " D=" MP_FORMAT "\n",
		   usize, vsize, uhat, vhat, A, B, C, D);
	    /* If \^v + C = 0 or \^v + D = 0, do multiprecision step, because
	     * either of these cases would result in division by zero. */
	    if (vhat == 0 && (C == 0 || D == 0))
		break;

	    /* q = (uhat+A)/(vhat+C). (uhat+A) can overflow, (vhat+C) can't */
	    uu = uhat + A;
	    vv = vhat + C;
	    ASSERT(vv >= C);
	    if (uu < A) {
		ASSERT(uu == 0);
		printf("uu overflow\n");
		digit_div(1, uu, vv, q, dummy);
	    } else {
		q = uu / vv;
	    }

	    /* q2 = (uhat+B)/(vhat+D). (vhat+D) can overflow, (uhat+B) can't */
	    uu = uhat + B;
	    vv = vhat + D;
	    ASSERT(uu >= B);
	    if (vv < D) { /* if (vhat+D) overflows q2 must = 0 */
		printf("vv overflow\n");
		q2 = 0;
	    } else {
		q2 = uu / vv;
	    }

	    printf("q1=" MP_FORMAT " q2=" MP_FORMAT "\n", q, q2);

	    if (q != q2)
		break;

	    printf("doing single precision\n");
	    /* Single-precision step. */
	    ASSERT(A >= q * C);
	    T = A - q * C;
	    A = C;
	    C = T;

	    ASSERT(B >= q * D);
	    T = B - q * D;
	    B = D;
	    D = T;

	    ASSERT(uhat >= q * vhat);
	    T = uhat - q * vhat;
	    uhat = vhat;
	    vhat = T;
	}

	/* Multi-precision step. */
	if (B == 0) {	/* Happens very rarely. */
	    printf("Doing multiprecision step #1\n");
	    /* Set t = u mod v, u = v, v = t */
	    mp_modi(utmp, usize, vtmp, vsize);	/* u[0..vsize-1] = u % v */
	    mp_xchg(utmp, vtmp, vsize);			/* u <=> v */
	    usize = vsize;
	    MP_NORMALIZE(vtmp, vsize);
	} else {
	    /* t = A*u, t = t+B*v, w = C*u, w = w+D*v, u = t, v = w */
	    mp_digit cy;

	    printf("Doing multiprecision step #2\n");

	    if (t == NULL)
		t = MP_TMP_ALLOC(usize + 1);
	    t[usize] = mp_dmul(utmp, usize, A, t);
	    cy = mp_dmul_add(vtmp, vsize, B, t);
	    if (cy)
		cy = mp_daddi(t + vsize, usize + 1 - vsize, cy);
	    ASSERT(cy == 0);

	    if (w == NULL)
		w = MP_TMP_ALLOC(usize + 1);
	    w[usize] = mp_dmul(utmp, usize, C, w);
	    cy = mp_dmul_add(vtmp, vsize, D, w);
	    if (cy)
		cy = mp_daddi(w + vsize, usize + 1 - vsize, cy);
	    ASSERT(cy == 0);

	    usize += (t[usize] != 0);
	    mp_copy(t, usize, utmp);

	    vsize += (t[vsize] != 0);
	    mp_copy(w, vsize, vtmp);
	}
    }
}
