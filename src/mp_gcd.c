/* mp_gcd.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved. */

#include "mp.h"
#include "mp_defs.h"

mp_digit
mp_digit_gcd(mp_digit u, mp_digit v)
{
	if (u == 0)
		return v;
	if (v == 0)
		return u;

	const unsigned shift = mp_digit_lsb_shift(u | v);
	u >>= mp_digit_lsb_shift(u);
	v >>= mp_digit_lsb_shift(v);
	while (u != v) {
		if (u > v) {
			u -= v;
			u >>= mp_digit_lsb_shift(u);
		} else {
			v -= u;
			v >>= mp_digit_lsb_shift(v);
		}
	}
	return u << shift;
}

/* Adapted from the binary GCD algorithm 4.5.2B by Knuth in The Art of Computer
 * Programming Vol.2 3rd ed. p.338.
 *
 * The advantage of this algorithm is that it operates entirely without
 * division; it uses only the elementary operations of parity testing, shifting
 * and subtracting. */
void
mp_gcd(const mp_digit *u, mp_size usize,
	   const mp_digit *v, mp_size vsize, mp_digit *w)
{
	ASSERT(u);
	ASSERT(usize > 0);
	ASSERT(v);
	ASSERT(vsize > 0);
	ASSERT(u != w);
	ASSERT(v != w);

	const mp_size result_size = MIN(usize, vsize);
	mp_zero(w, result_size);

	MP_NORMALIZE(u, usize);
	MP_NORMALIZE(v, vsize);
	if (usize == 0) {
		/* Return V. */
		mp_copy(v, vsize, w);
		return;
	} else if (vsize == 0) {
		/* Return U. */
		mp_copy(u, usize, w);
		return;
	}

	if (usize == 1 || vsize == 1) {
		if (usize == 1 && vsize == 1) {
			w[0] = mp_digit_gcd(u[0], v[0]);
		} else {
			if (usize == 1) {
				SWAP(u, v, const mp_digit *);
				SWAP(usize, vsize, mp_size);
			}
			ASSERT(vsize == 1);
			mp_digit rem = mp_dmod(u, usize, v[0]);
			if (rem == 0) {
				w[0] = v[0];
			} else {
				w[0] = mp_digit_gcd(v[0], rem);
			}
		}
		return;
	}

	if ((u[0] | v[0]) == 0) {
		unsigned digit_skip = 1;
		while ((u[digit_skip] | v[digit_skip]) == 0)
			++digit_skip;
		u += digit_skip; usize -= digit_skip;
		v += digit_skip; vsize -= digit_skip;
		w += digit_skip;
	}
	const unsigned digit_shift = mp_digit_lsb_shift(u[0] | v[0]);

	while (u[0] == 0)
		++u, --usize;
	while (v[0] == 0)
		++v, --vsize;

	mp_digit *tmp;
	MP_TMP_ALLOC(tmp, usize + vsize);
	mp_digit *utmp = tmp;
	mp_digit *vtmp = tmp + usize;

	ASSERT(u[0]);
	unsigned shift = mp_digit_lsb_shift(u[0]);
	if (shift) {
		ASSERT(mp_rshift(u, usize, shift, utmp) == 0);
		usize -= (utmp[usize - 1] == 0);
	} else {
		mp_copy(u, usize, utmp);
	}
	ASSERT(v[0]);
	shift = mp_digit_lsb_shift(v[0]);
	if (shift) {
		ASSERT(mp_rshift(v, vsize, shift, vtmp) == 0);
		vsize -= (vtmp[vsize - 1] == 0);
	} else {
		mp_copy(v, vsize, vtmp);
	}

	for (;;) {
		const int cmp = mp_cmp(utmp, usize, vtmp, vsize);
		if (cmp > 0) {
			ASSERT(mp_subi(utmp, usize, vtmp, vsize) == 0);
			MP_NORMALIZE(utmp, usize);
			while (*utmp == 0)
				++utmp, --usize;
			ASSERT(utmp[0]);
			shift = mp_digit_lsb_shift(utmp[0]);
			if (shift) {
				ASSERT(mp_rshifti(utmp, usize, shift) == 0);
				usize -= (utmp[usize - 1] == 0);
			}
		} else if (cmp < 0) {
			ASSERT(mp_subi(vtmp, vsize, utmp, usize) == 0);
			MP_NORMALIZE(vtmp, vsize);
			while (*vtmp == 0)
				++vtmp, --vsize;
			ASSERT(vtmp[0]);
			shift = mp_digit_lsb_shift(vtmp[0]);
			if (shift) {
				ASSERT(mp_rshifti(vtmp, vsize, shift) == 0);
				vsize -= (vtmp[vsize - 1] == 0);
			}
		} else {
			ASSERT(cmp == 0);
			if (digit_shift) {
				mp_digit cy = mp_lshift(utmp, usize, digit_shift, w);
				if (cy)
					w[usize] = cy;
			} else {
				if (utmp != w)
					mp_copy(utmp, usize, w);
			}
			break;
		}
	}
	MP_TMP_FREE(tmp);
}

bool
mp_coprime(const mp_digit *u, mp_size usize,
		   const mp_digit *v, mp_size vsize)
{
	ASSERT(u != NULL);
	ASSERT(v != NULL);

	MP_NORMALIZE(u, usize);
	MP_NORMALIZE(v, vsize);

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
