/*
 * mp_dmul_add.S
 * Copyright (C) 2001-2010 Farooq Mela. All rights reserved.
 *
 * mp_digit_t mp_dmul_add(const mp_digit_t *u, mp_len_t len, mp_digit_t v,
 * 						  mp_digit_t *w);
 *
 * Parameters:
 * esp+4	u
 * esp+8	len
 * esp+12	v
 * esp+16	w
 *
 * $Id: mp_dmul_add.S,v 1.11 2003/08/03 07:35:02 farooq Exp $
 */

#include "mp_config.h"

#ifdef MP_DMUL_ADD_ASM

#define STACK	0x10
#define U_LOC	(STACK+0x04)(%esp)
#define L_LOC	(STACK+0x08)(%esp)
#define V_LOC	(STACK+0x0c)(%esp)
#define W_LOC	(STACK+0x10)(%esp)

.macro mul_add digit
	movl	\digit(%esi),%eax
	mull	%ebp
	addl	%ebx,%eax
	adcl	$0,%edx
	xorl	%ebx,%ebx
	addl	%eax,\digit(%edi)
	adcl	%edx,%ebx
.endm

.text
	.globl	mp_dmul_add
#ifndef __APPLE__
	.type	mp_dmul_add,@function
#endif
	.align	8
mp_dmul_add:
	pushl	%ebx
	pushl	%edi
	pushl	%esi
	pushl	%ebp

	xorl	%ebx,%ebx
	movl	L_LOC,%ecx
	orl		%ecx,%ecx
	jz		.Lexit
	movl	V_LOC,%ebp
	orl		%ebp,%ebp
	jz		.Lexit
	movl	U_LOC,%esi
	movl	W_LOC,%edi

	shrl	$3,%ecx
	jz		.Lskipunroll

	.align	8
.Lunroll:
	mul_add 0
	mul_add	4
	mul_add	8
	mul_add	12
	mul_add	16
	mul_add	20
	mul_add	24
	mul_add	28

	addl	$32,%esi
	addl	$32,%edi
	decl	%ecx
	jnz		.Lunroll

	.align	8
.Lskipunroll:
	movl	L_LOC,%ecx
	andl	$7,%ecx
	jz		.Lexit

	negl	%ecx
	sall	$2,%ecx
	subl	%ecx,%esi
	subl	%ecx,%edi
	sarl	$2,%ecx

	.align	8
.Lsimple:
	movl	(%esi,%ecx,4),%eax
	mull	%ebp
	addl	%ebx,%eax
	adcl	$0,%edx
	xorl	%ebx,%ebx
	addl	%eax,(%edi,%ecx,4)
	adcl	%edx,%ebx
	incl	%ecx
	jnz		.Lsimple

	.align	8
.Lexit:
	movl	%ebx,%eax
	popl	%ebp
	popl	%esi
	popl	%edi
	popl	%ebx
	ret

#endif /* MP_DMUL_ADD_ASM */
