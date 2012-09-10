/* mpi_defs.h
 * Copyright (C) 2002-2012 Farooq Mela. All rights reserved. */

#ifndef _MPI_DEFS_H_
#define _MPI_DEFS_H_

#define MPI_MIN_ALLOC(n,s) \
    do { \
	mpi *const __n = (n); \
	const mp_size __s = (s); \
	if (__n->alloc < __s) { \
	    __n->digits = mp_resize(__n->digits, \
				    __n->alloc = ((__s + 3) & ~3U)); \
	} \
    } while (0)

#define MPI_SIZE(n,s) \
    do { \
	mpi *const __n = (n); \
	__n->size = (s); \
	if (__n->alloc < __n->size) { \
	    __n->digits = mp_resize(__n->digits, \
				    __n->alloc = ((__n->size + 3) & ~3U)); \
	} \
    } while (0)

#define MPI_NORMALIZE(n) \
    do { \
	mpi *const __n = (n); \
	while (__n->size && !__n->digits[__n->size - 1]) \
	    --__n->size; \
    } while (0)

#endif /* !_MPI_DEFS_H_ */
