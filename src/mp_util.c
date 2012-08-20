/* mp_util.c
 * Copyright (C) 2002-2012 Farooq Mela. All rights reserved. */

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
	if (!size)
		return;

	*u = 1;
	while (--size)
		*++u = 0;
}

void
mp_max(mp_digit *u, mp_size size)
{
	mp_fill(u, size, MP_DIGIT_MAX);
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
		*u++ ^= MP_DIGIT_MAX;
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
	MP_NORMALIZE(u, usize);
	MP_NORMALIZE(v, vsize);
	if (usize < vsize) return -1;
	if (usize > vsize) return +1;
	return usize ? mp_cmp_n(u, v, usize) : 0;
}

#ifndef MP_RSIZE_ASM
mp_size
mp_rsize(const mp_digit *u, mp_size size)
{
	u += size;
	while (size && !*--u)
		--size;
	return size;
}
#endif /* !MP_RSIZE_ASM */

bool
mp_setbit(mp_digit *u, mp_size size, unsigned bit)
{
	unsigned digit = bit / MP_DIGIT_BITS;
	if (digit >= size)
		return false;
	u[digit] |= MP_DIGIT_LSB << (bit % MP_DIGIT_BITS);
	return true;
}

bool
mp_clearbit(mp_digit *u, mp_size size, unsigned bit)
{
	unsigned digit = bit / MP_DIGIT_BITS;
	if (digit >= size)
		return false;
	u[digit] &= ~(MP_DIGIT_LSB << (bit % MP_DIGIT_BITS));
	return true;
}

bool
mp_flipbit(mp_digit *u, mp_size size, unsigned bit)
{
	unsigned digit = bit / MP_DIGIT_BITS;
	if (digit >= size)
		return false;
	u[digit] ^= MP_DIGIT_LSB << (bit % MP_DIGIT_BITS);
	return true;
}

bool
mp_testbit(mp_digit *u, mp_size size, unsigned bit)
{
	unsigned digit = bit / MP_DIGIT_BITS;
	if (digit >= size)
		return false;
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

#ifndef mp_digit_msb_shift
unsigned
mp_digit_msb_shift(mp_digit u)
{
	ASSERT(u);

	unsigned steps = 0;
#if MP_DIGIT_BITS != 8
#if MP_DIGIT_BITS == 16
	const mp_digit high8 = UINT16_C(0xff00);
#elif MP_DIGIT_BITS == 32
	const mp_digit high8 = UINT32_C(0xff000000);
#elif MP_DIGIT_BITS == 64
	const mp_digit high8 = UINT64_C(0xff00000000000000);
#endif
	while (!(u & high8))
		steps += 8, u <<= 8;
#endif
	while (!(u & MP_DIGIT_MSB))
		steps += 1, u <<= 1;
	return steps;
}
#endif

#ifndef mp_digit_lsb_shift
unsigned
mp_digit_lsb_shift(mp_digit u)
{
	ASSERT(u);

	unsigned steps = 0;
#if MP_DIGIT_BITS != 8
	const mp_digit low8 = 0xff;
	while (!(u & low8))
		steps += 8, u >>= 8;
#endif
	while (!(u & 1))
		steps += 1, u >>= 1;
	return steps;
}
#endif

unsigned
mp_msb_normalize(mp_digit *u, mp_size size)
{
	ASSERT(size != 0);
	ASSERT(u[size - 1] != 0);

	const unsigned shift = mp_digit_msb_shift(u[size - 1]);
	if (shift) {
		ASSERT(mp_lshifti(u, size, shift) == 0);
	}
	return shift;
}

unsigned
mp_odd_shift(const mp_digit *u, mp_size size)
{
	MP_NORMALIZE(u, size);
	if (!size)
		return 0;

	unsigned bits = 0;
	while (*u == 0) {
		++u;
		bits += MP_DIGIT_BITS;
	}
	return bits + mp_digit_lsb_shift(*u);
}

unsigned
mp_significant_bits(const mp_digit *u, mp_size size)
{
	MP_NORMALIZE(u, size);
	if (!size)
		return 0;
	/* +1 because we want to know how many bits it takes to represent most
	 * significant digit */
	size -= 1;
	return (MP_DIGIT_BITS * size) + mp_digit_log2(u[size]) + 1;
}
