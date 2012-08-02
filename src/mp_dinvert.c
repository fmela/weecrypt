/*
 * mp_dinvert.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mp.h"
#include "mp_defs.h"

/* This table was generated from ~/src/misc/mul_inv.c */
static const uint8_t inv8[128] = {
	0x01, 0xab, 0xcd, 0xb7, 0x39, 0xa3, 0xc5, 0xef, 0xf1, 0x1b, 0x3d, 0xa7,
	0x29, 0x13, 0x35, 0xdf, 0xe1, 0x8b, 0xad, 0x97, 0x19, 0x83, 0xa5, 0xcf,
	0xd1, 0xfb, 0x1d, 0x87, 0x09, 0xf3, 0x15, 0xbf, 0xc1, 0x6b, 0x8d, 0x77,
	0xf9, 0x63, 0x85, 0xaf, 0xb1, 0xdb, 0xfd, 0x67, 0xe9, 0xd3, 0xf5, 0x9f,
	0xa1, 0x4b, 0x6d, 0x57, 0xd9, 0x43, 0x65, 0x8f, 0x91, 0xbb, 0xdd, 0x47,
	0xc9, 0xb3, 0xd5, 0x7f, 0x81, 0x2b, 0x4d, 0x37, 0xb9, 0x23, 0x45, 0x6f,
	0x71, 0x9b, 0xbd, 0x27, 0xa9, 0x93, 0xb5, 0x5f, 0x61, 0x0b, 0x2d, 0x17,
	0x99, 0x03, 0x25, 0x4f, 0x51, 0x7b, 0x9d, 0x07, 0x89, 0x73, 0x95, 0x3f,
	0x41, 0xeb, 0x0d, 0xf7, 0x79, 0xe3, 0x05, 0x2f, 0x31, 0x5b, 0x7d, 0xe7,
	0x69, 0x53, 0x75, 0x1f, 0x21, 0xcb, 0xed, 0xd7, 0x59, 0xc3, 0xe5, 0x0f,
	0x11, 0x3b, 0x5d, 0xc7, 0x49, 0x33, 0x55, 0xff
};

/* cf. Knuth vol.2, 3rd ed., 4.5.2 exercise 17, and
 * cf. Jebelean, "An Algorithm for Exact Division," end of section 3
 * Newton's method for computing modular inverse U' of V mod 2^2N, given U
 * such that U*V mod 2^N = 1 (each step doubles precision)
 * U' = ((2 - V * U) * U) mod 2^2N
 * Jebelean gives the same formula stated as:
 * U' = (((1 - V * U) * U) + U) mod 2^2N */
mp_digit
mp_digit_invert(mp_digit v)
{
	if ((v & 1) == 0)	/* V must be odd. */
		return 0;

	mp_digit u = inv8[(v & 0xff) >> 1];
#if MP_DIGIT_SIZE >= 2
	u = ((2 - v * u) * u) & 0xFFFFU;
#if MP_DIGIT_SIZE >= 4
	u = ((2 - v * u) * u) & 0xFFFFFFFFU;
#if MP_DIGIT_SIZE >= 8
	u = ((2 - v * u) * u);
#endif /* MP_DIGIT_SIZE >= 8 */
#endif /* MP_DIGIT_SIZE >= 4 */
#endif /* MP_DIGIT_SIZE >= 2 */
	ASSERT((mp_digit)(u * v) == 1);
	return u;
}
