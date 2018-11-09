#ifndef __V0_VM_INS_H__
#define __V0_VM_INS_H__

/* INSTRUCTION FORMAT */
/*
 * - the shortest instructions (2 register operands, no special functionality)
 *   are 16-bit
 *   - the high bit (0x80) of the opcode is used to denote there's another 16-
 *     bit instruction parcel to follow
 * - instructions are always fetched as 64-bit or 32-bit chunks
 * - immediate operands following 32-bit instructions are always 32-bit
 */

/*
 * 32-bit little-endian argument parcel
 * - declared as union for 32-bit alignment of all arguments
 */
union v0insarg {
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

struct v0insdata {
    union {
        struct {
            uint8_t info;
            uint8_t cnt;
        } w8;
        int16_t     i16;
        uint16_t    u16;
    } data;
    union v0insarg  val[VLA];
};

#define V0_SZSFT_MASK     0x03
#define V0_ADR_MASK       0x0c
/* addressing modes */
/* register addressing is detected with (op->reg) */
#define V0_IMM_ADR        0    // operand follows opcode, e.g. op->arg[0].i32
#define V0_DIR_ADR        0x04 // address follows opcode, op->arg[0].adr
#define V0_NDX_ADR        0x08 // reg[ndx << op->parm], ndx val or after opcode
#define V0_PIC_ADR        0x0c // pc[ndx << op->parm], ndx val or after opcode
/* flags for arithmetic instructions */
#define V0_ATOM_BIT       (1U << 5) // LOCK-prefix
#define V0_SAT_BIT        (1U << 6) // SAT-prefix + size in parm
#define V0_SATU_BIT       (1U << 7) // SATU-prefix + size in parm
/* flags for logical instructions */
#define V0_INV_BIT        (1U << 6)
/* flags for shift instructions */
#define V0_MASK_BIT       (1U << 5)
#define V0_ROT_BIT        (1U << 6)
#define V0_ADD_BIT        (1U << 7)
/* imm16- and imu16- immediate fields */
#define V0_IMM16_VAL_MAX  0x7fff
#define V0_IMM16_VAL_MIN  (-0x7fff - 1)
#define V0_IMU16_VAL_MAX  0xffffU
#define V0_IMU16_VAL_MIN  0U

#define V0_INS_CODE_BITS  8
#define V0_INS_IS_32_BIT  (1 << (V0_INS_CODE_BITS - 1))
#define V0_INS_REG_BITS   4
#define V0_INS_REG_MASK   ((1 << V0_INS_REG_BITS) - 1)
#define vogetinscnt(ins)  ((ins)->data.parm.cnt & 0x2f)
/* NOP is declared as all 1-bits */
#define V0_NOP_CODE       (~UINT16_C(0))
#define v0insisnop(ins)   (*(uint16_t *)(ins) == V0_NOP_CODE)
#define v0insis32bit(ins) ((ins)->code & V0_INS_32_BIT)
#define v0inscode(ins)    ((ins)->code)
#define v0insreg(ins, id) (((ins)->regs >> 4 * id) & V0_INS_REG_MASK)
#define v0jmpofs(ins)     v0imm16(ins)
#define v0imm16(ins)      ((ins)->data.i16)
#define v0imu16(ins)      ((ins)->data.u16)
#define v0parm(ins)       ((ins)->arg[0].data.w8.info & 0x03)
#define v0adrmode(ins)    ((ins)->arg[0].data.w8.info & 0x0c)
#define v0flg(ins)        ((ins)->arg[0].data.w8.info & 0xf0)
#define v0hasflg(ins, f)  ((ins)->arg[0].data.w8.info & (f))
#define v0cnt(ins)     ((ins)->arg[0].data.w8.cnt)
#define v0getadr(vm, op, id, tmp)                                       \
    (((tmp) = v0insreg(ins, id))                                        \
     ? (void *)(&(vm)->mem[(vm)->regs[(tmp)]])                          \
     : ((((tmp) = (op)->adr) == V0_IMM_ADR)                             \
        ? (void *)(&(vm)->mem[(op)->arg[0].i32])                        \
        : (((tmp) == V0_DIR_ADR)                                        \
           ? (void *)(&(vm)->mem[(op)->arg[0].adr])                     \
           : (void *)(&(vm)->mem[_v0calcadr((vm)->regs[v0insreg(ins, id)], (op))]))))
#define v0getarg1(vm, op, type) (*(type *)v0getadr(vm, op, 0, tmp))
#define v0getarg2(vm, op, type) (*(type *)v0getadr(vm, op, 1, tmp))
#define v0getjmpofs(vm, op, tmp)                                        \
    ((((tmp) = v0adrmode((op))) == V0_PIC_ADR)                          \
     ? (vm)->mem[_v0calcjmpofs((vm)->regs[V0_PC_REG], op)]              \
     : (((tmp) = v0insreg(op, 0))                                       \
        ? (vm)->regs[(tmp)]                                             \
        : ((((tmp) = v0adrmode(op)) == V0_IMM_ADR)                      \
           ? (vm)->mem[(op)->arg[0].ofs]                                \
           : (((tmp) == V0_DIR_ADR)                                     \
              ? (vm)->mem[(op)->arg[0].val[0].adr]                      \
              : (vm)->mem[_v0calcadr((vm), v0insreg(op, 0), (op))]))))
#define _v0calcadr(vm, reg, op)                                         \
    ((vm)->regs[(reg)] + ((op)->arg[0].ofs << ((op)->parm)))
#define _v0calcjmpofs(vm, reg, op)                                      \
    ((vm)->regs[(reg)] + (((op)->val) ? ((op)->val << 1) : ((op)->arg[0].ofs << 1)))

struct v0ins {
    uint8_t          code; // instruction ID + possible V0_INS_32_BIT-flag
    uint8_t          regs; // register operand IDs (if non-zero)
    struct v0insdata data[VLA]; // instruction data if present
};

#if 0
struct v0ins {
    unsigned int   code : 8; // unit and instruction IDs
    unsigned int   reg1 : 4; // register argument #1 ID
    unsigned int   reg2 : 4; // register argument #2 ID
    unsigned int   parm : 2; // address scale shift count
    unsigned int   adr  : 2; // addressing mode
    unsigned int   flg  : 4; // instruction flags
    unsigned int   val  : 8; // immediate value; shift count, register range
    union v0insarg arg[VLA]; // possible argument value
};
#endif

#endif /* __V0_VM_INS_H__ */

