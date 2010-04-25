#include "rsa.h"

#include <string.h>

void
rsa_init(rsa_ctx *rsa, unsigned bits, mp_rand_ctx *rand_ctx)
{
	mpi_t p, q;

	ASSERT(bits >= 128);

	memset(rsa, 0, sizeof(*rsa));

	/* Generate P. */
	mpi_init(p);
	mpi_rand_ctx(p, bits/2, rand_ctx);
	p->digits[0] |= 1;
	while (mp_sieve(p->digits, p->size, 0) ||
		   mp_composite(p->digits, p->size, 10)) {
		mpi_add_ui(p, 2, p);
	}

	/* Generate Q. */
	mpi_init(q);
	mpi_rand_ctx(q, bits-bits/2, rand_ctx);
	q->digits[0] |= 1;
	while (mp_sieve(q->digits, q->size, 0) ||
		   mp_composite(q->digits, q->size, 10)) {
		mpi_add_ui(q, 2, q);
	}

	/* Set N = PQ */
	mpi_init(rsa->n);
	mpi_mul(p, q, rsa->n);

	/* Set Phi = (P-1)(Q-1) = PQ-P-Q+1 = N-P-Q+1 */
	mpi_init(rsa->phi);
	mpi_sub(rsa->n, p, rsa->phi);		/* phi = n-p   */
	mpi_sub(rsa->phi, q, rsa->phi);		/* phi = phi-q */
	mpi_inc(rsa->phi);					/* phi = phi+1 */

	/* Choose an integer E such that 1<E<Phi and E coprime to Phi */
	mpi_init(rsa->e);
	for (;;) {
		mpi_rand_ctx(rsa->e, bits, rand_ctx);
		if (mpi_cmp(rsa->e, rsa->phi))
			mpi_mod(rsa->e, rsa->phi, rsa->e);

		mpi_gcd(rsa->e, rsa->phi, p);
		if (mpi_is_one(p))
			break;
	}

	mpi_free_zero(p);
	mpi_free_zero(q);

	/* Compute D = multiplicative inverse of E, mod Phi */
	mpi_init(rsa->d);
	if (!mpi_modinv(rsa->phi, rsa->e, rsa->d)) {
		printf("Modular inverse of "), mpi_print_dec(rsa->e), printf(" mod "), mpi_print_dec(rsa->phi), printf(" does not exist.\n");
		abort();
	}
}

void
rsa_free(rsa_ctx *rsa)
{
	mpi_free_zero(rsa->n);
	mpi_free_zero(rsa->phi);
	mpi_free_zero(rsa->e);
	mpi_free_zero(rsa->d);
}
