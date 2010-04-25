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
 * Power algorithm from high bit to low bit.
 */
void
mpi_fibonacci(unsigned n, mpi *fib)
{
	unsigned k;
	mpi_t a0, a1, a3;
	mpi_t a12, a03;

	if (n <= 2) {
		mpi_one(fib);
		return;
	}

	k = 1U << 31;
	while ((k & n) == 0)
		k >>= 1;

	mpi_init_ui(a0, 1);				/*  a0 = 1 */
	mpi_init(a1);					/*  a1 = 0 */
	mpi_init_ui(a3, 1);				/*  a3 = 1 */
	mpi_init(a12);					/* a12 = 0 */
	mpi_init(a03);					/* a03 = 0 */

	do {
		mpi_sqr(a1, a12);			/* a12 = a1^2 */
		mpi_add(a0, a3, a03);		/* a03 = a0 + a3 */
		mpi_sqr(a0, a0);			/*  a0 *= a0 */
		mpi_add(a0, a12, a0);		/*  a0 += a12 */
		mpi_mul(a1, a03, a1);		/*  a1 *= a03 */
		mpi_add(a0, a1, a3);		/*  a3 = a0 * a1 */
		if (k & n) {
			mpi_add(a3, a1, a3);	/*  a3 *= a1 */
			mpi_swap(a1, a0);		/*  a1 <-> a0 */
			mpi_add(a0, a1, a1);	/*  a1 += a0 */
		}
	} while (k >>= 1);

	mpi_set_mpi(fib, a1);			/* F[n] = a1 */

	mpi_free(a0);
	mpi_free(a1);
	mpi_free(a3);
	mpi_free(a12);
	mpi_free(a03);
}
