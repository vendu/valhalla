#ifndef __V0_VM_INS_H__
#define __V0_VM_INS_H__

#include <stdint.h>
#include <valhalla/cdefs.h>
#include <valhalla/param.h>
#include <v0/types.h>

/* INSTRUCTION FORMAT */
/*
 * - the shortest instructions (2 register operands, no special functionality)
 *   are 16-bit
 *   - the high bit (0x80) of the opcode is used to denote there's another 16-
 *     bit instruction parcel to follow
 * - instructions are always fetched as 64-bit or 32-bit chunks
 * - immediate operands following 32-bit instructions are always 32-bit
 */

#define V0_INS_REG_BITS    4
#define V0_INS_REG_MASK    ((1 << V0_INS_REG_BITS) - 1)
#define vogetinsval(ins)   ((ins)->arg.parm.val & V0_PARM_MASK)
/* NOP is declared as all 0-bits */
#define V0_NOP_CODE        (UINT16_C(0))
#define V0_COP_CODE        0xff
/* instruction prefixes have all 1-bits in code */
#define V0_SYS_CODE        (UINT16_C(~0))
/* predefined coprocessor IDs in val-field after 0xff-opcode */
#define V0_COPROC_FPM      0x01 // fixed-point
#define V0_COPROC_FPU      0x02 // floating-point unit
#define V0_COPROC_SIMD     0x03 // SIMD-unit
#define V0_COPROC_VEC      0x04 // vector processor
#define V0_COPROC_DSP      0x05 // digital signal processor
#define v0isnop(ins)       (*(uint8_t *)(ins) == V0_NOP_CODE)
#define v0coprocid(ins)    (((uint8_t *)(ins))[1])
#define v0getcode(ins)     ((ins)->code)
#define v0getxreg(ins)     ((ins)->arg[0].op.val)
#define v0getval(ins)      ((ins)->arg[0].op.val)
#define v0setreg(ins, reg, id)                                          \
    ((ins)->parm |= (reg) << (4 * (id)))
#define v0setxreg(ins, reg)                                             \
    ((ins)->u.arg16[0].op.val = (reg))

/* values for the val-field; GEN */
/* common flags */
#define V0_UNS_BIT  (1U << 0)   // unsigned operands
#define V0_FLG_BIT  (1U << 1)   // processor flag-bits altered
#define V0_MSW_BIT  (1U << 2)   // processor status-word (MSW) altered
/* logical operations */
#define V0_AND_BIT  (1U << 0)   // logical AND operation: R2 &= R1
#define V0_EXC_BIT  (1U << 0)   // exclusive [OR] operation: R2 ^= R1
#define V0_OR_BIT   (1U << 1)   // logical OR operation: R2 |= R1
#define V0_NEG_BIT  (1U << 2)   // arithmetic negation: R2 = -R1
/* shift operations */
#define V0_DIR_BIT  (1U << 0)   // right shift operations: SHR, SAR, ROR
#define V0_ARI_BIT  (1U << 1)   // arithmetic [right] shift: SAR
#define V0_ROT_BIT  (1U << 2)   // bitwise rotation: ROL, ROR
#define V0_SHA_BIT  (1U << 3)   // shift and add: SLA, SRA
#define V0_SAM_MASK 0x0a        // shift and mask: SLM, SRM
/* add operations */
#define V0_DEC_BIT  (1U << 0)   // DEC
#define V0_INC_BIT  (1U << 2)   // INC
#define V0_CMP_BIT  (1U << 2)   // CMP
#define V0_SUB_BIT  (1U << 3)   // subtract
/* multiplication operations */
#define V0_REM_BIT  (1U << 1)
#define V0_HI_BIT   (1U << 1)
#define V0_DIV_BIT  (1U << 2)
#define V0_RPC_BIT  (1U << 3)
/* bit operations */
#define V0_CLR_BIT  (1U << 0)
#define V0_ODD_BIT  (1U << 0)
#define V0_ONE_BIT  (1U << 0)
#define V0_DCD_BIT  (1U << 0)
#define V0_CNT_BIT  (1U << 1)
#define V0_PAR_BIT  (1U << 2)
#define V0_BCD_BIT  (1U << 2)
#define V0_CRC_BIT  (1U << 3)
#define V0_COND_BIT (1U << 3)
#define V0_HSH_MASK 0x0c
/* memory operations */
#define V0_RD_BIT   (1U << 0)
#define V0_REG_BIT  (1U << 1)
#define V0_WR_BIT   (1U << 1)
#define V0_N_BIT    (1U << 1)
#define V0_CL_BIT   (1U << 2)
#define V0_BAR_BIT  (1U << 3)
//#define V0_PG_BIT   (1U << 4)
/* atomic operations */
#define V0_SYN_BIT  (1U << 0)
#define V0_DBL_BIT  (1U << 0)
#define V0_CAS_BIT  (1U << 1)
#define V0_BT_BIT   (1U << 2)
/* values for the branch unit  (FLOW) */
#define V0_BOF_BIT  (1U << 0)
#define V0_EQ_BIT   (1U << 0)
#define V0_NE_BIT   (1U << 1)
#define V0_LT_BIT   (1U << 2)
#define V0_GT_MASK  0x06
#define V0_BFL_BIT  (1U << 3)
/* subroutines */
#define V0_TERM_BIT (1U << 0)
#define V0_FIN_BIT  (1U << 0)
#define V0_SYS_BIT  (1U << 0)
#define V0_SUB_BIT  (1U << 1)
#define V0_RET_BIT  (1U << 2)
#define V0_THR_BIT  (1U << 3)
/* interrupt management */
#define V0_ON_BIT   (1U << 0)
#define V0_RST_BIT  (1U << 0)
#define V0_MSK_BIT  (1U << 1)
#define V0_INT_BIT  (1U << 2)
#define V0_EV_BIT   (1U << 3)
#define V0_HLT_MASK 0x06
/* I/O control */
#define V0_MAP_BIT  (1U << 0)
#define V0_IR_BIT   (1U << 1)
#define V0_CMD_BIT  (1U << 2)

#define v0getreg1(vm, ins)    ((vm)->regs[(ins)->regs & 0x0f])
#define v0getreg2(vm, ins)    ((vm)->regs[((ins)->regs >> 4) & 0x0f])
#define v0getreg(vm, reg)     (((v0reg *)(vm)->regs)[(reg)])
#define v0getureg(vm, reg)    (((v0ureg *)(vm)->regs)[(reg)])
#define v0setreg(vm, reg, u)  (((v0reg *)(vm)->regs)[(reg)] = (u))
#define v0setureg(vm, reg, u) (((v0ureg *)(vm)->regs)[(reg)] = (u))
#define v0getofs(vm, ins)     (((ins)->flg & V0_IMM_BIT)                \
                               ? (ins)->imm[0].ofs                      \
                               : 0)
#define v0getimm(ins)         ((ins)->imm[0].val)
#define v0getimmu(ins)        ((ins)->imm[0].uval)
static __inline__ v0reg
v0decadr1(struct v0 *vm, struct v0ins *ins)
{
    v0reg adr = ((ins)->flg & V0_REG_ADR) ? v0getreg1(vm, ins) : 0;
    v0reg ofs = ((ins)->flg & V0_PIC_ADR) ? v0getreg(vm, V0_PC_REG) : 0;
    v0reg imm = ((ins)->flg & V0_IMM_BIT) ? v0getofs(ins) : 0;

    adr += ofs;
    adr += imm;

    return adr;
}

v0decadr2(struct v0 *vm, struct v0ins *ins)
{
    v0reg adr = ((ins)->flg & V0_REG_ADR) ? v0getreg2(vm, ins) : 0;
    v0reg ofs = ((ins)->flg & V0_PIC_ADR) ? v0getreg(vm, V0_PC_REG) : 0;
    v0reg imm = ((ins)->flg & V0_IMM_BIT) ? v0getofs(ins) : 0;

    adr += ofs;
    adr += imm;

    return adr;
}

#endif /* __V0_VM_INS_H__ */

