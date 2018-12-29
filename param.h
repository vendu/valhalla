#ifndef __VALHLLA_PARAM_H__
#define __VALHLLA_PARAM_H__

#define min(a, b)         ((b) ^ (((a) ^ (b)) & -((a) < (b))))
#define max(a, b)         ((a) ^ (((a) ^ (b)) & -((a) < (b))))
#if !defined(powerof2)
#define powerof2(x)       (!((x) & ((x) - 1)))
#endif
#define roundup2(a, b2)   (((a) + ((b2) - 0x01)) & -(b2))
#define rounddown2(a, b2) ((a) & ~((b2) - 0x01))
#define ptralign(ptr, b2) ((void *)roundup2((uintptr_t)(ptr), (b2)))

#endif /*f __VALHLLA_PARAM_H__ */

