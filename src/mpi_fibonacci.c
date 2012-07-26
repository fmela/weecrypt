/*
 * mpi_fibonacci.c
 * Copyright (C) 2003-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mpi.h"
#include "mpi_defs.h"

/* Compute the Nth Fibonnaci number F_n, where
 * F_n = 1                  if n <= 2,
 * F_n = F_{n-1} + F_{n-2}  otherwise.
 *
 * This works off of the identity
 *        N
 * [ 0 1 ]  = [ F_{n-1} F_n ]
 * [ 1 1 ]    [ F_n F_{n+1} ]
 *
 * Exponentiation uses binary power algorithm from high bit to low bit.
 */
void
mpi_fibonacci(unsigned n, mpi *fib)
{
	ASSERT(fib != NULL);

	if (n <= 2) {
		mpi_one(fib);
		return;
	}

	/* Scan for highest set bit. TODO: change to binary search. */
	unsigned k = 1U << 31;
	while (!(k & n))
		k >>= 1;

	mpi *a1 = fib;					/* Use output param fib as a1 */

	mpi_t a0, a3, a12, a03;
	mpi_init_u32(a0, 1);			/*  a0 = 1 */
	mpi_set_u32(a1, 0);				/*  a1 = 0 */
	mpi_init_u32(a3, 1);			/*  a3 = 1 */
	mpi_init(a12);					/* a12 = 0 */
	mpi_init(a03);					/* a03 = 0 */

	do {
		mpi_sqr(a1, a12);			/* a12 = a1^2 */
		mpi_add(a0, a3, a03);		/* a03 = a0 + a3 */
		mpi_sqr(a0, a0);			/*  a0 = a0^2 */
		mpi_add(a0, a12, a0);		/*  a0 += a12 (a0^2 + a1^2) */
		mpi_mul(a1, a03, a1);		/*  a1 *= a03 */
		mpi_add(a0, a1, a3);		/*  a3 = a0 * a1 */
		if (k & n) {
			mpi_add(a3, a1, a3);	/*  a3 *= a1 */
			mpi_swap(a1, a0);		/*  a1 <-> a0 */
			mpi_add(a0, a1, a1);	/*  a1 += a0 */
		}
	} while (k >>= 1);

	/* Now a1 (alias of output parameter fib) = F[n] */

	mpi_free(a0);
	mpi_free(a3);
	mpi_free(a12);
	mpi_free(a03);
}
