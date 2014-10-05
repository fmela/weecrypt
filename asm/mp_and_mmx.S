/*
 * mp_and_mmx.S
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * void mp_and(mp_digit_t *u, mp_len_t len, const mp_digit_t *v)
 *
 * Parameters:
 * esp+4	u
 * esp+8	len
 * esp+12	v
 *
 * $Id$
 */

#include "mp_config.h"

#ifdef MP_AND_MMX

#define STACK	0x08
#define U_LOC	(STACK+0x04)(%esp)
#define L_LOC	(STACK+0x08)(%esp)
#define V_LOC	(STACK+0x0c)(%esp)

.text
	.globl	MP_ASM_NAME(mp_and_mmx)
#ifndef __APPLE__
	.type	MP_ASM_NAME(mp_and_mmx),@function
#endif
	.p2align	3
MP_ASM_NAME(mp_and_mmx):
	pushl	%esi				# save esi
	pushl	%edi				# save edi

	movl	L_LOC,%ecx			# load length
	jecxz	.Lexit				# exit if zero
	movl	V_LOC,%esi			# edi = u
	movl	U_LOC,%edi			# esi = v

	shrl	$4,%ecx
	jz	.Lskip

.Lmmx:
	prefetcht0	(%esi)
	movq	0(%esi),%mm0
	pand	0(%edi),%mm0
	movq	8(%esi),%mm1
	pand	8(%edi),%mm1
	movq	16(%esi),%mm2
	pand	16(%edi),%mm2
	movq	24(%esi),%mm3
	pand	24(%edi),%mm3
	movq	32(%esi),%mm4
	pand	32(%edi),%mm4
	movq	40(%esi),%mm5
	pand	40(%edi),%mm5
	movq	48(%esi),%mm6
	pand	48(%edi),%mm6
	movq	56(%esi),%mm7
	pand	56(%edi),%mm7

	movq	%mm0,0(%edi)
	movq	%mm1,8(%edi)
	movq	%mm2,16(%edi)
	movq	%mm3,24(%edi)
	movq	%mm4,32(%edi)
	movq	%mm5,40(%edi)
	movq	%mm6,48(%edi)
	movq	%mm7,56(%edi)

	addl	$64,%esi
	addl	$64,%edi
	decl	%ecx
	jnz	.Lmmx

	emms

.Lskip:
	movl	L_LOC,%ecx
	andl	$15,%ecx
	jz	.Lexit

.Lnormal:
	movl	(%esi),%edx
	andl	%edx,(%edi)
	addl	$4,%esi
	addl	$4,%edi
	decl	%ecx
	jnz	.Lnormal

.Lexit:
	popl	%edi
	popl	%esi
	ret

#endif /* MP_AND_MMX */
