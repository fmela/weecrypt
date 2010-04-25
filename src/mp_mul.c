/*
 * mp_mul.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mp.h"
#include "mp_defs.h"

/* Multiply u[usize] by v[vsize] and store the result in w[usize + vsize],
 * using the simple quadratic-time algorithm. */
void
_mp_mul_base(const mp_digit *u, mp_size usize,
			 const mp_digit *v, mp_size vsize, mp_digit *w)
{
	mp_size ul, vl;
	mp_digit vd, *wp;

	ASSERT(usize >= vsize);

	/* Find real sizes and zero any part of answer which will not be set. */
	ul = mp_rsize(u, usize);
	vl = mp_rsize(v, vsize);
	/* One or both are zero. */
	if (ul == 0 || vl == 0) {
		mp_zero(w, usize + vsize);
		return;
	}
	/* Zero digits which won't be set in multiply-and-add loop. */
	if (ul + vl != usize + vsize)
		mp_zero(w + (ul + vl), usize + vsize - (ul + vl));

	/* Now multiply by forming partial products and adding them to the result
	 * so far. Rather than zero the low ul digits of w before starting, we
	 * store, rather than add, the first partial product. */
	wp = w + ul;
	*wp = mp_dmul(u, ul, *v, w);
	while (--vl) {
#if 0
		vd = *++v; ++w; ++wp;
		if (vd == 0)
			*wp = 0;
		else if (vd == 1)
			*wp = mp_addi_n(w, u, ul);
		else
			*wp = mp_dmul_add(u, ul, vd, w);
#else
		vd = *++v;
		*++wp = mp_dmul_add(u, ul, vd, ++w);
#endif
	}
}

/* Karatsuba multiplication [cf. Knuth 4.3.3, vol.2, 3rd ed, pp.294-295]
 * Given U = U1*2^N + U0 and V = V1*2^N + V0,
 * we can recursively compute U*V with
 * (2^2N + 2^N)U1*V1 + (2^N)(U1-U0)(V0-V1) + (2^N + 1)U0*V0
 *
 * We might otherwise use
 * (2^2N - 2^N)U1*V1 + (2^N)(U1+U0)(V1+V0) + (1 - 2^N)U0*V0
 * except that (U1+U0) or (V1+V0) may become N+1 bit numbers if there is carry
 * in the additions, and this will slow down the routine.  However, if we use
 * the first formula the middle terms will not grow larger than N bits. */
#ifdef TUNE_KARATSUBA
# undef KARATSUBA_MUL_THRESHOLD
mp_size KARATSUBA_MUL_THRESHOLD = 32;
#else
# ifndef KARATSUBA_MUL_THRESHOLD
#  define KARATSUBA_MUL_THRESHOLD 32
# endif /* !KARATSUBA_MUL_THRESHOLD */
#endif
void
mp_mul_n(const mp_digit *u, const mp_digit *v, mp_size size, mp_digit *w)
{
	/* TODO: Only allocate a temporary buffer which is large enough for all
	 * following recursive calls, rather than allocating at each call. */
	mp_size half_size, even_size;
	mp_digit cy, *tmp, *w0, *w1, *u_tmp, *v_tmp;
	const mp_digit *u0, *u1, *v0, *v1;
	int neg, odd;

	if (u == v) {
		mp_sqr(u, size, w);
		return;
	}

	if (size < KARATSUBA_MUL_THRESHOLD) {
		_mp_mul_base(u, size, v, size, w);
		return;
	}

	even_size = size - (odd = size & 1);
	half_size = even_size / 2;

	u1 = (u0 = u) + half_size;
	v1 = (v0 = v) + half_size;
	w1 = (w0 = w) + even_size;

	/* U0 * V0 => w[0..even_size-1]; */
	/* U1 * V1 => w[even_size..2*even_size-1]. */
	if (half_size >= KARATSUBA_MUL_THRESHOLD) {
		mp_mul_n(u0, v0, half_size, w0);
		mp_mul_n(u1, v1, half_size, w1);
	} else {
		_mp_mul_base(u0, half_size, v0, half_size, w0);
		_mp_mul_base(u1, half_size, v1, half_size, w1);
	}

	/* Since we cannot add w[0..even_size-1] to w[half_size ...
	 * half_size+even_size-1] in place, we have to make a copy of it now. This
	 * later gets used to store U1-U0 and V0-V1. */
	MP_TMP_COPY(tmp, w0, even_size);

	/* w[half_size..half_size+even_size-1] += U1*V1. */
	cy  = mp_addi_n(w + half_size,  w1, even_size);
	/* w[half_size..half_size+even_size-1] += U0*V0. */
	cy += mp_addi_n(w + half_size, tmp, even_size);

	/* Get absolute value of U1-U0. */
	u_tmp = tmp;
	if ((neg = (mp_cmp_n(u1, u0, half_size) < 0)) != 0)
		mp_sub_n(u0, u1, half_size, u_tmp);
	else
		mp_sub_n(u1, u0, half_size, u_tmp);

	/* Get absolute value of V0-V1. */
	v_tmp = tmp + half_size;
	if (mp_cmp_n(v0, v1, half_size) < 0)
		mp_sub_n(v1, v0, half_size, v_tmp), neg ^= 1;
	else
		mp_sub_n(v0, v1, half_size, v_tmp);

	/* tmp = (U1-U0)*(V0-V1). */
	MP_TMP_ALLOC(tmp, even_size);
	if (half_size >= KARATSUBA_MUL_THRESHOLD)
		mp_mul_n(u_tmp, v_tmp, half_size, tmp);
	else
		_mp_mul_base(u_tmp, half_size, v_tmp, half_size, tmp);
	MP_TMP_FREE(u_tmp);
	/* Now add / subtract (U1-U0)*(V0-V1) from
	 * w[half_size..half_size+even_size-1] based on whether it is negative or
	 * positive. */
	if (neg)
		cy -= mp_subi_n(w + half_size, tmp, even_size);
	else
		cy += mp_addi_n(w + half_size, tmp, even_size);
	MP_TMP_FREE(tmp);
	/* Now if there was any carry from the middle digits (which is at most 2),
	 * add that to w[even_size+half_size..2*even_size-1]. */
	if (cy)
		cy = mp_daddi(w + even_size + half_size, half_size, cy);
	ASSERT(cy == 0);

	if (odd) {
		/* We have the product U[0..even_size-1] * V[0..even_size-1] in
		 * W[0..2*even_size-1].  We need to add the following to it:
		 * V[size-1] * U[0..size-2]
		 * U[size-1] * V[0..size-1] */
		w[even_size*2+0] = mp_dmul_add(u, even_size, v[even_size], &w[even_size]);
		w[even_size*2+1] = mp_dmul_add(v, size, u[even_size], &w[even_size]);
	}
}

void
mp_mul(const mp_digit *u, mp_size usize,
	   const mp_digit *v, mp_size vsize, mp_digit *w)
{
	mp_digit cy, *tmp = NULL;
	mp_size wsize;

	if (usize != 0)
		ASSERT(u[usize - 1] != 0);
	if (vsize != 0)
		ASSERT(v[vsize - 1] != 0);

	{
		mp_size ul, vl;

		ul = mp_rsize(u, usize);
		vl = mp_rsize(v, vsize);
		if (ul == 0 || vl == 0) {
			mp_zero(w, usize + vsize);
			return;
		}
		/* Zero digits which won't be set. */
		if (ul + vl != usize + vsize)
			mp_zero(w + (ul + vl), (usize + vsize) - (ul + vl));

		/* Wanted: ULEN >= VLEN. */
		if (ul < vl) {
			const mp_digit *t;
			t = u, u = v, v = t;
			usize = vl; vsize = ul;
		} else {
			usize = ul; vsize = vl;
		}
		ASSERT(usize >= vsize);
	}

	if (vsize < KARATSUBA_MUL_THRESHOLD) {
		_mp_mul_base(u, usize, v, vsize, w);
		return;
	}

#if 0
	if (usize == vsize) {
		mp_mul_n(u, v, vsize, w);
		return;
	}
	/* Recurse: set w[vsize:vsize+usize-1] = u[vsize:usize-1] * v[0:vsize-1] */
	mp_mul(u + vsize, usize - vsize, v, vsize, w + vsize);
	/* Allocate space for temporary product */
	MP_TMP_ALLOC(tmp, vsize * 2);
	/* Set tmp = u[0:vsize-1] * v[0:vsize-1] */
	mp_mul_n(u, v, vsize, tmp);
	/* Set W[0:vsize-1] = tmp[0:vsize-1] */
	mp_copy(tmp, vsize, w);
	/* Add tmp[vsize:vsize*2-1] to w[vsize:usize+vsize-1] */
	cy = mp_addi(w + vsize, usize, tmp + vsize, vsize);
	ASSERT(cy == 0);
	/* Free storage for tmp. */
	MP_TMP_FREE(tmp);
#else	/* This method is faster and uses less stack and temporary space. */
	mp_mul_n(u, v, vsize, w);
	if (usize == vsize)
		return;
	wsize = usize + vsize;
	mp_zero(w + (vsize * 2), wsize - (vsize * 2));
	w += vsize; wsize -= vsize;
	u += vsize; usize -= vsize;
	if (usize >= vsize) {
		MP_TMP_ALLOC(tmp, vsize * 2);
		do {
			mp_mul_n(u, v, vsize, tmp);
			cy = mp_addi(w, wsize, tmp, vsize * 2);
			ASSERT(cy == 0);
			w += vsize; wsize -= vsize;
			u += vsize; usize -= vsize;
		} while (usize >= vsize);
	}
	if (usize) {	/* Size of U isn't a multiple of size of V. */
		if (tmp == NULL)
			MP_TMP_ALLOC(tmp, usize + vsize);
		/* Now usize < vsize. Rearrange operands. */
		if (usize < KARATSUBA_MUL_THRESHOLD)
			_mp_mul_base(v, vsize, u, usize, tmp);
		else
			mp_mul(v, vsize, u, usize, tmp);
		cy = mp_addi(w, wsize, tmp, usize + vsize);
		ASSERT(cy == 0);
	}
	MP_TMP_FREE(tmp);
#endif
}

/* w[wsize] = u[usize] * v[vsize] mod ((2^MP_DIGIT_BITS)*wsize)
 * low WLEN digits of product, good for barrett modular reduction */
void
mp_mul_mod_powb(const mp_digit *u, mp_size usize,
				const mp_digit *v, mp_size vsize,
				mp_digit *w, mp_size wsize)
{
	mp_size j, msize, lim;

	ASSERT(wsize != 0);

	usize = mp_rsize(u, usize);
	vsize = mp_rsize(v, vsize);
	if (usize == 0 || vsize == 0) {
		mp_zero(w, wsize);
		return;
	}

	if (wsize >= usize + vsize) {
		mp_mul(u, usize, v, vsize, w);
		mp_zero(w + usize + vsize, wsize - (usize + vsize));
		return;
	}

	mp_zero(w, wsize);

	if (usize < vsize) {
		SWAP(u, v, const mp_digit *);
		SWAP(usize, vsize, mp_size);
	}

	if (wsize > usize)
		mp_mul(u, usize, v, j = wsize - usize, w);
	else
		j = 0;
	lim = MIN(vsize, wsize);
	for (; j < lim; j++) {
		msize = MIN(usize, wsize - j);
		mp_dmul_add(u, msize, v[j], &w[j]);
	}
}
