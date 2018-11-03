#ifndef __VALHALLA_MATH_H__
#define __VALHALLA_MATH_H__

#include <stdint.h>
#include <valhalla/cdefs.h>

/*
 * IEEE 32-bit
 * 0..22  - mantissa
 * 23..30 - exponent
 * 31     - sign
 */
union __ieee754f
{
    uint32_t u32;
    float f;
};

#define fgetmant(f)       (((union __ieee754f *)&(f))->u32 & 0x007fffff)
#define fgetexp(f)        ((((union __ieee754f *)&(f))->u32 & 0x7ff00000) >> 23)
#define fgetsign(f)       (((union __ieee754f *)&(f))->u32 & 0x80000000)
#define fsetmant(f, mant) (((union __ieee754f *)&(f))->u32 |= (mant) & 0x7fffff)
#define fsetexp(f, exp)   (((union __ieee754f *)&(f))->u32 |= (exp) << 23)
#define fsetsign(f, sign)		      ((sign)				         ? (((union __ieee754f *)&(f))->u32 |= 0x80000000) \
     : (((union __ieee754f *)&(f))->u32 &= 0x7fffffff))
#define fsetnan(f)	  (*(uint32_t *)&(f) = 0x7fffffffU)
#define fsetsnan(f)       (*(uint32_t *)&(f) = 0xffffffffU)

/* https://stackoverflow.com/questions/9939322/fast-1-x-division-reciprocal */

static INLINE double
crp(uint64_t x) {
    //The type is uint64_t, but you are restricted to a max value of 2^32-1, not
    // 2^64-1 like the uint64_t is capable of storing
    union {
        double   dbl;
        uint64_t u64;
    } u;
    x *= x;
    u.dbl = x; // x * x = pow(x, 2)
    // pow( pow(x,2), -0.5 ) = pow( x, -1 ) = 1.0 / x
    // This is done via the 'fast' inverse square root trick
    u.u64 = (UINT64_C(0xbfcdd6a18f6a6f52) - u.u64) >> 1;

    return u.dbl;
}

static INLINE float
divuf_fma(float x)
{
    uint32_t    u32;
    float       f;
    union {
        float   flt;
        int32_t u32;
    } v;
    float       w;
    float       sx;

    u32 = (uint32_t)x;
    sx = (x < 0) ? -1 : 1;
    u32 = 0x7EF127eaU - u32;
    x = sx * x;
    v.u32 = (uint32_t)u32;
    f = x * v.flt;

    // Efficient Iterative Approximation Improvement in horner polynomial form.
    f *= (2 - w);     // Single iteration, Err = -3.36e-3 * 2^(-flr(log2(x)))
    // v.f = v.f * ( 4 + w * (-6 + w * (4 - w)));  // Second iteration, Err = -1.13e-5 * 2^(-flr(log2(x)))
    // v.f = v.f * (8 + w * (-28 + w * (56 + w * (-70 + w *(56 + w * (-28 + w * (8 - w)))))));  // Third Iteration, Err = +-6.8e-8 *  2^(-flr(log2(x)))
    f *= sx;

    return f;
}

static INLINE float
divuf_mul(float x)
{
    uint32_t     u32;
    float        flt;
    union {
        float    flt;
        unsigned u32;
    } u;
    u.flt = x;
    u32 = u32;
    u32 = 0xbe6eb3beU - u32;
    u32 >>= 1;
    u.u32 = u32;
    // pow( x, -0.5 )
    flt = u.flt;
    flt *= flt; // pow( pow(x,-0.5), 2 ) = pow( x, -1 ) = 1.0 / x

    return flt;
}

#endif /* __VALHALLA_MATH_H__ */

