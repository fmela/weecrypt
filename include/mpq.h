/*
 * mpq.h
 * Copyright (C) 2003-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#ifndef _MPQ_H_
#define _MPQ_H_

#include "mpi.h"

typedef struct mpq {
	mpi_t		num;	/* Numerator */
	mpi_t		den;	/* Denominator */
} mpq;
typedef struct mpq mpq_t[1];

#define	mpq_num(q)	((q)->num)
#define mpq_den(q)	((q)->den)

void	mpq_init(mpq *p);
void	mpq_init_mpq(mpq *p, const mpq *q);
void	mpq_init_mpi(mpq *p, const mpi *q);

void	mpq_init_ui(mpq *p, unsigned int q);
void	mpq_init_si(mpq *p,   signed int q);
void	mpq_init_ul(mpq *p, unsigned long q);
void	mpq_init_sl(mpq *p,   signed long q);
void	mpq_init_f(mpq *p, float f);
void	mpq_init_d(mpq *p, double d);

void	mpq_init_ui_ui(mpq *p, unsigned int n, unsigned int d);
void	mpq_init_si_si(mpq *p, signed int n, signed int d);
void	mpq_init_ul_ul(mpq *p, unsigned long n, unsigned long d);
void	mpq_init_sl_sl(mpq *p, signed long n, signed long d);

void	mpq_free(mpq *p);

void	mpq_set_mpq(mpq *p, const mpq *q);
void	mpq_set_ui(mpq *p, unsigned int q);
void	mpq_set_si(mpq *p,   signed int q);
void	mpq_set_ul(mpq *p, unsigned long q);
void	mpq_set_sl(mpq *p,   signed long q);
void	mpq_set_ui_ui(mpq *p, unsigned int n, unsigned int d);
void	mpq_set_si_si(mpq *p, signed int n, signed int d);
void	mpq_set_ul_ul(mpq *p, unsigned long n, unsigned long d);
void	mpq_set_sl_sl(mpq *p, signed long n, signed long d);

/* Conversion to/from IEEE754 floating point. */
void	mpq_set_f(mpq *p, float f);
void	mpq_set_d(mpq *q, double f);
float	mpq_get_f(const mpq *p);
double	mpq_get_d(const mpq *p);

/* zero = 0/1 */
#define	mpq_is_zero(n)	mpi_is_zero((n)->num)

#if 0
/* undef = 0/0 */
#define mpq_is_undef(n)	(mpi_is_zero((n)->num) && mpi_is_zero((n)->den))
/* +oo = +1/0 */
#define mpq_is_pinf(n)	(mpi_is_one((n)->num) && mpi_is_zero((n)->den))
/* -oo = -1/0 */
#define mpq_is_ninf(n)	(mpi_is_pos((n)->num))
#define mpq_is_inf(n)	(mpi_is_pinf(n) || mpi_is_ninf(n))
#endif

#define mpq_is_pos(n)	(mpi_is_pos((n)->num))
#define mpq_is_neg(n)	(mpi_is_neg((n)->num))
#define mpq_is_one(n)	(mpi_is_one((n)->num) && mpi_is_one((n)->den))

void	mpq_zero(mpq *q);
void	mpq_one(mpq *q);
void	mpq_neg(mpq *q);
void	mpq_abs(mpq *q);
void	mpq_swap(mpq *p, mpq *q);
void	mpq_invert(mpq *p);
void	mpq_normalize(mpq *p);

#if 0
void	mpq_pinf(mpq *q);
void	mpq_ninf(mpq *q);
void	mpq_undef(mpq *q);
#endif

void	mpq_add(const mpq *u, const mpq *v, mpq *w);
void	mpq_sub(const mpq *u, const mpq *v, mpq *w);
void	mpq_mul(const mpq *u, const mpq *v, mpq *w);
void	mpq_mul_si(const mpq *u,   signed int v, mpq *w);
void	mpq_mul_ui(const mpq *u, unsigned int v, mpq *w);
void	mpq_div(const mpq *u, const mpq *v, mpq *w);
void	mpq_div_si(const mpq *u,   signed int v, mpq *w);
void	mpq_div_ui(const mpq *u, unsigned int v, mpq *w);

int		mpq_cmp(const mpq *p, const mpq *q);
int		mpq_cmp_ui(const mpq *p, unsigned int q);
int		mpq_cmp_si(const mpq *p,   signed int q);
int		mpq_cmp_f(const mpq *p,  float q);
int		mpq_cmp_d(const mpq *p, double q);
#define	mpq_cmp_gt(p,q)	(mpq_cmp((p), (q)) >  0)
#define	mpq_cmp_ge(p,q)	(mpq_cmp((p), (q)) >= 0)
#define	mpq_cmp_lt(p,q)	(mpq_cmp((p), (q)) <  0)
#define	mpq_cmp_le(p,q)	(mpq_cmp((p), (q)) <= 0)
#define mpq_cmp_eq(p,q)	(mpq_cmp((p), (q)) == 0)
#define mpq_cmp_ne(p,q)	(mpq_cmp((p), (q)) != 0)

void	mpq_fprint(const mpq *p, unsigned base, FILE *fp);
#define	mpq_fprint_bin(p,fp)	mpq_fprint((p),  2, (fp))
#define	mpq_fprint_oct(p,fp)	mpq_fprint((p),  8, (fp))
#define	mpq_fprint_dec(p,fp)	mpq_fprint((p), 10, (fp))
#define	mpq_fprint_hex(p,fp)	mpq_fprint((p), 16, (fp))
#define	mpq_print(p,base)		mpq_fprint((p), (base), stdout)
#define mpq_print_bin(p)		mpq_print((p),  2)
#define mpq_print_oct(p)		mpq_print((p),  8)
#define mpq_print_dec(p)		mpq_print((p), 10)
#define mpq_print_hex(p)		mpq_print((p), 16)

#endif // !_MPQ_H_
