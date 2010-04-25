/*
 * mp_copy_mmx.S
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * mp_digit_t *mp_copy(const mp_digit_t *u, mp_len_t len, mp_digit_t *v);
 *
 * Parameters:
 * esp+4	u
 * esp+8	len
 * esp+12	v
 *
 * $Id$
 */

#include "mp_config.h"

#define STACK	0x08
#define U_LOC	(STACK+0x04)(%esp)
#define L_LOC	(STACK+0x08)(%esp)
#define V_LOC	(STACK+0x0c)(%esp)

#ifdef MP_COPY_MMX_ASM
.text
	.align	8
	.globl	mp_copy_mmx
#ifndef __APPLE__
	.type	mp_copy_mmx,@function
#endif
mp_copy_mmx:
	pushl	%esi				# save edi
	pushl	%edi				# save esi

	movl	L_LOC,%ecx			# load length
	jecxz	.Lexit				# if zero exit
	movl	U_LOC,%esi			# load source
	movl	V_LOC,%edi			# load dest

	shrl	$4,%ecx				# divide by 16
	jz		.Lsimple			# if zero jump to simple

	.align	8
.Lunroll:
	movq	 0(%esi),%mm0
	movq	%mm0, 0(%edi)
	movq	 8(%esi),%mm0
	movq	%mm0, 8(%edi)
	movq	16(%esi),%mm0
	movq	%mm0,16(%edi)
	movq	24(%esi),%mm0
	movq	%mm0,24(%edi)
	movq	32(%esi),%mm0
	movq	%mm0,32(%edi)
	movq	40(%esi),%mm0
	movq	%mm0,40(%edi)
	movq	48(%esi),%mm0
	movq	%mm0,48(%edi)
	movq	56(%esi),%mm0
	movq	%mm0,56(%edi)

	addl	$64,%esi			# move src pointer along 16 digits
	addl	$64,%edi			# move src pointer along 16 digits
	decl	%ecx				# decrement # 16-digit clumps
	jnz		.Lunroll			# loop
	emms						# done with MMX registers

.Lsimple:
	movl	L_LOC,%ecx			# load length
	andl	$15,%ecx			# remainder from 16
	jz		.Lexit				# happens iff count multiple of 16
	cld							# go forward
	rep		movsl				# copy

.Lexit:
	movl	V_LOC,%eax			# return value = dest
	popl	%edi				# restore edi
	popl	%esi				# restore esi
	ret							# return
#endif
