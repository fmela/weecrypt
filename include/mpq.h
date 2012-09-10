/* mpq.h
 * Copyright (C) 2003-2012 Farooq Mela. All rights reserved. */

#ifndef _MPQ_H_
#define _MPQ_H_

#include "mpi.h"

typedef struct {
    mpi_t   num;    /* Numerator. */
    mpi_t   den;    /* Denominator. */
} mpq, mpq_t[1];

#define	mpq_num(q)	((q)->num)
#define mpq_den(q)	((q)->den)

void	mpq_init(mpq *p);
void	mpq_init_mpq(mpq *p, const mpq *q);
void	mpq_init_mpi(mpq *p, const mpi *q);

void	mpq_init_u32(mpq *p, uint32_t q);
void	mpq_init_s32(mpq *p, int32_t q);
void	mpq_init_u64(mpq *p, uint64_t q);
void	mpq_init_s64(mpq *p, int64_t q);
void	mpq_init_f(mpq *p, float f);
void	mpq_init_d(mpq *p, double d);

void	mpq_init_u32_u32(mpq *p, uint32_t n, uint32_t d);
void	mpq_init_s32_s32(mpq *p, int32_t n, int32_t d);
void	mpq_init_u64_u64(mpq *p, uint64_t n, uint64_t d);
void	mpq_init_s64_s64(mpq *p, int64_t n, int64_t d);

void	mpq_free(mpq *p);

void	mpq_set_mpq(mpq *p, const mpq *q);
void	mpq_set_u32(mpq *p, uint32_t q);
void	mpq_set_s32(mpq *p, int32_t q);
void	mpq_set_u64(mpq *p, uint64_t q);
void	mpq_set_s64(mpq *p, int64_t q);
void	mpq_set_u32_u32(mpq *p, uint32_t n, uint32_t d);
void	mpq_set_s32_s32(mpq *p, int32_t n, int32_t d);
void	mpq_set_u64_u64(mpq *p, uint64_t n, uint64_t d);
void	mpq_set_s64_s64(mpq *p, int64_t n, int64_t d);

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
void	mpq_mul_s32(const mpq *u, int32_t v, mpq *w);
void	mpq_mul_u32(const mpq *u, uint32_t v, mpq *w);
void	mpq_muli(mpq *w, const mpi *v);
void	mpq_div(const mpq *u, const mpq *v, mpq *w);
void	mpq_div_s32(const mpq *u, int32_t v, mpq *w);
void	mpq_div_u32(const mpq *u, uint32_t v, mpq *w);
void	mpq_divi(mpq *w, const mpi *v);

int	mpq_cmp(const mpq *p, const mpq *q);
int	mpq_cmp_u32(const mpq *p, uint32_t q);
int	mpq_cmp_s32(const mpq *p, int32_t q);
int	mpq_cmp_f(const mpq *p,  float q);
int	mpq_cmp_d(const mpq *p, double q);
#define	mpq_cmp_gt(p,q)		(mpq_cmp((p), (q)) >  0)
#define	mpq_cmp_ge(p,q)		(mpq_cmp((p), (q)) >= 0)
#define	mpq_cmp_lt(p,q)		(mpq_cmp((p), (q)) <  0)
#define	mpq_cmp_le(p,q)		(mpq_cmp((p), (q)) <= 0)
#define mpq_cmp_eq(p,q)		(mpq_cmp((p), (q)) == 0)
#define mpq_cmp_ne(p,q)		(mpq_cmp((p), (q)) != 0)

void	mpq_fprint(const mpq *p, unsigned base, FILE *fp);
#define	mpq_fprint_bin(p,fp)	mpq_fprint((p),  2, (fp))
#define	mpq_fprint_oct(p,fp)	mpq_fprint((p),  8, (fp))
#define	mpq_fprint_dec(p,fp)	mpq_fprint((p), 10, (fp))
#define	mpq_fprint_hex(p,fp)	mpq_fprint((p), 16, (fp))
#define	mpq_print(p,base)	mpq_fprint((p), (base), stdout)
#define mpq_print_bin(p)	mpq_print((p),  2)
#define mpq_print_oct(p)	mpq_print((p),  8)
#define mpq_print_dec(p)	mpq_print((p), 10)
#define mpq_print_hex(p)	mpq_print((p), 16)

#endif // !_MPQ_H_
