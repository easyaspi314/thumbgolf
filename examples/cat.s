	.include "thumbgolf.inc"
.Lstart:
	putc	r0
	// fallthrough
	.globl main
	.thumb_func
main:
	getc	r0
	beq	.Lstart
	bx	lr
