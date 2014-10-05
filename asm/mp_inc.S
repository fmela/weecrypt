/*
 * mp_inc.S
 * Copyright (C) 2001-2010 Farooq Mela. All rights reserved.
 *
 * mp_digit_t mp_inc(mp_digit_t *u, mp_len_t len);
 *
 * Parameters:
 * esp+4	u
 * esp+8	len
 *
 * $Id: mp_inc.S,v 1.5 2002/07/07 18:37:27 farooq Exp $
 */

#include "mp_config.h"

#ifdef MP_INC_ASM

#define STACK	0x00
#define U_LOC	(STACK+0x04)(%esp)
#define L_LOC	(STACK+0x08)(%esp)

.text
	.globl	MP_ASM_NAME(mp_inc)
#ifndef __APPLE__
	.type	MP_ASM_NAME(mp_inc),@function
#endif
	.p2align 3
MP_ASM_NAME(mp_inc):
	movl	U_LOC,%edx
	movl	L_LOC,%ecx

	movl	$1,%eax
	jecxz	.L1

	shll	$2,%ecx
	addl	%ecx,%edx
	shrl	$2,%ecx
	negl	%ecx

	.p2align 3
.L0:
	addl	$1,(%edx,%ecx,4)
	setc	%al
	jnc	.L1
	incl	%ecx
	jnz	.L0

.L1:
	ret

#endif /* MP_INC_ASM */
