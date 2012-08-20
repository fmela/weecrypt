/* mp_modadd.c
 * Copyright (C) 2002-2012 Farooq Mela. All rights reserved. */

#include "mp.h"
#include "mp_defs.h"

/* w[0..msize-1] = (u[0..msize-1] + v[0..msize-1]) mod m[0..msize-1]
 * w and m must be unique, but if u=v, u=w, or v=w it's fine */
void
mp_modadd(const mp_digit *u, const mp_digit *v,
		  const mp_digit *m, mp_size msize, mp_digit *w)
{
	mp_digit cy;

	ASSERT(u != NULL);
	ASSERT(v != NULL);
	ASSERT(m != NULL);
	ASSERT(msize != 0);
	ASSERT(m[msize - 1] != 0);
	ASSERT(w != NULL);

	ASSERT(mp_cmp_n(u, m, msize) < 0);
	ASSERT(mp_cmp_n(v, m, msize) < 0);

	cy = mp_add_n(u, v, msize, w);
	if (cy || mp_cmp_n(w, m, msize) >= 0) {
		cy -= mp_sub_n(w, m, msize, w);
		ASSERT(cy == 0);
	}
}
