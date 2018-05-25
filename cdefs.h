#ifndef __VALHALLA_CDEFS_H__
#define __VALHALLA_CDEFS_H__

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#   define EMPTY
#else
#   define EMPTY  0
#endif
#define    INLINE __inline__ __attribute__ ((__always_inline__))
#define    ALIGNED(a) __attribute__ ((__aligned__(a)))

#define    rounduppow2(a, b2) (((a) + ((b2) - 0x01)) & -(b2))

#define tscmp(ts1, ts2)                                                \
    (((ts2).tv_sec - (ts1).tv_nsec) * 1000000000                       \
     + ((ts2).tv_nsec - (ts1).tv_nsec))

#endif /* __VALHALLA_CDEFS_H__ */

