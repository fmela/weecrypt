/* mp_jacobi.c
 * Copyright (C) 2002-2012 Farooq Mela. All rights reserved. */

#include "mp.h"
#include "mp_defs.h"

int
mp_jacobi(const mp_digit *a, mp_size asize,
		  const mp_digit *p, mp_size psize)
{
	/* A precomputed table of values for evaluating the (-1)^((A^2-1)/8)
	 * terms in the Kronecker algorithm. To evalute this for A, index the table
	 * with (A & 7) which is equivalent to A mod 8. */
	static const int etab[8] = { 0, +1, 0, -1, 0, -1, 0, +1 };
	int k;
	mp_digit *atmp, *ptmp, *mtmp;

	ASSERT(a != NULL);
	ASSERT(p != NULL);

	MP_NORMALIZE(a, asize);
	MP_NORMALIZE(p, psize);

	/* J(0,1) = 1, otherwise J(0,x) = 0 */
	if (asize == 0)
		return (psize == 1 && p[0] == 1);

	/* J(2x,2y) = 0 */
	if (((a[0] | p[0]) & 1) == 0)
		return 0;

	/* Allocate temporaries. */
	atmp = MP_TMP_COPY(a, asize);
	ptmp = MP_TMP_COPY(p, psize);
	mtmp = MP_TMP_ALLOC(asize);

	a = NULL; p = NULL;

	if (mp_odd_shift(ptmp, psize) & 1)
		k = etab[atmp[0] & 7];
	else
		k = 1;

	do {
		if (mp_odd_shift(atmp, asize) & 1)
			k = etab[ptmp[0] & 7];
		if (atmp[0] & ptmp[0] & 2)
			k = -k;
		/* M = A */
		mp_copy(atmp, asize, mtmp);
		/* A = P mod M */
		mp_mod(ptmp, psize, mtmp, asize, atmp);
		/* P = M */
		mp_copy(mtmp, psize = asize, ptmp);
	} while ((asize = mp_rsize(atmp, asize)) != 0);

	/* If P = 0, return 0, otherwise return K. */
	if (mp_rsize(ptmp, psize) == 0)
		k = 0;

	/* Free temporaries. */
	MP_TMP_FREE(atmp);
	MP_TMP_FREE(ptmp);
	MP_TMP_FREE(mtmp);

	return k;
}
