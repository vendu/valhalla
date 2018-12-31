#ifndef __V0_VM_INS_H__
#define __V0_VM_INS_H__

#include <stdint.h>
#include <valhalla/cdefs.h>
#include <valhalla/param.h>
#include <v0/types.h>

/*
 * LOGIC UNIT
 * ----------
 * NOT RI1, R2          ; R2 = ~RI1
 * AND RI1, R2          ; R2 &= RI1
 * IOR RI1, R2          ; R2 |= RI1
 * XOR RI1, R2          ; R2 ^= RI1
 */
/*
 * SHIFT UNIT
 * ----------
 * SHL RI1, R2          ; R2 <<= RI1
 * SHR RI1, R2          ; R2 >>= RI1
 * SAR RI1, R2          ; R2 >>>= RI1
 * ROL RI1, R2          ; R2 <<.= RI1
 * ROR RI1, R2          ; R2 >>.= RI1
 * SLA V, R1, R2        ; R2 = (R2 << V) + R1
 * SRA V, R1, R2        ; R2 = (R2 >> V) + R1
 * SLM V, R1, R2        ; R2 = (R2 << V) & R1
 * SRM V, R1, R2        ; R2 = (R2 >> V) & R1
 */
/*
 * ADDER UNIT
 * ----------
 * INC RI1, R2          ; R2 = ++RI1
 * ICU RI1, R2
 * DEC RI1, R2          ; R2 = --RI�
 * DCU RI1, R2
 * ADD RI1, R2          ; R2 += RI1
 * ADU RI1, R2
 * ADC RI1, R2          ; R2 += RI1, set CF in MSW
 * ACU RI1, R2
 * SUB RI1, R2          ; R2 -= RI1
 * SBU RI1, R2
 * SBC RI1, R2          ; R2 -= RI1, set CF in MSW
 * SCU RI1, R2
 * CMP RI1, R2          ; R2 -= RI1, set ZF|LT|GT in MSW
 * CMU RI1, R2
 */
/*
 * MULTI UNIT
 * ----------
 *
 * NOTES
 * -----
 * - RPS and RPU return the inverse reciprocal in R2 as well as shift count in
 *   R1 for simulating
 *
 *       R2 / VI
 *
 *   by doing
 *
 *       (R2 * RP(VI)) >> R1    ; simulate division with inverse multiplication
 *
 * MUL RI1, R2          ; R2 = R2 * RI1
 * MLU RI1, R2          ; R2 = R2 * RI1, unsigned
 * MLH RI1, R2          ; R2 = (R2 * RI1) >> 32
 * MHU RI1, R2          ; R2 = (R2 * RI1) >> 32, unsigned
 * DIV RI1, R2          ; R2 = R2 / RI1
 * DVU RI1, R2          ; R2 = R2 / RI1, unsigned
 * REM RI1, R2          ; R2 = R2 % RI1
 * RMU RI1, R2          ; R2 = R2 % RI1, unsigned
 * RPS VI, R1, R2       ; R2 = RPS(VI), R1 = CNT, inverse reciprocal + shift
 * RPU VI, R1, R2       ; R2 = RPU(VI), R1 = CNT, unsigned
 */
/*
 * BIT UNIT
 * --------
 * SEX V, RI1, R2       ; R2 = SEX(RI1, V), V0_BYTE_BIT, V0_HALF_BIT
 * ZEX V, RI1, R2       ; R2 = ZEX(RI1, V), V0_BYTE_BIT, V0_HALF_BIT
 * CLZ RI1, R2          ; R2 = CLZ(RI1), count leading zeroes in RI1
 * HAM RI1, R2          ; R2 = HAM(RI1), Hamming weight i.e. # of 1-bits in RI1
 * PAR RV1, R2          ; R2 = PAR(RV1), byte parity (0 means even)
 * NEG RI1, R2          ; R2 = -RI1
 * BCD RI1, ADR         ; *ADR = BCD(RI1)
 * DCD RI1, ADR         ; *ADR = DCD(RI1)
 * CLR RV1, R2          ; R2 &= ~(1 << RV1)
 * SET RV1, R2          ; R2 |= (1 << RV1)
 * CRC V, I, ADR, R2    ; R2 = CRC(ADR, VI), V is algorithm, I is byte-count
 * Sxx RI1, ADR         ; conditional store *ADR = RI1
 * Lxx ADR, R2          ; conditional load R2 = *ADR
 * SWP RI1, R2          ; R2 = bswap(RI1), V0_BYTE_BIT, V0_HALF_BIT
 * HSH RI1, R2          ; R2 = hash(RI1)
 * HUN RI1, R2          ; R2 = unhash(RI1)
 * CLR V, ADR           ; *ADR &= ~(1 << V)
 * SET V, ADR           ; *ADR |= (1 << V)
 */
/*
 * MEM UNIT
 * --------
 * LEA ADR, R2          ; R2 = ADR
 * LDR ADR, R2          ; R2 = *ADR
 * STR R1, ADR          ; *ADR = R1
 * IPG ADR              ; flush TLB-entry for page of ADR
 * CLR ADR              ; lock cacheline of ADR
 * CPF ADR              ; prefetch cacheline of ADR
 * CFL ADR              ; flush and unlock cacheline of ADR
 * BAR                  ; full memory barrier
 * BRD                  ; memory read barrier
 * BWR                  ; memory write barrier
 * LFL R2               ; R2 = MSW
 * SFL R1               ; MSW = R1
 * LDX ADR, XR2         ; XR2 = XR1
 * STX XR1, ADR         ; *ADR = XR1
 */
/*
 * STACK UNIT
 * ----------
 * PSH RI1              ; *--SP = RI1
 * POP R2               ; R2 = *SP++
 * PSM V                ; push registers (V & 0x0f)..(V >> 4) in reverse order
 * POM V                ; pop registers (V & 0x0f)..(V >> 4)
 * PSF                  ; *--SP = MSW
 * POF                  ; MSW = *SP++
 * MKF V, ADR           ; stack and registers with V words from table at ADR
 * PSX XR1, ADR         ; *--SP = XR1
 * POX XR2              ; XR2 = *SP++
 */
/*
 * ATOM UNIT
 * ---------
 * LNK ADR, R2          ;; load-link R2 = *ADR
 * STC RI1, ADR         ;; *ADR = RI1
 * CAS I, R1, ADR       ;; *ADR = R1 iff *ADR == I
 * BTS RV1, ADR         ;; CF = *ADR & (1 << V), *ADR |= (1 << V)
 * BTC RV1, ADR         ;; CF = *ADR & (1 << V), *ADR &= ~(1 << V)
 */
/*
 * FLOW UNIT
 * ---------
 * JMP RI1              ; jump to *RI1
 * JMR RV               ; jump to PC[RV]
 * BEQ RI1
 * BNE RI1
 * BLT RI1
 * BGE RI1
 * BLE RI1
 * BGT RI1
 * BCF RI1
 * BNC RI1
 * BOF RI1
 * BNO RI1
 *
 * condition-flags in MSW
 * ----------------------
 * BEQ - ZF
 * BNE - !ZF
 * BLT - !ZF && LT
 * BLE - ZF || LT
 * BGT - !ZF && GT
 * BGE - ZF || GT
 * BCF - CF
 * BNC - !CF
 * BOF - OF
 * BNO - !OF
 */
/*
 * SUBR UNIT
 * ---------
 * BEG RI1              ; function prologue
 * CSR RI1              ; call subroutine at RI1
 * FIN                  ; function epilogue
 * SYS                  ; invoke system mode/call
 * RET                  ; return from function
 * IRT                  ; return from interrupt function
 * SRT                  ; return from system mode
 * THR                  ; launch new thread
 * THX                  ; terminate thread
 * THS                  ; launch system thread
 */
/*
 * SYS UNIT
 * --------
 * CLI                  ; disable interrupts
 * STI                  ; enable interrupts
 * SIM                  ; set interrupt mask
 * LIM                  ; load interrupt mask
 * INT                  ; trigger software interrupt
 * SIV                  ; set interrupt vector
 * HLT                  ; halt; wait for interrupt
 * RST                  ; reset
 * WFE                  ; wait for [cacheline] event
 * SEV                  ; signal [cacheline] event
 */
/*
 * IO UNIT
 * -------
 * IOC                  ; I/O command
 * ILD                  ; load I/O register
 * IST                  ; store I/O register
 * ILM                  ; load interrupt descriptor map
 * IRD                  ; I/O read operation
 * IWR                  ; I/O write operation
 */
#define V0_INS_UNIT_BITS      4
#define V0_INS_OP_BITS        4
#define V0_INS_REG_BITS       4
#define V0_INS_OP_MASK        ((1 << V0_INS_OP_BITS) - 1)
#define V0_INS_REG_MASK       ((1 << V0_INS_REG_BITS) - 1)
/* NOP is declared as all 0-bits in code */
#define V0_NOP_CODE           0x00
/* unit ID 0x0f is reserved for instruction prefixes */
#define V0_COP_CODE           0xf0 // instruction in low, unit high 8 bits
#define V0_LOCK_CODE          0xf1 // prefix flags in high, type in low 8 bits
#define v0isprefix(ins)       ((ins)->code == V0_PREFIX_CODE)
/* predefined coprocessor IDs in high 8 parm-bits */
#define V0_COPROC_FPM         0x0100 // fixed-point
#define V0_COPROC_FPU         0x0200 // floating-point unit
#define V0_COPROC_SIMD        0x0300 // SIMD-unit
#define V0_COPROC_VEC         0x0400 // vector processor
#define V0_COPROC_DSP         0x0500 // digital signal processor
#define V0_COPROC_GPU         0x0600 // graphics processor unit
/* predefined prefix IDs in high 8 parm-bits */
#define V0_LOCK_PREFIX        0x0000 // bus-lock prefix for atomic memory access
#define v0isnop(ins)          ((ins)->code == V0_NOP_CODE)
#define v0iscop(ins)          ((ins)->code == V0_COP_CODE)
#define v0copid(ins)          ((ins)->parm >> 12)
#define v0copflg(ins)         ((ins)->parm & 0x0f00)
#define v0copins(ins)         ((ins)->parm & 0xff)

#define v0getreg1(vm, ins)    ((vm)->regs[(ins)->regs & 0x0f])
#define v0getreg2(vm, ins)    ((vm)->regs[((ins)->regs >> 4) & 0x0f])
#define v0getreg(vm, reg)     (((v0reg *)(vm)->regs)[(reg)])
#define v0getureg(vm, reg)    (((v0ureg *)(vm)->regs)[(reg)])
#define v0setreg(vm, reg, u)  (((v0reg *)(vm)->regs)[(reg)] = (u))
#define v0setureg(vm, reg, u) (((v0ureg *)(vm)->regs)[(reg)] = (u))
#define v0getval(vm, ins)     \
    (((vm)->parm & V0_SIGN_BIT)                                         \
     ? (-((vm)->parm & V0_VAL_MASK))                                    \
     : ((vm)->parm & V0_VAL_MASK))
                                                                        \
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

/* parm-member bits */
/* addressing modes */
#define V0_VAL_MASK   0x0fff
#define V0_NO_ADR     0x00      // register-only operands
#define V0_BYTE_BIT   0x2000    // 8-bit byte value in ins->parm
#define V0_HALF_BIT   0x4000    // 16-bit halfword value in ins->parm
#define V0_SIZE_MASK  0x6000
#define V0_SIGN_BIT   9x1000    // 13-bit immediate value sign-bit
#define V0_REG_ADR    0x2000    // base register, e.g. $4(%r1) (else PC)
#define V0_NDX_ADR    0x4000    // indexed, i.e. %r1(%r2) or $c1(%r2)
#define V0_IMM_BIT    0x8000    // immediate argument follows opcode
#define V0_ADR_MASK   0x7000    // mask of addressing mode bits

/* common flags */
#define V0_UNS_BIT    (1 << 0)  // unsigned operands
#define V0_RD_BIT     (1 << 0)  // read operation
#define V0_WR_BIT     (1 << 1)  // write operation
#define v0mkcode(unit, op) (((unit) << 4) | (op))
/* LOGIC-unit instructions */
#define V0_LOGIC_UNIT 0x01
#define V0_AND_BIT    (1 << 0)  // logical AND operation: R2 &= R1
#define V0_EXC_BIT    (1 << 0)  // exclusive [OR] operation: R2 ^= R1
#define V0_OR_BIT     (1 << 1)  // logical OR operation: R2 |= R1
#define V0_NOT_OP     0x00
#define V0_AND_OP     (V0_AND_BIT)
#define V0_IOR_OP     (V0_OR_BIT)
#define V0_XOR_OP     (V0_EXC_BIT | V0_OR_BIT)
/* SHIFT-unit */
#define V0_SHIFT_UNIT 0x02
#define V0_DIR_BIT    (1 << 0)  // right shift operations: SHR, SAR, ROR
#define V0_ARI_BIT    (1 << 1)  // arithmetic [right] shift: SAR
#define V0_ROT_BIT    (1 << 2)  // bitwise rotation: ROL, ROR
#define V0_SAM_BIT    (1 << 3)  // shift and mask: SLM, SRM
#define V0_SHA_MASK   0x06      // shift and add: SLA, SRA
#define V0_SHL_OP     0x00
#define V0_SHR_OP     (V0_DIR_BIT)
#define V0_SAR_OP     (V0_ARI_BIT)
#define V0_ROL_OP     (V0_ROT_BIT)
#define V0_ROR_OP     (V0_DIR_BIT | V0_ROT_BIT)
#define V0_SLA_OP     V0_SHA_MASK
#define V0_SRA_OP     (V0_DIR_BIT | V0_SHA_MASK)
#define V0_SLM_OP     (V0_SAM_BIT)
#define V0_SRM_OP     (V0_DIR_BIT | V0_SAM_BIT)
/* ADDER-unit */
#define V0_ADDER_UNIT 0x03
#define V0_CF_BIT     (1 << 1)  // carry-flag altered
#define V0_ADD_BIT    (1 << 2)  // ADD, ADU, ADC, AUC
#define V0_CMP_BIT    (1 << 2)  // CMP
#define V0_SUB_BIT    (1 << 3)  // SUB, SBU, SBC, SUC, CMP, CMU
#define V0_INC_OP     0x00
#define V0_ICU_OP     (V0_UNS_BIT)
#define V0_DEC_OP     (V0_DEC_BIT)
#define V0_DCU_OP     (V0_UNS_BIT | V0_DEC_BIT)
#define V0_ADD_OP     (V0_ADD_BIT)
#define V0_ADU_OP     (V0_UNS_BIT | V0_ADD_BIT)
#define V0_ADC_OP     (V0_CF_BIT | V0_ADD_BIT)
#define V0_ACU_OP     (V0_UNS_BIT | V0_CF_BIT | V0_ADD_BIT)
#define V0_SUB_OP     (V0_SUB_BIT)
#define V0_SBU_OP     (V0_UNS_BIT | V0_SUB_BIT)
#define V0_SBC_OP     (V0_CF_BIT | V0_SUB_BIT)
#define V0_SCU_OP     (V0_UNS_BIT | V0_CF_BIT | V0_SUB_BIT)
#define V0_CMP_OP     (V0_CMP_BIT | V0_SUB_BIT)
#define V0_CMU_OP     (V0_UNS_BIT | V0_CMP_BIT | V0_SUB_BIT)
/* MULTI-unit */
#define V0_MULTI_UNIT 0x04
#define V0_REM_BIT    (1 << 1)  // remainder
#define V0_HI_BIT     (1 << 1)  // return high word of result
#define V0_DIV_BIT    (1 << 2)  // division
#define V0_RPC_BIT    (1 << 3)  // inverse reciprocal
#define V0_MUL_OP     0x00
#define V0_MLU_OP     (V0_UNS_BIT)
#define V0_MLH_OP     (V0_HI_BIT)
#define V0_MHU_OP     (V0_UNS_BIT | V0_HI_BIT)
#define V0_DIV_OP     (V0_DIV_BIT)
#define V0_DVU_OP     (V0_UNS_BIT | V0_DIV_BIT)
#define V0_REM_OP     (V0_REM_BIT | V0_DIV_BIT)
#define V0_RMU_OP     (V0_UNS_BIT | V0_REM_BIT | V0_DIV_BIT)
#define V0_RPS_OP     (V0_RPC_BIT)
#define V0_RPU_OP     (V0_UNS_BIT | V0_RPC_BIT)
/* BIT-unit */
#define V0_BIT_UNIT   0x05
#define V0_CLR_BIT    (1 << 0)  // clear bit
#define V0_ODD_BIT    (1 << 0)  // odd parity
#define V0_ONE_BIT    (1 << 0)  // one-bits   (Hamming weight)
#define V0_DCD_BIT    (1 << 0)  // decode binary-coded decimal
#define V0_CNT_BIT    (1 << 1)  // bit-counting instruction
#define V0_PAR_BIT    (1 << 2)  // parity instruction
#define V0_BCD_BIT    (1 << 2)  // binary-coded decimal
#define V0_CRC_BIT    (1 << 3)  // cyclic-redundancy checksum
#define V0_COND_BIT   (1 << 3)  // conditional load/store
#define V0_NEG_MASK   0x05      // arithmetic negation: R2 = -R1
#define V0_SWP_MASK   0x0b      // swap byte order
#define V0_HSH_MASK   0x0c      // hash instruction
#define V0_SET_MASK   0x0e      // bit set or clear
#define V0_SEX_OP     0x00
#define V0_ZEX_OP     (V0_UNS_BIT)
#define V0_CLZ_OP     (V0_CNT_BIT)
#define V0_HAM_OP     (V0_ONE_BIT | V0_CNT_BIT)
#define V0_PAR_OP     (V0_PAR_BIT)
#define V0_NEG_OP     V0_NEG_MASK
#define V0_BCD_OP     (V0_WR_BIT | V0_BCD_BIT)
#define V0_DCD_OP     (V0_RD_BIT | V0_WR_BIT | V0_BCD_BIT)
#define V0_CRC_OP     (V0_CRC_BIT)
#define V0_CST_OP     (V0_RD_BIT � V0_COND_BIT)
#define V0_CLD_OP     (V0_WR_BIT � V0_COND_BIT)
#define V0_SWP_OP     V0_SWP_MASK
#define V0_HSH_OP     V0_HSH_MASK
#define V0_HUN_OP     (V0_RD_BIT | V0_HSH_MASK)
#define V0_SET_OP     V0_SET_MASK
#define V0_CLR_OP     (V0_CLR_BIT | V0_SET_MASK)
/* MEM-unit */
#define V0_MEM_UNIT   0x06
#define V0_CL_BIT     (1 << 2)  // cacheline operation
#define V0_BAR_BIT    (1 << 3)  // memory barrier
#define V0_IPG_MASK   0x03
#define V0_MSW_MASK   0x0a
#define V0_SR_MASK    0x0d
#define V0_LEA_OP     0x00
#define V0_LDR_OP     (V0_RD_BIT)
#define V0_STR_OP     (V0_WR_BIT)
#define V0_IPG_OP     V0_IPG_MASK
#define V0_CLR_OP     (V0_CL_BIT)
#define V0_CPF_OP     (V0_RD_BIT | V0_CL_BIT)
#define V0_CFL_OP     (V0_WR_BIT | V0_CL_BIT)
#define V0_BAR_OP     (V0_BAR_BIT)
#define V0_BRD_OP     (V0_RD_BIT | V0_BAR_BIT)
#define V0_BWR_OP     (V0_WR_BIT | V0_BAR_BIT)
#define V0_LFL_OP     (V0_RD_BIT | V0_MSW_MASK)
#define V0_SFL_OP     (V0_WR_BIT | V0_MSW_MASK)
#define V0_LDX_OP     (V0_RD_BIT | V0_SR_MASK)
#define V0_STX_OP     (V0_WR_BIT | V0_SR_MASK)
/* STACK-unit */
#define V0_STACK_UNIT 0x07
#define V0_RNG_BIT    (1 << 1)  // range operation
#define V0_FRM_BIT    (1 << 2)  // MKF
#define V0_FLG_BIT    (1 << 2)  // machine status word
#define V0_XR_BIT     (1 << 3)
#define V0_PSH_OP     0x00
#define V0_POP_OP     (V0_RD_BIT)
#define V0_PSM_OP     (V0_RNG_BIT)
#define V0_POM_OP     (V0_RD_BIT | V0_RNG_BIT)
#define V0_PSF_OP     (V0_FLG_BIT)
#define V0_POF_OP     (V0_RD_BIT | V0_FLG_BIT)
#define V0_MKF_OP     (V0_RNG_BIT | V0_FRM_BIT)
#define V0_PSX_OP     (V0_XR_BIT)
#define V0_POX_OP     (V0_RD_BIT | V0_XR_BIT)

/* ATOM-unit */
#define V0_ATOM_UNIT  0x08
#define V0_SYN_BIT    (1 << 0)  // synchronous operation
#define V0_DBL_BIT    (1 << 0)  // dual-word/pointer operation
#define V0_CAS_BIT    (1 << 1)  // compare and swap
#define V0_TST_BIT    (1 << 2)  // bit-test operation
#define V0_LNK_OP     0x00
#define V0_STC_OP     (V0_SYN_BIT)
#define V0_CAS_OP     (V0_CAS_BIT)
#define V0_BTS_OP     (V0_TST_BIT)
#define V0_BTC_OP     (V0_CLR_BIT | V0_TST_BIT)
/* FLOW-unit */
#define V0_FLOW_UNIT  0x09
#define V0_NOT_BIT    (1 << 0)
#define V0_REL_BIT    (1 << 0)  // PC-relative addressing
#define V0_OF_BIT     (1 << 0)  // branch on overflow
#define V0_EQ_BIT     (1 << 1)  // branch on equal
#define V0_LT_BIT     (1 << 2)  // branch if less than
#define V0_CF_BIT     (1 << 3)  // branch if greater than
#define V0_OF_MASK    0x0a
#define V0_JMP_OP     0x00
#define V0_JMR_OP     (V0_REL_BIT)
#define V0_BEQ_OP     (V0_EQ_BIT)
#define V0_BZF_OP     V0_BEQ_OP
#define V0_BNE_OP     (V0_NOT_BIT � V0_EQ_BIT)
#define V0_BNZ_OP     (V0_BNE_OP)
#define V0_BLT_OP     (V0_LT_BIT)
#define V0_BGE_OP     (V0_NOT_BIT | V0_LT_BIT)
#define V0_BLE_OP     (V0_EQ_BIT | V0_LT_BIT)
#define V0_BGT_OP     (V0_NOT_BIT | V0_EQ_BIT | V0_LT_BIT)
#define V0_BCF_OP     V0_CF_BIT
#define V0_BNC_OP     (V0_NOT_BIT | V0_CF_BIT)
#define V0_BOF_OP     V0_OF_MASK
#define V0_BNO_OP     (V0_NOT_BIT | V0_OF_MASK)
/* SUBR-unit */
#define V0_SUBR_UNIT  0x0a
#define V0_TERM_BIT   (1 << 0)  // unusual return  (interrupt or thread)
#define V0_FIN_BIT    (1 << 0)  // FIN
#define V0_SYS_BIT    (1 << 0)  // system-mode operation
#define V0_SUBR_BIT   (1 << 1)  // subroutine operation
#define V0_RET_BIT    (1 << 2)  // return from subroutine
#define V0_THR_BIT    (1 << 3)  // thread operation
#define V0_BEG_OP     0x00
#define V0_CSR_OP     (V0_SUBR_BIT)
#define V0_FIN_OP     (V0_FIN_BIT)
#define V0_SYS_OP     (V0_SYS_BIT |�V0_SUBR_BIT)
#define V0_RET_OP     (V0_RET_BIT)
#define V0_IRT_OP     (V0_TERM_BIT | V0_RET_BIT)
#define V0_SRT_OP     (V0_SYS_BIT | V0_RET_BIT)
#define V0_THR_OP     (V0_THR_BIT)
#define V0_THX_OP     (V0_TERM_BIT | V0_THR_BIT)
#define V0_THS_OP     (V0_SYS_BIT | V0_THR_BIT)
/* SYS-unit */
#define V0_SYS_UNIT   0x0b
#define V0_ON_BIT     (1 << 0)  // enable operation
#define V0_MASK_BIT   (1 << 1)  // mask operation
#define V0_INT_BIT    (1 << 2)  // software interrupt
#define V0_EV_BIT     (1 << 3)  // event operation
#define V0_HLT_MASK   0x06      // halt
#define V0_CLI_OP     0x00
#define V0_STI_OP     (V0_ON_BIT)
#define V0_SIM_OP     (V0_MASK_BIT)
#define V0_LIM_OP     (V0_ON_BIT | V0_MASK_BIT)
#define V0_INT_OP     (V0_INT_BIT)
#define V0_SIV_OP     (V0_ON_BIT | V0_INT_BIT)
#define V0_HLT_OP     V0_HLT_MASK
#define V0_RST_OP     (V0_ON_BIT | V0_HLT_MASK)
#define V0_WFE_OP     (V0_EV_BIT)
#define V0_SEV_OP     (V0_ON_BIT | V0_EV_BIT)
/* IO-unit */
#define V0_IO_UNIT    0x0c
#define V0_MAP_BIT    (1 << 0)
#define V0_IR_BIT     (1 << 1)
#define V0_CMD_BIT    (1 << 2)
#define V0_IOC_OP     0x00
#define V0_ILD_OP     (V0_RD_BIT)
#define V0_IST_OP     (V0_WR_BIT)
#define V0_ILM_OP     (V0_MAP_BIT)
#define V0_IRD_OP     (V0_RD_BIT | V0_MAP_BIT)
#define V0_IWD_OP     (V0_WR_BIT | V0_MAP_BIT)

#endif /* __V0_VM_INS_H__ */

