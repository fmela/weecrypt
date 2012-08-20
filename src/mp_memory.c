/* weecrypt_memory.c
 * Copyright (C) 2002-2012 Farooq Mela. All rights reserved. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "weecrypt_memory.h"

static void *(*_weecrypt_malloc)(size_t) = malloc;
static void *(*_weecrypt_realloc)(void *, size_t) = realloc;
static void  (*_weecrypt_free)(void *) = free;

weecrypt_malloc_func
weecrypt_set_malloc(weecrypt_malloc_func func)
{
	weecrypt_malloc_func ret = _weecrypt_malloc;
	_weecrypt_malloc = func ? func : malloc;
	return ret;
}

weecrypt_realloc_func
weecrypt_set_realloc(weecrypt_realloc_func func)
{
	weecrypt_realloc_func ret = _weecrypt_realloc;
	_weecrypt_realloc = func ? func : realloc;
	return ret;
}

weecrypt_free_func
weecrypt_set_free(weecrypt_free_func func)
{
	weecrypt_free_func ret = _weecrypt_free;
	_weecrypt_free = func ? func : free;
	return ret;
}

void *
weecrypt_xmalloc(size_t size)
{
	void *p;

	if ((p = (*_weecrypt_malloc)(size)) == NULL) {
		fprintf(stderr, "out of memory.\n");
		abort();
	}
	return p;
}

void *
weecrypt_xcalloc(size_t size)
{
	void *p;

	if ((p = (*_weecrypt_malloc)(size)) == NULL) {
		fprintf(stderr, "out of memory.\n");
		abort();
	}
	memset(p, 0, size);
	return p;
}

void *
weecrypt_xrealloc(void *ptr, size_t size)
{
	void *p;

	if ((p = (*_weecrypt_realloc)(ptr, size)) == NULL && size != 0) {
		fprintf(stderr, "out of memory.\n");
		abort();
	}
	return p;
}

void
weecrypt_xfree(void *ptr)
{
	(*_weecrypt_free)(ptr);
}
