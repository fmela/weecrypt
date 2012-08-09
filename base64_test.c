#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "weecrypt.h"

int
main(void)
{
	unsigned char text[256] = { 0 };
	for (unsigned i = 0; i < 256; ++i)
		text[i] = i;
	for (unsigned i = 1; i <= sizeof(text); ++i) {
		char *encode = base64_encode(text, i);
		assert(encode != NULL);

		unsigned decode_len = 0;
		char *decode = base64_decode(encode, &decode_len);
		assert(decode != NULL);

		int bad = 0;
		if (decode_len != i) {
			printf("%u: decode_len=%u i=%u\n", i, decode_len, i);
			bad = 1;
		}
		if (memcmp(text, decode, i) != 0) {
			printf("%u: text and decode differ!", i);
			bad = 1;
		}
		if (bad) {
			printf("Original:");
			for (unsigned j = 0; j < i; ++j) {
				if (j % 8 == 0) {
					printf("\n");
				}
				printf("  0x%02X", text[j]);
			}
			printf("\n");
			printf("Encoded: %s\n", encode);
			printf("Decoded:");
			for (unsigned j = 0; j < decode_len; ++j) {
				if (j % 8 == 0) {
					printf("\n");
				}
				printf("  0x%02X", ((unsigned char *)decode)[j]);
			}
			printf("\n");
		}

		free(encode);
		free(decode);
	}

	return 0;
}
