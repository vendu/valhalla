#ifndef __V0_TYPES_H__
#define __V0_TYPES_H__

#include <v0/conf.h>
#include <stdint.h>

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

typedef long v0vmopfunc_t(struct vm *vm, void *op); // virtual machine operation

struct v0 {
    v0wreg   regs[V0_STD_REGS];
    uint8_t *mem;
};

/*
 * 32-bit little-endian argument parcel
 * - declared as union, 32-bit aligned
 */
union v0imm32 {
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

#define V0_ATOM_BIT 0x01        // atomic [bus-locking] instruction
#define V0_IMM_BIT  0x02        // immediate argument follows opcode
/* addressing modes */
/* register addressing is detected with nonzero (op->reg) */
#define V0_NO_ADR   0x04        // register operands
#define V0_REG_ADR  0x08        // base address in register
#define V0_PIC_ADR  0x10        // PC-relative, i.e. x(%pc) for shared objects
#define V0_NDX_ADR  0x20        // indexed, i.e. %r1(%r2) or $c1(%r2)
#define V0_BYTE_BIT 0x40        // 8-bit operand
#define V0_HALF_BIT 0x80        // 16-bit operand
/* else indexed: pc[ndx << op->parm], ndx follows opcode */
struct v0ins {
    uint8_t       flg;          // instruction flags
    uint8_t       unit;         // unit ID
    uint8_t       op;           // instruction ID
    uint8_t       regs;         // register IDs
    union v0arg32 imm[VLA];     // immediate argument if present
};

struct v0callctx {
    v0reg fp; // copy of FP (frame pointer)
    v0reg r1; // registers R1 through R7
    v0reg r2;
    v0reg r3;
    v0reg r4;
    v0reg r5;
    v0reg r6;
    v0reg r7;
    v0reg ln; // return address back to caller
};

#endif /* __V0_TYPES_H__ */

