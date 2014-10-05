/*
 * mp_dmul_add.S
 * Copyright (C) 2001-2010 Farooq Mela. All rights reserved.
 *
 * mp_digit_t mp_dmul_add(const mp_digit_t *u, mp_len_t len, mp_digit_t v,
 * 			  mp_digit_t *w);
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

#ifdef __APPLE__
.macro mul_add
	movl	$0(%esi),%eax
	mull	%ebp
	addl	%ebx,%eax
.macros_off
	adcl	$0,%edx
.macros_on
	xorl	%ebx,%ebx
	addl	%eax,$0(%edi)
	adcl	%edx,%ebx
.endmacro
#else
.macro mul_add digit
	movl	\digit(%esi),%eax
	mull	%ebp
	addl	%ebx,%eax
	adcl	$0,%edx
	xorl	%ebx,%ebx
	addl	%eax,\digit(%edi)
	adcl	%edx,%ebx
.endm
#endif

.text
	.globl	MP_ASM_NAME(mp_dmul_add)
#ifndef __APPLE__
	.type	MP_ASM_NAME(mp_dmul_add),@function
#endif
	.p2align 4
MP_ASM_NAME(mp_dmul_add):
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

	.p2align 4
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
	addl	%eax,(%edi,%ecx,4)
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

#endif /* MP_DMUL_ADD_ASM */
