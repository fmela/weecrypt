/*
 * mpi_factorial.c
 * Copyright (C) 2012 Farooq Mela. All rights reserved.
 */

#include "mpi.h"
#include "mpi_defs.h"

/* Compute N! = N x (N-1) x (N-2) ... x (2) x (1) */
/* TODO: use a better algorithm, e.g. the recursive-splitting algorithm. */
void
mpi_factorial(uint64_t n, mpi *fact)
{
	ASSERT(fact != NULL);

	if (!n) {
		mpi_zero(fact);
	} else {
		mpi_set_u64(fact, n);
		for (uint64_t m = n - 1; m >= 2; --m) {
			mpi_mul_u64(fact, m, fact);
		}
	}
}
