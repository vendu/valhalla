#ifndef __VALHLLA_PARAM_H__
#define __VALHLLA_PARAM_H__

#define min(a, b) ((b) ^ (((a) ^ (b)) & -((a) < (b))))
#define max(a, b) ((a) ^ (((a) ^ (b)) & -((a) < (b))))

#define roundup2(a, b2) (((a) + ((b2) - 0x01)) & -(b2))
#define rounddown2(a, b2) ((a) & ~((b2) - 0x01))

#endif /*f __VALHLLA_PARAM_H__ */

