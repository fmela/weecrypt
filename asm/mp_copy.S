/*
 * mp_copy.S
 * Copyright (C) 2001-2010 Farooq Mela. All rights reserved.
 *
 * void mp_copy(const mp_digit_t *u, mp_len_t len, mp_digit_t *v);
 *
 * Parameters:
 * esp+4	u
 * esp+8	len
 * esp+12	v
 *
 * $Id: mp_copy.S,v 1.4 2002/07/31 09:38:04 farooq Exp $
 */

#include "mp_config.h"

#ifdef MP_COPY_ASM

#define STACK	0x08
#define U_LOC	(STACK+0x04)(%esp)
#define L_LOC	(STACK+0x08)(%esp)
#define V_LOC	(STACK+0x0c)(%esp)

.text
	.globl	mp_copy
#ifndef __APPLE__
	.type	mp_copy,@function
#endif
	.align	8
mp_copy:
	pushl	%esi
	pushl	%edi

	movl	L_LOC,%ecx
	jecxz	.Lexit
	movl	U_LOC,%esi
	movl	V_LOC,%edi
	cmpl	%esi,%edi
	je		.Lexit
	cld
	rep		movsl

.Lexit:
	popl	%edi
	popl	%esi
	ret

#endif /* MP_COPY_ASM */
