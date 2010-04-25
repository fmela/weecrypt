/*
 * mp_sqr.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mp.h"
#include "mp_defs.h"

extern void _mp_mul_base(const mp_digit *u, mp_size usize,
						 const mp_digit *v, mp_size vsize, mp_digit *w);

#ifndef MP_SQR_DIAG_ASM
void
mp_sqr_diag(const mp_digit *u, mp_size size, mp_digit *v)
{
	mp_digit cy, p0, p1;

	ASSERT(size != 0);

	/* No compiler seems to recognize that if ((A+B) mod 2^N) < A (or B) iff
	 * (A+B) >= 2^N and it can use the carry flag after the adds rather than
	 * doing comparisons to see if overflow has ocurred. Instead they generate
	 * code to perform comparisons, retaining values in already scarce
	 * registers after they should be "dead." At any rate this isn't the
	 * time-critical part of squaring so it's nothing to lose sleep over. */

	digit_sqr(*u, p1, p0);
	p1 += (v[0] += p0) < p0;
	cy = (v[1] += p1) < p1;
	while (--size) {
		u += 1; v += 2;
		digit_sqr(*u, p1, p0);
		p1 += (p0 += cy) < cy;
		p1 += (v[0] += p0) < p0;
		cy = (v[1] += p1) < p1;
	}
	ASSERT(cy == 0);
}
#endif

#ifndef BASE_SQR_THRESHOLD
# define BASE_SQR_THRESHOLD	10
#endif /* !BASE_SQR_THRESHOLD */
static void
mp_sqr_base(const mp_digit *u, mp_size usize, mp_digit *v)
{
	mp_digit ud, *vp;
	const mp_digit *ui;
	mp_size ul;

	ASSERT(usize != 0);

	/* Find size, and zero any digits which will not be set. */
	if ((ul = mp_rsize(u, usize)) != usize) {
		mp_zero(v + (ul * 2), (usize - ul) * 2);
		if (ul == 0)
			return;
		usize = ul;
	}

	/* Simple, albeit rare, case. */
	if (usize == 1) {
	//	digit_sqr(u[0], v[1], v[0]);
		mp_digit v0,v1;
		digit_sqr(*u, v1, v0);
		v[1] = v1; v[0] = v0;
		return;
	}

	/* It is better to use the multiply routine if the number is small. */
	if (usize <= BASE_SQR_THRESHOLD) {
		_mp_mul_base(u, usize, u, usize, v);
		return;
	}

	/* Calculate products u[i] * u[j] for i != j.
	 * Most of the savings vs long multiplication come here, since we only
	 * perform (N-1) + (N-2) + ... + 1 = (N^2-N)/2 multiplications, vs a full
	 * N^2 in long multiplication. */
	v[0] = 0;
	vp = &v[1];
	ul = usize - 1;
	ud = *(ui = u);
	vp[ul] = mp_dmul(++ui, ul, ud, vp);
	for (vp += 2; --ul; vp += 2) {
		ud = *ui;
		vp[ul] = mp_dmul_add(++ui, ul, ud, vp);
	}

	/* Double cross-products. */
	ul = usize * 2 - 1;
	v[ul] = mp_lshifti(v + 1, ul - 1, 1);

	/* Add "main diagonal:"
	 * for i=0 .. n-1
	 *     v += u[i]^2 * B^2i */
	mp_sqr_diag(u, usize, v);
}

/* Karatsuba squaring; recursively apply formula
 * U = U1*2^N + U0
 * U^2 = (2^2N + 2^N)U1^2 - (U1-U0)^2 + (2^N + 1)U0^2
 * From my own testing this uses ~20% less time compared with the formula
 * U^2 = (2^2N)U1^2 + (2^(N+1))(U1*U0) + U0^2
 * which is slightly easier to code. */
#ifdef TUNE_KARATSUBA
# undef KARATSUBA_SQR_THRESHOLD
mp_size KARATSUBA_SQR_THRESHOLD = 64;
#else
# ifndef KARATSUBA_SQR_THRESHOLD
#  define KARATSUBA_SQR_THRESHOLD 64
# endif /* !KARATSUBA_SQR_THRESHOLD */
#endif
void
mp_sqr(const mp_digit *u, mp_size size, mp_digit *v)
{
	mp_size half_size, even_size, rsize;
	const mp_digit *u0, *u1;
	mp_digit cy, *v0, *v1;
	mp_digit *tmp, *tmp2;
	int odd, rv;
	void (*sqr_fn)(const mp_digit *, mp_size, mp_digit *);

	if ((rsize = mp_rsize(u, size)) != size) {
		mp_zero(v + rsize*2, size*2-rsize*2);
		size = rsize;
	}

	if (size < KARATSUBA_SQR_THRESHOLD) {
		if (size == 0)
			return;
		if (size <= BASE_SQR_THRESHOLD)
			_mp_mul_base(u, size, u, size, v);
		else
			mp_sqr_base(u, size, v);
		return;
	}

	even_size = size - (odd = size & 1);
	half_size = even_size / 2;
	u1 = (u0 = u) + half_size;
	v1 = (v0 = v) + even_size;

	/* U0^2 => V0 */
	/* U1^2 => V1 */
	sqr_fn = (half_size >= KARATSUBA_SQR_THRESHOLD) ? mp_sqr : mp_sqr_base;
	sqr_fn(u0, half_size, v0);
	sqr_fn(u1, half_size, v1);

	MP_TMP_ALLOC(tmp, even_size * 2);
	tmp2 = tmp + even_size;
	/* tmp = w[0..even_size-1] */
	mp_copy(v0, even_size, tmp);
	/* v += U1^2 * 2^N */
	cy  = mp_addi_n(v + half_size,  v1, even_size);
	/* v += U0^2 * 2^N */
	cy += mp_addi_n(v + half_size, tmp, even_size);

	rv = mp_cmp_n(u1, u0, half_size);
	if (rv) {
		if (rv < 0)
			mp_sub_n(u0, u1, half_size, tmp);
		else
			mp_sub_n(u1, u0, half_size, tmp);
		sqr_fn(tmp, half_size, tmp2);
		cy -= mp_subi_n(v + half_size, tmp2, even_size);
	}
	MP_TMP_FREE(tmp);
	if (cy)
		cy = mp_daddi(v + even_size + half_size, half_size, cy);
	ASSERT(cy == 0);

	if (odd) {
		v[even_size*2+0] = mp_dmul_add(u, even_size, u[even_size], &v[even_size]);
		v[even_size*2+1] = mp_dmul_add(u, size, u[even_size], &v[even_size]);
	}
}
