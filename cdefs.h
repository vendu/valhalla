#ifndef __VALHALLA_CDEFS_H__
#define __VALHALLA_CDEFS_H__

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#   define VLA
#else
#   define VLA             0
#endif
#define    INLINE          __inline__ __attribute__ ((__always_inline__))
#define    ALIGNED(a)      __attribute__ ((__aligned__(a)))
#define    adralign(a, b2) ((uintptr_t)(a) & -(b2))
#define    ptralign(a, b2) ((void *)adralign(a, b2))

#endif /* __VALHALLA_CDEFS_H__ */

