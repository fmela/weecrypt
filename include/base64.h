/* base64.h
 * Copyright (C) 2000-2010 Farooq Mela. All rights reserved.
 *
 * Routines for base64 encoding and decoding.
 *
 * $Id$
 */

#ifndef _BASE64_H_
#define _BASE64_H_

void *base64_encode(const void *input, unsigned input_size, unsigned *output_size);
void *base64_decode(const void *input, unsigned input_size, unsigned *output_size);

#endif // !_BASE64_H_
