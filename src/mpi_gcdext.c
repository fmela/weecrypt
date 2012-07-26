/*
 * mpi_gcdext.c
 * Copyright (C) 2003-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mpi.h"
#include "mpi_defs.h"

/* Cohen algorithm 1.3.6 (Extended Euclid)
 * Given non-negative integers A and B, this algorithm will determine U, V,
 * and D such that A * U + B * V = G = GCD(A,B) */
void
mpi_gcdext(const mpi *a, const mpi *b, mpi *u, mpi *v, mpi *g)
{
	mpi_t v1, v3, t1, t3, q;

	/* Make sure we have non-negative inputs. */
	ASSERT(a->sign == 0);
	ASSERT(b->sign == 0);

	/* 1. Initialize. */
	mpi_set_u32(u, 1);
	mpi_set_mpi(g, a);
	if (mpi_is_zero(b)) {
		mpi_zero(v);
		return;
	}
	mpi_init_u32(v1, 0);
	mpi_init_mpi(v3, b);

	mpi_init(t1);
	mpi_init(t3);
	mpi_init(q);

	for (;;) {
		/* 2. Finished? */
		if (mpi_is_zero(v3)) {
			/* Set v = (g-a*u)/b */
			mpi_mul(a, u, v);		/* v = a*u */
			mpi_sub(g, v, v);		/* v = g-v */
			mpi_divexact(v, b, v);	/* v = v/b */

			mpi_free(v1);
			mpi_free(v3);
			mpi_free(t1);
			mpi_free(t3);
			mpi_free(q);
			return;
		}
		/* 3. Euclidean step. */
		/* Calculate q and t3 with g = q*v3 + t3. */
		mpi_divrem(g, v3, q, t3);	/* q=floor(g/v3), t3=g mod v3 */
		/* Set t1 = u - q*v1 */
		mpi_mul(q, v1, t1);			/* t1 = q*v1 */
		mpi_sub(u, t1, t1);			/* t1 = u-t1 */
		mpi_set_mpi(u, v1);			/* u = v1 */
		mpi_set_mpi(g, v3);			/* g = v3 */
		mpi_set_mpi(v1, t1);		/* v1 = t1 */
		mpi_set_mpi(v3, t3);		/* v3 = t3 */
	}
}
