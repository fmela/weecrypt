/* mp_sqr.c
 * Copyright (C) 2002-2012 Farooq Mela. All rights reserved. */

#include "mp.h"
#include "mp_defs.h"

extern void _mp_mul_base(const mp_digit *u, mp_size usize,
			 const mp_digit *v, mp_size vsize, mp_digit *w);

#ifndef MP_SQR_DIAG_ASM
void
mp_sqr_diag(const mp_digit *u, mp_size size, mp_digit *v)
{
    if (!size)
	return;
    /* No compiler seems to recognize that if ((A+B) mod 2^N) < A (or B) iff
     * (A+B) >= 2^N and it can use the carry flag after the adds rather than
     * doing comparisons to see if overflow has ocurred. Instead they generate
     * code to perform comparisons, retaining values in already scarce
     * registers after they should be "dead." At any rate this isn't the
     * time-critical part of squaring so it's nothing to lose sleep over. */
    mp_digit p0, p1;
    digit_sqr(*u, p1, p0);
    p1 += (v[0] += p0) < p0;
    mp_digit cy = (v[1] += p1) < p1;
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
    if (!usize)
	return;

    /* Find size, and zero any digits which will not be set. */
    mp_size ul = mp_rsize(u, usize);
    if (ul != usize) {
	mp_zero(v + (ul * 2), (usize - ul) * 2);
	if (ul == 0)
	    return;
	usize = ul;
    }

    /* Single-precision case. */
    if (usize == 1) {
	/* FIXME can't do this: digit_sqr(u[0], v[1], v[0]); */
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
    const mp_digit *ui = u;
    mp_digit *vp = &v[1];
    ul = usize - 1;
    vp[ul] = mp_dmul(&ui[1], ul, ui[0], vp);
    for (vp += 2; ++ui, --ul; vp += 2) {
	vp[ul] = mp_dmul_add(&ui[1], ul, ui[0], vp);
    }

    /* Double cross-products. */
    ul = usize * 2 - 1;
    v[ul] = mp_lshifti(v + 1, ul - 1, 1);

    /* Add "main diagonal:"
     * for i=0 .. n-1
     *     v += u[i]^2 * B^2i */
    mp_sqr_diag(u, usize, v);
}

/* Karatsuba squaring recursively applies the formula:
 *		U = U1*2^N + U0
 *		U^2 = (2^2N + 2^N)U1^2 - (U1-U0)^2 + (2^N + 1)U0^2
 * From my own testing this uses ~20% less time compared with slighly easier to
 * code formula:
 *		U^2 = (2^2N)U1^2 + (2^(N+1))(U1*U0) + U0^2
 */
#ifdef TUNE_KARATSUBA
# undef KARATSUBA_SQR_THRESHOLD
mp_size KARATSUBA_SQR_THRESHOLD = 64;
#else
# ifndef KARATSUBA_SQR_THRESHOLD
#  define KARATSUBA_SQR_THRESHOLD 32
# endif /* !KARATSUBA_SQR_THRESHOLD */
#endif
void
mp_sqr(const mp_digit *u, mp_size size, mp_digit *v)
{
    mp_size rsize = mp_rsize(u, size);
    if (rsize != size) {
	mp_zero(v + rsize * 2, (size - rsize) * 2);
	size = rsize;
    }

    if (size < KARATSUBA_SQR_THRESHOLD) {
	if (!size)
	    return;
	if (size <= BASE_SQR_THRESHOLD)
	    _mp_mul_base(u, size, u, size, v);
	else
	    mp_sqr_base(u, size, v);
	return;
    }

    const bool odd_size = size & 1;
    const mp_size even_size = size & ~1;
    const mp_size half_size = even_size / 2;
    const mp_digit *u0 = u, *u1 = u + half_size;
    mp_digit *v0 = v, *v1 = v + even_size;

    /* Choose the appropriate squaring function. */
    void (*sqr_fn)(const mp_digit *, mp_size, mp_digit *) =
	(half_size >= KARATSUBA_SQR_THRESHOLD) ? mp_sqr : mp_sqr_base;
    /* Compute the low and high squares, potentially recursively. */
    sqr_fn(u0, half_size, v0);	/* U0^2 => V0 */
    sqr_fn(u1, half_size, v1);	/* U1^2 => V1 */

    mp_digit *tmp = MP_TMP_ALLOC(even_size * 2);
    mp_digit *tmp2 = tmp + even_size;
    /* tmp = w[0..even_size-1] */
    mp_copy(v0, even_size, tmp);
    /* v += U1^2 * 2^N */
    mp_digit cy  = mp_addi_n(v + half_size,  v1, even_size);
    /* v += U0^2 * 2^N */
    cy +=          mp_addi_n(v + half_size, tmp, even_size);

    int cmp = mp_cmp_n(u1, u0, half_size);
    if (cmp) {
	if (cmp < 0)
	    mp_sub_n(u0, u1, half_size, tmp);
	else
	    mp_sub_n(u1, u0, half_size, tmp);
	sqr_fn(tmp, half_size, tmp2);
	cy -= mp_subi_n(v + half_size, tmp2, even_size);
    }
    MP_TMP_FREE(tmp);
    if (cy) {
	ASSERT(mp_daddi(v + even_size + half_size, half_size, cy) == 0);
    }

    if (odd_size) {
	v[even_size*2] = mp_dmul_add(u, even_size, u[even_size], &v[even_size]);
	v[even_size*2+1] = mp_dmul_add(u, size, u[even_size], &v[even_size]);
    }
}
