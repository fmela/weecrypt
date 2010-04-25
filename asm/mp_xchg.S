/*
 * mp_xchg.S
 * Copyright (C) 2005-2010 Farooq Mela. All rights reserved.
 *
 * void mp_xchg(mp_digit *u, mp_digit *v, mp_size len);
 *
 * Parameters:
 * esp+4	u
 * esp+8	v
 * esp+12	len
 *
 * $Id$
 */

#include "mp_config.h"

#ifdef MP_XCHG_ASM

#define STACK	0x08
#define U_LOC	(STACK+0x04)(%esp)
#define V_LOC	(STACK+0x08)(%esp)
#define L_LOC	(STACK+0x0c)(%esp)

.macro xchg digit
	movl	\digit(%esi),%eax
	xchgl	\digit(%edi),%eax
	movl	%eax,\digit(%esi)
.endm

.text
	.globl	mp_xchg
#ifndef __APPLE__
	.type	mp_xchg,@function
#endif
	.align	8
mp_xchg:
	pushl	%esi
	pushl	%edi

	movl	L_LOC,%ecx					/* ecx = len						*/
	orl		%ecx,%ecx
	jz		.Lexit						/* exit if zero						*/
	movl	U_LOC,%esi					/* esi = u							*/
	movl	V_LOC,%edi					/* edi = v							*/
	cmpl	%esi,%edi
	je		.Lexit						/* exit if equal					*/

	shrl	$3,%ecx						/* divide by 8 for unrolled loop	*/
	jz		.Lskipunroll				/* if zero we have < 8 digits		*/

	.align	16
.Lunroll:
	xchg 0
	xchg 4
	xchg 8
	xchg 12
	xchg 16
	xchg 20
	xchg 24
	xchg 28
	addl $32,%esi						/* move u pointer along				*/
	addl $32,%edi						/* move v pointer along				*/
	decl %ecx							/* decrement # of 8-digit clumps	*/
	jnz .Lunroll						/* loop								*/

	.align	8
.Lskipunroll:
	movl	L_LOC,%ecx					/* ecx = len						*/
	andl	$7,%ecx						/* digits modulo 8					*/
	jz		.Lexit						/* nothing left to do				*/

	shll	$2,%ecx
	addl	%ecx,%esi
	addl	%ecx,%edi
	shrl	$2,%ecx
	negl	%ecx

	.align	8
.Lsimple:
	movl	(%esi,%ecx,4),%eax
	xchgl	(%edi,%ecx,4),%eax
	movl	%eax,(%esi,%ecx,4)
	incl	%ecx
	jnz		.Lsimple

	.align	8
.Lexit:
	popl	%edi
	popl	%esi
	ret

#endif /* MP_XCHG_ASM */
