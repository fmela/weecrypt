/*
 * mp_rshifti.S
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * mp_digit_t mp_rshifti(mp_digit_t *u, mp_len_t len, unsigned s);
 *
 * Parameters:
 * esp+4	u
 * esp+8	len
 * esp+12	v
 *
 * $Id$
 */

 #include "mp_config.h"

 #ifdef MP_RSHIFTI_ASM

 #define STACK	0x08
 #define U_LOC	(STACK+0x04)(%esp)
 #define L_LOC	(STACK+0x08)(%esp)
 #define V_LOC	(STACK+0x0c)(%esp)

 .text
 	.globl	mp_rshifti
#ifndef __APPLE__
	.type	mp_rshifti,@function
#endif
	.align	8
mp_rshifti:
	pushl	%ebx
	pushl	%esi

	xorl	%eax,%eax					/* Clear return value */

	movl	L_LOC,%ebx					/* load length */
	orl		%ebx,%ebx					/* if zero .. */
	jz		.L1							/* .. jump to exit */
	movl	U_LOC,%esi					/* load u pointer */
	movl	V_LOC,%ecx					/* load shift .. */
	andb	$31,%cl						/* .. modulo 32 */
	jz		.L1							/* jump to exit if zero */

	.align	8
.L0:
	decl	%ebx						/* decrement length */
	movl	(%esi,%ebx,4),%edx			/* edx = digit */
	shrdl	%cl,%eax,(%esi,%ebx,4)		/* shift right with eax as prev bits */
	movl	%edx,%eax					/* eax = old digit */
	orl		%ebx,%ebx
	jnz		.L0

	movl	$1,%edx
	shll	%cl,%edx
	decl	%edx
	andl	%edx,%eax

	.align	8
.L1:
	popl	%esi
	popl	%ebx
	ret

#endif /* MP_RSHIFTI_ASM */
