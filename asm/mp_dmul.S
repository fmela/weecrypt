/*
 * mp_dmul.S
 * Copyright (C) 2001-2010 Farooq Mela. All rights reserved.
 *
 * mp_digit_t mp_dmul(const mp_digit_t *u, mp_len_t len,
 *                    mp_digit_t v, mp_digit_t *w);
 *
 * Parameters:
 * esp+4	u
 * esp+8	len
 * esp+12	v
 * esp+16	w
 *
 * $Id: mp_dmul.S,v 1.5 2002/01/02 08:56:51 farooq Exp $
 */

#include "mp_config.h"

#ifdef MP_DMUL_ASM

#define STACK	0x10
#define U_LOC	(STACK+0x04)(%esp)
#define L_LOC	(STACK+0x08)(%esp)
#define V_LOC	(STACK+0x0c)(%esp)
#define W_LOC	(STACK+0x10)(%esp)

.text
	.globl	mp_dmul
#ifndef __APPLE__
	.type	mp_dmul,@function
#endif
	.align	8
mp_dmul:
	pushl	%ebx
	pushl	%edi
	pushl	%esi
	pushl	%ebp

	xorl	%ebx,%ebx
	movl	L_LOC,%ecx
	jecxz	.L1
	movl	U_LOC,%esi
	movl	V_LOC,%ebp
	movl	W_LOC,%edi

	negl	%ecx
	sall	$2,%ecx
	subl	%ecx,%esi
	subl	%ecx,%edi
	sarl	$2,%ecx

	.align	8
.L0:
	movl	(%esi,%ecx,4),%eax
	mull	%ebp
	addl	%ebx,%eax
	movl	%eax,(%edi,%ecx,4)
	adcl	$0,%edx
	movl	%edx,%ebx
	incl	%ecx
	jnz		.L0

	.align	8
.L1:
	movl	%ebx,%eax

	popl	%ebp
	popl	%esi
	popl	%edi
	popl	%ebx
	ret

#endif /* MP_DMUL_ASM */
