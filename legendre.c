/*
 * legendre.c -- Produce the coefficients of Legendre polynomials.
 *
 * Copyright (C) 2003-2010 Farooq Mela. All rights reserved.
 *
 * Legendre polynomials are orthogonal, with respect to the L^2 inner product, on [-1,1].
 * These polynomials are useful in many applications, including least squares minimization problems.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <assert.h>
#include <float.h>

#include "weecrypt.h"

int
main(int argc, char **argv)
{
	int N;
	mpq_poly *p;
	mpq_poly_t tmp, tmp2;
	int j, k;
	mpq_t q, r;

	if (argc == 1) {
		printf("This program will exactly compute the first N Legendre polynomials.\n"
			   "Please enter how many polynomials to compute: ");
		fflush(stdout);
		if (!fscanf(stdin, "%d", &N) || N<=1) {
			fprintf(stderr, "Invalid value for N\n");
			exit(1);
		}
	} else {
		N = atoi(argv[1]);
		if (N <= 1) {
			fprintf(stderr, "Invalid value for N\n");
			exit(1);
		}
	}

	p = MALLOC(sizeof(*p) * N);
	for (k=0; k<N; k++)
		mpq_poly_init(&p[k]);
	mpq_poly_init(tmp);
	mpq_poly_init(tmp2);

	/* P0(x) = 1 */
	mpq_init_ui(p[0].c[0], 1);

	/* P1(x) = x */
	mpq_poly_deg(&p[1], 1);
	mpq_init_ui(p[1].c[1], 1);

	/* P(x) = (2k-1)        k-1
	    k     ------xP(x) + ---P(x)
	             k    k-1    k  k-2 */
	mpq_init(q);
	mpq_init(r);

	for (k=2; k<N; k++) {
		mpq_set_ui_ui(q, 2*k-1, k);
		mpq_poly_mul(&p[1], &p[k-1], tmp);
		mpq_poly_mulq(tmp, q);
		mpq_set_si_si(q,   k-1, -k);
		mpq_poly_set(&p[k-2], tmp2);
		mpq_poly_mulq(tmp2, q);
		mpq_poly_add(tmp, tmp2, &p[k]);
	}

	for (k=0; k<N; k++) {
		mpq_poly_print(&p[k], 'x', "P%d(x)=", k);
		printf("\n");
	}

#if 0
	/* Check for orthogonality on [-1,1] */
	for (j=0; j<N; j++) {
		for (k=j+1; k<N; k++) {
			mpq_poly_mul(&p[j], &p[k], tmp);
			mpq_poly_int(tmp, tmp2);

			mpq_poly_print(tmp2, 'x', "int(p%d * p%d) = ", j, k);
			printf("\n");

			mpq_set_ui(q, 0);
			mpq_poly_eval(tmp2, q, r);
			printf("int(p%d*p%d)|x=0 =", j, k);
			mpq_print_dec(r);
			printf("\n");

			mpq_set_ui(q, 1);
			mpq_poly_eval(tmp2, q, r);
			printf("int(p%d*p%d)|x=1 =", j, k);
			mpq_print_dec(r);
			printf("\n");
		}
	}
#endif

	return 0;
}
