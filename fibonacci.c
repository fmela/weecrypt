#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "weecrypt.h"

int
main(void)
{
	char buf[512];
	mpi_t fib;
	mpi_init(fib);

	for (;;) {
		printf("Enter N: ");
		if (!fgets(buf, sizeof(buf), stdin))
			break;
		unsigned n = (unsigned)strtoul(buf, NULL, 10);
		if (n == 0)
			break;

		printf("You entered %u\n", n);
		mpi_fibonacci(n, fib);
		printf("F(%u)=", n), mpi_print_dec(fib), printf("\n");
	}

	return 0;
}
