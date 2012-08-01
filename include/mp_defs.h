/*
 * mp_defs.h
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#ifndef _MP_DEFS_H_
#define _MP_DEFS_H_

#include "mp_config.h"

#if (defined(i386) || defined(__i386__))
# if MP_DIGIT_SIZE == 1
#  define digit_mul(u,v,hi,lo)												\
	__asm__("mulb %3"														\
			: "=a"(lo), "=a"(hi)											\
			: "%0"(u), "g"(v)												\
			: "cc")
#  define digit_div(n1,n0,d,q,r)											\
	__asm__("divb %4"														\
			: "=a"(q), "=a"(r)												\
			: "0"(n0), "1"(n1),	"g"(d))
#  define digit_mul(u,v,hi,lo)												\
	do {																	\
		mp16_t __p = (mp16_t)(u) * (mp16_t)(v);								\
		(lo) = (mp8_t)__p;													\
		(hi) = (mp8_t)(__p >> 8);											\
	} while (0)
#  define digit_div(n1,n0,d,q,r)											\
	do {																	\
		mp8_t __d = (d);													\
		mp16_t __n = ((mp16_t)(n1) << 8) | (mp16_t)(n0);					\
		(q) = (mp8_t)(__n / __d);											\
		(r) = (mp8_t)(__n % __d);											\
	} while (0)
# elif MP_DIGIT_SIZE == 2
#  define digit_mul(u,v,hi,lo)												\
	__asm__("mulw %3"														\
			: "=a"(lo), "=d"(hi)											\
			: "%0"(u), "g"(v)												\
			: "cc")
#  define digit_div(n1,n0,d,q,r)											\
	__asm__("divw %4"														\
			: "=a"(q), "=d"(r)												\
			: "0"(n0), "1"(n1), "g"(d))
# elif MP_DIGIT_SIZE == 4
#  define digit_mul(u,v,hi,lo)												\
	__asm__("mull %3"														\
			: "=a"(lo), "=d"(hi)											\
			: "%0"((u)), "g"(v)												\
			: "cc")
#  define digit_div(n1,n0,d,q,r)											\
	__asm__("divl %4"														\
			: "=a"(q), "=d"(r)												\
			: "0"(n0), "1"(n1),	"g"(d))
# endif
#elif MP_DIGIT_SIZE == 8 && (defined(__amd64__) || defined(__x86_64__))
# define digit_mul(u,v,hi,lo)												\
	__asm__("mulq %3"														\
			: "=a" (lo), "=d" (hi)											\
			: "%0" ((u)), "rm" ((v)))
# define digit_div(n1,n0,d,q,r)												\
	__asm__("divq %4"														\
			: "=a" (q), "=d" (r)											\
			: "0" ((n0)), "1" ((n1)), "rm" ((d)))
#endif

#ifdef USE_ALLOCA
# include <stdlib.h>
# ifdef __linux__
#  include <alloca.h>
# endif
# define MP_PTR_ALLOC(p,size)	(p) = alloca(size)
# define MP_PTR_FREE(p)			(void)0
# define MP_TMP_ALLOC(n,size)	(n) = alloca((size) * MP_DIGIT_SIZE)
# define MP_TMP_FREE(tmp)		(void)0
#else
# include "mp_memory.h"
# define MP_PTR_ALLOC(p,size)	(p) = xmalloc(size)
# define MP_PTR_FREE(p)			free(p)
# define MP_TMP_ALLOC(n,size)	(n) = mp_new(size)
# define MP_TMP_FREE(n)			mp_free(n)
#endif

#define MP_TMP_ALLOC0(n,size) \
	do { \
		MP_TMP_ALLOC((n), (size)); \
		mp_zero((n), (size)); \
	} while (0)

#define MP_TMP_COPY(n,m,size) \
	do { \
		MP_TMP_ALLOC((n), (size)); \
		mp_copy((m), (size), (n)); \
	} while (0)

void _mp_digit_mul(mp_digit u, mp_digit v, mp_digit *hi, mp_digit *lo);
void _mp_digit_sqr(mp_digit u, mp_digit *hi, mp_digit *lo);
void _mp_digit_div(mp_digit n1, mp_digit n0, mp_digit d,
				   mp_digit *q, mp_digit *r);

/* If we don't have inline assembly versions of these primitive operations,
 * fall back onto the (slow) C versions. */
#ifndef digit_sqr
# ifdef  digit_mul
#  define digit_sqr(u,hi,lo)	digit_mul((u),(u),(hi),(lo))
# else
#  define digit_sqr(u,hi,lo) \
	do { \
		mp_digit __u0, __u1, __hi, __lo; \
		__u0 = (u); __u1 = __u0 >> MP_DIGIT_HSHIFT; __u0 &= MP_DIGIT_LMASK; \
		__lo = __u0 * __u0; \
		__hi = __u1 * __u1; \
		__u1 *= __u0; \
		__u0 = __u1 << (MP_DIGIT_HSHIFT + 1); \
		__u1 >>= (MP_DIGIT_HSHIFT - 1); \
		__hi += __u1 + ((__lo += __u0) < __u0); \
		(lo) = __lo; \
		(hi) = __hi; \
	} while (0)
# endif
#endif

#ifndef digit_mul
# define digit_mul(u,v,hi,lo) \
	do { \
		mp_digit __u0, __u1, __v0, __v1, __lo, __hi; \
		__u0 = (u); __u1 = __u0 >> MP_DIGIT_HSHIFT; __u0 &= MP_DIGIT_LMASK; \
		__v0 = (v); __v1 = __v0 >> MP_DIGIT_HSHIFT; __v0 &= MP_DIGIT_LMASK; \
		__lo = __u0 * __v0; \
		__hi = __u1 * __v1; \
		__u1 *= __v0; \
		__v0 = __u1 << MP_DIGIT_HSHIFT; __u1 >>= MP_DIGIT_HSHIFT; \
		__hi += __u1 + ((__lo += __v0) < __v0); \
		__v1 *= __u0; \
		__u0 = __v1 << MP_DIGIT_HSHIFT; __v1 >>= MP_DIGIT_HSHIFT; \
		__hi += __v1 + ((__lo += __u0) < __u0); \
		(lo) = __lo; \
		(hi) = __hi; \
	} while (0)
#endif

/* Not worth inlining. */
#ifndef digit_div
# define digit_div(n1,n0,d,q,r)	_mp_digit_div((n1), (n0), (d), &(q), &(r))
#endif

/* Commonly used definitions. */
#ifndef FALSE
#define FALSE	0
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#endif

#ifndef SWAP
#define SWAP(a,b,type) \
	do { \
		type __tmp = (a); \
		(a) = (b); \
		(b) = __tmp; \
	} while (0)
#endif

#if defined(__GNUC__)
# define GCC_INLINE		__inline__
# define GCC_CONST		__attribute__((__const__))
# define GCC_UNUSED		__attribute__((__unused__))
# define GCC_NORETURN	__attribute__((__noreturn__))
#else
# define GCC_INLINE
# define GCC_CONST
# define GCC_UNUSED
# define GCC_NORETURN
#endif

#ifndef NDEBUG
# include <stdio.h>
# include <stdlib.h>
# undef ASSERT
# if defined(__GNUC__)
#  define ASSERT(expr)														\
	do {																	\
		if (!(expr)) {														\
			fprintf(stderr, "%s:%d (%s) assertion failed: \"%s\"\n",		\
					__FILE__, __LINE__, __PRETTY_FUNCTION__, #expr);		\
			abort();														\
		}																	\
	} while (0)
# else
#  define ASSERT(expr)														\
	do {																	\
		if (!(expr)) {														\
			fprintf(stderr, "%s:%d assertion failed: \"%s\"\n",				\
					__FILE__, __LINE__, #expr);								\
			abort();														\
		}																	\
	} while (0)
# endif
#else
# define ASSERT(expr)	(void)0
#endif

#endif /* !_MP_DEFS_H_ */
