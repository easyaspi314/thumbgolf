	.include "../thumbgolf.inc"
	.globl main
	.thumb_func
main:
	movs	r4, #0
.Lloop:
	adds	r4, #1
.Lthree:
	// r0 = r4 % 3
	movs	r0, r4
	umodi	r0, 3	// udf #0300; .short 0xd003
	// if r0 == 0
	cbnz	r0, .Lten
	// if so print "Fizz"
	adr	r5, .Lfizz
	puts	r5	// udf #0005
.Lten:
	// do the same for Buzz
	// r6 = (r4 * 2) % 10, a.k.a. r4 % 5
	// we do this because umod.10 is a special narrow instruction :)
	lsls	r6, r4, #1
	umod.10	r6	// udf #0156
	cbnz	r6, .Lnum
	adr	r1, .Lbuzz
	puts	r1	// udf #0001
	// jump to the newline
	b	.Lnoprint

	// Normally, "Fizz" and "Buzz" are 5 byte strings due to the null
	// terminator, which SUCKS. This is because Thumb instructions MUST
	// be 2 byte aligned, and pool loads that are not 4 byte aligned are
	// annoying wide instructions.
	//
	// However, I have a trick up my sleeve.
	//
	// CBZ is encoded as so:
	//    |15 14 13 12|11|10| 9| 8| 7  6  5  4  3 |2  1  0|
	//    | 1  0  1  1| 0| 0| i| 1|      imm5     |  Rn   |
	// Let's focus on bits 0-7, which, since ARM is little endian, appear
	// first.
	//
	// imm5 is the offset in opcodes relative to 4 bytes after the current
	// instruction (a.k.a. where the PC is for dumb reasons). Technically,
	// there is a 6th bit in bit 9, but that is only for larger offsets.
	//
	// .Lnoprint is exactly 4 bytes after the instruction, so the offset
	// is zero:
	//   0 0 0 0 0 r r r
	// Now, for the register argument, if we set it to r0...
	//   0 0 0 0 0 0 0 0
	// Behold. A free null terminator.
	//
	// This is great, as it not only lets us cheat a byte in Fizz, but also,
	// it makes everything align perfectly so we don't need any wide adr
	// instructions. We can just put Buzz at the bottom of the file and it
	// will be 4 byte aligned as well.
.Lfizz:
	.ascii	"Fizz"	// null terminated by cbz encoding
.Lnum:
	// was r4 % 3 not zero?
	cbz	r0, .Lnoprint
	// if so, print r4 as integer
	puti	r4	// udf #0044
.Lnoprint:
	// print newline
	putspc	'\n'	// udf #0072
	// loop while r4 != 100
	// replace with a register for variable length
	cmp	r4, #100
	bne	.Lloop
	// return
	bx	lr
.Lbuzz:
	.asciz	"Buzz"
