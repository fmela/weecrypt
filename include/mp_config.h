/* mp_config.h
 * Copyright (C) 2001-2012 Farooq Mela. All rights reserved. */

#ifndef _MP_CONFIG_H_
#define _MP_CONFIG_H_

/* To override the default, #define MP_DIGIT_SIZE here. */
/* #define MP_DIGIT_SIZE 1 */

#ifndef MP_DIGIT_SIZE
# if defined(__LP64__) || defined(__x86_64__) || defined(__amd64__)
#  define MP_DIGIT_SIZE	8
# else
#  define MP_DIGIT_SIZE	4
# endif
#endif

#if MP_DIGIT_SIZE == 4 && (defined(i386) || defined(__i386__))
# define MP_ADDI_N_ASM
# define MP_ADD_N_ASM
# define MP_CMP_N_ASM
# define MP_DDIVI_ASM
# define MP_DDIV_ASM
# define MP_DEC_ASM
# define MP_DMOD_ASM
# define MP_DMULI_ASM
# define MP_DMUL_ADD_ASM
# define MP_DMUL_ASM
# define MP_DMUL_SUB_ASM
# define MP_FILL_ASM
# define MP_FLIP_ASM
# define MP_INC_ASM
# define MP_LSHIFTI_ASM
# define MP_RSIZE_ASM
# define MP_RSHIFTI_ASM
# define MP_SQR_DIAG_ASM
# define MP_SUBI_N_ASM
# define MP_SUB_N_ASM
# define MP_XCHG_ASM
#endif

#if defined(__APPLE__)
# define MP_ASM_NAME(fn)	_ ## fn
#else
# define MP_ASM_NAME(fn)	fn
#endif

/* Tunable parameters - Karatsuba multiplication and squaring cutoff. */
/* #define TUNE_KARATSUBA */

/* If this is defined, then there are external variables by the same name of
 * type mp_size which can be modified at run-time. */
#ifndef TUNE_KARATSUBA
# define KARATSUBA_MUL_THRESHOLD 32
# define KARATSUBA_SQR_THRESHOLD 64
#endif

/* Define this if routines should use alloca() to allocate temporaries on the
 * stack instead of using malloc() and friends. Allocating using alloca() may
 * be faster than allocating using malloc(). */
#define USE_ALLOCA

#endif /* !_MP_CONFIG_H_ */
