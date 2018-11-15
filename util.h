#ifndef __VALHALLA_UTIL_H__
#define __VALHALLA_UTIL_H__

#include <limits.h>
#include <stdint.h>
#include <valhalla/cdefs.h>
#include <valhalla/param.h>

#define tscmp(ts1, ts2)                                                \
    (((ts2).tv_sec - (ts1).tv_nsec) * 1000000000                       \
     + ((ts2).tv_nsec - (ts1).tv_nsec))

static INLINE uint32_t
mulhu32(uint32_t a, uint32_t b)
{
    uint64_t u64a = a;
    uint64_t u64b = b;

    u64a *= u64b;
    u64a >>= 32;

    return (uint32_t)u64a;
}

#define clz32(u) (!(u) ? 32 : __builtin_clz(u))

/*
 * round longword u to next power of two if not power of two
 */
static INLINE uint64_t
ceil2u32(uint32_t u)
{
    int32_t  tmp = 32 - clz32(u);
    uint32_t ret = 0;

    if (u) {
        if (!powerof2(u)) {
            tmp++;
        }
        ret = 1UL << tmp;
    }

    return ret;
}

static INLINE uint32_t
divu17(uint32_t uval)
{
        uint32_t mul = UINT32_C(0xf0f0f0f1);
        uint32_t res = uval;

        res *= mul;
        res >>= 4;
        uval = (unsigned long)res;

        return uval;
}

#if 0
static INLINE uint32_t
divu32(uint32_t num, uint32_t div)
{
    uint32_t cnt = clz32(div);
    uint64_t val;
    uint64_t den;
    uint64_t tmp;
    uint64_t res;

    den = div;
    cnt++;
    val = num;
    den <<= cnt;
    val <<= cnt;
    tmp = 48/17 + 32 * den / 17;
    res = tmp + tmp * (1 - den * tmp);
    res *= num;

    return (uint32_t)res;
}
#endif

#endif /* __VALHALLA_UTIL_H__ */

