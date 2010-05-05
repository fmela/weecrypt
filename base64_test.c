#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "weecrypt.h"

int
main(void)
{
	const char text[] = "Hello World! Hello World! Hello World! Hello World! Hello World! Hello World! Hello World! Hello World! Hello World! Hello World! Hello World! Hello World! Hello World! Hello World! Hello World! Hello World! Hello World! Hello World! Hello World! Hello World! Hello World! Hello World! Hello World! Hello World!";
	unsigned i, base64_len;
	char *base64;

	for (i=1; i<=1024; i++) {
		base64 = base64_encode(text, i, &base64_len);

		printf("base64_len=%u strlen=%u\n", base64_len, strlen(base64));
		free(base64);
	}

	return 0;
}
