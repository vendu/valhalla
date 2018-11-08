#include <v0/vm/conf.h>
#include <vas/conf.h>
#include <stdlib.h>
#include <string.h>
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
            v0entry(nop);
            v0entry(not);
            v0entry(and);
            v0entry(ior);
            v0entry(xor);
            v0entry(shl);
            v0entry(shr);
            v0entry(sar);
            v0entry(inc);
            v0entry(dec);
            v0entry(add);
            v0entry(adc);
            v0entry(sub);
            v0entry(sbc);
            v0entry(cmp);
            v0entry(crp);
            v0entry(mul);
            v0entry(muh);
            v0entry(div);
            v0entry(sex);
            v0entry(lea);
            v0entry(clz);
            v0entry(ham);
            v0entry(bsw);
            v0entry(bts);
            v0entry(btc);
            v0entry(ldr);
            v0entry(str);
            v0entry(rsr);
            v0entry(wsr);
            v0entry(ldl);
            v0entry(stc);
            v0entry(jmp);
            v0entry(beq);
            v0entry(bne);
            v0entry(blt);
            v0entry(bul);
            v0entry(bgt);
            v0entry(bug);
            v0entry(baf);
            v0entry(csr);
            v0entry(beg);
            v0entry(fin);
            v0entry(ret);
            v0entry(sys);
            v0entry(srt);
            v0entry(thr);
            v0entry(thx);
            v0entry(psh);
            v0entry(psm);
            v0entry(pop);
            v0entry(pom);
            v0entry(cpf);
            v0entry(bar);
            v0entry(brd);
            v0entry(bwr);
            v0entry(wfe);
            v0entry(sev);
#if 0
            v0entry(clk);
            v0entry(clr);
#endif
            v0entry(icd);
            v0entry(imm);
            v0entry(ird);
            v0entry(iwr);
            v0entry(hlt);
            v0entry(rst);
            v0entry(cli);
            v0entry(sti);
            v0entry(int);
            v0entry(slp);
            v0entry(fpg);
            v0entry(fls);
        }
    } while (1);

#else /* !defined(__GNUC__) */

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
        //        v0initops();
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

