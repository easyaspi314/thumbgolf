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

/*
 * This really needs to be reorganized...
 */
#if !(defined(__arm__) || defined(__thumb__))
#  error "This is for 32-bit ARM only!"
#endif

#include <signal.h>
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <ctype.h>

// to match register types
typedef unsigned long u32;
typedef long s32;
/*
 * How function calls work
 *
 * Because ThumbGolf uses a sigaction handler, we can't simply call any function
 * we want from the handler. That is because most of the libc functions aren't
 * Async-Signal-Safe, and tend to cause softlocks if we call them directly.
 *
 * Additionally, we can't simply set PC to the function we want, for one VERY
 * frustrating reason:
 *
 * We can't exchange instruction sets from sigaction. If libc was compiled in
 * ARM mode, we're screwed.
 *
 * Therefore, we have to do a manual veneer and actually raise the signal twice,
 * first to call the function and then to clean up after the function.
 *
 * Each callback, if it returns a value, returns in both r0 and r1, written as
 * returning uint64_t.
 *
 * The lower 32 bits (r0) represent the value we got, and the upper 32 bits (r1)
 * are the "success" flag. We use this to set the Z flag to indicate success.
 */
#define THUMB_CPSR_BIT 5
#define ITHI_CPSR_BIT 10
#define ITLO_CPSR_BIT 25
#define N_CPSR_BIT 31
#define Z_CPSR_BIT 30
#define C_CPSR_BIT 29
#define V_CPSR_BIT 28

#define THUMB_CPSR (1U << THUMB_CPSR_BIT)
#define N_CPSR (1U << N_CPSR_BIT)
#define Z_CPSR (1U << Z_CPSR_BIT)
#define C_CPSR (1U << C_CPSR_BIT)
#define V_CPSR (1U << V_CPSR_BIT)
#define IT_STATE(cpsr) ({ \
    u32 _tmp = (cpsr); \
    ((_tmp >> ITHI_CPSR_BIT) % (1 << 6) << 2) | ((_tmp >> ITLO_CPSR_BIT) % 4); \
})
#define REG_SP 13
#define REG_LR 14
#define REG_PC 15
#define REG_CPSR 16

_Static_assert(offsetof(struct sigcontext, arm_cpsr) - offsetof(struct sigcontext, arm_r0) == 64, "bad siginfo_t");

_Noreturn static void illegal(void)
{
    signal(SIGILL, SIG_DFL);
    signal(SIGTRAP, SIG_DFL);
    raise(SIGILL);
    exit(132);
}

_Noreturn static void die(const char *msg)
{
    write(2, msg, strlen(msg));
    illegal();
}

static void *xmalloc(size_t num)
{
    void *ret = calloc(num, 1);
    if (!ret) {
        fprintf(stderr, "Out of memory!\n");
        abort();
    }
    return ret;
}

static uint64_t get_int(void)
{
    int tmp = 0;
    uint64_t ret = scanf("%i", &tmp) == 1;
    if (!ret) {
        // skip to end on error
        int c;
        while ((c = getchar()) != EOF && !isspace(c)) {}
    }

    return (u32)tmp | (ret << 32);
}

// Wide puti will probably have a flag for no newline and for hex/unsigned/octal.
static void put_int_n(int x)
{
    printf("%i\n", x);
}

// Narrow puts is literally puts
// Wide puts will have a flag for no newline
static void put_str_n(const char *str)
{
    puts(str);
}


static char *scanf_buffer = NULL;

// TODO: replace %m with something that works on Android <= 8
static uint64_t get_str(void)
{
    free(scanf_buffer);
    scanf_buffer = NULL;
    uint64_t ret = scanf("%m[^\n]", &scanf_buffer) == 1;
    return (u32)scanf_buffer | (ret << 32);
}

static uint64_t get_word(void)
{
    free(scanf_buffer);
    scanf_buffer = NULL;
    uint64_t ret = scanf("%ms", &scanf_buffer) == 1;
    return (u32)scanf_buffer | (ret << 32);
}


static uint64_t get_char(void)
{
    int c = getchar();
    if (c == EOF) {
        return 0;
    }
    return 0x100000000ull | c;
}
static void put_char(int c)
{
    putchar(c);
}

// Prints one of 8 special characters to stdout.
//
// These include null byte, tab, newline, space, comma, curly brackets, and
// double quote.
static const char special_chars[] = {
    '\0', '\t', '\n', ' ', ',', '{', '}', '"'
};
static void put_special(u32 insn)
{
    putchar(special_chars[insn & 7]);
}

static uint64_t get_rand(void)
{
    u32 ret = 0;
    FILE *f = fopen("/dev/urandom", "rb");
    fread(&ret, 4, 1, f);
    fclose(f);
    uint64_t is_zero = ret == 0;
    return ret | (is_zero << 32);
}

#define MODE_VOID 0
#define MODE_REG_Z 1
#define MODE_R0 2
#define MODE_OUTPUT_REG 4
#define MODE_IMM 8

struct io_callback
{
    const void *func;
    int mode;
};

// Basic I/O instructions, udf #00xx
// These all require veneers since they call unsafe libc functions.
// Wide versions are planned whic
static const struct io_callback io_funcs[] = {
    { &put_str_n, MODE_VOID | MODE_OUTPUT_REG },  // puts   Rn
    { &get_str, MODE_REG_Z },                     // gets   Rd
    { &put_char, MODE_VOID | MODE_OUTPUT_REG },   // putc   Rn
    { &get_char, MODE_REG_Z },                    // getc   Rd
    { &put_int_n, MODE_VOID | MODE_OUTPUT_REG },  // puti   Rn
    { &get_int, MODE_REG_Z },                     // geti   Rd
    { &get_word, MODE_REG_Z },                    // getw   Rd
    { &put_special, MODE_VOID | MODE_IMM },       // putspc #imm
};

_Static_assert(sizeof(sig_atomic_t) >= sizeof(u32), "bad sig_atomic_t");

static volatile sig_atomic_t in_libc_call = 0;
// Caller saved registers
static volatile sig_atomic_t old_r0 = 0;
static volatile sig_atomic_t old_r1 = 0;
static volatile sig_atomic_t old_r2 = 0;
static volatile sig_atomic_t old_r3 = 0;
static volatile sig_atomic_t old_r12 = 0;
static volatile sig_atomic_t old_lr = 0;
static volatile sig_atomic_t old_cpsr = 0;

// A basic call veneer.
__attribute__((__naked__, target("thumb")))
static int64_t do_veneer(void)
{
    __asm__(
        "bx      r12\n"
    );
}

// Since we can't manually exchange instruction sets, and because stdio is NOT
// signal safe, we need to veneer.
// TODO: make more generic and simplify
static void veneer(u32 *regs, const void *func, u32 insn, int mode)
{
    if (!in_libc_call) {
        // Enter libc mode
        in_libc_call = 1;
        // Save r0, r1, r2, r3, r12, lr, and the CPSR.
        old_r0   = (sig_atomic_t)regs[0];
        old_r1   = (sig_atomic_t)regs[1];
        old_r2   = (sig_atomic_t)regs[2];
        old_r3   = (sig_atomic_t)regs[3];
        old_r12  = (sig_atomic_t)regs[12];
        old_lr   = (sig_atomic_t)regs[REG_LR];
        old_cpsr = (sig_atomic_t)regs[REG_CPSR];
        // If MODE_OUTREG, set r0 to the target register
        if (mode & MODE_OUTPUT_REG) {
            regs[0] = regs[insn & 7];
        } else if (mode & MODE_IMM) {
            regs[0] = insn;
        }

        // bx func via a rather typical r12 veneer
        regs[12] = (u32)func;
        regs[REG_LR] = regs[REG_PC] | 1; // set Thumb bit
        // On actual hardware, the PC register ignores the lowest bit, and the
        // address I write here will always be executed in Thumb state.
        //
        // However, QEMU, for some bizarre reason, will switch to ARM when the
        // Thumb bit is set.
        //
        // Yes, you read that correctly.
        regs[REG_PC] = ((u32)&do_veneer) & ~1;
    } else {
        // Exit libc mode
        in_libc_call = 0;
        // Restore the CPSR first.
        regs[REG_CPSR] = (u32)old_cpsr;
        // Restore registers in their expected position.
        regs[2] = (u32)old_r2;
        regs[3] = (u32)old_r3;
        switch (mode & 3) {
        case MODE_VOID: // Restore everything.
            regs[0] = (u32)old_r0;
            regs[1] = (u32)old_r1;
            break;
        case MODE_REG_Z:
            // Set Z flag to result
            regs[REG_CPSR] &= ~Z_CPSR;
            regs[REG_CPSR] |= !!regs[1] * Z_CPSR;
            // Restore r1
            regs[1] = (u32)old_r1;
            if ((insn & 7) != 0) {
                regs[insn & 7] = regs[0];
                regs[0] = old_r0;
            }
            break;
        default: // TODO
            break;
        }
        // Restore r12 and lr
        regs[12]     = (u32)old_r12;
        regs[REG_LR] = (u32)old_lr;
        regs[REG_PC] += 2;
    }
}


static void is_bad_reg(u32 regno)
{
    if (regno == REG_SP || regno == REG_PC) {
        die("Use of SP and PC are not allowed.");
    }
}

// Unsigned arithmetic
// 000 aaaabbbbccccdddd
// if a == SP
//    regs[b] %= cd
// else if a == PC
//    regs[b] = umax(regs[c], regs[d])
// else if b == SP
//    regs[a] /= cd
// else if b == PC
//    regs[a] = umin(regs[c], regs[d])
// else
//    regs[a] = regs[c] / regs[d]
//    regs[b] = regs[c] % regs[d]
static void decode_wide_000(u32 *regs, u32 insn)
{
    u32 Rd = (insn >> 12) & 0xF;
    u32 Rp = (insn >> 8) & 0xF;
    u32 Rn = (insn >> 4) & 0xF;
    u32 Rm = (insn >> 0) & 0xF;

    if (Rd == REG_SP) { // umod Rd, #imm8
        is_bad_reg(Rp);
        u32 imm8 = insn & 0xFF;
        if (imm8 == 0) {
            die("umod with imm8 == 0 is reserved!\n");
        }
        regs[Rp] %= imm8;
    } else if (Rd == REG_PC) { // umax Rd, Rn, Rm
        is_bad_reg(Rp);
        regs[Rp] = (regs[Rn] > regs[Rm]) ? regs[Rn] : regs[Rm];
    } else if (Rp == REG_SP) { // udiv Rd, #imm8
        u32 imm8 = insn & 0xFF;
        if (imm8 == 0) {
            die("udiv with imm8 == 0 is reserved!\n");
        }
        regs[Rd] /= imm8;
    } else if (Rp == REG_PC) { // umin Rd, Rn, Rm
        regs[Rd] = (regs[Rn] < regs[Rm]) ? regs[Rn] : regs[Rm];
    } else { // udivm Rd, Rp, Rn, Rm or umod Rd, Rn, Rm
        is_bad_reg(Rn);
        is_bad_reg(Rm);
        if (Rn == Rm) { // x / x == 1, ya dummy.
            die("udivm a, b, c, c is reserved!\n");
        }
        u32 quotient = regs[Rm] != 0 ? regs[Rn] / regs[Rm] : 0;
        u32 remainder = regs[Rm] != 0 ? regs[Rn] % regs[Rm] : 0;
        regs[Rd] = quotient;
        regs[Rp] = remainder; // just overwrite for sdivm
    }
    regs[REG_PC] += 4;
}

// Signed arithmetic, otherwise identical to the unsigned one
// 001 aaaabbbbccccdddd
// if a == SP
//    regs[b] %= cd
// else if a == PC
//    regs[b] = smax(regs[c], regs[d])
// else if b == SP
//    regs[a] /= cd
// else if b == PC
//    regs[a] = smin(regs[c], regs[d])
// else
//    regs[a] = regs[c] / regs[d]
//    regs[b] = regs[c] % regs[d]
static void decode_wide_001(u32 *regs, u32 insn)
{
    u32 Rd = (insn >> 12) & 0xF;
    u32 Rp = (insn >> 8) & 0xF;
    u32 Rn = (insn >> 4) & 0xF;
    u32 Rm = (insn >> 0) & 0xF;
    regs[REG_PC] += 4;
    if (Rd == REG_SP) { // smod Rd, #imm8
        is_bad_reg(Rp);
        s32 imm8 = insn & 0xFF;
        if (imm8 == 0) {
            die("smod with imm8 == 0 is reserved!\n");
        }
        regs[Rp] = (u32)((s32)regs[Rp] % imm8);
    } else if (Rd == REG_PC) { // smax Rd, Rn, Rm
        is_bad_reg(Rp);
        regs[Rp] = ((s32)regs[Rn] > (s32)regs[Rm]) ? regs[Rn] : regs[Rm];
    } else if (Rp == REG_SP) { // sdiv Rd, #imm8
        s32 imm8 = insn & 0xFF;
        if (imm8 == 0) {
	    die("sdiv with imm8 == 0 is reserved!\n");
        }
        regs[Rd] = (u32)((s32)regs[Rd] / imm8);
    } else if (Rp == REG_PC) { // smin Rd, Rn, Rm
        regs[Rp] = ((s32)regs[Rn] < (s32)regs[Rm]) ? regs[Rn] : regs[Rm];
    } else { // sdivm Rd, Rp, Rn, Rm or smod Rd, Rn, Rm
        is_bad_reg(Rn);
        is_bad_reg(Rm);
        if (Rn == Rm) {
            die("sdivm a, b, c, c is reserved!\n");
        }
        s32 quotient = regs[Rm] != 0 ? (s32)regs[Rn] / (s32)regs[Rm] : 0;
        s32 remainder = regs[Rm] != 0 ? (s32)regs[Rn] % (s32)regs[Rm] : 0;
        regs[Rd] = (u32)quotient;
        regs[Rp] = (u32)remainder; // just overwrite for smod
    }
}

// Wide instructions are significantly more flexible and feel more like Thumb-2.
// Similar to Thumb-2, it treats SP and PC differently than other registers, and
// it is used to encode other instructions.
//
// Note that not all instructions are in the X, Y, Z format. See udiv Rd, #imm.
// This is because we only have ~19 bits (compared to Thumb-2's ~28 bits).
//
// I would rather have some instructions be dyadic than limit ALL instructions
// to Lo registers or give up functionality.
static void decode_wide_insn(u32 *regs, u32 insn)
{
    // We have 34 wide insn classes. That is because the swap register insn
    // is commutative, so we can just make sure that it is sorted.
    switch ((insn >> 16) & 077) {
        case 000: decode_wide_000(regs, insn); break;
        case 001: decode_wide_001(regs, insn); break;
        default: illegal(); // WIP
    }
}

static void sighandler(int signo, siginfo_t *si, void *data)
{
    ucontext_t *uc = (ucontext_t *)data;
    u32 *regs = &uc->uc_mcontext.arm_r0;
    u32 *cpsr = &uc->uc_mcontext.arm_cpsr;
    // only trap Thumb instructions
    if (*cpsr & THUMB_CPSR) {
        uint16_t insn = *(const uint16_t *)(uc->uc_mcontext.arm_pc&~1);
        if (IT_STATE(*cpsr)) {
            die("ThumbGolf instructions are not supported in IT blocks (yet).\n");
        }
        // udf.n: DExx
        // NOTE: Narrow instructions and wide opcodes are in octal, while
        // wide instructions are in hex.
        if ((insn & 0xFF00) == 0xDE00) {
            // Narrow instructions are 0xyz, where x is a 2 bit selector,
            // y is the operation (or register in a swap), z is the target
            // register.

            // Wide instructions have a 6 bit ID, starting with 03xy, but
            // it is actually 34 because x must be <= y. When x > y, it is a
            // commutative swap.
            switch (insn & 0300) {
                case 0000: { // SPIM-style I/O
                    int id = (insn >> 3) & 7;
                    veneer(regs, io_funcs[id].func, insn, io_funcs[id].mode);
                    break;
                }
                case 0100: { // narrow arithmetic
                    switch (insn & 0070) {
                        case 0000:  // adcs Rd, #0
                        case 0010:  // sbcs Rd, #0
                            die("adcs/sbcs are coming soon.\n");
                        case 0020: { // movs Rd, #-1
                            regs[insn & 7] = -1;
                            regs[REG_CPSR] &= ~(Z_CPSR | N_CPSR | C_CPSR | V_CPSR);
                            regs[REG_CPSR] |= N_CPSR;
                            regs[REG_PC] += 2;
                            break;
                        }
                        case 0030: { // abs Rd
                            if ((s32)regs[insn & 7] < 0) {
                                regs[insn & 7] = -regs[insn & 7];
                            }
                            regs[REG_PC] += 2;
                            break;
                        }
                        case 0040: { // udiv Rd, #10
                            regs[insn & 7] /= 10;
                            regs[REG_PC] += 2;
                            break;
                        }
                        case 0050: { // umod Rd, #10
                            regs[insn & 7] %= 10;
                            regs[REG_PC] += 2;
                            break;
                        }
                        case 0060: { // mul Rd, #10
                            regs[insn & 7] *= 10;
                            regs[REG_PC] += 2;
                            break;
                        }
                        case 0070: { // rand Rd
                            veneer(regs, &get_rand, insn, MODE_REG_Z);
                            break;
                        }
                    }
                    break;
                }
                case 0200: { // narrow swap with memory
                    // swapm  Rd, [Rt]
                    int Rt = insn & 7;
                    int Rd = (insn >> 3) & 7;

                    u32 *Mt = (u32 *)regs[Rt];
                    u32 tmp = *Mt;
                    *Mt = regs[Rd];
                    regs[Rd] = tmp;
                    regs[REG_PC] += 2;
                    break;
                }
                case 0300: { // swap register or wide instruction
                    // The function of this instruction depends on if Rn is
                    // greater than Rm. If it is, it is a swap instruction.
                    // If it isn't, it is a wide instruction.
                    int Rn = insn & 7;
                    int Rm = (insn >> 3) & 7;
                    if (Rn > Rm) { // swap Rn, Rm
                        u32 tmp = regs[Rn];
                        regs[Rn] = regs[Rm];
                        regs[Rm] = tmp;
                        regs[REG_PC] += 2;
                    } else { // wide instruction
                        u32 insn2 = ((const uint16_t *)(uc->uc_mcontext.arm_pc&~1))[1];
                        u32 wide_insn = (((u32)insn & 077) << 16) | insn2;
                        decode_wide_insn(regs, wide_insn);
                    }
                    break;
                }
            }
        // bkpt: BExx
        // When xx == 0, exits the program.
        // Otherwise, does a "relative bl". Doesn't exchange instruction sets.
        } else if ((insn & 0xFF00) == 0xBE00) {
            s32 offset = ((s32)(int8_t)insn) * 2;
            if (offset == 0) {
                exit(0);
            }
            regs[REG_LR] = (regs[REG_PC] + 2) | 1;
            regs[REG_PC] += offset;
        // Emulate the division instructions if we don't have them.
        // I honestly don't know why this was optional in ARMv7-A. ¯\_(ツ)_/¯
        // sdiv: FB9x xxFx
        // udiv: FBBx xxFx
        // XXX: is this mask correct?
        }  else if ((insn & 0xFF90) == 0xFB90) {
              u32 insn2 = ((const uint16_t *)(uc->uc_mcontext.arm_pc&~1))[1];
              if ((insn2 & 0x00F0) == 0x00F0) {
                  // To be ABSOLUTELY safe, forcibly call libgcc.
                  // The last thing we want is an infinite loop.
                  if (insn & 0x0040) { // udiv
                      extern unsigned __aeabi_uidiv(unsigned num, unsigned dem);
                      u32 Rn = regs[insn & 0xF];
                      u32 Rm = regs[insn2 & 0xF];
                      regs[(insn2 >> 8) & 0xF] = Rm != 0 ? __aeabi_uidiv(Rn, Rm) : 0;
                  } else { // sdiv
                      extern int __aeabi_idiv(int num, int dem);
                      s32 Rn = (s32)regs[insn & 0xF];
                      s32 Rm = (s32)regs[insn2 & 0xF];
                      regs[(insn2 >> 8) & 0xF] = Rm != 0 ? (u32)__aeabi_idiv(Rn, Rm) : 0;
                  }
                  regs[REG_PC] += 4;
                  return;
              } else {
                  illegal();
              }
        }
        // TODO: udf.w? That is 12 true bits
    } else {
        // Joking aside, there is no reason to golf with ARM instruction, and
        // our code expects the program to be in Thumb state.
        die(
            "This IS called ThumbGolf for a reason, ya know?\n"
            "Did you forget to compile your program in Thumb mode?\n"
        );
    }
}

// Run before main
__attribute__((constructor))
void thumbgolf_inject(void)
{
    // Set stdout, stdin, and stderr to unbuffered mode to avoid conflicts
    // with manual syscalls.
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    // Set up the signal handler
    struct sigaction sa, osa;
    sa.sa_flags = SA_ONSTACK | SA_RESTART | SA_NODEFER | SA_SIGINFO;
    sa.sa_sigaction = sighandler;
    sigaction(SIGILL, &sa, &osa);
    sigaction(SIGTRAP, &sa, &osa);
};

