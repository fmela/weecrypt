/* md2.h
 * Copyright (C) 2001-2012 Farooq Mela. All rights reserved. */

#ifndef _MD2_H_
#define _MD2_H_

#include "mp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned char   md2[16];
    unsigned char   cksum[16];
    unsigned char   buf[16];
    unsigned int    len;
} md2_context;

void md2_init(md2_context *ctx);
void md2_update(md2_context *ctx, const void *input, unsigned len);
void md2_final(md2_context *ctx, void *digest);
void md2_hash(const void *in, unsigned len, void *digest);

#ifdef __cplusplus
}
#endif

#endif /* !_MD2_H_ */
