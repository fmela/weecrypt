/*
 * mp_convert.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include <ctype.h>

#include "mp.h"
#include "mp_defs.h"
#include "weecrypt_memory.h"

/* radix_sizes[B] = number of radix-B digits needed to represent an 8-bit
 * unsigned integer; B on [2, 36] */
static const float radix_sizes[37] = {
	/*  0 */ 0.00000000f,
	/*  1 */ 0.00000000f,
	/*  2 */ 8.00000000f,
	/*  3 */ 5.04743803f,
	/*  4 */ 4.00000000f,
	/*  5 */ 3.44541246f,
	/*  6 */ 3.09482246f,
	/*  7 */ 2.84965750f,
	/*  8 */ 2.66666667f,
	/*  9 */ 2.52371901f,
	/* 10 */ 2.40823997f,
	/* 11 */ 2.31251861f,
	/* 12 */ 2.23154357f,
	/* 13 */ 2.16190524f,
	/* 14 */ 2.10119628f,
	/* 15 */ 2.04766420f,
	/* 16 */ 2.00000000f,
	/* 17 */ 1.95720434f,
	/* 18 */ 1.91849973f,
	/* 19 */ 1.88327131f,
	/* 20 */ 1.85102571f,
	/* 21 */ 1.82136199f,
	/* 22 */ 1.79395059f,
	/* 23 */ 1.76851784f,
	/* 24 */ 1.74483434f,
	/* 25 */ 1.72270623f,
	/* 26 */ 1.70196843f,
	/* 27 */ 1.68247934f,
	/* 28 */ 1.66411678f,
	/* 29 */ 1.64677466f,
	/* 30 */ 1.63036038f,
	/* 31 */ 1.61479269f,
	/* 32 */ 1.60000000f,
	/* 33 */ 1.58591891f,
	/* 34 */ 1.57249306f,
	/* 35 */ 1.55967218f,
	/* 36 */ 1.54741123f
};

static const struct {
	mp_digit	max_radix;
	unsigned	max_power;
} radix_table[37] = {
#if MP_DIGIT_SIZE == 1
	/*  0 */ { 0x00,  0 },
	/*  1 */ { 0x00,  0 },
	/*  2 */ { 0x80,  7 },
	/*  3 */ { 0xF3,  5 },
	/*  4 */ { 0x40,  3 },
	/*  5 */ { 0x7D,  3 },
	/*  6 */ { 0xD8,  3 },
	/*  7 */ { 0x31,  2 },
	/*  8 */ { 0x40,  2 },
	/*  9 */ { 0x51,  2 },
	/* 10 */ { 0x64,  2 },
	/* 11 */ { 0x79,  2 },
	/* 12 */ { 0x90,  2 },
	/* 13 */ { 0xA9,  2 },
	/* 14 */ { 0xC4,  2 },
	/* 15 */ { 0xE1,  2 },
	/* 16 */ { 0x10,  1 },
	/* 17 */ { 0x11,  1 },
	/* 18 */ { 0x12,  1 },
	/* 19 */ { 0x13,  1 },
	/* 20 */ { 0x14,  1 },
	/* 21 */ { 0x15,  1 },
	/* 22 */ { 0x16,  1 },
	/* 23 */ { 0x17,  1 },
	/* 24 */ { 0x18,  1 },
	/* 25 */ { 0x19,  1 },
	/* 26 */ { 0x1A,  1 },
	/* 27 */ { 0x1B,  1 },
	/* 28 */ { 0x1C,  1 },
	/* 29 */ { 0x1D,  1 },
	/* 30 */ { 0x1E,  1 },
	/* 31 */ { 0x1F,  1 },
	/* 32 */ { 0x20,  1 },
	/* 33 */ { 0x21,  1 },
	/* 34 */ { 0x22,  1 },
	/* 35 */ { 0x23,  1 },
	/* 36 */ { 0x24,  1 }
#elif MP_DIGIT_SIZE == 2
	/*  0 */ { 0x0000,  0 },
	/*  1 */ { 0x0000,  0 },
	/*  2 */ { 0x8000, 15 },
	/*  3 */ { 0xE6A9, 10 },
	/*  4 */ { 0x4000,  7 },
	/*  5 */ { 0x3D09,  6 },
	/*  6 */ { 0xB640,  6 },
	/*  7 */ { 0x41A7,  5 },
	/*  8 */ { 0x8000,  5 },
	/*  9 */ { 0xE6A9,  5 },
	/* 10 */ { 0x2710,  4 },
	/* 11 */ { 0x3931,  4 },
	/* 12 */ { 0x5100,  4 },
	/* 13 */ { 0x6F91,  4 },
	/* 14 */ { 0x9610,  4 },
	/* 15 */ { 0xC5C1,  4 },
	/* 16 */ { 0x1000,  3 },
	/* 17 */ { 0x1331,  3 },
	/* 18 */ { 0x16C8,  3 },
	/* 19 */ { 0x1ACB,  3 },
	/* 20 */ { 0x1F40,  3 },
	/* 21 */ { 0x242D,  3 },
	/* 22 */ { 0x2998,  3 },
	/* 23 */ { 0x2F87,  3 },
	/* 24 */ { 0x3600,  3 },
	/* 25 */ { 0x3D09,  3 },
	/* 26 */ { 0x44A8,  3 },
	/* 27 */ { 0x4CE3,  3 },
	/* 28 */ { 0x55C0,  3 },
	/* 29 */ { 0x5F45,  3 },
	/* 30 */ { 0x6978,  3 },
	/* 31 */ { 0x745F,  3 },
	/* 32 */ { 0x8000,  3 },
	/* 33 */ { 0x8C61,  3 },
	/* 34 */ { 0x9988,  3 },
	/* 35 */ { 0xA77B,  3 },
	/* 36 */ { 0xB640,  3 }
#elif MP_DIGIT_SIZE == 4
	/*  0 */ { 0x00000000U,  0 },
	/*  1 */ { 0x00000000U,  0 },
	/*  2 */ { 0x80000000U, 31 },
	/*  3 */ { 0xCFD41B91U, 20 },
	/*  4 */ { 0x40000000U, 15 },
	/*  5 */ { 0x48C27395U, 13 },
	/*  6 */ { 0x81BF1000U, 12 },
	/*  7 */ { 0x75DB9C97U, 11 },
	/*  8 */ { 0x40000000U, 10 },
	/*  9 */ { 0xCFD41B91U, 10 },
	/* 10 */ { 0x3B9ACA00U,  9 },
	/* 11 */ { 0x8C8B6D2BU,  9 },
	/* 12 */ { 0x19A10000U,  8 },
	/* 13 */ { 0x309F1021U,  8 },
	/* 14 */ { 0x57F6C100U,  8 },
	/* 15 */ { 0x98C29B81U,  8 },
	/* 16 */ { 0x10000000U,  7 },
	/* 17 */ { 0x18754571U,  7 },
	/* 18 */ { 0x247DBC80U,  7 },
	/* 19 */ { 0x3547667BU,  7 },
	/* 20 */ { 0x4C4B4000U,  7 },
	/* 21 */ { 0x6B5A6E1DU,  7 },
	/* 22 */ { 0x94ACE180U,  7 },
	/* 23 */ { 0xCAF18367U,  7 },
	/* 24 */ { 0x0B640000U,  6 },
	/* 25 */ { 0x0E8D4A51U,  6 },
	/* 26 */ { 0x1269AE40U,  6 },
	/* 27 */ { 0x17179149U,  6 },
	/* 28 */ { 0x1CB91000U,  6 },
	/* 29 */ { 0x23744899U,  6 },
	/* 30 */ { 0x2B73A840U,  6 },
	/* 31 */ { 0x34E63B41U,  6 },
	/* 32 */ { 0x40000000U,  6 },
	/* 33 */ { 0x4CFA3CC1U,  6 },
	/* 34 */ { 0x5C13D840U,  6 },
	/* 35 */ { 0x6D91B519U,  6 },
	/* 36 */ { 0x81BF1000U,  6 }
#elif MP_DIGIT_SIZE == 8
	/*  0 */ { UINT64_C(0x0000000000000000),  0 },
	/*  1 */ { UINT64_C(0x0000000000000000),  0 },
	/*  2 */ { UINT64_C(0x8000000000000000), 63 },
	/*  3 */ { UINT64_C(0xA8B8B452291FE821), 40 },
	/*  4 */ { UINT64_C(0x4000000000000000), 31 },
	/*  5 */ { UINT64_C(0x6765C793FA10079D), 27 },
	/*  6 */ { UINT64_C(0x41C21CB8E1000000), 24 },
	/*  7 */ { UINT64_C(0x3642798750226111), 22 },
	/*  8 */ { UINT64_C(0x8000000000000000), 21 },
	/*  9 */ { UINT64_C(0xA8B8B452291FE821), 20 },
	/* 10 */ { UINT64_C(0x8AC7230489E80000), 19 },
	/* 11 */ { UINT64_C(0x4D28CB56C33FA539), 18 },
	/* 12 */ { UINT64_C(0x1ECA170C00000000), 17 },
	/* 13 */ { UINT64_C(0x780C7372621BD74D), 17 },
	/* 14 */ { UINT64_C(0x1E39A5057D810000), 16 },
	/* 15 */ { UINT64_C(0x5B27AC993DF97701), 16 },
	/* 16 */ { UINT64_C(0x1000000000000000), 15 },
	/* 17 */ { UINT64_C(0x27B95E997E21D9F1), 15 },
	/* 18 */ { UINT64_C(0x5DA0E1E53C5C8000), 15 },
	/* 19 */ { UINT64_C(0xD2AE3299C1C4AEDB), 15 },
	/* 20 */ { UINT64_C(0x16BCC41E90000000), 14 },
	/* 21 */ { UINT64_C(0x2D04B7FDD9C0EF49), 14 },
	/* 22 */ { UINT64_C(0x5658597BCAA24000), 14 },
	/* 23 */ { UINT64_C(0xA0E2073737609371), 14 },
	/* 24 */ { UINT64_C(0x0C29E98000000000), 13 },
	/* 25 */ { UINT64_C(0x14ADF4B7320334B9), 13 },
	/* 26 */ { UINT64_C(0x226ED36478BFA000), 13 },
	/* 27 */ { UINT64_C(0x383D9170B85FF80B), 13 },
	/* 28 */ { UINT64_C(0x5A3C23E39C000000), 13 },
	/* 29 */ { UINT64_C(0x8E65137388122BCD), 13 },
	/* 30 */ { UINT64_C(0xDD41BB36D259E000), 13 },
	/* 31 */ { UINT64_C(0x0AEE5720EE830681), 12 },
	/* 32 */ { UINT64_C(0x1000000000000000), 12 },
	/* 33 */ { UINT64_C(0x172588AD4F5F0981), 12 },
	/* 34 */ { UINT64_C(0x211E44F7D02C1000), 12 },
	/* 35 */ { UINT64_C(0x2EE56725F06E5C71), 12 },
	/* 36 */ { UINT64_C(0x41C21CB8E1000000), 12 }
#endif
};

size_t
mp_string_size(mp_size size, unsigned radix)
{
	ASSERT(radix >= 2);
	ASSERT(radix <= 36);

	if ((radix & (radix - 1)) == 0) {
		unsigned lg = mp_digit_lsb_shift(radix);
		return ((size * MP_DIGIT_BITS + lg - 1) / lg) + 1;
	} else {
		/* Be generous: round up to second next largest integer. */
		return (size_t)(radix_sizes[radix] * (size * MP_DIGIT_SIZE)) + 2;
	}
}

/* XXX Only works for ASCII systems. */
static const unsigned char ascii_to_value[256] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
	0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
	0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
	0x21, 0x22, 0x23, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
	0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
	0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
	0x21, 0x22, 0x23, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

mp_size
mp_string_digits(const char *str, unsigned radix)
{
	ASSERT(radix >= 2);
	ASSERT(radix <= 36);

	while (*str && isspace(*str))
		str++;
	while (*str && *str == '0')
		str++;
	if (*str == '\0')
		return 0;

	const char *p = str;
	while (*p) {
		unsigned value = ascii_to_value[(unsigned)*p];
		if (value >= radix)
			break;
		++p;
	}
	unsigned n_digits = p - str;
	return (mp_size)(n_digits / (radix_sizes[radix] * MP_DIGIT_SIZE)) + 1;
}

#define SIZE_INCREMENT	4
mp_digit *
mp_from_str(const char *str, unsigned radix, mp_size *size)
{
	ASSERT(radix >= 2);
	ASSERT(radix <= 36);

	while (*str && isspace(*str))
		str++;
	while (*str && *str == '0')
		str++;
	if (*str == '\0')
		return 0;

	mp_size rsize = SIZE_INCREMENT;
	mp_digit *r = mp_new0(rsize);
	while (*str) {
		mp_digit value = ascii_to_value[(unsigned)*str++];
		if (value >= radix)
			break;
		mp_digit cy = mp_dmuli(r, rsize, radix);
		if (value)
			cy += mp_daddi(r, rsize, value);
		if (cy) {
			r = mp_resize(r, rsize + SIZE_INCREMENT);
			mp_zero(r + rsize, SIZE_INCREMENT);
			r[rsize] = cy;
			rsize += SIZE_INCREMENT;
		}
	}

	*size = mp_rsize(r, rsize);
	if (*size != rsize)
		r = mp_resize(r, *size); /* can only shrink r */
	return r;
}

static const char radix_chars[37] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

void
mp_fprint(const mp_digit *u, mp_size size, unsigned radix, FILE *fp)
{
	ASSERT(u != NULL);

	ASSERT(radix >= 2);
	ASSERT(radix <= 36);

	MP_NORMALIZE(u, size);
	const size_t string_size = mp_string_size(size, radix) + 1;
	char *str = MP_PTR_ALLOC(string_size);
	char *p = mp_get_str(u, size, radix, str);
	ASSERT(p != NULL);
	fputs(p, fp);
	MP_PTR_FREE(str);
}

char *
mp_get_str(const mp_digit *u, mp_size size, unsigned radix, char *out)
{
	ASSERT(u != NULL);
	ASSERT(radix >= 2);
	ASSERT(radix <= 36);

	MP_NORMALIZE(u, size);
	if (size == 0 || (size == 1 && u[0] < radix)) {
		if (out == NULL)
			out = MALLOC(2);
		out[0] = size ? radix_chars[u[0]] : '0';
		out[1] = '\0';
		return out;
	}

	const mp_digit max_radix = radix_table[radix].max_radix;
	const unsigned max_power = radix_table[radix].max_power;

	if (!out)
		out = MALLOC(mp_string_size(size, radix) + 1);
	char *outp = out;

	if ((radix & (radix - 1)) == 0) {	/* Radix is a power of two. */
		/* We need to extract LG bits for each digit. */
		const unsigned lg = mp_digit_lsb_shift(radix);
		const mp_digit mask = radix - 1; /* mask = ((mp_digit)1 << lg) - 1; */
		const unsigned od = MP_DIGIT_BITS / lg;
		if (MP_DIGIT_BITS % lg == 0) {	/* bases 2 (2^1), 4 (2^2), 16 (2^4) */
			const mp_digit *ue = u + size;

			do {
				mp_digit r = *u;
				unsigned i = 0;
				do {
					*outp++ = radix_chars[r & mask];
					r >>= lg;
				} while (++i < od);
			} while (++u < ue);
		} else {						/* bases 8 (2^3), 32 (2^5) */
			/* Do it the lazy way (for now..) */
			const unsigned shift = lg * od;

			ASSERT(shift < MP_DIGIT_BITS);

			mp_digit *tmp = MP_TMP_COPY(u, size);
			mp_size tsize = size;

			do {
				mp_digit r = mp_rshifti(tmp, tsize, shift);
				tsize -= (tmp[tsize - 1] == 0);
				unsigned i = 0;
				do {
					*outp++ = radix_chars[r & mask];
					r >>= lg;
				} while (++i < od);
			} while (tsize != 0);

			MP_TMP_FREE(tmp);
		}
	} else {
		mp_digit *tmp = MP_TMP_COPY(u, size);
		mp_size tsize = size;

		do {
			/* Multi-precision: divide U by largest power of RADIX to fit in
			 * one mp_digit and extract remainder. */
			mp_digit r = mp_ddivi(tmp, tsize, max_radix);
			tsize -= (tmp[tsize - 1] == 0);
			/* Single-precision: extract K remainders from that remainder,
			 * where K is the largest integer such that RADIX^K < 2^BITS. */
			unsigned i = 0;
			do {
				mp_digit rq = r / radix; /* Hopefully compiler will .. */
				mp_digit rr = r % radix; /* .. combine into a single divide. */
				*outp++ = radix_chars[rr];
				r = rq;
				if (tsize == 0 && r == 0) /* Eliminate any leading zeroes. */
					break;
			} while (++i < max_power);
			ASSERT(r == 0);
			/* Loop until TMP = 0. */
		} while (tsize != 0);

		MP_TMP_FREE(tmp);
	}

	char *f = outp - 1;
	/* Eliminate leading (trailing) zeroes. */
	while (*f == '0')
		--f;
	/* NUL terminate. */
	f[1] = '\0';
	/* Reverse digits. */
	for (char *s = out; s < f; ++s, --f)
		SWAP(*s, *f, char);
	return out;
}
