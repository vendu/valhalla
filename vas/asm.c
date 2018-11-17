#include <stddef.h>

/*
 * file.c
 * ------
 * asmreadfile()                buffered file I/O
 * asmgetline()                 read and translate code line
 * asmreadline()                read line into an asmop structure
 */

/* assembler pseudo-machine instruction */
struct asm {
    void   *base;               // translation core base address
    size_t  size;               // core size
    size_t  nseg;               // # of segments present
    long   *segflg;             // segment permissions and other flags
    void   *segtab;
};

