#ifndef __V0_ISA_H__
#define __V0_ISA_H__

#include <v0/conf.h>
#include <stdint.h>
#include <v0/ins.h>

#define V0_INSTS_MAX 256

/*
 * V0 INSTRUCTION SET ARCHITECTURE
 * -------------------------------
 */

/* instruction opcodes */
#define V0_LOGIC_UNIT 0x01 // NOT, AND, IOR, XOR
#define V0_SHIFT_UNIT 0x02 // SHL, SHR, SAR, ROL, ROR
#define V0_ADD_UNIT   0x03 // ADD, ADU, ADC, AUC, SUB, SBU, SBC, SUC, CMP
#define V0_MUL_UNIT   0x04 // MUL, MLU, MLH, MHU, DIV, DVU, DIR, DUR, RPS, RPU
#define V0_SIGN_UNIT  0x05
#define V0_MISC_UNIT  0x06
#define V0_MEM_UNIT   0x07
#define V0_STK_UNIT   0x08
#define V0_BIT_UNIT   0x09
#define V0_ATOM_UNIT  0x0a
#define V0_JMP_UNIT   0x0b
#define V0_SUB_UNIT   0x0c
#define V0_SYS_UNIT   0x0d
#define V0_IO_UNIT    0x0e
#define V0_CTL_UNIT   0x0f

/* integer limits */
#define V0_IMM_MIN    (-0x7fff - 1)
#define V0_IMM_MAX    0x7fff

/* bits for MFR feature register and CR control register */
#define V0_THR_BIT    (1 << 31) // multitasking and multicore support
#define V0_VM_BIT     (1 << 30) // [virtual] memory management/paging support
#define V0_FPU_BIT    (1 << 29) // floating-point instructions present
#define V0_FPM_BIT    (1 << 28) // fixed-point instructions present
#define V0_DSP_BIT    (1 << 27) // standard signal processing support
#define V0_SIMD_BIT   (1 << 26) // single instruction, multiple data vectors
#define V0_VEC_BIT    (1 << 25) // vector operation

/* bits for CR control register */
/*
 * V0 REGISTER FILE
 * ----------------
 *
 * General-Purpose Registers
 * -------------------------
 * %r0          zero register; reads always return zero, writes are NOPs
 * %zr          synonymous with %r0
 * %r1          function return value, first caller save register
 * %r2..%7      function argument registers (the rest on stack)
 * %r7..%r15    scratch registers; callee-save
 *
 * System Registers
 * ----------------
 * %pc          program counter i.e. instruction pointer
 * %fp          frame pointer
 * %sp          stack pointer
 * %msw         machine status word (carry, overflow, interrupts, etc.)
 * %ln          function return address back to caller
 * %tr          thread-local storage segment info
 * %iv          interrupt vector [handler table] base address
 * %im          interrupt mask register; 1-bit denote enabled interrupts
 * %pd          page-directory base address for virtual memory
 * %io          I/O-map descriptor with paramters
 * %fp0         system-mode execution stack frame-pointer
 * %sp0         system-mode stack pointer
 * %cr          control register (for system control)
 * %mfr         machine feature word, e.g. indication of optional hardware
 * %fps         fixed-point status register
 * %fst         floating-point status register
 */
#define V0_ZERO_REG      V0_R0_REG // always returns zero, writes equal to NOP
#define V0_RET_REG       V0_R1_REG // function return value register
#define V0_REG_ARGS      7         // R1..R7, function register arguments
/* CALLER-SAVE REGISTERS */
#define V0_ZR_REG        V0_R0_REG
#define V0_LN_REG        V0_R15_REG // link register, i.e. return address
#define V0_R0_REG        0x00 // zero register
#define V0_R1_REG        0x01 // function return value, first function argument
#define V0_R2_REG        0x02 // extra return value, second function argument
#define V0_R3_REG        0x03 // third function argument
#define V0_R4_REG        0x04 // fourth function argument
#define V0_R5_REG        0x05 // fifth function argument
#define V0_R6_REG        0x06 // sixth function argument
#define V0_R7_REG        0x07 // seventh function argument
/* CALLEE-SAVE REGISTERS */
#define V0_R8_REG        0x08 // scratch register #1
#define V0_R9_REG        0x09 // scratch register #2
#define V0_R10_REG       0x0a // scratch register #3
#define V0_R11_REG       0x0b // scratch register #4
#define V0_R12_REG       0x0c // scratch register #5
#define V0_R13_REG       0x0d // scratch register #6
#define V0_R14_REG       0x0e // scratch register #7
#define V0_R15_REG       0x0f // stratch register #8
#define V0_INT_REGS      16   // # of integer/scalar registers
#define V0_SAVE_REG_MIN  V0_R1_REG
#define V0_SAVE_REG_MAX  V0_R7_REG
#define V0_SAVE_REGS     7    // caller saves r1..r7, callee r8..r15
/* SYSTEM REGISTERS */
#define V0_PC_ID         0x00 // program counter
#define V0_FP_ID         0x01 // frame pointer
#define V0_SP_ID         0x02 // stack pointer
#define V0_MSW_ID        0x03 // machine status word
#define V0_LN_ID         0x04 // link register (return address)
#define V0_TR_ID         0x05 // thread status register
#define V0_IV_ID         0x06 // interrupt vector address
#define V0_IM_ID         0x07 // interrupt-mask (1-bit enabled)
#define V0_PD_ID         0x08 // page directory address + flags
#define V0_IOM_ID        0x09 // I/O descriptor map address + flags
#define V0_FP0_ID        0x0a // system frame-pointer (ring 0)
#define V0_SP0_ID        0x0b // system stack-pointer (ring 0)
#define V0_BR_ID         0x0c // bound-range register (buffer violations)
#define V0_VC_ID         0x0d // local variable count
#define V0_CR_ID         0x0e // control-register
#define V0_MFR_ID        0x0f // machine feature word
#define V0_SYS_REGS      16
#define V0_PC_REG        V0_SYSREG(V0_PC_ID)
#define V0_FP_REG        V0_SYSREG(V0_FP_ID)
#define V0_SP_REG        V0_SYSREG(V0_SP_ID)
#define V0_MSW_REG       V0_SYSREG(V0_MSW_ID)
#define V0_LN_REG        V0_SYSREG(V0_LN_ID)
#define V0_TR_REG        V0_SYSREG(V0_TR_ID)
#define V0_IV_REG        V0_SYSREG(V0_IV_ID)
#define V0_IM_REG        V0_SYSREG(V0_IM_ID)
#define V0_PD_REG        V0_SYSREG(V0_PD_ID)
#define V0_IOM_REG       V0_SYSREG(V0_IOM_ID)
#define V0_FP0_REG       V0_SYSREG(V0_FP0_ID)
#define V0_SP0_REG       V0_SYSREG(V0_SP0_ID)
#define V0_BR_REG        V0_SYSREG(V0_BR_ID)
#define V0_VC_REG        V0_SYSREG(V0_VC_ID)
#define V0_CR_REG        V0_SYSREG(V0_CR_ID)
#define V0_MFR_REG       V0_SYSREG(V0_MFR_ID)
/* system register IDs */
#define V0_STD_REGS      (V0_INT_REGS + V0_SYS_REGS) // total # of registers
#define V0_FPU_REGS      (V0_INT_REGS)
#define V0_SYSREG(x)     (V0_INT_REGS + (x))         // system register indices
/* values for regs[V0_MSW_REG] */
#define V0_MSW_DEF_BITS  (V0_IF_BIT)
#define V0_MSW_PF_BIT    (1 << 0)   // system/privileged execution mode
#define V0_MSW_ZF_BIT    (1 << 1)   // zero
#define V0_MSW_LT_BIT    (1 << 2)   // carry-flag, return bit for BTR, BTS, BTC
#define V0_MSW_GT_BIT    (1 << 3)   // carry-flag, return bit for BTR, BTS, BTC
#define V0_MSW_CF_BIT    (1 << 4)   // carry
#define V0_MSW_OF_BIT    (1 << 5)   // overflow
#define V0_MSW_IF_BIT    (1 << 6)   // interrupts pending
/* values for MFW- and MSW-registers */
#define V0_M_PG_BIT      (1 << 31)  // paging/virtual memory enabled
#define V0_M_FP_BIT      (1 << 30)  // floating-point processor present
#define V0_M_FM_BIT      (1 << 29)  // fixed-point support
#define V0_M_GP_BIT      (1 << 28)  // graphics processor present
#define V0_M_DP_BIT      (1 << 27)  // DSP support
#define V0_M_MD_BIT      (1 << 26)  // SIMD support
#Define V0_M_VP_BIT      (1 << 25)  // vector processor present
#define V0_M_MP_BIT      (1 << 24)  // multiprocessor support
#define V0_M_IO_BIT      (1 << 23)  // I/O permission control
#define V0_M_TF_BIT      (1 << 22)  // multithread support
/* BR-register */
#define V0_BR_LOW_LIM    (1 << 31)
#define V0_BR_ADR_MASK   (~V0_BR_LOW_LIM)

#endif /* __V0_ISA_H__ */

