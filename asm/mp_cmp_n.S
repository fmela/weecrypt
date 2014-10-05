/*
 * mp_cmp_n.S
 * Copyright (C) 2003-2010 Farooq Mela. All rights reserved.
 *
 * int mp_cmp_n(const mp_digit *u, const mp_digit *v, mp_len len);
 *
 * Parameters:
 * esp+4	u
 * esp+8	v
 * esp+12	len
 *
 * $Id$
 */

#include "mp_config.h"

#ifdef MP_CMP_N_ASM

#define STACK	0x08
#define U_LOC	(STACK+0x04)(%esp)
#define V_LOC	(STACK+0x08)(%esp)
#define L_LOC	(STACK+0x0c)(%esp)

.text
	.globl	MP_ASM_NAME(mp_cmp_n)
#ifndef __APPLE__
	.type	MP_ASM_NAME(mp_cmp_n),@function
#endif
	.p2align 3
MP_ASM_NAME(mp_cmp_n):
	pushl	%esi
	pushl	%edi

	xorl	%eax,%eax
	movl	L_LOC,%ecx
	orl	%ecx,%ecx
	jz	.Leq
	movl	U_LOC,%esi
	movl	V_LOC,%edi

	.p2align 3
.Loop:
	decl	%ecx
	movl	(%esi,%ecx,4),%edx
	cmpl	(%edi,%ecx,4),%edx
	ja	.Labove
	jb	.Lbelow
	orl	%ecx,%ecx
	jz	.Leq
	jmp	.Loop

.Labove:
	movl	$+1,%eax
	jmp	.Leq

.Lbelow:
	movl	$-1,%eax

.Leq:
	popl	%edi
	popl	%esi
	ret

#endif /* MP_CMP_N_ASM */
