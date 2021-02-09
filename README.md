# ThumbGolf

ThumbGolf is an extension to the Thumb-2 instruction set for golfing.

If Jelly and Pyth are like toolsheds, ThumbGolf is a Swiss Army knife (and no, not
[this kind](https://stylesdaddy.com/wp-content/uploads/2019/11/Wenger-Swiss-Army-Knife-Giant.jpg)).
It gives you tools to let you focus on the algorithm, not to do the algorithm for
you, keeping the RISC philosophy.

**It should go without saying, but please do not use this for anything serious.**

ThumbGolf remains a register-register instruction set. Very few operations work
directly on memory; that is CISC garbage.

Most instructions are for I/O or arithmetic, as that is pretty tedious and should
not be the reason you don't golf.

The way you include ThumbGolf into your project is actually suprisingly simple:

    gcc -mthumb -march=armv7-a -O2 your_file.s -I thumbgolf thumbgolf/thumbgolf.c -static

You can then run it on hardware or via QEMU.

**No encodings are final** except for `bl.n` or the narrow I/O instructions. I am
definitely sticking with those. However, behavior and the mnemonics may change.

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
 - [x] Narrow instructions for unsigned division, multiplication, and modulo by 10
 - [ ] Wide instructions
   - [x] **Guaranteed availibility** of `sdiv` and `udiv` even if it isn't
available on the target device†
   - [x] A proper divmod (no more subtraction loops or `mls`! 🎉)
   - [x] Min and max
   - [ ] Wide versions of the narrow instructions
   - [x] Division and modulo by an 8-bit immediate
   - [ ] `pow` and other interesting arithmetic stuff.
 - [ ] If I write a custom assembler:
   - [ ] More natural syntax
   - [ ] IT blocks? Technically, ARM has deprecated IT blocks for udf.
   - [ ] Misaligned functions for literal pools
   - [ ] `ldr r0, ="Hello, World!"` possibly
   - [ ] `ldr` expands to `movs` for small constants
   - [ ] Allowing explicit width specifiers for all instructions
   - [ ] Better warnings
 - [ ] And more (I haven't decided yet).

Note that ThumbGolf does **not** support IT blocks (yet). This also includes
`sdiv` and `udiv` emulation. `thumbgolf.inc` will cause predicated `udiv` and
`sdiv` instructions to error until/if it is ever fixed.

## How it works

**This isn't a Thumb-2 emulator** (god, that would be painful to write), it is
an overlay for the ARMv7-A instruction set using a (possibly) magic runtime.

Specifically, it encodes instructions in the permanently undefined and breakpoint
opcode spaces, namely the `udf` and `bkpt` instructions.

This causes the processor to raise an exception, and the kernel translates it to
a `SIGILL` or `SIGTRAP` signal. The ThumbGolf runtime uses `sigaction()` to
catch these exception.

`sigaction()` is very generous. Unlike `signal()`, `sigaction()` gives us a
**mutable snapshot of the processor state**, designed to allow emulating
unsupported instructions.

Using that state and the immediate bits of the instruction, we interpret
the ThumbGolf opcode, then return control to the original program as if it is a
native instruction.

It actually has a rather complex method of running C library calls, because
 1. Many common libc functions, including everything in `<stdio.h>`, are
**unsafe** to call from signal handlers.
 2. While QEMU seems to be able to switch instruction sets (albeit incorrectly:
if the Thumb bit is set, it switches to ARM mode 🤔), it is not possible to do
on real hardware.

Long story short, we simulate a call to the function on the main thread, and
trap when it returns to finalize the instruction. It is very complex.

See the comment in thumbgolf.c.

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
