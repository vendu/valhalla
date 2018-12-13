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

#define V0_PARM_MASK       0x0f
#define V0_ADR_MASK        0xf0 // addressing mode ival
/* addressing modes */
/* register addressing is detected with nonzero (op->reg) */
#define V0_REG_ADR         0x00 // register operands
#define V0_IMM_ADR         0x10 // operand follows opcode, e.g. op->arg[0].i32
#define V0_PIC_ADR         0x20 // PC-relative, i.e. x(%pc) for shared objects
#define V0_NDX_ADR         0x30 // indexed, i.e. %r1(%r2) or $c1(%r2)
/* else indexed: pc[ndx << op->parm], ndx follows opcode */
/* imm16- and imu16- immediate fields */
#define V0_IMM16_VAL_MAX   0x7fff
#define V0_IMM16_VAL_MIN   (-0x7fff - 1)
#define V0_IMU16_VAL_MAX   0xffffU
#define V0_IMU16_VAL_MIN   0U

#define V0_INS_CODE_BITS   8
#define V0_INS_IMMED_BIT   (1 << (V0_INS_CODE_BITS - 1)) // 16-bit value
#define V0_INS_ALIGN_BIT   (1 << (V0_INS_CODE_BITS - 2)) // aligned value
#define V0_INS_REG_BITS    4
#define V0_INS_REG_MASK    ((1 << V0_INS_REG_BITS) - 1)
#define vogetinsval(ins)   ((ins)->arg.parm.val & 0x2f)
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
#define v0getreg(ins, id)  (((ins)->parm >> (4 * id)) & V0_INS_REG_MASK)
#define v0getofs(ins)      (v0imm16(ins))
#define v0getimm16(ins)    ((ins)->u.arg16[0].i16)
#define v0getimmu16(ins)   ((ins)->u.arg16[0].u16)
#define v0getadrmode(ins)  ((ins)->u.arg16[0].op.parm & V0_ADR_MASK)
#define v0getxreg(ins)     ((ins)->arg[0].op.val)
#define v0getval(ins)      ((ins)->arg[0].op.val)
#define v0setreg(ins, reg, id)                                          \
    ((ins)->parm |= (reg) << (4 * (id)))
#define v0setxreg(ins, reg)                                             \
    ((ins)->u.arg16[0].op.val = (reg))
#define v0setaln(ins, reg)                                              \
    ((ins)->u.arg32 = roundup2(ins, 4))

/* values for the val-field; GEN */
/* logical operations */
#define V0_AND_BIT (1U << 0)
#define V0_OR_BIT  (1U << 1)
#define V0_XCL_BIT (1U << 2)
/* shift operations */
#define V0_DIR_BIT (1U << 0)
#define V0_ARI_BIT (1U << 1)
#define V0_ROT_BIT (1U << 2)
/* add operations */
#define V0_UNS_BIT (1U << 0)
#define V0_FLG_BIT (1U << 1)
#define V0_INC_BIT (1U << 2)
#define V0_SUB_BIT (1U << 3)
/* multiplication operations */
#define V0_DIV_BIT (1U << 1)
#define V0_HI_BIT  (1U << 1)
#define V0_REM_BIT (1U << 2)
#define V0_RPC_BIT (1U << 3)
/* bit operations */
#define V0_CNT_BIT (1U << 0)
#define V0_ONE_BIT (1U << 1)
/* memory operations */
#define V0_RD_BIT  (1U << 0)
#define V0_WR_BIT  (1U << 1)
#define V0_N_BIT   (1U << 1)
#define V0_MEM_BIT (1U << 2)
#define V0_BAR_BIT (1U << 3)
#define V0_PG_BIT  (1U << 4)
#define V0_MSW_BIT (1U << 5)
/* atomic operations */
#define V0_LDR_BIT (1U << 2)
#define V0_CLR_BIT (1U << 3)
#define V0_SYN_BIT (1U << 4)
/* values for the branch unit (BRA) */
#define V0_EQ_BIT  (1U << 1)
#define V0_NE_BIT  (1U << 2)
#define V0_LT_BIT  (1U << 3)
#define V0_GT_BIT  (1U << 4)
#define V0_BC_BIT  (1U << 5)
#define V0_BO_BIT  (1U << 6)
/* subroutines */
#define V0_SYS_BIT (1U << 0)
#define V0_BEG_BIT (1U << 1)
#define V0_FIN_BIT (1U << 2)
#define V0_RET_BIT (1U << 3)
#define V0_THR_BIT (1U << 4)
/* interrupt management */
#define V0_ON_BIT  (1U << 0)
#define V0_MSK_BIT (1U << 1)
#define V0_INT_BIT (1U << 2)
#define V0_SLP_BIT (1U << 3)
#define V0_EV_BIT  (1U << 4)
#define V0_HLT_BIT (1U << 5)

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

#define v0getadrmode(ins) ((long)((ins)->arg[0].info.parm & V0_ADR_MASK))
struct v0op {
    uint8_t parm;
    uint8_t val;
};

union v0arg16 {
    struct v0op op;
    int16_t     i16;
    uint16_t    u16;
};

struct v0ins {
    uint8_t            code;    // instruction ID + possible V0_INS_32_BIT-flag
    uint8_t            parm;    // register operand IDs (if non-zero)
    union {
        union v0arg16  arg[VLA]; // instruction argument if present
        union v0arg32 *aln;      // aligned 32-bit word following opcode
};

#endif /* __V0_VM_INS_H__ */

