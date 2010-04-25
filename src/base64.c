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

void *
base64_encode(const void *input, unsigned input_size, unsigned *output_size)
{
	const unsigned char *in;
	unsigned char *data, *code;
	unsigned pos, outbuf_size, pad, line = 0;

	ASSERT(input != NULL);
	ASSERT(input_size != 0);
	ASSERT(output_size != NULL);

	in = (const unsigned char *)input;
	pad = input_size % 3;
	outbuf_size = (input_size * 4 + 2)/3;
	outbuf_size += (outbuf_size / 36) + 2;
	if (pad)
		outbuf_size++;
	outbuf_size += 2;
	data = MALLOC(outbuf_size);

	for (code = data, pos = 0; pos + 3 <= input_size; in += 3, pos += 3) {
		*code++ = base64_chars[((in[0] >> 2) & 0x3f)];
		*code++ = base64_chars[((in[0] << 4) & 0x30) | ((in[1] >> 4) & 0xf)];
		*code++ = base64_chars[((in[1] << 2) & 0x3c) | ((in[2] >> 6) & 0x3)];
		*code++ = base64_chars[((in[2] >> 0) & 0x3f)];

		if ((line += 4) >= 72) {
			*code++ = '\r';
			*code++ = '\n';
			line = 0;
		}
	}

	if (pad) {
		*code++ = base64_chars[((in[0] >> 2) & 0x3f)];
		if (pad > 1) {
			*code++ = base64_chars[((in[0] << 4) & 0x30)|((in[1] >> 4) & 0xf)];
			*code++ = base64_chars[((in[1] << 2) & 0x3c)];
		} else {
			*code++ = base64_chars[(in[0] << 4) & 0x30];
			*code++ = BASE64_PAD;
		}
		*code++ = BASE64_PAD;
	}

	*code++ = '\r';
	*code++ = '\n';
	*code++ = '\0';
	*output_size = code - data;

	if (*output_size > outbuf_size)
		printf("%u: Allocated %u, wrote %u\n", input_size, outbuf_size, *output_size);

	return data;
}

void *
base64_decode(const void *input, unsigned input_size, unsigned *output_size)
{
	const unsigned char *p, *in;
	unsigned char *output, *code;
	unsigned int data[4];
	int i;
	unsigned pos = 0;

	ASSERT(input != NULL);
	ASSERT(input_size != 0);
	ASSERT(output_size != NULL);

	code = output = MALLOC(((input_size * 3) / 4) + 2);
	in = (const unsigned char *)input;

	while (pos + 3 < input_size) {
		for (i = 0; i < 4; i++) {
			for (p = base64_chars; *p; p++)
				if (*p == in[pos])
					break;
			pos++;
			if (*p)
				data[i] = p - base64_chars;
			else
				i--;
		}

		code[0] = ((data[0] << 2) & 0xfc) | ((data[1] >> 4) & 0x03);
		code[1] = ((data[1] << 4) & 0xf0) | ((data[2] >> 2) & 0x0f);
		code[2] = ((data[2] << 6) & 0xc0) | ((data[3] >> 0) & 0x3f);

		if (data[3] == 64) {
			if (data[2] == 64) {
				code[1] = 0;
				code -= 2;
			} else {
				code[2] = 0;
				code--;
			}
		}
		code += 3;
	}

	*output_size = code - output;

	return output;
}
