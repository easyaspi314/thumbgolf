# ThumbGolf

ThumbGolf is an extension to the Thumb-2 instruction set for golfing.

It isn't a partial implementation of Thumb, it is a **true superset** of ARMv7-A
that runs on actual hardware (or QEMU). It uses `sigaction` to trap on `udf`
and `bkpt`, using the 8-bit immediate value, as well as an optional second
16-bit trail, to encode its instructions. It is both ugly and beautiful at the
same time.

**It should go without saying, but please do not use this for anything serious.**

I may decide to also use `udf.w`, but according to the ARM Architecture Reference
Manual, we only have 12 bits, since 4 of the immediate bits must be `0b1111`.

If Jelly and Pyth are like toolsheds, ThumbGolf is a Swiss Army knife (and no, not
[this kind](https://stylesdaddy.com/wp-content/uploads/2019/11/Wenger-Swiss-Army-Knife-Giant.jpg)).
It gives you tools to let you focus on the algorithm, not to do the algorithm for
you, keeping the RISC philosophy.

ThumbGolf remains a register-register instruction set. Very few operations work
directly on memory; that is CISC garbage.

Most instructions are for I/O or arithmetic, as that is pretty tedious and should
not be the reason you don't golf.

The way you include ThumbGolf into your project is actually suprisingly simple:

    gcc -mthumb -march=armv7-a -O2 your_file.s -I thumbgolf thumbgolf/thumbgolf.c -static

You can then run it on hardware or via QEMU.

**No encodings are final** except for `bl.n` or the narrow I/O instructions. I am
definitely sticking with those. However, behavior may change.

Currently, ThumbGolf uses a bunch of ugly GAS macros which have some limitations,
are very incomplete, and have some very awkward mnemonics because there is no
easy macro overloading (and I can't encode memory references)

Once the encoding is mostly finalized, I may create a wrapper or patch which
naturally encodes these using the intended syntax and adds more features.

Currently requires glibc or Android 9.0 to run, and any ARMv7-A processor or
QEMU. I may lower this requirement in the future.

## Features (WIP):

 - [x] Easy high-level SPIM-style I/O, allowing you to do `puts r0`, `getc r3`, etc.
 - [x] A 2 byte PC-relative `bl`, perfect for calling internal subroutines
 - [x] Register swapping
 - [x] Swap register to memory
 - [x] Random number generators
 - [ ] Missing narrow instructions like `movs Rd, #-1` or `adcs Rn, #0` (WIP).
 - [x] MinMax
 - [x] Narrow instructions for unsigned division, multiplication, and modulo by 10
 - [ ] Wide instructions
   - [x] **Guaranteed availibility** of `sdiv` and `udiv` even if it isn't
available on the target device
   - [x] A proper divmod (no more subtraction loops or `mls`! 🎉)
   - [x] Min and max
   - [ ] Wide versions of the narrow instructions
   - [x] Division and modulo by an 8-bit immediate
   - [ ] `pow` and other interesting arithmetic stuff.
 - [ ] If I write a custom assembler:
   - [ ] More natural syntax
   - [ ] Misaligned functions for literal pools
   - [ ] `ldr r0, ="Hello, World!"` possibly
   - [ ] `ldr` expands to `movs` for small constants
   - [ ] Allowing explicit width specifiers for all instructions
   - [ ] Better warnings
   - [ ] Possibly figure out IT blocks
 - [ ] And more (I haven't decided yet).

## Examples

Hello World! (same size as normal libc version lul):
```asm
        .include "thumbgolf.inc"
        .globl main
        .thumb_func
main:
        adr     r0, .Lstr
        puts    r0
        bx      lr
.Lstr:
        .asciz "Hello, World!"
```
`cat`, 8 bytes:
```asm
        .include "thumbgolf.inc"
.Lloop: // nobody said you had to start at the top of a function :)
        putc    r0
        // fallthrough
        .globl main
        .thumb_func
main:
        getc    r0
        beq     .Lloop
        bx      lr
```
