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
#include <stdbool.h>
#include <ctype.h>

// to match register types
typedef unsigned long u32;
typedef long s32;
typedef unsigned short u16;
typedef unsigned long long u64;

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
 * returning u64.
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

typedef union {
    u32 raw;
    struct {
        u32  mode     : 5;
        bool thumb    : 1;
        bool firq     : 1;
        bool irq      : 1;
        bool async    : 1;
        bool endian   : 1;
        bool it_c     : 1;
        bool it_b     : 1;
        bool it_a     : 1;
        u32  it_cond  : 3;
        u32  gt_eq    : 4;
        u32  unused   : 4;
        bool jazelle  : 1;
        bool it_e     : 1;
        bool it_d     : 1;
        bool saturate : 1;
        bool overflow : 1;
        bool carry    : 1;
        bool zero     : 1;
        bool negative : 1;
    };
} arm_cpsr;

typedef struct {
    u32 r[16];
    arm_cpsr cpsr;
// TODO
//    union {
//        double qregs[16];
//        float dregs[32];
//    };
} tgolf_state;

_Static_assert(offsetof(struct sigcontext, arm_cpsr) - offsetof(struct sigcontext, arm_r0) == 64, "bad siginfo_t");

// Rethrow SIGILL. We must disable our signal handlers first.
_Noreturn static void rethrow(void)
{
    signal(SIGILL, SIG_DFL);
    signal(SIGTRAP, SIG_DFL);
    raise(SIGILL);
    // just in case
    exit(132);
}

_Noreturn static void die(const char *msg)
{
    write(2, msg, strlen(msg));
    rethrow();
}

static char tohex(int x)
{
    if (x > 9) return 'a' + (x - 10);
    return '0' + x;
}

_Noreturn static void illegal(u32 insn)
{
    char buf[] = "Illegal instruction xxxx\n\0xxx\n";
    buf[23] = tohex(insn & 0xF);
    buf[22] = tohex((insn >> 4) & 0xF);
    buf[21] = tohex((insn >> 8) & 0xF);
    buf[20] = tohex((insn >> 12) & 0xF);
    if (insn >> 16) {
        buf[24] = ' ';
        buf[28] = tohex((insn >> 16) & 0xF);
        buf[27] = tohex((insn >> 20) & 0xF);
        buf[26] = tohex((insn >> 24) & 0xF);
        buf[25] = tohex((insn >> 28) & 0xF);
    }
    die(buf);
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

static u64 get_int(void)
{
    int tmp = 0;
    u64 ret = scanf("%i", &tmp) == 1;
    if (!ret) {
        // skip to end on error
        int c;
        while ((c = getchar()) != EOF && !isspace(c)) {}
    }

    return (u32)tmp | (ret << 32);
}

// Wide puti will probably have a flag for hex/unsigned/octal.
static void put_int_n(int x)
{
    printf("%i", x);
}

// Narrow puts does NOT print a new line
static void put_str_n(const char *str)
{
    if (str != NULL)
        printf("%s", str);
}


static char *scanf_buffer = NULL;

// TODO: replace %m with something that works on Android <= 8
static u64 get_str(void)
{
    free(scanf_buffer);
    scanf_buffer = NULL;
    u64 ret = scanf("%m[^\n]", &scanf_buffer) == 1;
    return (u32)scanf_buffer | (ret << 32);
}

static u64 get_word(void)
{
    free(scanf_buffer);
    scanf_buffer = NULL;
    u64 ret = scanf("%ms", &scanf_buffer) == 1;
    return (u32)scanf_buffer | (ret << 32);
}


static u64 get_char(void)
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
//
// This is designed to compensate for the narrow I/O functions not printing
// separators.
static const char special_chars[] = {
    '\0', '\t', '\n', ' ', ',', '{', '}', '"'
};
static void put_special(u32 insn)
{
    put_char(special_chars[insn & 7]);
}

static u64 get_rand(void)
{
    u32 ret = 0;
    FILE *f = fopen("/dev/urandom", "rb");
    fread(&ret, 4, 1, f);
    fclose(f);
    u64 is_zero = ret == 0;
    return ret | (is_zero << 32);
}

// todo: use
__attribute__((__naked__))
static void save_vfp(double *vfpstate)
{
    __asm__(
        "    vstmia r0!, {d0-d15}\n"
        "    vstmia r0!, {d16-d31}\n"
        "    bx     lr"
    );
}

__attribute__((__naked__))
static void restore_vfp(double *vfpstate)
{
    __asm__(
        "    vldmia r0!, {d0-d15}\n"
        "    vldmia r0!, {d16-d31}\n"
        "    bx     lr"
    );
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
// Wide versions are planned which will accept all registers and add more
// options.
static const struct io_callback io_funcs[] = {
    { &put_str_n, MODE_OUTPUT_REG },  // puts   Rn
    { &get_str, MODE_REG_Z },         // gets   Rd
    { &put_char, MODE_OUTPUT_REG },   // putc   Rn
    { &get_char, MODE_REG_Z },        // getc   Rd
    { &put_int_n, MODE_OUTPUT_REG },  // puti   Rn
    { &get_int, MODE_REG_Z },         // geti   Rd
    { &get_word, MODE_REG_Z },        // getw   Rd
    { &put_special, MODE_IMM },       // putspc #imm
};

_Static_assert(sizeof(sig_atomic_t) >= sizeof(u32), "bad sig_atomic_t");


static void is_bad_reg(u32 regno)
{
    if (regno == REG_SP || regno == REG_PC) {
        die("Use of SP and PC are not allowed.");
    }
}

// Unsigned arithmetic
// 000000 aaaabbbbccccdddd
// if a == SP
//    RESERVED
// else if a == PC
//    regs[b] = umax(regs[c], regs[d])
// else if b == SP
//    RESERVED
// else if b == PC
//    regs[a] = umin(regs[c], regs[d])
// else
//    regs[a] = regs[c] / regs[d]
//    regs[b] = regs[c] % regs[d]
static void decode_wide_000(tgolf_state *s, u32 insn)
{
    u32 Rd = (insn >> 12) & 0xF;
    u32 Rp = (insn >> 8) & 0xF;
    u32 Rn = (insn >> 4) & 0xF;
    u32 Rm = (insn >> 0) & 0xF;

    if (Rd == REG_PC) { // umax Rd, Rn, Rm
        is_bad_reg(Rp);
        s->r[Rp] = (s->r[Rn] > s->r[Rm]) ? s->r[Rn] : s->r[Rm];
    } else if (Rp == REG_PC) { // umin Rd, Rn, Rm
        s->r[Rd] = (s->r[Rn] < s->r[Rm]) ? s->r[Rn] : s->r[Rm];
    } else { // udivm Rd, Rp, Rn, Rm or umod Rd, Rn, Rm
        is_bad_reg(Rd);
        is_bad_reg(Rp);
        is_bad_reg(Rn);
        is_bad_reg(Rm);
        if (Rn == Rm) { // x / x == 1, ya dummy.
            die("udivm a, b, c, c is reserved!\n");
        }
        u32 quotient = s->r[Rm] != 0 ? s->r[Rn] / s->r[Rm] : 0;
        u32 remainder = s->r[Rm] != 0 ? s->r[Rn] % s->r[Rm] : 0;
        s->r[Rd] = quotient;
        s->r[Rp] = remainder; // just overwrite for sdivm
    }
}

// Signed arithmetic, otherwise identical to the unsigned one
// 000001 aaaabbbbccccdddd
// if a == SP
//    RESERVED
// else if a == PC
//    regs[b] = smax(regs[c], regs[d])
// else if b == SP
//    RESERVED
// else if b == PC
//    regs[a] = smin(regs[c], regs[d])
// else
//    regs[a] = regs[c] / regs[d]
//    regs[b] = regs[c] % regs[d]
static void decode_wide_001(tgolf_state *s, u32 insn)
{
    u32 Rd = (insn >> 12) & 0xF;
    u32 Rp = (insn >> 8) & 0xF;
    u32 Rn = (insn >> 4) & 0xF;
    u32 Rm = (insn >> 0) & 0xF;

    if (Rd == REG_PC) { // smax Rd, Rn, Rm
        is_bad_reg(Rp);
        s->r[Rp] = ((s32)s->r[Rn] > (s32)s->r[Rm]) ? s->r[Rn] : s->r[Rm];
    } else if (Rp == REG_PC) { // smin Rd, Rn, Rm
        s->r[Rp] = ((s32)s->r[Rn] < (s32)s->r[Rm]) ? s->r[Rn] : s->r[Rm];
    } else { // sdivm Rd, Rp, Rn, Rm or smod Rd, Rn, Rm
        is_bad_reg(Rd);
        is_bad_reg(Rp);
        is_bad_reg(Rn);
        is_bad_reg(Rm);
        if (Rn == Rm) {
            die("sdivm a, b, c, c is reserved!\n");
        }
        s32 quotient = s->r[Rm] != 0 ? (s32)s->r[Rn] / (s32)s->r[Rm] : 0;
        s32 remainder = s->r[Rm] != 0 ? (s32)s->r[Rn] % (s32)s->r[Rm] : 0;
        s->r[Rd] = (u32)quotient;
        s->r[Rp] = (u32)remainder; // just overwrite for smod
    }
}

// unsigned divide by immediate
//   000010 dddd nnnn iiiiiiii
// unsigned modulo by immediate
//   000011 dddd nnnn iiiiiiii
// signed divide by immediate
//   000110 dddd nnnn iiiiiiii
// signed modulo by immediate
//   000117 dddd nnnn iiiiiiii
// dddd and nnnn must not be PC or SP
// i must not be zero
static void decode_wide_divmodi(tgolf_state *s, u32 insn)
{
    u32 Rd = (insn >> 12) & 0xF;
    u32 Rn = (insn >> 8) & 0xF;
    u32 divisor = insn & 0xFF;

    is_bad_reg(Rd);
    is_bad_reg(Rn);
    if (divisor == 0) {
        die("division/modulo with imm == 0 is reserved!\n");
    }
    switch ((insn >> 16) & 007) {
        case 002: s->r[Rd] = s->r[Rn] / divisor; break;
        case 003: s->r[Rd] = s->r[Rn] % divisor; break;
        case 006: s->r[Rd] = (s32)s->r[Rn] / (s32)divisor; break;
        case 007: s->r[Rd] = (s32)s->r[Rn] % (s32)divisor; break;
        // should never happen
        default: illegal(insn);
    }
}

// Wide I/O instructions
//
// 000010 op: 3 or 4 ...
// on output instructions, pc represents stdout and sp
// represents stderr.
// on input instructions, pc represents stdin

// op = 0000: write
// note:
//    when llll == 1111, it uses strlen
//    when llll == 1101, it uses strlen and adds newline
// 000100 0000 ffff bbbb llll -> write(f, b, l)
// 000100 0000 1111 bbbb llll -> write(1, b, l) (a.k.a. puts.w)
// 000100 0000 1101 bbbb llll -> write(2, b, l)
static void decode_wide_write(tgolf_state *s, u32 insn)
{
    char *str = (char *)s->r[(insn >> 4) & 0xF];
    if (str == NULL)
        return;

    u32 len_arg = insn & 0xF;
    int add_newline = 0;
    size_t len;
    switch (len_arg) {
    case REG_SP:
        add_newline = 1;
        // fallthrough
    case REG_PC:
        len = strlen(str);
        break;
    default:
        len = s->r[len_arg];
        break;
    }

    u32 fd_arg = (insn >> 8) & 0xF;
    FILE *f;
    switch (fd_arg) {
    case REG_PC:
        f = stdout;
        break;
    case REG_SP:
        f = stderr;
        break;
    default:
        f = (FILE *)s->r[fd_arg];
        break;
    }

    fwrite(str, 1, len, f);
    if (add_newline) {
        fputc('\n', f);
    }
}

// op = 0001: read (not implemented yet)
// note:
//    sets zf on success
//    when llll == 1111, reads a full line, removing newline
//    when llll == 1101, reads a full line, including newline
// 000100 0001 ffff bbbb llll -> read(f, b, l)
// 000100 0001 1111 bbbb llll -> read(0, b, l)
// 000100 0001 1101 dddd ffff -> d = fgetc(f)

// planned but not implemented yet
// 000100 0010 ffff nnnn 0mmm -> f = fopen(n, mode)
// 000100 0010 ffff dddd 1mmm -> f = fdopen(d, mode)
// 000100 0010 ffff 1111 0000 -> fclose(f)
// 000100 0010 dddd 1101 ffff -> d = fileno(f) (automatically flushes)
// 000100 0010 1111 1101 ffff -> fflush(f)
// 000100 0010 1101 1101 ffff -> 
// 000100 0100 ffff iiiiiiii  -> write(f, &c, 1)
// 000100 0101 ffff oooooooo  -> write(f, pc + o, strlen(pc + o)
// if lowest bit is set, print newline
// 000100 0110 ffff rrrr 000 n -> fprintf(f, "%i", r)
// 000100 0110 ffff rrrr 100 n -> fprintf(f, "%u", r)
// 000100 0110 ffff rrrr 101 n -> fprintf(f, "%08x", r)
// Annoyingly, we don't get a snapshot of VFP.
// 000100 0110 ffff rrrrr 11 n -> fprintf(f, "%g", dr) (vfp 64-bit reg)
// 000100 0110 ffff rrrrr 10 n -> fprintf(f, "%g", (float)sr) (vfp 32-bit reg)
// Syscalls
// 000100 100x xxxx xxxx dddd -> d = syscall(x, r0 - r6)
static void decode_wide_io(tgolf_state *s, u32 insn)
{
    switch (insn & 0xE000) {
    case 0x0000:
        decode_wide_write(s, insn);
        break;
    default:
        illegal(insn);
    }
}

// 000101: register/memory manip
// 000101 0000 xxxx yyyy 1111 -> swap x, y
// 000101 0000 xxxx yyyy tttt -> swp  x, y, [t]
// 000101 0001 xxxx yyyy tttt -> swpb x, y, [t]
// 000101 1000 dddd iiii iiii -> d = calloc(1, i * 8)
// 000101 1000 xxxx 0000 0000 -> free(x); x = NULL
// 000101 1001 dddd nnnn 0000 -> d = calloc(1, n)

// Wide instructions are significantly more flexible and feel more like Thumb-2.
// Similar to Thumb-2, it treats SP and PC differently than other registers, and
// it is used to encode other instructions.
//
// Note that not all instructions may be in the X, Y, Z format, nor will you get
// the fancy imm12 encoding found in most wide instructions.
//
// This is because we only have ~21 bits (compared to Thumb-2's ~28 bits).
//
// Classes are in octal, instruction data is usually in hex.
static void decode_wide_insn(tgolf_state *s, u32 insn)
{
    // We have 34 wide insn classes. That is because the swap register
    // instruction is commutative, so we encode it as 03XY where X > Y. If
    // X <= Y, it is a wide instruction
    switch ((insn >> 16) & 077) {
        case 000:
            decode_wide_000(s, insn);
            break;
        case 001:
            decode_wide_001(s, insn);
            break;
        case 002:
        case 003:
        case 006:
        case 007:
            decode_wide_divmodi(s, insn);
            break;
        case 004:
            decode_wide_io(s, insn);
            break;
        case 005:
        default: illegal(insn); // WIP
    }
}

// Ghetto mutex
#define HANDLER_STATE_NORMAL 0
#define HANDLER_STATE_RETURNING 1
#define HANDLER_STATE_BUSY 2
static volatile sig_atomic_t handler_state = HANDLER_STATE_NORMAL;

static volatile sig_atomic_t regs_buf[16];
static volatile sig_atomic_t cpsr_buf;

void tgolf_parse(void)
{
    // We don't want to do everything with volatiles. That is dumb.
    // Make a convenient local copy in an easy to parse struct.
    tgolf_state s;
    for (int i = 0; i < 16; i++) {
        s.r[i] = (u32)regs_buf[i];
    }
    s.cpsr.raw = (u32)cpsr_buf;

    u32 insn = *(u16 *)(s.r[REG_PC] & ~1);

    // Narrow instructions are 0xyz, where x is a 2 bit selector,
    // y is the operation (or register in a swap), z is the target
    // register.

    // Wide instructions have a 6 bit ID, starting with 03xy, but
    // it is actually 34 because x must be <= y. When x > y, it is a
    // commutative swap.
    switch (insn & 0300) {
        case 0000: { // SPIM-style I/O
            int id = (insn >> 3) & 7;
            switch (io_funcs[id].mode) {
                case MODE_REG_Z: {
                    u64 result = ((u64(*)(void))io_funcs[id].func)();
                    s.r[insn & 7] = (u32)result;
                    // set zero flag for success
                    s.cpsr.zero = result >> 32;
                    break;
                }
                case MODE_OUTPUT_REG: {
                    ((void(*)(u32))io_funcs[id].func)(s.r[insn & 7]);
                    break;
                }
                default: {
                    ((void(*)(u32))io_funcs[id].func)(insn);
                    break;
                }
            }
            break;
        } case 0100: { // narrow arithmetic
            switch (insn & 0070) {
                case 0000:  // adcs Rd, #0
                case 0010:  // sbcs Rd, #0
                    die("adcs/sbcs are coming soon.\n");
                case 0020: { // movs Rd, #-1
                    s.r[insn & 7] = -1;
                    s.cpsr.zero = 0;
                    s.cpsr.negative = 1;
                    s.cpsr.carry = 0;
                    s.cpsr.overflow = 0;
                    break;
                }
                case 0030: { // abs Rd
                    if ((s32)s.r[insn & 7] < 0) {
                        s.r[insn & 7] = -s.r[insn & 7];
                    }
                    break;
                }
                case 0040: { // udiv Rd, #10
                    s.r[insn & 7] /= 10;
                    break;
                }
                case 0050: { // umod Rd, #10
                    s.r[insn & 7] %= 10;
                    break;
                }
                case 0060: { // mul Rd, #10
                    s.r[insn & 7] *= 10;
                    break;
                }
                case 0070: { // rand Rd
                    s.r[insn & 7] = get_rand();
                    break;
                }
            }
            break;
        }
        case 0200: { // narrow swap with memory
            // swapm  Rd, [Rt]
            int Rt = insn & 7;
            int Rd = (insn >> 3) & 7;

            u32 *Mt = (u32 *)s.r[Rt];
            u32 tmp = *Mt;
            *Mt = s.r[Rd];
            s.r[Rd] = tmp;
            break;
        }
        case 0300: { // swap register or wide instruction
            // The function of this instruction depends on if Rn is
            // greater than Rm. If it is, it is a swap instruction.
            // If it isn't, it is a wide instruction.
            int Rn = (insn >> 3) & 7;
            int Rm = insn & 7;
            if (Rn > Rm) { // swap Rn, Rm
                u32 tmp = s.r[Rn];
                s.r[Rn] = s.r[Rm];
                s.r[Rm] = tmp;
            } else { // wide instruction
                u32 insn2 = ((const u16 *)(s.r[REG_PC]&~1))[1];
                u32 wide_insn = (((u32)insn & 077) << 16) | insn2;
                decode_wide_insn(&s, wide_insn);
                s.r[REG_PC] += 2; // add on to the PC advance so we skip 4 bytes
            }
            break;
        }
    }
    // Advance the PC.
    // Note that for wide instructions, we added 2 beforehand.
    s.r[REG_PC] += 2;

    // Restore the state
    for (int i = 0; i < 16; i++)
        regs_buf[i] = (sig_atomic_t)s.r[i];

    cpsr_buf = (sig_atomic_t)s.cpsr.raw;

    // Mark the handler state for stage 2.
    handler_state = HANDLER_STATE_RETURNING;
}

// The main entry point to the ThumbGolf runtime, triggered via sigaction.
//
// sigaction will pass the a snapshot of the processor, which we can modify.
// We use this to emulate the ThumbGolf extensions.
static void thumbgolf_handler(int signo, siginfo_t *si, void *data)
{
    ucontext_t *uc = (ucontext_t *)data;
    u32 *regs = &uc->uc_mcontext.arm_r0;
    arm_cpsr *cpsr = (arm_cpsr*)&uc->uc_mcontext.arm_cpsr;
    // only trap Thumb instructions
    if ((si->si_signo == SIGILL || si->si_signo == SIGTRAP) && cpsr->thumb) {
        u16 insn = *(const u16 *)(regs[REG_PC] &~1);

        // udf in IT blocks acts differently on hardware vs QEMU.
        // On hardware, udf will always trap, but QEMU skips it on false conditions
        // which is bad given how we encode wide instructions as a narrow instruction
        // and a trail.
        //
        // Additionally, udf in an IT block is technically deprecated, so the
        // inconsistency can only get worse.
        //
        // A possible workaround would be to require two IT cases for wide instructions,
        // but that would be pretty confusing to use. You already need to manually
        // encode the instruction.
        if (cpsr->it_cond) {
            die("ThumbGolf instructions are not supported in IT blocks.\n");
        }
        // udf.n: DExx
        // NOTE: Narrow instructions and wide opcodes are in octal, while
        // wide instructions are in hex.
        if ((insn & 0xFF00) == 0xDE00) {

            // Veneer into user mode. We can't actually do much from a signal
            // handler, thanks to the good stuff not being Async-Signal-Safe.
            //
            // This works by throwing SIGILL twice.
            // The first time, we save all registers to a buffer, then effectively
            // call tgolf_parse() by modifying the PC directly.
            // The return address is set to the offending instruction, but we
            // set a flag.
            //
            // The second time, we restore the registers and clear the flag.

            switch (handler_state) {
                case HANDLER_STATE_NORMAL: {
                    handler_state = HANDLER_STATE_BUSY;

                    // Save the registers to our buffer to access them from the
                    // main thread.
                    for (int i = 0; i < 16; i++)
                        regs_buf[i] = (sig_atomic_t)regs[i];

                    cpsr_buf = (sig_atomic_t)cpsr->raw;

                    // Simulate a bl to tgolf_parse.

                    // Set the return address to the offending instruction, so
                    // upon returning from tgolf_parse(), we will throw SIGILL again.
                    regs[REG_LR] = regs[REG_PC] | 1; // Thumb bit

                    // And jump to tgolf_parse by modifying the PC.
                    // We must clear the Thumb bit. It doesn't make a difference on hardware
                    // but it is against the spec and causes issues on QEMU.
                    regs[REG_PC] = (u32)(&tgolf_parse) & ~1;
                    break;
                }
                case HANDLER_STATE_RETURNING: {
                    handler_state = HANDLER_STATE_BUSY;

                    // Restore the modified register set
                    for (int i = 0; i < 16; i++)
                        regs[i] = (u32)regs_buf[i];

                    cpsr->raw = (u32)cpsr_buf;

                    // Clear the flag.
                    handler_state = HANDLER_STATE_NORMAL;
                    break;
                }
                default: { // handler during handling?!
                    die(
                       "Double ThumbGolf exception detected!\n"
                       "Note that multiple threads are not supported at this time.\n"
                    );
                }
            }
        // bkpt: BExx
        // When xx == 0, exits the program.
        // Otherwise, does a "relative bl". Doesn't exchange instruction sets.
        } else if ((insn & 0xFF00) == 0xBE00) {
            // Sign extend
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
        // TODO: IT blocks
        }  else if ((insn & 0xFFD0) == 0xFB90) {
              u32 insn2 = ((const u16 *)(uc->uc_mcontext.arm_pc & ~1))[1];
              if ((insn2 & 0x00F0) == 0x00F0) {
                  if (cpsr->it_cond) {
                      die("Division emulation in IT blocks is currently not supported.");
                  }
                  // To be ABSOLUTELY safe, forcibly call libgcc.
                  // The last thing we want is an infinite loop.
                  if (insn & 0x0020) { // udiv
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
                  illegal((insn2 << 16) | insn);
              }
        } else {
            illegal(insn);
        }
        // TODO: udf.w? That is only 12 real bits, while udf.n followed by an
        // arbitrary halfword is MUCH better.
        // If we run out of instructions, I might consider it.
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
// Yes, this can be static. No symbols are exported from this file. :)
__attribute__((constructor))
static void thumbgolf_inject(void)
{
    // Set stdout, stdin, and stderr to unbuffered mode to avoid conflicts
    // with manual syscalls.
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    // Set up the signal handler
    struct sigaction sa = {0}, osa = {0};
    sa.sa_flags =  SA_ONSTACK | SA_RESTART | SA_NODEFER | SA_SIGINFO;
    sa.sa_sigaction = thumbgolf_handler;
    sigaction(SIGILL, &sa, &osa);
    sigaction(SIGTRAP, &sa, &osa);
}
