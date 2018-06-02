#include <v0/conf.h>
#include <vas/conf.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
//#include <zero/fastudiv.h>
#include <v0/types.h>
#include <v0/mach.h>
#include <v0/vm.h>
#include <v0/io.h>
#include <v0/op.h>
#include <vas/vas.h>

extern void   vasinit(void);
extern long * vasgetinst(char *str);
extern long   vasaddop(const char *str, uint8_t code, uint8_t narg);

extern struct vasop    *v0optab[256];
extern char            *vaslinebuf;
extern long             vasreadbufcur;
struct v0              *v0vm;
extern vasuword         _startadr;
extern vasuword         _startset;
#if defined(V0_GAME)
static long long        v0speedcnt;
#endif
#if defined(V0_DEBUG_TABS)
static struct v0opinfo  v0opinfotab[V0_NINST_MAX];
#endif
char                   *v0opnametab[V0_NINST_MAX];

void
v0printop(struct v0op *op)
{
    int val = op->code;

    fprintf(stderr, "code\t%x - unit == %x, inst == %x\n",
            val, v0getunit(val), v0getinst(val));

    return;
}

void
v0initseg(struct v0 *vm, v0memadr base, size_t npage, v0memflg flg)
{
    v0memflg *bits = vm->membits;
    size_t    ndx = base / V0_PAGE_SIZE;

    /* text, rodata, data, bss, stk */
    for (ndx = 0 ; ndx < npage ; ndx++) {
        bits[ndx] = flg;
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
    vec[V0_STDERR_PORT].wrfunc = v0writetty;
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
    void   *mem = calloc(1, V0_MEM_SIZE);
    void   *ptr;
    long    newvm = 0;
    size_t  vmnpg = V0_MEM_SIZE / V0_PAGE_SIZE;

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
        ptr = calloc(vmnpg, sizeof(v0memflg));
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
        v0initseg(vm, vmnpg * PAGESIZE / 4, vmnpg / 2, V0_DATA_PERMS);
        v0initseg(vm, vmnpg * PAGESIZE * 3 / 4, vmnpg / 4 - 8, V0_KERN_PERMS);
        v0initseg(vm, vmnpg * (PAGESIZE - 16), 8, V0_TLS_PERMS);
        v0initseg(vm, vmnpg * (PAGESIZE - 8), 8, V0_STACK_PERMS);
        v0initio(vm);
        vm->regs[V0_FP_REG] = 0x00000000;
        vm->regs[V0_SP_REG] = V0_MEM_SIZE - 1;
    }
    v0vm = vm;

    return vm;
}

int
v0loop(struct v0 *vm, v0ureg pc)
{
    static _V0OPTAB_T  jmptab[V0_MAX_INSTS];
    struct v0op       *op = v0adrtoptr(vm, pc);

    v0initops(jmptab);

#if defined(__GNUC__)
    do {
        v0reg code = op->code;

        goto *jmptab[code];

        {
            v0opnop:
                pc = v0nop(vm, pc);

                opjmp(vm, pc);
            v0opinc:
                pc = v0inc(vm, pc);

                opjmp(vm, pc);
            v0opdec:
                pc = v0dec(vm, pc);

                opjmp(vm, pc);
            v0opcmp:
                pc = v0cmp(vm, pc);

                opjmp(vm, pc);
            v0opadd:
                pc = v0add(vm, pc);

                opjmp(vm, pc);
            v0opadc:
                pc = v0adc(vm, pc);

                opjmp(vm, pc);
            v0opsub:
                pc = v0sub(vm, pc);

                opjmp(vm, pc);
            v0opsbc:
                pc = v0sbc(vm, pc);

                opjmp(vm, pc);
            v0opshl:
                pc = v0shl(vm, pc);

                opjmp(vm, pc);
            v0opshr:
                pc = v0shr(vm, pc);

                opjmp(vm, pc);
            v0opsar:
                pc = v0sar(vm, pc);

                opjmp(vm, pc);
            v0opnot:
                pc = v0not(vm, pc);

                opjmp(vm, pc);
            v0opand:
                pc = v0and(vm, pc);

                opjmp(vm, pc);
            v0opxor:
                pc = v0xor(vm, pc);

                opjmp(vm, pc);
            v0opior:
                pc = v0ior(vm, pc);

                opjmp(vm, pc);
            v0opcrp:
                pc = v0crp(vm, pc);

                opjmp(vm, pc);
            v0opmul:
                pc = v0mul(vm, pc);

                opjmp(vm, pc);
            v0opmuh:
                pc = v0muh(vm, pc);

                opjmp(vm, pc);
            v0opldr:
                pc = v0ldr(vm, pc);

                opjmp(vm, pc);
            v0opstr:
                pc = v0str(vm, pc);

                opjmp(vm, pc);
            v0oppsh:
                pc = v0psh(vm, pc);

                opjmp(vm, pc);
            v0oppsm:
                pc = v0psm(vm, pc);

                opjmp(vm, pc);
            v0oppop:
                pc = v0pop(vm, pc);

                opjmp(vm, pc);
            v0oppom:
                pc = v0pom(vm, pc);

                opjmp(vm, pc);
            v0opjmp:
                pc = v0jmp(vm, pc);

                opjmp(vm, pc);
            v0opjmr:
                pc = v0jmr(vm, pc);

                opjmp(vm, pc);
            v0opbiz:
                pc = v0biz(vm, pc);

                opjmp(vm, pc);
            v0opbeq:
                pc = v0biz(vm, pc);

                opjmp(vm, pc);
            v0opbnz:
                pc = v0bnz(vm, pc);

                opjmp(vm, pc);
            v0opbne:
                pc = v0bnz(vm, pc);

                opjmp(vm, pc);
            v0opblt:
                pc = v0blt(vm, pc);

                opjmp(vm, pc);
            v0opble:
                pc = v0ble(vm, pc);

                opjmp(vm, pc);
            v0opbgt:
                pc = v0bgt(vm, pc);

                opjmp(vm, pc);
            v0opbge:
                pc = v0bge(vm, pc);

                opjmp(vm, pc);
            v0opbio:
                pc = v0bio(vm, pc);

                opjmp(vm, pc);
            v0opbno:
                pc = v0bno(vm, pc);

                opjmp(vm, pc);
            v0opbic:
                pc = v0bic(vm, pc);

                opjmp(vm, pc);
            v0opbnc:
                pc = v0bnc(vm, pc);

                opjmp(vm, pc);
            v0opcsr:
                pc = v0csr(vm, pc);

                opjmp(vm, pc);
            v0opbeg:
                pc = v0beg(vm, pc);

                opjmp(vm, pc);
            v0opfin:
                pc = v0fin(vm, pc);

                opjmp(vm, pc);
            v0opret:
                pc = v0ret(vm, pc);

                opjmp(vm, pc);
            v0opird:
                pc = v0ird(vm, pc);

                opjmp(vm, pc);
            v0opiwr:
                pc = v0iwr(vm, pc);

                opjmp(vm, pc);
            v0opicf:
                pc = v0icf(vm, pc);

                opjmp(vm, pc);
            v0opsti:
                pc = v0sti(vm, pc);

                opjmp(vm, pc);
            v0opcli:
                pc = v0cli(vm, pc);

                opjmp(vm, pc);
            v0opslp:
                pc = v0slp(vm, pc);

                opjmp(vm, pc);
            v0oprst:
                pc = v0rst(vm, pc);

                opjmp(vm, pc);
            v0ophlt:
                pc = v0hlt(vm, pc);

                opjmp(vm, pc);
        }
    } while (1);

#else n/* !defined(__GNUC__) */

    while (v0opisvalid(vm, op)) {
        struct v0op *op = v0adrtoptr(vm, pc);
        uint8_t      code = op->code;
        v0opfunc    *func = jmptab[code];

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
        v0addops();
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

