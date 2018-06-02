#ifndef __VAS_VAS_H__
#define __VAS_VAS_H__

#include <vas/conf.h>

#define VASNHASH 1024

#include <stddef.h>
#include <stdint.h>
#include <vas/types.h>

#define VASRESOLVE     (~0U)

#define VASTOKENVALUE  1
#define VASTOKENLABEL  2
#define VASTOKENINST   3
#define VASTOKENREG    4
#define VASTOKENDEF    5
#define VASTOKENSYM    6
#define VASTOKENCHAR   7
#define VASTOKENIMMED  8
#define VASTOKENINDIR  9
#define VASTOKENADR    10
#define VASTOKENINDEX  11
#define VASTOKENDATA   12
#define VASTOKENGLOBL  13
#define VASTOKENSPACE  14
#define VASTOKENORG    15
#define VASTOKENALIGN  16
#define VASTOKENASCIZ  17
#define VASTOKENSTRING 18
#define VASTOKENPAREN  19
#if (VASPREPROC)
#define VASTOKENOP     20
#endif
#define VASNTOKEN      32

#define V0_MNEMO_LEN   8
#define V0_DEF_SHIFT   2
struct vasop {
    char          name[V0_MNEMO_LEN];
    uint8_t       code;
    uint8_t       narg;
    uint8_t       sft;
    struct vasop *next;
};

struct vasinst {
    const char *name;
    uint8_t     code;
    uint8_t     narg;
    uint8_t     sft;
};

struct vaslabel {
    char            *name;
    uintptr_t        adr;
    struct vaslabel *next;
};

struct vasvalue {
    vasword val;
    uint8_t sft;
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
    vasword              sft;
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
        uint8_t          sft;
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

struct vasline  * vasfindline(vasuword adr);
#endif

void              v0disasm(struct v0 *vm, struct v0op *op, v0reg pc);
uint32_t          tmhash32(unsigned long u);
extern void       vasinit(void);
#if (VASALIGN)
extern void       vasinitalign(void);
#endif
extern vasuword   vastranslate(vasuword base);
extern void       vasresolve(void);
extern void       vasfreesyms(void);
#if (VASBUF) && !(VASMMAP)
extern void       vasreadfile(char *name, vasuword adr, int bufid);
#else
extern void       vasreadfile(char *name, vasuword adr);
#endif

extern void       vasfreetoken(struct vastoken *token);
extern void       vasqueuesym(struct vassymrec *sym);

extern void       vasprinttoken(struct vastoken *token);

typedef struct vastoken * vastokfunc_t(struct vastoken *, vasuword,
                                       vasuword *);

#endif /* __VAS_VAS_H__ */

