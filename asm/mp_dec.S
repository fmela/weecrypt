/*
 * mp_dec.S
 * Copyright (C) 2001-2010 Farooq Mela. All rights reserved.
 *
 * mp_digit_t mp_dec(mp_digit_t *u, mp_len_t len);
 *
 * Parameters:
 * esp+4	u
 * esp+8	len
 *
 * $Id: mp_dec.S,v 1.5 2002/01/02 08:56:51 farooq Exp $
 */

#include "mp_config.h"

#ifdef MP_DEC_ASM

#define STACK	0x00
#define U_LOC	(STACK+0x04)(%esp)
#define L_LOC	(STACK+0x08)(%esp)

.text
	.globl	MP_ASM_NAME(mp_dec)
#ifndef __APPLE__
	.type	MP_ASM_NAME(mp_dec),@function
#endif
	.p2align 3
MP_ASM_NAME(mp_dec):
	movl	U_LOC,%edx
	movl	L_LOC,%ecx

	movl	$1,%eax
	jecxz	.L1

	.p2align 3
.L0:
	subl	$1,(%edx)
	setc	%al
	jnc	.L1
	addl	$4,%edx
	decl	%ecx
	jnz		.L0

	.p2align 3
.L1:
	ret

#endif /* MP_DEC_ASM */
