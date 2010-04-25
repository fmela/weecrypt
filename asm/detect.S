/*
 * detect.S
 * Copyright (C) 2002-2010 Farooq Mela. All rights reserved.
 *
 * int detect_cpuid(void);
 * int detect_mmx(void);
 * int detect_sse(void);
 *
 * $Id$
 */

.text
	.globl	detect_cpuid
#ifndef __APPLE__
	.type	detect_cpuid,@function
#endif
	.align	8
detect_cpuid:
	pushf					# push eflags
	popl %ecx				# pop eflags into ecx
	movl %ecx,%edx			# copy ecx to edx
	xorl $0x00200000,%ecx	# flip bit 21 in ecx
	pushl %ecx				# push ecx
	popf					# pop flags
	xorl %eax,%eax			# clear return value
	pushf					# push eflags
	popl %ecx				# pop eflags into ecx
	cmpl %ecx,%edx			# compare with edx
	sete %al				# store equal flag
	xorb $1,%al				# flip flag
	ret						# return

# These routines require that the CPU has CPUID instruction support.  Call
# detect_cpuid() to ensure this is the case before calling these routines.
	.globl	detect_mmx
/*	.type	detect_mmx,@function */
	.align	8
detect_mmx:
	movl $1,%eax			# request feature flags
	cpuid					# execute cpuid (0x0f 0xa2)
	xorl %eax,%eax			# clear return value
	testl $0x00800000,%edx	# test bit 23
	setz %al				# store zero flag
	xorb $1,%al				# flip flag
	ret						# return

	.globl	detect_sse
/*	.type	detect_sse,@function */
	.align	8
detect_sse:
	movl $1,%eax			# request feature flags
	cpuid					# execute cpuid (0x0f 0xa2)
	xorl %eax,%eax			# clear return value
	testl $0x02000000,%edx	# test bit 25
	setz %al				# store zero flag
	xorb $1,%al				# flip flag
	ret						# return
