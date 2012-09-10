/* mp_modmul.c
 * Copyright (C) 2002-2012 Farooq Mela. All rights reserved. */

#include "mp.h"
#include "mp_internal.h"

/* Set w <- (u * v) mod m
 * w[0..msize-1] u[0..msize-1] v[0..msize-1] */
void
mp_modmul(const mp_digit *u, const mp_digit *v,
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

    mp_digit *tmp = MP_TMP_ALLOC(msize * 2);
    mp_mul_n(u, v, msize, tmp);
    mp_modi(tmp, msize * 2, m, msize);
    mp_copy(tmp, msize, w);
    MP_TMP_FREE(tmp);
}
