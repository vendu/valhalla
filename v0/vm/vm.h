#ifndef __V0_VM_VM_H__
#define __V0_VM_VM_H__

/* VIRTUAL MACHINE */

#define V0_ROOT_UID 0
#define V0_ROOT_GID 0
#define V0_NO_ACL   0

#include <v0/vm/conf.h>
#include <stdio.h>
#include <stdint.h>
#include <endian.h>
#include <valhalla/cdefs.h>
#include <v0/vm/isa.h>
#include <v0/vm/ins.h>

struct v0;

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

typedef int64_t  v0wreg; // full-width register (temporary values)
typedef int32_t  v0reg;  // signed user-register type
typedef uint32_t v0ureg; // unsigned user-register type
typedef v0ureg   v0memadr; // memory address
typedef v0ureg   v0pagedesc;
typedef void     v0iofunc_t(struct v0 *vm, uint8_t port, v0reg reg);

struct v0iofuncs {
    v0iofunc_t *rdfunc;
    v0iofunc_t *wrfunc;
};

/* I/O credential structure */
struct v0iocred {
    uint32_t uid;       // [owner] user ID
    uint32_t gid;       // [owner] group ID
    uint32_t usr;       // object user ID
    uint32_t grp;       // object group ID
};

/* permission bits for v0chkperm() type-argument */
#define V0_EXEC_PERM  1
#define V0_WRITE_PERM 2
#define V0_READ_PERM  4

/* execute, write, and read permissions for user, group, all */
#define V0_IO_AX_BIT  (V0_EXEC_PERM << 0)
#define V0_IO_AW_BIT  (V0_WRITE_PERM << 0)
#define V0_IO_AR_BIT  (V0_READ_PERM << 0)
#define V0_IO_GX_BIT  (V0_EXEC_PERM << 3)
#define V0_IO_GW_BIT  (V0_WRITE_PERM << 3)
#define V0_IO_GR_BIT  (V0_READ_PERM << 3)
#define V0_IO_UX_BIT  (V0_EXEC_PERM << 6)
#define V0_IO_UW_BIT  (V0_WRITE_PERM << 6)
#define V0_IO_UR_BIT  (V0_READ_PERM << 6)
/* user-space access control bit */
#define V0_IO_USR_BIT 0x04000000
/* map-permission */
#define V0_IO_MAP_BIT 0x08000000
/* share-permission */
#define V0_IO_SHM_BIT 0x10000000
/* buffer-cache block */
#define V0_IO_BUF_BIT 0x20000000
/* synchronous I/O */
#define V0_IO_SYN_BIT 0x40000000
/* ACL for permissions */
#define V0_IO_ACL_BIT 0x80000000
struct v0acl {
    uint32_t uid;       // user ID
    uint32_t gid;       // group ID
    uint32_t flg;       // permission bits
    uint32_t list;      // next ^ prev control objects in list or NULL
};

struct v0iodesc {
    uint32_t        adr;        // mapped I/O-address
    uint32_t        lim;        // last mapped byte address
    uint32_t        flg;        // permission and other flag-bits
    uint32_t        acl;        // optional ACL base address or NULL
    struct v0iocred cred;       // access credentials
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
/* predefined I/O ports */
#define V0_STDIN_PORT    0 // keyboard input
#define V0_STDOUT_PORT   1 // console or framebuffer output
#define V0_STDERR_PORT   2 // console or framebuffer output
#define V0_RTC_PORT      3 // real-time clock
#define V0_TMR_PORT      4 // high-resolution timer for profiling
#define V0_MOUSE_PORT    5 // mouse input
#define V0_VTD_PORT      6 // virtual tape drive
#define V0_MAP_PORT      7 // memory-mapped device control

/* framebuffer graphics interface */
#define V0_FB_BASE       (3UL * 1024 * 1024 * 1024)      // base address

/* traps (exceptions and interrupts) - lower number means higher priority */

#define V0_NTRAP         256

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

#endif /* __V0_VM_VM_H__ */

