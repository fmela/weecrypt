/*
 * mp_flip.S
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * void mp_flip(mp_digit_t *u, mp_len_t len);
 *
 * Parameters:
 * esp+4	u
 * esp+8	len
 *
 * $Id$
 */

#include "mp_config.h"

#define STACK	0x00
#define U_LOC	(STACK+0x04)(%esp)
#define L_LOC	(STACK+0x08)(%esp)

#ifdef MP_FLIP_ASM
.text
	.globl	MP_ASM_NAME(mp_flip)
#ifndef __APPLE__
	.type	MP_ASM_NAME(mp_flip),@function
#endif
	.p2align	3
MP_ASM_NAME(mp_flip):
	movl	L_LOC,%ecx				# ecx = length
	jecxz	.L1						# done if zero
	movl	U_LOC,%edx				# edx = u ptr

	negl	%ecx					# ecx = -ecx
	sall	$2,%ecx					# ecx *= 4
	subl	%ecx,%edx				# edx -= ecx
	sarl	$2,%ecx					# ecx /= 4

	.p2align	3
.L0:
	notl	(%edx,%ecx,4)			# flip digit
	incl	%ecx
	jnz		.L0

.L1:
	ret

#endif /* MP_FLIP_ASM */
