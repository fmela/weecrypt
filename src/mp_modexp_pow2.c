/*
 * mp_modexp_pow2.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mp.h"
#include "mp_defs.h"

#define MAX_K	8
#define MAX_NK	(1U << MAX_K)

static const uint8_t twotab[256] = {
	0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3,
	0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0,
	1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1,
	0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0,
	2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2,
	0, 1, 0, 7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0,
	1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1,
	0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0,
	3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0,
	1, 0, 2, 0, 1, 0
};

static const uint8_t oddtab[256] = {
	0x00, 0x01, 0x01, 0x03, 0x01, 0x05, 0x03, 0x07, 0x01, 0x09, 0x05, 0x0b,
	0x03, 0x0d, 0x07, 0x0f, 0x01, 0x11, 0x09, 0x13, 0x05, 0x15, 0x0b, 0x17,
	0x03, 0x19, 0x0d, 0x1b, 0x07, 0x1d, 0x0f, 0x1f, 0x01, 0x21, 0x11, 0x23,
	0x09, 0x25, 0x13, 0x27, 0x05, 0x29, 0x15, 0x2b, 0x0b, 0x2d, 0x17, 0x2f,
	0x03, 0x31, 0x19, 0x33, 0x0d, 0x35, 0x1b, 0x37, 0x07, 0x39, 0x1d, 0x3b,
	0x0f, 0x3d, 0x1f, 0x3f, 0x01, 0x41, 0x21, 0x43, 0x11, 0x45, 0x23, 0x47,
	0x09, 0x49, 0x25, 0x4b, 0x13, 0x4d, 0x27, 0x4f, 0x05, 0x51, 0x29, 0x53,
	0x15, 0x55, 0x2b, 0x57, 0x0b, 0x59, 0x2d, 0x5b, 0x17, 0x5d, 0x2f, 0x5f,
	0x03, 0x61, 0x31, 0x63, 0x19, 0x65, 0x33, 0x67, 0x0d, 0x69, 0x35, 0x6b,
	0x1b, 0x6d, 0x37, 0x6f, 0x07, 0x71, 0x39, 0x73, 0x1d, 0x75, 0x3b, 0x77,
	0x0f, 0x79, 0x3d, 0x7b, 0x1f, 0x7d, 0x3f, 0x7f, 0x01, 0x81, 0x41, 0x83,
	0x21, 0x85, 0x43, 0x87, 0x11, 0x89, 0x45, 0x8b, 0x23, 0x8d, 0x47, 0x8f,
	0x09, 0x91, 0x49, 0x93, 0x25, 0x95, 0x4b, 0x97, 0x13, 0x99, 0x4d, 0x9b,
	0x27, 0x9d, 0x4f, 0x9f, 0x05, 0xa1, 0x51, 0xa3, 0x29, 0xa5, 0x53, 0xa7,
	0x15, 0xa9, 0x55, 0xab, 0x2b, 0xad, 0x57, 0xaf, 0x0b, 0xb1, 0x59, 0xb3,
	0x2d, 0xb5, 0x5b, 0xb7, 0x17, 0xb9, 0x5d, 0xbb, 0x2f, 0xbd, 0x5f, 0xbf,
	0x03, 0xc1, 0x61, 0xc3, 0x31, 0xc5, 0x63, 0xc7, 0x19, 0xc9, 0x65, 0xcb,
	0x33, 0xcd, 0x67, 0xcf, 0x0d, 0xd1, 0x69, 0xd3, 0x35, 0xd5, 0x6b, 0xd7,
	0x1b, 0xd9, 0x6d, 0xdb, 0x37, 0xdd, 0x6f, 0xdf, 0x07, 0xe1, 0x71, 0xe3,
	0x39, 0xe5, 0x73, 0xe7, 0x1d, 0xe9, 0x75, 0xeb, 0x3b, 0xed, 0x77, 0xef,
	0x0f, 0xf1, 0x79, 0xf3, 0x3d, 0xf5, 0x7b, 0xf7, 0x1f, 0xf9, 0x7d, 0xfb,
	0x3f, 0xfd, 0x7f, 0xff
};

/* Based on algorithm 1.2.4 from Cohen, "A Course in Computational Algebraic
 * Number Theory," but modified for modular reduction after each squaring or
 * multiplication. */

void
mp_modexp_pow2_ul(const mp_digit *u, mp_size usize, unsigned long power,
				  const mp_digit *m, mp_size msize, mp_digit *w)
{
	mp_digit *tmp, *up[MAX_NK];
	unsigned a, j, t, f, k, nk;

	ASSERT(u != NULL);
	ASSERT(m != NULL);
	ASSERT(w != NULL);

	mp_zero(w, msize);
	msize = mp_rsize(m, msize);
	ASSERT(msize != 0);

	if (msize == 1 && m[0] == 1)
		return;

	usize = mp_rsize(u, usize);
	if (usize == 0 || power == 0) {
		w[0] = (usize != 0);
		return;
	}

	if (power == 1) {
		mp_mod(u, usize, m, msize, w);
		return;
	}

	if (power == 2) {
		MP_TMP_ALLOC(tmp, usize * 2);
		mp_sqr(u, usize, tmp);
		mp_modi(tmp, usize * 2, m, msize);
		mp_copy(tmp, msize, w);
		MP_TMP_FREE(tmp);
		return;
	}

	/* Precompute U mod M */
	mp_mod(u, usize, m, msize, w);
	if (mp_rsize(w, msize) == 0)
		return;

	k = MAX_K;
	nk = 1U << k;

	for (j = 0; j < nk; j++)
		up[j] = NULL;

	MP_TMP_COPY(up[1], w, msize);

	MP_TMP_ALLOC(tmp, msize * 2);
	mp_sqr(up[1], msize, tmp);
	mp_modi(tmp, msize * 2, m, msize);
	MP_TMP_COPY(up[2], tmp, msize);

	/* Precompute U^3 mod M, U^5 mod M, ... U^(2^K-1) mod M */
	for (j = 3; j < nk; j += 2) {
		mp_mul_n(up[2], up[j-2], msize, tmp);
		mp_modi(tmp, msize * 2, m, msize);
		MP_TMP_COPY(up[j], tmp, msize);
	}

	a = power;
	for (f = 0; a != 0; f++)
		a >>= k;

	a = (power >> ((f-1) * k)) & (nk - 1);
	ASSERT(a != 0);
	t = twotab[a];
	a = oddtab[a];
	mp_copy(up[a], msize, w);
	for (j = t; j != 0; j--) {
		mp_sqr(w, msize, tmp);
		mp_modi(tmp, msize * 2, m, msize);
		mp_copy(tmp, msize, w);
	}

	while (--f != 0) {
		a = (power >> ((f-1) * k)) & (nk - 1);
		if (a == 0) {
			for (j = k; j != 0; j--) {
				mp_sqr(w, msize, tmp);
				mp_modi(tmp, msize * 2, m, msize);
				mp_copy(tmp, msize, w);
			}
		} else {
			t = twotab[a];
			a = oddtab[a];
			for (j = k - t; j != 0; j--) {
				mp_sqr(w, msize, tmp);
				mp_modi(tmp, msize * 2, m, msize);
				mp_copy(tmp, msize, w);
			}
			mp_mul_n(w, up[a], msize, tmp);
			mp_modi(tmp, msize * 2, m, msize);
			mp_copy(tmp, msize, w);
			for (j = t; j != 0; j--) {
				mp_sqr(w, msize, tmp);
				mp_modi(tmp, msize * 2, m, msize);
				mp_copy(tmp, msize, w);
			}
		}
	}

	MP_TMP_FREE(up[1]);
	MP_TMP_FREE(up[2]);
	for (j = 3; j < nk; j += 2)
		if (up[j])
			MP_TMP_FREE(up[j]);
	MP_TMP_FREE(tmp);
}

/*  Input: u[0..usize-1], p[0..psize-1], m[0..msize-1]
 * Output: w[0..msize-1] = (u ** p) mod m */
void
mp_modexp_pow2(const mp_digit *u, mp_size usize,
			   const mp_digit *p, mp_size psize,
			   const mp_digit *m, mp_size msize, mp_digit *w)
{
	mp_digit *tmp, *up[MAX_NK], mask;
	unsigned a, j, t, k, nk, b;

	ASSERT(u != NULL);
	ASSERT(m != NULL);
	ASSERT(w != NULL);

	mp_zero(w, msize);
	msize = mp_rsize(m, msize);
	ASSERT(msize != 0);

	if (msize == 1 && m[0] == 1)
		return;

	usize = mp_rsize(u, usize);
	psize = mp_rsize(p, psize);
	if (usize == 0 || psize == 0) {
		w[0] = (usize != 0);
		return;
	}

	if (psize == 1) {
		if (p[0] == 1) {
			mp_mod(u, usize, m, msize, w);
			return;
		}
		if (p[0] == 2) {
			MP_TMP_ALLOC(tmp, usize * 2);
			mp_sqr(u, usize, tmp);
			mp_modi(tmp, usize * 2, m, msize);
			mp_copy(tmp, msize, w);
			MP_TMP_FREE(tmp);
			return;
		}
	}

	/* Precompute U mod M */
	mp_mod(u, usize, m, msize, w);
	if (mp_rsize(w, msize) == 0)
		return;

	/* Choose suitable value of K */
	b = mp_significant_bits(p, psize) - 1;
	for (k = MAX_K; k >= 2; k--) {
		if (((k - 1) * (k << ((k - 1) << 1)) / ((1 << k) - k - 1)) < b)
			break;
	}
	nk = 1U << k;

	for (j = 0; j < nk; j++)
		up[j] = NULL;

	MP_TMP_COPY(up[1], w, msize);

	MP_TMP_ALLOC(tmp, msize * 2);
	mp_sqr(up[1], msize, tmp);
	mp_modi(tmp, msize * 2, m, msize);
	MP_TMP_COPY(up[2], tmp, msize);

	/* Precompute U^3 mod M, U^5 mod M, ... U^(2^K-1) mod M */
	for (j = 3; j < nk; j += 2) {
		mp_mul_n(up[2], up[j-2], msize, tmp);
		MP_TMP_ALLOC(up[j], msize);
		mp_mod(tmp, msize * 2, m, msize, up[j]);
	}

	b = mp_significant_bits(p, psize);
	if (b % k)
		b += k - (b % k);
	ASSERT(b % k == 0);

	a = 0;
	mask = (mp_digit)1 << (b % MP_DIGIT_BITS);
	for (j = k; j != 0; j--) {
		ASSERT(b != 0);
		b -= 1;
		if ((mask >>= 1) == 0)
			mask = MP_DIGIT_MSB;
		if ((b / MP_DIGIT_BITS) >= psize)
			continue;
		a <<= 1;
		if (p[b / MP_DIGIT_BITS] & mask)
			a |= 1;
	}
	ASSERT(a != 0);
	t = twotab[a];
	a = oddtab[a];
	mp_copy(up[a], msize, w);
	for (j = 0; j < t; j++) {
		mp_sqr(w, msize, tmp);
		mp_modi(tmp, msize * 2, m, msize);
		mp_copy(tmp, msize, w);
	}

	while (b != 0) {
		a = 0;
		for (j = k; j != 0; j--) {
			ASSERT(b != 0);
			b -= 1;
			if ((mask >>= 1) == 0)
				mask = MP_DIGIT_MSB;
			a <<= 1;
			if (p[b / MP_DIGIT_BITS] & mask)
				a |= 1;
		}
		if (a == 0) {
			for (j = k; j != 0; j--) {
				mp_sqr(w, msize, tmp);
				mp_modi(tmp, msize * 2, m, msize);
				mp_copy(tmp, msize, w);
			}
		} else {
			t = twotab[a];
			a = oddtab[a];
			for (j = k - t; j != 0; j--) {
				mp_sqr(w, msize, tmp);
				mp_modi(tmp, msize * 2, m, msize);
				mp_copy(tmp, msize, w);
			}
			mp_mul_n(w, up[a], msize, tmp);
			mp_modi(tmp, msize * 2, m, msize);
			mp_copy(tmp, msize, w);
			for (j = t; j != 0; j--) {
				mp_sqr(w, msize, tmp);
				mp_modi(tmp, msize * 2, m, msize);
				mp_copy(tmp, msize, w);
			}
		}
	}

	for (j = 0; j < nk; j++)
		if (up[j])
			MP_TMP_FREE(up[j]);
	MP_TMP_FREE(tmp);
}
