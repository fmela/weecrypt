#ifndef _RMD160_H_
#define _RMD160_H_

#include <stdint.h>

typedef struct {
	uint64_t	len;
	uint32_t	rmd[5];
	uint8_t		buf[64];
	uint32_t	nbuf;
} rmd160_context;

void rmd160_init(rmd160_context *ctx);
void rmd160_update(rmd160_context *ctx, const void *data, unsigned nbytes);
void rmd160_final(rmd160_context *ctx, unsigned char digest[20]);
void rmd160_hash(const void *input, unsigned len, unsigned char digest[20]);

#endif /* !_RMD160_H_ */
