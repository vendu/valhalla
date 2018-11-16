#ifndef __V0_VM_ISA_H__
#define __V0_VM_ISA_H__

#include <stdint.h>

#define V0_INSTS_MAX 256

/*
 * V0 INSTRUCTION SET ARCHITECTURE
 * -------------------------------
 */

/*
 * Addressing Modes
 * ----------------
 *
 * Notes
 * -----
 * - for indexed and PC-relative addressing, parm gives a shift count so that
 *   operation size is (8 << parm) bits. the full address could be
 *
 *   uint16_t *ptr;
 *
 *   ptr[n] = 0xffffU; // ptr is in a register, val or argument is n, so the
 *   halfword (16-bit) address becomes ADR = %reg1 + (n << 1)
 *
 * Mode         Brief                           C               Assembly
 * ----         -----                           -               --------
 * register     operand in a register           val = x;        str %r1, *%r2
 * immediate    operand follows opcode          val = CONST     $const
 * direct       address follows opcode          *ptr = x;       ldr *adr, %r1;
 * indexed      index in val or after opcode    ptr[0xff] = x;  ldr 8(%r1), %r2
 * PC-relative  index in val or after opcode    goto label;     jmp 0xff(%pc)
 *
 * Mnemonics
 * ---------
 * - I have chosen to use 3-letter mnemonics so far... instructions for the
 *   supplementary units such as floating-point would be 4 letters to emphasize
 *   the "external" processor/unit nature of them. for these instructions, I'm
 *   thinking of using the following initials in the mnemonics
 *   - P - fixed-point mathematics
 *   - F - floating-point, i.e. FADD, FMAC (multiply-and-add)
 *   - S - SIMD
 *   - M - multimedia (audio, graphics, ...)
 *   - V - vector
 *   - A - audio
 *   - D - DSP
 *
 * Instruction Prefixes
 * --------------------
 *
 * The COP-prefix marks co-processor instructions
 * The MLK-prefix may be used to lock the memory bus for a single operation of
 * - NOT, AND, XOR, OR,INC, DEC, ADD, SDC, SUB, SBC, BTR, BTS, BTC, CAS
 *
 * Fused Instructions
 * ------------------
 * - I'm thinking of doing at least fused shift-and-add as well as multiply-and-
 *   add [the last typically for floating-point]
 */
#define V0_IMM_VAL_MIN (-0x7fff - 1)
#define V0_IMM_VAL_MAX 0xffffU

/* bits for MFR feature register */

#define V0_MFR_THR (1U << 0) // multithread support
#define V0_MFR_MMU (1U << 0) // memory management/paging support
#define V0_MFR_FPU (1U << 1) // floating-point instructions present
#define V0_MFR_FPM (1U << 4) // fixed-point instructions present

#define V0_NOP     0x00

/*
 * ARITHMETIC-LOGICAL UNIT (ALU)
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
/* add/subtract instructions */
#define V0_INC 0x08 // V0_ATOM_BIT
#define V0_DEC 0x09 // V0_ATOM_BIT
#define V0_ADD 0x0a // V0_ATOM_BIT
#define V0_ADU 0x0b // NEW, V0_ATOM_BIT
#define V0_ADC 0x0c // V0_ATOM_BIT      - CF
#define V0_SUB 0x0d // V0_ATOM_BIT
#define V0_SBU 0x0e // NEW, V0_ATOM_BIT
#define V0_SBC 0x0f // V0_ATOM_BIT      - CF
/* compare (subtract and set flags) */
#define V0_CMP 0x10
/* multiplication and division operations */
#define V0_CRP 0x11
#define V0_MUL 0x12
#define V0_MLU 0x13 // NEW, V0_ATOM_BIT
#define V0_MUH 0x14
#define V0_MHU 0x15 // NEW, V0_ATOM_BIT
#define V0_DIV 0x16
#define V0_DVU 0x17

/*
 * BIT OPERATION UNIT (BOP)
 * ------------------
 *
 * Mnemo Opcode  Brief
 * ----- ------  -----
 *
 * Sign- and Zero-Extension
 * ------------------------
 * SEX	 0x18    sign-extend operand to given size
 * ZEX   0x19    zero-extend operand to given size
 *
 * Count Leading Zeroes
 * --------------------
 * CLZ	 0x1a    count leading zero bits
 *
 * Count Hamming Weight
 * --------------------
 * HAM   0x1b    compute Hamming weight/bit population (number of 1-bits)
 *
 * Swap Byte-Order
 * ---------------
 * BSW   0x1c    swap byte-order
 */
#define V0_SEX 0x18
#define V0_ZEX 0x19
#define V0_CLZ 0x1a
#define V0_HAM 0x1b
#define V0_BSW 0x1c

/*
 * THREAD AND BRANCH UNIT (FLOW)
 * ----------------------
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
 * BCB     0x28    branch if carry-bit (CF) set (carry or borrow)
 * BOF     0x29    branch if overflow-bit (OF) set
 * CSR     0x2a    call subroutine
 * BEG     0x2b    function prologue; adjust stack
 * FIN     0x2c    function epilogue; adjust stack
 * RET     0x2d    return from subroutine
 * SYS     0x2e    enter system mode
 * SRT     0x2f    return from system mode
 */
#define V0_JMP 0x20
#define V0_BEQ 0x21
#define V0_BNE 0x22
#define V0_BLT 0x23
#define V0_BUL 0x24
#define V0_BGT 0x25
#define V0_BUG 0x26
#define V0_BAF 0x27
#define V0_BCB 0x28 // NEW
#define V0_BOF 0x29 // NEW
/* standard function call support */
#define V0_CSR 0x2a
#define V0_BEG 0x2b
#define V0_FIN 0x2c
#define V0_RET 0x2d
/* system call interface */
#define V0_SYS 0x2e
#define V0_SRT 0x2f

/*
 * THREAD UNIT (THR)
 * -----------------
 *
 * Thread Launch and Termination
 * -----------------------------
 * THR     0x30    launch thread        R1 is base address, R2 return status
 * THX     0x31    exit thread          R1 is return status
 *
 * Cache Control
 * -------------
 * CPF     0x32    cache prefetch       R1 is fetch word address
 * FPG     0x33    flush page TLB-entry R1 is page-address
 * FLS     0x34    flush cacheline      R2 is cacheline address
 *
 * SYSTEM OPERATIONS
 * -----------------
 * CTR     0x35    R1 is command, R2 parameter
 */
/* V0_MULTICORE extensions */
#define V0_THR 0x30 // launch thread at *reg2, *reg1 = exit-status address
#define V0_THX 0x31 // exit thread, provide exit status to kernel
/* V0_VIRTMEM extensions */
#define V0_CPF 0x32 // prefetch [cacheline]
#define V0_FPG 0x33 // flush a page-TLB entry
#define V0_FLS 0x34 // flush [write-to-RAM] cacheline
#define V0_CTR 0x35 // system control operations
/*
 * INTERRUPT CONTROL (INTR)
 * ------------------------
 *
 * Instructions
 * ------------
 *
 * Mnemo   Opcode  Brief
 * -----   ------  -----
 * CLI     0x36    disable all interrupts, optionally store mask
 * STI     0x37    enable interrupts, optionally restore/set mask
 * INT     0x38    send/signal software interrupt  number (8-bit)
 * SLP     0x39    sleep/wait for interrupt
 * WFE     0x3a    sleep/wait for event
 * SEV     0x3b    signal event
 * IRT     0x3c    return from interrupt routine
 * HLT     0x3d    halt the processor
 * RST     0x3e    reset system
 */
#define V0_CLI 0x36 // disable interrupts
#define V0_STI 0x37 // enable interrupts
#define V0_INT 0x38 // send software interrupt
#define V0_SLP 0x39 // put thread to sleep
#define V0_WFE 0x3a // wait for event on object/cacheline
#define V0_SEV 0x3b // signal cacheline event
#define V0_IRT 0x3c
#define V0_HLT 0x3d
#define V0_RST 0x3e

/*
 * MEMORY UNIT (MMU)
 * -----------
 *
 * Mnemo Opcode  Brief
 * ----- ------  -----
 *
 * Address Calculation
 * -------------------
 * LEA   0x40    load effective address
 *
 * Load and Store
 * --------------
 * LDR   0x41    load value into register
 * STR   0x42    store register into memory
 *
 * Stack Operations
 * ----------------
 * PSH   0x43    push register          // push register to stack
 * POP   0x44    pop register           // pop register from stack
 * PSM   0x45    push many registers    // parm lists min and max registers
 * POM   0x46    pop many registers     // parm lists min and max registers
 *
 * ATOMIC OPERATIONS
 * -----------------
 *
 * [Atomic] Bit-Manipulation
 * -------------------------
 * BTS   0x47    [atomically] test bit and set to 1 if clear/0
 * BTC   0x48    [atomically] test bit and clear/zero if set/1
 * BCL   0x49    [atomically] clear bit
 * Synchronization
 * ---------------
 *
 * Memory Barriers
 * ---------------
 * BAR   0x4a    full memory barrier
 * BRD   0x4b    memory read-barrier
 * BWR   0x4c    memory write-barrier
 *
 * Load-Link Store Conditional
 * ---------------------------
 * LDL   0x4d    load linked (clear cacheline dirty-bit)
 * STC   0x4e    store conditional (if cacheline unmodified)
 *
 * Compare and Swap
 * ----------------
 * CAS   0x4f    atomic compare-and-swap (swap if current value as expected)
 */
/*load/calculate effective address */
#define V0_LEA 0x40
#define V0_LDR 0x41
#define V0_STR 0x42
/* stack operations */
#define V0_PSH 0x43 // push register reg2
#define V0_POP 0x44 // pop register reg2
#define V0_PSM 0x45 // push 1 << parm registers starting from val
#define V0_POM 0x46 // pop 1 << parm registers starting from val
/* atomic bit-manipulation */
#define V0_BTS 0x47 // NEW, V0_ATOM_BIT
#define V0_BTC 0x48 // NEW, V0_ATOM_BIT
#define V0_BCL 0x49 // NEW, V0_ATOM_BIT
// clear cacheline ACQ-bit
#define V0_BAR 0x4a // full [read-write] memory barrier
#define V0_BRD 0x4b // memory read barrier
#define V0_BWR 0x4c // memory write barrier
/* V0_MULTICORE extensions */
#define V0_LDL 0x4d // V0_ACQ_BIT
#define V0_STC 0x4e
#define V0_CAS 0x4f // [atomic] compare and swap [swap if old value as expected)

/*
 * INPUT-OUTPUT OPERATIONS
 * -----------------------
 *
 * Mnemo   Opcode  Brief
 * -----   ------  -----
 * ICD     0x50    configure I/O device
 * IRC     0X51    read I/O control register
 * IWC     0x52    write I/O control register
 * ILM     0x53    load I/O-map descriptor; map device region
 * IOC     0x54    I/O-command (sel, start, stop, xmit, recv, sync, setf, ack,
 *                 fin
 * IOP     0x55    I/O-permission control
 * BLT     0x56    block transfers using fast transfer such as DMA
 * BUS     0x57    I/O-bus management
 * COM     0x58    inter-processor and inter-thread communication
 * TAM     0x59    pseudo-transactional memory, cacheline dirty-bitmap
 * SMM     0x5a    system management mode
 *
 * Notes
 * -----
 * IRD, IWR - bits 0-1 of parm indication operation size
 *         0x00     8-bit
 *         0x01     16-bit
 *         0x02     32-bit
 *         0x03     64-bit
 */
#define V0_ICD 0x50
#define V0_IRC 0x51
#define V0_IWC 0x52
#define V0_ILM 0x53
#define V0_IOP 0x54
#define V0_IOC 0x55
#define V0_BLT 0x56
#define V0_BUS 0x57
#define V0_COM 0x58
#define V0_TAM 0x59
#define V0_SMM 0x5a

/*
 * V0 REGISTER FILE
 * ----------------
 *
 * General-Purpose Registers
 * -------------------------
 * %r0          zero register; reads always return zero, writes are NOPs
 * %r1          function return value, first caller save register
 * %r2..%7      function argument registers (the rest on stack)
 * %r7..%r15    scratch registers; callee-save
 *
 * System Registers
 * ----------------
 * %pc          program counter i.e. instruction pointer
 * %fp          frame pointer
 * %sp          stack pointer
 * %msw         machine status word (carry + overflow, interrupts, etc.)
 * %lnk         function return address back to caller
 * %tls         thread-local storage segment info
 * %ivr         interrupt vector [handler table] base address
 * %imr         interrupt mask register; 1-bit denote enabled interrupts
 * %pdr         page-directory base address for virtual memory
 * %iod         I/O-map descriptor with paramters
 * %fp0         system-mode execution stack frame-pointer
 * %sp0         system-mode stack pointer
 * %iv0         system-mode interrupt vector pointer
 * %im0         system-mode interrupt mask register
 * %ctr         control register (for system control)
 * %mfw         machine feature word, e.g. indication of optional hardware
 */
#define V0_ZERO_REG      V0_R0_REG // always returns zero, writes equal to NOP
#define V0_RET_REG       V0_R1_REG // function return value register
#define V0_REG_ARGS      7         // R1..R7, function register arguments
/* CALLER-SAVE REGISTERS */
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
#define V0_SAVE_REG_0    V0_R1_REG
#define V0_SAVE_REG_MAX  V0_R7REG
#define V0_SAVE_REGS     7    // caller saves r1..r7, callee r8..r15
/* SYSTEM REGISTERS */
#define V0_PC            0x00 // program counter
#define V0_FP            0x01 // frame pointer
#define V0_SP            0x02 // stack pointer
#define V0_MSW           0x03 // machine status word
#define V0_LNK           0x04 // link register (return address)
#define V0_TLS           0x05 // thread-local storage segment
#define V0_IVR           0x06 // interrupt vector address
#define V0_IMR           0x07 // interrupt-mask (1-bit enabled)
#define V0_PDR           0x08 // page directory address + flags
#define V0_IOD           0x09 // I/O descriptor map address + flags
#define V0_FP0           0x0a // system frame-pointer (ring 0)
#define V0_SP0           0x0b // system stack-pointer (ring 0)
#define V0_IV0           0x0c // system-mode interrupt vector
#define V0_IM0           0x0d // system-mode interrupt mask
#define V0_CTR           0x0e // control-register
#define V0_MFW           0x0f // machine feature word
#define V0_SYS_REGS      16
#define V0_PC_REG        V0_SYSREG(V0_PC)
#define V0_FP_REG        V0_SYSREG(V0_FP)
#define V0_SP_REG        V0_SYSREG(V0_SP)
#define V0_MSW_REG       V0_SYSREG(V0_MSW)
#define V0_LNK_REG       V0_SYSREG(V0_LNK)
#define V0_THR_REG       V0_SYSREG(V0_TID)
#define V0_IVR_REG       V0_SYSREG(V0_IVR)
#define V0_IMR_REG       V0_SYSREG(V0_IMR)
#define V0_PDR_REG       V0_SYSREG(V0_PDR)
#define V0_IOD_REG       V0_SYSREG(V0_IOD)
#define V0_FP0_REG       V0_SYSREG(V0_FP0)
#define V0_SP0_REG       V0_SYSREG(V0_SP0)
#define V0_IV0_REG       V0_SYSREG(V0_IV0)
#define V0_IM0_REG       V0_SYSREG(V0_IM0)
#define V0_CTR_REG       V0_SYSREG(V0_CTR)
#define V0_MFW_REG       V0_SYSREG(V0_MFW)

/* system register IDs */
#define V0_STD_REGS      (V0_INT_REGS + V0_SYS_REGS) // total number of user and system registers
// shadow registers are used for function and system calls instead of stack
#define V0_SYSREG(x)     (V0_INT_REGS + (x)) // shadow-registers
/* values for regs[V0_MSW] */
#define V0_MSW_DEF_BITS  (V0_IF_BIT)
#define V0_MSW_ZF_BIT    (1U << 0)  // zero
#define V0_MSW_OF_BIT    (1U << 1)  // overflow
#define V0_MSW_CF_BIT    (1U << 2)  // carry-flag, return bit for BTR, BTS, BTC
#define V0_MSW_IF_BIT    (1U << 3)  // interrupts enabled
#define V0_MSW_SYS_BIT   (1U << 31) // system-mode

#endif /* __V0_VM_ISA_H__ */

