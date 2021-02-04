	.include "../thumbgolf.inc"
	.globl main
	.thumb_func
main:
	geti	r0
	muls	r0, r0
	puti	r0
	bx	lr
