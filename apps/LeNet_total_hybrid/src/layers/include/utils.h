#ifndef UTILS_H
#define UTILS_H

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>

#define LEA_FLT_SIZE 100
#define LEA_SRC_SIZE 100
#define LEA_DST_SIZE 100
#define LEA_MIN_SIZE 100

#define LUPE_MIN 0x8000
#define LUPE_Q15_MAX 32766 // 2^15 - 2

#define MAKE_ALIGN_2(s) ((s) + ((s) & 0x1))

extern _q15 lea_flt[LEA_FLT_SIZE];
extern _q15 lea_src[LEA_SRC_SIZE];
extern _q15 lea_tmp[LEA_DST_SIZE];
extern _q15 lea_dst[LEA_DST_SIZE];
extern _iq31 lea_res[2];

extern DMA_initParam dma_config;

#define __STICH(A, B) A##B
#define STICH(A, B) __STICH(A, B)
#define __STIC3(A, B, C) A##B##C
#define STIC3(A, B, C) __STIC3(A, B, C)

// Should generate the channel instead of hardcode 0
// #define DMA_makeTransfer_opt(CHANNEL, SRCADDR, DSTADDR, SIZE) {             \
//     STIC3(DMA, CHANNEL, CTL) = DMADT_1 | DMADSTINCR_3 | DMASRCINCR_3 |      \
//       DMADSTBYTE__WORD | DMASRCBYTE__WORD | DMAIE;                          \
//     __data16_write_addr((uintptr_t)(&STIC3(DMA, CHANNEL, SA)), SRCADDR);    \
//     __data16_write_addr((uintptr_t)(&STIC3(DMA, CHANNEL, DA)), DSTADDR);    \
//     STIC3(DMA, CHANNEL, SZ) = SIZE;                                         \
//     STIC3(DMA, CHANNEL, CTL) |= DMAEN | DMAREQ;                             \
//     __bis_SR_register(GIE + LPM0_bits);                                     \
//   }

#define DMA_makeTransfer(SRCADDR, DSTADDR, SIZE) {                     \
  STIC3(DMA, 0, CTL) = DMADT_1 | DMADSTINCR_3 | DMASRCINCR_3 |         \
    DMADSTBYTE__WORD | DMASRCBYTE__WORD | DMAIE;                       \
  __data16_write_addr((uintptr_t)(&STIC3(DMA, 0, SA)), SRCADDR);       \
  __data16_write_addr((uintptr_t)(&STIC3(DMA, 0, DA)), DSTADDR);       \
  STIC3(DMA, 0, SZ) = SIZE;                                            \
  STIC3(DMA, 0, CTL) |= DMAEN | DMAREQ;                                \
  __bis_SR_register(GIE + LPM0_bits);                                  \
}

void init_lupe();

#endif