	.include "../thumbgolf.inc"
	.globl main
	.thumb
	.thumb_func
	// Technically the same size as the normal libc one :P
main:
	adr	r0, .Lstr
	puts	r0
	bx	lr
.Lstr:
	.asciz "Hello, World!"
