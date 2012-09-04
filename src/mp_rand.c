/* mp_rand.c
 * Copyright (C) 2002-2012 Farooq Mela. All rights reserved. */

#include "mp.h"
#include "mp_defs.h"

/* Try to use all bits output by RNG. */
void
mp_rand_digits(mt64_context *ctx, mp_digit *u, mp_size size)
{
    /* FIXME: not thread safe. */
    static mt64_context static_context = MT64_INITIALIZER;

    if (ctx == NULL)
	ctx = &static_context;

#if MP_DIGIT_SIZE == 1
    while (size >= 8) {
	uint64_t r = mt64_gen_u64(ctx);
	*u++ = r;
	*u++ = r >> 8;
	*u++ = r >> 16;
	*u++ = r >> 24;
	*u++ = r >> 32;
	*u++ = r >> 40;
	*u++ = r >> 48;
	*u++ = r >> 56;
	size -= 8;
    }
    if (size) {
	uint64_t r = mt64_gen_u64(ctx);
	switch (size) {
	    case 7: *u++ = r; r >>= 8;
	    case 6: *u++ = r; r >>= 8;
	    case 5: *u++ = r; r >>= 8;
	    case 4: *u++ = r; r >>= 8;
	    case 3: *u++ = r; r >>= 8;
	    case 2: *u++ = r; r >>= 8;
	    case 1: *u++ = r; r >>= 8;
	}
    }
#elif MP_DIGIT_SIZE == 2
    while (size >= 4) {
	uint64_t r = mt64_gen_u64(ctx);
	*u++ = r;
	*u++ = r >> 16;
	*u++ = r >> 32;
	*u++ = r >> 48;
	size -= 4;
    }
    if (size) {
	uint64_t r = mt64_gen_u64(ctx);
	switch (size) {
	    case 3: *u++ = r; r >>= 16;
	    case 2: *u++ = r; r >>= 16;
	    case 1: *u++ = r; r >>= 16;
	}
    }
#elif MP_DIGIT_SIZE == 4
    while (size >= 2) {
	uint64_t r = mt64_gen_u64(ctx);
	*u++ = r;
	*u++ = r >> 32;
    }
    if (size) {
	ASSERT(size == 1);
	*u++ = mt64_gen_u64(ctx);
    }
#elif MP_DIGIT_SIZE == 8
    while (size--) {
	*u++ = mt64_gen_u64(ctx);
    }
#else
# error Bad or unknown MP_DIGIT_SIZE
#endif
}
