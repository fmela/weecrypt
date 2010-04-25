/* md5.h
 * Copyright (C) 2000-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#ifndef _MD5_H_
#define _MD5_H_

typedef struct {
	unsigned int	md5[ 4];
	unsigned int	len[ 2];
	unsigned char	buf[64];
} md5_context;

void md5_init(md5_context *ctx);
void md5_update(md5_context *ctx, const void *data, unsigned len);
void md5_final(md5_context *ctx, void *digest);
void md5_hash(const void *input, unsigned len, void *digest); // digest must be 16 bytes

#endif // !_MD5_H_
