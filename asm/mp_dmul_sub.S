/*
 * mp_dmul_sub.S
 * Copyright (C) 2001-2010 Farooq Mela. All rights reserved.
 *
 * mp_digit_t mp_dmul_sub(const mp_digit_t *u, mp_len_t len, mp_digit_t v,
 * 			  mp_digit_t *w);
 *
 * Parameters:
 * esp+4	u
 * esp+8	len
 * esp+12	v
 * esp+16	w
 *
 * $Id: mp_dmul_sub.S,v 1.7 2002/06/29 05:55:54 farooq Exp $
 */

#include "mp_config.h"

#ifdef MP_DMUL_SUB_ASM

#define STACK	0x10
#define U_LOC	(STACK+0x04)(%esp)
#define L_LOC	(STACK+0x08)(%esp)
#define V_LOC	(STACK+0x0c)(%esp)
#define W_LOC	(STACK+0x10)(%esp)

#ifdef __APPLE__
.macro mul_sub
	movl	$0(%esi),%eax
	mull	%ebp
	addl	%ebx,%eax
.macros_off
	adcl	$0,%edx
.macros_on
	xorl	%ebx,%ebx
	subl	%eax,$0(%edi)
	adcl	%edx,%ebx
.endmacro
#else
.macro mul_sub digit
	movl	\digit(%esi),%eax
	mull	%ebp
	addl	%ebx,%eax
	adcl	$0,%edx
	xorl	%ebx,%ebx
	subl	%eax,\digit(%edi)
	adcl	%edx,%ebx
.endm
#endif

.text
	.globl	MP_ASM_NAME(mp_dmul_sub)
#ifndef __APPLE__
	.type	MP_ASM_NAME(mp_dmul_sub),@function
#endif
	.p2align 3
MP_ASM_NAME(mp_dmul_sub):
	pushl	%ebx
	pushl	%edi
	pushl	%esi
	pushl	%ebp

	xorl	%ebx,%ebx
	movl	L_LOC,%ecx
	orl	%ecx,%ecx
	jz	.Lexit
	movl	V_LOC,%ebp
	orl	%ebp,%ebp
	jz	.Lexit
	movl	U_LOC,%esi
	movl	W_LOC,%edi

	shrl	$3,%ecx
	jz	.Lskipunroll

	.p2align 3
.Lunroll:
	mul_sub 0
	mul_sub 4
	mul_sub 8
	mul_sub 12
	mul_sub 16
	mul_sub 20
	mul_sub 24
	mul_sub 28

	addl	$32,%esi
	addl	$32,%edi
	decl	%ecx
	jnz	.Lunroll

	.p2align 3
.Lskipunroll:
	movl	L_LOC,%ecx
	andl	$7,%ecx
	jz	.Lexit

	negl	%ecx
	sall	$2,%ecx
	subl	%ecx,%esi
	subl	%ecx,%edi
	sarl	$2,%ecx

	.p2align 3
.Lsimple:
	movl	(%esi,%ecx,4),%eax
	mull	%ebp
	addl	%ebx,%eax
	adcl	$0,%edx
	xorl	%ebx,%ebx
	subl	%eax,(%edi,%ecx,4)
	adcl	%edx,%ebx
	incl	%ecx
	jnz	.Lsimple

	.p2align 3
.Lexit:
	movl	%ebx,%eax
	popl	%ebp
	popl	%esi
	popl	%edi
	popl	%ebx
	ret

#endif /* MP_DMUL_SUB_ASM */
