#ifndef __VAS_VAS_H__
#define __VAS_VAS_H__

#include <vas/conf.h>

#define VASNHASH 1024

#include <stddef.h>
#include <stdint.h>
#include <vas/types.h>

#define VASRESOLVE     (~0U)

#define VASTOKENVALUE  0x01
#define VASTOKENLABEL  0x02
#define VASTOKENINST   0x03
#define VASTOKENREG    0x04
#define VASTOKENDEF    0x07
#define VASTOKENSYM    0x08
#define VASTOKENCHAR   0x09
#define VASTOKENIMMED  0x0a
#define VASTOKENINDIR  0x0b
#define VASTOKENADR    0x0c
#define VASTOKENINDEX  0x0d
#define VASTOKENDATA   0x0e
#define VASTOKENGLOBL  0x0f
#define VASTOKENSPACE  0x10
#define VASTOKENORG    0x11
#define VASTOKENALIGN  0x12
#define VASTOKENASCIZ  0x13
#define VASTOKENSTRING 0x14
#define VASTOKENPAREN  0x15
#if (VASPREPROC)
#define VASTOKENOP     0x16
#endif
#define VASNTOKEN      0x20

struct vasop {
    const char   *name;
    uint8_t       code;
    uint8_t       narg;
    uint8_t       len;
    struct vasop *next;
};

struct vasinst {
    const char *name;
    uint8_t     code;
    uint8_t     narg;
    uint8_t     len;
};

struct vaslabel {
    char            *name;
    uintptr_t        adr;
    struct vaslabel *next;
};

struct vasvalue {
    vasword val;
    uint8_t size;
};

struct vassymrec {
    struct vassymrec *next;
    char             *name;
    uintptr_t         adr;
};

struct vasdef {
    struct vasdef *next;
    char          *name;
    vasword        val;
};

struct vassym {
    char     *name;
    vasuword  adr;
};

struct vasadr {
    char      *name;
    uintptr_t  val;
};

struct vasndx {
    vasword reg;
    vasword val;
};

struct vasval {
    char          *name;
    vasword        val;
    struct vasval *next;
};

struct vastoken {
    struct vastoken     *prev;
    struct vastoken     *next;
    unsigned long        type;
    vasword              val;
    vasword              size;
    vasword              unit;
#if (VASDB)
    uint8_t             *file;
    unsigned long        line;
#endif
    union {
        struct vaslabel  label;
        struct vasvalue  value;
        struct vasinst   inst;
        struct vasdef    def;
        struct vassym    sym;
        struct vasadr    adr;
        struct vasndx    ndx;
        char            *str;
        uint8_t          ch;
        uint8_t          size;
        vasuword         reg;
    } data;
};

#if (VASDB)
struct vasline {
    struct vasline *next;
    uintptr_t       adr;
    uint8_t        *file;
    unsigned long   num;
    uint8_t        *data;
};

struct vasline  * vasfindline(vasmemadr adr);
#endif

void              v0disasm(struct v0 *vm, struct v0op *op, v0reg pc);
uint32_t          tmhash32(unsigned long u);
extern void       vasinit(void);
#if (VASALIGN)
extern void       vasinitalign(void);
#endif
extern vasmemadr  vastranslate(vasmemadr base);
extern void       vasresolve(void);
extern void       vasfreesyms(void);
#if (VASBUF) && !(VASMMAP)
extern void       vasreadfile(char *name, vasmemadr adr, int bufid);
#else
extern void       vasreadfile(char *name, vasmemadr adr);
#endif

extern void       vasfreetoken(struct vastoken *token);
extern void       vasqueuesym(struct vassymrec *sym);

extern void       vasprinttoken(struct vastoken *token);

typedef struct vastoken * vastokfunc_t(struct vastoken *, vasmemadr,
                                       vasmemadr *);

#endif /* __VAS_VAS_H__ */

