/*
 * mp_mexp.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mp.h"
#include "mp_defs.h"

#define MAX_K	8
#define MAX_NK	(1U << MAX_K)

/* Decompose an unsigned 8-bit integer N into the form 2^K * Q, Q odd:
 * K = pow2tab[N], Q = odd_tab[N] */
static const uint8_t pow2tab[256] = {
	0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};

static const uint8_t odd_tab[256] = {
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

/* Montgomery product, as given by Dusse & Kaliski:
 *  Input: a[0 .. s-1], b[0 .. s-1], n[0 .. s-1], n odd.
 * Output: w[0 .. s-1]
 * 1. Set t = a * b, n0' = -n^-1 mod B
 * 2. For i = 0 to s-1 do
 *      m = (t[i] * n0') mod B
 *      t = t + n * m * B^i
 * 3. Set t = t / B^s; if t >= n, t -= n
 * 4. Output w = t
 *
 * TODO: Implement other operand scanning method(s) suggested in "Analyzing and
 * Comparing Montgomery Multiplication Algorithms", by Koc, Acar & Kaliski,
 * but it may not give as much performance increase as it would on a DSP, and
 * it would be a pain to implement as it would have to be in assembly for any
 * real gain to be made */

/*  Input: t[0..2s-1] = a * b, n[0..s-1] = modulus, n0_inv = -n^-1 mod B
 * Output: t[0.. s-1] = 0, t[s..2s-1] = montgomery product */
static void
redc(mp_digit *t, const mp_digit *n, mp_digit n0_inv, mp_size s)
{
	mp_size i;
	mp_digit m, cy, co;

	cy = 0;
	for (i = 0; i < s; i++) {
		m = t[i] * n0_inv;
		co = mp_dmul_add(n, s, m, &t[i]);
		if (co)
			cy += mp_daddi(&t[s + i], s - i, co);
		ASSERT(t[i] == 0);
	}

	t += s;
	if (cy || mp_cmp_n(t, n, s) >= 0) {
		cy -= mp_subi_n(t, n, s);
		ASSERT(cy == 0);
	}
}

/* Base 2^k exponentiation with Montgomery modular reduction */
void
mon_exp_2k(const mp_digit *u, mp_size usize,
		   const mp_digit *p, mp_size psize,
		   const mp_digit *m, mp_size msize, mp_digit *w)
{
	ASSERT(usize != 0);
	ASSERT(u[usize - 1] != 0);

	ASSERT(psize != 0);
	ASSERT(p[psize - 1] != 0);

	ASSERT(msize != 0);
	ASSERT(m[msize - 1] != 0);
	ASSERT((m[0] & 1) == 1);

	/* Compute u*r mod m */
	const mp_size tmp_size = msize + MAX(usize, msize);
	mp_digit *tmp = MP_TMP_ALLOC(tmp_size);
	mp_digit *tmp2 = tmp + msize;
	mp_zero(tmp, msize);
	mp_copy(u, usize, tmp2);
	mp_modi(tmp, usize + msize, m, msize);
	mp_copy(tmp, msize, w);

	/* Choose optimal value of K */
	unsigned b = mp_significant_bits(p, psize);
	unsigned k = MAX_K;
	for (; k >= 2; k--) {
		if (((k - 1) * (k << ((k - 1) << 1)) / ((1U << k) - k - 1)) < (b - 1))
			break;
	}
	unsigned nk = 1U << k;

	mp_digit m0_inv = -mp_digit_invert(m[0]);

	mp_digit *up[MAX_NK] = { NULL };
	up[1] = MP_TMP_COPY(w, msize);

	mp_sqr(up[1], msize, tmp);
	redc(tmp, m, m0_inv, msize);
	up[2] = MP_TMP_COPY(tmp2, msize);

	/* Precompute U^3 mod M, U^5 mod M, ... U^(2^K-1) mod M */
	for (unsigned j = 3; j < nk; j += 2) {
		mp_mul_n(up[2], up[j-2], msize, tmp);
		redc(tmp, m, m0_inv, msize);
		up[j] = MP_TMP_COPY(tmp2, msize);
	}

	unsigned t = b % k;
	if (t != 0)
		b += k - (b % k);
	ASSERT(b % k == 0);

	unsigned a = 0;
	mp_digit mask = (mp_digit)1 << (b % MP_DIGIT_BITS);
	for (unsigned j = k; j != 0; j--) {
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
	t = pow2tab[a];
	a = odd_tab[a];
	mp_copy(up[a], msize, w);
	for (unsigned j = t; j != 0; j--) {
		mp_sqr(w, msize, tmp);
		redc(tmp, m, m0_inv, msize);
		mp_copy(tmp2, msize, w);
	}

	while (b != 0) {
		a = 0;
		for (unsigned j = k; j != 0; j--) {
			ASSERT(b != 0);
			b -= 1;
			if ((mask >>= 1) == 0)
				mask = MP_DIGIT_MSB;
			a <<= 1;
			if (p[b / MP_DIGIT_BITS] & mask)
				a |= 1;
		}
		if (a == 0) {
			for (unsigned j = k; j != 0; j--) {
				mp_sqr(w, msize, tmp);
				redc(tmp, m, m0_inv, msize);
				mp_copy(tmp2, msize, w);
			}
		} else {
			t = pow2tab[a];
			a = odd_tab[a];
			for (unsigned j = k - t; j != 0; j--) {
				mp_sqr(w, msize, tmp);
				redc(tmp, m, m0_inv, msize);
				mp_copy(tmp2, msize, w);
			}
			mp_mul_n(w, up[a], msize, tmp);
			redc(tmp, m, m0_inv, msize);
			mp_copy(tmp2, msize, w);
			for (unsigned j = t; j != 0; j--) {
				mp_sqr(w, msize, tmp);
				redc(tmp, m, m0_inv, msize);
				mp_copy(tmp2, msize, w);
			}
		}
	}

	MP_TMP_FREE(up[1]);
	MP_TMP_FREE(up[2]);
	for (unsigned j = 3; j < nk; j += 2)
		MP_TMP_FREE(up[j]);

	mp_copy(w, msize, tmp);
	mp_zero(tmp2, msize);
	redc(tmp, m, m0_inv, msize);
	mp_copy(tmp2, msize, w);

	MP_TMP_FREE(tmp);
}

#define MODI

/* Base 2^k exponentiation with modular reduction by division */
void
mod_exp_2k(const mp_digit *u, mp_size usize,
		   const mp_digit *p, mp_size psize,
		   const mp_digit *m, mp_size msize, mp_digit *w)
{
	ASSERT(usize != 0);
	ASSERT(u[usize - 1] != 0);
	ASSERT(psize != 0);
	ASSERT(p[psize - 1] != 0);
	ASSERT(msize != 0);
	ASSERT(m[msize - 1] != 0);

	/* Precompute U mod M */
	mp_mod(u, usize, m, msize, w);
	if (mp_rsize(w, msize) == 0)
		return;

	/* Choose suitable value of K */
	unsigned b = mp_significant_bits(p, psize) - 1;
	unsigned k = MAX_K;
	for (; k >= 2; k--) {
		if (((k - 1) * (k << ((k - 1) << 1)) / ((1 << k) - k - 1)) < b)
			break;
	}
	const unsigned nk = 1U << k;

	mp_digit *up[MAX_NK] = { NULL };
	up[1] = MP_TMP_COPY(w, msize);

	mp_digit *tmp = MP_TMP_ALLOC(msize * 2);
	mp_sqr(up[1], msize, tmp);
#ifdef MODI
	mp_modi(tmp, msize * 2, m, msize);
	up[2] = MP_TMP_COPY(tmp, msize);
#else
	up[2] = MP_TMP_ALLOC(msize);
	mp_mod(tmp, msize * 2, m, msize, up[2]);
#endif

	/* Precompute U^3 mod M, U^5 mod M, ... U^(2^K-1) mod M */
	for (unsigned j = 3; j < nk; j += 2) {
		mp_mul_n(up[2], up[j-2], msize, tmp);
#ifdef MODI
		mp_modi(tmp, msize * 2, m, msize);
		up[j] = MP_TMP_COPY(tmp, msize);
#else
		up[j] = MP_TMP_ALLOC(msize);
		mp_mod(tmp, msize * 2, m, msize, up[j]);
#endif
	}

	b += 1;	/* still holds number of significant bits */
	if (b % k)
		b += k - (b % k);
	ASSERT(b % k == 0);

	unsigned a = 0;
	mp_digit mask = (mp_digit)1 << (b % MP_DIGIT_BITS);
	for (unsigned j = k; j != 0; j--) {
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
	unsigned t = pow2tab[a];
	a = odd_tab[a];
	mp_copy(up[a], msize, w);
	for (unsigned j = t; j != 0; j--) {
		mp_sqr(w, msize, tmp);
#ifdef MODI
		mp_modi(tmp, msize * 2, m, msize);
		mp_copy(tmp, msize, w);
#else
		mp_mod(tmp, msize * 2, m, msize, w);
#endif
	}

	while (b != 0) {
		a = 0;
		for (unsigned j = k; j != 0; j--) {
			ASSERT(b != 0);
			b -= 1;
			if ((mask >>= 1) == 0)
				mask = MP_DIGIT_MSB;
			a <<= 1;
			if (p[b / MP_DIGIT_BITS] & mask)
				a |= 1;
		}
		if (a == 0) {
			for (unsigned j = k; j != 0; j--) {
				mp_sqr(w, msize, tmp);
#ifdef MODI
				mp_modi(tmp, msize * 2, m, msize);
				mp_copy(tmp, msize, w);
#else
				mp_mod(tmp, msize * 2, m, msize, w);
#endif
			}
		} else {
			t = pow2tab[a];
			a = odd_tab[a];
			for (unsigned j = k - t; j != 0; j--) {
				mp_sqr(w, msize, tmp);
#ifdef MODI
				mp_modi(tmp, msize * 2, m, msize);
				mp_copy(tmp, msize, w);
#else
				mp_mod(tmp, msize * 2, m, msize, w);
#endif
			}
			mp_mul_n(w, up[a], msize, tmp);
#ifdef MODI
			mp_modi(tmp, msize * 2, m, msize);
			mp_copy(tmp, msize, w);
#else
			mp_mod(tmp, msize * 2, m, msize, w);
#endif
			for (unsigned j = t; j != 0; j--) {
				mp_sqr(w, msize, tmp);
#ifdef MODI
				mp_modi(tmp, msize * 2, m, msize);
				mp_copy(tmp, msize, w);
#else
				mp_mod(tmp, msize * 2, m, msize, w);
#endif
			}
		}
	}

	MP_TMP_FREE(up[1]);
	MP_TMP_FREE(up[2]);
	for (unsigned j = 3; j < nk; j += 2)
		MP_TMP_FREE(up[j]);
	MP_TMP_FREE(tmp);
}

/* Based on algorithm 1.2.4 from Cohen, "A Course in Computational Algebraic
 * Number Theory," and modified for modular reduction after each square or
 * multiply. */

/*  Input: u[0..usize-1], p[0..psize-1], m[0..msize-1]
 * Output: w[0..msize-1] = (u ** p) mod m */
void
mp_mexp(const mp_digit *u, mp_size usize,
		const mp_digit *p, mp_size psize,
		const mp_digit *m, mp_size msize, mp_digit *w)
{
	mp_zero(w, msize);
	MP_NORMALIZE(m, msize);
	ASSERT(msize != 0);

	if (msize == 1 && m[0] == 1)
		return;

	MP_NORMALIZE(u, usize);
	MP_NORMALIZE(p, psize);
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
			mp_digit *tmp = MP_TMP_ALLOC(usize * 2);
			mp_sqr(u, usize, tmp);
			mp_mod(tmp, usize * 2, m, msize, w);
			MP_TMP_FREE(tmp);
			return;
		}
	}

	mp_digit *tmp, *tmp2;
	mp_digit m0_inv;
	if (m[0] & 1) {
		const mp_size tmp_size = msize + MAX(usize, msize);
		tmp = MP_TMP_ALLOC(tmp_size);
		tmp2 = tmp + msize;

		mp_zero(tmp, msize);
		mp_copy(u, usize, tmp2);
		mp_mod(tmp, usize + msize, m, msize, w);

		m0_inv = -mp_digit_invert(m[0]);
	} else {
		tmp = MP_TMP_ALLOC(msize * 2);
		tmp2 = tmp;

		mp_mod(u, usize, m, msize, w);

		m0_inv = 0;
	}

	/* Choose optimal value of K */
	unsigned b = mp_significant_bits(p, psize);
	unsigned k = MAX_K;
	for (; k > 1; k--) {
		if (((k - 1) * (k << ((k - 1) << 1)) / ((1 << k) - k - 1)) < (b - 1))
			break;
	}
	unsigned nk = 1U << k;

	mp_digit *up[MAX_NK] = { NULL };
	up[1] = MP_TMP_COPY(w, msize);

#define REDUCE(t) \
	do { \
		if (m0_inv) \
			redc((t), m, m0_inv, msize); \
		else \
			mp_modi(t, msize * 2, m, msize); \
	} while (0)

	mp_sqr(up[1], msize, tmp);
	REDUCE(tmp);
	up[2] = MP_TMP_COPY(tmp2, msize);

	for (unsigned j = 3; j < nk; j += 2) {
		mp_mul_n(up[2], up[j-2], msize, tmp);
		REDUCE(tmp);
		up[j] = MP_TMP_COPY(tmp2, msize);
	}

	unsigned t = b % k;
	if (t != 0)
		b += k - t;
	ASSERT(b % k == 0);

	unsigned a = 0;
	mp_digit mask = (mp_digit)1 << (b % MP_DIGIT_BITS);
	for (unsigned j = k; j != 0; j--) {
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
	t = pow2tab[a];
	a = odd_tab[a];
	mp_copy(up[a], msize, w);
	for (unsigned j = t; j != 0; j--) {
		mp_sqr(w, msize, tmp);
		REDUCE(tmp);
		mp_copy(tmp2, msize, w);
	}

	while (b != 0) {
		a = 0;
		for (unsigned j = k; j != 0; j--) {
			ASSERT(b != 0);
			b -= 1;
			if ((mask >>= 1) == 0)
				mask = MP_DIGIT_MSB;
			a <<= 1;
			if (p[b / MP_DIGIT_BITS] & mask)
				a |= 1;
		}
		if (a == 0) {
			for (unsigned j = k; j != 0; j--) {
				mp_sqr(w, msize, tmp);
				REDUCE(tmp);
				mp_copy(tmp2, msize, w);
			}
		} else {
			t = pow2tab[a];
			a = odd_tab[a];
			for (unsigned j = k - t; j != 0; j--) {
				mp_sqr(w, msize, tmp);
				REDUCE(tmp);
				mp_copy(tmp2, msize, w);
			}
			mp_mul_n(w, up[a], msize, tmp);
			REDUCE(tmp);
			mp_copy(tmp2, msize, w);
			for (unsigned j = t; j != 0; j--) {
				mp_sqr(w, msize, tmp);
				REDUCE(tmp);
				mp_copy(tmp2, msize, w);
			}
		}
	}

	MP_TMP_FREE(up[1]);
	MP_TMP_FREE(up[2]);
	for (unsigned j = 3; j < nk; j += 2)
		MP_TMP_FREE(up[j]);

	if (m0_inv) {
		mp_copy(w, msize, tmp);
		mp_zero(tmp2, msize);
		redc(tmp, m, m0_inv, msize);
	}
	mp_copy(tmp2, msize, w);

	MP_TMP_FREE(tmp);
}
