/*
 * mp_util.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mp.h"
#include "mp_defs.h"
#include "weecrypt_memory.h"
#include <string.h>

mp_digit *
mp_new(mp_size size)
{
	ASSERT(size != 0);

	return MALLOC(size * MP_DIGIT_SIZE);
}

mp_digit *
mp_new0(mp_size size)
{
	ASSERT(size != 0);

	return mp_zero(mp_new(size), size);
}

mp_digit *
mp_resize(mp_digit *u, mp_size size)
{
	if (u != NULL)
		return REALLOC(u, size * MP_DIGIT_SIZE);
	else
		return mp_new(size);
}

mp_digit *
mp_dup(const mp_digit *u, mp_size size)
{
	ASSERT(size != 0);

	return mp_copy(u, size, mp_new(size));
}

void
mp_free(mp_digit *u)
{
	FREE(u);
}

void
mp_one(mp_digit *u, mp_size size)
{
	if (size == 0)
		return;

	*u = 1;
	while (--size)
		*++u = 0;
}

void
mp_max(mp_digit *u, mp_size size)
{
	mp_fill(u, size, MP_DIGIT_MASK);
}

#ifndef MP_FILL_ASM
mp_digit *
mp_fill(mp_digit *u, mp_size size, mp_digit d)
{
	mp_digit *up = u;

	while (size--)
		*up++ = d;
	return u;
}
#endif /* !MP_FILL_ASM */

#ifndef MP_FLIP_ASM
void
mp_flip(mp_digit *u, mp_size size)
{
	while (size--)
		*u++ ^= MP_DIGIT_MASK;
}
#endif

void
mp_complement(mp_digit *u, mp_size size)
{
	if (size) {
		mp_flip(u, size);
		mp_inc(u, size);
	}
}

#ifndef MP_XCHG_ASM
void
mp_xchg(mp_digit *u, mp_digit *v, mp_size size)
{
	mp_digit ud, vd;

	while (size--) {
		ud = *u; vd = *v;
		*u = vd; *v = ud;
		u++; v++;
	}
}
#endif

#ifndef MP_COPY_ASM
mp_digit *
mp_copy(const mp_digit *u, mp_size size, mp_digit *v)
{
#if 0
	mp_digit *vp = v;

	if (size == 0 || u == v)
		return v;

	do {
		*vp++ = *u++;
	} while (--size);
	return v;
#else
	return memcpy(v, u, size * sizeof(mp_digit));
#endif
}
#endif /* !MP_COPY_ASM */

#ifndef MP_CMP_N_ASM
int
mp_cmp_n(const mp_digit *u, const mp_digit *v, mp_size size)
{
	u += size;
	v += size;
	while (size--) {
		--u; --v;
		if (*u < *v) return -1;
		if (*u > *v) return +1;
	}
	return 0;
}
#endif

int
mp_cmp(const mp_digit *u, mp_size usize,
	   const mp_digit *v, mp_size vsize)
{
	usize = mp_rsize(u, usize);
	vsize = mp_rsize(v, vsize);
	if (usize < vsize) return -1;
	if (usize > vsize) return +1;
	return usize ? mp_cmp_n(u, v, usize) : 0;
}

#ifndef MP_RSIZE_ASM
mp_size
mp_rsize(const mp_digit *u, mp_size size)
{
	if (size == 0)
		return 0;

	u += size;
	while (size && *--u == 0)
		size--;
	return size;
}
#endif /* !MP_RSIZE_ASM */

int
mp_setbit(mp_digit *u, mp_size size, unsigned bit)
{
	unsigned digit;

	if ((digit = bit / MP_DIGIT_BITS) >= size)
		return -1;
	u[digit] |= MP_DIGIT_LSB << (bit % MP_DIGIT_BITS);
	return 0;
}

int
mp_clrbit(mp_digit *u, mp_size size, unsigned bit)
{
	unsigned digit;

	if ((digit = bit / MP_DIGIT_BITS) >= size)
		return -1;
	u[digit] &= ~(MP_DIGIT_LSB << (bit % MP_DIGIT_BITS));
	return 0;
}

int
mp_flpbit(mp_digit *u, mp_size size, unsigned bit)
{
	unsigned digit;

	if ((digit = bit / MP_DIGIT_BITS) >= size)
		return -1;
	u[digit] ^= MP_DIGIT_LSB << (bit % MP_DIGIT_BITS);
	return 0;
}

int
mp_tstbit(mp_digit *u, mp_size size, unsigned bit)
{
	unsigned digit;

	if ((digit = bit / MP_DIGIT_BITS) >= size)
		return -1;
	return (u[digit] & (MP_DIGIT_LSB << (bit % MP_DIGIT_BITS))) ? 1 : 0;
}

void
mp_and(mp_digit *u, mp_size size, const mp_digit *v)
{
	while (size--)
		*u++ &= *v++;
}

void
mp_andnot(mp_digit *u, mp_size size, const mp_digit *v)
{
	while (size--)
		*u++ &= ~*v++;
}

void
mp_or(mp_digit *u, mp_size size, const mp_digit *v)
{
	while (size--)
		*u++ |= *v++;
}

void
mp_ornot(mp_digit *u, mp_size size, const mp_digit *v)
{
	while (size--)
		*u++ |= ~*v++;
}

void
mp_xor(mp_digit *u, mp_size size, const mp_digit *v)
{
	while (size--)
		*u++ ^= *v++;
}

void
mp_xornot(mp_digit *u, mp_size size, const mp_digit *v)
{
	while (size--)
		*u++ ^= ~*v++;
}

unsigned
mp_msb_shift(mp_digit u)
{
#if MP_DIGIT_BITS == 16
	mp_digit high8 = 0xff00;
#elif MP_DIGIT_BITS == 32
	mp_digit high8 = 0xff000000U;
#elif MP_DIGIT_BITS == 64
	mp_digit high8 = CONST64(0xff00000000000000);
#endif
	unsigned steps = 0;

	if (u == 0)
		return -1;

#if MP_DIGIT_BITS != 8
	while ((u & high8) == 0)
		steps += 8, u <<= 8;
#endif
	while ((u & MP_DIGIT_MSB) == 0)
		steps += 1, u <<= 1;
	return steps;
}

unsigned
mp_msb_normalize(mp_digit *u, mp_size size)
{
	unsigned sh;

	ASSERT(size != 0);
	ASSERT(u[size - 1] != 0);

	sh = mp_msb_shift(u[size - 1]);
	if (sh) {
		mp_digit cy;

		cy = mp_lshifti(u, size, sh);
		ASSERT(cy == 0);
	}
	return sh;
}

unsigned
mp_digit_log2(mp_digit u)
{
	unsigned lg;

	ASSERT(u != 0);

	for (lg = 0; u != 1; lg++)
		u >>= 1;

	return lg;
}

unsigned
mp_odd_shift(const mp_digit *u, mp_size size)
{
	unsigned i, j;
	mp_digit ui;

	if ((size = mp_rsize(u, size)) == 0)
		return 0;

	for (i = 0; u[i] == 0; i++)
		/* void */;
	ui = u[i];
	for (j = 0; (ui & 1) == 0; j++)
		ui >>= 1;

	return (MP_DIGIT_BITS * i) + j;
}

unsigned
mp_significant_bits(const mp_digit *u, mp_size size)
{
	size = mp_rsize(u, size);
	if (size == 0)
		return 0;
	/* +1 because we want to know how many bits it takes to represent most
	 * significant digit */
	size -= 1;
	return (MP_DIGIT_BITS * size) + mp_digit_log2(u[size]) + 1;
}
