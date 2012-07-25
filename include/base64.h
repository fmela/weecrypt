/* base64.h
 * Copyright (C) 2000-2010 Farooq Mela. All rights reserved.
 *
 * Routines for base64 encoding and decoding.
 *
 * $Id$
 */

#ifndef _BASE64_H_
#define _BASE64_H_

char *base64_encode(const void *input, unsigned input_size);
char *base64_encode_string(const char *input);
void *base64_decode(const char *input, unsigned *output_size);

#endif // !_BASE64_H_
