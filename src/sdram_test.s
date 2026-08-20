	.section .text
	.align  2

	.global _Hw32xTestSdramAddressLines
_Hw32xTestSdramAddressLines:
	/* Preserve the ABI callee-saved registers before SDRAM becomes quiescent. */
	mov.l	r8,@-r15
	mov.l	r9,@-r15
	mov.l	r10,@-r15
	sts	pr,r7
	stc	sr,r6

	/* Mask interrupts, purge the cache, and leave it disabled. */
	mov	#0xf0,r0
	ldc	r0,sr
	mov.l	.Lcctl,r1
	mov.b	@r1,r5
	extu.b	r5,r5
	mov	#0x10,r0
	mov.b	r0,@r1

	/*
	 * Address-line test.  Compare the base word with one word selected by
	 * each SDRAM address bit.  Both locations are restored after every probe.
	 */
	mov.l	.Lsdram_start,r2
	mov	#2,r4
.Laddress_loop:
	mov.w	@r2,r3
	mov	r2,r0
	add	r4,r0
	mov.w	@r0,r8

	mov.l	.Lpattern_5555,r9
	mov.w	r9,@r2
	mov.l	.Lpattern_aaaa,r10
	mov.w	r10,@r0

	mov.w	@r2,r9
	extu.w	r9,r9
	mov.l	.Lpattern_5555,r10
	cmp/eq	r10,r9
	bf	.Laddress_base_failed
	mov.w	@r0,r9
	extu.w	r9,r9
	mov.l	.Lpattern_aaaa,r10
	cmp/eq	r10,r9
	bf	.Laddress_target_failed

	mov.w	r8,@r0
	mov.w	r3,@r2
	shll	r4
	mov.l	.Lsdram_size,r9
	cmp/eq	r9,r4
	bf	.Laddress_loop
	mov	#-1,r0
	bra	.Lfinish
	nop

.Laddress_base_failed:
	mov.w	r8,@r0
	mov.w	r3,@r2
	mov	r2,r0
	bra	.Lfinish
	nop

.Laddress_target_failed:
	mov.w	r8,@r0
	mov.w	r3,@r2
	bra	.Lfinish
	nop

	/*
	 * Test one caller-selected pattern across every physical 16-bit SDRAM
	 * cell.  Splitting the patterns lets the UI advance one row at a time.
	 */
	.align 2
	.global _Hw32xTestSdramPattern
_Hw32xTestSdramPattern:
	mov.l	r8,@-r15
	mov.l	r9,@-r15
	mov.l	r10,@-r15
	sts	pr,r7
	stc	sr,r6
	mov	r4,r10
	extu.w	r10,r10

	mov	#0xf0,r0
	ldc	r0,sr
	mov.l	.Lcctl,r1
	mov.b	@r1,r5
	extu.b	r5,r5
	mov	#0x10,r0
	mov.b	r0,@r1

	mov.l	.Lsdram_start,r2
	mov.l	.Lsdram_end,r4
.Lcell_loop:
	mov.w	@r2,r3

	mov.w	r10,@r2
	mov.w	@r2,r9
	extu.w	r9,r9
	cmp/eq	r10,r9
	bf	.Lcell_failed

	/* Verify that the original word was restored before advancing. */
	mov.w	r3,@r2
	mov.w	@r2,r9
	cmp/eq	r3,r9
	bf	.Lcell_failed_restored

	add	#2,r2
	cmp/eq	r4,r2
	bf	.Lcell_loop
	mov	#-1,r0
	bra	.Lfinish
	nop

.Lcell_failed:
	mov.w	r3,@r2
.Lcell_failed_restored:
	mov	r2,r0

.Lfinish:
	/* Purge test data, restore the entry cache mode and interrupt mask. */
	mov	#0x10,r4
	mov.b	r4,@r1
	mov.b	r5,@r1
	mov.l	@r15+,r10
	mov.l	@r15+,r9
	mov.l	@r15+,r8
	ldc	r6,sr
	jmp	@r7
	nop

	.align 2
.Lcctl:
	.long	0xfffffe92
.Lsdram_start:
	.long	0x26000000
.Lsdram_end:
	.long	0x26040000
.Lsdram_size:
	.long	0x00040000
.Lpattern_5555:
	.long	0x00005555
.Lpattern_aaaa:
	.long	0x0000aaaa

	.align 2
	.global _Hw32xSecondaryPark
_Hw32xSecondaryPark:
	sts	pr,r7
	stc	sr,r6
	mov	#0xf0,r0
	ldc	r0,sr
	mov.l	.Lsec_cctl,r1
	mov.b	@r1,r5
	extu.b	r5,r5
	mov	#0x10,r0
	mov.b	r0,@r1

	/* Acknowledge only after the secondary no longer needs SDRAM. */
	mov.l	.Lcomm4,r2
	mov.l	.Lparked,r0
	mov.w	r0,@r2
.Lpark_wait:
	mov.w	@r2,r0
	extu.w	r0,r0
	mov	#10,r3
	cmp/eq	r3,r0
	bf	.Lpark_wait

	mov	#0,r0
	mov.w	r0,@r2
	mov	#0x10,r0
	mov.b	r0,@r1
	mov.b	r5,@r1
	ldc	r6,sr
	jmp	@r7
	nop

	.align 2
.Lsec_cctl:
	.long	0xfffffe92
.Lcomm4:
	.long	0x20004024
.Lparked:
	.long	0x00005350
