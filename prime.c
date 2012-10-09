#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "weecrypt.h"

void factorize(mp_digit *n, mp_size len);

int
main(void)
{
    char buf[8192];
    while (printf("Enter an integer greater than 2 in base 10:\n") &&
	   fgets(buf, sizeof(buf), stdin)) {
	mp_size len;
	mp_digit *n = mp_from_str(buf, 10, &len);

	if (!n || !len || (len == 1 && n[0] <= 2)) {
	    printf("Invalid number.\n");
	    continue;
	}

	factorize(n, len);
	free(n);
    }

    return 0;
}

void
factorize(mp_digit *n, mp_size len)
{
    printf("Factors of ");
    mp_print_dec(n, len);
    printf(": ");
    int nfactors = 0;
    mp_digit factor;
    while ((factor = mp_sieve(n, len)) != 0) {
	int npowers = 0;
	do {
	    mp_digit remainder = mp_ddivi(n, len, factor);
	    ASSERT(remainder == 0);
	    len -= (n[len-1] == 0);
	    npowers++;
	} while (mp_dmod(n, len, factor) == 0);
	if (nfactors++)
	    printf(" * ");
	if (npowers > 1)
	    printf(MP_FORMAT "^%d", factor, npowers);
	else
	    printf(MP_FORMAT, factor);
    }

    if (nfactors == 0 || !(len == 1 && n[0] == 1)) {
	if (nfactors)
	    printf(" * ");
	mp_print_dec(n, len);
    }
    printf("\n");

    if (!(len == 1 && n[0] <= 2)) {
	/* Now run 10 rounds of the Rabin-Miller test. */
	printf("Remainder ");
	mp_print_dec(n, len);
	if (mp_composite(n, len, 10))
	    printf(" is definitely composite.\n");
	else
	    printf(" is prime with high probability.\n");
    }
}
