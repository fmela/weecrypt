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

	while (printf("Enter N: ") &&
		   fgets(buf, sizeof(buf), stdin)) {
		unsigned n = (unsigned)strtoul(buf, NULL, 10);
		if (!n)
			break;

		mpi_fibonacci(n, fib);
		printf("F(%u)=", n), mpi_print_dec(fib), printf("\n");
		printf("As double: %g\n", mpi_get_d(fib));
	}

	mpi_free(fib);

	return 0;
}
