/*
 * mp_modsub.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mp.h"
#include "mp_defs.h"

/* If U = V, W = 0
 * If U > V, W = (U - V) mod M
 * If U < V, W = M - ((V - U) mod M) */
void
mp_modsub(const mp_digit *u, const mp_digit *v,
		  const mp_digit *m, mp_size msize, mp_digit *w)
{
	ASSERT(u != NULL);
	ASSERT(v != NULL);
	ASSERT(m != NULL);
	ASSERT(msize != 0);
	ASSERT(m[msize - 1] != 0);
	ASSERT(w != NULL);

	ASSERT(mp_cmp_n(u, m, msize) < 0);
	ASSERT(mp_cmp_n(v, m, msize) < 0);

	const int cmp = mp_cmp_n(u, v, msize);
	if (cmp == 0) {
		mp_zero(w, msize);
	} else if (cmp > 0) {
		ASSERT(mp_sub_n(u, v, msize, w) == 0);
	} else /* cmp < 0 */ {
		ASSERT(mp_sub_n(v, u, msize, w) == 0);	/* w <- v - u */
		ASSERT(mp_sub_n(m, w, msize, w) == 0);	/* w <- m - w */
	}
}
