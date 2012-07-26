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
	ASSERT(mpi_cmp(b, m) < 0);

	mpi_t v, g;
	mpi_init(v);
	mpi_init(g);
	mpi_gcdext(b, m, inv, v, g);
	mpi_free(v);
	int g_is_one = mpi_is_one(g);
	mpi_free(g);
	if (g_is_one) {
		if (mpi_is_neg(inv))
			mpi_add(inv, m, inv);
		return 1;
	} else {
		return 0;
	}
}
