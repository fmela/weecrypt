/*
 * rsa.c
 * 10-20-05
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "weecrypt.h"

bool rsa_test(unsigned bits, mp_rand_ctx *rand_ctx);

int
main(void)
{
	const unsigned rsa_bits = 8;
	const unsigned ntrials = 5;
	unsigned i;
	mp_rand_ctx rand_ctx;

	mp_rand_ctx_init_seed(&rand_ctx, 1234567890U);

	for (i=0; i<ntrials; i++) {
		printf("Testing %u-bit RSA...\n", 1U<<(rsa_bits+i));
		if (!rsa_test(1U<<(rsa_bits+i), &rand_ctx))
			break;
	}

	if (i != ntrials)
		printf("Trial %u (%u bytes) failed.\n", i, 1U<<(rsa_bits+i));
	else
		printf("Trials with %u-bit thru %u-bit RSA succeeded.\n", 1U<<rsa_bits, 1U<<(rsa_bits+ntrials-1));

	return 0;
}

bool
rsa_test(unsigned bits, mp_rand_ctx *rand_ctx)
{
	rsa_ctx rsa[1];

	mp_rand_ctx_init_seed(rand_ctx, 1234567890U);

	rsa_init(rsa, bits, rand_ctx);

	/* tmp = (rsa->e * rsa->e) % rsa->phi */
	mpi_t tmp;
	mpi_init(tmp);
	mpi_mul(rsa->e, rsa->d, tmp);
	mpi_mod(tmp, rsa->phi, tmp);
	bool ok = mpi_is_one(tmp);
//	if (!ok) {
		printf("  N="), mpi_print_dec(rsa->n),
			printf(" (%u bits)\n", mpi_significant_bits(rsa->n));
		printf("Phi="), mpi_print_dec(rsa->phi),
			printf(" (%u bits)\n", mpi_significant_bits(rsa->phi));
		printf("  E="), mpi_print_dec(rsa->e),
			printf(" (%u bits)\n", mpi_significant_bits(rsa->e));
		printf("  D="), mpi_print_dec(rsa->d),
			printf(" (%u bits)\n", mpi_significant_bits(rsa->d));
		printf("ED mod Phi="), mpi_print_dec(tmp), printf("\n");
//	}

	mpi_free(tmp);

	rsa_free(rsa);
	return ok;
}
