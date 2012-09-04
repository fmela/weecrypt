/*
 * mp_add_n.S
 * Copyright (C) 2001-2010 Farooq Mela. All rights reserved.
 *
 * mp_digit_t mp_add_n(const mp_digit_t *u, const mp_digit_t *v,
 *                     mp_len_t len, mp_digit_t *w);
 *
 * Parameters:
 * esp+4	u
 * esp+8	v
 * esp+12	len
 * esp+16	w
 *
 * $Id: mp_add_n.S,v 1.11 2002/07/31 09:32:38 farooq Exp $
 */

#include "mp_config.h"

#ifdef MP_ADD_N_ASM

#define STACK	0x08
#define U_LOC	(STACK+0x04)(%esp)
#define V_LOC	(STACK+0x08)(%esp)
#define L_LOC	(STACK+0x0c)(%esp)
#define W_LOC	(STACK+0x10)(%esp)

#ifdef __APPLE__
.macro add
	movl	$0(%esi),%eax
	adcl	$0(%edx),%eax
	movl	%eax,$0(%edi)
.endmacro
#else
.macro add digit
	movl	\digit(%esi),%eax
	adcl	\digit(%edx),%eax
	movl	%eax,\digit(%edi)
.endm
#endif

.text
	.globl	MP_ASM_NAME(mp_add_n)
#ifndef __APPLE__
	.type	MP_ASM_NAME(mp_add_n),@function
#endif
	.p2align	3
MP_ASM_NAME(mp_add_n):
	pushl	%esi
	pushl	%edi

	xorl	%eax,%eax				/* zero return value					*/
	movl	L_LOC,%ecx				/* ecx = len							*/
	orl		%ecx,%ecx
	jz		.Lexit
	movl	U_LOC,%esi				/* esi = u								*/
	movl	V_LOC,%edx				/* edx = v								*/
	movl	W_LOC,%edi				/* edi = w								*/

	shrl	$4,%ecx					/* divide by 16 for unrolled loop		*/
	jz		.Lskipunroll			/* if zero we have < 16 digits			*/
	clc								/* clear carry							*/

	.p2align	5
.Lunroll:
	add 0
	add 4
	add	8
	add 12
	add 16
	add 20
	add 24
	add 28
	add 32
	add 36
	add 40
	add 44
	add 48
	add 52
	add 56
	add 60

	leal	64(%esi),%esi			/* move pointers along...				*/
	leal	64(%edx),%edx			/*     ... without clobbering ...		*/
	leal	64(%edi),%edi			/*           ... carry flag				*/
	decl	%ecx					/* decrement # of 16-digit clumps		*/
	jnz		.Lunroll				/* loop									*/

	setc	%al						/* eax = carry							*/
	andl	$1,%eax

	.p2align	3
.Lskipunroll:
	movl	L_LOC,%ecx				/* reload ecx							*/
	andl	$15,%ecx				/* get remaining digit count			*/
	jz		.Lexit					/* done if zero							*/

	shll	$2,%ecx					/* to bytes								*/
	addl	%ecx,%esi				/* offset u								*/
	addl	%ecx,%edx				/* offset v								*/
	addl	%ecx,%edi				/* offset w								*/
	shrl	$2,%ecx					/* back to digits						*/
	negl	%ecx					/* negate ecx							*/

	clc
	orb		%al,%al					/* remember carry						*/
	jz		.Lsimple
	stc								/* set carry							*/

	.p2align	3
.Lsimple:
	movl	(%esi,%ecx,4),%eax		/* eax = u-digit						*/
	adcl	(%edx,%ecx,4),%eax		/* eax += v-digit + carry				*/
	movl	%eax,(%edi,%ecx,4)		/* w-digit = eax						*/
	incl	%ecx					/* decrement count						*/
	jnz		.Lsimple				/* loop									*/

	setc	%al						/* al = carry							*/
	andl	$1,%eax					/* mask off rest of eax					*/

	.p2align	3
.Lexit:
	popl	%edi
	popl	%esi
	ret

#endif /* MP_ADD_N_ASM */
