/*
 * mpi_binomial.c
 * Copyright (C) 2012 Farooq Mela. All rights reserved.
 */

#include "mpi.h"
#include "mpi_defs.h"

/* Compute binomial coefficient N choose K. */
void
mpi_binomial(uint64_t n, uint64_t k, mpi *coeff)
{
	ASSERT(coeff != NULL);

	/* Trivial cases */
	if (k > n) {
		mpi_zero(coeff);
		return;
	} else if (k == 0 || k == n) {
		mpi_one(coeff);
		return;
	} else if (k == 1 || k == n-1) {
		mpi_set_u64(coeff, n);
		return;
	}

	/*             (N-K+1) x (N-K+2) x ... x N
	  N choose K = ---------------------------
	                     1 x 2 x ... x K       */
	mpi_t num, den;
	mpi_init_u64(num, n - k + 1);
	mpi_init_u32(den, 1);
	for (uint64_t i = 2; i <= k; ++i) {
		mpi_mul_u64(num, n - k + i, num);
		mpi_mul_u64(den, i, den);
		if ((i & 31) == 0) {
			unsigned digits = 0;
			while ((num->digits[digits] | den->digits[digits]) == 0)
				++digits;
			unsigned shift = mp_digit_lsb_shift(num->digits[digits] |
												den->digits[digits]);
			if (digits || shift) {
				mpi_rshift(num, digits * MP_DIGIT_BITS + shift, num);
				mpi_rshift(den, digits * MP_DIGIT_BITS + shift, den);
			}
		}
	}
	mpi_divexact(num, den, coeff);
	mpi_free(num);
	mpi_free(den);
}
