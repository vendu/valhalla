#ifndef __V0_VM_OP_H__
#define __V0_VM_OP_H__

/* FIXME: make this file work */

#include <v0/vm/conf.h>
#include <stddef.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <endian.h>
//#include <zero/param.h>
//#include <zero/cdefs.h>
//#include <zero/trix.h>
//#include <zero/fastudiv.h>
#include <valhalla/param.h>
#include <vas/vas.h>
#include <v0/vm/isa.h>
#include <v0/vm/vm.h>

#define V0_HALTED                   0xffffffffU // special PC-value
#define V0_OP_INVAL                 NULL
#define V0_ADR_INVAL                0x00000000
#define V0_CNT_INVAL                (-1)
#if !defined(__GNUC__)
#define _V0OPFUNC_T FASTCALL INLINE uint32_t
#else
#define _V0OPFUNC_T INLINE          uint32_t
#endif

#if defined(__GNUC__)
#define _v0opadr(x) &&v0op##x
#define _V0OPTAB_T  void *
#else
#define _v0opadr(x) v0##x
typedef v0reg       v0opfunc(struct v0 *vm, uint8_t *ptr, v0ureg pc);
#define _V0OPTAB_T  v0opfunc *
#endif

#if defined(V0_GAME)
#define v0addspeedcnt(n)            (v0speedcnt += (n))
#else
#define v0addspeedcnt(n)
#endif

extern char     *v0opnametab[V0_NINST_MAX];
extern vasuword  _startadr;

#define v0doxcpt(xcpt)                                                  \
    v0procxcpt(xcpt, __FILE__, __FUNCTION__, __LINE__)
static __inline__ uintptr_t
v0procxcpt(const int xcpt, const char *file, const char *func, const int line)
{
#if defined(_VO_PRINT_XCPT)
    fprintf(stderr, "EXCEPTION: %s - %s/%s:%d\n", #xcpt, file, func, line);
#endif

    exit(xcpt);
}

#if defined(V0_DEBUG_TABS) && 0
#define opaddinfo(proc, inst, handler)                                  \
    do {                                                                \
        long _code = v0mkopid(proc, inst);                              \
                                                                        \
        v0opinfotab[_code].unit = strdup(#proc);                        \
        v0opinfotab[_code].op = strdup(#inst);                          \
        v0opinfotab[_code].func = strdup(#handler);                     \
    } while (0)
#else
#define opaddinfo(unit, op)
#endif

#define v0traceop(vm, op, pc) v0disasm(vm, op, pc);
#define v0opisvalid(vm, pc) (vm)
#if defined(__GNUC__)
#define opjmp(vm, pc)                                                   \
    do {								\
        struct v0op *_op = v0adrtoptr(vm, pc);				\
                                                                        \
        if (pc == V0_HALTED) {                                          \
                                                                        \
            exit(1);                                                    \
        }                                                               \
	if (v0opisvalid(_op, pc)) {					\
            v0disasm(vm, _op, pc);                                      \
	    goto *jmptab[(_op)->code];					\
	} else {							\
	    v0doxcpt(V0_TEXT_FAULT);					\
	  								\
	    return V0_TEXT_FAULT;					\
	}								\
    } while (0)
#if 0
#define opjmp(vm, pc)                                                   \
    do {								\
        struct v0op *_op = v0adrtoptr(vm, pc);				\
									\
	while (v0opisnop(_op)) {					\
	    (pc) += sizeof(struct v0op);				\
	}								\
	if (v0opisvalid(_op, pc)) {					\
	    if (_op->adr == V0_DIR_ADR || _op->adr == V0_NDX_ADR) {	\
	      (pc) += sizeof(struct v0op) + sizeof(union v0oparg);	\
	    } else {							\
	      (pc) += sizeof(struct v0op);				\
	    }								\
	    goto *jmptab[(_op)->code];					\
	    vm->regs[V0_PC_REG] = (pc);					\
	} else {							\
	    v0doxcpt(V0_TEXT_FAULT);					\
	  								\
	    return V0_TEXT_FAULT;					\
	}								\
    } while (0)
#endif
#endif /* defined(__GNUC__) */
#define v0addop(op, str, narg)                                          \
    (vasaddop(#str, op, narg))
#define v0setop(op, str, narg, tab)                                     \
    (v0opnametab[(op)] = #str,                                          \
     ((_V0OPTAB_T *)(tab))[(op)] = _v0opadr(str))

#define v0setopbits(op, bits1, bits2, tab)                              \
    ((tab)[(op)] = (bits1) | ((bits2 << 16)))
#define v0getopbits1(op, tab) ((tab)[(op)] & 0xffff)
#define v0getopbits2(op, tab) (((tab)[(op)] >> 16) & 0xffff)

#define v0initopbits(tab)                                               \
    do {                                                                \
	v0setopbits(V0_INC, V0_R_ARG, 0, tab);				\
	v0setopbits(V0_DEC, V0_R_ARG, 0, tab);				\
	v0setopbits(V0_CMP, V0_RI_ARG, V0_R_ARG, tab);			\
	v0setopbits(V0_ADD, V0_RI_ARG, V0_R_ARG, tab);			\
	v0setopbits(V0_ADC, V0_RI_ARG, V0_R_ARG, tab);			\
	v0setopbits(V0_SUB, V0_RI_ARG, V0_R_ARG, tab);			\
	v0setopbits(V0_SBC, V0_RI_ARG, V0_R_ARG, tab);			\
	v0setopbits(V0_SHL, V0_RI_ARG, V0_R_ARG, tab);			\
	v0setopbits(V0_SHR, V0_RI_ARG, V0_R_ARG, tab);			\
	v0setopbits(V0_SAR, V0_RI_ARG, V0_R_ARG, tab);			\
	v0setopbits(V0_NOT, V0_RI_ARG, V0_R_ARG, tab);			\
	v0setopbits(V0_AND, V0_RI_ARG, V0_R_ARG, tab);			\
	v0setopbits(V0_XOR, V0_RI_ARG, V0_R_ARG, tab);			\
	v0setopbits(V0_IOR, V0_RI_ARG, V0_R_ARG, tab);			\
	v0setopbits(V0_CRP, V0_RI_ARG, V0_R_ARG, tab);			\
	v0setopbits(V0_MUL, V0_RI_ARG, V0_R_ARG, tab);			\
	v0setopbits(V0_MUL, V0_RI_ARG, V0_R_ARG, tab);			\
	v0setopbits(V0_LDR, V0_RIM_ARG, V0_R_ARG, tab);			\
	v0setopbits(V0_STR, V0_RI_ARG, V0_RM_ARG, tab);			\
	v0setopbits(V0_PSH, V0_RI_ARG, V0_R_ARG, tab);			\
        v0setopbits(V0_PSM, V0_RI_ARG, 0, tab);                         \
        v0setopbits(V0_POP, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_POM, V0_RI_ARG, 0, tab);                         \
        v0setopbits(V0_JMP, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_JMR, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_BIZ, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_BNZ, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_BLT, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_BLE, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_BGT, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_BGE, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_BIO, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_BNO, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_BIC, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_BNC, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_CSR, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_BEG, V0_RI_ARG, V0_RIM_ARG, tab);                \
        v0setopbits(V0_FIN, V0_RI_ARG, V0_RIM_ARG, tab);                \
        v0setopbits(V0_RET, V0_RIM_ARG, 0, tab);                        \
        v0setopbits(V0_IRD, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_IWR, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_ICF, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_CLI, V0_M_ARG, 0, tab);                          \
        v0setopbits(V0_STI, V0_M_ARG, 0, tab);                          \
    } while (0)

#define v0initops(tab)                                                  \
    do {                                                                \
        v0setop(V0_NOP, nop, 0, tab);                                   \
        v0setop(V0_INC, inc, 1, tab);                                   \
        v0setop(V0_DEC, dec, 1, tab);                                   \
        v0setop(V0_CMP, cmp, 2, tab);                                   \
        v0setop(V0_ADD, add, 2, tab);                                   \
        v0setop(V0_ADC, adc, 2, tab);                                   \
        v0setop(V0_SUB, sub, 2, tab);                                   \
        v0setop(V0_SBC, sbc, 2, tab);                                   \
        v0setop(V0_SHL, shl, 2, tab);                                   \
        v0setop(V0_SHR, shr, 2, tab);                                   \
        v0setop(V0_SAR, sar, 2, tab);                                   \
        v0setop(V0_NOT, not, 1, tab);                                   \
        v0setop(V0_AND, and, 2, tab);                                   \
        v0setop(V0_XOR, xor, 2, tab);                                   \
        v0setop(V0_IOR, ior, 2, tab);                                   \
        v0setop(V0_CRP, crp, 1, tab);                                   \
        v0setop(V0_MUL, mul, 2, tab);                                   \
        v0setop(V0_MUL, muh, 2, tab);                                   \
        v0setop(V0_LDR, ldr, 2, tab);                                   \
        v0setop(V0_STR, str, 2, tab);                                   \
        v0setop(V0_PSH, psh, 1, tab);                                   \
        v0setop(V0_PSM, psm, 1, tab);                                   \
        v0setop(V0_POP, pop, 1, tab);                                   \
        v0setop(V0_POM, pom, 1, tab);                                   \
        v0setop(V0_JMP, jmp, 1, tab);                                   \
        v0setop(V0_JMR, jmr, 1, tab);                                   \
        v0setop(V0_BIZ, biz, 1, tab);                                   \
        v0setop(V0_BEQ, beq, 1, tab);                                   \
        v0setop(V0_BIZ, biz, 1, tab);                                   \
        v0setop(V0_BNE, bne, 1, tab);                                   \
        v0setop(V0_BNZ, bnz, 1, tab);                                   \
        v0setop(V0_BLT, blt, 1, tab);                                   \
        v0setop(V0_BLE, ble, 1, tab);                                   \
        v0setop(V0_BGT, bgt, 1, tab);                                   \
        v0setop(V0_BGE, bge, 1, tab);                                   \
        v0setop(V0_BIO, bio, 1, tab);                                   \
        v0setop(V0_BNO, bno, 1, tab);                                   \
        v0setop(V0_BIC, bic, 1, tab);                                   \
        v0setop(V0_BNC, bnc, 1, tab);                                   \
        v0setop(V0_CSR, csr, 1, tab);                                   \
        v0setop(V0_BEG, beg, 2, tab);                                   \
        v0setop(V0_FIN, fin, 2, tab);                                   \
        v0setop(V0_RET, ret, 1, tab);                                   \
        v0setop(V0_IRD, ird, 2, tab);                                   \
        v0setop(V0_IWR, iwr, 2, tab);                                   \
        v0setop(V0_ICF, icf, 2, tab);                                   \
        v0setop(V0_CLI, cli, 1, tab);                                   \
        v0setop(V0_STI, sti, 1, tab);                                   \
        v0setop(V0_SLP, slp, 0, tab);                                   \
        v0setop(V0_RST, rst, 0, tab);                                   \
        v0setop(V0_HLT, hlt, 0, tab);                                   \
    } while (0)

#define v0addops()                                                      \
    do {                                                                \
        v0addop(V0_NOP, nop, 0);                                        \
        v0addop(V0_INC, inc, 1);                                        \
        v0addop(V0_DEC, dec, 1);                                        \
        v0addop(V0_CMP, cmp, 2);                                        \
        v0addop(V0_ADD, add, 2);                                        \
        v0addop(V0_ADC, adc, 2);                                        \
        v0addop(V0_SUB, sub, 2);                                        \
        v0addop(V0_SBC, sbc, 2);                                        \
        v0addop(V0_SHL, shl, 2);                                        \
        v0addop(V0_SHR, shr, 2);                                        \
        v0addop(V0_SAR, sar, 2);                                        \
        v0addop(V0_NOT, not, 1);                                        \
        v0addop(V0_AND, and, 2);                                        \
        v0addop(V0_XOR, xor, 2);                                        \
        v0addop(V0_IOR, ior, 2);                                        \
        v0addop(V0_CRP, crp, 2);                                        \
        v0addop(V0_MUL, mul, 2);                                        \
        v0addop(V0_MUL, muh, 2);                                        \
        v0addop(V0_LDR, ldr, 2);                                        \
        v0addop(V0_STR, str, 2);                                        \
        v0addop(V0_PSH, psh, 1);                                        \
        v0addop(V0_PSM, psm, 1);                                        \
        v0addop(V0_POP, pop, 1);                                        \
        v0addop(V0_POM, pom, 1);                                        \
        v0addop(V0_JMP, jmp, 1);                                        \
        v0addop(V0_JMR, jmr, 1);                                        \
        v0addop(V0_BIZ, biz, 1);                                        \
        v0addop(V0_BEQ, beq, 1);                                        \
        v0addop(V0_BNZ, bnz, 1);                                        \
        v0addop(V0_BNE, bne, 1);                                        \
        v0addop(V0_BLT, blt, 1);                                        \
        v0addop(V0_BLE, ble, 1);                                        \
        v0addop(V0_BGT, bgt, 1);                                        \
        v0addop(V0_BGE, bge, 1);                                        \
        v0addop(V0_BIO, bio, 1);                                        \
        v0addop(V0_BNO, bno, 1);                                        \
        v0addop(V0_BIC, bic, 1);                                        \
        v0addop(V0_BNC, bnc, 1);                                        \
        v0addop(V0_CSR, csr, 1);                                        \
        v0addop(V0_BEG, beg, 2);                                        \
        v0addop(V0_FIN, fin, 2);                                        \
        v0addop(V0_RET, ret, 1);                                        \
        v0addop(V0_IRD, ird, 2);                                        \
        v0addop(V0_IWR, iwr, 2);                                        \
        v0addop(V0_ICF, icf, 2);                                        \
        v0addop(V0_CLI, cli, 1);                                        \
        v0addop(V0_STI, sti, 1);                                        \
        v0addop(V0_SLP, slp, 0);                                        \
        v0addop(V0_RST, rst, 0);                                        \
        v0addop(V0_HLT, hlt, 0);                                        \
    } while (0)

#define v0calcadr(reg, ofs, shift)                                      \
    ((reg) + ((ofs) << (shift)))
#define v0calcjmp(reg, ofs, shift)                                      \
    ((reg) + ((ofs) << (2 + (shift))))
#define v0fetcharg(vm, reg, ofs, shift)                                 \
    (*(v0reg *)(&(vm)->mem[v0calcadr(reg, ofs, shift)]))
#define v0getarg1(vm, op)                                               \
    (((op)->adr == V0_REG_ADR)                                          \
     ? (*(v0reg *)(&(vm)->mem[(vm)->regs[(op)->reg1]]))                 \
     : (((op)->adr == V0_IMM_ADR)                                       \
        ? (*(v0reg *)(&(vm)->mem[(op)->val]))                           \
        : (((op)->adr == V0_DIR_ADR)                                    \
           ? (*(v0reg *)(&(vm)->mem[(op)->arg[0].adr]))                 \
           : (v0fetcharg((vm), (op)->reg1, (op)->arg[0].ofs, (op)->parm)))))
#define v0getarg2(vm, op)                                               \
    (((op)->adr == V0_REG_ADR)                                          \
     ? (&((vm)->mem[(vm)->regs[(op)->reg2]]))                           \
     : (((op)->adr == V0_IMM_ADR)                                       \
        ? ((v0reg *)(&(vm)->mem[(op)->val]))                            \
        : (((op)->adr == V0_DIR_ADR)                                    \
           ? (*(v0reg *)(&(vm)->mem[(op)->arg[0].adr]))                 \
           : (v0fetcharg((vm), (op)->reg2, (op)->arg[0].ofs, (op)->parm)))))
#define v0getadr1(vm, op)                                               \
    (((op)->adr == V0_REG_ADR)                                          \
     ? ((v0reg *)(&(vm)->mem[(vm)->regs[(op)->reg1]]))                  \
     : (((op)->adr == V0_IMM_ADR)                                       \
        ? ((v0reg *)(&(vm)->mem[(op)->val]))                            \
        : (((op)->adr == V0_DIR_ADR)                                    \
           ? ((v0reg *)(&(vm)->mem[(op)->arg[0].adr]))                  \
           : ((v0reg *)(&(vm)->mem[v0calcadr((op)->reg1, (op)->arg[0].ofs, (op)->parm)])))))
#define v0getadr2(vm, op)                                               \
    (((op)->adr == V0_REG_ADR)                                          \
     ? ((v0reg *)(&(vm)->mem[(vm)->regs[(op)->reg2]]))                  \
     : (((op)->adr == V0_IMM_ADR)                                       \
        ? ((v0reg *)(&(vm)->mem[(op)->val]))                            \
        : (((op)->adr == V0_DIR_ADR)                                    \
           ? ((v0reg *)(&(vm)->mem[(op)->arg[0].adr]))                  \
           : ((v0reg *)(&(vm)->mem[v0calcadr((op)->reg2, (op)->arg[0].ofs, (op)->parm)])))))
#define v0getjmpadr(vm, op)                                             \
    (((op)->adr == V0_REG_ADR)                                          \
     ? ((vm)->mem[(vm)->regs[(op)->reg1]])                              \
     : (((op)->adr == V0_IMM_ADR)                                       \
        ? ((vm)->mem[(op)->val])                                        \
        : (((op)->adr == V0_DIR_ADR)                                    \
           ? ((vm)->mem[(op)->arg[0].adr])                              \
           : ((vm)->mem[v0calcadr((op)->reg1, (op)->arg[0].ofs, (op)->parm)]))))
#define v0getjmradr(vm, op)                                             \
    (((op)->adr == V0_REG_ADR)                                          \
     ? (((vm)->mem[(vm)->regs[(op)->reg1]]))                            \
     : (((op)->adr == V0_IMM_ADR)                                       \
        ? (v0calcjmp((vm)->regs[V0_PC_REG], (op)->val, (op)->parm))     \
        : (v0calcjmp((vm)->regs[(op)->reg1], (op)->val, (op)->parm))))

#define v0getioport(op) ((op)->val)

static _V0OPFUNC_T
v0nop(struct v0 *vm, v0ureg pc)
{
    pc += sizeof(struct v0op);
    v0addspeedcnt(1);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0inc(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg       *sptr = v0getadr1(vm, op);
    v0reg        src = *sptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    src++;
    v0addspeedcnt(2);
    vm->regs[V0_PC_REG] = pc;
    *sptr = src;

    return pc;
}

static _V0OPFUNC_T
v0dec(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg       *sptr = v0getadr1(vm, op);
    v0reg        src = *sptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    src--;
    v0addspeedcnt(2);
    vm->regs[V0_PC_REG] = pc;
    *sptr = src;

    return pc;
}

static _V0OPFUNC_T
v0cmp(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg       *dptr = v0getadr2(vm, op);
    v0reg        src = v0getarg1(vm, op);
    v0reg        dest = *dptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest -= src;
    v0addspeedcnt(8);
    v0clrmsw(vm);
    if (!dest) {
        v0setzf(vm);
    } else if (dest < 0) {
        v0setcf(vm);
    }
    *dptr = dest;
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0add(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg       *dptr = v0getadr2(vm, op);
    v0reg        src = v0getarg1(vm, op);
    v0reg        dest = *dptr;
    v0reg        res = src;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    res += dest;
    v0addspeedcnt(4);
    if (res < dest) {
        v0setof(vm);
    }
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

/* FIXME: set carry-bit */
static _V0OPFUNC_T
v0adc(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg       *dptr = v0getadr2(vm, op);
    v0reg        src = v0getarg1(vm, op);
    v0reg        dest = *dptr;
    v0reg        res = src;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    res += dest;
    v0addspeedcnt(4);
    if (res < dest) {
        v0setof(vm);
    }
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

static _V0OPFUNC_T
v0sub(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg       *dptr = v0getadr2(vm, op);
    v0reg        src = v0getarg1(vm, op);
    v0reg        dest = *dptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest -= src;
    v0addspeedcnt(4);
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

static _V0OPFUNC_T
v0sbc(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg       *dptr = v0getadr2(vm, op);
    v0reg        src = v0getarg1(vm, op);
    v0reg        dest = *dptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest -= src;
    v0addspeedcnt(4);
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

static _V0OPFUNC_T
v0shl(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg       *dptr = v0getadr2(vm, op);
    v0reg        src = v0getarg1(vm, op);
    v0reg        dest = *dptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest <<= src;
    v0addspeedcnt(4);
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

static _V0OPFUNC_T
v0shr(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg       *dptr = v0getadr2(vm, op);
    v0reg        src = v0getarg1(vm, op);
    v0reg        dest = *dptr;
    v0reg        fill = ~((v0reg)0) >> src;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest >>= src;
    v0addspeedcnt(4);
    dest &= fill;
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

static _V0OPFUNC_T
v0sar(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg       *dptr = v0getadr2(vm, op);
    v0reg        src = v0getarg1(vm, op);
    v0reg        dest = *dptr;
    v0reg        mask = ~(v0reg)0;
#if (V0_WORD_SIZE == 8)
    v0reg        fill = (((dest) & (INT64_C(1) << 63))
                   ? (mask >> (64 - src))
                   : 0);
#else
    v0reg        fill = (((dest) & (INT32_C(1) << 31))
                   ? (mask >> (32 - src))
                   : 0);
#endif

    dest >>= src;
    fill = -fill << (CHAR_BIT * sizeof(v0reg) - src);
    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    v0addspeedcnt(4);
    dest &= fill;
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

static _V0OPFUNC_T
v0not(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg       *sptr = v0getadr1(vm, op);
    v0reg        src = *sptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    src = ~src;
    v0addspeedcnt(1);
    vm->regs[V0_PC_REG] = pc;
    *sptr = src;

    return pc;
}

static _V0OPFUNC_T
v0and(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg       *dptr = v0getadr2(vm, op);
    v0reg        src = v0getarg1(vm, op);
    v0reg        dest = *dptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest &= src;
    v0addspeedcnt(2);
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

static _V0OPFUNC_T
v0xor(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg       *dptr = v0getadr2(vm, op);
    v0reg        src = v0getarg1(vm, op);
    v0reg        dest = *dptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest ^= src;
    v0addspeedcnt(2);
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

static _V0OPFUNC_T
v0ior(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg       *dptr = v0getadr2(vm, op);
    v0reg        src = v0getarg1(vm, op);
    v0reg        dest = *dptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest |= src;
    v0addspeedcnt(2);
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

/* CRP */

static _V0OPFUNC_T
v0crp(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0wreg       src = v0getarg1(vm, op);
    double      *dptr = (double *)v0getadr2(vm, op);
    double       res = 1.0 / src;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    *dptr = res;

    return pc;
}

static _V0OPFUNC_T
v0mul(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg       *dptr = v0getadr2(vm, op);
    double       src = (double)v0getarg1(vm, op);
    double       res = *(double *)dptr;
    v0wreg       dest;

    res *= src;
    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest = (v0wreg)res;
    vm->regs[V0_PC_REG] = pc;
    dest &= 0xffffffff;
    v0addspeedcnt(16);
    *dptr = (v0reg)dest;

    return pc;
}

static _V0OPFUNC_T
v0muh(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg       *dptr = v0getadr2(vm, op);
    double       src = (double)v0getarg1(vm, op);
    double       res = *(double *)dptr;
    v0wreg       dest;

    res *= src;
    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest = (v0wreg)res;
    vm->regs[V0_PC_REG] = pc;
    dest >>= 32;
    v0addspeedcnt(16);
    *dptr = (v0reg)dest;

    return pc;
}

static _V0OPFUNC_T
v0ldr(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg        reg;
    v0reg        parm = op->parm;
    v0reg       *dptr = v0regtoptr(vm, op->reg2);
    v0reg       *sptr = NULL;
    v0reg        src = 0;
    v0reg        mask;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    if (op->adr == V0_REG_ADR) {
        reg = op->reg1;
        mask = CHAR_BIT << parm;
        sptr = v0regtoptr(vm, reg);
        mask--;
        src = *sptr;
        v0addspeedcnt(4);
        src &= mask;
        *dptr |= src;
    } else {
        v0addspeedcnt(16);
        if (v0opissigned(op)) {
            switch (parm) {
                case 0:
                    src = *(int8_t *)sptr;

                    break;
                case 1:
                    src = *(int16_t *)sptr;

                    break;
                case 2:
                    src = *(int32_t *)sptr;

                    break;
#if 0
                case 3:
                    wsrc = *(int64_t *)sptr;

                    break;
#endif
            }
            *dptr = src;
        } else {
            switch (op->parm) {
                case 0:
                    *(v0ureg *)dptr = *(uint8_t *)sptr;

                    break;
                case 1:
                    *(v0ureg *)dptr = *(uint16_t *)sptr;

                    break;
                case 2:
                    *(v0ureg *)dptr = *(uint32_t *)sptr;

                    break;
#if 0
                case 3:
                    uwsrc = *(uint64_t *)sptr;

                    break;
#endif
            }
        }
    }
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0str(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg        reg;
    v0reg       *adr = v0getadr2(vm, op);
    v0reg        src;
    v0ureg       usrc;
    v0reg        parm = op->parm;
    v0reg       *dptr;
    v0reg       *sptr = v0adrtoptr(vm, op->reg1);
    v0reg        mask;

    if (!adr) {
        v0doxcpt(V0_INV_MEM_ADR);
    }
    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    if (op->adr == V0_REG_ADR) {
        reg = op->reg2;
        mask = 1 << (parm + CHAR_BIT);
        src = *sptr;
        mask--;
        v0addspeedcnt(4);
        dptr = v0regtoptr(vm, reg);
        src &= mask;
        *dptr |= src;
    } else {
        v0addspeedcnt(16);
        if (v0opissigned(op)) {
            switch (op->parm) {
                case 0:
                    src = *(int8_t *)adr;
                    *(int8_t *)adr = (int8_t)src;

                    break;
                case 1:
                    src = *(int16_t *)adr;
                    *(int16_t *)adr = (int16_t)src;

                    break;
                case 2:
                    src = *(int32_t *)adr;
                    *(int32_t *)adr = (int32_t)src;

                    break;
                case 3:
                    v0doxcpt(V0_INV_MEM_WRITE);

                    break;
            }
        } else {
            switch (op->parm) {
                case 0:
                    usrc = *(uint8_t *)adr;
                    *(uint8_t *)adr = (uint8_t)usrc;

                    break;
                case 1:
                    usrc = *(uint16_t *)adr;
                    *(uint16_t *)adr = (uint16_t)usrc;

                    break;
                case 2:
                    usrc = *(int32_t *)adr;
                    *(uint32_t *)adr = (uint32_t)usrc;

                    break;
                case 3:
                    v0doxcpt(V0_INV_MEM_WRITE);

                    break;
            }
        }
    }
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

/*
 * PSH takes register ID to be pushed in op->val
 */
static _V0OPFUNC_T
v0psh(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0ureg       sp = vm->regs[V0_SP_REG];
    v0reg        reg = op->val;
    v0reg       *sptr = v0regtoptr(vm, reg);
    v0reg       *dptr;

    sp -= sizeof(v0reg);
    pc += sizeof(struct v0op);
    dptr = v0adrtoptr(vm, sp);
    if (op->adr == V0_REG_ADR) {
        v0addspeedcnt(4);
    } else {
        v0addspeedcnt(8);
        pc += sizeof(union v0oparg);
    }
    *dptr = *sptr;
    vm->regs[V0_SP_REG] = sp;
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

/*
 * PSM takes the lowest register ID to be pushed in op->val and shift count
 * for number of registers in op->parm
 */
static _V0OPFUNC_T
v0psm(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0ureg       sp = vm->regs[V0_SP_REG];
    v0reg        lo = op->val;
    v0reg        cnt = 4 << op->parm;
    v0reg       *sptr = v0regtoptr(vm, lo);
    v0reg       *dptr = v0adrtoptr(vm, sp);
    v0reg        num;

    pc += sizeof(struct v0op);
    v0addspeedcnt(8 * cnt);
    dptr -= cnt;
    while (cnt) {
        num = min(8, cnt);
        switch (num) {
            case 8:
                dptr[7] = sptr[7];
            case 7:
                dptr[6] = sptr[6];
            case 6:
                dptr[5] = sptr[5];
            case 5:
                dptr[4] = sptr[4];
            case 4:
                dptr[3] = sptr[3];
            case 3:
                dptr[2] = sptr[2];
            case 2:
                dptr[1] = sptr[1];
            case 1:
                dptr[0] = sptr[0];
            case 0:

                break;
        }
        sptr += num;
        dptr += num;
        cnt -= num;
    }
    vm->regs[V0_SP_REG] = sp;
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0pop(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0ureg       sp = vm->regs[V0_SP_REG];
    v0reg        reg = op->val;
    v0reg       *sptr = v0adrtoptr(vm, sp);
    v0reg       *dest = v0regtoptr(vm, reg);

    pc += sizeof(struct v0op);
    sp += sizeof(v0reg);
    *dest = *sptr;
    v0addspeedcnt(4);
    vm->regs[V0_SP_REG] = sp;
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0pom(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0ureg       sp = vm->regs[V0_SP_REG];
    v0reg        lo = op->val;
    v0reg        cnt = 4 << op->parm;
    v0reg       *sptr = v0adrtoptr(vm, sp);
    v0reg       *dptr = v0regtoptr(vm, lo);
    v0reg        ndx = lo;
    v0reg        num;

    pc += sizeof(struct v0op);
    v0addspeedcnt(8 * cnt);
    sptr -= cnt;
    sp -= cnt;
    while (cnt) {
        num = min(8, cnt);
        switch (num) {
            case 8:
                dptr[7] = sptr[7];
            case 7:
                dptr[6] = sptr[6];
            case 6:
                dptr[5] = sptr[5];
            case 5:
                dptr[4] = sptr[4];
            case 4:
                dptr[3] = sptr[3];
            case 3:
                dptr[2] = sptr[2];
            case 2:
                dptr[1] = sptr[1];
            case 1:
                dptr[0] = sptr[0];
            case 0:

                break;
        }
        cnt -= num;
        sptr += num;
        dptr += num;
    }
    vm->regs[V0_SP_REG] = sp;
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0jmp(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);

    pc = v0getjmpadr(vm, op);
    v0addspeedcnt(8);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0jmr(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg        ofs = v0getjmradr(vm, op);

    pc += ofs;
    v0addspeedcnt(8);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0biz(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg        dest = v0getjmpadr(vm, op);

    if (v0zfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(16);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0bnz(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg        dest = v0getjmpadr(vm, op);

    if (!v0zfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(16);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0blt(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg        dest = v0getjmpadr(vm, op);

    if (!v0ofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(16);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0ble(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg        dest = v0getjmpadr(vm, op);

    if (!v0ofset(vm) || v0zfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(16);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0bgt(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg        dest = v0getjmpadr(vm, op);

    if (v0ofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(16);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0bge(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg        dest = v0getjmpadr(vm, op);

    if (v0ofset(vm) || v0zfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(16);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0bio(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg        dest = v0getjmpadr(vm, op);

    if (v0ofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(16);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0bno(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg        dest = v0getjmpadr(vm, op);

    if (!v0ofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(16);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0bic(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg        dest = v0getjmpadr(vm, op);

    if (v0cfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(16);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0bnc(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg        dest = v0getjmpadr(vm, op);

    if (!v0cfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(16);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

/*
 * call
 * ----
 * - push return address
 *
 * stack frame after call - CREATED BY COMPILER
 * ----------------------
 * r7
 * r6
 * r5
 * r4
 * r3
 * r2
 * r1
 * r0
 * argN
 * ...
 * arg0 <- after compiler has generated stack frame
 * retadr <- sp
 */
static _V0OPFUNC_T
v0csr(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0ureg       sp = vm->regs[V0_SP_REG];
    v0ureg       dest = v0getarg1(vm, op);
    v0reg       *sptr;

    pc += sizeof(struct v0op);
    sp -= sizeof(v0reg);
    sptr = v0adrtoptr(vm, sp);
    if (op->adr == V0_REG_ADR) {
        v0addspeedcnt(4);
    } else {
        v0addspeedcnt(8);
    }
    *sptr = pc;
    vm->regs[V0_PC_REG] = dest;
    vm->regs[V0_SP_REG] = sp;

    return dest;
}

/* create subroutine stack-frame;
 * - push frame pointer
 * - copy stack pointer to frame pointer
 * - push callee-save registers r8..r15
 * - allocate room for local variables on stack
 */
/*
 * stack after enter
 * -----------------
 * retadr
 * oldfp <- fp
 * r15
 * ...
 * r8
 * var0
 * ...
 * varN <- sp
 */
static _V0OPFUNC_T
v0beg(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0ureg       fp = vm->regs[V0_SP_REG];
    v0ureg       sp = vm->regs[V0_SP_REG];
    v0reg       *rptr = v0regtoptr(vm, V0_R0_REG);
    v0reg        nb = v0getarg1(vm, op);
    v0reg       *sptr;

    /* set stack frame up */
    sp -= sizeof(v0reg);
    v0addspeedcnt(32);
    sptr = v0adrtoptr(vm, sp);
    vm->regs[V0_FP_REG] = sp;
    *sptr = fp;
    pc += sizeof(struct v0op);
    sptr -= V0_SAVE_REGS;
    sp -= V0_SAVE_REGS * sizeof(v0reg);
    sptr[V0_R8_REG] = rptr[V0_R8_REG];
    sptr[V0_R9_REG] = rptr[V0_R9_REG];
    sptr[V0_R10_REG] = rptr[V0_R10_REG];
    sptr[V0_R11_REG] = rptr[V0_R11_REG];
    sp -= nb;
    sptr[V0_R12_REG] = rptr[V0_R12_REG];
    sptr[V0_R13_REG] = rptr[V0_R13_REG];
    sptr[V0_R14_REG] = rptr[V0_R14_REG];
    sptr[V0_R15_REG] = rptr[V0_R15_REG];
    vm->regs[V0_SP_REG] = sp;
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

/* destroy subroutine stack-frame
 * - return value is in r0
 * - deallocate local variables
 * - pop callee save registers r8..r15
 * - pop caller frame pointer
 */
/*
 * stack after fin
 * -----------------
 * retadr <- sp
 * oldfp
 * callee save registers r8..r15
 * local variables
 */
static _V0OPFUNC_T
v0fin(struct v0 *vm, v0ureg pc)
{
    v0ureg  sp = vm->regs[V0_FP_REG];
    v0ureg  fp;
    v0reg  *rptr = v0regtoptr(vm, V0_R0_REG);
    v0reg  *sptr = v0adrtoptr(vm, sp);

    pc += sizeof(struct v0op);
    sptr -= V0_SAVE_REGS + 1;
    v0addspeedcnt(8);
    rptr[V0_R15_REG] = sptr[0];
    rptr[V0_R14_REG] = sptr[1];
    sp += sizeof(v0reg);
    rptr[V0_R13_REG] = sptr[2];
    rptr[V0_R12_REG] = sptr[3];
    rptr[V0_R11_REG] = sptr[4];
    rptr[V0_R10_REG] = sptr[5];
    sptr = v0adrtoptr(vm, sp);
    rptr[V0_R9_REG] = sptr[6];
    rptr[V0_R8_REG] = sptr[7];
    sptr = v0adrtoptr(vm, sp);
    vm->regs[V0_PC_REG] = pc;
    fp = *sptr;
    vm->regs[V0_SP_REG] = sp;
    vm->regs[V0_FP_REG] = fp;

    return pc;
}

/* return from subroutine;
 * - pop return address
 */
/*
 * stack after ret
 * ---------------
 * r7
 * ...
 * r0
 * argN
 * ...
 * arg0 <- sp
 * retadr
 */
static _V0OPFUNC_T
v0ret(struct v0 *vm, v0ureg pc)
{
    v0ureg  sp = vm->regs[V0_SP_REG];
    v0reg  *sptr = v0adrtoptr(vm, sp);

    v0addspeedcnt(16);
    sp -= sizeof(v0reg);
    pc = *sptr;
    vm->regs[V0_SP_REG] = sp;
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

#if 0
void
v0mkframe(struct v0 *vm, size_t narg, v0reg *tab)
{
    v0ureg  pc = vm->regs[V0_PC_REG];
    v0ureg  sp = vm->regs[V0_SP_REG];
    v0reg  *rptr = v0regtoptr(vm, V0_R0_REG);
    v0reg  *sptr;
    v0reg   n;

    sp -= V0_SAVE_REGS;
    pc += sizeof(struct v0op);
    sptr = v0adrtoptr(vm, sp);
    sptr[V0_R0_REG] = rptr[V0_R0_REG];
    sptr[V0_R1_REG] = rptr[V0_R1_REG];
    sptr[V0_R2_REG] = rptr[V0_R2_REG];
    sptr[V0_R3_REG] = rptr[V0_R3_REG];
    n = narg;
    sptr[V0_R4_REG] = rptr[V0_R4_REG];
    sptr[V0_R5_REG] = rptr[V0_R5_REG];
    sptr[V0_R6_REG] = rptr[V0_R6_REG];
    sptr[V0_R7_REG] = rptr[V0_R7_REG];
    tab += narg;
    while (n > 8) {
        n = min(narg, 4);
        sptr -= n;
        tab -= n;
        sp -= n;
        switch (narg) {
            case 4:
                sptr[3] = tab[3];
            case 3:
                sptr[2] = tab[2];
            case 2:
                sptr[1] = tab[1];
            case 1:
                sptr[0] = tab[0];
            case 0:

                break;
        }
    }
    vm->regs[V0_SP_REG] = sp;
    if (n) {
        tab -= narg;
        switch (n) {
            case 8:
                rptr[7] = tab[7];
            case 7:
                rptr[6] = tab[6];
            case 6:
                rptr[5] = tab[5];
            case 5:
                rptr[4] = tab[4];
            case 4:
                rptr[3] = tab[3];
            case 3:
                rptr[2] = tab[2];
            case 2:
                rptr[1] = tab[1];
            case 1:
                rptr[0] = tab[0];

                break;
        }
    }

    return;
}
#endif

/* call epilogue;
 * - number of return values (0, 1, 2) in op->parm
 * - get possible return value from r0
 * - deallocate stack arguments
 * - restore caller-save registers r0..r7
 * - set r0 to return value
 */
static _V0OPFUNC_T
v0rmframe(struct v0 *vm, size_t narg)
{
    v0reg  *rptr = v0regtoptr(vm, V0_R0_REG);
    v0ureg  pc = vm->regs[V0_PC_REG];
    v0ureg  sp = vm->regs[V0_SP_REG];
    v0reg  *sptr;

    /* adjust SP past stack arguments */
    sp += narg;
    pc += sizeof(struct v0op);
    /* store r0 and r1 in ret and rethi */
    sptr = v0adrtoptr(vm, sp);
    sp += V0_SAVE_REGS * sizeof(struct v0op);
    /* restore caller-save registers */
    rptr[V0_R0_REG] = sptr[V0_R0_REG];
    rptr[V0_R1_REG] = sptr[V0_R1_REG];
    rptr[V0_R2_REG] = sptr[V0_R2_REG];
    rptr[V0_R3_REG] = sptr[V0_R3_REG];
    rptr[V0_R4_REG] = sptr[V0_R4_REG];
    rptr[V0_R5_REG] = sptr[V0_R5_REG];
    rptr[V0_R6_REG] = sptr[V0_R6_REG];
    rptr[V0_R7_REG] = sptr[V0_R7_REG];
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0ird(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);

    return pc;
}

static _V0OPFUNC_T
v0iwr(struct v0 *vm, v0ureg pc)
{
    struct v0op      *op = v0adrtoptr(vm, pc);
    uint8_t           port = op->val;
    v0reg             parm = op->parm;
    struct v0iofuncs *funcs = vm->iovec;
    v0reg             val;

    if (op->adr == V0_REG_ADR) {
        pc += sizeof(struct v0op);
        val = vm->regs[op->reg2];
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
        val = v0getarg1(vm, op);
    }
    if (funcs[port].wrfunc) {
        funcs[port].wrfunc(vm, port, val);
    }

    return pc;
}

static _V0OPFUNC_T
v0icf(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);

    return pc;
}

// disable _all_ interrupts
static _V0OPFUNC_T
v0cli(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0reg        imr = vm->regs[V0_IMR_REG];
    v0reg       *dptr = v0getadr1(vm, op);

    pc += sizeof(struct v0op);
    if (op->adr == V0_DIR_ADR || op->adr == V0_NDX_ADR) {
        *dptr = imr;
    }
    vm->regs[V0_IMR_REG] = 0;
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

// add bits to IMR in order to enable interrupts
// optional memory-argument is mask to be restored
static _V0OPFUNC_T
v0sti(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);
    v0ureg       imr = vm->regs[V0_IMR_REG];
    v0ureg       mask = vm->regs[op->reg1];
    v0reg       *sptr = v0getadr1(vm, op);

    pc += sizeof(struct v0op);
    if (op->adr == V0_DIR_ADR || op->adr == V0_NDX_ADR) {
        imr = *sptr;
    } else {
        imr |= mask;
    }
    vm->regs[V0_PC_REG] = pc;
    vm->regs[V0_IMR_REG] = imr;

    return pc;
}

static _V0OPFUNC_T
v0slp(struct v0 *vm, v0ureg pc)
{
    struct v0op *op = v0adrtoptr(vm, pc);

    pc += sizeof(struct v0op);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0rst(struct v0 *vm, v0ureg pc)
{

    return _startadr;
}

static _V0OPFUNC_T
v0hlt(struct v0 *vm, v0ureg pc)
{

    return V0_HALTED;
}

#endif /* __V0_VM_OP_H__ */

