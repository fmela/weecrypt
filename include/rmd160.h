#ifndef _RMD160_H_
#define _RMD160_H_

typedef struct {
	unsigned int	rmd[5];
	unsigned char	buf[64];
	unsigned int	nbuf;
} rmd160_context;

void rmd160_init(rmd160_context *ctx);
void rmd160_update(rmd160_context *ctx, const void *data, unsigned len);
void rmd160_final(rmd160_context *ctx, void *digest);
void rmd160_hash(const void *input, unsigned len, void *digest); // digest must be 20 bytes

#endif /* !_RMD160_H_ */
