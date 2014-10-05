/*
 * mp_dmod.S
 * Copyright (C) 2001-2010 Farooq Mela. All rights reserved.
 *
 * mp_digit_t mp_dmod(const mp_digit_t *u, mp_len_t len, mp_digit_t v);
 *
 * Parameters:
 * esp+4	u
 * esp+8	len
 * esp+12	v
 *
 * $Id: mp_dmod.S,v 1.6 2002/06/30 07:55:31 farooq Exp $
 */

#include "mp_config.h"

#ifdef MP_DMOD_ASM

#define STACK	0x08
#define U_LOC	(STACK+0x04)(%esp)
#define L_LOC	(STACK+0x08)(%esp)
#define V_LOC	(STACK+0x0c)(%esp)

.text
	.globl	MP_ASM_NAME(mp_dmod)
#ifndef __APPLE__
	.type	MP_ASM_NAME(mp_dmod),@function
#endif
	.p2align 3
MP_ASM_NAME(mp_dmod):
	pushl	%ebx
	pushl	%esi

	xorl	%eax,%eax
	movl	L_LOC,%ecx
	jecxz	.L1
	movl	U_LOC,%esi
	movl	V_LOC,%ebx
	xorl	%edx,%edx

	movl	%ebx,%eax
	decl	%eax
	andl	%ebx,%eax
	jnz	.L0		    /* v is a power of 2 if v & (v-1) == 0 */

	decl	%ebx
	andl	(%esi),%ebx
	movl	%ebx,%eax
	jmp	.L1

	.p2align 3
.L0:
	decl	%ecx
	movl	(%esi,%ecx,4),%eax
	divl	%ebx
	orl	%ecx,%ecx
	jnz	.L0
	movl	%edx,%eax

	.p2align 3
.L1:
	popl	%esi
	popl	%ebx
	ret

#endif /* MP_DMOD_ASM */
