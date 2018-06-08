#ifndef __VALHALLA_V0_MMU_H__
#define __VALHALLA_V0_MMU_H__

/* Memory Management & Synchronisation
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
 * CLD     0x05    m    r           cache load linked               word-address
 * CST     0x06    r    m           cache store conditional (if cacheline clean)
 * FPG     0x07    m                flush memory TLB-entry          word-address
 * FLS     0x08    m    ri          flush cachelines                count
 * CAS     0x09    r, d, m          compare and swap                val/want/adr
 * CS2     0x0a    r, d, m          dual-word compare and swap      val/want/adr
 * BTC     0x0b    m     i          bit test-and-clear              adr, ndx
 * BTS     0x0c    m     i          bit test-and-set                adr, ndx
 */
#define V0_BAR 0x00 // full memory barrier/synchronisation
#define V0_BRD 0x01 // memory read-barrier
#define V0_BWR 0x02 // memory write-barrier
#define V0_CPF 0x03 // cacheline memory-prefetch
#define V0_CWB 0x04 // cacheline memory-flush (write-back)
#define V0_CLD 0x05 // cacheline load-linked (clear dirty-bit)
#define V0_CST 0x06 // cacheline store-conditional (check dirty-bit)
#define V0_FPG 0x07 // flush page-entry
#define V0_FLS 0x08 // flush cachelines
#define V0_CAS 0x09 // compare-and-swap
#define V0_CS2 0x0a // dual-word/pointer compare-and-swap
#define V0_BTC 0x0b // bit test-and-clear; returns original in VF
#define V0_BTS 0x0c // bit test-and-set; returns original in VF

#endif /* __VALHALLA_V0_MMU_H__ */
