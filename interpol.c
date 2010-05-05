/*
 * interpol.c -- Produce the coefficients of an interpolation polynomial.
 * 08-05-03
 *
 * Takes as input a set of N points x_k and corresponding function values f_k
 * such that f(x_k) = f_k. Produces the exact coefficients of the unique
 * (N-1)th degree interpolation polynomial which passes through the supplied
 * points.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <float.h>

#include "weecrypt.h"

char* strtompq(const char *str, mpq_t n);

/*
 * Given a set of N distinct points x_i for i = 1 .. N and a corresponding
 * set of values f_i, construct a polynomial p(x) of degree at most N-1 such
 * that p(x_i) = f_i for i = 1 .. N
 *
 * Method (Newton's Interpolatory Divided Difference Formula):
 * The polynomial p(x) can be expressed as
 * p(x) = f[x_1] + f[x_1,x_2](x-x_1) + f[x_1,x_2,x_3](x-x_1)(x-x_2) + ...
 *      + f[x_1,x_2,...,xN](x-x_1)(x-x_2)...(x-x_{N-1})
 *
 * where f[x_i,...,x_{i+k}] is the divided difference coefficient defined by:
 *                      f[x_{i+1},...,x_{i+k}] - f[x_i,...,x_{i+k-1}]
 * f[x_i,...,x_{i+k}] = ---------------------------------------------
 *                                   x_{i+k} - x_i
 * and f[x_i] = f_i
 */
int
main(int argc, char **argv)
{
	int i, j, n;
	mpq_t *x, *f;
	mpq_t **ff, t;
	char *p;
	mpq_poly_t tp, pp0, pp1, mp, pp;

	/* Read command line arguments. For example, to find the coefficients of
	 * the 2nd degree polynomial for p(0) = 1, p(1) = 2, p(2) = 3, use
	 * ./interpol 0:1 1:2 2:3, etc. */
	if (argc <= 1) {
		fprintf(stderr, "usage: interpol x_1:f_1 x_2:f_2 ... x_N:f_N\n");
		exit(1);
	}
	n = argc - 1;

	x = MALLOC(sizeof(*x) * n);
	f = MALLOC(sizeof(*f) * n);

	for (j = 0; j < n; j++) {
		/*
		double d,td;

		d = strtod(argv[j+1], &p);
		if (*p != ':') {
			fprintf(stderr, "interpol: bad input format\n");
			exit(1);
		}
		mpq_init_d(x[j], d);
		if ((td = mpq_get_d(x[j])) != d)
			printf("interpol: Warning! Non-exact representation of X=%g: %g\n",
				   d, td);
		*/
		mpq_init(x[j]);
		p = strtompq(argv[j+1], x[j]);
		if (*p != ':') {
			fprintf(stderr, "interpol: bad input format\n");
			exit(1);
		}
		for (i = 0; i < j; i++)
			if (mpq_cmp_eq(x[i], x[j])) {
				fprintf(stderr, "interpol: cannot have duplicate x values\n");
				fprintf(stderr, "x[%d]=", i); mpq_print_dec(x[i]); fprintf(stderr, "\n");
				fprintf(stderr, "x[%d]=", j); mpq_print_dec(x[j]); fprintf(stderr, "\n");
				exit(1);
			}
		/*
		d = strtod(p+1, &p);
		mpq_init_d(f[j], d);
		if (*p != '\0') {
			fprintf(stderr, "interpol: bad input format\n");
			exit(1);
		}
		if ((td = mpq_get_d(f[j])) != d)
			printf("interpol: Warning! Non-exact representation of F(X)=%g: %g\n",
				   d, td);
		*/
		mpq_init(f[j]);
		p = strtompq(p+1, f[j]);
		if (*p != '\0') {
			fprintf(stderr, "interpol: bad input format\n");
			exit(1);
		}
		/*
		printf("f[%d]=", j);
		mpq_print(f[j], 10);
		printf("\n");
		*/
		printf("f[");
		mpq_print_dec(x[j]);
		printf("]=");
		mpq_print_dec(f[j]);
		printf("\n");
	}

	/* Allocate and initialize coefficient table. */
	ff = MALLOC(sizeof(*ff) * n);
	for (i = 0; i < n; i++) {
		ff[i] = MALLOC(sizeof(**ff) * (i+1));
		mpq_init_mpq(ff[i][0], f[i]);
		for (j = 1; j <= i; j++)
			mpq_init(ff[i][j]);
	}

	/* Compute the coefficients by the divided difference formula. */
	/*
	 * for i <- 1 to n do
	 *     f[i,0] <- f_i
	 * for i <- 1 to n-1 do
	 *   for j <- 1 to i do
	 *     f[i,j] <- (f[i,j-1] - f[i-1][j-1]) / (x[i] - x[i-j])
	 * Then the f[i,i] entries are the coefficients for the forward-difference
	 * formula. */
	mpq_init(t);
	for (i = 1; i < n; i++) {
		for (j = 1; j <= i; j++) {
			mpq_sub(ff[i][j-1], ff[i-1][j-1], ff[i][j]);
			mpq_sub(x[i], x[i-j], t);
			mpq_div(ff[i][j], t, ff[i][j]);
		}
	}
	mpq_free(t);

	/* Compute the polynomials p_k=\prod_{i=0}^{k-1}(x-x_i) for i = 0 ... n-1 */
	mpq_poly_init(tp);
	mpq_poly_init(pp0);
	mpq_poly_init(pp1);
	mpq_poly_init(mp);
	mpq_poly_init(pp);

	/* pp <- f(x_0) */
	mpq_set_mpq(pp->c[0], ff[0][0]);

	/* tp <- (x-?) */
	mpq_poly_deg(tp, 1);
	mpq_set_si(tp->c[1], 1);

	/* pp0 <- 1 */
	mpq_set_si(pp0->c[0], 1);

	for (i = 1; i < n; i++) {
		/* tp <- (1-x[i-1]) */
		mpq_set_mpq(tp->c[0], x[i-1]);
		mpq_neg(tp->c[0]);
		mpq_poly_mul(pp0, tp, pp1);
		mpq_poly_set(pp1, mp);
		mpq_poly_mulq(mp, ff[i][i]);
		mpq_poly_add(pp, mp, pp);
		mpq_poly_swap(pp0, pp1);
	}

	/* Now output it. */
#if 0
	printf("Points interpolated by degree-%d polynomial\n", pp->deg);
	printf("p(x)=c0+c1*x+c2*x^2+...+cn*x^n where:\n");
	i = (pp->deg >= 1000) ? 4 :
		(pp->deg >=  100) ? 3 :
		(pp->deg >=   10) ? 2 : 1;
	for (j = pp->deg; j >= 0; j--) {
		printf("c%*d=", i, j);
		mpq_print_dec(pp->c + j);
		printf(" (%.*g)", DBL_DIG+1, mpq_get_d(pp->c + j));
		printf("\n");
	}
	printf("\n");
#endif
	mpq_poly_print(pp, 'x', "Points interpolated by degree-%d polynomial P(x)=", pp->deg);
	printf("\n");

	/* Evaluate it at supplied points. */
	mpq_init(t);
	for (i = 0; i < n; i++) {
		mpq_poly_eval(pp, x[i], t);
		printf("p(%.*g)=", DBL_DIG+1, mpq_get_d(x[i])); mpq_print_dec(t);
		printf("=%.*g", DBL_DIG+1, mpq_get_d(t));
		if (mpq_cmp(t, f[i]))
			printf(" [doesnt match with value=%.*g]",
				   DBL_DIG+1, mpq_get_d(f[i]));
		printf("\n");
	}
	mpq_free(t);

	/* Clean up all our shite. */
	mpq_poly_free(tp);
	mpq_poly_free(pp0);
	mpq_poly_free(pp1);
	mpq_poly_free(mp);
	mpq_poly_free(pp);
	for (i = 0; i < n; i++) {
		for (j = 0; j <= i; j++)
			mpq_free(ff[i][j]);
		FREE(ff[i]);
	}
	FREE(ff);

	for (j = 0; j < n; j++) {
		mpq_free(x[j]);
		mpq_free(f[j]);
	}
	FREE(x);
	FREE(f);

	return 0;
}

char*
strtompq(const char *str, mpq_t n)
{
	int neg = 0;

	while (*str && isspace(*str))
		str++;
	if (*str == '-') {
		neg = 1;
		str++;
	} else if (*str == '+') {
		str++;
	}
	mpq_set_ui(n, 0);
	while (*str && isdigit(*str)) {
		mpi_mul_ui(n->num, 10, n->num);
		mpi_add_ui(n->num, *str++ - '0', n->num);
	}
	if (*str == '.') {
		str++;
		while (*str && isdigit(*str)) {
			mpi_mul_ui(n->num, 10, n->num);
			mpi_add_ui(n->num, *str++ - '0', n->num);
			mpi_mul_ui(n->den, 10, n->den);
		}
	}
	if (neg)
		mpq_neg(n);
	mpq_normalize(n);
	return (char*)str;
}
