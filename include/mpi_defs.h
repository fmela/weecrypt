/*
 * mpi_defs.h
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#ifndef _MPI_DEFS_H_
#define _MPI_DEFS_H_

#define MPI_MIN_ALLOC(n,s)													\
	do {																	\
		if ((n)->alloc < (s)) {												\
			mp_size __s = ((s)+3)&~3U;										\
			(n)->digits = mp_resize((n)->digits, (__s));					\
			(n)->alloc = (__s);												\
		}																	\
	} while (0)

#define MPI_SIZE(n,s)														\
	do {																	\
		MPI_MIN_ALLOC((n), (s));											\
		n->size = (s);														\
	} while (0)

#define MPI_RSIZE(n)														\
		(n)->size = mp_rsize((n)->digits, (n)->size)

#endif /* !_MPI_DEFS_H_ */
