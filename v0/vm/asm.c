/* zero assembler [virtual] machine interface */

#include <vas/conf.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <stdint.h>
//#include <zero/trix.h>
#include <vas/vas.h>
#include <v0/conf.h>
#include <v0/isa.h>
#include <v0/vm.h>
#include <v0/op.h>

extern struct v0 *v0vm;
struct vasop     *v0optab[V0_HASH_SIZE];
static long       vasopbits[V0_NINST_MAX];
static long       vasinitflg;

#define vasopisnop(op)    (*((uint32_t *)(op)) == V0_NOP_CODE)
//#define vasophasval(code) (bitset(v0valbits, code))

#define vassetnop(adr)    (*((uint32_t *)(op)) = V0_NOP)
#define vassetop(op, id, n, scl)                                        \
    do {                                                                \
        (op)->code = (id);                                              \
        (op)->narg = (n);                                               \
        (op)->sft = (scl);                                              \
    } while (0)

/*
 * operation info structure addresses are stored in a multilevel table
 * - the top level table is indexed with the first byte of mnemonic and so on
 * TODO: more compact encoding of 6-bit characters + some flag bits in int32_t
 */
long
vasaddop(const char *str, uint8_t code, uint8_t narg)
{
    char         *cptr = (char *)str;
    long          key1 = 0;
    long          key2;
    struct vasop *op1;
    struct vasop *op2;
    uint8_t       len;

    fprintf(stderr, "%x (%d)\t%s\n", code, narg, str);
    op1 = calloc(1, sizeof(struct vasop));
    if (!op1) {

        return 0;
    }
    len = 0;
    while ((*cptr) && len < V0_MNEMO_LEN - 1) {
        op1->name[len] = *cptr;
        key1 += *cptr++;
        len++;
    }
    key2 = key1;
    if (key1) {
        key1 = tmhash32(key1);
        key1 &= V0_HASH_SIZE - 1;
        vassetop(op1, code, narg, V0_DEF_SHIFT);
        op1->next = v0optab[key1];
        v0optab[key1] = op1;
        op2 = calloc(1, sizeof(struct vasop));
        memcpy(op2, op1, sizeof(struct vasop));
        key1 = key2 + 'b';
        op2->name[len] = 'b';
        key1 = tmhash32(key1);
        key1 &= V0_HASH_SIZE - 1;
        vassetop(op2, code, narg, 0);
        op2->next = v0optab[key1];
        v0optab[key1] = op2;
        op2 = calloc(1, sizeof(struct vasop));
        memcpy(op2, op1, sizeof(struct vasop));
        key1 = key2 + 'w';
        op2->name[len] = 'w';
        key1 = tmhash32(key1);
        key1 &= V0_HASH_SIZE - 1;
        vassetop(op2, code, narg, 1);
        op2->next = v0optab[key1];
        v0optab[key1] = op2;
        op2 = calloc(1, sizeof(struct vasop));
        memcpy(op2, op1, sizeof(struct vasop));
        key1 = key2 + 'l';
        op2->name[len] = 'l';
        key1 = tmhash32(key1);
        key1 &= V0_HASH_SIZE - 1;
        vassetop(op2, code, narg, 2);
        op2->next = v0optab[key1];
        v0optab[key1] = op2;
        op2 = calloc(1, sizeof(struct vasop));
        memcpy(op2, op1, sizeof(struct vasop));
        key1 = key2 + 'q';
        op2->name[len] = 'q';
        key1 = tmhash32(key1);
        key1 &= V0_HASH_SIZE - 1;
        vassetop(op2, code, narg, 3);
        op2->next = v0optab[key1];
        v0optab[key1] = op2;
    }

    return key1;
}

static struct vasop *
vasfindop(char *str, vasword *retsft, char **retptr)
{
    char         *cptr = (char *)str;
    struct vasop *op = NULL;
    long          key = 0;
    vasword       sft = 0;
    size_t        len = 0;

    while ((*cptr) && !isspace(*cptr) && len < V0_MNEMO_LEN - 1) {
        key += *cptr++;
        len++;
    }
    if (key) {
        key = tmhash32(key);
        key &= V0_HASH_SIZE - 1;
        cptr[0] = '\0';
        op = v0optab[key];
        cptr++;
        while ((op) && strcmp(op->name, str)) {
            op = op->next;
        }
        if (op) {
            sft = op->sft;
        }
    }
    *retsft = sft;
    *retptr = cptr;

    return op;
}

struct vasop *
vasgetop(char *str, vasword *retsft, char **retptr)
{
    struct vasop *op = NULL;
    vasword       sft = 0;

#if (VASDEBUG)
    fprintf(stderr, "getop: %s\n", str);
#endif
    op = vasfindop(str, &sft, &str);
    if (op) {
        *retsft = sft;
        *retptr = str;
    }

    return op;
}

vasword
vasgetreg(char *str, vasword *retsft, char **retptr)
{
    vasword reg = -1;
    vasword sft = 0;

#if (VASDEBUG)
    fprintf(stderr, "getreg: %s\n", str);
#endif
    if (*str == 'r') {
        str++;
        sft = 2;
    } else if (*str == 'b') {
        str++;
        sft = 0;
    } else if (*str == 'w') {
        str++;
        sft = 1;
    } else if (*str == 'l') {
        str++;
        sft = 2;
#if (!V032BIT)
    } else if (*str == 'q') {
        str++;
        sft = 3;
#endif
    }
    if ((sft) && (*str) && isdigit(*str)) {
        reg = 0;
        while ((*str) && isdigit(*str)) {
            reg *= 10;
            reg += *str - '0';
            str++;
        }
        if ((*str) && (*str == ',' || isspace(*str))) {
            str[0] = '\0';
            str++;
            *retsft = sft;
            *retptr = str;
        } else if ((*str) && *str == ')') {
            *retsft = sft;
            *retptr = str;
        } else {
            fprintf(stderr, "invalid register name %s\n", str);

            exit(1);
        }
    } else {
        fprintf(stderr, "invalid register name %s\n", str);

        exit(1);
    }

    return reg;
}

static void
vasinitops(void)
{
    if (!vasinitflg) {
        v0initopbits(vasopbits);
        vasinitflg = 1;
    }

    return;
}

void
v0disasm(struct v0 *vm, struct v0op *op, v0ureg pc)
{
    fprintf(stderr, "%p:%p\t%s @ 0x%x\t", vm, op, v0opnametab[op->code], pc);
    fprintf(stderr, "r1 = %x, r2 = %x, a = %x, p = %x, v = %x\n",
            op->reg1, op->reg2, op->adr, op->parm, op->val);

    return;
}

struct vastoken *
vasprocinst(struct vastoken *token, v0memadr adr,
            v0memadr *retadr)
{
    struct v0op      *op = (struct v0op *)&v0vm->mem[adr];
    struct vastoken  *token1 = NULL;
    struct vastoken  *token2 = NULL;
    struct vastoken  *retval = NULL;
    struct vassymrec *sym;
    long              havearg = 0;
    vasword           val;
    vasword           flg;
    uint8_t           narg = token->data.inst.narg;
    uint8_t           sft = token->data.inst.sft;

    if (!vasinitflg) {
        vasinitops();
    }
#if (VASDB)
    vasaddline(adr, token->data.inst.data, token->file, token->line);
#endif
    op->code = token->data.inst.code;
    if (vasopisnop(op)) {
        retval = token->next;
        vassetnop(adr);
        adr += sizeof(struct v0op);
    } else if (!narg) {
        retval = token->next;
        op->code = token->data.inst.code;
        vasfreetoken(token);
    } else {
        token1 = token->next;
        vasfreetoken(token);
        if (token1) {
            switch(token1->type) {
                case VASTOKENIMMED:
                case VASTOKENVALUE:
                    val = token1->data.value.val;
                    flg = v0getopbits1(op->code, vasopbits);
                    if ((flg & V0_I_ARG)
                        && ((val >= V0_IMM_VAL_MIN
                             && val <= V0_IMM_VAL_MAX / 2)
                            || (val >= 0 && val <= V0_IMM_VAL_MAX))) {
                        op->adr = V0_IMM_ADR;
                        op->flg |= V0_IMM_BIT;
                        if (val < 0) {
                            op->flg |= V0_SIGNED_BIT;
                        }
                        op->val = val & 0xff;
                        adr += sizeof(struct v0op);
                    } else {
                        //                        sft = token->data.value.sft;
                        op->adr = V0_DIR_ADR;
                        adr += sizeof(struct v0op) + sizeof(union v0oparg);
                        op->parm = sft;
                        op->arg[0].i32 = val;
                    }

                    break;
                case VASTOKENREG:
                    op->adr = V0_REG_ADR;
                    op->reg1 = token1->data.reg;

                    break;
                case VASTOKENSYM:
                case VASTOKENADR:
                    op->adr = V0_DIR_ADR;
                    sym = malloc(sizeof(struct vassymrec));
                    sym->name = strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->arg[0].adr;
                    vasqueuesym(sym);

                    break;
                case VASTOKENINDIR:
                    token2 = token1->next;
                    if (token2->type == VASTOKENREG) {
                        op->adr = V0_NDX_ADR;
                        op->flg |= V0_IMM_BIT;
                        op->val = 0;
                        op->reg1 = token2->data.reg;
                    } else {
                        fprintf(stderr,
                                "indirect addressing requires a register\n");

                        exit(1);
                    }
                    token1 = token2;

                    break;
                case VASTOKENINDEX:
                    token2 = token1->next;
                    if (token2->type == VASTOKENREG) {
                        op->reg1 = token2->data.ndx.reg;
                        op->adr = V0_NDX_ADR;
                        op->arg[0].ofs = token1->data.ndx.val;
                    }
                    token1 = token2;

                    break;
                default:
                    fprintf(stderr,
                            "invalid argument 1 of type %lx\n", token1->type);
                    vasprinttoken(token1);

                    exit(1);

                    break;
            }
            if (op->adr != V0_REG_ADR) {
                havearg = 1;
            }
            token2 = token1->next;
            vasfreetoken(token1);
            retval = token2;
            if (narg == 2) {
                if (token2->type == VASTOKENREG) {
                    op->reg2 = token2->data.reg;
                } else if (havearg) {
                    fprintf(stderr,
                            "too many non-register arguments\n");

                    exit(1);
                } else {
                    switch(token2->type) {
                        case VASTOKENVALUE:
                        case VASTOKENIMMED:
                            val = token2->data.value.val;
                            flg = v0getopbits2(op->code, vasopbits);
                            if ((flg & V0_I_ARG)
                                && ((val >= V0_IMM_VAL_MIN
                                     && val <= V0_IMM_VAL_MAX / 2)
                                    || (val >= 0 && val <= V0_IMM_VAL_MAX))) {
                                op->adr = V0_IMM_ADR;
                                op->flg |= V0_IMM_BIT;
                                if (val < 0) {
                                    op->flg |= V0_SIGNED_BIT;
                                }
                                op->val = val & 0xff;
                            } else {
                                //                                sft = token->data.value.sft;
                                op->adr = V0_DIR_ADR;
                                op->arg[0].i32 = val;
                                op->parm = sft;

                                break;
                            }

                            break;
                        case VASTOKENSYM:
                        case VASTOKENADR:
                            op->adr = V0_DIR_ADR;
                            sym = malloc(sizeof(struct vassymrec));
                            sym->name = strdup((char *)token2->data.sym.name);
                            sym->adr = (uintptr_t)&op->arg[0].adr;
                            vasqueuesym(sym);

                            break;
                        case VASTOKENINDIR:
                            token2 = token2->next;
                            if (token2->type == VASTOKENREG) {
                                op->adr = V0_NDX_ADR;
                                op->flg |= V0_IMM_BIT;
                                op->val = 0;
                                op->reg2 = token2->data.reg;
                            } else {
                                fprintf(stderr,
                                        "indirect addressing requires a register\n");

                                exit(1);
                            }

                            break;
                        case VASTOKENINDEX:
                            token2 = token2->next;
                            if (token2->type == VASTOKENREG) {
                                op->adr = V0_NDX_ADR;
                                op->reg2 = token2->data.ndx.reg;
                                op->arg[0].ofs = token2->data.ndx.val;
                            }

                            break;
                        default:
                            fprintf(stderr,
                                    "invalid argument 2 of type %lx\n", token2->type);
                            vasprinttoken(token2);

                            exit(1);

                            break;
                    }
                }
                retval = token2->next;
                vasfreetoken(token2);
            }
        }
    }
    if (op->adr == V0_REG_ADR || (op->flg & V0_IMM_BIT)) {
        adr += sizeof(struct v0op);
    } else {
        adr += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    *retadr = adr;

    return retval;
}

