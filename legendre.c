/*
 * legendre.c -- Produce the coefficients of Legendre polynomials.
 *
 * Copyright (C) 2003-2010 Farooq Mela. All rights reserved.
 *
 * Legendre polynomials are orthogonal w.r.t the L^2 inner product on [-1,1].
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <assert.h>
#include <float.h>

#include "weecrypt.h"

void legendre(int N);

int
main(int argc, char **argv)
{
    int N;

    if (argc == 1) {
	printf("This program will compute the first N Legendre polynomials.\n"
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
    legendre(N);
    return 0;
}

void legendre(int N)
{
    mpq_poly *p = MALLOC(sizeof(*p) * N);
    for (int j=0; j<N; j++)
	mpq_poly_init(&p[j]);
    mpq_poly_t tmp, tmp2;
    mpq_poly_init(tmp);
    mpq_poly_init(tmp2);

    /* P0(x) = 1 */
    mpq_set_u32(p[0].c[0], 1);

    /* P1(x) = x */
    mpq_poly_set_degree(&p[1], 1);
    mpq_set_u32(p[1].c[1], 1);

    /* P(x) = (2k-1)        k-1
        k     ------xP(x) + ---P(x)
                 k    k-1    k  k-2 */
    mpq_t q;
    mpq_init(q);

    for (int k=2; k<N; k++) {
	mpq_set_u32_u32(q, 2*k-1, k);
	mpq_poly_mul(&p[1], &p[k-1], tmp);
	mpq_poly_mulq(tmp, q);
	mpq_set_s32_s32(q,   k-1, -k);
	mpq_poly_set(&p[k-2], tmp2);
	mpq_poly_mulq(tmp2, q);
	mpq_poly_add(tmp, tmp2, &p[k]);
    }

    for (int k=0; k<N; k++) {
	mpq_poly_print(&p[k], 'x', "P%d(x)=", k);
	printf("\n");
    }

    /* Check for orthogonality on [-1,1] */
    mpq_t r, r1;
    mpq_init(r);
    mpq_init(r1);
    for (int j=0; j<N; j++) {
	for (int k=j; k<N; k++) {
	    mpq_poly_mul(&p[j], &p[k], tmp);
	    mpq_poly_int(tmp, tmp2);

	    mpq_set_s32(q, -1);
	    mpq_poly_eval(tmp2, q, r);

	    mpq_set_u32(q, 1);
	    mpq_poly_eval(tmp2, q, r1);

	    mpq_sub(r1, r, r1);

#if 0
	    mpq_poly_print(tmp2, 'x', "∫p%d(x)p%d(x)dx) = ", j, k);
	    printf("\n");
	    printf("∫p%d(x)p%d(x)dx)|x=-1,1 = ", j, k);
	    mpq_print_dec(r1);
	    printf("\n");
#endif

	    /* ∫p_j(x)p_k(x)dx = 2/(2k+1)d_jk where d_jk is Kronecker delta */
	    if (j == k && !(j == 0 && k == 0)) {
		mpq_set_u32_u32(r, 2, 2*k + 1);
		if (!mpq_cmp_eq(r1, r)) {
		    printf("Polynomials %d and %d not orthogonal!\n", j, k);
		    printf("Expected: ");
		    mpq_print_dec(r);
		    printf("\n");
		    printf("  Actual: ");
		    mpq_print_dec(r1);
		    printf("\n");
		}
	    } else {
		if (!mpq_is_zero(r1)) {
		    printf("Polynomials %d and %d not orthogonal!\n", j, k);
		    printf("Expected: 0\n");
		    printf("  Actual: ");
		    mpq_print_dec(r1);
		    printf("\n");
		}
	    }
	}
    }

    for (int j=0; j<N; j++)
	mpq_poly_free(&p[j]);
    FREE(p);
    mpq_poly_free(tmp);
    mpq_poly_free(tmp2);
    mpq_free(q);
    mpq_free(r);
    mpq_free(r1);
}
