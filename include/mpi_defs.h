/* mpi_defs.h
 * Copyright (C) 2002-2012 Farooq Mela. All rights reserved. */

#ifndef _MPI_DEFS_H_
#define _MPI_DEFS_H_

#define MPI_MIN_ALLOC(n,s)													\
	do {																	\
		mpi *__n = (n);														\
		mp_size __s = (s);													\
		if (__n->alloc < __s) {												\
			__s = (__s + 3) & ~3U;											\
			__n->digits = mp_resize(__n->digits, __s);						\
			__n->alloc = __s;												\
		}																	\
	} while (0)

#define MPI_SIZE(n,s)														\
	do {																	\
		mpi *__n = (n);														\
		mp_size __s = (s);													\
		__n->size = __s;													\
		if (__n->alloc < __s) {												\
			__s = (__s + 3) & ~3U;											\
			__n->digits = mp_resize(__n->digits, __s);						\
			__n->alloc = __s;												\
		}																	\
	} while (0)

#define MPI_NORMALIZE(n)													\
	do {																	\
		mpi *__n = (n);														\
		while (__n->size && !__n->digits[__n->size - 1])					\
			--__n->size;													\
	} while (0)

#endif /* !_MPI_DEFS_H_ */
