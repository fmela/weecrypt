/* mp.h
 * Copyright (C) 2001-2012 Farooq Mela. All rights reserved.
 *
 * Arbitrary precision arithmetic low-level functions. */

#ifndef _MP_H_
#define _MP_H_

#include <inttypes.h>
#include <stdio.h>	/* for FILE*, stdout */
#include <string.h>	/* for memmove */
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include "mp_config.h"
#include "mt64.h"

#if   MP_DIGIT_SIZE == 1
typedef uint8_t			mp_digit;
# define MP_DIGIT_HMASK		UINT8_C(0xf0)
# define MP_DIGIT_LMASK		UINT8_C(0x0f)
# define MP_DIGIT_LSB		UINT8_C(0x01)
# define MP_DIGIT_MSB		UINT8_C(0x80)
# define MP_DIGIT_BITS		8U
# define MP_DIGIT_HSHIFT	4U
# define MP_DIGIT_MAX		UINT8_MAX
# define MP_FORMAT		"%" PRIu8
# define MP_HEX_FORMAT		"%#02" PRIX8
#elif MP_DIGIT_SIZE == 2
typedef uint16_t		mp_digit;
# define MP_DIGIT_HMASK		UINT16_C(0xff00)
# define MP_DIGIT_LMASK		UINT16_C(0x00ff)
# define MP_DIGIT_LSB		UINT16_C(0x0001)
# define MP_DIGIT_MSB		UINT16_C(0x8000)
# define MP_DIGIT_BITS		16U
# define MP_DIGIT_HSHIFT	8U
# define MP_DIGIT_MAX		UINT16_MAX
# define MP_FORMAT		"%" PRIu16
# define MP_HEX_FORMAT		"%#04" PRIX16
#elif MP_DIGIT_SIZE == 4
typedef uint32_t		mp_digit;
# define MP_DIGIT_HMASK		UINT32_C(0xffff0000)
# define MP_DIGIT_LMASK		UINT32_C(0x0000ffff)
# define MP_DIGIT_LSB		UINT32_C(0x00000001)
# define MP_DIGIT_MSB		UINT32_C(0x80000000)
# define MP_DIGIT_BITS		32U
# define MP_DIGIT_HSHIFT	16U
# define MP_DIGIT_MAX		UINT32_MAX
# define MP_FORMAT		"%" PRIu32
# define MP_HEX_FORMAT		"%#08" PRIX32
#elif MP_DIGIT_SIZE == 8
typedef uint64_t		mp_digit;
# define MP_DIGIT_HMASK		UINT64_C(0xffffffff00000000)
# define MP_DIGIT_LMASK		UINT64_C(0x00000000ffffffff)
# define MP_DIGIT_LSB		UINT64_C(0x0000000000000001)
# define MP_DIGIT_MSB		UINT64_C(0x8000000000000000)
# define MP_DIGIT_BITS		64U
# define MP_DIGIT_HSHIFT	32U
# define MP_DIGIT_MAX		UINT64_MAX
# define MP_FORMAT		"%" PRIu64
# define MP_HEX_FORMAT		"%#016" PRIX64
#else
# error "MP_DIGIT_SIZE must be 1, 2, 4, or 8"
#endif

typedef uint32_t mp_size;

/* Allocate an uninitialized size-digit number. */
mp_digit    *mp_new(mp_size size);
/* Allocate a zeroed out size-digit number. */
mp_digit    *mp_new0(mp_size size);
/* Resize the number U to size digits. */
mp_digit    *mp_resize(mp_digit *u, mp_size size);
/* Allocate a duplicate of u[size]. */
mp_digit    *mp_dup(const mp_digit *u, mp_size size);
/* Release the number U. */
void	    mp_free(mp_digit *u);

/* Set u[0..size-1] to one. */
void	    mp_one(mp_digit *u, mp_size size);
/* Set u[0..size-1] to largest value it can accomodate. */
void	    mp_max(mp_digit *u, mp_size size);
/* Set all digits of u[size] to d. */
mp_digit   *mp_fill(mp_digit *u, mp_size size, mp_digit d);
/* Set u[size] to zero. */
#define	    mp_zero(u, size) memset((u), 0, (size) * MP_DIGIT_SIZE)
/* Flip the bits of all digits of u[size]. */
void	    mp_flip(mp_digit *u, mp_size size);
/* Turn u[size] into it's 2's complement. */
void	    mp_complement(mp_digit *u, mp_size size);
/* Copy u[size] onto v[size]. */
#define	    mp_copy(u, size, v) memmove((v), (u), (size) * MP_DIGIT_SIZE)
/* Exchange u[size] with v[size]. */
void	    mp_xchg(mp_digit *u, mp_digit *v, mp_size size);

/* Compare equally sized U and V. */
int	    mp_cmp_n(const mp_digit *u, const mp_digit *v, mp_size size);
/* Compare u[usize] to v[vsize]; -1 if u < v, 0 if u == v, and +1 if u > v; */
int	    mp_cmp(const mp_digit *u, mp_size usize,
		   const mp_digit *v, mp_size vsize);

#define	    mp_cmp_gt(u,usize,v,vsize)	(mp_cmp((u),(usize),(v),(vsize)) >  0)
#define	    mp_cmp_ge(u,usize,v,vsize)	(mp_cmp((u),(usize),(v),(vsize)) >= 0)
#define	    mp_cmp_lt(u,usize,v,vsize)	(mp_cmp((u),(usize),(v),(vsize)) <  0)
#define	    mp_cmp_le(u,usize,v,vsize)	(mp_cmp((u),(usize),(v),(vsize)) <= 0)
#define	    mp_cmp_eq(u,usize,v,vsize)	(mp_cmp((u),(usize),(v),(vsize)) == 0)
#define	    mp_cmp_ne(u,usize,v,vsize)	(mp_cmp((u),(usize),(v),(vsize)) != 0)

/* Return non-zero if u[size] == 0. */
#define	    mp_is_zero(u,size)	(mp_rsize((u),(size)) == 0)
/* Return non-zero if u[size] == 1. */
#define	    mp_is_one(u,size)	(mp_rsize((u),(size)) == 1 && (u)[0] == 1)
/* Return real size of u[size] with leading zeros removed. */
mp_size	    mp_rsize(const mp_digit *u, mp_size size);

/* Set the b-th bit of u[size]. */
bool	    mp_setbit(mp_digit *u, mp_size, unsigned b);
/* Clear the b-th bit of u[size]. */
bool	    mp_clearbit(mp_digit *u, mp_size, unsigned b);
/* Flip the b-th bit of u[size]. */
bool	    mp_flipbit(mp_digit *u, mp_size, unsigned b);
/* Test the b-th bit of u[size]. */
bool	    mp_testbit(mp_digit *u, mp_size, unsigned b);

/* Set u[size] = u[size] &  v[size]. */
void	    mp_and(mp_digit *u, mp_size size, const mp_digit *v);
/* Set u[size] = u[size] & ~v[size]. */
void	    mp_andnot(mp_digit *u, mp_size size, const mp_digit *v);
/* Set u[size] = u[size] |  v[size]. */
void	    mp_or(mp_digit *u, mp_size size, const mp_digit *v);
/* Set u[size] = u[size] | ~v[size]. */
void	    mp_ornot(mp_digit *u, mp_size size, const mp_digit *v);
/* Set u[size] = u[size] ^  v[size]. */
void	    mp_xor(mp_digit *u, mp_size size, const mp_digit *v);
/* Set u[size] = u[size] ^ ~v[size]. */
void	    mp_xornot(mp_digit *u, mp_size size, const mp_digit *v);

/* Return the number of shift positions that U must be shifted left until its
 * most significant bit is set. Argument MUST be non-zero. */
#if MP_DIGIT_SIZE == 4
# define    mp_digit_msb_shift(u)	__builtin_clz(u)
#elif MP_DIGIT_SIZE == 8
# define    mp_digit_msb_shift(u)	__builtin_clzll(u)
#else
unsigned    mp_digit_msb_shift(mp_digit u);
#endif
/* Return the number of shift positions that U must be shifted right until its
 * least significant bit is set. Argument MUST be non-zero. */
#if MP_DIGIT_SIZE == 4
# define    mp_digit_lsb_shift(u)	__builtin_ctz(u)
#elif MP_DIGIT_SIZE == 8
# define    mp_digit_lsb_shift(u)	__builtin_ctzll(u)
#else
unsigned    mp_digit_lsb_shift(mp_digit u);
#endif
/* Shift U left until it's most significant digit is set, and return the number
 * of positions shifted (which may be zero). size and u[size - 1] may NOT be
 * zero. */
unsigned    mp_msb_normalize(mp_digit *u, mp_size size);
/* Return floor(lg(u)) where lg(x) = binary logarithm of x; argument must be
 * non-zero. */
#define	    mp_digit_log2(u) (MP_DIGIT_BITS - 1 - mp_digit_msb_shift(u))
/* Return the number of bit positions u[size] needs to be shifted right until it
 * is odd; if u[size] is zero, 0 will be returned. */
unsigned    mp_odd_shift(const mp_digit *u, mp_size size);
/* Return the number of significant bits in u[size]. */
unsigned    mp_significant_bits(const mp_digit *u, mp_size size);
/* Compute the multiplicative inverse of a digit, modulo 2^MP_DIGIT_BITS. A
 * number N does not have a modular inverse mod M if N and M are not coprime.
 * Since the radix we work in is always a power of 2, this is simply the
 * requirement that N is odd. */
mp_digit    mp_digit_invert(mp_digit n);
/* Compute the floor of the square root of a digit. */
mp_digit    mp_digit_sqrt(mp_digit n);
/* Compute the great common divisor of two digits. */
mp_digit    mp_digit_gcd(mp_digit u, mp_digit v);

/* Set u[size] = u[size] + 1, and return the carry. */
mp_digit    mp_inc(mp_digit *u, mp_size size);
/* Set u[size] = u[size] - 1, and return the borrow. */
mp_digit    mp_dec(mp_digit *u, mp_size size);

/* Set w[size] = u[size] + v, and return the carry. */
mp_digit    mp_dadd(const mp_digit *u, mp_size size, mp_digit v, mp_digit *w);
/* Set u[size] = u[size] + v and return the carry. */
mp_digit    mp_daddi(mp_digit *u, mp_size size, mp_digit v);

/* Set w[size] = u[size] + v[size] and return the carry. */
mp_digit    mp_add_n(const mp_digit *u, const mp_digit *v, mp_size size,
		     mp_digit *w);
/* Set w[max(usize, vsize)] = u[usize] + v[vsize] and return the carry. */
mp_digit    mp_add(const mp_digit *u, mp_size usize,
		   const mp_digit *v, mp_size vsize, mp_digit *w);

/* Set u[size] = u[size] + v[size] and return the carry. */
/* mp_digit	mp_addi_n(mp_digit *u, const mp_digit *v, mp_size size); */
#define	    mp_addi_n(u,v,size)	mp_add_n(u,v,size,u)
/* Set u[usize] = u[usize] + v[vsize], usize >= vsize, and return the carry. */
mp_digit    mp_addi(mp_digit *u, mp_size usize,
		    const mp_digit *v, mp_size vsize);

/* Set w[size] = u[size] - v, and return the borrow. */
mp_digit    mp_dsub(const mp_digit *u, mp_size size, mp_digit v, mp_digit *w);
/* Set u[size] = u[size] - v, and return the borrow. */
mp_digit    mp_dsubi(mp_digit *u, mp_size size, mp_digit v);

/* Set u[size] = u[size] - v[size] and return the borrow. */
mp_digit    mp_subi_n(mp_digit *u, const mp_digit *v, mp_size size);
/* Set w[size] = u[size] - v[size], and return the borrow. */
mp_digit    mp_sub_n(const mp_digit *u, const mp_digit *v,
		     mp_size size, mp_digit *w);
/* Set w[size] = |u[size] - v[size]| and return the sign (-1, 0, or +1) */
int	    mp_diff_n(const mp_digit *u, const mp_digit *v, mp_size size,
		      mp_digit *w);

/* Set u[usize] = u[usize] - v[vsize], usize >= vsize, and return the borrow. */
mp_digit    mp_subi(mp_digit *u, mp_size usize,
		    const mp_digit *v, mp_size vsize);
/* Set w[usize] = u[usize] - v[vsize], and return the borrow. */
mp_digit    mp_sub(const mp_digit *u, mp_size usize,
		   const mp_digit *v, mp_size vsize, mp_digit *w);

/* Set w[size] = u[size] * v, and return the carry. */
mp_digit    mp_dmul(const mp_digit *u, mp_size size,
					mp_digit v, mp_digit *w);
/* Set u[size] = u[size] * v, and return the carry. */
mp_digit    mp_dmuli(mp_digit *u, mp_size size, mp_digit v);
/* Set w[size] = w[size] + u[size] * v, and return the carry. */
mp_digit    mp_dmul_add(const mp_digit *u, mp_size size,
			mp_digit v, mp_digit *w);
/* Set w[size] = w[size] - u[size] * v, and return the borrow. */
mp_digit    mp_dmul_sub(const mp_digit *u, mp_size size,
			mp_digit v, mp_digit *w);

/* Set w[usize + vsize] = u[usize] * v[vsize]. */
void	    mp_mul_n(const mp_digit *u, const mp_digit *v, mp_size size,
		     mp_digit *w);
void	    mp_mul(const mp_digit *u, mp_size usize,
		   const mp_digit *v, mp_size vsize, mp_digit *w);
/* Set w[wsize] = u[usize] * v[vsize] mod ((2 ** MP_DIGIT_BITS) * wsize) */
void	    mp_mul_mod_powb(const mp_digit *u, mp_size usize,
			    const mp_digit *v, mp_size vsize,
			    mp_digit *w, mp_size wsize);

/* Set v[usize*2] = u[usize]^2. */
void	    mp_sqr(const mp_digit *u, mp_size usize, mp_digit *v);
/* Set v[usize*exp] = u[usize]^exp. */
void	    mp_exp(const mp_digit *u, mp_size usize, uint64_t exp, mp_digit *v);
/* Square diagonal. */
void	    mp_sqr_diag(const mp_digit *u, mp_size size, mp_digit *v);

/* Set w[size] = u[usize] / v, and return the remainder. */
mp_digit    mp_ddiv(const mp_digit *u, mp_size size, mp_digit v, mp_digit *w);
/* Set u[size] = u[usize] / v, and return the remainder. */
mp_digit    mp_ddivi(mp_digit *u, mp_size size, mp_digit v);
/* Return u[size] % v. */
mp_digit    mp_dmod(const mp_digit *u, mp_size usize, mp_digit v);

/* Divide the number u[usize] by v[vsize], storing the quotient in
 * q[usize-vsize+1]. The most significant bit of V must be set (V must be
 * normalized). The least significant VLEN digits of U will be set to the
 * remainder after division. */
void	    mp_norm_div(mp_digit *u, mp_size usize,
			const mp_digit *v, mp_size vsize, mp_digit *q);

/* Divide the the number u[usize] by v[vsize], storing the quotient in
 * q[usize-vsize+1] and remainder in r[vsize]. The most significant digit of V
 * MUST be non-zero. Either Q or R may be NULL. */
void	    mp_divrem(const mp_digit *u, mp_size usize,
		      const mp_digit *v, mp_size vsize,
		      mp_digit *q, mp_digit *r);
/* Convenience macros for division without remainder and modulus without
 * quotient. */
#define	 mp_div(u,usize,v,vsize,q) mp_divrem((u),(usize),(v),(vsize),(q),NULL)
#define	 mp_mod(u,usize,v,vsize,r) mp_divrem((u),(usize),(v),(vsize),NULL,(r))
void	    mp_divexact(const mp_digit *u, mp_size usize,
			const mp_digit *d, mp_size dsize, mp_digit *q);

/* Return true if V divides U exactly, false otherwise. */
bool	    mp_digit_divides(const mp_digit *u, mp_size usize, mp_digit v);


/* Set u[vsize] = u[usize] mod v[vsize], in-place. */
void	    mp_modi(mp_digit *u, mp_size usize,
		    const mp_digit *v, mp_size vsize);

void	    mp_modadd(const mp_digit *u, const mp_digit *v,
		      const mp_digit *m, mp_size msize, mp_digit *w);
void	    mp_modsub(const mp_digit *u, const mp_digit *v,
		      const mp_digit *m, mp_size msize, mp_digit *w);
/* Compute (U * V) mod M and put the result in W. The most significant digit of
 * M MUST be non-zero. */
void	    mp_modmul(const mp_digit *u, const mp_digit *v,
		      const mp_digit *m, mp_size msize, mp_digit *w);
/* Compute (U ^ 2) mod M and put the result in W. The most significant digit of
 * M MUST be non-zero, and W must be at least as large as M. */
void	    mp_modsqr(const mp_digit *u, const mp_digit *m, mp_size msize,
		      mp_digit *w);
/* Compute (U ^ P) mod M and put the result in W, which must be at least as
 * large as M. */
void	    mp_modexp_u64(const mp_digit *u, mp_size usize, uint64_t exponent,
			  const mp_digit *m, mp_size msize, mp_digit *w);
void	    mp_modexp(const mp_digit *u, mp_size usize,
		      const mp_digit *p, mp_size psize,
		      const mp_digit *m, mp_size msize, mp_digit *w);
void	    mp_mexp(const mp_digit *u, mp_size usize,
		    const mp_digit *p, mp_size psize,
		    const mp_digit *m, mp_size msize, mp_digit *w);

typedef struct {
    const mp_digit* m;
    mp_digit*	    mu;
    mp_size	    k;
} mp_barrett_ctx;

#define		MP_BARRETT_CTX_INITIALIZER	{ NULL, NULL, 0 }

void	    mp_barrett_ctx_init(mp_barrett_ctx *ctx,
				const mp_digit *m, mp_size msize);
void	    mp_barrett_ctx_free(mp_barrett_ctx *ctx);

/* w[0 .. ctx->k-1] = (u ** exponent) mod m */
void	    mp_barrett_u64(const mp_digit *u, mp_size usize, uint64_t exponent,
			   const mp_barrett_ctx *ctx, mp_digit *w);
void	    mp_barrett(const mp_digit *u, mp_size usize,
		       const mp_digit *p, mp_size psize,
		       const mp_barrett_ctx *ctx, mp_digit *w);

void	    mp_modexp_pow2(const mp_digit *u, mp_size usize,
			   const mp_digit *p, mp_size psize,
			   const mp_digit *m, mp_size msize, mp_digit *w);
void	    mp_modexp_pow2_u64(const mp_digit *u, mp_size usize,
			       uint64_t exponent,
			       const mp_digit *m, mp_size msize, mp_digit *w);

/* Compute the GCD of the natural numbers U and V, and store the result at W.
 * W must be at least as large as the smaller of U or V. */
void	    mp_gcd(const mp_digit *u, mp_size usize,
		   const mp_digit *v, mp_size vsize, mp_digit *w);
/* Return true if U and V are relatively prime, false otherwise. */
bool	    mp_coprime(const mp_digit *u, mp_size usize,
		       const mp_digit *v, mp_size vsize);

/* Compute the Jacobi symbol A over P. Returns -1, 0, or +1. */
int	    mp_jacobi(const mp_digit *a, mp_size asize,
		      const mp_digit *p, mp_size psize);

mp_digit    mp_sieve(const mp_digit *u, mp_size size);
/* Run NROUNDS rounds of the Miller-Rabin primality test on U. Return true if
 * composite, false if "probably" prime. */
bool	    mp_composite(const mp_digit *u, mp_size size, unsigned nrounds);

/* Compute the integer portion of the square root of u[size] and store it in
 * v[(size+1)/2], and remainder in r[size]. Either V or R may be NULL. */
void	    mp_sqrtrem(const mp_digit *u, mp_size size,
		       mp_digit *v, mp_digit *r);
#define	    mp_sqrt(u, size, v)	mp_sqrtrem((u), (size), (v), NULL)
/* Return true if u[usize] is a perfect square, false otherwise. */
bool	    mp_perfsqr(const mp_digit *u, mp_size usize);

/* Multiply or divide by a power of two, with power taken modulo MP_DIGIT_BITS,
 * and return the carry (left shift) or remainder (right shift). */
mp_digit    mp_lshift(const mp_digit *u, mp_size size,
		      unsigned shift, mp_digit *v);
mp_digit    mp_rshift(const mp_digit *u, mp_size size,
		      unsigned shift, mp_digit *v);
mp_digit    mp_lshifti(mp_digit *u, mp_size size, unsigned shift);
mp_digit    mp_rshifti(mp_digit *u, mp_size size, unsigned shift);

/* Store a random number in u[size]. */
void	    mp_rand_digits(mt64_context *ctx, mp_digit *u, mp_size size);
#define	    mp_rand(u,size)	mp_rand_digits(NULL, (u), (size))

/* Return the Hamming weight (bit population count) of u[size]. */
unsigned    mp_hamming_weight(const mp_digit *u, mp_size size);
/* Return the Hamming distance of u[size] and v[size]. */
unsigned    mp_hamming_dist(const mp_digit *u, mp_size size, const mp_digit *v);

/* Return the size, in bytes, that a character string must be in order to hold
 * the representation of a LEN-digit number in BASE. Return value does NOT
 * account for terminating '\0'. */
size_t	    mp_string_size(mp_size size, unsigned radix);
/* Return u[size] as a null-terminated character string in a radix on [2,36]. */
char*	    mp_get_str(const mp_digit *u, mp_size size, unsigned radix,
		       char *out);
#define	    mp_to_str(u,size,radix)	mp_get_str((u),(size),(radix),NULL)
#define	    mp_get_str_bin(u,size)	mp_to_str((u),(size),2)
#define	    mp_get_str_oct(u,size)	mp_to_str((u),(size),8)
#define	    mp_get_str_dec(u,size)	mp_to_str((u),(size),10)
#define	    mp_get_str_hex(u,size)	mp_to_str((u),(size),16)

/* Print u[size] in a radix on [2,36] to the stream fp. No newline is output. */
void	    mp_fprint(const mp_digit *u, mp_size size,
		      unsigned radix, FILE *fp);
/* Convenience macros for bases 2, 8, 10, and 16. */
#define	    mp_fprint_bin(u,size,fp)	mp_fprint((u),(size),2,(fp))
#define	    mp_fprint_oct(u,size,fp)	mp_fprint((u),(size),8,(fp))
#define	    mp_fprint_dec(u,size,fp)	mp_fprint((u),(size),10,(fp))
#define	    mp_fprint_hex(u,size,fp)	mp_fprint((u),(size),16,(fp))
/* Convenience macros for bases 2, 8, 10, and 16, with fp = stdout. */
#define	    mp_print(u,size,b)			mp_fprint((u),(size),(b),stdout)
#define	    mp_print_bin(u,size)		mp_fprint_bin((u),(size),stdout)
#define	    mp_print_oct(u,size)		mp_fprint_oct((u),(size),stdout)
#define	    mp_print_dec(u,size)		mp_fprint_dec((u),(size),stdout)
#define	    mp_print_hex(u,size)		mp_fprint_hex((u),(size),stdout)

/* Return the size, in digits, of the number that will required to represent
 * the number in base 'radix' in character string 'str'. */
mp_size	    mp_string_digits(const char *str, unsigned radix);

mp_digit*   mp_from_str(const char *u, unsigned radix, mp_size *size);
#define	    mp_from_str_bin(u,size)	mp_from_str((u),2,(size))
#define	    mp_from_str_oct(u,size)	mp_from_str((u),8,(size))
#define	    mp_from_str_dec(u,size)	mp_from_str((u),10,(size))
#define	    mp_from_str_hex(u,size)	mp_from_str((u),16,(size))

#define MP_NORMALIZE(u,usize) \
    while ((usize) && !(u)[(usize) - 1]) --(usize);

#endif /* !_MP_H_ */
