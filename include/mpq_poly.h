#ifndef _MPQ_POLY_H_
#define _MPQ_POLY_H_

#include "mpq.h"

typedef struct {	/* rational polynomial */
	int deg;		/* degree N >= 0 */
	mpq_t *c;		/* N+1 coeff's c_0 ... c_N */
} mpq_poly, mpq_poly_t[1];

void mpq_poly_init(mpq_poly_t p);
void mpq_poly_free(mpq_poly_t p);
void mpq_poly_set(const mpq_poly_t p, mpq_poly_t q);
void mpq_poly_swap(mpq_poly_t p, mpq_poly_t q);
void mpq_poly_deg(mpq_poly_t p, int deg);
void mpq_poly_zero(mpq_poly_t p);
void mpq_poly_neg(mpq_poly_t p);
void mpq_poly_normalize(mpq_poly_t p);
void mpq_poly_add(const mpq_poly_t u, const mpq_poly_t v, mpq_poly_t w);

void mpq_poly_mul(const mpq_poly_t u, const mpq_poly_t v, mpq_poly_t w);
void mpq_poly_mul_ui(mpq_poly_t p, unsigned int q);
void mpq_poly_mul_si(mpq_poly_t p,   signed int q);
void mpq_poly_muli(mpq_poly_t p, const mpi_t q);	/* TODO */
void mpq_poly_mulq(mpq_poly_t p, const mpq_t q);

void mpq_poly_div_ui(mpq_poly_t p, unsigned int q);	/* TODO */
void mpq_poly_div_si(mpq_poly_t p,   signed int q);	/* TODO */
void mpq_poly_divi(mpq_poly_t p, const mpi_t q);	/* TODO */

void mpq_poly_print(const mpq_poly_t p, char coeff, const char *fmt, ...);
void mpq_poly_eval(const mpq_poly_t p, const mpq_t x, mpq_t f);
void mpq_poly_dif(const mpq_poly_t p, mpq_poly_t q);
void mpq_poly_int(const mpq_poly_t p, mpq_poly_t q);
int  mpq_poly_equ(const mpq_poly_t u, const mpq_poly_t v);

#endif /* !_MPQ_POLY_H_ */
