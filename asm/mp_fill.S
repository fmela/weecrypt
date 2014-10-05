/*
 * mp_fill.S
 * Copyright (C) 2001-2010 Farooq Mela. All rights reserved.
 *
 * mp_digit_t *mp_fill(mp_digit_t *u, mp_len_t len, mp_digit_t v);
 *
 * Parameters:
 * esp+4	u
 * esp+8	len
 * esp+12	v
 *
 * $Id: mp_fill.S,v 1.6 2002/05/22 19:17:28 farooq Exp $
 */

#include "mp_config.h"

#ifdef MP_FILL_ASM

#define STACK	0x04
#define U_LOC	(STACK+0x04)(%esp)
#define L_LOC	(STACK+0x08)(%esp)
#define V_LOC	(STACK+0x0c)(%esp)

.text
	.globl	MP_ASM_NAME(mp_fill)
#ifndef __APPLE__
	.type	MP_ASM_NAME(mp_fill),@function
#endif
	.p2align	3
MP_ASM_NAME(mp_fill):
	pushl	%edi

	movl	U_LOC,%edi
	movl	L_LOC,%ecx
	movl	V_LOC,%eax
	cld
	rep	stosl

	movl	U_LOC,%eax

	popl	%edi
	ret

#endif /* MP_FILL_ASM */
