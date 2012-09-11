/* mpi.h
 * Copyright (C) 2001-2012 Farooq Mela. All rights reserved.
 *
 * Arbitrary precision integer functions. */

#ifndef _MPI_H_
#define _MPI_H_

#include <stdio.h>

#include "mp.h"
#include "mp_internal.h"

typedef struct {
    mp_digit*	digits;	    /* Digits of number. */
    mp_size	size;	    /* Length of number. */
    mp_size	alloc;	    /* Size of allocation. */
    unsigned	sign:1;	    /* Sign bit. */
} mpi, mpi_t[1];

#define MPI_INITIALIZER	{ { NULL, 0, 0, 0 } }

void	mpi_init(mpi *p);
void	mpi_init_mpi(mpi *p, const mpi *q);
void	mpi_init_size(mpi *p, mp_size size);
void	mpi_init_str(mpi *p, const char *str, unsigned base);
void	mpi_init_mp(mpi *p, mp_digit *n, mp_size size);
void	mpi_init_u32(mpi *p, uint32_t q);
void	mpi_init_s32(mpi *p, int32_t q);
void	mpi_init_u64(mpi *p, uint64_t q);
void	mpi_init_s64(mpi *p, int64_t q);
void	mpi_free(mpi *p);
void	mpi_free_zero(mpi *p);

void	mpi_set_mpi(mpi *p, const mpi *q);
void	mpi_set_u32(mpi *p, uint32_t q);
void	mpi_set_s32(mpi *p, int32_t q);
void	mpi_set_u64(mpi *p, uint64_t q);
void	mpi_set_s64(mpi *p, int64_t q);
bool	mpi_set_str(mpi *p, const char *str, unsigned base);

bool	mpi_get_u32(const mpi *p, uint32_t *q);
bool	mpi_get_s32(const mpi *p, int32_t *q);
bool	mpi_get_u64(const mpi *p, int64_t *q);
bool	mpi_get_s64(const mpi *p, uint64_t *q);

float	mpi_get_f(const mpi *p);
double	mpi_get_d(const mpi *p);

#define mpi_is_zero(n)	    ((n)->size == 0)
#define mpi_is_one(n)	    ((n)->size == 1 && (n)->sign == 0 && \
			     (n)->digits[0] == 1)
#define mpi_is_negone(n)    ((n)->size == 1 && (n)->sign == 1 && \
			     (n)->digits[0] == 1)
#define mpi_is_pos(n)	    ((n)->size != 0 && (n)->sign == 0)
#define mpi_is_neg(n)	    ((n)->sign)
#define mpi_is_even(n)	    ((n)->size == 0 ? 1 : (0 == ((n)->digits[0] & 1)))
#define mpi_is_odd(n)	    ((n)->size == 0 ? 0 : (1 == ((n)->digits[0] & 1)))
#define mpi_significant_bits(n)	\
			    mp_significant_bits((n)->digits, (n)->size)

void	mpi_zero(mpi *p);
void	mpi_neg(mpi *p);
void	mpi_abs(mpi *n);
void	mpi_rand(mpi *n, unsigned bits);
void	mpi_rand_ctx(mpi *n, unsigned bits, mt64_context *ctx);
void	mpi_swap(mpi *a, mpi *b);

int	mpi_cmp(const mpi *p, const mpi *q);
#define	mpi_cmp_gt(p,q)	(mpi_cmp((p), (q)) >  0)
#define	mpi_cmp_ge(p,q)	(mpi_cmp((p), (q)) >= 0)
#define	mpi_cmp_lt(p,q)	(mpi_cmp((p), (q)) <  0)
#define	mpi_cmp_le(p,q)	(mpi_cmp((p), (q)) <= 0)
#define mpi_cmp_eq(p,q)	(mpi_cmp((p), (q)) == 0)
#define mpi_cmp_ne(p,q)	(mpi_cmp((p), (q)) != 0)
int	mpi_cmp_u32(const mpi *p, uint32_t q);
int	mpi_cmp_s32(const mpi *p, int32_t q);
int	mpi_cmp_u64(const mpi *p, uint64_t q);
int	mpi_cmp_s64(const mpi *p, int64_t q);

void	mpi_setbit(mpi *p, unsigned bit);
void	mpi_clrbit(mpi *p, unsigned bit);
void	mpi_flpbit(mpi *p, unsigned bit);
void	mpi_tstbit(mpi *p, unsigned bit);
void	mpi_rshift(const mpi *q, unsigned bits, mpi *p);
void	mpi_lshift(const mpi *q, unsigned bits, mpi *p);
void	mpi_shift(const mpi *q, int bits, mpi *p);

/* Increment: A += 1 */
void	mpi_inc(mpi *a);
/* Decrement: A -= 1 */
void	mpi_dec(mpi *b);
/* S = A + B */
void	mpi_add(const mpi *a, const mpi *b, mpi *s);
void	mpi_add_u32(const mpi *a, uint32_t b, mpi *s);
void	mpi_add_s32(const mpi *a, int32_t b, mpi *s);
/* S = A - B */
void	mpi_sub(const mpi *a, const mpi *b, mpi *s);
void	mpi_sub_u32(const mpi *a, uint32_t b, mpi *s);
void	mpi_sub_s32(const mpi *a, int32_t b, mpi *s);
/* P = A * B */
void	mpi_mul(const mpi *a, const mpi *b, mpi *p);
void	mpi_mul_u32(const mpi *a, uint32_t b, mpi *p);
void	mpi_mul_s32(const mpi *a, int32_t b, mpi *p);
void	mpi_mul_u64(const mpi *a, uint64_t b, mpi *p);
void	mpi_mul_s64(const mpi *a, int64_t b, mpi *p);
/* B = A * A */
void	mpi_sqr(const mpi *a, mpi *b);
/* Q = A / B, R = A % B */
void	mpi_divrem(const mpi *a, const mpi *b, mpi *q, mpi *r);
void	mpi_divrem_u32(const mpi *a, uint32_t b, mpi *q, mpi *r);
void	mpi_divrem_s32(const mpi *a, int32_t b, mpi *q, mpi *r);
/* Q = A / B */
void	mpi_div(const mpi *a, const mpi *b, mpi *q);
#define	mpi_div_u32(a, b, q)	mpi_divrem_u32((a), (b), (q), NULL)
#define	mpi_div_s32(a, b, q)	mpi_divrem_s32((a), (b), (q), NULL)
void	mpi_divexact(const mpi *a, const mpi *b, mpi *q);
/* R = A % B */
void	mpi_mod(const mpi *a, const mpi *b, mpi *r);
/* R = A % B (R always positive) */
void	mpi_pmod(const mpi *a, const mpi *b, mpi *r);
/* B = Floor(A^(1/2)) (ignores sign and sets B->sign = A->sign) */
void	mpi_sqrt(const mpi *a, mpi *b);
/* C = GCD(A, B) */
void	mpi_gcd(const mpi *a, const mpi *b, mpi *c);
/* Return 1 if A & B are coprime, 0 otherwise. */
bool	mpi_coprime(const mpi *a, const mpi *b);
/* Extended GCD */
void	mpi_gcdext(const mpi *a, const mpi *b, mpi *u, mpi *v, mpi *d);
/* Modular inverse */
int	mpi_modinv(const mpi *m, const mpi *b, mpi *inv);
/* Modular exponentiation: R = (A ^ P) mod M */
void	mpi_modexp_u32(const mpi *a, uint32_t p, const mpi *m, mpi *r);
void	mpi_modexp(const mpi *a, const mpi *p, const mpi *m, mpi *r);

/* Compute Nth fibonacci number, where F_0 = 0 and F_1 = 1. */
void	mpi_fibonacci(uint64_t n, mpi *fib);
/* Compute factorial of N. */
void	mpi_factorial(uint64_t n, mpi *fact);
/* Compute binomial coefficient N choose K. */
void	mpi_binomial(uint64_t n, uint64_t k, mpi *coeff);

/* Chinese Remainder Theorem algorithm. */
typedef struct {
	unsigned	i;
	mpi_t		x;
	mpi_t		m;
} mpi_crt_ctx;
void	mpi_crt_init(mpi_crt_ctx *ctx);
int	mpi_crt_step(mpi_crt_ctx *ctx, const mpi *a_i, const mpi *m_i);
int	mpi_crt_finish(mpi_crt_ctx *ctx, mpi *a);

void	mpi_fprint(const mpi *n, unsigned base, FILE *fp);
#define mpi_fprint_bin(n,fp)	mpi_fprint((n),  2, (fp))
#define mpi_fprint_oct(n,fp)	mpi_fprint((n),  8, (fp))
#define mpi_fprint_dec(n,fp)	mpi_fprint((n), 10, (fp))
#define mpi_fprint_hex(n,fp)	mpi_fprint((n), 16, (fp))
#define	mpi_print(n,base)	mpi_fprint((n), (base), stdout)
#define mpi_print_bin(n)	mpi_print((n),  2)
#define mpi_print_oct(n)	mpi_print((n),  8)
#define mpi_print_dec(n)	mpi_print((n), 10)
#define mpi_print_hex(n)	mpi_print((n), 16)
#define mpi_to_str(n,base)	mp_to_str((n)->digits, (n)->size, (base))

#endif /* !_MPI_H_ */
