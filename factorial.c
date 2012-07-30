#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "weecrypt.h"

void print_factorial(unsigned n);

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
		print_factorial(n);
	}

	return 0;
}

void print_factorial(unsigned n)
{
	mpi_t f;
	mpi_init_u32(f, n);
	printf("%u! = ", n);
	fflush(stdout);
	for (unsigned m = 2; m < n; ++m) {
		mpi_mul_u32(f, m, f);
	}
	mpi_print_dec(f), printf("\n");
	printf("As double: %g\n", mpi_get_d(f));
	mpi_free(f);
}
