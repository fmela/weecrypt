/*
 * mp_sqr_diag.S
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * void mp_sqr_diag(const mp_digit *u, mp_len len, mp_digit *v)
 *
 * Parameters:
 * esp+4	u
 * esp+8	len
 * esp+12	v
 *
 * $Id$
 */

/* TODO: Unroll the loop so it processes 8 digits at a time */

#include "mp_config.h"

#ifdef MP_SQR_DIAG_ASM

#define STACK	0x0c
#define U_LOC	(STACK+0x04)(%esp)
#define L_LOC	(STACK+0x08)(%esp)
#define V_LOC	(STACK+0x0c)(%esp)

.text
	.globl	MP_ASM_NAME(mp_sqr_diag)
#ifndef __APPLE__
	.type	MP_ASM_NAME(mp_sqr_diag),@function
#endif
	.p2align	3
MP_ASM_NAME(mp_sqr_diag):
	pushl	%esi
	pushl	%edi
	pushl	%ebx

	movl	L_LOC,%ecx					/* load len							*/
	jecxz	.Lexit						/* done if zero						*/
	movl	U_LOC,%esi					/* load u							*/
	movl	V_LOC,%edi					/* load v							*/

	shll	$2,%ecx
	addl	%ecx,%esi					/* u += len							*/
	shll	%ecx
	addl	%ecx,%edi					/* v += len*2						*/
	shrl	$3,%ecx
	negl	%ecx						/* negate len						*/
	xorl	%ebx,%ebx					/* clear carry						*/

	/* It would make more sense to keep carry in the carry flag instead of ebx,
	 * but since mul clobbers the carry flag we have no choice. */
	.p2align	3
.Lsimple:
	movl	(%esi,%ecx,4),%eax			/* eax = u[i]						*/
	mull	%eax						/* square hi=edx lo=eax				*/
	addl	%ebx,%eax					/* lo += prev carry					*/
	adcl	$0,%edx						/* hi += carry						*/
	xorl	%ebx,%ebx					/* clear next carry					*/
	addl	%eax,(%edi,%ecx,8)			/* v[i*2] += lo						*/
	adcl	%edx,4(%edi,%ecx,8)			/* v[i*2+1] += hi + carry			*/
	adcl	$0,%ebx						/* put next carry in ebx			*/
	incl	%ecx
	jnz		.Lsimple					/* loop								*/

.Lexit:
	popl	%ebx
	popl	%edi
	popl	%esi
	ret

#endif /* MP_SQR_DIAG_ASM */
