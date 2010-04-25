#ifndef _RSA_H_
#define _RSA_H_

#include "mpi.h"

/* Everything must be kept private except for n and e.
 * (n,e) is the public key, (n,d) is the private key. */
typedef struct {
	mpi_t	n;		/* modulus n = pq */
	mpi_t	phi;	/* phi = (p-1)(q-1) */
	mpi_t	e;		/* public exponent 1 < e < phi s.t. gcd(e,phi)=1 */
	mpi_t	d;		/* secret exponent 1 < d < phi s.t. ed=1 (mod phi) */
} rsa_ctx;

void rsa_init(rsa_ctx *rsa, unsigned bits, mp_rand_ctx *rand_ctx);
void rsa_free(rsa_ctx *rsa);

/* Transform cleartext into encrypted data. */
void rsa_encrypt(rsa_ctx *ctx, const void *input, unsigned input_size, void *output, unsigned *output_size);
/* Transform encrypted data back to cleartext. */
void rsa_decrypt(rsa_ctx *ctx, const void *input, unsigned input_size, void *output, unsigned *output_size);

#endif /* !_RSA_H_ */
