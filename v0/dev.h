#ifndef __V0_DEV_H__
#define __V0_DEV_H__

#define V0_TMR_HIRES 0
#define V0_HIRES_TMR (1 << V0_TMR_HIRES)

struct v0tmr {
    unsigned long mode;
};

#endif /* __V0_DEV_H__ */

