#ifndef __V0_TYPES_H__
#define __V0_TYPES_H__

#include <v0/conf.h>
#include <stdint.h>
#include <v0/gpu.h>

typedef int32_t    v0reg;     	// signed register value
typedef uint32_t   v0ureg;    	// unsigned register value
typedef int64_t    v0wreg;    	// signed wide-register value
typedef uint64_t   v0uwreg;   	// unsigned wide-register value
typedef v0ureg     v0adr;     	// machine memory address
typedef char      *v0ptr8;    	// 8-bit pointer
typedef uint8_t   *v0ptru8;   	// 8-bit unsigned pointer
typedef void      *v0ptr;     	// generic pointer
typedef intptr_t   v0memofs;  	// signed pointer value
typedef uintptr_t  v0memadr;    // unsigned pointer value

typedef uint32_t   v0trapdesc;  // interrupt/exception/abort/fault spec
typedef uint32_t   v0pagedesc;  // [virtual] memory page descriptor

typedef uint32_t   v0ioperm;    // I/O-permission bits
typedef uint32_t   v0iodesc;    // I/O-device page address + flags

typedef long v0vmopfunc_t(struct vm *vm, void *op); // virtual machine operation

/*
 * 32-bit little-endian argument parcel
 * - declared as union, 32-bit aligned
 */
union v0arg32 {
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

#define v0getreg1(ins)                                                  \
    ((ins)->regs & 0x0f)
#define v0getreg2(ins)                                                  \
    ((ins)->regs >> 4)
#define v0readreg1(vm, ins)                                             \
    ((vm)->regs[(ins)->regs & V0_INS_REG_MASK])
#define v0readreg2(vm, ins)                                             \
    ((vm)->regs[(ins)->regs >> V0_INS_REG_BITS])
#define v0setreg1(ins, id)                                              \
    ((ins)->regs &= ~0x0f, (ins)->regs |= (id))
#define v0setreg2(ins, id)                                              \
    ((ins)->regs &= 0x0f, (ins)->regs |= ((id) << 4))
#define v0getadr1(vm, ins)                                              \
    (((ins)->parm & V0_ADR_MASK == V0_REG_ADR)                          \
     ? (v0getreg1(vm, ins))                                             \
     : ((v0getflg(ins) & V0_NDX_ADR)                                    \
        ? (v0getreg1(vm, ins) + v0getofs(vm, ins))                      \
        : ((vm)->regs[V0_PC_REG] + sizeof(struct v0ins) * v0getval(ins))))
#define v0getadr2(vm, ins)                                              \
    (((ins)->parm & V0_ADR_MASK == V0_REG_ADR)                          \
     ? (v0getreg2(vm, ins))                                             \
     : ((v0getflg(ins) & V0_NDX_ADR)                                    \
        ? (v0getreg2(vm, ins) + v0getofs(vm, ins))                      \
        : ((vm)->regs[V0_PC_REG] + sizeof(struct v0ins) * v0getval(ins))))
#define v0getop(ins)   ((ins)->code & V0_INS_OP_MASK)
#define v0getunit(ins) ((ins)->code >> V0_INS_OP_BITS)
#define v0getop(ins)   ((ins)->code & V0_INS_OP_MASK)
#define v0getflg(ins)  ((ins)->parm 0xf000)
#define v0getval(ins)  (((ins)->parm & 0x0800)                          \
                        ? (-((ins)->parm & 0x07ff) - 1)                 \
                        : ((ins)->parm & 0x07ff))
#define v0getofs(ins)  ((ins)->arg[0].ofs)
struct v0ins {
    uint8_t       code;                 // unit + instruction IDs
    uint8_t       regs;                 // register IDs
    uint16_t      parm;                 // address-mode bits + 13-bit immediate
    union v0arg32 arg[VLA];             // immediate if (ins->parm & V0_IMM_BIT)
};

/*
 * CSR: psm $0x0207; mkf $n, tab;       // $n # of function args
 * BEG: psx %fp; ldx %sp, %fp; psm $0x080f;
 * FIN: ldx %fp, %sp; pox %fp; ret
 * RET: ldx %fp, %sp; pom $0x0207; pox %fp
 */

/* stack pointer points to this after CSR + BEG */
struct v0localvars {
    v0reg tab[VLA];                     // local [automatic] variables
};

/* callee-save structure */
struct v0calleeframe {
    v0reg r8;                           // registers R8..R15
    v0reg r9;
    v0reg r10;
    v0reg r11;
    v0reg r12;
    v0reg r13;
    v0reg r14;
    v0reg r15;
    v0reg fp;                           // caller frame pointer         <- FP
    v0reg args[VLA];                    // stack parameters to callee   <- SP
};

/* caller-save registers */
struct v0callregs {
    /* r1 is return value */
    v0reg r2;                                                           <- FP
    v0reg r3;
    v0reg r4;
    v0reg r5;
    v0reg r6;
    v0reg r7;
};

/*
 * CALL STACK
 * ----------
 *

/*
 * NOTES
 * ------
 * - return address is stored in the link register %lr (%r15)
 * - number of stack arguments is stored in %ac
 */
struct v0calleectx {
    v0reg             fp;               // copy of FP (frame pointer)
    struct v0callregs regs;             // caller-save register
    v0reg             args[VLA];        // args followed by struct v0calleeregs
};

#endif /* __V0_TYPES_H__ */

