#include "mpq_poly.h"

#include <stdarg.h>

#include "weecrypt_memory.h"

void
mpq_poly_init(mpq_poly_t p)
{
	p->deg = 0;
	p->c = MALLOC(sizeof(p->c[0]));
	mpq_init(p->c[0]);
}

void
mpq_poly_free(mpq_poly_t p)
{
	int i;

	for (i = 0; i <= p->deg; i++)
		mpq_free(p->c[i]);
	FREE(p->c);
}

void
mpq_poly_deg(mpq_poly_t p, int deg)
{
	int j;

	ASSERT(p->deg >= 0);
	ASSERT(deg >= 0);

	if (p->deg < deg) {			/* "Grow" coefficient array. */
		p->c = REALLOC(p->c, sizeof(mpq_t) * (deg+1));
		for (j = p->deg+1; j <= deg; j++)
			mpq_init(p->c[j]);
	} else if (p->deg > deg) {	/* "Shrink" coefficient array. */
		for (j = deg+1; j <= p->deg; j++)
			mpq_free(p->c[j]);
		p->c = REALLOC(p->c, sizeof(p->c[0]) * (deg+1));
	}
	p->deg = deg;
}

void
mpq_poly_zero(mpq_poly_t p)
{
	mpq_poly_deg(p, 0);
	mpq_zero(p->c[0]);
}

void
mpq_poly_neg(mpq_poly_t p)
{
	int j;

	ASSERT(p->deg >= 0);

	for (j = p->deg; j>=0; j--)
		mpq_neg(p->c[j]);
}

void
mpq_poly_normalize(mpq_poly_t p)
{
	int j;

	ASSERT(p->deg >= 0);

	j = p->deg;
	while (j && mpq_is_zero(p->c[j]))
		j--;
	ASSERT(j >= 0);
	mpq_poly_deg(p, j);
}

void
mpq_poly_add(const mpq_poly_t u, const mpq_poly_t v, mpq_poly_t w)
{
	int i, ud, vd;

	ASSERT(u->deg >= 0);
	ASSERT(v->deg >= 0);
	ASSERT(w->deg >= 0);

	ud = u->deg;
	vd = v->deg;

	if (ud > vd) {
		mpq_poly_deg(w, ud);
		for (i = 0; i <= vd; i++)
			mpq_add(u->c[i], v->c[i], w->c[i]);
		for (; i <= ud; i++)
			mpq_set_mpq(w->c[i], u->c[i]);
	} else {
		mpq_poly_deg(w, vd);
		for (i = 0; i <= ud; i++)
			mpq_add(u->c[i], v->c[i], w->c[i]);
		for (; i <= vd; i++)
			mpq_set_mpq(w->c[i], v->c[i]);
		if (ud == vd)
			mpq_poly_normalize(w);
	}
}

void
mpq_poly_mul(const mpq_poly_t u, const mpq_poly_t v, mpq_poly_t w)
{
	int i, j, d;
	mpq_t t, *p;

	ASSERT(u->deg >= 0);
	ASSERT(v->deg >= 0);
	ASSERT(w->deg >= 0);

	d = u->deg + v->deg;
	p = MALLOC(sizeof(*p) * (d+1));
	for (j = 0; j <= d; j++)
		mpq_init(p[j]);

	mpq_init(t);
	for (i = 0; i <= u->deg; i++)
		for (j = 0; j <= v->deg; j++) {
			mpq_mul(u->c[i], v->c[j], t);
			mpq_add(p[i+j], t, p[i+j]);
		}
	mpq_free(t);

	mpq_poly_deg(w, d);
	for (j = 0; j <= d; j++) {
		mpq_set_mpq(w->c[j], p[j]);
		mpq_free(p[j]);
	}
	FREE(p);
	mpq_poly_normalize(w);
}

void
mpq_poly_mul_u32(mpq_poly_t p, unsigned int q)
{
	if (q == 0) {
		mpq_poly_deg(p, 0);
		mpq_zero(p->c[0]);
	} else if (q != 1) {
		int j;
		for (j = 0; j <= p->deg; j++)
			mpq_mul_u32(p->c[j], q, p->c[j]);
	}
}

void
mpq_poly_mul_s32(mpq_poly_t p, signed int q)
{
	if (q == 0) {
		mpq_poly_deg(p, 0);
		mpq_zero(p->c[0]);
	} else if (q != 1) {
		int j;
		for (j = 0; j <= p->deg; j++)
			mpq_mul_s32(p->c[j], q, p->c[j]);
	}
}

void
mpq_poly_mulq(mpq_poly_t p, const mpq_t q)
{
	int j;

	if (mpq_is_zero(q)) {
		mpq_poly_deg(p, 0);
		mpq_zero(p->c[0]);
	} else {
		for (j = 0; j <= p->deg; j++)
			mpq_mul(p->c[j], q, p->c[j]);
	}
}

int
mpq_poly_equ(const mpq_poly_t u, const mpq_poly_t v)
{
	int i;

	if (u->deg != v->deg)
		return 0;

	for (i = 0; i <= u->deg; i++)
		if (!mpq_cmp_eq(u->c[i], v->c[i]))
			return 0;
	return 1;
}

void
mpq_poly_print(const mpq_poly_t p, char coeff, const char *fmt, ...)
{
	int j;

	if (fmt) {
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}

	for (j = p->deg; j > 0; j--) {
		int neg;

		if (mpq_is_zero(p->c[j]))
			continue;

		if ((neg = mpq_is_neg(p->c[j])) != 0) {
			mpq_neg(p->c[j]);
			putchar('-');
		} else {
			if (j != p->deg)
				putchar('+');
		}

		if (mpi_is_one(p->c[j]->den)) {
			if (!mpi_is_one(p->c[j]->num))
				mpi_print_dec(p->c[j]->num);
		} else {
			printf("(");
			mpq_print_dec(p->c[j]);
			printf(")");
		}
		putchar(coeff);
		if (j > 1)
			printf("^%d", j);

		if (neg)
			mpq_neg(p->c[j]);
	}
	if (mpq_is_zero(p->c[0])) {
		/* If the last coefficient is zero, we don't have to bother printing
		 * anything if we've already printed prior coefficients. If we haven't
		 * printed any prior coefficients, just print zero. */
		if (p->deg == 0)
			printf("0");
	} else {
		/* If we already printed previous coefficients and the last coefficient
		 * is positive, print a plus sign. */
		if (p->deg != 0 && mpq_is_pos(p->c[0]))
			printf("+");

		if (mpi_is_one(p->c[j]->den))
			mpi_print_dec(p->c[0]->num);
		else
			mpq_print_dec(p->c[0]);
	}
}

void
mpq_poly_set(const mpq_poly_t p, mpq_poly_t q)
{
	int i;

	mpq_poly_deg(q, p->deg);

	for (i = 0; i <= p->deg; i++)
		mpq_set_mpq(q->c[i], p->c[i]);
}

void
mpq_poly_swap(mpq_poly_t p, mpq_poly_t q)
{
	mpq_poly tp;

	tp = *p;
	*p = *q;
	*q = tp;
}

void
mpq_poly_eval(const mpq_poly_t p, const mpq_t x, mpq_t f)
{
	int j;

	/* Use Horner's rule for polynomial evaluation. */

	/* F <- P[n] */
	mpq_set_mpq(f, p->c[p->deg]);
	/* For j <- n-1 ... 0 */
	for (j = p->deg-1; j >= 0; j--) {
		/* F <- F*x + P[j] */
		mpq_mul(f, x, f);
		mpq_add(f, p->c[j], f);
	}
}

void
mpq_poly_dif(const mpq_poly_t p, mpq_poly_t q)
{
	if (p->deg == 0) {
		mpq_poly_zero(q);
	} else {
		int j;
		mpq_t jj;

		mpq_poly_deg(q, p->deg);
		mpq_init(jj);
		for (j = 1; j <= p->deg; j++) {
			mpq_set_u32(jj, j);
			mpq_mul(p->c[j], jj, q->c[j-1]);
		}
		mpq_free(jj);
		mpq_poly_deg(q, q->deg - 1);
	}
}

void
mpq_poly_int(const mpq_poly_t p, mpq_poly_t q)
{
	if (p->deg == 0) {
		mpq_poly_zero(q);
	} else {
		int j;

		mpq_poly_deg(q, p->deg + 1);
		mpq_zero(q->c[0]);
		for (j = 1; j <= q->deg; j++)
			mpq_div_u32(p->c[j-1], j, q->c[j]);
	}
}
