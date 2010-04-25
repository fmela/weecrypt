/*
 * mp_ddiv.S
 * Copyright (C) 2001-2010 Farooq Mela. All rights reserved.
 *
 * mp_digit_t mp_ddiv(mp_digit_t *u, mp_len_t len,
 *                    mp_digit_t v, mp_digit_t *w);
 *
 * Parameters:
 * esp+4	u
 * esp+8	len
 * esp+12	v
 * esp+16	w
 *
 * $Id: mp_ddiv.S,v 1.4 2002/05/07 06:03:35 farooq Exp $
 */

#include "mp_config.h"

#ifdef MP_DDIV_ASM

#define STACK	0x0c
#define U_LOC	(STACK+0x04)(%esp)
#define L_LOC	(STACK+0x08)(%esp)
#define V_LOC	(STACK+0x0c)(%esp)
#define W_LOC	(STACK+0x10)(%esp)

.text
	.globl	mp_ddiv
#ifndef __APPLE__
	.type	mp_ddiv,@function
#endif
	.align	8
mp_ddiv:
	pushl	%ebx
	pushl	%esi
	pushl	%edi

	xorl	%edx,%edx			/* clear remainder							*/
	movl	L_LOC,%ecx
	jecxz	.L1
	movl	U_LOC,%esi
	movl	V_LOC,%ebx
	movl	W_LOC,%edi

	.align	8
.L0:
	decl	%ecx
	movl	(%esi,%ecx,4),%eax	/* eax = u-digit							*/
	divl	%ebx				/* divide edx:eax into q=eax r=edx			*/
	movl	%eax,(%edi,%ecx,4)	/* store one digit at w						*/
	orl		%ecx,%ecx
	jnz		.L0

	.align	8
.L1:
	movl	%edx,%eax			/* put remainder in eax						*/
	popl	%edi
	popl	%esi
	popl	%ebx
	ret

#endif /* MP_DDIV_ASM */
