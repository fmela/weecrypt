/*
 * mp_subi_n.S
 * Copyright (C) 2001-2010 Farooq Mela. All rights reserved.
 *
 * mp_digit_t mp_subi_n(mp_digit_t *u, const mp_digit_t *v, mp_len_t len);
 *
 * Parameters:
 * esp+4	u
 * esp+8	v
 * esp+12	len
 *
 * $Id: mp_subi_n.S,v 1.5 2002/01/02 08:56:51 farooq Exp $
 */

#include "mp_config.h"

#ifdef MP_SUBI_N_ASM

#define STACK	0x08
#define U_LOC	(STACK+0x04)(%esp)
#define V_LOC	(STACK+0x08)(%esp)
#define L_LOC	(STACK+0x0c)(%esp)

#ifdef __APPLE__
.macro sub
	movl	$0(%edi),%eax
	sbbl	$0(%esi),%eax
	movl	%eax,$0(%edi)
.endmacro
#else
.macro sub digit
	movl	\digit(%edi),%eax
	sbbl	\digit(%esi),%eax
	movl	%eax,\digit(%edi)
.endm
#endif

.text
	.globl	MP_ASM_NAME(mp_subi_n)
#ifndef __APPLE__
	.type	MP_ASM_NAME(mp_subi_n),@function
#endif
	.p2align	3
MP_ASM_NAME(mp_subi_n):
	pushl	%esi
	pushl	%edi

	xorl	%eax,%eax
	movl	L_LOC,%ecx
	orl	%ecx,%ecx
	jz	.Lexit
	movl	U_LOC,%edi
	movl	V_LOC,%esi

	shrl	$4,%ecx
	jz	.Lskipunroll
	clc

	.p2align	5
.Lunroll:
	sub 0
	sub 4
	sub 8
	sub 12
	sub 16
	sub 20
	sub 24
	sub 28
	sub 32
	sub 36
	sub 40
	sub 44
	sub 48
	sub 52
	sub 56
	sub 60

	leal	64(%esi),%esi
	leal	64(%edi),%edi
	decl	%ecx
	jnz	.Lunroll

	setc	%al
	andl	$1,%eax

	.p2align 3
.Lskipunroll:
	movl	L_LOC,%ecx
	andl	$15,%ecx
	jz	.Lexit

	negl	%ecx
	sall	$2,%ecx
	subl	%ecx,%esi
	subl	%ecx,%edi
	sarl	$2,%ecx

	clc
	orb	%al,%al
	jz	.Lsimple
	stc

	.p2align 3
.Lsimple:
	movl	(%edi,%ecx,4),%eax
	sbbl	(%esi,%ecx,4),%eax
	movl	%eax,(%edi,%ecx,4)
	incl	%ecx
	jnz	.Lsimple

	setc	%al
	andl	$1,%eax

	.p2align 3
.Lexit:
	popl	%edi
	popl	%esi
	ret

#endif /* !MP_SUBI_N_ASM */
