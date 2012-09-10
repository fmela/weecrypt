#ifndef _RSA_H_
#define _RSA_H_

#include "mpi.h"

/*  (n,e) is the public key, (n,d) is the private key. */
typedef struct {
	mpi* n;		/* modulus n = pq */
	mpi* phi;	/* phi = (p-1)(q-1) */
	mpi* e;		/* public exponent 1 < e < phi s.t. gcd(e,phi)=1 */
	mpi* d;		/* secret exponent 1 < d < phi s.t. ed=1 (mod phi)
			   i.e. e and d are multiplicative inverses mod phi */
} rsa_ctx;

/* Generate the public and private key. Sets n, phi, e, and d. */
bool rsa_init_keygen(rsa_ctx *rsa, unsigned bits, mt64_context *rand_ctx);
/* Initialize the RSA context for encryption using the public key. */
bool rsa_init_public_key(rsa_ctx *rsa, const mpi_t n, const mpi_t e);
/* Initialize the RSA context for decryption using the private key. */
bool rsa_init_private_key(rsa_ctx *rsa, const mpi_t n, const mpi_t d);
/* Clear the context. */
void rsa_free(rsa_ctx *rsa);

/* Transform cleartext into encrypted data. */
bool rsa_encrypt(rsa_ctx *ctx, const void *input, unsigned input_size,
				 void *output, unsigned *output_size);
/* Transform encrypted data back to cleartext. */
bool rsa_decrypt(rsa_ctx *ctx, const void *input, unsigned input_size,
				 void *output, unsigned *output_size);

#endif /* !_RSA_H_ */
