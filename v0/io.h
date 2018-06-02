#ifndef __V0_IO_H__
#define __V0_IO_H__

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <v0/vm.h>

#define V0_TMR_HIRES    0x80000000
#define V0_TMR_INTERVAL 0x40000000
#define v0mkitmr(hz)

void v0readkbd(struct v0 *vm, uint8_t port, v0ureg reg);
void v0writetty(struct v0 *vm, uint8_t port, v0ureg val);
void v0writeerr(struct v0 *vm, uint8_t port, v0ureg val);
void v0readrtc(struct v0 *vm, uint8_t port, v0ureg reg);
void v0readtmr(struct v0 *vm, uint8_t port, v0ureg reg);
void v0conftmr(struct v0 *vm, uint8_t port, v0ureg val);
void v0writevtd(struct v0 *vm, uint8_t port, v0ureg val);

#endif /* __V0_IO_H__ */

