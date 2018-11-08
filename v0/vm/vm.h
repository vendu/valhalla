#ifndef __V0_VM_VM_H__
#define __V0_VM_VM_H__

/* VIRTUAL MACHINE */

#include <v0/vm/conf.h>
#include <stdio.h>
#include <stdint.h>
#include <endian.h>
#include <valhalla/cdefs.h>

struct v0;

typedef int64_t  v0wreg; // full-width register (temporary values)
typedef int32_t  v0reg;  // signed user-register type
typedef uint32_t v0ureg; // unsigned user-register type
typedef v0ureg   v0memadr; // memory address
typedef v0ureg   v0pagedesc;
typedef void     v0iofunc_t(struct v0 *vm, uint8_t port, v0reg reg);

struct v0iofuncs {
    v0iofunc_t *rdfunc;
    v0iofunc_t *wrfunc;
};

#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define v0setloval(op, val)                                             \
    ((op)->val = ((op)->val & 0x3e) | (val))
#define v0sethival(op, val)                                             \
    ((op)->val = ((op)->val & 0x1f) | ((val) << 5))
#else
#define v0setloval(op, val)                                             \
    ((op)->val = ((op)->val & 0x1f) | ((val) << 5))
#define v0sethival(op, val)                                             \
    ((op)->val = ((op)->val & 0x3e) | (val))
#endif

#define V0_RET_REG       V0_R0_REG
#define V0_AC_REG        V0_R6_REG
#define V0_VC_REG        V0_R7_REG
/* CALLER-SAVE REGISTERS */
#define V0_R0_REG        0x00 // function return value, first function argument
#define V0_R1_REG        0x01 // second function argument
#define V0_R2_REG        0x02 // third function argument
#define V0_R3_REG        0x03 // fourth function argument
#define V0_R4_REG        0x04 // fifth function argument
#define V0_R5_REG        0x05 // sixth function argument
#define V0_R6_REG        0x06 // stack-argument count
#define V0_R7_REG        0x07 // stack-variable count
/* CALLEE-SAVE REGISTERS */
#define V0_R8_REG        0x08
#define V0_R9_REG        0x09
#define V0_R10_REG       0x0a
#define V0_R11_REG       0x0b
#define V0_R12_REG       0x0c
#define V0_R13_REG       0x0d
#define V0_R14_REG       0x0e
#define V0_R15_REG       0x0f
#define V0_INT_REGS      16 // # of integer/scalar registers
#define V0_SAVE_REGS     8  // caller saves r0..r7, callee r8..r15
/* SYSTEM REGISTERS */
#define V0_PC_REG        (V0_INT_REGS + 0) // program counter
#define V0_FP_REG        (V0_INT_REGS + 1) // frame pointer
#define V0_SP_REG        (V0_INT_REGS + 2) // stack pointer
#define V0_RTA_REG       (V0_INT_REGS + 3) // return address
#define V0_MSW_REG       (V0_INT_REGS + 4) // machine status word
#define V0_IMR_REG       (V0_INT_REGS + 5) // interrupt-mask (1-bit for enabled)
#define V0_IVR_REG       (V0_INT_REGS + 6) // interrupt vector address
#define V0_PDR_REG       (V0_INT_REGS + 7) // page directory address
#define V0_THR_REG       (V0_INT_REGS + 8) // thread ID + permission flags
/* READ-ONLY REGISTERS */
#define V0_MFW_REG       (V0_INT_REGS + V0_SYS_REGS - 1) // machine feature word
#define V0_SYS_REGS      16
/* system register IDs */
#define V0_STD_REGS      (V0_INT_REGS + V0_SYS_REGS) // total number of user and system registers
// shadow registers are used for function and system calls instead of stack
#define V0_SREG(x)       (V0_STD_REGS + (x)) // shadow-registers
/* values for regs[V0_MSW] */
#define V0_MSW_DEF_BITS  (V0_IF_BIT)
#define V0_MSW_ZF_BIT    (1U << 0)
#define V0_MSW_OF_BIT    (1U << 1)  // overflow
#define V0_MSW_CF_BIT    (1U << 2)  // carry-flag, return bit for BTR, BTS, BTC
#define V0_MSW_IF_BIT    (1U << 3)  // interrupts enabled
#define V0_MSW_MF_BIT    (1U << 31) // memory-bus lock-flag
/* program segments */
#define V0_TRAP_SEG      0x00
#define V0_CODE_SEG      0x01 // code
#define V0_RODATA_SEG    0x02 // read-only data such as literals
#define V0_DATA_SEG      0x03 // read-write (initialised) data
#define V0_KERN_SEG      0x04 // code to implement system [call] interface
#define V0_STACK_SEG     0x05
#define V0_SEGS          8
#if 0
/* option-bits for flg-member */
#define V0_TRACE         0x01
#define V0_PROFILE       0x02
#endif

#define v0clrmsw(vm)     ((vm)->regs[V0_MSW_REG] = 0)
#define v0setcf(vm)      ((vm)->regs[V0_MSW_REG] |= V0_MSW_CF_BIT)
#define v0setzf(vm)      ((vm)->regs[V0_MSW_REG] |= V0_MSW_ZF_BIT)
#define v0setof(vm)      ((vm)->regs[V0_MSW_REG] |= V0_MSW_OF_BIT)
#define v0setif(vm)      ((vm)->regs[V0_MSW_REG] |= V0_MSW_IF_BIT)
#define v0cfset(vm)      ((vm)->regs[V0_MSW_REG] & V0_MSW_CF_BIT)
#define v0zfset(vm)      ((vm)->regs[V0_MSW_REG] & V0_MSW_ZF_BIT)
#define v0ofset(vm)      ((vm)->regs[V0_MSW_REG] & V0_MSW_OF_BIT)
#define v0ifset(vm)      ((vm)->regs[V0_MSW_REG] & V0_MSW_IF_BIT)

struct v0seg {
    v0ureg   id;
    v0memadr base;
    v0memadr lim;
    v0ureg   perm;
};

struct v0 {
    v0reg             regs[V0_INT_REGS + V0_SYS_REGS];
  //    struct v0seg      segs[V0_SEGS];
    long              flg;
    v0pagedesc       *membits;
    char             *mem;
    struct v0iofuncs *iovec;
    FILE             *vtdfp;
    char             *vtdpath;
    struct divuf16   *divu16tab;
};

/* OPCODES */

/*
 * 32-bit little-endian argument parcel
 * - declared as union for 32-bit alignment of all arguments
 */

union v0oparg {
    v0ureg   adr;
    v0ureg   uval;
    v0reg    val;
    v0reg    ofs;
    int32_t  i32;
    uint32_t u32;
    int16_t  i16;
    uint16_t u16;
    int8_t   i8;
    uint8_t  u8;
};

#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define v0mkopid(unit, inst) ((uint8_t)((unit) | ((inst) << 4)))
#define v0getunit(code)      ((code) & 0x0f)
#define v0getinst(code)      ((code) >> 4)
#else
#define v0mkopid(unit, inst) ((uint8_t)(((unit) << 4) | (inst)))
#define v0getunit(code)      ((code) >> 4)
#define v0getop(code)        ((code) & 0x0f)
#endif
#define v0adrtoptr(vm, adr)  ((void *)(&(vm)->mem[(adr)]))
#define v0regtoptr(vm, reg)  ((void *)(&(vm)->regs[(reg)]))

#define V0_REG_BIT       (1 << V0_REG_ADR)
#define V0_DIR_BIT       (1 << V0_DIR_ADR)
#define V0_IMM_BIT       (1 << V0_IMM_ADR)
#define V0_NDX_BIT       (1 << V0_NDX_ADR)
#define V0_R_ARG         V0_REG_BIT
#define V0_I_ARG         (V0_IMM_BIT | V0_DIR_BIT)
#define V0_M_ARG         V0_NDX_BIT
#define V0_RI_ARG        (V0_R_ARG | V0_I_ARG)
#define V0_RIM_ARG       (V0_R_ARG | V0_I_ARG | V0_M_ARG)
#define V0_RM_ARG        (V0_R_ARG | V0_M_ARG)

/* addressing modes */
#define V0_REG_ADR       0x00 // %reg, argument in register
#define V0_DIR_ADR       0x01 // $val, address follows opcode
#define V0_IMM_ADR       0x02 // $val, address in val-field
#define V0_NDX_ADR       0x03 // ndx(%reg) << op->parm], ndx follows opcode
#define V0_PIC_ADR       V0_IMM_ADR // PC-relative addressing-mode
/* parm-field */
#define V0_TRAP_BIT      0x01 // breakpoint
#define V0_AUX_BIT       0x04 // reserved for per-instruction flags
/* val-field */
#define V0_IMM_VAL_MAX   0xff
#define V0_IMM_VAL_MIN   (-0x7f - 1)

/* NOP is declared as all 1-bits */
#define V0_NOP_CODE      (~UINT32_C(0))
#define v0opisnop(op)    (*(uint32_t *)(op) == V0_NOP_CODE)
#define V0_SIGNED_BIT    (1 << 3)
#define v0opissigned(op) ((op)->parm & V0_SIGNED_BIT)

/* attributes for the flg-field */
#define V0_BUSLK_BIT     (1 << 3) // lock bus/cacheline for operation
#define V0_PICADR_BIT    (1 << 2) // address is relative to PC
/* I/O-operations always have a single register argument */
struct v0op {
    unsigned int   code : 8; // unit and instruction IDs
    unsigned int   reg1 : 4; // register argument #1 ID
    unsigned int   reg2 : 4; // register argument #2 ID
    unsigned int   adr  : 2; // addressing mode
    unsigned int   parm : 2; // address scale shift count
    unsigned int   flg  : 4; // instruction flags
    unsigned int   val  : 8; // immediate value; shift count, register range
    union v0oparg  arg[VLA]; // possible argument value
};

/* memory parameters */
#define V0_MEM_TRAP      0x00   // traditionally, interrupt-vector @ 0x00000000
#define V0_MEM_EXEC      0x01   // execute-permission
#define V0_MEM_WRITE     0x02   // write-permission
#define V0_MEM_READ      0x04   // read-permission
#define V0_MEM_PRESENT   0x08   // memory present in physical core
#define V0_MEM_MAP       0x10   // memory may be mapped across multiple users
#define V0_MEM_SYS       0x20   // system code
#define V0_MEM_TLS       0x40   // thread-local storage
#define V0_MEM_STACK     0x80   // segment grows downward in core

#define V0_VTD_PATH      "vtd.txt"
/* predefined I/O ports */
#define V0_STDIN_PORT    0 // keyboard input
#define V0_STDOUT_PORT   1 // console or framebuffer output
#define V0_STDERR_PORT   2 // console or framebuffer output
#define V0_RTC_PORT      3 // real-time clock
#define V0_TMR_PORT      4 // high-resolution timer for profiling
#define V0_MOUSE_PORT    5 // mouse input
#define V0_VTD_PORT      6 // virtual tape drive
#define V0_MAP_PORT      7 // memory-mapped device control

/* framebuffer graphics interface */
#define V0_FB_BASE       (3UL * 1024 * 1024 * 1024)      // base address

/* traps (exceptions and interrupts) - lower number means higher priority */

#define V0_NTRAP         256

/* USER [programmable] traps */
#define v0trapisuser(t)  (((t) & V0_SYS_TRAP_BIT) == 0)
#define v0trapissys(t)   ((t) & V0_SYS_TRAP_BIT)
#define V0_BREAK_POINT   0x00 // debugging breakpoint; highest priority
#define V0_TMR_INTR      0x01 // timer interrupt
#define V0_KBD_INTR      0x02 // keyboard
#define V0_PTR_INTR      0x03 // mouse, trackpad, joystick, ...
#define V0_PAGE_FAULT    0x04 // reserved for later use (paging); adr | bits
#define V0_FAST_INTR     0x1f // fast interrupts
#define V0_SYS_TRAP_BIT  0x20 // denotes system interrupts
#define V0_SYS_TRAP_MAX  0x3f // maximum system interrupt number
#define V0_TRAPS         64
#define V0_USR_TRAP_MASK 0x1f

/* SYSTEM TRAPS */
/* aborts */
#define V0_ABORT_TRAP    0x20 // traps that terminate the process
/* memory-related violations */
#define V0_STACK_FAULT   0x10 // stack segment limits exceeded; adr
#define V0_TEXT_FAULT    0x10 // invalid address for instruction; adr
#define V0_INV_MEM_READ  0x11 // memory read error; push address
#define V0_INV_MEM_WRITE 0x12 // memory write error
#define V0_INV_MEM_ADR   0x13 // invalid memory address; segment violation
/* instruction-related problems */
/* instruction format violations - terminate process */
#define V0_INV_OP_CODE   0x20 // invalid operation; code
#define V0_INV_OP_ARG    0x21 // invalid argument; (type << 1) | num
#define V0_INV_OP_ADR    0x22 // invalid addressing-mode for instruction
#define V0_INV_OP_
/* I/O-related exceptions */
#define V0_IO_TRAP       0x20 // I/O traps
#define V0_INV_IO_READ   0x20 // no permission to read from input port; port
#define V0_INV_IO_WRITE  0x21 // no permission to write to input port; port
/* programmatic errors */
#define V0_PROG_TRAP     0x30
#define V0_DIV_BY_ZERO   0x30 // division by zero - terminate process

/* debugging */

struct v0opinfo {
    char *unit;
    char *op;
    char *func;
};

struct v0 * v0init(struct v0 *vm);
void        v0disasm(struct v0 *vm, struct v0op *op, v0ureg pc);

#endif /* __V0_VM_VM_H__ */

