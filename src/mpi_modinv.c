/*
 * mpi_modinv.c
 * Copyright (C) 2003-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mpi.h"
#include "mpi_defs.h"

/* Find multiplicative inverse B^-1 of B (mod M) such that B*B^-1 (mod M) = 1.
 * If such an inverse exists, stores the inverse in INV and returns 1.
 * Returns 0 otherwise. */
int
mpi_modinv(const mpi *m, const mpi *b, mpi *inv)
{
	int r;
	mpi_t v, g;

	ASSERT(mpi_cmp(b, m) < 0);

	mpi_init(v);
	mpi_init(g);
	mpi_gcdext(b, m, inv, v, g);
	if (mpi_is_one(g)) {
		if (mpi_is_neg(inv))
			mpi_add(inv, m, inv);
		r = 1;
	} else {
		r = 0;
	}
	mpi_free(v);
	mpi_free(g);
	return r;
}
