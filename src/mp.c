/*
 * mp.c
 * Copyright (C) 2001-2010 Farooq Mela. All rights reserved.
 *
 * $Id: mp.c,v 1.13 2001/12/30 09:23:23 farooq Exp farooq $
 */

#include <stdlib.h>
#include <string.h>

#include "mp.h"
#include "mp_defs.h"
#include "weecrypt_memory.h"

#ifndef MP_INC_ASM
mp_digit
mp_inc(mp_digit *u, mp_size size)
{
	if (size == 0)
		return 1;

	for (; size--; ++u) {
		if (++*u)
			return 0;
	}
	return 1;
}
#endif /* !MP_INC_ASM */

#ifndef MP_DEC_ASM
mp_digit
mp_dec(mp_digit *u, mp_size size)
{
	if (size == 0)
		return 1;

	for (;;) {
		if (u[0]--)
			return 0;
		if (--size == 0)
			return 1;
		u++;
	}
	return 1;
}
#endif /* !MP_DEC_ASM */

mp_digit
mp_dadd(const mp_digit *u, mp_size size, mp_digit v, mp_digit *w)
{
	mp_copy(u, size, w);
	return mp_daddi(w, size, v);
}

mp_digit
mp_daddi(mp_digit *u, mp_size size, mp_digit v)
{
	if (v == 0 || size == 0)
		return v;
	return ((u[0] += v) < v) ? mp_inc(&u[1], size - 1) : 0;
}

#ifndef MP_ADD_N_ASM
/* Set w[size] = u[size] + v[size] and return the carry. */
mp_digit
mp_add_n(const mp_digit *u, const mp_digit *v, mp_size size, mp_digit *w)
{
	ASSERT(u);
	ASSERT(v);
	ASSERT(w);

	mp_digit cy = 0;
	while (size--) {
		mp_digit ud = *u++;
		const mp_digit vd = *v++;
		cy  = (ud += cy) < cy;
		cy += (*w = ud + vd) < vd;
		++w;
	}
	return cy;
}
#endif /* !MP_ADD_N_ASM */

mp_digit
mp_add(const mp_digit *u, mp_size usize,
	   const mp_digit *v, mp_size vsize, mp_digit *w)
{
	ASSERT(u);
	ASSERT(usize > 0);
	ASSERT(u[usize - 1]);
	ASSERT(v);
	ASSERT(vsize > 0);
	ASSERT(v[vsize - 1]);

	if (usize < vsize) {
		mp_digit cy = mp_add_n(u, v, usize, w);
		if (v != w) {
			mp_copy(v + usize, vsize - usize, w + usize);
		}
		return cy ? mp_inc(w + usize, vsize - usize) : 0;
	} else if (usize > vsize) {
		mp_digit cy = mp_add_n(u, v, vsize, w);
		if (u != w) {
			mp_copy(u + vsize, usize - vsize, w + vsize);
		}
		return cy ? mp_inc(w + vsize, usize - vsize) : 0;
	} else { /* usize == vsize */
		return mp_add_n(u, v, usize, w);
	}
}

/* Set u[usize] = u[usize] + v[vsize] and return the carry. */
mp_digit
mp_addi(mp_digit *u, mp_size usize, const mp_digit *v, mp_size vsize)
{
	ASSERT(u != NULL);
	ASSERT(v != NULL);
	ASSERT(usize >= vsize);

	mp_digit cy = mp_addi_n(u, v, vsize);
	return cy ? mp_inc(u + vsize, usize - vsize) : 0;
}

mp_digit
mp_dsub(const mp_digit *u, mp_size size, mp_digit v, mp_digit *w)
{
	mp_copy(u, size, w);
	return mp_dsubi(w, size, v);
}

mp_digit
mp_dsubi(mp_digit *u, mp_size size, mp_digit v)
{
	if (v == 0 || size == 0)
		return v;
	const mp_digit u0 = u[0];
	return ((u[0] -= v) > u0) ? mp_dec(u + 1, size - 1) : 0;
}

/* Set w[size] = u[size] - v[size] and return the borrow. */
#ifndef MP_SUB_N_ASM
mp_digit
mp_sub_n(const mp_digit *u, const mp_digit *v, mp_size size, mp_digit *w)
{
	ASSERT(u != NULL);
	ASSERT(v != NULL);
	ASSERT(w != NULL);

	mp_digit cy = 0;
	while (size--) {
		const mp_digit ud = *u++;
		mp_digit vd = *v++;
		cy  = (vd += cy) < cy;
		cy += (*w = ud - vd) > ud;
		++w;
	}
	return cy;
}
#endif /* !MP_SUB_N_ASM */

mp_digit
mp_sub(const mp_digit *u, mp_size usize,
	   const mp_digit *v, mp_size vsize, mp_digit *w)
{
	mp_digit cy;

	ASSERT(usize >= vsize);

	if (usize == vsize)
		return mp_sub_n(u, v, usize, w);

	cy = mp_sub_n(u, v, vsize, w);
	usize -= vsize;
	w += vsize;
	mp_copy(u + vsize, usize, w);
	return cy ? mp_dec(w, usize) : 0;
}

#ifndef MP_SUBI_N_ASM
/* Set u[size] = u[size] - v[size] and return the borrow. */
mp_digit
mp_subi_n(mp_digit *u, const mp_digit *v, mp_size size)
{
	ASSERT(u != NULL);
	ASSERT(v != NULL);

	mp_digit cy = 0;
	while (size--) {
		mp_digit vd = *v++;
		const mp_digit ud = *u;
		cy  = (vd += cy) < cy;
		cy += (*u -= vd) > ud;
		++u;
	}
	return cy;
}
#endif /* !MP_SUBI_N_ASM */

mp_digit
mp_subi(mp_digit *u, mp_size usize, const mp_digit *v, mp_size vsize)
{
	mp_digit cy;

	ASSERT(u != NULL);
	ASSERT(v != NULL);
	ASSERT(usize >= vsize);

	cy = mp_subi_n(u, v, vsize);
	return cy ? mp_dec(u + vsize, usize - vsize) : 0;
}

int
mp_diff_n(const mp_digit *u, const mp_digit *v, mp_size size, mp_digit *w)
{
	int cmp;

	if (size == 0)
		return 0;

	cmp = mp_cmp_n(u, v, size);
	if (cmp > 0)
		mp_sub_n(u, v, size, w);
	else if (cmp < 0)
		mp_sub_n(v, u, size, w);
	else
		mp_zero(w, size);
	return cmp;
}

#ifndef MP_DMUL_ASM
mp_digit
mp_dmul(const mp_digit *u, mp_size size, mp_digit v, mp_digit *w)
{
	if (v <= 1) {
		if (v == 0)
			mp_zero(w, size);
		else
			mp_copy(u, size, w);
		return 0;
	}

	mp_digit cy = 0;
	while (size--) {
		mp_digit p1, p0;
		digit_mul(*u, v, p1, p0);
		cy = ((p0 += cy) < cy) + p1;
		*w++ = p0;
		++u;
	}
	return cy;
}
#endif /* !MP_DMUL_ASM */

#ifndef MP_DMULI_ASM
mp_digit
mp_dmuli(mp_digit *u, mp_size size, mp_digit v)
{
	if (v <= 1) {
		if (v == 0)
			mp_zero(u, size);
		return 0;
	}

	mp_digit cy = 0;
	while (size--) {
		mp_digit p1, p0;
		digit_mul(*u, v, p1, p0);
		cy = ((p0 += cy) < cy) + p1;
		*u++ = p0;
	}
	return cy;
}
#endif /* !MP_DMULI_ASM */

#ifndef MP_DMUL_ADD_ASM
mp_digit
mp_dmul_add(const mp_digit *u, mp_size size, mp_digit v, mp_digit *w)
{
	ASSERT(u);
	ASSERT(w);

	if (v <= 1)
		return v ? mp_addi_n(w, u, size) : 0;

	mp_digit cy = 0;
	while (size--) {
		mp_digit p1, p0;
		digit_mul(*u, v, p1, p0);
		cy  = ((p0 += cy) < cy) + p1;
		cy += ((*w += p0) < p0);
		++u; ++w;
	}
	return cy;
}
#endif /* !MP_DMUL_ADD_ASM */

#ifndef MP_DMUL_SUB_ASM
mp_digit
mp_dmul_sub(const mp_digit *u, mp_size size, mp_digit v, mp_digit *w)
{
	mp_digit cy, p0, p1;

	ASSERT(u != NULL);
	ASSERT(w != NULL);

	if (v <= 1)
		return v ? mp_subi_n(w, u, size) : 0;

	for (cy = 0; size--; ++u, ++w) {
		digit_mul(*u, v, p1, p0);
		cy  = ((p0 += cy) < cy) + p1;
		cy += (*w < p0);
		*w -= p0;
	}
	return cy;
}
#endif

/* Set v[size*exp] = u[size]^exp. */
void
mp_exp(const mp_digit *u, mp_size size, unsigned long exp, mp_digit *v)
{
	/* Knuth's algorithm 4.6.3-A.
	 * Perform binary powering, making use of the basic identity
	 *      2i     2 i
	 *     u   = (u )
	 * to exponentiate a number faster than the naive method. While the
	 * exponent is greater than one, we square the intermediate result (which
	 * began at base). If the exponent is odd, we multiply by the base. Then
	 * we halve the exponent and repeat the process. */
	mp_zero(v, size * exp);
	/* 0^0 is not clearly defined, but we return 0 here. */
	if (size == 0)
		return;
	if (exp == 0) {
		v[0] = 1;
		return;
	}
	if (exp == 1) {
		mp_copy(u, size, v);
		return;
	}

	/* A1. */
	mp_digit *y;
	MP_TMP_ALLOC(y, size * exp);
	y[0] = 1;
	mp_size ysize = 1;

	mp_digit *z;
	MP_TMP_ALLOC0(z, size * exp);
	mp_copy(u, size, z);
	mp_size zsize = size;

	for (;;) {
		/* A2. */
		const bool odd = exp & 1;
		exp >>= 1;
		if (odd) {
			/* A3. */
			mp_mul(y, ysize, z, zsize, v);
			ysize = mp_rsize(v, ysize + zsize);
			mp_copy(v, ysize, y);
			/* A4. */
			if (exp == 0) {
				mp_copy(y, ysize, v);
				MP_TMP_FREE(y);
				MP_TMP_FREE(z);
				return;
			}
		}
		/* A5. */
		mp_sqr(z, zsize, v);
		zsize = mp_rsize(v, zsize * 2);
		mp_copy(v, zsize, z);
	}
}

/* Multiply u[size] by 2^shift and store in v[size], returning carry.
 * shift will be taken modulo MP_DIGIT_BITS. */
mp_digit
mp_lshift(const mp_digit *u, mp_size size, unsigned shift, mp_digit *v)
{
	if (!size)
		return 0;
	shift &= MP_DIGIT_BITS - 1;
	if (!shift) {
		if (u != v)
			mp_copy(u, size, v);
		return 0;
	}
	const unsigned subp = MP_DIGIT_BITS - shift;
	mp_digit q = 0;
	do {
		const mp_digit p = *u++;
		*v++ = (p << shift) | q;
		q = p >> subp;
	} while (--size);
	return q;
}

/* Divide u[size] by 2^shift. */
mp_digit
mp_rshift(const mp_digit *u, mp_size size, unsigned shift, mp_digit *v)
{
	if (size == 0)
		return 0;
	shift &= MP_DIGIT_BITS - 1;
	if (!shift) {
		if (u != v)
			mp_copy(u, size, v);
		return 0;
	}

	const unsigned subp = MP_DIGIT_BITS - shift;
	u += size;
	v += size;
	mp_digit q = 0;
	do {
		const mp_digit p = *--u;
		*--v = (p >> shift) | q;
		q = p << subp;
	} while (--size);
	return q >> subp;
}

/* Multiply u[size] by 2^shift, shift taken modulo MP_DIGIT_BITS. */
#ifndef MP_LSHIFTI_ASM
mp_digit
mp_lshifti(mp_digit *u, mp_size size, unsigned shift)
{
	shift &= MP_DIGIT_BITS - 1;
	if (!size || !shift)
		return 0;

	const unsigned subp = MP_DIGIT_BITS - shift;
	mp_digit q = 0;
	do {
		const mp_digit p = *u;
		*u++ = (p << shift) | q;
		q = p >> subp;
	} while (--size);
	return q;
}
#endif /* !MP_LSHIFTI_ASM */

/* Divide u[size] by 2^shift, shift taken modulo MP_DIGIT_BITS. */
#ifndef MP_RSHIFTI_ASM
mp_digit
mp_rshifti(mp_digit *u, mp_size size, unsigned shift)
{
	shift &= MP_DIGIT_BITS - 1;
	if (!size || !shift)
		return 0;

	unsigned subp = MP_DIGIT_BITS - shift;
	u += size;
	mp_digit q = 0;
	do {
		const mp_digit p = *--u;
		*u = (p >> shift) | q;
		q = p << subp;
	} while (--size);
	return q >> subp;
}
#endif /* !MP_RSHIFTI_ASM */

#ifndef MP_DDIV_ASM
mp_digit
mp_ddiv(const mp_digit *u, mp_size size, mp_digit v, mp_digit *w)
{
	ASSERT(u != NULL);
	ASSERT(w != NULL);
	ASSERT(v != 0);

	mp_copy(u, size, w);
	if (v == 1)
		return 0;
	return mp_ddivi(w, size, v);
}
#endif /* !MP_DDIV_ASM */

#ifndef MP_DDIVI_ASM
mp_digit
mp_ddivi(mp_digit *u, mp_size size, mp_digit v)
{
	ASSERT(u != NULL);
	ASSERT(v != 0);

	if (v == 1)
		return 0;

	MP_NORMALIZE(u, size);
	if (!size)
		return 0;

	if ((v & (v - 1)) == 0) {
		return mp_rshifti(u, size, mp_digit_lsb_shift(v));
	} else {
		mp_digit s1 = 0;
		u += size;
		do {
			mp_digit s0 = *--u;
			mp_digit q, r;
			if (s1 == 0) {
				q = s0 / v;
				r = s0 % v;
			} else {
				digit_div(s1, s0, v, q, r);
			}
			*u = q;
			s1 = r;
		} while (--size);
		return s1;
	}
}
#endif /* !MP_DDIVI_ASM */

#ifndef MP_DMOD_ASM
mp_digit
mp_dmod(const mp_digit *u, mp_size size, mp_digit v)
{
	ASSERT(u != NULL);
	ASSERT(v != 0);

	MP_NORMALIZE(u, size);
	if (size == 0)
		return 0;

	if ((v & (v - 1)) == 0) {
		return u[0] & (v - 1);
	} else {
		mp_digit s1 = 0;
		u += size;
		do {
			mp_digit s0 = *--u;
			mp_digit r;
			if (s1 == 0) {
				r = s0 % v;
			} else {
				mp_digit q;
				digit_div(s1, s0, v, q, r);
			}
			s1 = r;
		} while (--size);

		return s1;
	}
}
#endif /* !MP_DMOD_ASM */

/* mp_byte_pop[X] = population count of an 8-bit quantity X. */
static const unsigned char mp_byte_pop[256] = {
	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

/* Hamming weight = # of non-zero bits. */
unsigned
mp_hamming_weight(const mp_digit *u, mp_size size)
{
	unsigned sum = 0;

#if MP_DIGIT_SIZE == 1
	MP_NORMALIZE(u, size);
	if (size == 0)
		return 0;
	do {
		sum += mp_byte_pop[*u++];
	} while (--size);
#else
	MP_NORMALIZE(u, size);
	if (size == 0)
		return 0;
	do {
		for (mp_digit q = *u++; q != 0; q >>= 8)
			sum += mp_byte_pop[q & 0xff];
	} while (--size);
#endif
	return sum;
}

/* Hamming distance = # of differing bits. */
unsigned
mp_hamming_dist(const mp_digit *u, mp_size size, const mp_digit *v)
{
	unsigned hdist = 0;

#if MP_DIGIT_SIZE == 1
	while (size--)
		hdist += mp_byte_pop[u[size] ^ v[size]];
#else
	while (size--)
		for (mp_digit k = u[size] ^ v[size]; k != 0; k >>= 8)
			hdist += mp_byte_pop[k & 0xff];
#endif
	return hdist;
}
