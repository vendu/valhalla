#ifndef __V0_VM_ISA_H__
#define __V0_VM_ISA_H__

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
 * i	immediate               op->arg[0].i32 etc.
 * d    direct                  vm->regs[op->reg]
 * m    memory [address]	v0getarg1(vm, op), v0getarg2(vm, op)
 *
 * Instruction Prefixes
 * --------------------
 *
 * 0xff    - memory bus lock
 * The MLK-prefix may be used to lock the memory bus for a single operation of
 * - SBC, NOT, AND, XOR, OR,INC, DEC, ADD, SDC, SUB, BTR, BTS, BTC, LDR, STR
 */

/* bits for MFR feature register */

#define V0_MFR_THR (1U << 0) // multithread support
#define V0_MFR_MMU (1U << 0) // memory management/paging support
#define V0_MFR_FPU (1U << 1) // floating-point instructions present
#define V0_MFR_FPM (1U << 4) // fixed-point instructions present

#define V0_NOP     0xff

/*
 * ARITHMETIC-LOGICAL OPERATIONS
 * -----------------------------
 *
 * Notes
 * -----
 * - reciprocals as well as results are stored in [64-bit] registers
 * - division is done by computing a reciprocal and multiplying the dividend
 *   with it EDIT: IF feasible... This may need to be done with floating-point
 *   operations.
 *
 * Instructions
 * ------------
 *
 * Mnemo Opcode  Brief
 * ----- ------  -----
 * NOT   0x01    bitwise inverse
 * AND   0x02    logical AND
 * IOR   0x03    logical inclusive OR
 * XOR   0x04    logical exclusive OR
 *
 * SHL   0x05    shift left logical
 * SHR   0x06    shift right logical (fill with zero)
 * SAR   0x07    shift right arithmetic (fill with sign-bit)
 *
 * INC	 0x08    increment by one
 * DEC   0x09    decrement by one
 * ADD   0x0a    addition (ignore over- and underflows)
 * ADU   0x0b    unsigned addition
 * ADC   0x0c    addition with carry-flag
 * SUB   0x0d    subtraction; ignore underflow
 * SBU   0x0e    unsigned subtraction
 * SBC   0x0f    subtract with carry-flag
 * CMP   0x10    compare (subtract + set MSW-flags)
 *
 * CRP   0x11    calculate reciprocal
 * MUL   0x12    multiplication, returns low word
 * MLU   0x13    unsigned multiplication, returns low word
 * MLH   0x14    multiplication, returns high word
 * MHU   0x15    unsigned multiplication, returns high word
 * DIV   0x16    division
 * DVU   0x17    unsigned division
 *
 * SEX	 0x18    sign-extend operand to given size
 * ZEX   0x19    zero-extend operand to given size
 *
 * LEA   0x1a    load effective address
 *
 * CLZ	 0x1b    count leading zero bits
 *
 * HAM   0x1c    compute Hamming weight/bit population (number of 1-bits)
 *
 * BSW   0x1d    swap byte-order
 *
 * LDR   0x1e    load value into register
 * STR   0x1f    store register into memory
 * RSR   0x20    read system register
 * WSR   0x21    write system register
 *
 * BTS   0x22    test and set bit; return original value in ZF-bit
 * BTC   0x23    test and clear bit; return original value in ZF-bit
 * CAS   0x24    compare and swap; return original value in register val
 *
 * LDL   0x25    load linked
 * STC   0x26    store conditional (if unmodified)
 */
/* logical instructions */
#define V0_NOT 0x01
#define V0_AND 0x02
#define V0_IOR 0x03
#define V0_XOR 0x04
/* shifter instructions */
#define V0_SHL 0x05
#define V0_SHR 0x06
#define V0_SAR 0x07
/* adder instructions */
#define V0_INC 0x08 // V0_ATOM_BIT
#define V0_DEC 0x09 // V0_ATOM_BIT
#define V0_ADD 0x0a // V0_ATOM_BIT
#define V0_ADC 0x0b // V0_ATOM_BIT      - CF
#define V0_SUB 0x0c // V0_ATOM_BIT
#define V0_SBC 0x0d // V0_ATOM_BIT      - CF
#define V0_CMP 0x0e
/* multiplication and division operations */
#define V0_CRP 0x0f
#define V0_MUL 0x10
#define V0_MUH 0x11
#define V0_DIV 0x12
/* sign-extend */
#define V0_SEX 0x13
/* load effective address */
#define V0_LEA 0x14
/* bit- and byte-operations */
#define V0_CLZ 0x15
#define V0_HAM 0x16
#define V0_BSW 0x17
/* load-store instructions */
#define V0_LDR 0x18
#define V0_STR 0x19
#define V0_RSR 0x1a
#define V0_WSR 0x1b
/* set and clear bits */
#define V0_BTS 0x1c // NEW
#define V0_BTC 0x1d // NEW
/* V0_MULTICORE extensions */
#define V0_LDL 0x1e // V0_ACQ_BIT
#define V0_STC 0x1f // clear cacheline ACQ-bit

/*
 * CONTROL FLOW OPERATIONS
 * -----------------------
 *
 * Instructions
 * ------------
 *
 * NOTES
 * -----
 * - with register addressing, register ID 0 means PC-relative (PIC)
 * - parm - size shift count
 * - type width in bits: b - 8, h - 16, w - 32, q - 64
 * - corresponding parm: b - 0, h - 1, w - 2, q - 3
 *
 * Mnemo   Opcode  Brief
 * -----   ------  -----
 * JMP     0x20    jump; branch unconditionally
 * BEQ     0x21    branch if equal/zero
 * BNE     0x22    branch if not equal/zero
 * BLT     0x23    branch if less than
 * BUL     0x24    branch if unsigned less than
 * BGT     0x25    branch if greater than
 * BUG     0x26    branch if unsigned greater than
 * BAF     0x27    branch if arithmetic flags (OF, CF) set
 * CSR     0x28    call subroutine
 * BEG     0x29    function prologue; adjust stack
 * FIN     0x2a    function epilogue; adjust stack
 * RET     0x2b    return from subroutine
 * SYS     0x2c    enter system mode
 * SRT     0x2d    return from system modr
 * THR     0x2e    launch thread
 * THX     0x2f    exit thread
 */
#define V0_JMP 0x30
#define V0_BEQ 0x31
#define V0_BNE 0x32
#define V0_BLT 0x33
#define V0_BUL 0x34
#define V0_BGT 0x35
#define V0_BUG 0x36
#define V0_BAF 0x37
#define V0_CSR 0x38
#define V0_BEG 0x39
#define V0_FIN 0x3a
#define V0_RET 0x3b
#define V0_SYS 0x3c
#define V0_SRT 0x3d
/* V0_MULTICORE extensions */
#define V0_THR 0x3e // launch thread at *reg2, direct argument (uintptr_t)
#define V0_THX 0x3f // exit thread, provide exit status to kernel

/*
 * STACK OPERATIONS
 * ----------------
 *
 * Mnemo   Opcode  Brief
 * -----   ------  -----
 * PSH     0x40    push register
 * POP     0x41    pop register
 * PSM     0x42    push many registers
 * POM     0x43    pop many registers
 *
 * Opcode Notes
 * ------------
 */
#define V0_PSH 0x40 // push register reg2
#define V0_POP 0x41 // pop register reg2
#define V0_PSM 0x42 // push 1 << parm registers starting from val
#define V0_POM 0x43 // pop 1 << parm registers starting from val

/*
 * SYNCHRONOUS OPERATIONS SUPPORT
 * ------------------------------
 *
 * Instructions
 * ------------
 *
 * Mnemo   Opcode  Brief
 * -----   ------  -----
 * CPF     0x50    cache prefetch
 * BAR     0x51    full memory barrier
 * BRD     0x52    memory read-barrier
 * BWR     0x53    memory write-barrier
 * WFE     0x54    sleep/wait for event
 * SEV     0x55    signal event
#if 0
 * CLK     0x56    lock cacheline
 * CLR     0x57    release cacheline
#endif
 */
#define V0_CPF 0x50
/* V0_MULTICORE extensions */
#define V0_BAR 0x51
#define V0_BRD 0x52
#define V0_BWR 0x53
#define V0_WFE 0x54
#define V0_SEV 0x55
#if 0
#define V0_CLK 0x56
#define V0_CLR 0x57
#endif

/*
 * INPUT-OUTPUT OPERATIONS
 * -----------------------
 *
 * Mnemo   Opcode  Brief
 * -----   ------  -----
 * ICD     0x60    configure I/O device
 * IMM     0x61    map i/O memory
 * IRD     0X62    read I/O data
 * IWR     0x63    write I/O data
 *
 * Notes
 * -----
 * IRD, IWR - bits 0-1 of parm are shift count for operation size
 */
#define V0_ICD 0x60
#define V0_IMM 0x61
#define V0_IRD 0x62
#define V0_IWR 0x63

/*
 * SYSTEM OPERATIONS
 * -----------------
 *
 * Mnemo   Opcode  Brief
* -----   ------  -----
 * HLT     0x70    halt the processor
 * RST     0x71    reset system
 * CLI     0x72    disable all interrupts, optionally store mask
 * STI     0x73    enable interrupts, optionally restore/set mask
 * INT     0x74    send/signal software interrupt  number (8-bit)
 * SLP     0x75    sleep/wait for interrupt
 * FPG     0x76    flush memory TLB-entry
 * FLS     0x77    flush cacheline
 */
#define V0_HLT 0x70
#define V0_RST 0x71
#define V0_CLI 0x72
#define V0_STI 0x73
#define V0_INT 0x74
#define V0_SLP 0x75
/* V0_VIRTMEM extensions */
#define V0_FPG 0x76
#define V0_FLS 0x77

#endif /* __V0_VM_ISA_H__ */

