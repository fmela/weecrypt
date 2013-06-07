/* weecrypt_memory.h
 * Copyright (C) 2010-2012 Farooq Mela. All rights reserved. */

#ifndef _WEECRYPT_MEMORY_H_
#define _WEECRYPT_MEMORY_H_

#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*weecrypt_malloc_func)(size_t);
typedef void *(*weecrypt_realloc_func)(void *, size_t);
typedef void  (*weecrypt_free_func)(void *);

weecrypt_malloc_func	weecrypt_set_malloc(weecrypt_malloc_func func);
weecrypt_realloc_func	weecrypt_set_realloc(weecrypt_realloc_func func);
weecrypt_free_func	weecrypt_set_free(weecrypt_free_func func);

void *weecrypt_xmalloc(size_t size);
void *weecrypt_xcalloc(size_t size);
void *weecrypt_xrealloc(void *ptr, size_t size);
void  weecrypt_xfree(void *ptr);

#ifdef __cplusplus
}
#endif

#define MALLOC(n)	weecrypt_xmalloc(n)
#define CALLOC(n)	weecrypt_xcalloc(n)
#define REALLOC(p,n)	weecrypt_xrealloc(p,n)
#define FREE(p)		weecrypt_xfree(p)

#endif /* !_WEECRYPT_MEMORY_H_ */
