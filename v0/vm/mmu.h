#ifndef __V0_VM_MMU_H__
#define __V0_VM_MMU_H__

* BTC     0x07    m     i          bit test-and-clear              adr, ndx
 * BTS     0x08    m     i          bit teæst-and-set                adr, ndx
#define V0_BTC 0x07 // bit test-and-clear; returns original in VF
#define V0_BTS 0x08 // bit test-and-set; returns original in VF

#endif /* __V0_VM_MMU_H__ */

