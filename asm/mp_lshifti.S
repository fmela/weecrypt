/*
 * mp_lshifti.S
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * mp_digit_t mp_lshifti(mp_digit_t *u, mp_len_t len, unsigned s);
 *
 * Parameters:
 * esp+4	u
 * esp+8	len
 * esp+12	s
 *
 * $Id$
 */

 #include "mp_config.h"

 #ifdef MP_LSHIFTI_ASM

 #define STACK	0x08
 #define U_LOC	(STACK+0x04)(%esp)
 #define L_LOC	(STACK+0x08)(%esp)
 #define S_LOC	(STACK+0x0c)(%esp)

 .text
 	.globl	MP_ASM_NAME(mp_lshifti)
#ifndef __APPLE__
	.type	MP_ASM_NAME(mp_lshifti),@function
#endif
	.p2align 3
MP_ASM_NAME(mp_lshifti):
	pushl	%ebx
	pushl	%esi

	xorl	%eax,%eax

	movl	L_LOC,%ebx
	orl	%ebx,%ebx
	jz	.L1
	movl	U_LOC,%esi
	movl	S_LOC,%ecx
	andb	$31,%cl
	jz	.L1

	.p2align 3
.L0:
	movl	(%esi),%edx
	shldl	%cl,%eax,(%esi)
	movl	%edx,%eax
	addl	$4,%esi
	decl	%ebx
	jnz	.L0

	negb	%cl
	addb	$32,%cl
	shrl	%cl,%eax

	.p2align 3
.L1:
	popl	%esi
	popl	%ebx
	ret

#endif /* MP_LSHIFTI_ASM */
