#ifndef __VALHALLA_V0_MACH_H__
#define __VALHALLA_V0_MACH_H__

#include <stdint.h>

#define V0_NINST_MAX 256

/* Valhalla 0 (V0) processor machine interface */

/*
 * V0 Instruction Set
 * ------------------
 *
 * Argument Descriptions
 * ---------------------
 * r    register                vm->regs[op->reg]
 * i	immediate               op->val
 * d    direct                  op->arg[0].i32 etc.
 * m    memory [address]	v0getarg1(vm, op), v0getarg2(vm, op)
 *
 * Instruction Prefixes
 * --------------------
 *
 * 0xff    - memory bus lock
 * The MLK-prefix may be used to lock the memory bus for a single operation of
 * - INC, DEC, ADD, SDC, SUB, SBC, NOT, AND, XOR, OR, BTR, BTS, BTC, LDR, STR
 *
 */

/* bits for MFR feature register */

/* TODO: integrate FPU-operations into the basic instruction set */

#define V0_MFR_MMU (1U << 0) // memory management unit present
#define V0_MFR_FPU (1U << 1) // floating-point processor present
#define V0_MFR_GPU (1U << 2) // graphics processor present
#define V0_MFR_DSP (1U << 3) // digital signal processor present
#define V0_MFR_FPM (1U << 4) // fixed-point processor present

#define V0_NOP     0xffff    // no-operation, 16-bit all-1 header (32-bit size)

/*
 * ARITHMETIC-LOGICAL OPERATIONS
 * -----------------------------
 *
 * Instructions
 * ------------
 *
 * Mnemo Opcode  Src    Dest	Brief
 * ----- ------  ---    ----    -----
 * INC	 0x01           r       increment by one
 * DEC   0x02           r       decrement by one
 * CMP   0x03    rid    r	compare (subtract + set MSW-flags)
 * ADD   0x04    rid	r	addition (ignore over and underflows)
 * ADC   0x05    rid    r       addition with carry-flag
 * SUB   0x06    rid	r	subtract; ignore underflow
 * SBC   0x07    rid    r       subtract with carry-flag
 * NOT   0x08           r       bitwise inverse
 * AND   0x09    rid    r       logical AND
 * XOR   0x0a    rid    r       logical exclusive OR
 * IOR   0x0b    rid    r       logical inclusive OR
 *
 * Macros
 * ------
 */
#define V0_INC 0x01 //   r     increment by one
#define V0_DEC 0x02 //   r     decrement by one
#define V0_CMP 0x03 //   r, r   compare (subtract + set flags)
#define V0_ADD 0x04 //   ri, r   addition
#define V0_ADC 0x05 //   ri, r   addition with carry
#define V0_SUB 0x06 //   ri, r   subtraction
#define V0_SBC 0x07 //   ri, r   subraction with borrow
#define V0_NOT 0x08 //   r       reverse all bits
#define V0_AND 0x09 //   ri, r   logical AND
#define V0_XOR 0x0a //   ri, r   logical exclusive OR
#define V0_IOR 0x0b //   ri, r   logical OR

/*
 * SHIFT UNIT
 * ----------
 * SHL   0x0c    ri     r       shift left [logical/fill with zero]
 * SHR   0x0d    ri     r       shift right logical (fill with zero)
 * SAR   0x0e    ri     r       shift right arithmetic (fill with sign-bit)
 */
#define V0_SHL 0x0c
#define V0_SHR 0x0d
#define V0_SAR 0x0e

/*
 * MULTIPLICATION AND DIVISION
 * ---------------------------
 *
 * Notes
 * -----
 * - reciprocals as well as results are stored in [64-bit] registers
 * - division is done by computing a reciprocal and multiplying the dividend
 *   with it EDIT: IF feasible... This needs to be done with floating-point
 *   operations.
 *
 * Instructions
 * ------------
 *
 * Mnemo Opcode  Src    Dest	Brief
 * ----- ------  ---    ----    -----
 * CRP   0x10    rid    r       calculate reciprocal
 * MUL   0x11    rid    r       multiply and return low word
 * MUH   0x12    rid    r       multiply and return high word
 *
 * Opcode Notes
 * ------------
 * MUL/MUH		- 0x2 denotes high word return
 */
#define V0_CRP   0x10 //  ri, r   Multiplication
#define V0_MUL   0x11 //  ri, r   Multiplication, get low word of results
#define V0_MUH   0x12 //  ri, r   Multiplication, get high word of results

/*
 * BIT OPERATIONS
 * --------------
 *
 * Notes
 * -----
 * - the instructions BTS, BTC, BSH, BSW, BSL, BSQ in this section are optional;
 *   indicated with the BO-bit in MFW.
 *
 * Instructions
 * ------------
 *
 * Mnemo   Opcode  Src	Dest	Brief
 * -----   ------  ---  ----    -----
 * CLZ	   0x13    rid	r	Hamming weight/bit population (count of 1-bits)
 * HAM     0x14    rid  r       count leading zero bits
 * SWP     0x15    rid  r       byteswap
 *
 * Opcode Notes
 * ------------
 * BTR/BTS/BTC          - bits 0-1: 0 - reset, 1 - set, 2 - complement
 *                      - store original value in RF
 * SWP                  - parm - size shift count
 * - type width in bits: b - 8, h - 16, w - 32, q - 64, o - 128, h - 256
 */
#define V0_CLZ 0x13
#define V0_HAM 0x14
/* support for 0x15 is optional */
#define V0_SWP 0x15

/*
 * SYSTEM OPERATIONS
 * -----------------
 * HLT     0x16                 halt the processor
 * RST     0x17                 reset system
 * CLI     0x18                 disable all interrupts          none
 * STI     0x19    rm           set or restore interrupt-bits   adr/reg
 * INT     0x1a    i            send/signal software interrupt  number (8-bit)
 * SLP     0x1b                 sleep/wait for interrupt
 * WFE     0x1c                 sleep/wait for event
 * SEV     0x1d                 signal event
 */
#define V0_HLT 0x16 // halt (shutdown)
#define V0_RST 0x17 // reset
#define V0_CLI 0x18 // clear interrupt mask (disable interrupts)
#define V0_STI 0x19 // set/restore interrupt mask (enable interrupts)
#define V0_INT 0x1a // send/signal software interrupt
#define V0_SLP 0x1b // wait for interrupt; low-weight alternative to busy-spin
#define V0_WFE 0x1c // wait for event
#define V0_SEV 0x1d // signal event

/*
 * LOAD-STORE, STACK, AND INPUT-OUTPUT OPERATIONS
 * ----------------------------------------------
 *
 * Instructions
 * ------------
 *
 * Mnemo   Opcode  Src	Dest	Brief
 * -----   ------  ---  ----    -----
 * LDR     0x20    rd   r       load register
 * LDX     0x21    rd   r       load register and sign-extend
 * STR     0x22    r    rm      store register
 * STx     0x23    r    rm      store register with sign-extension
 * PSH     0x24    r            push register
 * POP     0x25    r            pop register
 * PSM     0x26    ri           push many registers             ilo, cntreg
 * POM     0x27    ri           pop many registers              ilo, cntreg
 * IRD     0x28    ri   r       read I/O port                   port, destreg
 * IWR     0x29    ri   r       write I/O port                  port, srcreg
 * ICF     0x2a    ri   r       configure I/O port              port, valreg
 *
 * Opcode Notes
 * ------------
 * LDR/STR/STX          - 0x01 denotes memory write, 0x02 means sign-extend,
 *                        parm - size shift count
 * - type width in bits: b - 8, h - 16, w - 32, q - 64, o - 128, h - 256
 * - corresponding parm: b - 0, h - 1, w - 2, q - 3, o - 4, h - 5
 * PSH/PSM, POP/POM     - 0x01 denotes push (write) operation
 *                      - dest is a register-bitmap (v0_R0_REG..v0_NREGS)
 * V0_IRD               - bits 0-1 of parm are shift count for operation size
 * V0_IWR               - bits 0-1 of parm are shift count for operation size
 */
#define V0_LDR 0x20
#define V0_LDX 0x21
#define V0_STR 0x22
#define V0_STX 0x23
#define V0_PSH 0x24
#define V0_POP 0x25
#define V0_PSM 0x26
#define V0_POM 0x27
#define V0_IRD 0x28
#define V0_IWR 0x29
#define V0_ICF 0x2a

/*
 * BRANCH AND SUBROUTINE OPERATIONS
 * --------------------------------
 *
 * Instructions
 * ------------
 *
 * Mnemo   Opcode  Src	Dest	Brief                           Arguments
 * -----   ------  ---  ----    -----                           ---------
 * JMP     0x30         rdm     jump; branch unconditionally
 * JMR     0x31         rdm     short [relative] jump
 * BIZ     0x32         rdm     branch if zero
 * BEQ     0x32         rdm     synonym for BIZ
 * BNZ     0x33         rdm     branch if non-zero
 * BNE     0x33         rdm     synonym for BNZ
 * BLT     0x34         rdm     branch if less than
 * BLE     0x35         rdm     branch if less than or equal
 * BGT     0x36         rdm     branch if greater than
 * BGE     0x37         rdm     branch if greater than or equal
 * BIO     0x38         rdm     branch if overflow
 * BNO     0x39         rdm     branch if no overflow
 * BIC     0x3a         rdm     branch if carry
 * BNC     0x3b         rdm     branch if no carry
 * CSR     0x3c         rdm     call subroutine
 * BEG     0x3d    ri   rdm     function prologue; adjust stack
 * FIN     0x3e    ri   rdm     function epilogue; adjust stack
 * RET     0x3f         rdm     return from subroutine
 *
 * Opcode Notes
 * ------------
 * JMP/JMR		- 0x01 denotes relative jump
 * BZ/BNZ, BLT/BLE,
 * BGT/BGE, BO/BNO,
 * BC/BNC               - 0x01 denotes zero-flag condition
 *
 */
#define V0_JMP 0x30
#define V0_JMR 0x31
#define V0_BIZ 0x32
#define V0_BEQ 0x32
#define V0_BNZ 0x33
#define V0_BNE 0x33
#define V0_BLT 0x34
#define V0_BLE 0x35
#define V0_BGT 0x36
#define V0_BGE 0x37
#define V0_BIO 0x38
#define V0_BNO 0x39
#define V0_BIC 0x3a
#define V0_BNC 0x3b
#define V0_CSR 0x3c
#define V0_BEG 0x3d
#define V0_FIN 0x3e
#define V0_RET 0x3f

#endif /* __VALHALLA_V0_MACH_H__ */

