/*
 * mp_types.h
 *
 * Type definitions.
 * Copyright (C) 2001-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#ifndef _MP_TYPES_H_
#define _MP_TYPES_H_

#include <limits.h>

#if CHAR_BIT != 8
# error "Your system's CHAR_BIT != 8. You will have problems!"
#endif

/* XXX - Need a better way to get portable sized types. C99 <stdint.h> ?*/

#if defined(_MSC_VER)
/* VC does not define __STDC__ unless /Za option given. */
# ifndef __STDC__
#  define __STDC__ 1
# endif

typedef unsigned char		mp8_t;
# define MP8_MAX			UCHAR_MAX

typedef unsigned short		mp16_t;
# define MP16_MAX			USHRT_MAX

typedef unsigned int		mp32_t;
# define MP32_MAX			UINT_MAX

typedef unsigned __int64	mp64_t;
# define MP64_MAX			UINT64_MAX
# define CONST64(u)			u ## I64
#else

typedef unsigned char		mp8_t;
# define MP8_MAX			UCHAR_MAX

typedef unsigned short		mp16_t;
# define MP16_MAX			USHRT_MAX

# if UINT_MAX == 0xffffffffU
typedef unsigned int		mp32_t;
#  define MP32_MAX			UINT_MAX
# else
typedef unsigned long		mp32_t;
#  define MP32_MAX			ULONG_MAX
# endif

typedef unsigned long long	mp64_t;
# define MP64_MAX			ULLONG_MAX
# define CONST64(u)			u ## LL

#endif

typedef mp32_t				mp_size;
#endif /* !_MP_TYPES_H_ */
