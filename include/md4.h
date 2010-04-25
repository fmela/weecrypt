/* md4.h
 * Copyright (C) 2000-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#ifndef _MD4_H_
#define _MD4_H_

typedef struct {
  unsigned int	md4[ 4];
  unsigned int  len[ 2];	// XXX should switch to u_int64_t?
  unsigned char	buf[64];
} md4_context;

void md4_init(md4_context *ctx);
void md4_update(md4_context *ctx, const void *data, unsigned len);
void md4_final(md4_context *ctx, void *digest);
void md4_hash(md4_context *ctx, const void *input, unsigned len, void *digest); // digest must be 16 bytes

#endif // !__FX_MD4_H__
