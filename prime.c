#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "weecrypt.h"

void factor(mp_digit *n, mp_size len);

int
main(void)
{
	mp_digit *n;
	mp_size len;
	char buf[8192];

	for (;;) {
		printf("Enter an integer greater than 2 in base 10:\n");
		if (!fgets(buf, sizeof(buf), stdin))
			break;

		n = mp_from_str(buf, 10, &len);

		if (n == 0 || len == 0 || (len == 1 && n[0]<=2)) {
			printf("Invalid number\n");
			exit(1);
		}

		printf("The number you entered was:\n");
		mp_print_dec(n, len);
		printf("\n");

		factor(n, len);
	}

	return 0;
}

void factor(mp_digit *n, mp_size len) {
	int nfactors = 0;
	mp_digit factor;
	while ((factor = mp_sieve(n, len, 0)) != 0) {
		int npowers = 0;

		nfactors++;
		do {
			mp_digit remainder = mp_ddivi(n, len, factor);
			ASSERT(remainder == 0);
			len -= (n[len-1] == 0);
			npowers++;
		} while (mp_dmod(n, len, factor) == 0);
		if (npowers > 1)
			printf("Trivial factor %u^%d removed\n", factor, npowers);
		else
			printf("Trivial factor %u removed\n", factor);
	}

	printf("Remainder after removing %d trivial factors is:\n", nfactors);
	mp_print_dec(n, len); printf("\n");

	if (len == 1 && n[0] <= 2) {
		printf("Trivial remainder\n");
	} else {
		/* Now run 10 rounds of the Rabin-Miller test. */
		if (mp_composite(n, len, 10))
			printf("Remainder is definitely composite.\n");
		else
			printf("Remainder is prime with extreme probability.\n");
	}
}
