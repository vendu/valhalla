/* valhalla assembler main file */

#include <vas/conf.h>

#ifndef VASDEBUG
#define VASDEBUG 0
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
//#include <zero/cdefs.h>
//#include <zero/param.h>
//#include <zero/trix.h>
#include <valhalla/param.h>
#include <vas/vas.h>
#include <vas/opt.h>
#include <vas/io.h>
#if (V0)
#include <v0/vm/conf.h>
#include <v0/vm/vm.h>
#endif

extern struct v0       *v0vm;

/* vasgetop() is machine-specific */
struct vasop           * vasgetop(char *str, vasword *retsft, char **retptr);
extern vasword           vasgetreg(char *str, vasword *retsft,
                                   char **retptr);
extern struct vasop    * vasfindop(const char *str);
static char            * vasgetlabel(char *str, char **retptr);
static char            * vasgetsym(char *str, char **retptr);
static long              vasgetvalue(char *str, vasword *retval,
                                     char **retptr);
static char *            vasgetdef(char *srt, char **retptr);
static int               vasgetchar(char *str, char **retptr);
#if (VASMMAP)
static void              vasgettoken(char *str, char **retptr,
                                     struct vasmap *map);
#else
static void              vasgettoken(char *str, char **retptr);
#endif

static struct vastoken * vasprocvalue(struct vastoken *, vasuword, vasuword *);
static struct vastoken * vasproclabel(struct vastoken *, vasuword, vasuword *);
/* vasprocinst() is machine-specific */
extern struct vastoken * vasprocinst(struct vastoken *, vasuword, vasuword *);
static struct vastoken * vasprocchar(struct vastoken *, vasuword, vasuword *);
static struct vastoken * vasprocdata(struct vastoken *, vasuword, vasuword *);
static struct vastoken * vasprocglobl(struct vastoken *, vasuword, vasuword *);
static struct vastoken * vasprocspace(struct vastoken *, vasuword, vasuword *);
static struct vastoken * vasprocorg(struct vastoken *, vasuword, vasuword *);
static struct vastoken * vasprocalign(struct vastoken *, vasuword, vasuword *);
static struct vastoken * vasprocasciz(struct vastoken *, vasuword, vasuword *);

static struct vassymrec *vassymhash[VASNHASH] ALIGNED(PAGESIZE);
static struct vasdef    *vasdefhash[VASNHASH];
static struct vasval    *vasvalhash[VASNHASH];
static struct vaslabel  *vasglobhash[VASNHASH];
#if (VASDB)
struct vasline          *vaslinehash[VASNHASH];
#endif

static vastokfunc_t *vastokfunctab[VASNTOKEN]
= {
    NULL,               // uninitialized (zero)
    vasprocvalue,
    vasproclabel,
    vasprocinst,
    NULL,               // VASTOKENREG
    NULL,               // VASTOKENDEF
    NULL,               // VASTOKENSYM
    vasprocchar,
    NULL,               // VASTOKENIMMED
    NULL,               // VASTOKENINDIR
    NULL,               // VASTOKENADR
    NULL,               // VASTOKENINDEX
    vasprocdata,
    vasprocglobl,
    vasprocspace,
    vasprocorg,
    vasprocalign,
    vasprocasciz,
    NULL,
    NULL
};
const char              *vastoknametab[VASNTOKEN]
= {
    "ZERO",             // uninitialized (zero)
    "value",
    "label",
    "inst",
    "reg",              // VASTOKENREG
    "def",              // VASTOKENDEF
    "sym",              // VASTOKENSYM
    "char",
    "immed",            // VASTOKENIMMED
    "indir",            // VASTOKENINDIR
    "adr",              // VASTOKENADR
    "index",            // VASTOKENINDEX
    "data",
    "globl",
    "space",
    "org",
    "align",
    "asciz",
    "string",
    "paren"
};
#if (VASALIGN)
vasuword                 vastokalntab[VASNTOKEN];
#endif

struct vastoken         *vastokenqueue;
struct vastoken         *vastokentail;
static struct vassymrec *symqueue;
vasuword                 _startadr;
vasuword                 _startset;
unsigned long            vasinputread;
char                    *vaslinebuf;
char                    *vasstrbuf;
long                     vasnreadbuf = VAS_READ_BUFS;
long                     vasreadbufcur;

void
vasprinttoken(struct vastoken *token)
{
    switch (token->type) {
        case VASTOKENVALUE:
            fprintf(stderr, "value 0x%08lx (sft == %ld)\n",
                    (unsigned long) token->data.value.val, (unsigned long)token->data.value.sft);

            break;
        case VASTOKENLABEL:
            fprintf(stderr, "label %s (adr == 0x%08lx)\n",
                    token->data.label.name,
                    (unsigned long)token->data.label.adr);

            break;
        case VASTOKENINST:
            fprintf(stderr, "instruction %s (code == 0x%02x)\n",
                    token->data.inst.name, token->data.inst.code);

            break;
        case VASTOKENREG:
            fprintf(stderr, "register r%lx\n", (long)token->data.reg);

            break;
        case VASTOKENSYM:
            fprintf(stderr, "symbol %s (adr == 0x%08lx)\n",
                    token->data.sym.name, (long)token->data.sym.adr);

            break;
        case VASTOKENCHAR:
            fprintf(stderr, "character 0x%02x\n", token->data.ch);

            break;
        case VASTOKENIMMED:
            fprintf(stderr, "immediate (val == 0x%08lx)\n", (long)token->data.value.val);

            break;
        case VASTOKENINDIR:
            fprintf(stderr, "indirection\n");

            break;
        case VASTOKENADR:
            fprintf(stderr, "address (sym == %s, adr == 0x%08lx)\n",
                    token->data.adr.name, (long)token->data.adr.val);

            break;
        case VASTOKENINDEX:
            fprintf(stderr, "index %ld(%%r%ld)\n", (long)token->data.ndx.val,
                    (long)token->data.ndx.reg);

            break;
    }

    return;
}

void
vasfreetoken(struct vastoken *token)
{
#if (VASDB)
    free(token->file);
#endif
    free(token);

    return;
}

static void
vasqueuetoken(struct vastoken *token)
{
    if (!token->type) {
        vasprinttoken(token);
    }
    token->prev = NULL;
    token->next = NULL;
    if (!vastokenqueue) {
        vastokenqueue = token;
        vastokentail = token;
    } else {
        token->prev = vastokentail;
        vastokentail->next = token;
        vastokentail = token;
        vasprinttoken(token->prev);
    }

    return;
}

#if (VASDB)

void
vasaddline(vasuword adr, uint8_t *data, uint8_t *filename, unsigned long line)
{
    struct vasline *newline = malloc(sizeof(struct vasline));
    unsigned long   key;

    key = (adr & 0xff) + ((adr >> 8) & 0xff) + ((adr >> 16) & 0xff) + ((adr >> 24) & 0xff);
    newline->adr = adr;
    newline->file = strdup(filename);
    newline->num = line;
    newline->data = data;
    newline->next = vaslinehash[key];
    key = tmhash32(key);
    key &= (VASNHASH - 1);
    vaslinehash[key] = newline;

    return;
}

struct vasline *
vasfindline(vasuword adr)
{
    struct vasline *line;
    unsigned long   key;

    key = (adr & 0xff) + ((adr >> 8) & 0xff) + ((adr >> 16) & 0xff) + ((adr >> 24) & 0xff);
    key = tmhash32(key);
    key &= (VASNHASH - 1);
    line = vaslinehash[key];
    while ((line) && line->adr != adr) {
        line = line->next;
    }

    return line;
}

#endif

void
vasadddef(struct vasdef *def)
{
    unsigned long   key = 0;
    char           *ptr;

    ptr = def->name;
    while (*ptr) {
        key += *ptr++;
    }
    key = tmhash32(key);
    key &= (VASNHASH - 1);
    def->next = vasdefhash[key];
    vasdefhash[key] = def;

    return;
}

struct vasdef *
vasfinddef(char *str, vasword *defptr, char **retptr)
{
    unsigned long  key = 0;
    struct vasdef *def = NULL;
    char          *ptr;
    long           len = 0;

    if ((*str) && (isalpha(*str) || *str == '_')) {
        ptr = str;
        key += *str;
        str++;
        len++;
        while ((*str) && (isalnum(*str) || *str == '_')) {
            key += *str;
            str++;
            len++;
        }
        key = tmhash32(key);
        key &= (VASNHASH - 1);
        def = vasdefhash[key];
        while ((def) && strncmp(def->name, ptr, len)) {
            def = def->next;
        }
        if (def) {
            *defptr = def->val;
            *retptr = str;
        }
    }

    return def;
}

void
vasaddval(struct vasval *val)
{
    unsigned long   key = 0;
    char           *ptr;

    ptr = val->name;
    while (*ptr) {
        key += *ptr++;
    }
    key = tmhash32(key);
    key &= (VASNHASH - 1);
    val->next = vasvalhash[key];
    vasvalhash[key] = val;

    return;
}

struct vasval *
vasfindval(char *str, vasword *valptr, char **retptr)
{
    unsigned long  key = 0;
    struct vasval *val = NULL;
    char          *ptr;
    long           len = 0;

    if ((*str) && (isalpha(*str) || *str == '_')) {
        ptr = str;
        key += *str;
        str++;
        len++;
        while ((*str) && (isalnum(*str) || *str == '_')) {
            key += *str;
            str++;
            len++;
        }
        key = tmhash32(key);
        key &= (VASNHASH - 1);
        val = vasvalhash[key];
        while ((val) && strncmp(val->name, ptr, len)) {
            val = val->next;
        }
        if (val) {
            *valptr = val->val;
            *retptr = str;
        }
    }

    return val;
}

void
vasqueuesym(struct vassymrec *sym)
{
    sym->next = NULL;
    if (!symqueue) {
        symqueue = sym;
    } else {
        sym->next = symqueue;
        symqueue = sym;
    }

    return;
}

static void
vasaddsym(struct vassymrec *sym)
{
    char          *str = sym->name;
    unsigned long  key = 0;

    while (*str) {
        key += *str++;
    }
    key = tmhash32(key);
    key &= (VASNHASH - 1);
    sym->next = vassymhash[key];
    vassymhash[key] = sym;

    return;
}

struct vassymrec *
vasfindsym(char *name)
{
    struct vassymrec *sym = NULL;
    char             *str = name;
    unsigned long     key = 0;

    while (*str) {
        key += *str++;
    }
    key = tmhash32(key);
    key &= (VASNHASH - 1);
    sym = vassymhash[key];
    while ((sym) && strcmp(sym->name, name)) {
        sym = sym->next;
    }

    return sym;
}

void
vasfreesyms(void)
{
    struct vassymrec *sym1;
    struct vassymrec *sym2;
    long              l;

    for (l = 0 ; l < VASNHASH ; l++) {
        sym1 = vassymhash[l];
        while (sym1) {
            sym2 = sym1;
            sym1 = sym1->next;
            free(sym2);
        }
        vassymhash[l] = NULL;
    }

    return;
}

static void
vasaddglob(struct vaslabel *label)
{
    char          *str = label->name;
    unsigned long  key = 0;

    while (*str) {
        key += *str++;
    }
    key = tmhash32(key);
    key &= (VASNHASH - 1);
    label->next = vasglobhash[key];
    vasglobhash[key] = label;

    return;
}

struct vaslabel *
vasfindglob(char *name)
{
    struct vaslabel *label = NULL;
    char            *str = name;
    unsigned long    key = 0;

    while (*str) {
        key += *str++;
    }
    key = tmhash32(key);
    key &= (VASNHASH - 1);
    label = vasglobhash[key];
    while ((label) && strcmp(label->name, name)) {
        label = label->next;
    }

    return label;
}

static char *
vasgetlabel(char *str, char **retptr)
{
    char *ptr = str;
    char *name = NULL;

#if (VASDEBUG)
    fprintf(stderr, "getlabel: %s\n", str);
#endif
    if ((*str) && (isalpha(*str) || *str == '_')) {
        str++;
        while ((*str) && (isalnum(*str) || *str == '_')) {
            str++;
        }
    }
    if (*str == ':') {
        *str++ = '\0';
        name = strdup(ptr);
        *retptr = str;
    }

    return name;
}

static char *
vasgetdef(char *str, char **retptr)
{
    char *ptr = str;
    char *name = NULL;

#if (VASDEBUG)
    fprintf(stderr, "getdef: %s\n", str);
#endif
    if (isalpha(*str) || *str == '_') {
        str++;
    }
    while (isalnum(*str) || *str == '_') {
        str++;
    }
    while ((*str) && (isspace(*str))) {
        str++;
    }
    if (*str == '=') {
        *str++ = '\0';
    }
    name = strdup(ptr);
    *retptr = str;

    return name;
}

static char *
vasgetsym(char *str, char **retptr)
{
    char *ptr = str;
    char *name = NULL;

#if (VASDEBUG)
    fprintf(stderr, "getsym: %s\n", str);
#endif
    if (isalpha(*str) || *str == '_') {
        str++;
        while (isalnum(*str) || *str == '_') {
            str++;
        }
        while ((*str) && (isspace(*str))) {
            str++;
        }
        if (*str == ',') {
            *str++ = '\0';
        }
        name = strdup(ptr);
    }
    *retptr = str;

    return name;
}

static long
vasgetvalue(char *str, vasword *valret, char **retstr)
{
    long                found = 0;
    vasuword          uval = 0;
    vasword           val = 0;
    long                neg = 0;

#if (VASDEBUG)
    fprintf(stderr, "getvalue: %s\n", str);
#endif
    if (*str == '-') {
        neg = 1;
        str++;
    }
    if (str[0] == '0' && tolower(str[1]) == 'x') {
        str += 2;
        while ((*str) && isxdigit(*str)) {
            uval <<= 4;
            uval += (isdigit(*str)
                     ? *str - '0'
                     : (islower(*str)
                        ? *str - 'a' + 10
                        : *str - 'A' + 10));
            str++;
        }
        found = 1;
    } else if (str[0] == '0' && tolower(str[1]) == 'b') {
        str += 2;
        while ((*str) && (*str == '0' || *str == '1')) {
            uval <<= 1;
            uval += *str - '0';
            str++;
        }
        found = 1;
    } else if (*str == '0') {
        str++;
        while ((*str) && isdigit(*str)) {
            if (*str > '7') {
                fprintf(stderr, "invalid number in octal constant: %s\n", str);

                exit(1);
            }
            uval <<= 3;
            uval += *str - '0';
            str++;
        }
        found = 1;
    } else if (isdigit(*str)) {
        while ((*str) && isdigit(*str)) {
            uval *= 10;
            uval += *str - '0';
            str++;
        }
        found = 1;
    }
    if (found) {
        *retstr = str;
        if (neg) {
            val = -((vasword)uval);
            *valret = val;
        } else {
            *valret = (vasword)uval;
        }
    }

    return found;
}

static char *
vasgetadr(char *str, char **retptr)
{
    char *ptr = str;
    char *name = NULL;

#if (VASDEBUG)
    fprintf(stderr, "getadr: %s\n", str);
#endif
    while (isalpha(*str) || *str == '_') {
        str++;
    }
    if (*str == ',') {
        *str++ = '\0';
    }
    name = strdup(ptr);
    *retptr = str;

    return name;
}

static vasuword
vasgetindex(char *str, vasword *retndx, char **retptr)
{
    vasword  reg = -1;
    vasword  val = 0;
    vasword  ndx = 0;
    vasword  sft = 0;
    long     neg = 0;

#if (VASDEBUG)
    fprintf(stderr, "getindex: %s\n", str);
#endif
    if (*str == '-') {
        neg = 1;
        str++;
    }
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        str += 2;
        while ((*str) && isxdigit(*str)) {
            val <<= 4;
            val += (isdigit(*str)
                    ? *str - '0'
                    : tolower(*str) - 'a' + 10);
            str++;
        }
    } else if (str[0] == '0' && (str[1] == 'b' || str[1] == 'B')) {
        str += 2;
        while ((*str) && (*str == '0' || *str == '1')) {
            val <<= 1;
            val += *str - '0';
            str++;
        }
    } else if (*str == '0') {
        str++;
        while ((*str) && isdigit(*str)) {
            if (*str > '7') {
                fprintf(stderr, "invalid number in octal constant: %s\n", str);

                exit(1);
            }
            val <<= 3;
            val += *str - '0';
            str++;
        }
    } else if (isdigit(*str)) {
        while ((*str) && isdigit(*str)) {
            val *= 10;
            val += *str - '0';
            str++;
        }
    }
    if (*str == '(') {
        ndx = 1;
        str++;
    }
    if (ndx) {
        if (*str == '%' && str[1]) {
            str++;
        }
        reg = vasgetreg(str, &sft, &str);
#if 0
        if (reg >= VASNREG) {
            fprintf(stderr, "invalid register name %s\n", str);

            exit(1);
        }
#endif
        if (*str == ')') {
            *retptr = str;
        }
    }
    if (sft) {
        val &= (CHAR_BIT << sft) - 1;;
    }
    if (neg) {
        val = -val;
    }
    *retndx = val;

    return reg;
}

static int
vasgetchar(char *str, char **retptr)
{
    char      *name = str;
    vasword  val = 0;

#if (VASDEBUG)
    fprintf(stderr,"getchar: %s\n", str);
#endif
    str++;
    if (*str == '\\') {
        str++;
        if (isalpha(*str)) {
            switch (*str) {
                case 'n':
                    val = '\n';
                    str++;

                    break;
                case 't':
                    val = '\t';
                    str++;

                    break;
                case 'r':
                    val = '\r';
                    str++;

                    break;
                default:
                    fprintf(stderr, "invalid character literal: %s\n", name);
            }
        } else {
            if (*str == '0') {
                str++;
            }
            while ((*str) && isdigit(*str)) {
                if (*str > '7') {
                    fprintf(stderr, "invalid number in octal constant: %s\n",
                            name);

                    exit(1);
                }
                val <<= 3;
                val += *str - '0';
                str++;
            }
        }
    } else if (isalpha(*str) || isspace(*str)) {
        val = *str;
        str++;
    } else {
        while ((*str) && isdigit(*str)) {
            val *= 10;
            val += *str - '0';
            str++;
        }
    }
    if (val > 0xff) {
        fprintf(stderr, "oversize character literal %s (%lx)\n",
                name, (long)val);

        exit(1);
    }
    if (*str == '\'') {
        str++;
    }
    if (*str == ',') {
        str++;
    }
    *retptr = str;

    return (unsigned char)val;
}

#if (VASMMAP)
static void
vasgettoken(char *str, char **retptr, struct vasmap *map)
#else
static void
vasgettoken(char *str, char **retptr)
#endif
{
    long             buflen = VAS_LINE_BUFSIZE;
    long             len;
    char            *buf = vasstrbuf;
    struct vastoken *token1 = malloc(sizeof(struct vastoken));
    struct vastoken *token2 = NULL;
    struct vasop    *op = NULL;
    char            *name = str;
    vasword          val = VASRESOLVE;
    vasword          ndx;
    vasword          sft = 0;
    int              ch;
    char            *ptr;

    while (*str && isspace(*str)) {
        str++;
    }
    if (*str == ',') {
        str++;
    }
    while (*str && isspace(*str)) {
        str++;
    }
#if (VASDEBUG)
    fprintf(stderr, "gettoken: %s\n", str);
#endif
    if ((*str) && (isdigit(*str) || *str == '-')) {
        val = vasgetindex(str, &ndx, &str);
        if (val >= 0) {
            token2 = malloc(sizeof(struct vastoken));
            token1->type = VASTOKENINDEX;
            token2->type = VASTOKENREG;
            token2->data.ndx.reg = val;
            token2->data.ndx.val = ndx;
        } else if (vasgetvalue(str, &val, &str)) {
            token1->type = VASTOKENVALUE;
            token1->data.value.val = val;
        } else {
            fprintf(stderr, "invalid token %s\n", vaslinebuf);

            exit(1);
        }
        vasqueuetoken(token1);
        if (token2) {
            vasqueuetoken(token2);
        }
    } else if ((*str) && *str == '"') {
        str++;
        len = 0;
        while (*str) {
            *buf++ = *str++;
            len++;
            if (len == buflen) {
                buflen <<= 1;
                ptr = vaslinebuf;
                vaslinebuf = realloc(ptr, buflen);
                if (!vaslinebuf) {
                    fprintf(stderr, "overlong line (%ld == %ld)\n",
                            len, buflen);

                    exit(1);
                }
                if (vaslinebuf != ptr) {
                    buf = &vaslinebuf[len];
                    str = buf;
                }
            }
        }
        while (*str != '"') {
            ch = *str++;
            if (ch == '\\') {
                switch (*str) {
                    case 'n':
                        *buf++ = (uint8_t)'\n';
                        str++;

                        break;
                    case 'r':
                        *buf++ = (uint8_t)'\r';
                        str++;

                        break;
                    case 't':
                        *buf++ = (uint8_t)'\t';
                        str++;

                        break;
                }
            } else {
                *buf++ = (uint8_t)ch;
            }
            len--;
        }
        if (*str == '"') {
            *buf++ = '\0';
            str++;
            token1->type = VASTOKENSTRING;
            token1->data.str = strdup(vasstrbuf);
        }
        vasqueuetoken(token1);
    } else if ((*str) && *str == '%') {
        str++;
        val = vasgetreg(str, &sft, &str);
        token1->type = VASTOKENREG;
        token1->sft = sft;
        token1->data.reg = val;
        vasqueuetoken(token1);
    } else if ((*str) && (isalpha(*str) || *str == '_')) {
#if (VASDB)
        ptr = str;
#endif
        name = vasgetlabel(str, &str);
        if (name) {
            token1->type = VASTOKENLABEL;
            token1->data.label.name = name;
            token1->data.label.adr = VASRESOLVE;
        } else {
            op = vasgetop(str, &sft, &str);
            if (op) {
                token1->type = VASTOKENINST;
                token1->data.inst.name = op->name;
                token1->data.inst.code = op->code;
                token1->data.inst.narg = op->narg;
                token1->data.inst.sft = sft;
#if (VASDB)
                token1->data.inst.data = strdup(ptr);
#endif
            } else {
                name = vasgetsym(str, &str);
                if (name) {
                    token1->type = VASTOKENSYM;
                    token1->data.sym.name = name;
                    token1->data.sym.adr = VASRESOLVE;
                }
                if (!name) {
                    name = vasgetadr(str, &str);
                    if (name) {
                        token1->type = VASTOKENSYM;
                        token1->data.sym.name = name;
                        token1->data.sym.adr = VASRESOLVE;
                    } else {
                        fprintf(stderr, "invalid token %s\n", vaslinebuf);

                        exit(1);
                    }
                }
            }
        }
        vasqueuetoken(token1);
    } else if ((*str) && *str == '$') {
        str++;
        if (isalpha(*str) || *str == '_') {
            if (vasfinddef(str, &val, &str)) {
                token1->type = VASTOKENDEF;
                token1->data.value.val = val;
            } else if ((*str) && (isalpha(*str)
                                  || *str == '_' || *str == '-')) {
                if (vasfindval(str, &val, &str)) {
                    token1->type = VASTOKENIMMED;
                    token1->data.value.val = val;
                } else if (vasgetvalue(str, &val, &str)) {
                    token1->type = VASTOKENIMMED;
                    token1->data.value.val = val;
                } else {
                    name = vasgetsym(str, &str);
                    if (name) {
                        token1->type = VASTOKENADR;
                        token1->data.adr.name = name;
                        token1->data.adr.val = VASRESOLVE;
                    } else {
                        fprintf(stderr, "invalid token %s\n", vaslinebuf);

                        exit(1);
                    }
                }
            }
            vasqueuetoken(token1);
        } else if ((*str) && isdigit(*str)) {
            if (vasgetvalue(str, &val, &str)) {
                token1->type = VASTOKENIMMED;
                token1->data.value.val = val;
                while (isspace(*str)) {
                    str++;
                }
                if (*str == ',') {
                    str++;
                }
            } else {
                fprintf(stderr, "invalid immediate %s\n", str);

                exit(1);
            }
            vasqueuetoken(token1);
        }
    } else if (*str == '\'') {
        val = vasgetchar(str, &str);
        token1->type = VASTOKENCHAR;
        token1->data.ch = val;
    } else if (*str == '.') {
        str++;
        sft = 0;
        if (!strncmp(str, "quad", 4)) {
            str += 4;
            token1->type = VASTOKENDATA;
            sft = token1->data.sft = 3;
        } else if (!strncmp(str, "long", 4)) {
            str += 4;
            token1->type = VASTOKENDATA;
            sft = token1->data.sft = 2;
        } else if (!strncmp(str, "byte", 4)) {
            str += 4;
            token1->type = VASTOKENDATA;
            sft = token1->data.sft = 0;
        } else if (!strncmp(str, "short", 5)) {
            str += 5;
            token1->type = VASTOKENDATA;
            sft = token1->data.sft = 1;
        } else if (!strncmp(str, "globl", 5)) {
            str += 5;
            token1->type = VASTOKENGLOBL;
        } else if (!strncmp(str, "space", 5)) {
            str += 5;
            token1->type = VASTOKENSPACE;
        } else if (!strncmp(str, "org", 3)) {
            str += 3;
            token1->type = VASTOKENORG;
        } else if (!strncmp(str, "align", 5)) {
            str += 5;
            token1->type = VASTOKENALIGN;
        } else if (!strncmp(str, "asciz", 5)) {
            str += 5;
            token1->type = VASTOKENASCIZ;
        }
        vasqueuetoken(token1);
    } else if (*str == '*') {
        str++;
        token1->type = VASTOKENINDIR;
        struct vastoken *token2 = malloc(sizeof(struct vastoken));
        if (*str == '%') {
            str++;
            val = vasgetreg(str, &sft, &str);
            token2->type = VASTOKENREG;
            token2->sft = sft;
            token2->data.reg = val;
        } else if ((*str) && (isalpha(*str) || *str == '_')) {
            name = vasgetsym(str, &str);
            if (name) {
                token1->type = VASTOKENSYM;
                token1->data.sym.name = name;
            } else {
                fprintf(stderr, "invalid token %s\n", vaslinebuf);

                exit(1);
            }
        } else {
            fprintf(stderr, "invalid token %s\n", vaslinebuf);

            exit(1);
        }
        vasqueuetoken(token1);
        vasqueuetoken(token2);
    } else if ((*str) && (isalpha(*str) || *str == '_')) {
        name = vasgetdef(str, &str);
        if (name) {
            while (isspace(*str)) {
                str++;
            }
            if (vasgetvalue(str, &val, &str)) {
                token1->type = VASTOKENDEF;
                token1->data.def.name = name;
                token1->data.def.val = val;
            }
            vasqueuetoken(token1);
        }
    } else {
        vasfreetoken(token1);
    }
    *retptr = str;

    return;
}

static struct vastoken *
vasprocvalue(struct vastoken *token, vasuword adr,
             vasuword *retadr)
{
    vasuword        ret = adr + token->data.value.sft;
    char            *valptr = vasadrtoptr(adr);
    struct vastoken *retval;

    switch (token->data.value.sft) {
        case 1:
            *valptr = token->data.value.val;

            break;
        case 2:
            *((uint16_t *)valptr) = token->data.value.val;

            break;
        case 4:
            *((uint32_t *)valptr) = token->data.value.val;

            break;
        case 8:
            *((uint64_t *)valptr) = token->data.value.val;

            break;
    }
    *retadr = ret;
    retval = token->next;
    vasfreetoken(token);

    return retval;
}

static struct vastoken *
vasproclabel(struct vastoken *token, vasuword adr,
             vasuword *retadr)
{
    struct vassymrec *sym;
    struct vastoken  *retval;

    if (!_startset && !strncmp(token->data.label.name, "_start", 6)) {
#if 0
        if (adr & (sizeof(vasop_t) - 1)) {
            adr = roundup2(adr, sizeof(vasop_t));
        }
#endif
        _startadr = adr;
        _startset = 1;
    }
    sym = vasfindsym(token->data.label.name);
    if (sym) {
        sym->adr = adr;
    } else {
        sym = malloc(sizeof(struct vassymrec));
        sym->name = token->data.label.name;
        sym->adr = adr;
        vasaddsym(sym);
    }
    *retadr = adr;
    retval = token->next;
    vasfreetoken(token);

    return retval;
}

static struct vastoken *
vasprocchar(struct vastoken *token, vasuword adr,
            vasuword *retadr)
{
    char            *valptr = vasadrtoptr(adr);
    struct vastoken *retval;

    *valptr = token->data.ch;
    adr++;
    *retadr = adr;
    retval = token->next;
    vasfreetoken(token);

    return retval;
}

static struct vastoken *
vasprocdata(struct vastoken *token, vasuword adr,
            vasuword *retadr)
{
    struct vastoken *token1 = token->next;
    vasuword        valadr = adr;

    while ((token1) && token1->type == VASTOKENVALUE) {
        token1->data.value.sft = token->data.sft;
        token1 = vasprocvalue(token1, valadr, &valadr);
    }
    *retadr = valadr;

    return token1;
}

static struct vastoken *
vasprocglobl(struct vastoken *token, vasuword adr,
             vasuword *retadr)
{
    struct vastoken *token1;
    struct vaslabel *label;

    token1 = token->next;
    while ((token1) && token1->type == VASTOKENSYM) {
        token1 = vasproclabel(token1, adr, &adr);
        label = malloc(sizeof(label));
        label->name = strdup(token1->data.label.name);
        label->adr = adr;
        vasaddglob(label);
    }
    *retadr = adr;

    return token1;
}

static struct vastoken *
vasprocspace(struct vastoken *token, vasuword adr,
             vasuword *retadr)
{
    struct vastoken *token1;
    struct vastoken *token2;
    vasuword      spcadr;
    char            *ptr;
    uint8_t          val;

    token1 = token->next;
    if ((token1) && token1->type == VASTOKENVALUE) {
        spcadr = token1->data.value.val;
        token2 = token1->next;
        if ((token2) && token2->type == VASTOKENVALUE) {
            ptr = vasadrtoptr(spcadr);
            val = token2->data.value.val;
            while (adr < spcadr) {
                ptr[0] = val;
                adr++;
                ptr++;
            }
            token1 = token2->next;
        } else {
            adr = spcadr;
        }
    } else {
        fprintf(stderr, "invalid .space attribute token type %lx\n",
                token1->type);

        exit(1);
    }
    *retadr = adr;

    return token1;
}

static struct vastoken *
vasprocorg(struct vastoken *token, vasuword adr,
           vasuword *retadr)
{
    struct vastoken *token1;
    vasuword      orgadr;
    char            *ptr;
    uint8_t          val;

    token1 = token->next;
    if ((token) && token->type == VASTOKENVALUE) {
        ptr = vasadrtoptr(adr);
        orgadr = token->data.value.val;
        val = token->data.value.val;
        while (adr < orgadr) {
            *ptr++ = val;
            adr++;
        }
        *retadr = adr;
    }

    return token1;
}

static struct vastoken *
vasprocalign(struct vastoken *token, vasuword adr,
             vasuword *retadr)
{
    struct vastoken *token1 = token->next;

    if ((token1) && token1->type == VASTOKENVALUE) {
        adr = roundup2(adr, token1->data.value.val);
    } else {
        fprintf(stderr, "invalid .align attribute token type %ld\n",
                token1->type);

        exit(1);
    }
    token1 = token1->next;
    *retadr = adr;

    return token1;
}

static struct vastoken *
vasprocasciz(struct vastoken *token, vasuword adr,
             vasuword *retadr)
{
    struct vastoken *token1 = token->next;
    long             len = 0;
    char            *ptr;
    char            *str;

    while ((token1) && token1->type == VASTOKENSTRING) {
        ptr = vasadrtoptr(adr);
        str = token1->data.str;
        while ((*str) && *str != '\"') {
            if (*str == '\\') {
                str++;
                switch (*str) {
                    case 'n':
                        *ptr++ = '\n';
                        str++;

                        break;
                    case 't':
                        *ptr++ = '\t';
                        str++;

                        break;
                    case 'r':
                        *ptr++ = '\r';
                        str++;

                        break;
                    default:
                        fprintf(stderr, "invalid character literal: \'%s\n", str);

                        exit(1);
                }
            } else {
                *ptr++ = *str++;
            }
            len++;
        }
        if (*str == '\"') {
            *ptr = '\0';
            len++;
        }
        adr += len;
        token1 = token1->next;
    }
    vasfreetoken(token);
    *retadr = adr;

    return token1;
}

void
vasinit(void)
{
    vasinitbuf();
#if (VASALIGN)
    vasinitalign();
#endif
}

vasuword
vastranslate(vasuword base)
{
    vasuword         adr = base;
    struct vastoken *token = vastokenqueue;
    struct vastoken *token1 = NULL;
    vastokfunc_t    *func;

    while (token) {
        vasprinttoken(token);
        func = vastokfunctab[token->type];
        if (func) {
            //            fprintf(stderr, "HANDLING token of type %s (%ld)\n", vastoknametab[token->type], token->type);
#if (VASALIGN) && 0
            adr = vasaligntok(adr, token->type);
#endif
            token1 = func(token, adr, &adr);
            if (!token1) {

                break;
            }
        } else if (token) {
            fprintf(stderr, "stray token of type %s (%ld) found\n",
                    vastoknametab[token->type], token->type);
            // vasprinttoken(token);

            exit(1);
        }
        token = token1;
    }

    return base;
}

void
vasresolve(void)
{
    struct vassymrec *sym = symqueue;
    struct vassymrec *sym1;
    struct vassymrec *item;
    struct vaslabel  *label;

    while (sym) {
        if (sym->adr == VASRESOLVE) {
            fprintf(stderr, "unresolved symbol %s\n", sym->name);

            exit(1);
        }
        item = vasfindsym(sym->name);
        if (item) {
            if (item->adr == VASRESOLVE) {
                fprintf(stderr, "invalid symbol %s\n", item->name);

                exit(1);
            }
            *((vasuword *)sym->adr) = item->adr;
        } else {
            label = vasfindglob(sym->name);
            if (label) {
                *((vasuword *)sym->adr) = label->adr;
            } else {
                fprintf(stderr, "unresolved symbol %s\n", sym->name);

                exit(1);
            }
        }
        sym1 = sym;
        sym = sym->next;
        free(sym1);
    }
    symqueue = NULL;

    return;
}

#if (VASBUF) && !(VASMMAP)
void
vasreadfile(char *name, vasuword adr, int bufid)
#else
    void
    vasreadfile(char *name, vasuword adr)
#endif
{
#if (VASMMAP)
    struct vasmap    map;
    struct stat      statbuf;
    int              sysret;
    char            *base;
#endif
    long             buflen = VAS_LINE_BUFSIZE;
#if (VASBUF) || (VASMMAP)
    int              fd = -1;
#elif !(VASMMAP)
    FILE            *fp = fopen((char *)name, "r");
#endif
    long             eof = 0;
    struct vasval   *def;
    char            *fname;
    char            *ptr;
    char            *str = vaslinebuf;
    char            *lim = NULL;
    long             loop = 1;
    int              ch;
    long             comm = 0;
    long             empty = 1;
    long             len = 0;
#if (VASDB)
    unsigned long    line = 0;
#endif
    vasword          val = 0;

#if (VASBUF) || (VASMMAP)
    do {
        fd = open((const char *)name, O_RDONLY);
        if (fd < 0) {
            if (errno == EINTR) {

                continue;
            } else {

                break;
            }
        }
    } while (fd < 0);
#endif
#if (VASMMAP)
    do {
        sysret = stat(name, &statbuf);
    } while (sysret < 0 && errno == EINTR);
    if (sysret < 0) {
        fprintf(stderr, "cannot stat %s\n", name);
    }
    base = mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (base == (void *)(-1)) {
        fprintf(stderr, "cannot map %s\n", name);
    }
    map.adr = base;
    map.cur = base;
    map.sz = statbuf.st_size;
    map.lim = base + map.sz;
#endif
    while (loop) {
#if (VASMMAP)
        if (map.cur > map.adr + map.sz) {

            break;
        }
#endif
        if (empty) {
            if (eof) {
                loop = 0;
                //                empty = 0;

                break;
            }
            str = vaslinebuf;
            empty = 0;
#if (VASMMAP)
            ch = vasgetc(&map);
#elif (VASBUF)
            ch = vasgetc(fd, bufid);
#else
            ch = fgetc(fp);
#endif
            if (ch == EOF) {
                loop = 0;

                break;
            } else {
                len = 0;
#if (VASDB)
                line++;
#endif
                while (ch != EOF && ch != '\n') {
                    *str++ = ch;
                    len++;
                    if (len == buflen) {
                        fprintf(stderr, "overlong line (%ld == %ld): %s\n",
                                len, buflen, vaslinebuf);

                        exit(1);
                    }
#if (VASMMAP)
                    ch = vasgetc(&map);
#elif (VASBUF)
                    ch = vasgetc(fd, bufid);
#else
                    ch = fgetc(fp);
#endif
                }
                eof = (ch == EOF);
                *str = '\0';
                str = vaslinebuf;
                lim = str + len;
                while ((*str) && isspace(*str)) {
                    str++;
                }
                if (str > lim) {
                    empty = 1;
                }
            }
        } else if (!strncmp((char *)str, ".define", 7)) {
            str += 7;
            while ((*str) && isspace(*str)) {
                str++;
            }
            if ((*str) && (isalpha(*str) || *str == '_')) {
                ptr = str;
                str++;
                while ((*str) && (isalnum(*str) || *str == '_')) {
                    str++;
                }
                *str++ = '\0';
                while ((*str) && isspace(*str)) {
                    str++;
                }
                if (vasgetvalue(str, &val, &str)) {
                    def = malloc(sizeof(struct vasval));
                    def->name = strdup((char *)ptr);
                    def->val = val;
                    vasaddval(def);
                } else {
                    fprintf(stderr, "invalid .define directive %s\n", ptr);

                    exit(1);
                }
            }
        } else if (!strncmp((char *)str, ".include", 8)) {
            str += 8;
            while ((*str) && isspace(*str)) {
                str++;
            }
            if (*str == '<') {
                str++;
                fname = str;
                while ((*str) && *str != '>') {
                    str++;
                }
                if (*str == '>') {
                    *str = '\0';
#if (VASBUF) && !(VASMMAP)
                    vasreadfile((char *)fname, adr, ++vasreadbufcur);
                    bufid--;
#else
                    vasreadfile((char *)fname, adr);
#endif
                    vasresolve();
                    vasfreesyms();
                } else {
                    fprintf(stderr, "invalid .include directive %s\n",
                            str);

                    exit(1);
                }
            }
            //            done = 1;
        } else if (!strncmp((char *)str, ".import", 7)) {
            str += 7;
            while ((*str) && isspace(*str)) {
                str++;
            }
            if (*str == '<') {
                str++;
                fname = str;
                while ((*str) && *str != '>') {
                    str++;
                }
                if (*str == '>') {
                    *str = '\0';
#if (VASBUF) && !(VASMMAP)
                    vasreadfile((char *)fname, adr, ++bufid);
                    bufid--;
#else
                    vasreadfile((char *)fname, adr);
#endif
                    vasresolve();
                    vasfreesyms();
                } else {
                    fprintf(stderr, "invalid .import directive %s\n",
                            vasstrbuf);

                    exit(1);
                }
            }
            //            empty = 1;
        } else if (str[0] == ';'
                   || (str[0] == '/' && str[1] == '/')) {
            /* the rest of the line is comment */
            empty = 1;
        } else if (str[0] == '/' && str[1] == '*') {
            /* comment */
            comm = 1;
            while (comm) {
#if (VASMMAP)
                ch = vasgetc(&map);
#elif (VASBUF)
                ch = vasgetc(fd, bufid);
#else
                ch = fgetc(fp);
#endif
                if (ch == EOF) {
                    loop = 0;

                    break;
#if (VASDB)
                } else if (ch == '\n') {
                    line++;
#endif
                } else if (ch == '*') {
#if (VASMMAP)
                    ch = vasgetc(&map);
#elif (VASBUF)
                    ch = vasgetc(fd, bufid);
#else
                    ch = fgetc(fp);
#endif
                    if (ch == '/') {

                        comm = 0;
                    } else if (ch == EOF) {
                        comm = 0;
                        loop = 0;
                        eof = 1;
                    }
                }
            }
            empty = 1;
        } else if (*str) {
#if (VASMMAP)
            vasgettoken(str, &str, &map);
#else
            vasgettoken(str, &str);
#endif
#if (VASDB)
            if (token) {
                token->file = strdup((char *)name);
                token->line = line;
                vasqueuetoken(token);
            }
#endif
            while (isspace(*str)) {
                str++;
            }
            if (str >= lim) {
                empty = 1;
            }
        } else {
            empty = 1;
        }
    }
#if (VASBUF) || (VASMMAP)
    close(fd);
#elif (VASMMAP)
    fclose(fp);
    munmap(map.adr, map.sz);
#endif

    return;
}

