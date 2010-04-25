/*
 * mp_defs.h
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#ifndef _MP_DEFS_H_
#define _MP_DEFS_H_

#include "mp_config.h"

/* XXX Use these instead? */
#if 0
#define MP_DIGIT_MASK		~(mp_digit)0
#define MP_DIGIT_HMASK		(MP_DIGIT_MASK << (MP_DIGIT_HSHIFT))
#define MP_DIGIT_LMASK		(MP_DIGIT_MASK >> (MP_DIGIT_HSHIFT))
#define MP_DIGIT_LSB		(mp_digit)1
#define MP_DIGIT_MSB		(MP_DIGIT_LSB << (MP_DIGIT_BITS - 1))
#endif

#if MP_DIGIT_SIZE == 1
# define MP_DIGIT_MASK		0xff
# define MP_DIGIT_HMASK		0xf0
# define MP_DIGIT_LMASK		0x0f
# define MP_DIGIT_LSB		0x01
# define MP_DIGIT_MSB		0x80
# define MP_DIGIT_BITS		8
# define MP_DIGIT_HSHIFT	4
#elif MP_DIGIT_SIZE == 2
# define MP_DIGIT_MASK		0xffff
# define MP_DIGIT_HMASK		0xff00
# define MP_DIGIT_LMASK		0x00ff
# define MP_DIGIT_LSB		0x0001
# define MP_DIGIT_MSB		0x8000
# define MP_DIGIT_BITS		16
# define MP_DIGIT_HSHIFT	8
#elif MP_DIGIT_SIZE == 4
# define MP_DIGIT_MASK		0xffffffffU
# define MP_DIGIT_HMASK		0xffff0000U
# define MP_DIGIT_LMASK		0x0000ffffU
# define MP_DIGIT_LSB		0x00000001U
# define MP_DIGIT_MSB		0x80000000U
# define MP_DIGIT_BITS		32
# define MP_DIGIT_HSHIFT	16
#elif MP_DIGIT_SIZE == 8
# define MP_DIGIT_MASK		CONST64(0xffffffffffffffff)
# define MP_DIGIT_HMASK		CONST64(0xffffffff00000000)
# define MP_DIGIT_LMASK		CONST64(0x00000000ffffffff)
# define MP_DIGIT_LSB		CONST64(0x0000000000000001)
# define MP_DIGIT_MSB		CONST64(0x8000000000000000)
# define MP_DIGIT_BITS		64
# define MP_DIGIT_HSHIFT	32
#endif

#if defined(__GNUC__)
# if (defined(i386) || defined(__i386__))
#  if MP_DIGIT_SIZE == 1
#  if 0 /* GCC can't deal with these. */
#   define digit_mul(u,v,hi,lo)												\
	__asm__("mulb %3"														\
			: "=a"(lo), "=a"(hi)											\
			: "%0"(u), "g"(v)												\
			: "cc")
#   define digit_div(n1,n0,d,q,r)											\
	__asm__("divb %4"														\
			: "=a"(q), "=a"(r)												\
			: "0"(n0), "1"(n1),	"g"(d))
#  endif
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
#  elif MP_DIGIT_SIZE == 2
#   define digit_mul(u,v,hi,lo)												\
	__asm__("mulw %3"														\
			: "=a"(lo), "=d"(hi)											\
			: "%0"(u), "g"(v)												\
			: "cc")
#   define digit_div(n1,n0,d,q,r)											\
	__asm__("divw %4"														\
			: "=a"(q), "=d"(r)												\
			: "0"(n0), "1"(n1), "g"(d))
#  elif MP_DIGIT_SIZE == 4
#   define digit_mul(u,v,hi,lo)												\
	__asm__("mull %3"														\
			: "=a"(lo), "=d"(hi)											\
			: "%0"((u)), "g"(v)												\
			: "cc")
#   define digit_div(n1,n0,d,q,r)											\
	__asm__("divl %4"														\
			: "=a"(q), "=d"(r)												\
			: "0"(n0), "1"(n1),	"g"(d))
#  endif
# endif /* (i386 || __i386__) */
#endif /* __GNUC__ */

#if defined(_MSC_VER) /* MS Visual C++ */
# if MP_DIGIT_SIZE == 1
#  define digit_mul(u,v,hi,lo) { \
	mp_digit __u = (u), __v = (v); \
	__asm { mov al, byte ptr [__u] } \
	__asm { mul byte ptr [__v] } \
	__asm { mov byte ptr [lo], al } \
	__asm { mov byte ptr [hi], ah } }
#  define digit_div(u1,u0,d,q,r) { \
	mp_digit __u1 = (u1), __u0 = (u0); \
	__asm { mov al, byte ptr [__u0] } \
	__asm { mov ah, byte ptr [__u1] } \
	__asm { div byte ptr [d] } \
	__asm { mov byte ptr [q], al } \
	__asm { mov byte ptr [r], ah } }
# elif MP_DIGIT_SIZE == 2
#  define digit_mul(u,v,hi,lo) { \
	mp_digit __u = (u), __v = (v); \
	__asm { mov ax, word ptr [__u] } \
	__asm { mul word ptr [__v] } \
	__asm { mov word ptr [lo], ax } \
	__asm { mov word ptr [hi], dx } }
#  define digit_sqr(u,hi,lo) { \
	mp_digit __u = (u); \
	__asm { mov ax, word ptr [__u]; } \
	__asm { mul ax } \
	__asm { mov word ptr [lo], ax } \
	__asm { mov word ptr [hi], dx } }
#  define digit_div(u1,u0,d,q,r) { \
	mp_digit __u1 = (u1), __u0 = (u0); \
	__asm { mov ax, word ptr [__u0] } \
	__asm { mov dx, word ptr [__u1] } \
	__asm { div word ptr [d] } \
	__asm { mov word ptr [q], ax } \
	__asm { mov word ptr [r], dx } }
# elif MP_DIGIT_SIZE == 4
#  define digit_mul(u,v,hi,lo) { \
	mp_digit __u = (u), __v = (v); \
	__asm { mov eax, dword ptr [__u] } \
	__asm { mul dword ptr [__v] } \
	__asm { mov dword ptr [lo], eax } \
	__asm { mov dword ptr [hi], edx } }
#  define digit_sqr(u,hi,lo) { \
	mp_digit __u = (u); \
	__asm { mov eax, dword ptr [__u] } \
	__asm { mul eax } \
	__asm { mov dword ptr [lo], eax } \
	__asm { mov dword ptr [hi], edx } }
#  define digit_div(u1,u0,d,q,r) { \
	mp_digit __u1 = (u1), __u0 = (u0); \
	__asm { mov eax, dword ptr [__u0] } \
	__asm { mov edx, dword ptr [__u1] } \
	__asm { div dword ptr [d] } \
	__asm { mov dword ptr [q], eax } \
	__asm { mov dword ptr [r], edx } }
# endif
#endif	/* _MSC_VER */

#ifdef USE_ALLOCA
# ifdef _MSC_VER
#  include <malloc.h>
#  define alloca _alloca
# else
#  include <stdlib.h>
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
		__u1 = (u); __u0 = __u1 & MP_DIGIT_LMASK; __u1 >>= MP_DIGIT_HSHIFT; \
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
		__u1 = (u); __u0 = __u1 & MP_DIGIT_LMASK; __u1 >>= MP_DIGIT_HSHIFT; \
		__v1 = (v); __v0 = __v1 & MP_DIGIT_LMASK; __v1 >>= MP_DIGIT_HSHIFT; \
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
