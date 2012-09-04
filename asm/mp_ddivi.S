/*
 * mp_ddivi.S
 * Copyright (C) 2001-2010 Farooq Mela. All rights reserved.
 *
 * mp_digit_t mp_ddivi(mp_digit_t *u, mp_len_t len, mp_digit_t v);
 *
 * Parameters:
 * esp+4	u
 * esp+8	len
 * esp+12	v
 *
 * $Id: mp_ddivi.S,v 1.6 2002/01/02 08:56:51 farooq Exp $
 */

#include "mp_config.h"

#ifdef MP_DDIVI_ASM

#define STACK	0x0c
#define U_LOC	(STACK+0x04)(%esp)
#define L_LOC	(STACK+0x08)(%esp)
#define V_LOC	(STACK+0x0c)(%esp)

.text
	.globl	MP_ASM_NAME(mp_ddivi)
#ifndef __APPLE__
	.type	MP_ASM_NAME(mp_ddivi),@function
#endif
	.p2align	3
MP_ASM_NAME(mp_ddivi):
	pushl	%ebx
	pushl	%esi
	pushl	%edi

	xorl	%eax,%eax
	movl	L_LOC,%ecx
	jecxz	.L3
	movl	U_LOC,%esi
	movl	V_LOC,%ebx

	movl	%ebx,%eax				/* power of two?						*/
	decl	%eax
	andl	%ebx,%eax
	jz		.L1

	xorl	%edx,%edx				/* clear remainder						*/
	.p2align	3
.L0:
	decl	%ecx					/* decrement count						*/
	movl	(%esi,%ecx,4),%eax		/* load divisor digit					*/
	divl	%ebx					/* divide edx:eax into q=eax r=edx		*/
	movl	%eax,(%esi,%ecx,4)		/* store quotient						*/
	orl		%ecx,%ecx				/* check count == 0						*/
	jnz		.L0						/* loop									*/
	movl	%edx,%eax				/* return remainder						*/
	jmp		.L3

	.p2align	3
.L1:
	movl	%ecx,%edi				/* edi = len so we can shift using cl	*/
	bsfl	%ebx,%ecx				/* cl = lg(v), rest of ecx zeroed		*/
	xorl	%eax,%eax				/* clear "previous" bits				*/

	.p2align	3
.L2:
	decl	%edi
	movl	(%esi,%edi,4),%edx		/* edx = u-digit						*/
	shrdl	%cl,%eax,(%esi,%edi,4)	/* shift u-digit & combine w/prev bits	*/
	movl	%edx,%eax				/* eax = u-digit before shift			*/
	orl		%edi,%edi
	jnz		.L2						/* loop									*/

	movl	$1,%eax
	shll	%cl,%eax
	decl	%eax					/* get mask = 2 ** lg(v) - 1 = v - 1	*/
	andl	%edx,%eax				/* put remainder onto [0, v)			*/

	.p2align	3
.L3:
	popl	%edi					/* yay we are done						*/
	popl	%esi
	popl	%ebx
	ret

#endif /* MP_DDIVI_ASM */
