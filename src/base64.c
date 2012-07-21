/* base64.c
 * Copyright (C) 2000-2010 Farooq Mela. All rights reserved.
 *
 * Routines for base64 encoding and decoding.
 *
 * Base64 takes binary (8-bit) input and converts it into 7-bit data that can
 * be understood by all SMTP servers. It takes 3 8-bit numbers and converts
 * them into 4 6-bit numbers, and uses the 6-bit numbers into indices for the
 * base64 charset, which is A-Z, a-z, 0-9, and finally `+' and `/'.
 *
 * If the number of bytes of data being encoded is not congruential to 3, then
 * padding must be added. If there is one extra byte, it is encoded and
 * appended, then two padding characters are added. If there are two extra
 * bytes, they are encoded and appended, followed by single padding character.
 * The padding character is `='.
 *
 * $Id$
 */

#include "base64.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mp.h"
#include "mp_defs.h"
#include "weecrypt_memory.h"

static const unsigned char base64_chars[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";

#define BASE64_PAD		'='

static unsigned char decode(unsigned char code) {
	if (code >= 'A' && code <= 'Z') {
		return code - 'A';
	} else if (code >= 'a' && code <= 'z') {
		return code - 'a' + 26;
	} else if (code >= '0' && code <= '9') {
		return code - '0' + 52;
	} else if (code == '+') {
		return 62;
	} else if (code == '/') {
		return 63;
	} else {
		return 0xFF;
	}
}

char *
base64_encode(const void *input, unsigned input_size, unsigned *output_size)
{
	if (!input || !input_size) {
		return "";
	}
	const unsigned char *in = (const unsigned char *)input;
	const unsigned pad = input_size % 3;
	unsigned outbuf_size = 4 * ((input_size + 2) / 3);
	/* Add two bytes for every 72 characters, plus one for the trailing '\0'. */
	outbuf_size += 2 * ((outbuf_size - 1) / 72) + 1;
	unsigned char *outbuf = MALLOC(outbuf_size);

	unsigned char *code = outbuf;
	unsigned line = 0;
	for (unsigned pos = 0; pos + 3 <= input_size; in += 3, pos += 3) {
		if (line >= 72) {
			*code++ = '\r';
			*code++ = '\n';
			line = 0;
		}
		*code++ = base64_chars[((in[0] >> 2) & 0x3f)];
		*code++ = base64_chars[((in[0] << 4) & 0x30) | ((in[1] >> 4) & 0xf)];
		*code++ = base64_chars[((in[1] << 2) & 0x3c) | ((in[2] >> 6) & 0x3)];
		*code++ = base64_chars[((in[2] >> 0) & 0x3f)];
		line += 4;
	}

	if (pad) {
		if (line >= 72) {
			*code++ = '\r';
			*code++ = '\n';
			line = 0;
		}
		*code++ = base64_chars[((in[0] >> 2) & 0x3f)];
		if (pad == 2) {
			*code++ = base64_chars[((in[0] << 4) & 0x30) |
				                   ((in[1] >> 4) & 0xf)];
			*code++ = base64_chars[((in[1] << 2) & 0x3c)];
		} else {
			ASSERT(pad == 1);
			*code++ = base64_chars[(in[0] << 4) & 0x30];
			*code++ = BASE64_PAD;
		}
		*code++ = BASE64_PAD;
	}

	if (output_size != NULL)
		*output_size = code - outbuf;
	*code++ = '\0';

	if (outbuf_size != code - outbuf) {
		printf("encoding %u bytes: allocated %u, wrote %ld\n",
			   input_size, outbuf_size, code - outbuf);
		printf("encoded: \"%s\"\n", outbuf);
	}

	return (char *)outbuf;
}

void *
base64_decode(const char *input, unsigned *output_size)
{
	ASSERT(input != NULL);

	unsigned input_size = 0;
	unsigned base64_bytes = 0;
	for (; input[input_size]; ++input_size) {
		if (decode(input[input_size]) < 64) {
			++base64_bytes;
		}
	}
	if (input_size == 0) {	/* Special handling of empty string. */
		if (output_size) {
			*output_size = 0;
		}
		return MALLOC(1);
	}
	unsigned outbuf_size = (base64_bytes * 3) / 4;
	unsigned char *outbuf = MALLOC(outbuf_size);
	const unsigned char *in = (const unsigned char *)input;

	unsigned char *code = outbuf;
	unsigned char data[4];
	unsigned int ndata = 0;
	for (unsigned pos = 0; pos < input_size; ++pos) {
		unsigned char decode_char = decode(in[pos]);
		if (decode_char < 64) {
			data[ndata] = decode_char;
			if (++ndata == 4) {
				*code++ = ((data[0] << 2) & 0xfc) | ((data[1] >> 4) & 0x03);
				*code++ = ((data[1] << 4) & 0xf0) | ((data[2] >> 2) & 0x0f);
				*code++ = ((data[2] << 6) & 0xc0) | ((data[3] >> 0) & 0x3f);
				ndata = 0;
			}
		}
	}
	if (ndata >= 2) {
		*code++ = ((data[0] << 2) & 0xfc) | ((data[1] >> 4) & 0x03);
		if (ndata >= 3) {
			*code++ = ((data[1] << 4) & 0xf0) | ((data[2] >> 2) & 0x0f);
		}
	} else if (ndata != 0) {
		printf("%d(s) stray input bytes.\n", ndata);
	}

	if (output_size != NULL)
		*output_size = code - outbuf;

	if (outbuf_size != code - outbuf) {
		printf("decoding %u bytes: allocated %u, wrote %ld\n",
			   input_size, outbuf_size, code - outbuf);
	}

	return outbuf;
}
