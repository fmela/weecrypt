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
	ASSERT(u > 0);
	ASSERT(v > 0);

	unsigned ushift = mp_lsb_shift(u), vshift = mp_lsb_shift(v);
	u >>= ushift;
	v >>= vshift;
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
	mp_zero(w, MIN(usize, vsize));

	usize = mp_rsize(u, usize);
	vsize = mp_rsize(v, vsize);
	if (!usize || !vsize)
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
	unsigned k_skip = 0;
	while ((u[k_skip] | v[k_skip]) == 0)
		k_skip++;
	u += k_skip; usize -= k_skip;
	v += k_skip; vsize -= k_skip;
	w += k_skip;
	const unsigned k_shift = mp_lsb_shift(u[0] | v[0]);

	const mp_size size = MAX(usize, vsize);
	mp_digit *utmp, *vtmp;
	MP_TMP_ALLOC0(utmp, size);
	MP_TMP_ALLOC0(vtmp, size);
	if (k_shift) {
		mp_digit cy = mp_rshift(u, usize, k_shift, utmp);
		ASSERT(cy == 0);
		cy = mp_rshift(v, vsize, k_shift, vtmp);
		ASSERT(cy == 0);
	} else {
		mp_copy(u, usize, utmp);
		mp_copy(v, vsize, vtmp);
	}
	mp_zero(utmp + usize, size - usize);
	mp_zero(vtmp + vsize, size - vsize);

	mp_digit *T;
	int tneg;
	mp_size tsize;
	MP_TMP_ALLOC0(T, size);
	/* B2: */
	if (utmp[0] & 1) {
		mp_copy(vtmp, vsize, T);
		tsize = vsize;
		tneg = 1;
	} else {
		mp_copy(utmp, usize, T);
		tsize = usize;
		tneg = 0;
	}

B3:	/* Combined with B4. */
	if (!(T[0] & 1)) {	/* T is even. */
		unsigned shift = mp_odd_shift(T, tsize);
		unsigned digits = shift / MP_DIGIT_BITS;
		shift %= MP_DIGIT_BITS;
		if (digits) {
			mp_digit cy = mp_rshift(T + digits, tsize, shift, T);
			ASSERT(cy == 0);
			tsize -= digits;
		} else {
			mp_digit cy = mp_rshifti(T, tsize, shift);
			ASSERT(cy == 0);
		}
		tsize -= (T[tsize - 1] == 0);
		ASSERT(tsize > 0);
	}

	/* B5: */
	if (tneg) {
		mp_copy(T, tsize, vtmp);
		vsize = tsize;
	} else {
		mp_copy(T, tsize, utmp);
		usize = tsize;
	}

	usize = mp_rsize(utmp, usize);
	vsize = mp_rsize(vtmp, vsize);
	int cmp = mp_cmp(utmp, usize, vtmp, vsize);
	if (cmp > 0) {
		tsize = usize;
		cmp = mp_sub(utmp, usize, vtmp, vsize, T);
		ASSERT(cmp == 0);
		tneg = 0;
	} else if (cmp < 0) {
		tsize = vsize;
		cmp = mp_sub(vtmp, vsize, utmp, usize, T);
		ASSERT(cmp == 0);
		tneg = 1;
	} else if (cmp == 0) {
		if (k_shift) {
			mp_digit cy = mp_lshift(utmp, usize, k_shift, w);
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
		MP_TMP_FREE(T);
		return;
	}

	tsize = mp_rsize(T, tsize);
	ASSERT(tsize != 0);
	goto B3;
}

bool
mp_coprime(const mp_digit *u, mp_size usize,
		   const mp_digit *v, mp_size vsize)
{
	ASSERT(u != NULL);
	ASSERT(v != NULL);

	usize = mp_rsize(u, usize);
	vsize = mp_rsize(v, vsize);

	if (usize == 0)
		return vsize == 0;	/* Is 0 relatively prime to itself? Who knows. */
	else if (vsize == 0)
		return false;

	mp_size tsize = MIN(usize, vsize);
	mp_digit *tmp;
	MP_TMP_ALLOC(tmp, tsize);
	mp_gcd(u, usize, v, vsize, tmp);
	bool gcd_is_one = mp_is_one(tmp, tsize);
	MP_TMP_FREE(tmp);
	return gcd_is_one;
}
