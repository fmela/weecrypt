/*
 * mp_modsqr.c
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mp.h"
#include "mp_defs.h"

/* w[0..msize-1] <- u[0..msize-1]^2 mod M (m[0..msize-1]) */
void
mp_modsqr(const mp_digit *u, const mp_digit *m, mp_size msize, mp_digit *w)
{
	mp_digit *tmp;

	ASSERT(u != NULL);
	ASSERT(m != NULL);
	ASSERT(msize != 0);
	ASSERT(m[msize - 1] != 0);
	ASSERT(w != NULL);

	MP_TMP_ALLOC(tmp, msize * 2);
	mp_sqr(u, msize, tmp);
	mp_modi(tmp, msize * 2, m, msize);
	mp_copy(tmp, msize, w);
	MP_TMP_FREE(tmp);
}
