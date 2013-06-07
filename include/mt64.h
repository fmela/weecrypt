/* mt64.h
 * Copyright (C) 2012 Farooq Mela. All rights reserved. */

#ifndef _MT64_H_
#define _MT64_H_

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int		mti;
    uint64_t	mt[312];
} mt64_context;

#define MT64_INITIALIZER	{ 313, { 0 } }

void mt64_init_u64(mt64_context *ctx, uint64_t seed);
void mt64_init_u64_array(mt64_context *ctx,
			 const uint64_t seed[], unsigned seed_size);
uint64_t mt64_gen_u64(mt64_context *ctx);

#ifdef __cplusplus
}
#endif

#endif /* !_MT64_H_ */
