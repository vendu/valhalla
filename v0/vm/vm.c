#include <v0/vm/conf.h>
#include <vas/conf.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
//#include <zero/fastudiv.h>
#include <v0/vm/types.h>
#include <v0/vm/isa.h>
#include <v0/vm/vm.h>
#include <v0/vm/io.h>
#include <v0/vm/op.h>
#include <vas/vas.h>

extern void   vasinit(void);
extern long * vasgetinst(char *str);
extern long   vasaddop(const char *str, uint8_t code, uint8_t narg);

#if !defined(__GNUC__) && 0
#define _V0INSFUNC_T __inline__ uint32_t
#else
#define _V0INSFUNC_T static uint32_t
#endif

extern struct vasop     *v0optab[256];
extern char             *vaslinebuf;
extern long              vasreadbufcur;
struct v0               *v0vm;
extern vasuword          _startadr;
extern vasuword          _startset;
#if defined(V0_GAME)
static long long         v0speedcnt;
#endif
#if defined(V0_DEBUG_TABS)
static struct v0insinfo  v0insinfotab[V0_INSTS_MAX];
#endif
char                    *v0opnametab[V0_INSTS_MAX];
#if defined(__GNUC__) && 0
#define v0entry(x)                                                      \
    v0ins##x:                                                           \
    pc = v0##x(vm, pc);                                                 \
    insjmp(vm, pc)
#define _v0insadr(x) &&v0##xins
#define _V0INSTAB_T  void *
#else
#define _v0insadr(x) v0##x##ins
typedef v0reg        v0insfunc(struct v0 *vm, v0ureg pc);
#define _V0INSTAB_T  v0insfunc *
#endif
static  _V0INSTAB_T *v0jmptab[V0_INSTS_MAX];

void
v0printop(struct v0ins *ins)
{
    int val = ins->code;

    fprintf(stderr, "code\t%x - reg1 == %x, reg2 == %x\n",
            val, v0insreg(ins, 0), v0insreg(ins, 1));

    return;
}

void
v0initseg(struct v0 *vm, v0memadr base, size_t npage, v0pagedesc flg)
{
    v0pagedesc *pgtab = vm->membits;
    size_t      ndx = base / V0_PAGE_SIZE;

    /* text, rodata, data, bss, stk */
    for (ndx = 0 ; ndx < npage ; ndx++) {
        pgtab[ndx] = flg;
    }

    return;
}

void
v0initio(struct v0 *vm)
{
    struct v0iofuncs *vec = vm->iovec;
    char             *vtd = vm->vtdpath;
    FILE             *fp;

    if (!vtd) {
        vtd = strdup(V0_VTD_PATH);
        if (!vtd) {
            fprintf(stderr, "V0: failed to duplicate VTD-path\n");

            exit(1);
        }
    }
    fp = fopen(vtd, "a+");
    if (!fp) {
        perror("V0: failed to open VTD-file");

        exit(errno);
    }
    vec[V0_STDIN_PORT].rdfunc = v0readkbd;
    vec[V0_STDOUT_PORT].wrfunc = v0writetty;
    vec[V0_STDERR_PORT].wrfunc = v0writeerr;
    vec[V0_RTC_PORT].rdfunc = v0readrtc;
    vec[V0_TMR_PORT].rdfunc = v0readtmr;

    return;
}

#define V0_TRAP_PERMS  (V0_MEM_PRESENT | V0_MEM_EXEC                    \
                        | V0_MEM_MAP | V0_MEM_TRAP | V0_MEM_SYS)
#define V0_CODE_PERMS  (V0_MEM_PRESENT | V0_MEM_READ | V0_MEM_EXEC)
#define V0_DATA_PERMS  (V0_MEM_READ | V0_MEM_WRITE | V0_MEM_MAP)
#define V0_KERN_PERMS  (V0_MEM_PRESENT | V0_MEM_EXEC | V0_MEM_SYS | V0_MEM_MAP)
#define V0_TLS_PERMS   (V0_DATA_PERMS | V0_MEM_TLS)
#define V0_STACK_PERMS (V0_MEM_PRESENT | V0_MEM_READ | V0_MEM_WRITE     \
                        | V0_MEM_STACK)

struct v0 *
v0init(struct v0 *vm)
{
    void   *mem = calloc(1, V0_RAM_SIZE);
    void   *ptr;
    long    newvm = 0;
    size_t  vmnpg = V0_RAM_SIZE / V0_PAGE_SIZE;

    if (!mem) {

        return NULL;
    }
    ptr = calloc(V0_MAX_IOPORTS, sizeof(struct v0iofuncs));
    if (ptr) {
        if (!vm) {
            vm = calloc(1, sizeof(struct v0));
            if (!vm) {
                free(mem);
                free(ptr);

                return NULL;
            }
            newvm = 1;
        }
        memset(vm, 0, sizeof(struct v0));
        vm->iovec = ptr;
#if 0
        ptr = calloc(65536, sizeof(struct divuf16));
        if (!ptr) {
            free(mem);
            free(vm->iovec);
            if (newvm) {
                free(vm);
            }

            return NULL;
        }
        fastuf16divuf16gentab(ptr, 0xffff);
        vm->divu16tab = ptr;
#endif
        ptr = calloc(vmnpg, sizeof(v0pagedesc));
        if (!ptr) {
            free(mem);
            free(vm->iovec);
            //      free(vm->divu16tab);
            if (newvm) {
                free(vm);
            }
        }
        vm->mem = mem;
        vm->membits = ptr;
        v0initseg(vm, 0, 1, V0_TRAP_PERMS);
        v0initseg(vm, V0_PAGE_SIZE, vmnpg / 8 - 1, V0_CODE_PERMS);
        v0initseg(vm, vmnpg * V0_PAGE_SIZE / 4, vmnpg / 2, V0_DATA_PERMS);
        v0initseg(vm, vmnpg * V0_PAGE_SIZE * 3 / 4, vmnpg / 4 - 8, V0_KERN_PERMS);
        v0initseg(vm, vmnpg * (V0_PAGE_SIZE - 16), 8, V0_TLS_PERMS);
        v0initseg(vm, vmnpg * (V0_PAGE_SIZE - 8), 8, V0_STACK_PERMS);
        v0initio(vm);
        vm->regs[V0_FP_REG] = 0x00000000;
        vm->regs[V0_SP_REG] = V0_RAM_SIZE - 1;
        vm->memsize = V0_RAM_SIZE;
    }
    v0vm = vm;

    return vm;
}

int
v0loop(struct v0 *vm, v0ureg pc)
{
    //    static _V0INSTAB_T  jmptab[V0_MAX_INSTS];
    struct v0ins *ins = v0adrtoptr(vm, pc);

#if defined(__GNUC__) && 0
    do {
        v0reg code = ins->code;

        {
            /* ALU */
            v0nopins:
            v0notins:
            v0andins:
            v0iorins:
            v0xorins:
            v0shlins:
            v0shrins:
            v0sarins:
            v0incins:
            v0decins:
            v0addins:
            v0aduins:
            v0adcins:
            v0subins:
            v0sbuins:
            v0sbcins:
            v0cmpins:
            v0crpins:
            v0mulins:
            v0mluins:
            v0muhins:
            v0mhuins:
            v0divins:
            v0dvuins:
            /* bit operations */
            v0sexins:
            v0zexins:
            v0leains:
            v0clzins:
            v0hamins:
            v0bswins:
            v0btsins:
            v0btcins:
            v0bclins:
            /* load-store unit */
            v0ldrins:
            v0strins:
            v0barins:
            v0brdins:
            v0bwrins:
            v0pshins:
            v0psmins:
            v0popins:
            v0pomins:
            v0cpfins:
            v0fpgins:
            v0flsins:
            v0ldlins:
            v0stcins:
            /* branches and control flow */
            v0jmpins:
            v0beqins:
            v0bneins:
            v0bltins:
            v0bulins:
            v0bgtins:
            v0bugins:
            v0bafins:
            v0csrins:
            v0begins:
            v0finins:
            v0retins:
            v0sysins:
            v0srtins:
            v0thrins:
            v0thxins:
            /* interrupts and system control */
            v0hltins:
            v0rstins:
            v0cliins:
            v0stiins:
            v0intins:
            v0slpins:
            v0wfeins:
            v0sevins:
            /* I/= operations */
            v0icdins:
            v0ircins:
            v0iwcins:
            v0ilmins:
            v0iocins:

            goto *(v0jmptab[code]);
        }
    } while (1);

#else /* !defined(__GNUC__) */

    v0initins(v0jmptab);
    while (v0insisvalid(vm, ins)) {
        struct v0ins *ins = v0adrtoptr(vm, pc);
        uint8_t       code = ins->code;
        _V0INSFUNC_T *func = v0jmptab[code];

        pc = func(vm, pc);
    }

#endif

    return EXIT_SUCCESS;
}

void
v0getopt(struct v0 *vm, int argc, char *argv[])
{
    return;
}

int
main(int argc, char *argv[])
{
    struct v0 *vm = v0init(NULL);
    vasuword   adr = V0_TEXT_ADR;
    int        ret = EXIT_FAILURE;
    long       ndx;

    if (vm) {
        v0getopt(vm, argc, argv);
        v0initops(v0jmptab);
        vasinit();
        for (ndx = 1 ; ndx < argc ; ndx++) {
#if (VASBUF) && !(VASMMAP)
            vasreadfile(argv[ndx], adr, ++vasreadbufcur);
#else
            vasreadfile(argv[ndx], adr);
#endif
            adr = vastranslate(adr);
            vasresolve();
            vasfreesyms();
            if (!vm->regs[V0_PC_REG]) {
                vm->regs[V0_PC_REG] = _startadr;
            }
            if (_startset) {
                ret = v0loop(vm, _startadr);
            }
        }

        exit(ret);
    }

    exit(0);
}

