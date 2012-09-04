/*
 * mp_dmuli.S
 * Copyright (C) 2001-2010 Farooq Mela. All rights reserved.
 *
 * mp_digit_t mp_dmuli(mp_digit_t *u, mp_len_t len, mp_digit_t v);
 *
 * Parameters:
 * esp+4	u
 * esp+8	len
 * esp+12	v
 *
 * $Id: mp_dmuli.S,v 1.6 2002/06/29 03:51:26 farooq Exp $
 */

#include "mp_config.h"

#ifdef MP_DMULI_ASM

#define STACK	0x0c
#define U_LOC	(STACK+0x04)(%esp)
#define L_LOC	(STACK+0x08)(%esp)
#define V_LOC	(STACK+0x0c)(%esp)

.text
	.globl	MP_ASM_NAME(mp_dmuli)
#ifndef __APPLE__
	.type	MP_ASM_NAME(mp_dmuli),@function
#endif
	.p2align	3
MP_ASM_NAME(mp_dmuli):
	pushl	%ebx
	pushl	%edi
	pushl	%esi

	movl	U_LOC,%esi
	movl	L_LOC,%ecx
	movl	V_LOC,%ebx
	xorl	%edi,%edi
	xorl	%eax,%eax

	jecxz	.L2

	jmp		.L1
	movl	%ebx,%eax
	decl	%eax
	andl	%ebx,%eax
	jnz		.L1

	movl	%ecx,%edi		/* edi = len so we can shift with cl			*/
	bsfl	%ebx,%ecx		/* cl = lg(v)									*/
	xorl	%eax,%eax		/* clear previous bits							*/

	.p2align	3
.L0:
	movl	(%esi),%edx		/* edx = u-digit								*/
	shldl	%cl,%eax,(%esi)	/* shift u-digit and combine with previous		*/
	movl	%edx,%eax		/* eax = u-digit before shift					*/
	addl	$4,%esi			/* move u-pointer along							*/
	decl	%edi			/* decrement length								*/
	jnz		.L0				/* loop											*/
	shrl	%cl,%eax		/* shift last digit right						*/
	jmp		.L2

	.p2align	3
.L1:
	movl	(%esi),%eax
	mull	%ebx
	addl	%edi,%eax
	movl	%eax,(%esi)
	adcl	$0,%edx
	movl	%edx,%edi

	addl	$4,%esi
	decl	%ecx
	jnz		.L1
	movl	%edi,%eax

	.p2align	3
.L2:
	popl	%esi
	popl	%edi
	popl	%ebx
	ret

#endif /* MP_DMULI_ASM */
