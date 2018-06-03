#ifndef __V0_VM16_H__
#define __V0_VM16_H__

#include <valhalla/cdefs.h>
#include <v0/mach.h>

#define v0getcode(op)       ((op)->code)
#define v0getparm(op)       ((op)->parm)
#define v0getreg1(op)       ((op)->reg1)
#define v0getreg2(op)       ((op)->reg2)
#define v0setcode(op, c)    ((op)->code = (c))
#define v0setparm(op, p)    ((op)->parm = (p))
#define v0setreg1(op, r)    ((op)->reg1 = (r))
#define v0setreg2(op, r)    ((op)->reg2 = (r))
struct v0op16a {
    unsigned int code : 6;      // instruction operation ID
    unsigned int parm : 2;      // shift count for scaling addresses, ...
    unsigned int reg1 : 4;      // register argument #1 ID
    unsigned int reg2 : 4;      // register argument #2 ID
};

/* v0op16b inspection and manipulation macros */
#define v0getval(op)        ((op)->val)
#define v0getadr(op)        ((op)->adr)
#define v0getflg(op)        ((op)->flg)
#define v0setval(op, x)     ((op)->val = (x))
#define v0setadr(op, x)     ((op)->adr = (x))
#define v0setflg(op, x)     ((op)->flg = (x))
/* flg-member flag-bits */
#define V0OP16_PROC_BIT     (1 << 0)    // route instruction to coprocessor unit
#define V0OP16_READBAR_BIT  (1 << 1)    // read memory barrier
#define V0OP16_WRITEBAR_BIT (1 << 2)    // write memory barrier
#define V0OP16_BUSLOCK_BIT  (1 << 3)    // buslocked/atomic memory access
#define V0OP16_SYSTEM_BIT   (1 << 4)
#define V0OP16_SIGNED_BIT   (1 << 5)    // signed operation
struct v0op16b {
    unsigned int val  : 8;      // a byte for miscellaneous use
    unsigned int adr  : 2;      // addressing modes
    unsigned int flg  : 6;      // instruction flags
};

/* 16-bit argument parcel (signed or unsigned) */
struct v0oparg16 {
    union {
        int          val  : 16;
        unsigned int uval : 16;
    } data;
};

/* 32-bit argument parcel (signed or unsigned, aligned) */
struct v0opadr32 {
    union {
        int          val  : 32;
        unsigned int uval : 32;
    } data;
};

struct v0oparg64 {
    union {
        struct {
            unsigned int lo   : 32;
            unsigned int hi   : 32;
        } dual;
        union {
            int          val  : 64;
            unsigned int uval : 64;
        } u64;
    } data;
};

#endif /* __V0_VM16_H__ */

