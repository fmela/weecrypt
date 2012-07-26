/* base64.c
 * Copyright (C) 2000-2010 Farooq Mela. All rights reserved.
 *
 * Routines for base64 encoding and decoding.
 *
 * Base64 takes binary (8-bit) input and converts it into 7-bit data that can
 * be understood by all SMTP servers. It takes 3 8-bit numbers and converts
 * them into 4 6-bit numbers, and uses the 6-bit numbers into indices for the
 * base64 charset, which is A-Z, a-z, 0-9, and finally '+' and '/'.
 *
 * If the number of bytes of data being encoded is not congruential to 3, then
 * padding must be added. If there is one extra byte, it is encoded and
 * appended, then two padding characters are added. If there are two extra
 * bytes, they are encoded and appended, followed by single padding character.
 * The padding character is '='.
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

static const unsigned char base64_table[] = {
	[0 ... '+'-1] = 0xFF,
	['+'] = 0x3E,
	['+'+1 ... '/'-1] = 0xFF,
	['/'] = 0x3F,
	['0'] = 0x34,
	['1'] = 0x35,
	['2'] = 0x36,
	['3'] = 0x37,
	['4'] = 0x38,
	['5'] = 0x39,
	['6'] = 0x3A,
	['7'] = 0x3B,
	['8'] = 0x3C,
	['9'] = 0x3D,
	['9'+1 ... 'A'-1] = 0xFF,
	['A'] = 0x00,
	['B'] = 0x01,
	['C'] = 0x02,
	['D'] = 0x03,
	['E'] = 0x04,
	['F'] = 0x05,
	['G'] = 0x06,
	['H'] = 0x07,
	['I'] = 0x08,
	['J'] = 0x09,
	['K'] = 0x0A,
	['L'] = 0x0B,
	['M'] = 0x0C,
	['N'] = 0x0D,
	['O'] = 0x0E,
	['P'] = 0x0F,
	['Q'] = 0x10,
	['R'] = 0x11,
	['S'] = 0x12,
	['T'] = 0x13,
	['U'] = 0x14,
	['V'] = 0x15,
	['W'] = 0x16,
	['X'] = 0x17,
	['Y'] = 0x18,
	['Z'] = 0x19,
	['Z'+1 ... 'a'-1] = 0xFF,
	['a'] = 0x1A,
	['b'] = 0x1B,
	['c'] = 0x1C,
	['d'] = 0x1D,
	['e'] = 0x1E,
	['f'] = 0x1F,
	['g'] = 0x20,
	['h'] = 0x21,
	['i'] = 0x22,
	['j'] = 0x23,
	['k'] = 0x24,
	['l'] = 0x25,
	['m'] = 0x26,
	['n'] = 0x27,
	['o'] = 0x28,
	['p'] = 0x29,
	['q'] = 0x2A,
	['r'] = 0x2B,
	['s'] = 0x2C,
	['t'] = 0x2D,
	['u'] = 0x2E,
	['v'] = 0x2F,
	['w'] = 0x30,
	['x'] = 0x31,
	['y'] = 0x32,
	['z'] = 0x33,
	['z'+1 ... 0xFF] = 0xFF,
};

char *
base64_encode(const void *input, unsigned input_size)
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
	*code++ = '\0';

	ASSERT(outbuf_size == (unsigned)(code - outbuf));
	return (char *)outbuf;
}

char *
base64_encode_string(const char *input)
{
	/* TODO: also encode trailing zero byte? */
	return base64_encode(input, strlen(input));
}

void *
base64_decode(const char *input, unsigned *output_size)
{
	ASSERT(input != NULL);

	unsigned base64_bytes = 0;
	for (unsigned pos = 0; input[pos]; ++pos) {
		if (base64_table[(unsigned char)input[pos]] < 64) {
			++base64_bytes;
		}
	}
	if (base64_bytes == 0) {	/* Special handling of empty string. */
		if (output_size) {
			*output_size = 0;
		}
		return MALLOC(1);
	}
	unsigned outbuf_size = (base64_bytes * 3) / 4;
	unsigned char *outbuf = MALLOC(outbuf_size);

	unsigned char *code = outbuf;
	unsigned char data[4];
	unsigned int ndata = 0;
	for (unsigned pos = 0; input[pos]; ++pos) {
		unsigned char decoded_byte = base64_table[(unsigned char)input[pos]];
		if (decoded_byte < 64) {
			data[ndata] = decoded_byte;
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
		printf("%d stray input byte(s).\n", ndata);
	}

	if (output_size != NULL)
		*output_size = code - outbuf;
	ASSERT(outbuf_size == (unsigned)(code - outbuf));
	return outbuf;
}
