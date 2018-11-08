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

/* addressing modes */
/* register addressing is detected with (op->reg) */
#define V0_IMM_ADR 0 // operand follows opcode, e.g. op->arg[0].i32
#define V0_DIR_ADR 1 // address follows opcode, op->arg[0].adr
#define V0_NDX_ADR 2 // reg[ndx << op->parm], ndx is val or follows opcode
#define V0_PIC_ADR 3 // pc[ndx << op->parm], ndx is val or follows opcode

/*
 * ARITHMETIC-LOGICAL OPERATIONS
 * -----------------------------
 */
/* flags for arithmetic and load-store instructions */
#define V0_ATOM_BIT (1U << 0) // LOCK-prefix
#define V0_SAT_BIT  (1U << 1) // SAT-prefix + size in parm
#define V0_SATU_BIT (1U << 2) // SATU-prefix + size in parm
/* flags for logical instructions */
#define V0_INV_BIT  (1U << 0)
/* flags for shift instructions */
#define V0_MASK_BIT (1U << 0)
#define V0_ROT_BIT  (1U << 1)
/*
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
 * Mnemo Opcode  Brief
 * ----- ------  -----
 * NOT   0x01    bitwise inverse
 * AND   0x02    logical AND
 * IOR   0x03    logical inclusive OR
 * XOR   0x04    logical exclusive OR
 * SHL   0x05    shift left [logical/fill with zero]
 * SHR   0x06    shift right logical (fill with zero)
 * SAR   0x07    shift right arithmetic (fill with sign-bit)
 * INC	 0x08    increment by one
 * DEC   0x09    decrement by one
 * ADD   0x0a    addition (ignore over- and underflows)
 * ADC   0x0b    addition with carry-flag
 * SUB   0x0c    subtract; ignore underflow
 * SBC   0x0d    subtract with carry-flag
 * CMP   0x0e    compare (subtract + set MSW-flags)
 * CRP   0x0f    calculate reciprocal
 * MUL   0x10    multiply and return low word
 * MUH   0x11    multiply and return high word
 * DIV   0x12    divide
 * SEX	 0x13    sign-extend operand to given size
 * LEA   0x14    load effective address
 * CLZ	 0x15    compute Hamming weight/bit population (count of 1-bits)
 * HAM   0x16    count leading zero bits
 * BSW   0x17    swap byte-order
 * BTS   0x18    test and set bit; return original value in ZF-bit
 * BTC   0x19    test and clear bit; return original value in ZF-bit
 * CAS   0x1a    compare and swap; return original value in register val
 * LDL   0x1b    load linked
 * STC   0x1c    store conditional (if unmodified)
 */
#define V0_NOT 0x01
#define V0_AND 0x02
#define V0_IOR 0x03
#define V0_XOR 0x04
#define V0_SHL 0x05
#define V0_SHR 0x06
#define V0_SAR 0x07
#define V0_INC 0x08 // V0_ATOM_BIT
#define V0_DEC 0x09 // V0_ATOM_BIT
#define V0_ADD 0x0a // V0_ATOM_BIT
#define V0_ADC 0x0b // V0_ATOM_BIT
#define V0_SUB 0x0c // V0_ATOM_BIT
#define V0_SBC 0x0d // V0_ATOM_BIT
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
/* set and clear bits */
#define V0_BTS 0x18 // NEW
#define V0_BTC 0x19 // NEW
/* load-store instructions */
#define V0_LDR 0x1a
#define V0_STR 0x1b
#define V0_RSR 0x1c
#define V0_WSR 0x1d
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
#define V0_JMP 0x20
#define V0_BEQ 0x21
#define V0_BNE 0x22
#define V0_BLT 0x23
#define V0_BUL 0x24
#define V0_BGT 0x25
#define V0_BUG 0x26
#define V0_BAF 0x27
#define V0_CSR 0x28
#define V0_BEG 0x29
#define V0_FIN 0x2a
#define V0_RET 0x2b
#define V0_SYS 0x2c
#define V0_SRT 0x2d
/* V0_MULTICORE extensions */
#define V0_THR 0x2e // launch thread at *reg2, direct argument (uintptr_t)
#define V0_THX 0x2f // exit thread, provide exit status to kernel

/*
 * LOAD-STORE OPERATIONS
 * ---------------------
 *
 * Instructions
 * ------------
 *
 * Mnemo   Opcode  Brief
 * -----   ------  -----
 * LDR     0x30    load register
 * STR     0x31    store register
 * RSR     0x32    read system register
 * WSR     0x33    write system register
 *
 ä Notes
 * -----
 * - parm - size shift count
 * - type width in bits: b - 8, h - 16, w - 32, q - 64
 * - corresponding parm: b - 0, h - 1, w - 2, q - 3
 */

/*
 * STACK OPERATIONS
 * ----------------
 *
 * Mnemo   Opcode  Brief
 * -----   ------  -----
 * PSH     0x34    push register
 * POP     0x35    pop register
 * PSM     0x36    push many registers
 * POM     0x37    pop many registers
 *
 * Opcode Notes
 * ------------
 */
#define V0_PSH 0x34 // push register reg2
#define V0_POP 0x35 // pop register reg2
#define V0_PSM 0x36 // push 1 << parm registers starting from val
#define V0_POM 0x37 // pop 1 << parm registers starting from val

/*
 * SYNCHRONOUS OPERATIONS SUPPORT
 * ------------------------------
 *
 * Instructions
 * ------------
 *
 * Mnemo   Opcode  Brief
 * -----   ------  -----
 * CPF     0x38    cache prefetch
 * BAR     0x39    full memory barrier
 * BRD     0x3a    memory read-barrier
 * BWR     0x3b    memory write-barrier
 * WFE     0x3c    sleep/wait for event
 * SEV     0x3d    signal event
#if 0
 * CLK     0x3e    lock cacheline
 * CLR     0x3f    release cacheline
#endif
 */
#define V0_CPF 0x38
/* V0_MULTICORE extensions */
#define V0_BAR 0x39
#define V0_BRD 0x3a
#define V0_BWR 0x3b
#define V0_WFE 0x3c
#define V0_SEV 0x3d
#if 0
#define V0_CLK 0x3e
#define V0_CLR 0x3f
#endif

/*
 * INPUT-OUTPUT OPERATIONS
 * -----------------------
 *
 * Mnemo   Opcode  Brief
 * -----   ------  -----
 * ICD     0x40    configure I/O device
 * IMM     0x41    map i/O memory
 * IRD     0X42    read I/O data
 * IWR     0x43    write I/O data
 *
 * Notes
 * -----
 * IRD, IWR - bits 0-1 of parm are shift count for operation size
 */
#define V0_ICD 0x40
#define V0_IMM 0x41
#define V0_IRD 0x42
#define V0_IWR 0x43

/*
 * SYSTEM OPERATIONS
 * -----------------
 *
 * Mnemo   Opcode  Brief
* -----   ------  -----
 * HLT     0x50    halt the processor
 * RST     0x51    reset system
 * CLI     0x52    disable all interrupts, optionally store mask
 * STI     0x53    enable interrupts, optionally restore/set mask
 * INT     0x54    send/signal software interrupt  number (8-bit)
 * SLP     0x55    sleep/wait for interrupt
 * FPG     0x56    flush memory TLB-entry
 * FLS     0x57    flush cacheline
 */
#define V0_HLT 0x50
#define V0_RST 0x51
#define V0_CLI 0x52
#define V0_STI 0x53
#define V0_INT 0x54
#define V0_SLP 0x55
/* V0_VIRTMEM extensions */
#define V0_FPG 0x56
#define V0_FLS 0x57

#endif /* __V0_VM_ISA_H__ */

