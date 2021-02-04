	.text
	.syntax unified
	.eabi_attribute	67, "2.09"	@ Tag_conformance
	.eabi_attribute	6, 10	@ Tag_CPU_arch
	.eabi_attribute	7, 65	@ Tag_CPU_arch_profile
	.eabi_attribute	8, 1	@ Tag_ARM_ISA_use
	.eabi_attribute	9, 2	@ Tag_THUMB_ISA_use
	.fpu	neon
	.eabi_attribute	34, 1	@ Tag_CPU_unaligned_access
	.eabi_attribute	15, 1	@ Tag_ABI_PCS_RW_data
	.eabi_attribute	16, 1	@ Tag_ABI_PCS_RO_data
	.eabi_attribute	17, 2	@ Tag_ABI_PCS_GOT_use
	.eabi_attribute	20, 1	@ Tag_ABI_FP_denormal
	.eabi_attribute	21, 0	@ Tag_ABI_FP_exceptions
	.eabi_attribute	23, 3	@ Tag_ABI_FP_number_model
	.eabi_attribute	24, 1	@ Tag_ABI_align_needed
	.eabi_attribute	25, 1	@ Tag_ABI_align_preserved
	.eabi_attribute	38, 1	@ Tag_ABI_FP_16bit_format
	.eabi_attribute	18, 4	@ Tag_ABI_PCS_wchar_t
	.eabi_attribute	26, 2	@ Tag_ABI_enum_size
	.eabi_attribute	14, 0	@ Tag_ABI_PCS_R9_use
	.file	"thumbgolf.c"
	.section	.text.tgolf,"ax",%progbits
	.globl	thumbgolf_inject                @ -- Begin function thumbgolf_inject
	.p2align	2
	.type	thumbgolf_inject,%function
	.code	16                              @ @thumbgolf_inject
	.thumb_func
thumbgolf_inject:
	.fnstart
@ %bb.0:
	.save	{r4, r5, r7, lr}
	push	{r4, r5, r7, lr}
	.setfp	r7, sp, #8
	add	r7, sp, #8
	.pad	#32
	sub	sp, #32
	ldr	r0, .LCPI0_0
	movs	r1, #0
	movs	r2, #2
	movs	r3, #0
.LPC0_0:
	add	r0, pc
	ldr	r0, [r0]
	ldr	r0, [r0]
	bl	setvbuf
	ldr	r0, .LCPI0_1
	movs	r1, #0
	movs	r2, #2
	movs	r3, #0
.LPC0_1:
	add	r0, pc
	ldr	r0, [r0]
	ldr	r0, [r0]
	bl	setvbuf
	ldr	r0, .LCPI0_2
	movs	r1, #0
	movs	r2, #2
	movs	r3, #0
.LPC0_2:
	add	r0, pc
	ldr	r0, [r0]
	ldr	r0, [r0]
	bl	setvbuf
	ldr	r0, .LCPI0_3
	movs	r1, #4
	add	r4, sp, #16
	movt	r1, #22528
.LPC0_3:
	add	r0, pc
	mov	r5, sp
	str	r1, [sp, #24]
	mov	r1, r4
	str	r0, [sp, #16]
	movs	r0, #4
	mov	r2, r5
	bl	sigaction
	movs	r0, #5
	mov	r1, r4
	mov	r2, r5
	bl	sigaction
	add	sp, #32
	pop	{r4, r5, r7, pc}
	.p2align	2
@ %bb.1:
.LCPI0_0:
.Ltmp0:
	.long	stdout(GOT_PREL)-((.LPC0_0+4)-.Ltmp0)
.LCPI0_1:
.Ltmp1:
	.long	stdin(GOT_PREL)-((.LPC0_1+4)-.Ltmp1)
.LCPI0_2:
.Ltmp2:
	.long	stderr(GOT_PREL)-((.LPC0_2+4)-.Ltmp2)
.LCPI0_3:
	.long	sighandler-(.LPC0_3+4)
.Lfunc_end0:
	.size	thumbgolf_inject, .Lfunc_end0-thumbgolf_inject
	.cantunwind
	.fnend
                                        @ -- End function
	.p2align	2                               @ -- Begin function sighandler
	.type	sighandler,%function
	.code	16                              @ @sighandler
	.thumb_func
sighandler:
	.fnstart
@ %bb.0:
	.save	{r4, r5, r6, r7, lr}
	push	{r4, r5, r6, r7, lr}
	.setfp	r7, sp, #12
	add	r7, sp, #12
	.save	{r8, r9, r10, r11}
	push.w	{r8, r9, r10, r11}
	.pad	#20
	sub	sp, #20
	ldr	r1, [r2, #96]
	lsls	r0, r1, #26
	bpl.w	.LBB1_97
@ %bb.1:
	movs	r0, #252
	mov	r4, r2
	and.w	r0, r0, r1, lsr #8
	ubfx	r2, r1, #25, #2
	adds	r0, r0, r2
	bne.w	.LBB1_98
@ %bb.2:
	ldr	r0, [r4, #92]
	bic	r3, r0, #1
	ldrh	r2, [r3]
	bic	r6, r2, #255
	cmp.w	r6, #48640
	beq	.LBB1_10
@ %bb.3:
	add.w	r11, r4, #32
	cmp.w	r6, #56832
	bne	.LBB1_13
@ %bb.4:
	ubfx	r6, r2, #6, #2
.LCPI1_43:
	tbb	[pc, r6]
@ %bb.5:
.LJTI1_0:
	.byte	(.LBB1_6-(.LCPI1_43+4))/2
	.byte	(.LBB1_21-(.LCPI1_43+4))/2
	.byte	(.LBB1_18-(.LCPI1_43+4))/2
	.byte	(.LBB1_19-(.LCPI1_43+4))/2
	.p2align	1
.LBB1_6:
	ldr	r6, .LCPI1_45
	ubfx	lr, r2, #3, #3
	ldr	r3, .LCPI1_46
.LPC1_0:
	add	r6, pc
.LPC1_1:
	add	r3, pc
	ldr	r5, [r3]
	cmp	r5, #0
	beq.w	.LBB1_36
@ %bb.7:
	add.w	r1, r6, lr, lsl #3
	ldr.w	r12, [r1, #4]
	ldr	r1, .LCPI1_47
	ldr	r5, .LCPI1_48
	ldr	r6, .LCPI1_49
.LPC1_10:
	add	r1, pc
.LPC1_11:
	add	r5, pc
	mov	lr, r1
	movs	r1, #0
.LPC1_12:
	add	r6, pc
	str	r1, [r3]
	ldr.w	r1, [lr]
	ldr	r3, [r5]
	ldr	r6, [r6]
	str	r1, [r4, #96]
	strd	r3, r6, [r4, #40]
	and	r3, r12, #3
	cmp	r3, #1
	beq.w	.LBB1_43
@ %bb.8:
	cmp	r3, #0
	bne.w	.LBB1_52
@ %bb.9:
	ldr	r1, .LCPI1_50
	ldr	r2, .LCPI1_51
.LPC1_13:
	add	r1, pc
.LPC1_14:
	add	r2, pc
	ldr	r1, [r1]
	ldr	r2, [r2]
	strd	r1, r2, [r4, #32]
	b	.LBB1_52
.LBB1_10:
	movs	r1, #0
	cmp.w	r1, r2, lsl #24
	beq.w	.LBB1_99
@ %bb.11:
	lsls	r1, r2, #24
	add.w	r1, r0, r1, asr #23
	adds	r0, #2
	orr	r0, r0, #1
.LBB1_12:
	str	r0, [r4, #88]
	str	r1, [r4, #92]
	b	.LBB1_60
.LBB1_13:
	bic	r0, r2, #79
	movw	r1, #64400
	cmp	r0, r1
	bne.w	.LBB1_60
@ %bb.14:
	ldrh	r5, [r3, #2]
	and	r0, r5, #240
	cmp	r0, #240
	bne.w	.LBB1_100
@ %bb.15:
	and	r0, r5, #15
	ldr.w	r1, [r11, r0, lsl #2]
	lsls	r0, r2, #25
	bmi	.LBB1_24
@ %bb.16:
	cmp	r1, #0
	beq	.LBB1_26
@ %bb.17:
	and	r0, r2, #15
	ldr.w	r0, [r11, r0, lsl #2]
	bl	__aeabi_idiv
	b	.LBB1_27
.LBB1_18:
	and	r0, r2, #7
	movs	r1, #28
	and.w	r1, r1, r2, lsr #1
	ldr.w	r0, [r11, r0, lsl #2]
	ldr.w	r2, [r11, r1]
	ldr	r3, [r0]
	str	r2, [r0]
	str.w	r3, [r11, r1]
	b	.LBB1_57
.LBB1_19:
	ubfx	r1, r2, #3, #3
	and	r6, r2, #7
	cmp	r6, r1
	bls	.LBB1_29
@ %bb.20:
	ldr.w	r0, [r11, r1, lsl #2]
	ldr.w	r2, [r11, r6, lsl #2]
	str.w	r0, [r11, r6, lsl #2]
	str.w	r2, [r11, r1, lsl #2]
	b	.LBB1_57
.LBB1_21:
	ubfx	r3, r2, #3, #3
.LCPI1_44:
	tbh	[pc, r3, lsl #1]
@ %bb.22:
.LJTI1_1:
	.short	(.LBB1_101-(.LCPI1_44+4))/2
	.short	(.LBB1_101-(.LCPI1_44+4))/2
	.short	(.LBB1_23-(.LCPI1_44+4))/2
	.short	(.LBB1_46-(.LCPI1_44+4))/2
	.short	(.LBB1_53-(.LCPI1_44+4))/2
	.short	(.LBB1_55-(.LCPI1_44+4))/2
	.short	(.LBB1_54-(.LCPI1_44+4))/2
	.short	(.LBB1_48-(.LCPI1_44+4))/2
	.p2align	1
.LBB1_23:
	and	r0, r2, #7
	mov.w	r1, #-1
	movs	r2, #8
	str.w	r1, [r11, r0, lsl #2]
	ldrd	r0, r1, [r4, #92]
	bfi	r1, r2, #28, #4
	adds	r0, #2
	strd	r0, r1, [r4, #92]
	b	.LBB1_60
.LBB1_24:
	cbz	r1, .LBB1_26
@ %bb.25:
	and	r0, r2, #15
	ldr.w	r0, [r11, r0, lsl #2]
	bl	__aeabi_uidiv
	b	.LBB1_27
.LBB1_26:
	movs	r0, #0
.LBB1_27:
	ubfx	r1, r5, #8, #4
	str.w	r0, [r11, r1, lsl #2]
.LBB1_28:
	ldr	r0, [r4, #92]
	adds	r0, #4
	b	.LBB1_59
.LBB1_29:
	ldrh	r1, [r3, #2]
	and	r2, r2, #63
	cmp	r2, #1
	beq	.LBB1_38
@ %bb.30:
	cmp	r2, #0
	bne.w	.LBB1_100
@ %bb.31:
	and	r2, r1, #15
	ubfx	r0, r1, #4, #4
	ubfx	r5, r1, #8, #4
	movs	r3, #15
	cmp.w	r3, r1, lsr #12
	beq.w	.LBB1_61
@ %bb.32:
	lsr.w	r10, r1, #12
	cmp.w	r10, #13
	bne.w	.LBB1_73
@ %bb.33:
	and	r0, r1, #3328
	cmp.w	r0, #3328
	beq.w	.LBB1_89
@ %bb.34:
	uxtb	r1, r1
	cmp	r1, #0
	beq.w	.LBB1_102
@ %bb.35:
	ldr.w	r0, [r11, r5, lsl #2]
	bl	__aeabi_uidivmod
	b	.LBB1_63
.LBB1_36:
	ldr.w	r6, [r6, lr, lsl #3]
	mov.w	r9, #1
	ldr	r5, [r4, #40]
	str	r6, [sp, #16]                   @ 4-byte Spill
	ldrd	r12, r10, [r4, #32]
	str	r5, [sp, #8]                    @ 4-byte Spill
	ldr	r5, [r4, #44]
	str	r5, [sp, #4]                    @ 4-byte Spill
	ldr	r5, [r4, #88]
	str	r5, [sp, #12]                   @ 4-byte Spill
	ldr	r5, [r4, #80]
	str	r5, [sp]                        @ 4-byte Spill
	ldr	r5, .LCPI1_2
	str.w	r9, [r3]
.LPC1_2:
	add	r5, pc
	ldr	r3, .LCPI1_3
	ldr	r6, .LCPI1_4
	str.w	r12, [r5]
.LPC1_3:
	add	r3, pc
	ldr	r5, .LCPI1_5
.LPC1_4:
	add	r6, pc
.LPC1_5:
	add	r5, pc
	mov	r9, r6
	mov	r12, r5
	ldr	r5, .LCPI1_6
	str.w	r10, [r3]
.LPC1_6:
	add	r5, pc
	ldr	r3, .LCPI1_7
	mov	r8, r5
	ldr	r5, .LCPI1_8
	ldr	r6, [sp, #8]                    @ 4-byte Reload
.LPC1_7:
	add	r3, pc
	str.w	r6, [r9]
.LPC1_8:
	add	r5, pc
	ldr	r6, [sp, #4]                    @ 4-byte Reload
	str.w	r6, [r12]
	ldr	r6, [sp]                        @ 4-byte Reload
	str.w	r6, [r8]
	ldr	r6, [sp, #12]                   @ 4-byte Reload
	str	r6, [r3]
	str	r1, [r5]
	movs	r1, #234
	lsr.w	r1, r1, lr
	lsls	r1, r1, #31
	bne	.LBB1_44
@ %bb.37:
	and	r1, r2, #7
	ldr.w	r2, [r11, r1, lsl #2]
	str.w	r2, [r11]
	b	.LBB1_45
.LBB1_38:
	adds	r0, #4
	str	r0, [r4, #92]
	and	r2, r1, #15
	ubfx	r0, r1, #4, #4
	ubfx	r5, r1, #8, #4
	movs	r3, #15
	cmp.w	r3, r1, lsr #12
	beq.w	.LBB1_64
@ %bb.39:
	lsr.w	r9, r1, #12
	cmp.w	r9, #13
	bne.w	.LBB1_77
@ %bb.40:
	and	r0, r1, #3328
	cmp.w	r0, #3328
	beq.w	.LBB1_89
@ %bb.41:
	uxtb	r1, r1
	cmp	r1, #0
	beq.w	.LBB1_103
@ %bb.42:
	ldr.w	r0, [r11, r5, lsl #2]
	bl	__aeabi_idivmod
	str.w	r1, [r11, r5, lsl #2]
	b	.LBB1_60
.LBB1_43:
	ldr	r3, .LCPI1_15
	bic	r1, r1, #1073741824
	ldr	r6, [r4, #36]
.LPC1_15:
	add	r3, pc
	cmp	r6, #0
	ldr	r3, [r3]
	it	ne
	addne.w	r1, r1, #1073741824
	str	r1, [r4, #96]
	str	r3, [r4, #36]
	b	.LBB1_50
.LBB1_44:
	cmp.w	lr, #7
	it	eq
	streq.w	r2, [r11]
.LBB1_45:
	ldr	r1, .LCPI1_9
	orr	r0, r0, #1
	ldr	r2, [sp, #16]                   @ 4-byte Reload
.LPC1_9:
	add	r1, pc
	str	r2, [r4, #80]
	bic	r1, r1, #1
	b	.LBB1_12
.LBB1_46:
	and	r1, r2, #7
	ldr.w	r2, [r11, r1, lsl #2]
	cmp.w	r2, #-1
	bgt	.LBB1_58
@ %bb.47:
	rsbs	r0, r2, #0
	str.w	r0, [r11, r1, lsl #2]
	b	.LBB1_57
.LBB1_48:
	ldr	r3, .LCPI1_20
.LPC1_20:
	add	r3, pc
	ldr	r6, [r3]
	cmp	r6, #0
	beq.w	.LBB1_81
@ %bb.49:
	ldr	r1, .LCPI1_30
	ldr	r5, .LCPI1_31
	ldr	r6, .LCPI1_32
.LPC1_30:
	add	r1, pc
.LPC1_31:
	add	r5, pc
.LPC1_32:
	add	r6, pc
	mov	lr, r6
	movs	r6, #0
	str	r6, [r3]
	ldr.w	r12, [r1]
	ldr	r3, [r5]
	ldr	r5, .LCPI1_33
	ldr.w	r6, [lr]
.LPC1_33:
	add	r5, pc
	ldr	r1, [r4, #36]
	ldr	r5, [r5]
	cmp	r1, #0
	strd	r5, r3, [r4, #36]
	bic	r3, r12, #1073741824
	str	r6, [r4, #44]
	it	ne
	addne.w	r3, r3, #1073741824
	str	r3, [r4, #96]
.LBB1_50:
	ands	r1, r2, #7
	beq	.LBB1_52
@ %bb.51:
	ldr	r2, .LCPI1_16
	ldr	r0, [r4, #32]
	str.w	r0, [r11, r1, lsl #2]
.LPC1_16:
	add	r2, pc
	ldr	r0, [r4, #92]
	ldr	r1, [r2]
	str	r1, [r4, #32]
.LBB1_52:
	ldr	r1, .LCPI1_17
	adds	r0, #2
	ldr	r2, .LCPI1_18
.LPC1_17:
	add	r1, pc
.LPC1_18:
	add	r2, pc
	ldr	r1, [r1]
	ldr	r2, [r2]
	str	r1, [r4, #80]
	str	r2, [r4, #88]
	b	.LBB1_59
.LBB1_53:
	and	r0, r2, #7
	movw	r2, #52429
	movt	r2, #52428
	ldr.w	r1, [r11, r0, lsl #2]
	umull	r1, r2, r1, r2
	lsrs	r1, r2, #3
	b	.LBB1_56
.LBB1_54:
	and	r0, r2, #7
	ldr.w	r1, [r11, r0, lsl #2]
	add.w	r1, r1, r1, lsl #2
	lsls	r1, r1, #1
	b	.LBB1_56
.LBB1_55:
	and	r0, r2, #7
	movw	r2, #52429
	movt	r2, #52428
	ldr.w	r1, [r11, r0, lsl #2]
	umull	r2, r3, r1, r2
	lsrs	r2, r3, #3
	add.w	r2, r2, r2, lsl #2
	sub.w	r1, r1, r2, lsl #1
.LBB1_56:
	str.w	r1, [r11, r0, lsl #2]
.LBB1_57:
	ldr	r0, [r4, #92]
.LBB1_58:
	adds	r0, #2
.LBB1_59:
	str	r0, [r4, #92]
.LBB1_60:
	add	sp, #20
	pop.w	{r8, r9, r10, r11}
	pop	{r4, r5, r6, r7, pc}
.LBB1_61:
	and	r1, r1, #3328
	cmp.w	r1, #3328
	beq.w	.LBB1_89
@ %bb.62:
	ldr.w	r1, [r11, r2, lsl #2]
	ldr.w	r0, [r11, r0, lsl #2]
	cmp	r0, r1
	it	hi
	movhi	r1, r0
.LBB1_63:
	str.w	r1, [r11, r5, lsl #2]
	b	.LBB1_28
.LBB1_64:
	and	r1, r1, #3328
	cmp.w	r1, #3328
	beq.w	.LBB1_89
@ %bb.65:
	ldr.w	r1, [r11, r2, lsl #2]
	ldr.w	r0, [r11, r0, lsl #2]
	cmp	r0, r1
	it	gt
	movgt	r1, r0
	str.w	r1, [r11, r5, lsl #2]
	b	.LBB1_60
	.p2align	2
@ %bb.66:
.LCPI1_45:
	.long	io_funcs-(.LPC1_0+4)
	.p2align	2
@ %bb.67:
.LCPI1_46:
	.long	in_libc_call-(.LPC1_1+4)
	.p2align	2
@ %bb.68:
.LCPI1_47:
	.long	old_cpsr-(.LPC1_10+4)
	.p2align	2
@ %bb.69:
.LCPI1_48:
	.long	old_r2-(.LPC1_11+4)
	.p2align	2
@ %bb.70:
.LCPI1_49:
	.long	old_r3-(.LPC1_12+4)
	.p2align	2
@ %bb.71:
.LCPI1_50:
	.long	old_r0-(.LPC1_13+4)
	.p2align	2
@ %bb.72:
.LCPI1_51:
	.long	old_r1-(.LPC1_14+4)
	.p2align	1
.LBB1_73:
	cmp	r5, #15
	beq	.LBB1_82
@ %bb.74:
	cmp	r5, #13
	bne	.LBB1_84
@ %bb.75:
	uxtb	r1, r1
	cmp	r1, #0
	beq.w	.LBB1_104
@ %bb.76:
	ldr.w	r0, [r11, r10, lsl #2]
	bl	__aeabi_uidiv
	str.w	r0, [r11, r10, lsl #2]
	b	.LBB1_28
.LBB1_77:
	cmp	r5, #15
	beq	.LBB1_83
@ %bb.78:
	cmp	r5, #13
	bne	.LBB1_88
@ %bb.79:
	uxtb	r1, r1
	cmp	r1, #0
	beq.w	.LBB1_105
@ %bb.80:
	ldr.w	r0, [r11, r9, lsl #2]
	bl	__aeabi_idiv
	str.w	r0, [r11, r9, lsl #2]
	b	.LBB1_60
.LBB1_81:
	ldr	r2, .LCPI1_21
	add.w	lr, r4, #32
	ldr	r5, .LCPI1_22
	orr	r0, r0, #1
	ldm.w	lr, {r6, r12, lr}
.LPC1_21:
	add	r2, pc
.LPC1_22:
	add	r5, pc
	ldr.w	r8, [r4, #44]
	mov	r10, r5
	movs	r5, #1
	str	r5, [r3]
	ldr	r3, .LCPI1_23
	ldr	r5, .LCPI1_24
	str	r6, [r2]
.LPC1_23:
	add	r3, pc
.LPC1_24:
	add	r5, pc
	ldr	r2, .LCPI1_25
	ldr.w	r9, [r4, #80]
	str.w	r12, [r10]
	mov	r11, r5
	ldr	r5, .LCPI1_26
.LPC1_25:
	add	r2, pc
	ldr	r6, .LCPI1_27
	str.w	lr, [r3]
.LPC1_26:
	add	r5, pc
.LPC1_27:
	add	r6, pc
	ldr	r3, .LCPI1_28
	str	r2, [r4, #80]
	ldr	r2, [r4, #88]
	mov	r12, r6
	str.w	r8, [r11]
.LPC1_28:
	add	r3, pc
	ldr	r6, .LCPI1_29
	bic	r3, r3, #1
	str.w	r9, [r5]
.LPC1_29:
	add	r6, pc
	strd	r0, r3, [r4, #88]
	str.w	r2, [r12]
	str	r1, [r6]
	b	.LBB1_60
.LBB1_82:
	ldr.w	r1, [r11, r2, lsl #2]
	ldr.w	r0, [r11, r0, lsl #2]
	cmp	r0, r1
	it	lo
	movlo	r1, r0
	str.w	r1, [r11, r10, lsl #2]
	b	.LBB1_28
.LBB1_83:
	ldr.w	r1, [r11, r2, lsl #2]
	ldr.w	r0, [r11, r0, lsl #2]
	cmp	r0, r1
	it	lt
	movlt	r1, r0
	str	r1, [r4, #92]
	b	.LBB1_60
.LBB1_84:
	and	r3, r1, #208
	cmp	r3, #208
	itt	ne
	andne	r1, r1, #13
	cmpne	r1, #13
	beq	.LBB1_89
@ %bb.85:
	cmp	r0, r2
	beq	.LBB1_106
@ %bb.86:
	ldr.w	r9, [r11, r2, lsl #2]
	cmp.w	r9, #0
	beq	.LBB1_93
@ %bb.87:
	ldr.w	r8, [r11, r0, lsl #2]
	mov	r1, r9
	mov	r0, r8
	bl	__aeabi_uidiv
	mls	r1, r0, r9, r8
	b	.LBB1_94
.LBB1_88:
	and	r3, r1, #208
	cmp	r3, #208
	itt	ne
	andne	r1, r1, #13
	cmpne	r1, #13
	bne	.LBB1_90
.LBB1_89:
	ldr	r0, .LCPI1_36
.LPC1_36:
	add	r0, pc
	bl	die
.LBB1_90:
	cmp	r0, r2
	beq	.LBB1_107
@ %bb.91:
	ldr.w	r4, [r11, r2, lsl #2]
	cbz	r4, .LBB1_95
@ %bb.92:
	ldr.w	r8, [r11, r0, lsl #2]
	mov	r1, r4
	mov	r0, r8
	bl	__aeabi_idiv
	mls	r1, r0, r4, r8
	b	.LBB1_96
.LBB1_93:
	movs	r0, #0
	movs	r1, #0
.LBB1_94:
	str.w	r0, [r11, r10, lsl #2]
	b	.LBB1_63
.LBB1_95:
	movs	r0, #0
	movs	r1, #0
.LBB1_96:
	str.w	r0, [r11, r9, lsl #2]
	str.w	r1, [r11, r5, lsl #2]
	b	.LBB1_60
.LBB1_97:
	ldr	r0, .LCPI1_34
.LPC1_34:
	add	r0, pc
	bl	die
.LBB1_98:
	ldr	r0, .LCPI1_35
.LPC1_35:
	add	r0, pc
	bl	die
.LBB1_99:
	movs	r0, #0
	bl	exit
.LBB1_100:
	bl	illegal
.LBB1_101:
	ldr	r0, .LCPI1_19
.LPC1_19:
	add	r0, pc
	bl	die
.LBB1_102:
	ldr	r0, .LCPI1_37
.LPC1_37:
	add	r0, pc
	bl	die
.LBB1_103:
	ldr	r0, .LCPI1_40
.LPC1_40:
	add	r0, pc
	bl	die
.LBB1_104:
	ldr	r0, .LCPI1_38
.LPC1_38:
	add	r0, pc
	bl	die
.LBB1_105:
	ldr	r0, .LCPI1_41
.LPC1_41:
	add	r0, pc
	bl	die
.LBB1_106:
	ldr	r0, .LCPI1_39
.LPC1_39:
	add	r0, pc
	bl	die
.LBB1_107:
	ldr	r0, .LCPI1_42
.LPC1_42:
	add	r0, pc
	bl	die
	.p2align	2
@ %bb.108:
.LCPI1_2:
	.long	old_r0-(.LPC1_2+4)
.LCPI1_3:
	.long	old_r1-(.LPC1_3+4)
.LCPI1_4:
	.long	old_r2-(.LPC1_4+4)
.LCPI1_5:
	.long	old_r3-(.LPC1_5+4)
.LCPI1_6:
	.long	old_r12-(.LPC1_6+4)
.LCPI1_7:
	.long	old_lr-(.LPC1_7+4)
.LCPI1_8:
	.long	old_cpsr-(.LPC1_8+4)
.LCPI1_9:
	.long	do_veneer-(.LPC1_9+4)
.LCPI1_15:
	.long	old_r1-(.LPC1_15+4)
.LCPI1_16:
	.long	old_r0-(.LPC1_16+4)
.LCPI1_17:
	.long	old_r12-(.LPC1_17+4)
.LCPI1_18:
	.long	old_lr-(.LPC1_18+4)
.LCPI1_19:
	.long	.L.str.1-(.LPC1_19+4)
.LCPI1_20:
	.long	in_libc_call-(.LPC1_20+4)
.LCPI1_21:
	.long	old_r0-(.LPC1_21+4)
.LCPI1_22:
	.long	old_r1-(.LPC1_22+4)
.LCPI1_23:
	.long	old_r2-(.LPC1_23+4)
.LCPI1_24:
	.long	old_r3-(.LPC1_24+4)
.LCPI1_25:
	.long	get_rand-(.LPC1_25+4)
.LCPI1_26:
	.long	old_r12-(.LPC1_26+4)
.LCPI1_27:
	.long	old_lr-(.LPC1_27+4)
.LCPI1_28:
	.long	do_veneer-(.LPC1_28+4)
.LCPI1_29:
	.long	old_cpsr-(.LPC1_29+4)
.LCPI1_30:
	.long	old_cpsr-(.LPC1_30+4)
.LCPI1_31:
	.long	old_r2-(.LPC1_31+4)
.LCPI1_32:
	.long	old_r3-(.LPC1_32+4)
.LCPI1_33:
	.long	old_r1-(.LPC1_33+4)
.LCPI1_34:
	.long	.L.str.2-(.LPC1_34+4)
.LCPI1_35:
	.long	.L.str-(.LPC1_35+4)
.LCPI1_36:
	.long	.L.str.12-(.LPC1_36+4)
.LCPI1_37:
	.long	.L.str.9-(.LPC1_37+4)
.LCPI1_38:
	.long	.L.str.10-(.LPC1_38+4)
.LCPI1_39:
	.long	.L.str.11-(.LPC1_39+4)
.LCPI1_40:
	.long	.L.str.13-(.LPC1_40+4)
.LCPI1_41:
	.long	.L.str.14-(.LPC1_41+4)
.LCPI1_42:
	.long	.L.str.15-(.LPC1_42+4)
.Lfunc_end1:
	.size	sighandler, .Lfunc_end1-sighandler
	.cantunwind
	.fnend
                                        @ -- End function
	.p2align	1                               @ -- Begin function die
	.type	die,%function
	.code	16                              @ @die
	.thumb_func
die:
	.fnstart
@ %bb.0:
	.save	{r7, lr}
	push	{r7, lr}
	.setfp	r7, sp
	mov	r7, sp
	mov	r4, r0
	bl	strlen
	mov	r2, r0
	movs	r0, #2
	mov	r1, r4
	bl	write
	bl	illegal
.Lfunc_end2:
	.size	die, .Lfunc_end2-die
	.cantunwind
	.fnend
                                        @ -- End function
	.p2align	2                               @ -- Begin function get_rand
	.type	get_rand,%function
	.code	16                              @ @get_rand
	.thumb_func
get_rand:
	.fnstart
@ %bb.0:
	.save	{r4, r6, r7, lr}
	push	{r4, r6, r7, lr}
	.setfp	r7, sp, #8
	add	r7, sp, #8
	.pad	#8
	sub	sp, #8
	ldr	r0, .LCPI3_0
	movs	r2, #0
	ldr	r1, .LCPI3_1
.LPC3_0:
	add	r0, pc
	str	r2, [sp, #4]
.LPC3_1:
	add	r1, pc
	bl	fopen
	mov	r4, r0
	add	r0, sp, #4
	movs	r1, #4
	movs	r2, #1
	mov	r3, r4
	bl	fread
	mov	r0, r4
	bl	fclose
	ldr	r0, [sp, #4]
	clz	r1, r0
	lsrs	r1, r1, #5
	add	sp, #8
	pop	{r4, r6, r7, pc}
	.p2align	2
@ %bb.1:
.LCPI3_0:
	.long	.L.str.7-(.LPC3_0+4)
.LCPI3_1:
	.long	.L.str.8-(.LPC3_1+4)
.Lfunc_end3:
	.size	get_rand, .Lfunc_end3-get_rand
	.cantunwind
	.fnend
                                        @ -- End function
	.p2align	1                               @ -- Begin function illegal
	.type	illegal,%function
	.code	16                              @ @illegal
	.thumb_func
illegal:
	.fnstart
@ %bb.0:
	.save	{r7, lr}
	push	{r7, lr}
	.setfp	r7, sp
	mov	r7, sp
	movs	r0, #4
	movs	r1, #0
	bl	signal
	movs	r0, #5
	movs	r1, #0
	bl	signal
	movs	r0, #4
	bl	raise
	movs	r0, #132
	bl	exit
.Lfunc_end4:
	.size	illegal, .Lfunc_end4-illegal
	.cantunwind
	.fnend
                                        @ -- End function
	.p2align	1                               @ -- Begin function do_veneer
	.type	do_veneer,%function
	.code	16                              @ @do_veneer
	.thumb_func
do_veneer:
	.fnstart
@ %bb.0:
	@APP
	bx	r12

	@NO_APP
.Lfunc_end5:
	.size	do_veneer, .Lfunc_end5-do_veneer
	.cantunwind
	.fnend
                                        @ -- End function
	.p2align	1                               @ -- Begin function put_str_n
	.type	put_str_n,%function
	.code	16                              @ @put_str_n
	.thumb_func
put_str_n:
	.fnstart
@ %bb.0:
	b	puts
.Lfunc_end6:
	.size	put_str_n, .Lfunc_end6-put_str_n
	.cantunwind
	.fnend
                                        @ -- End function
	.p2align	2                               @ -- Begin function get_str
	.type	get_str,%function
	.code	16                              @ @get_str
	.thumb_func
get_str:
	.fnstart
@ %bb.0:
	.save	{r4, r6, r7, lr}
	push	{r4, r6, r7, lr}
	.setfp	r7, sp, #8
	add	r7, sp, #8
	ldr	r4, .LCPI7_0
.LPC7_0:
	add	r4, pc
	ldr	r0, [r4]
	bl	free
	ldr	r0, .LCPI7_1
	movs	r1, #0
	str	r1, [r4]
	mov	r1, r4
.LPC7_1:
	add	r0, pc
	bl	scanf
	subs	r1, r0, #1
	ldr	r0, [r4]
	clz	r1, r1
	lsrs	r1, r1, #5
	pop	{r4, r6, r7, pc}
	.p2align	2
@ %bb.1:
.LCPI7_0:
	.long	scanf_buffer-(.LPC7_0+4)
.LCPI7_1:
	.long	.L.str.3-(.LPC7_1+4)
.Lfunc_end7:
	.size	get_str, .Lfunc_end7-get_str
	.cantunwind
	.fnend
                                        @ -- End function
	.p2align	1                               @ -- Begin function put_char
	.type	put_char,%function
	.code	16                              @ @put_char
	.thumb_func
put_char:
	.fnstart
@ %bb.0:
	b	putchar
.Lfunc_end8:
	.size	put_char, .Lfunc_end8-put_char
	.cantunwind
	.fnend
                                        @ -- End function
	.p2align	1                               @ -- Begin function get_char
	.type	get_char,%function
	.code	16                              @ @get_char
	.thumb_func
get_char:
	.fnstart
@ %bb.0:
	.save	{r7, lr}
	push	{r7, lr}
	.setfp	r7, sp
	mov	r7, sp
	bl	getchar
	subs.w	r1, r0, #-1
	mov.w	r3, #1
	mov	r2, r1
	it	ne
	movne	r2, r0
	it	ne
	orrne.w	r1, r3, r0, asr #31
	mov	r0, r2
	pop	{r7, pc}
.Lfunc_end9:
	.size	get_char, .Lfunc_end9-get_char
	.cantunwind
	.fnend
                                        @ -- End function
	.p2align	2                               @ -- Begin function put_int_n
	.type	put_int_n,%function
	.code	16                              @ @put_int_n
	.thumb_func
put_int_n:
	.fnstart
@ %bb.0:
	mov	r1, r0
	ldr	r0, .LCPI10_0
.LPC10_0:
	add	r0, pc
	b	printf
	.p2align	2
@ %bb.1:
.LCPI10_0:
	.long	.L.str.4-(.LPC10_0+4)
.Lfunc_end10:
	.size	put_int_n, .Lfunc_end10-put_int_n
	.cantunwind
	.fnend
                                        @ -- End function
	.p2align	2                               @ -- Begin function get_int
	.type	get_int,%function
	.code	16                              @ @get_int
	.thumb_func
get_int:
	.fnstart
@ %bb.0:
	.save	{r4, r6, r7, lr}
	push	{r4, r6, r7, lr}
	.setfp	r7, sp, #8
	add	r7, sp, #8
	.pad	#8
	sub	sp, #8
	ldr	r0, .LCPI11_0
	add	r1, sp, #4
.LPC11_0:
	add	r0, pc
	bl	scanf
	cmp	r0, #1
	bne	.LBB11_2
@ %bb.1:
	movs	r1, #1
	movs	r0, #0
	b	.LBB11_7
.LBB11_2:                               @ =>This Inner Loop Header: Depth=1
	bl	getchar
	mov	r4, r0
	adds	r0, #1
	beq	.LBB11_4
@ %bb.3:                                @   in Loop: Header=BB11_2 Depth=1
	mov	r0, r4
	bl	isspace
	cmp	r0, #0
	beq	.LBB11_2
.LBB11_4:
	mov	r0, r4
	bl	isspace
	cbz	r0, .LBB11_6
@ %bb.5:
	ldr	r0, .LCPI11_1
.LPC11_1:
	add	r0, pc
	ldr	r0, [r0]
	ldr	r1, [r0]
	mov	r0, r4
	bl	ungetc
.LBB11_6:
	movs	r0, #0
	movs	r1, #0
.LBB11_7:
	ldr	r2, [sp, #4]
	orrs	r0, r2
	add	sp, #8
	pop	{r4, r6, r7, pc}
	.p2align	2
@ %bb.8:
.LCPI11_0:
	.long	.L.str.5-(.LPC11_0+4)
.LCPI11_1:
.Ltmp3:
	.long	stdin(GOT_PREL)-((.LPC11_1+4)-.Ltmp3)
.Lfunc_end11:
	.size	get_int, .Lfunc_end11-get_int
	.cantunwind
	.fnend
                                        @ -- End function
	.p2align	2                               @ -- Begin function get_word
	.type	get_word,%function
	.code	16                              @ @get_word
	.thumb_func
get_word:
	.fnstart
@ %bb.0:
	.save	{r4, r6, r7, lr}
	push	{r4, r6, r7, lr}
	.setfp	r7, sp, #8
	add	r7, sp, #8
	ldr	r4, .LCPI12_0
.LPC12_0:
	add	r4, pc
	ldr	r0, [r4]
	bl	free
	ldr	r0, .LCPI12_1
	movs	r1, #0
	str	r1, [r4]
	mov	r1, r4
.LPC12_1:
	add	r0, pc
	bl	scanf
	subs	r1, r0, #1
	ldr	r0, [r4]
	clz	r1, r1
	lsrs	r1, r1, #5
	pop	{r4, r6, r7, pc}
	.p2align	2
@ %bb.1:
.LCPI12_0:
	.long	scanf_buffer-(.LPC12_0+4)
.LCPI12_1:
	.long	.L.str.6-(.LPC12_1+4)
.Lfunc_end12:
	.size	get_word, .Lfunc_end12-get_word
	.cantunwind
	.fnend
                                        @ -- End function
	.p2align	2                               @ -- Begin function put_special
	.type	put_special,%function
	.code	16                              @ @put_special
	.thumb_func
put_special:
	.fnstart
@ %bb.0:
	ldr	r1, .LCPI13_0
	and	r0, r0, #7
.LPC13_0:
	add	r1, pc
	ldrb	r0, [r1, r0]
	b	putchar
	.p2align	2
@ %bb.1:
.LCPI13_0:
	.long	special_chars-(.LPC13_0+4)
.Lfunc_end13:
	.size	put_special, .Lfunc_end13-put_special
	.cantunwind
	.fnend
                                        @ -- End function
	.type	.L.str,%object                  @ @.str
	.section	.rodata.str1.1,"aMS",%progbits,1
.L.str:
	.asciz	"ThumbGolf instructions are not supported in IT blocks (yet).\n"
	.size	.L.str, 62

	.type	io_funcs,%object                @ @io_funcs
	.section	.rodata.tgolf,"aw",%progbits
	.p2align	2
io_funcs:
	.long	put_str_n
	.long	4                               @ 0x4
	.long	get_str
	.long	1                               @ 0x1
	.long	put_char
	.long	4                               @ 0x4
	.long	get_char
	.long	1                               @ 0x1
	.long	put_int_n
	.long	4                               @ 0x4
	.long	get_int
	.long	1                               @ 0x1
	.long	get_word
	.long	1                               @ 0x1
	.long	put_special
	.long	8                               @ 0x8
	.size	io_funcs, 64

	.type	.L.str.1,%object                @ @.str.1
	.section	.rodata.str1.1,"aMS",%progbits,1
.L.str.1:
	.asciz	"adcs/sbcs are coming soon.\n"
	.size	.L.str.1, 28

	.type	.L.str.2,%object                @ @.str.2
.L.str.2:
	.asciz	"This IS called ThumbGolf for a reason, ya know?\nDid you forget to compile your program in Thumb mode?\n"
	.size	.L.str.2, 103

	.type	in_libc_call,%object            @ @in_libc_call
	.section	.bss.tgolf,"aw",%nobits
	.p2align	2
in_libc_call:
	.long	0                               @ 0x0
	.size	in_libc_call, 4

	.type	old_r0,%object                  @ @old_r0
	.p2align	2
old_r0:
	.long	0                               @ 0x0
	.size	old_r0, 4

	.type	old_r1,%object                  @ @old_r1
	.p2align	2
old_r1:
	.long	0                               @ 0x0
	.size	old_r1, 4

	.type	old_r2,%object                  @ @old_r2
	.p2align	2
old_r2:
	.long	0                               @ 0x0
	.size	old_r2, 4

	.type	old_r3,%object                  @ @old_r3
	.p2align	2
old_r3:
	.long	0                               @ 0x0
	.size	old_r3, 4

	.type	old_r12,%object                 @ @old_r12
	.p2align	2
old_r12:
	.long	0                               @ 0x0
	.size	old_r12, 4

	.type	old_lr,%object                  @ @old_lr
	.p2align	2
old_lr:
	.long	0                               @ 0x0
	.size	old_lr, 4

	.type	old_cpsr,%object                @ @old_cpsr
	.p2align	2
old_cpsr:
	.long	0                               @ 0x0
	.size	old_cpsr, 4

	.type	scanf_buffer,%object            @ @scanf_buffer
	.p2align	2
scanf_buffer:
	.long	0
	.size	scanf_buffer, 4

	.type	.L.str.3,%object                @ @.str.3
	.section	.rodata.str1.1,"aMS",%progbits,1
.L.str.3:
	.asciz	"%m[^\n]"
	.size	.L.str.3, 7

	.type	.L.str.4,%object                @ @.str.4
.L.str.4:
	.asciz	"%i\n"
	.size	.L.str.4, 4

	.type	.L.str.5,%object                @ @.str.5
.L.str.5:
	.asciz	"%i"
	.size	.L.str.5, 3

	.type	.L.str.6,%object                @ @.str.6
.L.str.6:
	.asciz	"%ms"
	.size	.L.str.6, 4

	.type	special_chars,%object           @ @special_chars
	.section	.rodata.tgolf,"aM",%progbits,8,unique,1
special_chars:
	.ascii	"\000\t\n ,{}\""
	.size	special_chars, 8

	.type	.L.str.7,%object                @ @.str.7
	.section	.rodata.str1.1,"aMS",%progbits,1
.L.str.7:
	.asciz	"/dev/urandom"
	.size	.L.str.7, 13

	.type	.L.str.8,%object                @ @.str.8
.L.str.8:
	.asciz	"rb"
	.size	.L.str.8, 3

	.type	.L.str.9,%object                @ @.str.9
.L.str.9:
	.asciz	"umod with imm8 == 0 is reserved!\n"
	.size	.L.str.9, 34

	.type	.L.str.10,%object               @ @.str.10
.L.str.10:
	.asciz	"udiv with imm8 == 0 is reserved!\n"
	.size	.L.str.10, 34

	.type	.L.str.11,%object               @ @.str.11
.L.str.11:
	.asciz	"udivm a, b, c, c is reserved!\n"
	.size	.L.str.11, 31

	.type	.L.str.12,%object               @ @.str.12
.L.str.12:
	.asciz	"Use of SP and PC are not allowed."
	.size	.L.str.12, 34

	.type	.L.str.13,%object               @ @.str.13
.L.str.13:
	.asciz	"smod with imm8 == 0 is reserved!\n"
	.size	.L.str.13, 34

	.type	.L.str.14,%object               @ @.str.14
.L.str.14:
	.asciz	"sdiv with imm8 == 0 is reserved!\n"
	.size	.L.str.14, 34

	.type	.L.str.15,%object               @ @.str.15
.L.str.15:
	.asciz	"sdivm a, b, c, c is reserved!\n"
	.size	.L.str.15, 31

	.section	.init_array,"aw",%init_array
	.p2align	2
	.long	thumbgolf_inject(target1)
	.ident	"clang version 11.0.0 (https://github.com/termux/termux-packages 39dec01e591687a324c84205de6c9713165c4802)"
	.section	".note.GNU-stack","",%progbits
