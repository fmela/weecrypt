/*
 * rsa.c
 * 10-20-05
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "weecrypt.h"

bool rsa_test(unsigned bits, mt64_context *rand_ctx);

int
main(void)
{
    const unsigned rsa_bits_log2 = 3;
    const unsigned ntrials = 10;
    mt64_context rand_ctx;

    mt64_init_u64(&rand_ctx, 1234567890U);

    unsigned i;
    for (i=0; i<ntrials; i++) {
	printf("Testing %u-bit RSA...\n", 1U<<(rsa_bits_log2+i));
	if (!rsa_test(1U<<(rsa_bits_log2+i), &rand_ctx))
	    break;
    }

    if (i != ntrials)
	printf("Trial %u (%u bytes) failed.\n",
	       i, 1U<<(rsa_bits_log2+i));
    else
	printf("Trials with %u-bit thru %u-bit RSA succeeded.\n",
	       1U<<rsa_bits_log2, 1U<<(rsa_bits_log2+ntrials-1));

    return 0;
}

bool
rsa_test(unsigned bits, mt64_context *rand_ctx)
{
    mt64_init_u64(rand_ctx, 1234567890U);

    rsa_ctx rsa;
    if (!rsa_init_keygen(&rsa, bits, rand_ctx))
	return false;
    ASSERT(rsa.n != NULL);
    ASSERT(rsa.phi != NULL);
    ASSERT(rsa.e != NULL);
    ASSERT(rsa.d != NULL);

    mpi_t m = MPI_INITIALIZER;
    mpi_mul(rsa.e, rsa.d, m);
    mpi_mod(m, rsa.phi, m);
    bool result = true;
    if (!mpi_is_one(m)) {
	printf("N: "), mpi_print_dec(rsa.n), printf("\n");
	printf("E: "), mpi_print_dec(rsa.e), printf("\n");
	printf("D: "), mpi_print_dec(rsa.d), printf("\n");
	printf("Φ: "), mpi_print_dec(rsa.phi), printf("\n");
	printf("E * D mod Φ: "), mpi_print_dec(m), printf("\n");
	result = false;
    }
    mpi_free(m);
    rsa_free(&rsa);
    return result;
}
