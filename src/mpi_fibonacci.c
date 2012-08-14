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
mpi_fibonacci(uint64_t n, mpi *fib)
{
	ASSERT(fib != NULL);

	if (n <= 2) {
		if (n == 0)
			mpi_zero(fib);
		else
			mpi_one(fib);
		return;
	}

	/* Set K to highest set bit. */
	uint64_t k = ((uint64_t)1) << (63 - __builtin_clzll(n));

	mpi *a1 = fib;					/* Use output param fib as a1 */

	mpi_t a0, tmp, a;
	mpi_init_u32(a0, 0);			/*  a0 = 0 */
	mpi_set_u32(a1, 1);				/*  a1 = 1 */
	mpi_init(tmp);					/* tmp = 0 */
	mpi_init(a);

	while (k >>= 1) {
#if 1
		mpi_lshift(a0, 1, a);       /* a03 = a0 * 2 */
		mpi_add(a, a1, a);          /*   ... + a1 */
		mpi_sqr(a1, tmp);           /* tmp = a1^2 */
		mpi_sqr(a0, a0);            /* a0 = a0 * a0 */
		mpi_add(a0, tmp, a0);       /*  ... + a1 * a1 */
		mpi_mul(a1, a, a1);         /*  a1 = a1 * a */
#else
		mpi_sqr(a1, a);				/* a12 = a1 * a1 */
		mpi_mul(a0, a1, tmp);		/* tmp = a0 * a1 */
		mpi_lshift(tmp, 1, tmp);	/* tmp = 2 * tmp */
		mpi_sqr(a0, a0);			/* a0 = a0 * a0 */
		mpi_add(a0, a, a0);			/*  ... + a1 * a1 */
		mpi_add(tmp, a, a1);		/* a1 = a1 * a1 + tmp */
#endif
		if (k & n) {
			mpi_swap(a1, a0);		/*  a1 <-> a0 */
			mpi_add(a0, a1, a1);	/*  a1 += a0 */
		}
	}

	/* Now a1 (alias of output parameter fib) = F[n] */

	mpi_free(a0);
	mpi_free(tmp);
	mpi_free(a);
}
