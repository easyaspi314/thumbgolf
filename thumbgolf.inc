/*
 * ThumbGolf, a sigaction-based ISA extension to the Thumb-2 instruction set
 *
 * Copyright (C) 2021 Devin Hussey
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to  use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// Incomplete temporary macro encoder for ThumbGolf
//
// Unfortunately, until I patch binutils, add a wrapper, or GAS gets decent
// macros, things are a little awkward:
//   - You can't prefix literals with #. Very dumb.
//   - No memory addresses
//   - Some macro names are a little awkward, like movs.n1, since they collide
//     with other ARM instructions.
//   - Error checking is a little hacky and also very incomplete.

// so you don't have to
.syntax unified
.arch armv7-a
// Force enable division. We trap it if we have to.
.arch_extension idiv
.thumb

// For all things which take registers, we use it as _tg_reg_arg_\Rd.
// This allows natural use of register names, but unfortunately fails for .req.
.equiv _tg_reg_arg_r0, 0
.equiv _tg_reg_arg_r1, 1
.equiv _tg_reg_arg_r2, 2
.equiv _tg_reg_arg_r3, 3
.equiv _tg_reg_arg_r4, 4
.equiv _tg_reg_arg_r5, 5
.equiv _tg_reg_arg_r6, 6
.equiv _tg_reg_arg_r7, 7

.equiv _tg_reg_arg_r8, 8
.equiv _tg_reg_arg_r9, 9
.equiv _tg_reg_arg_r10, 10
.equiv _tg_reg_arg_sl, 10
.equiv _tg_reg_arg_r11, 11
.equiv _tg_reg_arg_fp, 11
.equiv _tg_reg_arg_r12, 12
.equiv _tg_reg_arg_ip, 12

// note: similar to Thumb-2, SP and PC are usually forbidden
.equiv _tg_reg_arg_r13, 13
.equiv _tg_reg_arg_sp, 13
.equiv _tg_reg_arg_r14, 14
.equiv _tg_reg_arg_lr, 14
.equiv _tg_reg_arg_r15, 15
.equiv _tg_reg_arg_pc, 15

.macro _tg_assert expr:vararg
.if !(\expr)
	.error	"Assertion failed: \expr"
.endif
.endm

.altmacro
.macro _tg_udf.n Opcode:req, Op:req
	_tg_assert (\Opcode >= 0 && \Opcode < 32)
	_tg_assert (\Op >= 0 && \Op < 8)
	udf.n	#(\Opcode << 3) | \Op
.endm

// I/O functions
.macro puts Rn:req; _tg_udf.n 000, _tg_reg_arg_\Rn; .endm
.macro gets Rd:req; _tg_udf.n 001, _tg_reg_arg_\Rd; .endm
.macro putc Rn:req; _tg_udf.n 002, _tg_reg_arg_\Rn; .endm
.macro getc Rd:req; _tg_udf.n 003, _tg_reg_arg_\Rd; .endm
.macro puti Rn:req; _tg_udf.n 004, _tg_reg_arg_\Rn; .endm
.macro geti Rd:req; _tg_udf.n 005, _tg_reg_arg_\Rd; .endm
.macro getw Rd:req; _tg_udf.n 006, _tg_reg_arg_\Rd; .endm
.macro putspc chr:req
	.if (\chr+0 == '\0')
		_tg_udf.n 007, 0
	.elseif (\chr+0 == '\t')
		_tg_udf.n 007, 1
	.elseif (\chr+0 == '\n')
		_tg_udf.n 007, 2
	.elseif (\chr+0 == ' ')
		_tg_udf.n 007, 3
	.elseif (\chr+0 == ',')
		_tg_udf.n 007, 4
	.elseif (\chr+0 == '{')
		_tg_udf.n 007, 5
	.elseif (\chr+0 == '}')
		_tg_udf.n 007, 6
	.elseif (\chr+0 == '"')
		_tg_udf.n 007, 7
	.else
		.error "invalid special char"
	.endif
.endm



// placeholder for movs Rd, -1
.macro movs.n1 Rd:req; _tg_udf.n 012, _tg_reg_arg_\Rd; .endm
.macro mvns.0 Rd:req;  _tg_udf.n 012, _tg_reg_arg_\Rd; .endm
.macro abs.n Rd:req;   _tg_udf.n 013, _tg_reg_arg_\Rd; .endm
// placeholder for udiv Rd, 10
.macro udiv.10 Rd:req; _tg_udf.n 014, _tg_reg_arg_\Rd; .endm
// placeholder for umod Rd, 10
.macro umod.10 Rd:req; _tg_udf.n 015, _tg_reg_arg_\Rd; .endm
// placeholder for umod Rd, 10
.macro mul.10 Rd:req;  _tg_udf.n 016, _tg_reg_arg_\Rd; .endm
.macro rand Rd:req;    _tg_udf.n 017, _tg_reg_arg_\Rd; .endm

// Use byte encoding. GAS can't handle negative labels.
.macro bl.n label:req
	// bkpt #(label - (. + 4)) / 2
	.byte	(\label - (. + 4)) / 2, 0xbe
.endm

// TODO: checking
.macro _tg_wide_4op Opcode:req, a:req, b:req, c:req, d:req
	udf.n	#0300 | (\Opcode & 077)
	.short	(\a << 12) | (\b << 8) | (\c << 4) | \d;
.endm

.macro udivm	Rd:req, Rp:req, Rn:req, Rm:req
	_tg_wide_4op	000, _tg_reg_arg_\Rd, _tg_reg_arg_\Rp, _tg_reg_arg_\Rn, _tg_reg_arg_\Rm
.endm

.macro sdivm	Rd:req, Rp:req, Rn:req, Rm:req
	_tg_wide_4op	001, _tg_reg_arg_\Rd, _tg_reg_arg_\Rp, _tg_reg_arg_\Rn, _tg_reg_arg_\Rm
.endm

.macro umod	Rd:req, Rn:req, Rm:req
	_tg_wide_4op	000, _tg_reg_arg_\Rd, _tg_reg_arg_\Rd, _tg_reg_arg_\Rn, _tg_reg_arg_\Rm
.endm

.macro smod	Rd:req, Rn:req, Rm:req
	_tg_wide_4op	001, _tg_reg_arg_\Rd, _tg_reg_arg_\Rd, _tg_reg_arg_\Rn, _tg_reg_arg_\Rm
.endm

.macro umodi	Rd:req, imm8:req
	_tg_wide_4op	000, _tg_reg_arg_sp, _tg_reg_arg_\Rd, (\imm8 >> 4), (\imm8 & 0xf)
.endm

.macro udivi	Rd:req, imm8:req
	_tg_wide_4op	000, _tg_reg_arg_\Rd, _tg_reg_arg_sp, (\imm8 >> 4), (\imm8 & 0xf)
.endm
.macro smodi	Rd:req, imm8:req
	_tg_wide_4op	001, _tg_reg_arg_sp, _tg_reg_arg_\Rd, (\imm8 >> 4), (\imm8 & 0xf)
.endm

.macro sdivi	Rd:req, imm8:req
	_tg_wide_4op	001, _tg_reg_arg_\Rd, _tg_reg_arg_sp, (\imm8 >> 4), (\imm8 & 0xf)
.endm

// Block the user from using sdiv and udiv in IT blocks. While it works on
// hardware, our emulation will not.
// TODO: fix this.
.irp cond, eq, ne, cs, hs, cc, lo, mi, pl, vs, vc, hi, ls, ge, lt, gt, le, al
.macro	udiv\cond x, y, z
	.error "udiv emulation is not currently allowed in IT blocks!"
.endm
.macro	sdiv\cond x, y, z
	.error "sdiv emulation is not currently allowed in IT blocks!"
.endm
.endr
