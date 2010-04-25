/*
 * mp_rlen.S
 * Copyright (C) 2001-2010 Farooq Mela. All rights reserved.
 *
 * mp_len_t mp_rlen(const mp_digit_t *u, mp_len_t len);
 *
 * Parameters:
 * esp+4	u
 * esp+8	len
 *
 * $Id: mp_rlen.S,v 1.4 2002/06/27 23:45:29 farooq Exp $
 */

#include "mp_config.h"

#ifdef MP_RLEN_ASM

#define STACK	0x00
#define U_LOC	(STACK+0x04)(%esp)
#define L_LOC	(STACK+0x08)(%esp)

.text
	.globl	mp_rlen
#ifndef __APPLE__
	.type	mp_rlen,@function
#endif
	.align	8
mp_rlen:
	movl	U_LOC,%edx
	movl	L_LOC,%ecx

	jecxz	.L2

	shll	$2,%ecx
	addl	%ecx,%edx
	shrl	$2,%ecx
	jmp		.L1

	.align	8
.L0:
	decl	%ecx
	jz		.L2
	.align	8
.L1:
	subl	$4,%edx
	cmpl	$0,(%edx)
	je		.L0

	.align	8
.L2:
	movl	%ecx,%eax
	ret

#endif /* MP_RLEN_ASM */
