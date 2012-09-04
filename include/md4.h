/* md4.h
 * Copyright (C) 2000-2012 Farooq Mela. All rights reserved. */

#ifndef _MD4_H_
#define _MD4_H_

#include <stdint.h>

typedef struct {
    uint32_t md4[4];
    uint32_t len[2];
    uint8_t buf[64];
} md4_context;

void md4_init(md4_context *ctx);
void md4_update(md4_context *ctx, const void *data, unsigned len);
void md4_final(md4_context *ctx, void *digest);
void md4_hash(const void *input, unsigned len, void *digest); // 16-byte output

#endif // !__FX_MD4_H__
