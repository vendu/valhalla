#ifndef __V0_VM_MMU_H__
#define __V0_VM_MMU_H__

/* Memory Management & Synchronization
 * -----------------------------------
 *
 * Instructions
 * ------------
 *
 * Mnemo   Opcode  Src  Dest        Brief                           Arguments
 * -----   ------  ---  ----        -----                           ---------
 * BAR     0x00                     full memory barrier             none
 * BRD     0x01                     memory read-barrier             none
 * BWR     0x02                     memory write-barrier            none
 * CPF     0x03    m                cache prefetch                  word-address
 * CWB     0x04    m                cache write-back                word-address
 * FPG     0x05    m                flush memory TLB-entry          word-address
 * FLS     0x06    m    ri          flush cachelines                count
 * BTC     0x07    m     i          bit test-and-clear              adr, ndx
 * BTS     0x08    m     i          bit test-and-set                adr, ndx
 */
#define V0_BAR 0x00 // full memory barrier/synchronisation
#define V0_BRD 0x01 // memory read-barrier
#define V0_BWR 0x02 // memory write-barrier
#define V0_CPF 0x03 // cacheline memory-prefetch
#define V0_CWB 0x04 // cacheline memory-flush (write-back)
#define V0_FPG 0x05 // flush page-entry
#define V0_FLS 0x06 // flush cachelines
#define V0_BTC 0x07 // bit test-and-clear; returns original in VF
#define V0_BTS 0x08 // bit test-and-set; returns original in VF

#endif /* __V0_VM_MMU_H__ */
