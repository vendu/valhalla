#ifndef __V0_VM_H__
#define __V0_VM_H__

/* compile-time options */
//#define V0_DEBUG_TABS
#define V0_PRINT_XCPT
#define V0_DEBUG_TABS

/* VIRTUAL MACHINE */

#define V0_ROOT_UID 0
#define V0_ROOT_GID 0
#define V0_NO_ACL   0

#include <v0/conf.h>
#include <stdio.h>
#include <stdint.h>
#include <endian.h>
#include <valhalla/cdefs.h>
#include <v0/isa.h>
#include <v0/ins.h>

/* type is one of V0_EXEC_PERM, V0_WRITE_PERM, or V0_READ_PERM */
#define v0chkperm(cred, perm, type)                                     \
    (!(usr)                                                             \
     ? 1                                                                \
     : (((cred)->usr == (perm)->uid                                     \
         && ((perm)->flg & ((type) << 6)))                              \
        ? 1                                                             \
        : (((cred)->grp == (perm)->gid                                  \
            && ((perm)->flg & ((type) << 3)))                           \
           ? 1                                                          \
           : ((perm)->flg & (type)))))
#define v0chkmap(cred, perm)                                            \
    (!(usr)                                                             \
     ? 1                                                                \
     : (((cred)->usr == (perm)->uid || (cred)->grp == (perm)->gid)      \
        && (perm)->flg & V0_IO_MAP_BIT))
#define v0chkshm(cred, perm)                                            \
    (!(usr)                                                             \
     ? 1                                                                \
     : (((cred)->usr == (perm)->uid || (cred)->grp == (perm)->gid)      \
        && (perm)->flg & V0_IO_SHM_BIT))
#define v0chkbuf(cred, perm)                                            \
    (!(usr)                                                             \
     ? 1                                                                \
     : (((cred)->usr == (perm)->uid || (cred)->grp == (perm)->gid)      \
        && (perm)->flg & V0_IO_BUF_BIT))
#define v0chksyn(cred, perm)                                            \
    (!(usr)                                                             \
     ? 1                                                                \
     : (((cred)->usr == (perm)->uid || (cred)->grp == (perm)->gid)      \
        && (perm)->flg & V0_IO_SYN_BIT))

struct v0iofuncs {
    v0iofunc *rdfunc;
    v0iofunc *wrfunc;
};

/* permission bits for v0chkperm() type-argument */
#define V0_EXEC_PERM  0x0001
#define V0_WRITE_PERM 0x0002
#define V0_READ_PERM  0x0004
/* user-space access control bit */
#define V0_IO_USR_BIT 0x0008
/* share-permission */
#define V0_IO_SHM_BIT 0x0010
/* buffered/combined I/O operations */
#define V0_IO_BUF_BIT 0x0020
#define V0_IO_SYN_BIT 0x0040

typedef uint16_t v0ioperm;

struct v0iodesc {
    v0memadr adr;       // mapped I/O-address
    v0memadr lim;       // last mapped byte address
    v0ioperm perm;      // permission flags
    uint8_t  _pad[V0_CACHE_LINE_SIZE - 2 * sizeof(v0memadr) - sizeof(v0ioperm)];
};

/* program segments */
#define V0_TRAP_SEG      0x00
#define V0_CODE_SEG      0x01 // code
#define V0_RODATA_SEG    0x02 // read-only data such as literals
#define V0_DATA_SEG      0x03 // read-write (initialised) data
#define V0_KERN_SEG      0x04 // code to implement system [call] interface
#define V0_STACK_SEG     0x05
#define V0_SEGS          8
#if 0
/* option-bits for flg-member */
#define V0_TRACE         0x01
#define V0_PROFILE       0x02
#endif

#define v0clrmsw(vm)     ((vm)->regs[V0_MSW_REG] = 0)
#define v0setcf(vm)      ((vm)->regs[V0_MSW_REG] |= V0_MSW_CF_BIT)
#define v0setzf(vm)      ((vm)->regs[V0_MSW_REG] |= V0_MSW_ZF_BIT)
#define v0setof(vm)      ((vm)->regs[V0_MSW_REG] |= V0_MSW_OF_BIT)
#define v0setif(vm)      ((vm)->regs[V0_MSW_REG] |= V0_MSW_IF_BIT)
#define v0cfset(vm)      ((vm)->regs[V0_MSW_REG] & V0_MSW_CF_BIT)
#define v0zfset(vm)      ((vm)->regs[V0_MSW_REG] & V0_MSW_ZF_BIT)
#define v0ofset(vm)      ((vm)->regs[V0_MSW_REG] & V0_MSW_OF_BIT)
#define v0ifset(vm)      ((vm)->regs[V0_MSW_REG] & V0_MSW_IF_BIT)
#define v0setreg(vm, op, val)                                           \
    ((vm)->regs[v0insreg(op, 0)] = (val))
struct v0seg {
    v0ureg   id;
    v0memadr base;
    v0memadr lim;
    v0ureg   perm;
};

struct v0 {
    v0reg             regs[V0_INT_REGS + V0_SYS_REGS];
  //    struct v0seg      segs[V0_SEGS];
    long              flg;
    v0pagedesc       *membits;
    char             *mem;
    size_t            memsize;
    struct v0iofuncs *iovec;
    FILE             *vtdfp;
    char             *vtdpath;
    struct divuf16   *divu16tab;
};

#define v0adrtoptr(vm, adr)  ((void *)(&(vm)->mem[(adr)]))
#define v0regtoptr(vm, reg)  ((void *)(&(vm)->regs[(reg)]))

/* memory parameters */
#define V0_MEM_TRAP      0x00   // traditionally, interrupt-vector @ 0x00000000
#define V0_MEM_EXEC      0x01   // execute-permission
#define V0_MEM_WRITE     0x02   // write-permission
#define V0_MEM_READ      0x04   // read-permission
#define V0_MEM_PRESENT   0x08   // memory present in physical core
#define V0_MEM_MAP       0x10   // memory may be mapped across multiple users
#define V0_MEM_SYS       0x20   // system code
#define V0_MEM_TLS       0x40   // thread-local storage
#define V0_MEM_STACK     0x80   // segment grows downward in core

#define V0_VTD_PATH      "vtd.txt"

/* framebuffer graphics interface */
#define V0_FB_BASE       (3UL * 1024 * 1024 * 1024)      // base address

/* traps (exceptions and interrupts) - lower number means higher priority */

#define V0_TRAPS_MAX     256

/* USER [programmable] traps */
#define v0trapisuser(t)  (((t) & V0_SYS_TRAP_BIT) == 0)
#define v0trapissys(t)   ((t) & V0_SYS_TRAP_BIT)
#define V0_BREAK_POINT   0x00 // debugging breakpoint; highest priority
#define V0_TMR_INTR      0x01 // timer interrupt
#define V0_KBD_INTR      0x02 // keyboard
#define V0_PTR_INTR      0x03 // mouse, trackpad, joystick, ...
#define V0_PAGE_FAULT    0x04 // reserved for later use (paging); adr | bits
#define V0_FAST_INTR     0x1f // fast interrupts
#define V0_SYS_TRAP_BIT  0x20 // denotes system interrupts
#define V0_SYS_TRAP_MAX  0x3f // maximum system interrupt number
#define V0_TRAPS         64
#define V0_USR_TRAP_MASK 0x1f

/* SYSTEM TRAPS */
/* aborts */
#define V0_ABORT_TRAP    0x20 // traps that terminate the process
/* memory-related violations */
#define V0_STACK_FAULT   0x10 // stack segment limits exceeded; adr
#define V0_TEXT_FAULT    0x10 // invalid address for instruction; adr
#define V0_INV_MEM_READ  0x11 // memory read error; push address
#define V0_INV_MEM_WRITE 0x12 // memory write error
#define V0_INV_MEM_ADR   0x13 // invalid memory address; segment violation
/* instruction-related problems */
/* instruction format violations - terminate process */
#define V0_INV_INS_CODE  0x20 // invalid instruction; code
#define V0_INV_INS_ARG   0x21 // invalid argument; (type << 1) | num
#define V0_INV_INS_ADR   0x22 // invalid addressing-mode for instruction
/* I/O-related exceptions */
#define V0_IO_TRAP       0x20 // I/O traps
#define V0_INV_IO_READ   0x20 // no permission to read from input port; port
#define V0_INV_IO_WRITE  0x21 // no permission to write to input port; port
/* programmatic errors */
#define V0_PROG_TRAP     0x30
#define V0_DIV_BY_ZERO   0x30 // division by zero - terminate process

/* debugging */

struct v0insinfo {
    char *unit;
    char *op;
    char *func;
};

struct v0 * v0init(struct v0 *vm);
void        v0disasm(struct v0 *vm, struct v0ins *ins, v0ureg pc);

#endif /* __V0_VM_H__ */

