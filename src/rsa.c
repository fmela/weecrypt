#include "rsa.h"
#include "weecrypt_memory.h"

#include <string.h>

static const unsigned kCompositeTestRounds = 10;

bool
rsa_init_keygen(rsa_ctx *rsa, unsigned bits, mt64_context *rand_ctx)
{
	mpi_t p, q;

	ASSERT(bits >= 8);

	memset(rsa, 0, sizeof(*rsa));

	/* Generate P. */
	mpi_init(p);
	mpi_rand_ctx(p, bits/2, rand_ctx);
	p->digits[0] |= 1;
	while (mp_sieve(p->digits, p->size) ||
		   mp_composite(p->digits, p->size, kCompositeTestRounds))
		mpi_add_u32(p, 2, p);

	/* Generate Q. */
	mpi_init(q);
	mpi_rand_ctx(q, bits-bits/2, rand_ctx);
	q->digits[0] |= 1;
	while (mpi_cmp(p, q) == 0 ||
		   mp_sieve(q->digits, q->size) ||
		   mp_composite(q->digits, q->size, kCompositeTestRounds))
		mpi_add_u32(q, 2, q);

	/* Set N = PQ */
	rsa->n = MALLOC(sizeof(*(rsa->n)));
	mpi_init(rsa->n);
	mpi_mul(p, q, rsa->n);

	/* Set Phi = (P-1)(Q-1) = PQ-P-Q+1 = N-P-Q+1 */
	rsa->phi = MALLOC(sizeof(*(rsa->phi)));
	mpi_init(rsa->phi);
	mpi_sub(rsa->n, p, rsa->phi);	/* phi = n-p   */
	mpi_sub(rsa->phi, q, rsa->phi);	/* phi = phi-q */
	mpi_inc(rsa->phi);			/* phi = phi+1 */

	mpi_free_zero(q);
	mpi_free_zero(p);

	/* Choose an integer E such that 1<E<Phi and E coprime to Phi */
	rsa->e = MALLOC(sizeof(*(rsa->e)));
	mpi_init_u32(rsa->e, (bits > 16) ? 65537 : 3);
	while (!mpi_coprime(rsa->e, rsa->phi)) {
		mpi_add_u32(rsa->e, 2, rsa->e);
		if (mpi_cmp(rsa->e, rsa->phi) >= 0) {
			fprintf(stderr, "Could not find E coprime to Phi: ");
			mpi_fprint_dec(rsa->phi, stderr);
			fprintf(stderr, "\n");
			rsa_free(rsa);
			return false;
		}
	}

	/* Compute D = E^-1 mod Phi */
	rsa->d = MALLOC(sizeof(*(rsa->d)));
	mpi_init(rsa->d);
	if (!mpi_modinv(rsa->phi, rsa->e, rsa->d)) {
		fprintf(stderr, "Inverse of ");
		mpi_fprint_dec(rsa->e, stderr);
		fprintf(stderr, " mod ");
		mpi_fprint_dec(rsa->phi, stderr);
		fprintf(stderr, " not found.\n");
		rsa_free(rsa);
		return false;
	}
	return true;
}

void
rsa_free(rsa_ctx *rsa)
{
	if (rsa->n) {
		mpi_free_zero(rsa->n);
		FREE(rsa->n);
	}
	if (rsa->phi) {
		mpi_free_zero(rsa->phi);
		FREE(rsa->phi);
	}
	if (rsa->e) {
		mpi_free_zero(rsa->e);
		FREE(rsa->e);
	}
	if (rsa->d) {
		mpi_free_zero(rsa->d);
		FREE(rsa->d);
	}
	memset(rsa, 0, sizeof(*rsa));
}

bool rsa_encrypt(rsa_ctx *ctx, const void *input, unsigned input_size,
				 void *output, unsigned *output_size) {
	/* TODO: implement. */
	(void)ctx;
	(void)input;
	(void)input_size;
	(void)output;
	(void)output_size;
	return false;
}

bool rsa_decrypt(rsa_ctx *ctx, const void *input,
				 unsigned input_size, void *output, unsigned *output_size) {
	/* TODO: implement. */
	(void)ctx;
	(void)input;
	(void)input_size;
	(void)output;
	(void)output_size;
	return false;
}
