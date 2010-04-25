/*
 * mp_gcd.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mp.h"
#include "mp_defs.h"

static mp_digit
dgcd(mp_digit u, mp_digit v)
{
	unsigned ushift, vshift;

	ASSERT(u != 0);
	ASSERT(v != 0);

	for (ushift = 0; (u & 1) == 0; ushift++)
		u >>= 1;
	for (vshift = 0; (v & 1) == 0; vshift++)
		v >>= 1;

	while (u != v) {
		if (u > v) {
			for (u = (u - v) >> 1; (u & 1) == 0; u >>= 1)
				/* void */;
		} else {
			for (v = (v - u) >> 1; (v & 1) == 0; v >>= 1)
				/* void */;
		}
	}
	return u << MIN(ushift, vshift);
}

/* Binary GCD algorithm, as given by Knuth (4.5.2B, vol.2, 3rd ed. p.338)
 *
 * For natural numbers U and V:
 * B1. [Find power of 2] Set K = 0, and then repeatedly set K = K + 1,
 *     V = V / 2, U = U / 2 until U and V are not both even.
 * B2. [Initialize] If U is odd, set T = -V and goto B4. Otherwise, set T = U.
 * B3. [Halve T] T = T / 2.
 * B4. [T Even?] If T is even, goto B3.
 * B5. [Reset max(U,V)] If T > 0, set U = T, otherwise set V = -T.
 * B6. [Subtract] Set T = U - V. If T is non-zero, go back to B3. Otherwise,
 *     algorithm terminates with U * 2^K as answer.
 *
 * The advantage of this algorithm is that it operates entirely without
 * division; it uses only the elementary operations of parity testing, shifting
 * and subtracting. */
void
mp_gcd(const mp_digit *u, mp_size usize,
	   const mp_digit *v, mp_size vsize, mp_digit *w)
{
	unsigned ki, ks;

	mp_digit *utmp, *vtmp;
	mp_digit *t;
	int tneg, rv;
	mp_size tsize;
	mp_size size;

	mp_zero(w, MIN(usize, vsize));

	usize = mp_rsize(u, usize);
	vsize = mp_rsize(v, vsize);
	if (usize == 0 || vsize == 0)
		return; /* Don't handle these. */

	if (usize == 1 || vsize == 1) {
		mp_digit a, b, r;

		if (usize == 1 && vsize == 1) {
			a = u[0]; b = v[0];
		} else {
			if (usize == 1) {
				if ((r = mp_dmod(v, vsize, u[0])) == 0) {
					w[0] = u[0];
					return;
				}
				a = u[0];
			} else {
				if ((r = mp_dmod(u, usize, v[0])) == 0) {
					w[0] = v[0];
					return;
				}
				a = v[0];
			}
			b = r;
		}

#if 0
		/* Classic Euclidean GCD algorithm; replace with single-precision
		 * binary method? */
		for (;;) {
			if ((r = a % b) == 0) {
				w[0] = b;
				return;
			}
			a = b;
			b = r;
		}
#endif
		w[0] = dgcd(a, b);
		return;
	}

	/* B1: */
	ki = 0;
	while ((u[ki] | v[ki]) == 0)
		ki++;
	if (((u[ki] | v[ki]) & 1) == 0) {
		mp_digit uk = u[ki] | v[ki];

		ks = 1;
		while (((uk >>= 1) & 1) == 0)
			ks++;
	} else {
		ks = 0;
	}
	u += ki; usize -= ki;
	v += ki; vsize -= ki;
	w += ki;

	size = MAX(usize, vsize);
	MP_TMP_ALLOC0(utmp, size);
	MP_TMP_ALLOC0(vtmp, size);
	MP_TMP_ALLOC0(t, size);
	mp_copy(u, usize, utmp);
	mp_copy(v, vsize, vtmp);
	if (ks) {
		mp_digit cy;
		cy = mp_rshifti(utmp, usize, ks);
		ASSERT(cy == 0);
		cy = mp_rshifti(vtmp, vsize, ks);
		ASSERT(cy == 0);
	}

	/* B2: */
	if (utmp[0] & 1) {
		mp_copy(vtmp, vsize, t);
		tsize = vsize;
		tneg = 1;
		goto B4;
	} else {
		mp_copy(utmp, usize, t);
		tsize = usize;
		tneg = 0;
	}

B3:
	mp_rshifti(t, tsize, 1);
	tsize -= (t[tsize - 1] == 0);
	ASSERT(tsize != 0);
B4:
	if ((t[0] & 1) == 0)
		goto B3;

	/* B5: */
	if (tneg) {
		mp_copy(t, tsize, vtmp);
		vsize = tsize;
	} else {
		mp_copy(t, tsize, utmp);
		usize = tsize;
	}

	usize = mp_rsize(utmp, usize);
	vsize = mp_rsize(vtmp, vsize);
	rv = mp_cmp(utmp, usize, vtmp, vsize);
	if (rv > 0) {
		tsize = usize;
		rv = mp_sub(utmp, usize, vtmp, vsize, t);
		ASSERT(rv == 0);
		tneg = 0;
	} else if (rv < 0) {
		tsize = vsize;
		rv = mp_sub(vtmp, vsize, utmp, usize, t);
		ASSERT(rv == 0);
		tneg = 1;
	} else if (rv == 0) {
		if (ks) {
			mp_digit cy;

			cy = mp_lshift(utmp, usize, ks, w);
			/* Do NOT remove this test for cy != 0. If U and V are of the same
			 * size then W is the same size, and we cannot write past the
			 * ULEN-th digit. */
			if (cy)
				w[usize] = cy;
		} else {
			mp_copy(utmp, usize, w);
		}
		MP_TMP_FREE(utmp);
		MP_TMP_FREE(vtmp);
		MP_TMP_FREE(t);
		return;
	}

	tsize = mp_rsize(t, tsize);
	ASSERT(tsize != 0);
	goto B3;
}

int
mp_coprime(const mp_digit *u, mp_size usize,
		   const mp_digit *v, mp_size vsize)
{
	int rv;
	mp_digit *tmp;
	mp_size tsize;

	ASSERT(u != NULL);
	ASSERT(v != NULL);

	usize = mp_rsize(u, usize);
	vsize = mp_rsize(v, vsize);

	if (usize == 0)
		return vsize == 0;	/* Is 0 relatively prime to itself? Who knows. */
	else if (vsize == 0)
		return 0;

	tsize = MIN(usize, vsize);
	MP_TMP_ALLOC(tmp, tsize);
	mp_gcd(u, usize, v, vsize, tmp);
	rv = mp_is_one(tmp, tsize);
	MP_TMP_FREE(tmp);

	return rv;
}
